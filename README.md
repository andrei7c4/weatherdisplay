# ESP8266 Weather display

This is a simple weather display built with ESP8266 WiFi chip and 7.4" E ink display from Pervasive Displays. Weather data is gathered from [OpenWeatherMap](http://openweathermap.org) service. Additionally, the device’s internal temperature sensor reading is sent to [data.sparkfun.com](https://data.sparkfun.com) service.

Most of the time the device stays in deep sleep mode consuming only 18 µA. While updating the weather power consumption varies from 80 to 150 mA. Update operation takes a few seconds, depending on WiFi router, DHCP server and internet connection speed. With 30 minute update interval 3000 mAh battery should last for a few months on one charge.

## How to build
### Software
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
Custom fonts and icons can be created with [this tool](https://github.com/andrei7c4/fontconverter).

### Hardware
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

## How to use
Device settings can be changed through the serial interface (115200,N,8). The following syntax should be used:
```
parameter:value<CR>
```
At least the following parameters must be set by the user:
 - ssid - WiFi SSID
 - pass - WiFi password
 - city - Weather and forecast are shown for that city ([check supported city names](http://openweathermap.org/find))
 - appid - [OpenWeatherMap API key](http://openweathermap.org/appid) 

Additionally, [data.sparkfun.com](https://data.sparkfun.com) service keys can be set:
 - publickey
 - privatekey

If these keys are not set, internal temperature will only be shown on the display but not sent anywhere. Please see the [config.c](app/src/config.c) file for additional supported parameters.

***
[![](http://img.youtube.com/vi/pryzzH_i0os/0.jpg)](http://www.youtube.com/watch?v=pryzzH_i0os)
