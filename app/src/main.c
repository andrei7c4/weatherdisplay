#include <os_type.h>
#include <osapi.h>
#include <ip_addr.h>
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
Forecast forecast[FORECAST_DAYS];
char indoorTemp[8] = "";
struct tm *curTime = NULL;

int dispUpdateDone = FALSE;
const uint sleepTimeLong = 60UL*60UL*1000000UL;

typedef enum{
	stateInit,
	stateConnectToAp,
	stateGetOwmIp,
	stateConnectToOwm,
	stateGetWeather,
	stateGetForecast,
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

	configRead(&config);
	debug("attempts %u, fails %u, retry %u\n", config.attempts, config.fails, config.retry);

	if (config.longSleepCnt > 0)
	{
		config.longSleepCnt--;
		configWrite(&config);
		setAppState(stateGotoSleep);
		system_deep_sleep(sleepTimeLong);
	}

	dispSpiInit();

	setAppState(stateConnectToAp);
	wifi_set_opmode(STATION_MODE);
	connectToWiFiAP();
}

LOCAL void ICACHE_FLASH_ATTR connectToWiFiAP(void)
{
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

	os_timer_disarm(&gpTmr);

	// check current connection status and own ip address
	wifi_get_ip_info(STATION_IF, &ipconfig);
	uint8 connStatus = wifi_station_get_connect_status();
	if (connStatus == STATION_GOT_IP && ipconfig.ip.addr != 0)
	{
		// connection with AP established -> get openweathermap server ip
		setAppState(stateGetOwmIp);

		tcpSock.proto.tcp = &tcpSockParams;
		tcpSock.type = ESPCONN_TCP;
		tcpSock.state = ESPCONN_NONE;
		serverIp.addr = 0;
		espconn_gethostbyname(&tcpSock, openWeatherMapHost, &serverIp, getHostByNameCb);

		os_timer_setfn(&gpTmr, (os_timer_func_t *)checkDnsStatus, &tcpSock);
		os_timer_arm(&gpTmr, 1000, 0);
	}
	else
	{
		if (connStatus == STATION_WRONG_PASSWORD ||
			connStatus == STATION_NO_AP_FOUND	 ||
			connStatus == STATION_CONNECT_FAIL)
		{
			debug("Failed to connect to AP (status: %u)", connStatus);
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
    if (stateGetOwmIp)
    {
        espconn_gethostbyname(pespconn, openWeatherMapHost, &serverIp, getHostByNameCb);
    }
    else
    {
        espconn_gethostbyname(pespconn, sparkfunHost, &serverIp, getHostByNameCb);
    }
    os_timer_arm(&gpTmr, 1000, 0);
}

LOCAL void ICACHE_FLASH_ATTR getHostByNameCb(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

	if (ipaddr == NULL || ipaddr->addr == 0)
	{
		debug("getHostByNameCb ip NULL\n");
		return;
	}

	if (serverIp.addr != 0)
	{
		debug("getHostByNameCb owmServerIp != 0\n");
		return;
	}

	setAppState((appState == stateGetOwmIp) ? stateConnectToOwm : stateConnectToSf);

	// connect to openweathermap server
	os_timer_disarm(&gpTmr);
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
			config.cityId[0] ? "id" : "q",
			config.cityId[0] ? config.cityId : config.city,
			config.appid, openWeatherMapHost);
		break;
	case stateGetForecast:
		if (!config.appid[0] || !config.city[0])
			return;
		os_sprintf(httpMsgTxBuf,
			"GET /data/2.5/forecast/daily?%s=%s&mode=json&cnt=5&lang=en&APPID=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			config.cityId[0] ? "id" : "q",
			config.cityId[0] ? config.cityId : config.city,
			config.appid, openWeatherMapHost);
		break;
	case stateSendTempToSf:
		if (!config.publickey[0] || !config.privatekey[0])
			return;
		os_sprintf(httpMsgTxBuf,
			"GET /input/%s?private_key=%s&temp=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			config.publickey, config.privatekey, indoorTemp, sparkfunHost);
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
    //os_printf("%s\n", pusrdata);

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
}

LOCAL void ICACHE_FLASH_ATTR onTcpConnFailed(void *arg, sint8 err)
{
	if (appState != stateGotoSleep)
	{
		os_printf("onTcpConnFailed, appState %d\n", (int)appState);
		gotoSleep(FALSE);
	}
}

LOCAL void ICACHE_FLASH_ATTR parseHttpReply(void)
{
	debug("processHttpMessage %d\n", httpMsgCurLen);
	httpMsgRxBuf[httpMsgCurLen] = 0;

	//os_printf("%s\n", httpMsgRxBuf);
	if (appState != stateGetWeather && appState != stateGetForecast)
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

					drawCurrentWeather(&curWeather);
					dispUpdate(eDispTopPart);

					// send forecast request
					setAppState(stateGetForecast);
					sendHttpRequest();
				}
				else
				{
					debug("parseWeather failed\n");
				}
			}
			else if (appState == stateGetForecast)
			{
				if (parseForecast(json, forecast) == OK)
				{
					msgValid = TRUE;
					os_timer_disarm(&gpTmr);

					drawForecast(forecast);

					dispReadTemp(indoorTemp);
					drawIndoorTemp(indoorTemp);

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
						os_timer_arm(&gpTmr, 1000, 0);
					}
					else
					{
						setAppState(stateWaitDispUpdate);
					}
				}
				else
				{
					debug("parseForecast failed\n");
				}
			}
		}
	}

	if (!msgValid)
	{
		if (json && !os_strncmp(json, "{\"cod\":\"404\"", 12))
		{
			// city not found or service down
			debug("404\n");
			gotoSleep(FALSE);
		}
		else	// if no reply with valid json in one second, resend request
		{
			os_timer_setfn(&gpTmr, (os_timer_func_t *)sendHttpRequest, NULL);
			os_timer_arm(&gpTmr, 1000, 0);
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
	config.attempts++;

	if (success)
	{
		config.retry = FALSE;
		if (curTime && (curTime->tm_hour >= 22 || curTime->tm_hour <= 2))
		{
			sleepTime = sleepTimeLong;
			config.longSleepCnt = 3;
		}
		else
		{
			sleepTime = config.interval;
		}
	}
	else	// failed
	{
		config.fails++;
		if (!config.retry)	// not retried yet
		{
			config.retry = TRUE;
			sleepTime = 60UL*1000000UL;	// retry after one minute
		}
		else	// retried once, still fails
		{
			config.retry = FALSE;
			sleepTime = config.interval;	// sleep for normal time
		}
	}
	debug("sleepTime %u\n", sleepTime);
	configWrite(&config);
	system_deep_sleep(sleepTime);
}

