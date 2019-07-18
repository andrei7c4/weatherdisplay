# ESP8266 Weather display

This is a simple weather display built with ESP8266 WiFi chip and E ink display. Weather data is gathered from [OpenWeatherMap](http://openweathermap.org) service. Additionally, the device’s internal temperature sensor reading is sent to [ThingSpeak](https://thingspeak.com) service.

Most of the time the device stays in deep sleep mode, consuming only 18 µA. Update operation takes a few seconds, depending on WiFi router, DHCP server and internet connection speed. With 3000 mAh battery and update interval of 15 minutes, the device will last on a single charge for one and a half years.

## Building the software
- Install [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk)
- Clone this repository
- Build and flash the binary
```
$ git clone https://github.com/andrei7c4/weatherdisplay
$ cd weatherdisplay/app
$ make all ESP_OPEN_SDK_PATH=/full/path/to/esp-open-sdk
$ make flash ESP_OPEN_SDK_PATH=/full/path/to/esp-open-sdk ESPPORT=/serial/port/devpath
```

Originally the project was designed with [this display](http://www.pervasivedisplays.com/kits/mpicosys740), which is now obsolete. In addition to the old display, software currently supports [this display from Waveshare](https://www.waveshare.com/7.5inch-e-paper-hat.htm). Software is built for this display by default. To build software for the old display, add `CFLAGS=-DDISP_MODEL=PERVASIVE` to the end of `make all` command.

## Building the hardware
Any ESP8266 based module with at least 1 MB flash and SPI pins, such as ESP-12E, can be used. Development boards, such as [NodeMCU-DEVKIT](https://github.com/nodemcu/nodemcu-devkit-v1.0) or similar, can be used too, but power consumption of these boards might not be as low.

Displays are connected to ESP8266 in the following way:

| ESP8266        | Waveshare    | Pervasive Displays (obsolete) |
| -------------- | ------------ | ----------------------------- |
| GPIO5          |              | /EN                           |
| GPIO4          | RST          | /BUSY                         |
| HMISO / GPIO12 | /BUSY        | MISO                          |
| HMOSI / GPIO13 | DIN          | MOSI                          |
| HCS / GPIO15   | /CS          | /CS                           |
| HSCLK / GPIO14 | CLK          | SCK                           |

### Notes about Waveshare dispay
#### DIP switch configuration

| DIP switch       | Position       |
| ---------------- | -------------- |
| Display Config   | B (Other)      |
| Interface Config | 1 (3-line SPI) |

#### Power consumption
Waveshare display lacks dedicated enable pin. However, it can be put into sleep mode by executing a command. Ideally, in this mode the display consumes 8 µA. Unfortunately, it has been noticed that the current doesn't always stay at this level. After some time spent in the sleep mode, the current starts to grow and can reach up to 100 µA. The issue was discussed with Waveshare, but they were not able to provide a solution. As a workaround, a high side switch can be used on display's 3V3 line to completely turn off the display. Software uses GPIO5 to control the switch. Software will still execute the sleep command, so the switch can be left out, but power consumption will be higher in this case.

Please see [the schematics here](schematics.pdf).

## Usage
Device settings can be changed through the serial interface (115200/8-N-1). The following syntax should be used:
```
parameter:value<CR>
```
At least the following parameters must be set by the user:
 - ssid - WiFi SSID
 - pass - WiFi password
 - city - Weather and forecast are shown for that city ([check supported city names](http://openweathermap.org/find))
 - appid - [OpenWeatherMap API key](http://openweathermap.org/appid) 

Additionally, [ThingSpeak](https://thingspeak.com) channel Write API Key can be set:
 - thingspeak

If this key is not set, internal temperature will not be sent to ThingSpeak (it will still be indicated on the display though).

The following user interface options are available:
 - chart:0 - [No forecast charts (icons only)](gui/chart0.png)
 - chart:1 - [Forecast chart for the next 24 hours](gui/chart1.png)
 - chart:2 - [Forecast chart for the next five days](gui/chart2.png)
 - chart:3 - [Both forecast charts](gui/chart3.png)

Please see the [config.c](app/src/config.c) file for additional supported parameters.

***
[![](http://img.youtube.com/vi/9eWtP8rnsAE/sddefault.jpg)](https://youtu.be/9eWtP8rnsAE)
