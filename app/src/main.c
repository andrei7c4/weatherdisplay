#include <os_type.h>
#include <osapi.h>
#include <ip_addr.h>
#include <lwip/err.h>
#include <lwip/dns.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>

#include "common.h"
#include "debug.h"
#include "parsejson.h"
#include "config.h"
#include "conv.h"
#include "dispspi.h"
#include "display.h"
#include "gui.h"
#include "datetime.h"
#include "drivers/uart.h"



const char openWeatherMapHost[] = "api.openweathermap.org";
const char sparkfunHost[] = "data.sparkfun.com";
const char sfFieldName[] = "temp";

#define HTTP_MSG_MAX_LEN	4096
char httpMsgRxBuf[HTTP_MSG_MAX_LEN];
char httpMsgTxBuf[200];
int httpMsgCurLen = 0;

LOCAL os_timer_t gpTmr;
LOCAL struct espconn tcpSock;
LOCAL struct _esp_tcp tcpSockParams;
ip_addr_t serverIp;
LOCAL os_timer_t timeoutTmr;

CurWeather curWeather;
Forecast forecastDaily[FORECAST_DAILY_CNT];
Forecast forecastHourly[FORECAST_HOURLY_CNT];
char indoorTemp[8] = "";
struct tm *curTime = NULL;

int dispUpdateDone = FALSE;
const uint sleepTimeLong = 60UL*60UL*1000000UL;

const uint dnsCheckInterval = 100;

typedef enum{
	stateInit,
	stateConnectToAp,
	stateGetOwmIp,
	stateConnectToOwm,
	stateGetWeather,
	stateGetHourlyForecast,
	stateGetDailyForecast,
	stateGetSparkfunIp,
	stateConnectToSf,
	stateSendTempToSf,
	stateWaitDispUpdate,
	stateGotoSleep
}AppState;
AppState appState = stateInit;

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
LOCAL void parseHttpReply(void);
LOCAL void gotoSleep(int success);


void user_init(void)
{
	os_timer_disarm(&gpTmr);
	os_timer_disarm(&timeoutTmr);	// timeout timer, normally should never fire
	os_timer_setfn(&timeoutTmr, (os_timer_func_t *)timeoutTmrCb, NULL);

	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	
	retainRead(&retain);
	if (retain.longSleepCnt > 0)
	{
		retain.longSleepCnt--;
		retainWrite(&retain);
		setAppState(stateGotoSleep);
		system_deep_sleep(sleepTimeLong);
		return;
	}
	configRead(&config);
	
	debug("Built on %s %s\n", __DATE__, __TIME__);
	debug("SDK version %s\n", system_get_sdk_version());
	debug("attempts %u, fails %u, retry %u\n", retain.attempts, retain.fails, retain.retry);

	dispSpiInit();

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
    else if (appState == stateGetSparkfunIp)
    {
		espconn_gethostbyname(pespconn, sparkfunHost, &serverIp, getHostByNameCb);
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
	
	setAppState((appState == stateGetOwmIp) ? stateConnectToOwm : stateConnectToSf);

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
	else if (appState == stateConnectToSf)
	{
		// send indoor temp to sparkfun
		setAppState(stateSendTempToSf);
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
	case stateGetHourlyForecast:
		if (!config.appid[0] || !config.city[0])
			return;
		os_sprintf(httpMsgTxBuf,
			"GET /data/2.5/forecast?%s=%s&mode=json&cnt=%d&lang=en&APPID=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			retain.cityId[0] ? "id" : "q",
			retain.cityId[0] ? retain.cityId : config.city,
			FORECAST_HOURLY_CNT,
			config.appid, openWeatherMapHost);
		break;
	case stateGetDailyForecast:
		if (!config.appid[0] || !config.city[0])
			return;
		os_sprintf(httpMsgTxBuf,
			"GET /data/2.5/forecast/daily?%s=%s&mode=json&cnt=%d&lang=en&APPID=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			retain.cityId[0] ? "id" : "q",
			retain.cityId[0] ? retain.cityId : config.city,
			FORECAST_DAILY_CNT,
			config.appid, openWeatherMapHost);
		break;
	case stateSendTempToSf:
		if (!config.publickey[0] || !config.privatekey[0])
			return;
		os_sprintf(httpMsgTxBuf,
			"GET /input/%s?private_key=%s&%s=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			config.publickey, config.privatekey, sfFieldName, indoorTemp, sparkfunHost);
		break;
	default: return;
	}

	httpMsgCurLen = 0;
	espconn_sent(&tcpSock, (uint8*)httpMsgTxBuf, os_strlen(httpMsgTxBuf));
}

LOCAL void ICACHE_FLASH_ATTR onTcpDataSent(void *arg)
{
	debug("onTcpDataSent\n");
	if (appState == stateSendTempToSf)
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
	os_timer_disarm(&gpTmr);
	debug("onTcpDataReceived %d\n", length);

	if ((HTTP_MSG_MAX_LEN-httpMsgCurLen) > length)
	{
		os_memcpy(httpMsgRxBuf+httpMsgCurLen, pusrdata, length);
		httpMsgCurLen += length;
		parseHttpReply();
	}
	else
	{
		debug("http message too long\n");
	}
}

LOCAL void ICACHE_FLASH_ATTR onTcpDisconnected(void *arg)
{
	debug("onTcpDisconnected\n");
	if (appState == stateGetWeather ||
		appState == stateGetHourlyForecast ||
		appState == stateGetDailyForecast)
	{
		debug("reconnect to owm\n");
		setAppState(stateConnectToOwm);
		espconn_connect(&tcpSock);
	}
	else if (appState == stateSendTempToSf)
	{
		debug("reconnect to sf\n");
		setAppState(stateConnectToSf);
		espconn_connect(&tcpSock);
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

LOCAL void ICACHE_FLASH_ATTR parseHttpReply(void)
{
	debug("parseHttpReply %d\n", httpMsgCurLen);
	httpMsgRxBuf[httpMsgCurLen] = 0;

	//os_printf("%s\n", httpMsgRxBuf);
	if (appState != stateGetWeather &&
		appState != stateGetHourlyForecast &&
		appState != stateGetDailyForecast)
	{
		return;
	}

	int msgValid = FALSE;
	char *body = (char*)os_strstr(httpMsgRxBuf, "\r\n\r\n");
	char *json = NULL;
	if (body)
	{
		json = (char*)os_strstr(body, "{");
		if (json)
		{
			if (appState == stateGetWeather)
			{
				if (parseWeather(json, &curWeather) == OK)
				{
					msgValid = TRUE;
					os_timer_disarm(&gpTmr);

					if (config.chart & eForecastHourly)
					{
						drawCurrentWeatherSmall(&curWeather);
						setAppState(stateGetHourlyForecast);
					}
					else
					{
						drawCurrentWeather(&curWeather);
						dispUpdate(eDispTopPart);
						setAppState(stateGetDailyForecast);
					}
					sendHttpRequest();
				}
				else
				{
					debug("parseWeather failed\n");
				}
			}
			else if (appState == stateGetHourlyForecast)
			{
				if (parseForecast(json, forecastHourly, eForecastHourly) == OK)
				{
					msgValid = TRUE;
					os_timer_disarm(&gpTmr);

					drawHourlyForecastChart(forecastHourly);
					dispUpdate(eDispTopPart);

					// send daily forecast request
					setAppState(stateGetDailyForecast);
					sendHttpRequest();
				}
				else
				{
					debug("parse forecastHourly failed\n");
				}
			}
			else if (appState == stateGetDailyForecast)
			{
				if (parseForecast(json, forecastDaily, eForecastDaily) == OK)
				{
					msgValid = TRUE;
					os_timer_disarm(&gpTmr);

					dispReadTemp(indoorTemp);
					if (config.chart & eForecastDaily)
					{
						drawDailyForecastChart(forecastDaily);
						drawIndoorTempSmall(indoorTemp);
					}
					else
					{
						drawForecast(forecastDaily);
						drawIndoorTemp(indoorTemp);
					}

					curTime = (struct tm*)os_zalloc(sizeof(struct tm));
					if (epochToTm(curWeather.datetime + config.utcoffset, curTime) != OK)
					{
						os_free(curTime);
						curTime = NULL;
					}
					drawMetaInfo(curTime);
					dispUpdate(eDispBottomPart);
					// dispUpdateDoneCb will be called later

					if (config.publickey[0] && config.privatekey[0])
					{
						setAppState(stateGetSparkfunIp);

						espconn_disconnect(&tcpSock);
						tcpSock.type = ESPCONN_TCP;
						tcpSock.state = ESPCONN_NONE;

						// get sparkfun server ip
						serverIp.addr = 0;
						espconn_gethostbyname(&tcpSock, sparkfunHost, &serverIp, getHostByNameCb);

						os_timer_setfn(&gpTmr, (os_timer_func_t *)checkDnsStatus, &tcpSock);
						os_timer_arm(&gpTmr, dnsCheckInterval, 0);
					}
					else
					{
						setAppState(stateWaitDispUpdate);
						//gotoSleep(TRUE);
					}
				}
				else
				{
					debug("parse forecastDaily failed\n");
				}
			}
		}
	}

	if (!msgValid)
	{
		if (json && !os_strncmp(json, "{\"cod\":\"404\"", 12))
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

void ICACHE_FLASH_ATTR dispUpdateDoneCb(uint16 rv)
{
	debug("dispUpdateDoneCb %x\n", rv);
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

LOCAL void ICACHE_FLASH_ATTR gotoSleep(int success)
{
	uint sleepTime;

	setAppState(stateGotoSleep);
	dispOff();
	retain.attempts++;

	if (success)
	{
		retain.retry = 0;
		if (curTime)
		{
			if (curTime->tm_hour >= 0 && curTime->tm_hour <= 3)
			{
				sleepTime = sleepTimeLong;
				retain.longSleepCnt = 2;
				if ((curTime->tm_hour+retain.longSleepCnt) > 3)	// if last long sleep
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

