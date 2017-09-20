# ESP8266 Weather display

This is a simple weather display built with ESP8266 WiFi chip and 7.4" E ink display from Pervasive Displays. Weather data is gathered from [OpenWeatherMap](http://openweathermap.org) service. Additionally, the device’s internal temperature sensor reading is sent to [ThingSpeak](https://thingspeak.com) service.

Most of the time the device stays in deep sleep mode consuming only 18 µA. While updating the weather power consumption varies from 80 to 150 mA. Update operation takes a few seconds, depending on WiFi router, DHCP server and internet connection speed. With update interval set to 15 minutes the device has been working on a single charge for more than a year. Battery capacity is 3000 mAh.

## Building the software
### On Linux
- Install [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk)
- Clone this repository
```
$ git clone https://github.com/andrei7c4/weatherdisplay
$ cd weatherdisplay/app
```
- Set SDK paths and esptool parameters for your ESP8266 module in makefile
- Finally build everything and flash the binary
```
$ make all
$ make flash
```
### On Windows
- Install [Unofficial Development Kit for Espressif ESP8266](https://github.com/CHERTS/esp8266-devkit)
- Download ZIP or clone this repository
- In Eclipse select File->New->Makefile Project with Existing Code
- Set Existing Code Location to `<full path to>weatherdisplay\app` and click Finish
- In newly created project properties (C/C++ Build) change build command to `mingw32-make.exe -f ${ProjDirPath}/Makefile`
- In C/C++ Build/Environment add `PATH` variable with paths to MinGW and MSYS, e.g. `C:\MinGW\bin;C:\MinGW\msys\1.0\bin`
- In Make Target view right click on newly created project and select New
- Set Target name to `all` and click OK. Repeat this for `clean` and `flash` targets.
- Set SDK paths and esptool parameters for your ESP8266 module in makefile
- You should now be able to build and clean the project and flash the binaries

Custom fonts and icons can be created with [this tool](https://github.com/andrei7c4/fontconverter).

## Building the hardware
Any ESP8266 based module with at least 1 MB flash and SPI pins, such as ESP-12E, can be used. Development boards, such as [NodeMCU-DEVKIT](https://github.com/nodemcu/nodemcu-devkit-v1.0) or similar, can be used too, but power consumption of these boards might not be as low.

The [display with control board](http://www.pervasivedisplays.com/kits/mpicosys740) can be purchased from [DigiKey](http://www.digikey.com/product-detail/en/SW074AS182/SW074AS182-ND/4898789).
Display is connected to ESP8266 in the following way:

| TCM-P74-230  | ESP8266        |
| ------------ | -------------- |
| /EN          | GPIO5          |
| /BUSY        | GPIO4          |
| MISO         | HMISO (GPIO12) |
| MOSI         | HMOSI (GPIO13) |
| /CS          | HCS (GPIO15)   |
| SCK          | HSCLK (GPIO14) |

Additionally, there are 3V LDO regulator, programming circuit and USB to UART bridge (CP2102 or similar can be used). Please see [the schematics here](schematics.pdf).

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

If this key is not set, internal temperature will only be shown on the display but not sent anywhere. 

The following user interface options are available:
 - chart:0 - [No forecast charts (icons only)](gui/chart0.png)
 - chart:1 - [Forecast chart for the next 24 hours](gui/chart1.png)
 - chart:2 - [Forecast chart for the next five days](gui/chart2.png)
 - chart:3 - [Both forecast charts](gui/chart3.png)

Please see the [config.c](app/src/config.c) file for additional supported parameters.

***
[![](http://img.youtube.com/vi/9eWtP8rnsAE/sddefault.jpg)](https://youtu.be/9eWtP8rnsAE)
