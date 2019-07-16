#include <os_type.h>
#include <osapi.h>
#include <ip_addr.h>
#include <lwip/err.h>
#include <lwip/dns.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>

#include "display.h"
#include "graphics.h"
#include "common.h"
#include "debug.h"
#include "parseobjects.h"
#include "config.h"
#include "retain.h"
#include "conv.h"
#include "gui.h"
#include "datetime.h"
#include "drivers/uart.h"

LOCAL const char *openWeatherMapHost = "api.openweathermap.org";
LOCAL const char *thingSpeakHost = "api.thingspeak.com";
LOCAL const char *tsFieldName = "field1";

#define HTTP_MSG_MAX_LEN	16000
LOCAL char *httpMsgRxBuf = NULL;
LOCAL char httpMsgTxBuf[200];
LOCAL int httpMsgCurLen = 0;

LOCAL os_timer_t gpTmr;
LOCAL os_timer_t httpRxTmr;
LOCAL struct espconn tcpSock;
LOCAL struct _esp_tcp tcpSockParams;
LOCAL ip_addr_t serverIp;
LOCAL os_timer_t timeoutTmr;

LOCAL CurWeather curWeather;
LOCAL Forecast forecastHourly[FORECAST_HOURLY_SIZE];
LOCAL Forecast forecastDaily[FORECAST_DAILY_SIZE];
LOCAL char indoorTempStr[8] = "";
struct tm curTime = {0};

LOCAL int dispUpdateDone = FALSE;
LOCAL int wifiSleeping = FALSE;
LOCAL const uint sleepTimeLong = 60UL*60UL*1000000UL;

LOCAL const uint dnsCheckInterval = 100;

typedef enum{
	stateInit,
	stateConnectToAp,
	stateGetOwmIp,
	stateConnectToOwm,
	stateGetWeather,
	stateGetForecast,
	stateGetThingSpeakIp,
	stateConnectToTs,
	stateSendTempToTs,
	stateWaitDispUpdate,
	stateGotoSleep
}AppState;
LOCAL AppState appState = stateInit;

LOCAL void ICACHE_FLASH_ATTR setAppState(AppState newState)
{
	if (appState != newState)
	{
		appState = newState;
		os_timer_arm(&timeoutTmr, 20000, 0);	// 20 s should be enough
		debug("appState %d\n", (int)appState);
	}
}

LOCAL void connectToWiFiAP(void);
LOCAL void checkWiFiConnStatus(void);
LOCAL void checkDnsStatus(void *arg);
LOCAL void getHostByNameCb(const char *name, ip_addr_t *ipaddr, void *arg);
LOCAL void onTcpConnected(void *arg);
LOCAL void sendHttpRequest(void);
LOCAL void onTcpDataSent(void *arg);
LOCAL void onTcpDataReceived(void *arg, char *pusrdata, unsigned short length);
LOCAL void onTcpDisconnected(void *arg);
LOCAL void onTcpConnFailed(void *arg, sint8 err);
LOCAL void timeoutTmrCb(void);
LOCAL void handleHttpReply(void);
LOCAL int createDailyFromHourly(const Forecast *hourly, int hCount, Forecast *daily, int dailySize);
LOCAL void wifiOff(void);
LOCAL void gotoSleep(int success);


void user_init(void)
{
	os_timer_disarm(&gpTmr);
	os_timer_disarm(&httpRxTmr);
	os_timer_setfn(&httpRxTmr, (os_timer_func_t *)handleHttpReply, NULL);
	os_timer_disarm(&timeoutTmr);	// timeout timer, normally should never fire
	os_timer_setfn(&timeoutTmr, (os_timer_func_t *)timeoutTmrCb, NULL);

	//uart_init(BIT_RATE_115200, BIT_RATE_115200);
	uart_init(BIT_RATE_921600, BIT_RATE_921600);
	os_printf("\n\n");

	retainRead(&retain);
	if (retain.longSleepCnt > 0)
	{
		retain.longSleepCnt--;
		retainWrite(&retain);
		setAppState(stateGotoSleep);
		system_deep_sleep(sleepTimeLong);
		return;
	}
	retain.attempts++;

	configRead(&config);

	debug("Built on %s %s\n", __DATE__, __TIME__);
	debug("SDK version %s\n", system_get_sdk_version());
	debug("attempts %u, fails %u, retry %u\n", retain.attempts, retain.fails, retain.retry);

	dispIfaceInit();

	setAppState(stateConnectToAp);
	wifi_set_opmode(STATION_MODE);
	connectToWiFiAP();
}

LOCAL void ICACHE_FLASH_ATTR connectToWiFiAP(void)
{
	if (retain.ipConfig.ip.addr && retain.ipConfig.netmask.addr &&
		retain.ipConfig.gw.addr && retain.dns1.addr)
	{
		wifi_station_dhcpc_stop();
		wifi_set_ip_info(STATION_IF, &retain.ipConfig);
		espconn_dns_setserver(0, &retain.dns1);
		espconn_dns_setserver(1, &retain.dns2);
	}

	struct station_config stationConf;
	stationConf.bssid_set = 0;	// mac address not needed
	os_memcpy(stationConf.ssid, config.ssid, sizeof(stationConf.ssid));
	os_memcpy(stationConf.password, config.pass, sizeof(stationConf.password));
	wifi_station_set_config(&stationConf);

	// setup timer to check connection status every 100 ms
	os_timer_setfn(&gpTmr, (os_timer_func_t *)checkWiFiConnStatus, NULL);
	os_timer_arm(&gpTmr, 100, 0);
}

LOCAL void ICACHE_FLASH_ATTR checkWiFiConnStatus(void)
{
	struct ip_info ipconfig;
	memset(&ipconfig, 0, sizeof(ipconfig));

	os_timer_disarm(&gpTmr);

	// check current connection status and own ip address
	wifi_get_ip_info(STATION_IF, &ipconfig);
	uint8 connStatus = wifi_station_get_connect_status();
	if (connStatus == STATION_GOT_IP && ipconfig.ip.addr != 0)
	{
		os_memcpy(&retain.ipConfig, &ipconfig, sizeof(ipconfig));
		retain.dns1 = dns_getserver(0);
		retain.dns2 = dns_getserver(1);

		// connection with AP established -> get openweathermap server ip
		setAppState(stateGetOwmIp);

		tcpSock.proto.tcp = &tcpSockParams;
		tcpSock.type = ESPCONN_TCP;
		tcpSock.state = ESPCONN_NONE;
		serverIp.addr = 0;
		espconn_gethostbyname(&tcpSock, openWeatherMapHost, &serverIp, getHostByNameCb);

		os_timer_setfn(&gpTmr, (os_timer_func_t *)checkDnsStatus, &tcpSock);
		os_timer_arm(&gpTmr, dnsCheckInterval, 0);
	}
	else
	{
		if (connStatus == STATION_WRONG_PASSWORD ||
			connStatus == STATION_NO_AP_FOUND	 ||
			connStatus == STATION_CONNECT_FAIL)
		{
			debug("Failed to connect to AP (status: %u)\n", connStatus);
			gotoSleep(FALSE);
		}
		else
		{
			// not yet connected, recheck later
			os_timer_setfn(&gpTmr, (os_timer_func_t *)checkWiFiConnStatus, NULL);
			os_timer_arm(&gpTmr, 100, 0);
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR checkDnsStatus(void *arg)
{
    struct espconn *pespconn = arg;	
    if (appState == stateGetOwmIp)
    {
		espconn_gethostbyname(pespconn, openWeatherMapHost, &serverIp, getHostByNameCb);
		os_timer_arm(&gpTmr, dnsCheckInterval, 0);		
    }
    else if (appState == stateGetThingSpeakIp)
    {
		espconn_gethostbyname(pespconn, thingSpeakHost, &serverIp, getHostByNameCb);
		os_timer_arm(&gpTmr, dnsCheckInterval, 0);		
    }
}

LOCAL void ICACHE_FLASH_ATTR getHostByNameCb(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;
	os_timer_disarm(&gpTmr);

	if (serverIp.addr != 0)
	{
		debug("getHostByNameCb serverIp != 0\n");
		return;
	}
	if (ipaddr == NULL || ipaddr->addr == 0)
	{
		debug("getHostByNameCb ip NULL\n");
		return;
	}
	debug("getHostByNameCb ip: "IPSTR"\n", IP2STR(ipaddr));
	
	setAppState((appState == stateGetOwmIp) ? stateConnectToOwm : stateConnectToTs);

	// connect to server
	serverIp.addr = ipaddr->addr;
	os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
	pespconn->proto.tcp->remote_port = 80;	// use HTTP port
	pespconn->proto.tcp->local_port = espconn_port();	// get next free local port number

	// register callbacks
	espconn_regist_connectcb(pespconn, onTcpConnected);
	espconn_regist_reconcb(pespconn, onTcpConnFailed);

	espconn_connect(pespconn); // tcp connect
}

LOCAL void ICACHE_FLASH_ATTR onTcpConnected(void *arg)
{
	struct espconn *pespconn = arg;

	// register callbacks
	espconn_regist_recvcb(pespconn, onTcpDataReceived);
	espconn_regist_sentcb(pespconn, onTcpDataSent);
	espconn_regist_disconcb(pespconn, onTcpDisconnected);

	if (appState == stateConnectToOwm)
	{
		// send weather request
		setAppState(stateGetWeather);
		sendHttpRequest();
	}
	else if (appState == stateConnectToTs)
	{
		// send indoor temp to ThingSpeak
		setAppState(stateSendTempToTs);
		sendHttpRequest();
	}
}

LOCAL void ICACHE_FLASH_ATTR sendHttpRequest(void)
{
	switch (appState)
	{
	case stateGetWeather:
		if (!config.appid[0] || !config.city[0])
			return;
		os_sprintf(httpMsgTxBuf,
			"GET /data/2.5/weather?%s=%s&mode=json&lang=en&APPID=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			retain.cityId[0] ? "id" : "q",
			retain.cityId[0] ? retain.cityId : config.city,
			config.appid, openWeatherMapHost);
		break;
	case stateGetForecast:
		if (!config.appid[0] || !config.city[0])
			return;
		os_sprintf(httpMsgTxBuf,
			"GET /data/2.5/forecast?%s=%s&mode=json&cnt=%d&lang=en&APPID=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			retain.cityId[0] ? "id" : "q",
			retain.cityId[0] ? retain.cityId : config.city,
			FORECAST_HOURLY_SIZE,
			config.appid, openWeatherMapHost);
		break;
	case stateSendTempToTs:
		if (!config.tsApiKey[0] || !indoorTempStr[0])
			return;
		os_sprintf(httpMsgTxBuf,
			"GET /update?api_key=%s&%s=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			config.tsApiKey, tsFieldName, indoorTempStr, thingSpeakHost);
		break;
	default:
		debug("sendHttpRequest appState %d\n", appState);
		return;
	}

	httpMsgCurLen = 0;
	espconn_sent(&tcpSock, (uint8*)httpMsgTxBuf, os_strlen(httpMsgTxBuf));
}

LOCAL void ICACHE_FLASH_ATTR onTcpDataSent(void *arg)
{
	debug("onTcpDataSent\n");
	if (appState == stateSendTempToTs)
	{
		if (!dispUpdateDone)
		{
			setAppState(stateWaitDispUpdate);
		}
		else
		{
			gotoSleep(TRUE);
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR onTcpDataReceived(void *arg, char *pusrdata, unsigned short length)
{
	debug("onTcpDataReceived %d\n", length);
	if (appState == stateGetWeather || appState == stateGetForecast)
	{
		if (!httpMsgRxBuf)
		{
			httpMsgRxBuf = os_malloc(HTTP_MSG_MAX_LEN);
			debug("free heap %u\n", system_get_free_heap_size());
			if (!httpMsgRxBuf)
			{
				debug("httpMsgRxBuf malloc failed\n");
				return;
			}
		}

		if ((HTTP_MSG_MAX_LEN-httpMsgCurLen) > length)
		{
			os_memcpy(httpMsgRxBuf+httpMsgCurLen, pusrdata, length);
			httpMsgCurLen += length;
			os_timer_disarm(&httpRxTmr);
			os_timer_arm(&httpRxTmr, 100, 0);
		}
		else
		{
			debug("http message too long\n");
		}
	}
	else if (appState == stateWaitDispUpdate)
	{
	    // ignore reply from ThingSpeak
	    espconn_disconnect(&tcpSock);
	}
}

LOCAL void ICACHE_FLASH_ATTR onTcpDisconnected(void *arg)
{
	debug("onTcpDisconnected\n");
	switch (appState)
	{
	case stateGetWeather:
	case stateGetForecast:
		debug("reconnect to owm\n");
		setAppState(stateConnectToOwm);
		espconn_connect(&tcpSock);
		break;
	case stateSendTempToTs:
		debug("reconnect to ts\n");
		setAppState(stateConnectToTs);
		espconn_connect(&tcpSock);
		break;
	case stateWaitDispUpdate:
		// no need for wifi anymore, turn off to save battery
		wifiOff();
		break;
	}
}

LOCAL void ICACHE_FLASH_ATTR onTcpConnFailed(void *arg, sint8 err)
{
	if (appState != stateGotoSleep)
	{
		debug("onTcpConnFailed, appState %d\n", (int)appState);
		gotoSleep(FALSE);
	}
}


LOCAL void ICACHE_FLASH_ATTR handleHttpReply(void)
{
	os_timer_disarm(&gpTmr);
	httpMsgRxBuf[httpMsgCurLen] = '\0';
	debug("httpMsgCurLen %d\n", httpMsgCurLen);
	//debug("%s\n", httpMsgRxBuf);

	int validReply = FALSE;
    int statusCode = 0;
	const char *body = os_strstr(httpMsgRxBuf, "\r\n\r\n");
	const char *json = NULL;
	if (body)
	{
		json = os_strstr(body, "{");
		if (json)
		{
		    int jsonLen = httpMsgCurLen - (json - httpMsgRxBuf);
			if (appState == stateGetWeather)
			{
			    ParsedFields parsed = parseWeather(json, jsonLen,
			                                       &curWeather,
			                                       retain.cityId, sizeof(retain.cityId),
			                                       &statusCode);
				if (parsed == eWeatherAllParsed)
				{
					validReply = TRUE;
					setAppState(stateGetForecast);
					sendHttpRequest();
				}
				else
				{
					debug("parseWeather failed (0x%02X, %d)\n", parsed, statusCode);
				}
			}
			else if (appState == stateGetForecast)
			{
				int hourlyCount = parseForecast(json, jsonLen,
				                                forecastHourly, FORECAST_HOURLY_SIZE,
				                                &statusCode);
				if (hourlyCount > 0)
				{
					validReply = TRUE;

					os_free(httpMsgRxBuf);	// all the data is parsed now -> we can free the buffer
					httpMsgRxBuf = NULL;

					float indoorTemp = 0;
					if (dispReadTemp(&indoorTemp) == OK)
					{
					    int intpart;
					    int fractpart = modfInt(indoorTemp, 1, &intpart);
						ets_snprintf(indoorTempStr, sizeof(indoorTempStr), "%d.%d", intpart, fractpart);
					}

					epochToTm(curWeather.datetime, &curTime);

					Forecast* hourlyShown = NULL;
                    Forecast* dailyShown = NULL;
					int hourlyShownCnt = 0;
					int dailyShownCnt = 0;
				    switch (config.chart)
				    {
				    case eNoChart:
				    {
                        int dailyCount = createDailyFromHourly(forecastHourly, hourlyCount, forecastDaily, FORECAST_DAILY_SIZE);
                        dailyShown = forecastDaily;
                        dailyShownCnt = MIN(dailyCount, FORECAST_DAILY_CNT);
				    }
				        break;
				    case eHourlyChart:
				    {
				        hourlyShown = forecastHourly;
				        hourlyShownCnt = MIN(hourlyCount, FORECAST_HOURLY_CHART_CNT);

				        int dailyCount = createDailyFromHourly(forecastHourly, hourlyCount, forecastDaily, FORECAST_DAILY_SIZE);
				        dailyShown = forecastDaily + 1;    // skip first day in daily
				        dailyShownCnt = MIN(dailyCount - 1, FORECAST_DAILY_CNT);
				    }
				        break;
				    case eDailyChart:
				    case eBothCharts:
				        hourlyShown = forecastHourly;
				        hourlyShownCnt = hourlyCount;
				        break;
				    }

					if (!retainedWeatherEqual(&curWeather, hourlyShown, hourlyShownCnt, dailyShown, dailyShownCnt, indoorTemp))
					{
						// save current weather and forecast in rtc memory
						retainWeather(&curWeather, hourlyShown, hourlyShownCnt, dailyShown, dailyShownCnt, indoorTemp);

						if (!gfxMem)	// alloc buffer for graphics
						{
							gfxMemAlloc();
							debug("free heap %u\n", system_get_free_heap_size());
							if (!gfxMem)
							{
								return;
							}
						}

						dispRegSetup();

						// draw current weather and forecast on the screen
						switch (config.chart)
						{
						case eNoChart:
							drawCurrentWeather(&curWeather);
							dispUpdate(eDispTopPart);

							drawForecast(dailyShown, dailyShownCnt);

                            #if GUI_SCALE == 1
                                drawIndoorTempSmall(indoorTempStr);
							#elif GUI_SCALE == 2
								drawIndoorTemp(indoorTempStr);
							#endif
							break;
						case eHourlyChart:
							drawCurrentWeatherSmall(&curWeather);

							drawHourlyForecastChart(hourlyShown, hourlyShownCnt);
							dispUpdate(eDispTopPart);

                            drawForecast(dailyShown, dailyShownCnt);

                            #if GUI_SCALE == 1
                                drawIndoorTempSmall(indoorTempStr);
							#elif GUI_SCALE == 2
						        drawIndoorTemp(indoorTempStr);
							#endif
							break;
						case eDailyChart:
						{
							drawCurrentWeather(&curWeather);
							dispUpdate(eDispTopPart);

							int dailyCount = createDailyFromHourly(forecastHourly, hourlyCount, forecastDaily, FORECAST_DAILY_SIZE);
							drawDailyForecastChart(forecastHourly, hourlyCount, forecastDaily, dailyCount);

							drawIndoorTempSmall(indoorTempStr);
						}
							break;
						case eBothCharts:
							drawCurrentWeatherSmall(&curWeather);

							drawHourlyForecastChart(hourlyShown, hourlyShownCnt);
							dispUpdate(eDispTopPart);

							if(hourlyCount > FORECAST_HOURLY_CHART_CNT)
							{
							    Forecast *hourly = forecastHourly + FORECAST_HOURLY_CHART_CNT;
							    int houtlyCnt = hourlyCount - FORECAST_HOURLY_CHART_CNT;
	                            int dailyCount = createDailyFromHourly(hourly, houtlyCnt, forecastDaily, FORECAST_DAILY_SIZE);
	                            drawDailyForecastChart(hourly, houtlyCnt, forecastDaily, dailyCount);
							}

							drawIndoorTempSmall(indoorTempStr);
							break;
						}

						retain.updates++;
						drawMetaInfo(retain.fails, retain.updates, retain.attempts);
						dispUpdate(eDispBottomPart);
						// dispUpdateDoneCb will be called later

						if (config.tsApiKey[0] && indoorTempStr[0])		// ThingSpeak api key configured
						{
							setAppState(stateGetThingSpeakIp);

							espconn_disconnect(&tcpSock);
							tcpSock.type = ESPCONN_TCP;
							tcpSock.state = ESPCONN_NONE;

							// get ThingSpeak server ip
							serverIp.addr = 0;
							espconn_gethostbyname(&tcpSock, thingSpeakHost, &serverIp, getHostByNameCb);

							os_timer_setfn(&gpTmr, (os_timer_func_t *)checkDnsStatus, &tcpSock);
							os_timer_arm(&gpTmr, dnsCheckInterval, 0);
						}
						else
						{
							setAppState(stateWaitDispUpdate);
							espconn_disconnect(&tcpSock);
						}
					}
					else	// no significant change in weather or forecast
					{		// go to sleep without updating the display (saves battery)
						debug("same weather\n");
						gotoSleep(TRUE);
					}
				}
				else
				{
					debug("parseForecast failed (%d)\n", statusCode);
				}
			}
		}
	}

	if (!validReply)
	{
		if (statusCode == 404)
		{
			// city not found or service down
			debug("OWM 404\n");
			gotoSleep(FALSE);
		}
		else
		{
			if (os_strstr(httpMsgRxBuf, "HTTP/1.1 500"))
			{
				// service down
				debug("OWM 500\n");
				gotoSleep(FALSE);
			}
			else
			{
				// resend request afer one second
				os_timer_setfn(&gpTmr, (os_timer_func_t *)sendHttpRequest, NULL);
				os_timer_arm(&gpTmr, 1000, 0);
			}
		}
	}
}

typedef struct
{
	IconId id;
	uint cnt;
}IconCnt;

LOCAL void ICACHE_FLASH_ATTR incrementIconCnt(IconCnt *iconCnts, int size, IconId icon)
{
	int i;
	for (i = 0; i < size; i++)
	{
		if (iconCnts[i].id.val == icon.val)	// icon found
		{
			iconCnts[i].cnt++;
			return;
		}
		else if (iconCnts[i].id.val == 0)		// end of the list
		{
			iconCnts[i].id.val = icon.val;	// add to the list
			iconCnts[i].cnt = 1;
			return;
		}
	}
}

LOCAL IconId ICACHE_FLASH_ATTR getMostFrequentIcon(const IconCnt *iconCnts, int size)
{
	IconCnt icon = {.id.val = 0, .cnt = 0};
	int i;
	for (i = 0; i < size; i++)
	{
		// prefer daytime icons
		if (iconCnts[i].id.str[2] == 'd' && iconCnts[i].cnt > icon.cnt)
		{
			icon = iconCnts[i];
		}
	}
	if (!icon.id.val)	// use nighttime if no daytime icons found
	{
		for (i = 0; i < size; i++)
		{
			if (iconCnts[i].cnt > icon.cnt)
			{
				icon = iconCnts[i];
			}
		}
	}
	return icon.id;
}

LOCAL int ICACHE_FLASH_ATTR createDailyFromHourly(const Forecast *hourly, int hCount, Forecast *daily, int dailySize)
{
	if (dailySize < 1 || hCount < 1)
	{
		return 0;
	}
	int hour = epochToHours(hourly[0].datetime);
	int curHour;
	int midDaySet = FALSE;
	daily[0].datetime = hourly[0].datetime;
	daily[0].value.tempMin = hourly[0].value.temp;
	daily[0].value.tempMax = hourly[0].value.temp;
	IconCnt iconCnts[8];	// hourly results interval is 3h so max 8 icons per day
	os_memset(iconCnts, 0, sizeof(iconCnts));
	incrementIconCnt(iconCnts, NELEMENTS(iconCnts), hourly[0].icon);

	int i, j;
	for (i = 1, j = 0; i < hCount; i++)
	{
		curHour = hour;
		hour = epochToHours(hourly[i].datetime);
		if (hour < curHour)		// detect day change
		{
			daily[j].icon = getMostFrequentIcon(iconCnts, NELEMENTS(iconCnts));
			j++;
			if (j >= dailySize)		// unlikely
			{
				return j;
			}
			midDaySet = FALSE;
			daily[j].datetime = hourly[i].datetime;
			daily[j].value.tempMin = hourly[i].value.temp;
			daily[j].value.tempMax = hourly[i].value.temp;
			os_memset(iconCnts, 0, sizeof(iconCnts));	// reset icon counts
		}
		else
		{
			if (!midDaySet && hour >= 12)	// try to set datetime at midday if possible
			{
				midDaySet = TRUE;
				daily[j].datetime = hourly[i].datetime;
			}
			if (hourly[i].value.temp < daily[j].value.tempMin)
				daily[j].value.tempMin = hourly[i].value.temp;
			if (hourly[i].value.temp > daily[j].value.tempMax)
				daily[j].value.tempMax = hourly[i].value.temp;
		}
		incrementIconCnt(iconCnts, NELEMENTS(iconCnts), hourly[i].icon);
	}
	daily[j].icon = getMostFrequentIcon(iconCnts, NELEMENTS(iconCnts));

	return j+1;
}


void ICACHE_FLASH_ATTR dispUpdateDoneCb(int rv)
{
	debug("dispUpdateDoneCb %d\n", rv);
	dispUpdateDone = TRUE;
	if (appState == stateWaitDispUpdate)
	{
		gotoSleep(TRUE);
	}
}

LOCAL void ICACHE_FLASH_ATTR timeoutTmrCb(void)
{
	os_printf("timeoutTmrCb, appState %d\n", (int)appState);
	gotoSleep(FALSE);
}

LOCAL void ICACHE_FLASH_ATTR wifiOff(void)
{
	if (!wifiSleeping)
	{
		debug("wifiSleep\n");
		wifiSleeping = TRUE;
		wifi_station_disconnect();
		wifi_set_opmode_current(NULL_MODE);
		wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
		wifi_fpm_open();
		wifi_fpm_do_sleep(0xFFFFFFFF);
	}
}

LOCAL void ICACHE_FLASH_ATTR gotoSleep(int success)
{
	uint sleepTime;

	setAppState(stateGotoSleep);
	dispTurnOff();

	if (success)
	{
		retain.retry = 0;
		if (curTime.valid)
		{
			if (curTime.tm_hour >= 0 && curTime.tm_hour <= 3)
			{
				sleepTime = sleepTimeLong;
				retain.longSleepCnt = 2;
				if ((curTime.tm_hour+retain.longSleepCnt) > 3)	// if this is last long sleep period
				{
					// renew ip on the next wake-up
					os_memset(&retain.ipConfig, 0, sizeof(retain.ipConfig));
				}
			}
			else
			{
				sleepTime = config.interval;
			}
		}
		else
		{
			sleepTime = config.interval;
			os_memset(&retain.ipConfig, 0, sizeof(retain.ipConfig));
		}
	}
	else	// failed
	{
		retain.fails++;
		if (retain.retry < 1)	// not retried yet
		{
			retain.retry++;
			sleepTime = 60UL*1000000UL;	// retry after one minute
		}
		else	// retried once, still fails
		{
			retain.retry = 0;
			sleepTime = config.interval;	// sleep for normal time
		}
	}
	debug("sleepTime %u\n", sleepTime);
	retainWrite(&retain);
	system_deep_sleep(sleepTime);
}
