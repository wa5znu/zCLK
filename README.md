# WA5ZNU Version of GitHub://@maarten-pennings/303WIFILC01/nCLC

[Maarten Pennings](https://github.com/maarten-pennings) documents how
he reverse-engineered the existing clock and then built an SDK for
Arduino/ESP8266 toolchain and used that to develop several new clock sketches.

I have taken created zCLK by taking Maarten's dCLC, which uses both
WiFi-based NTP and DS1302-based RTC, and hard-coded the configuration.
I've also made a few changes to time and date handling, DST crossover,
RTC before NTP start time during DST, etc.

## More detailed Changes

- Moved TZ settings around to ensure that RTC is UTC, so gmtime and localtime work
- Tested DST start, restart after DST change, etc.  More test cases loom.
- Removed web cfg and replaced with compile time constants in a .gitignore'd file
- Moved Serial.printf() after time-critical sections
- Adapted the original README to produce the rest of this README

# zCLK

An NTp-disciplined Quartz Real-Time Clock for the ESP8266 and DS1302 based on
- [Maarten Pennings](https://github.com/maarten-pennings) 
- [Utyf](https://github.com/Utyff)
- [DS103 driver](https://github.com/Erriez/ErriezDS1302) from [Erriez](https://github.com/Erriez)
- [NTP Tips and Tricks](https://www.weigu.lu/microcontroller/tips_tricks/esp_NTP_tips_tricks/index.html)
- [Arduino ESP8266 NTP](https://werner.rothschopf.net/202011_arduino_esp8266_ntp_en.htm)


## Configuration

```
 cp mycfg.h.sample mycfg.h && ed mycfg.h
 grep mycfg.h .gitignore
```

Then recompile and install. For timezone, copy the text of one from `hardware/esp8266/3.0.2/cores/esp8266/TZ.h`.
If you'd rather manage configuration with a captive-AP Web UI, you would be better served with the original nCLK).

## Libraries

This sketch relies on the DS103 driver:
 - [DS103 driver](https://github.com/Erriez/ErriezDS1302) from [Erriez](https://github.com/Erriez).

## Buttons
 - Button SET is unused 
 - During normal operation, pressing DOWN steps the display brightness.
 - During normal operation, pressing UP toggles time and date.
 
## Display
- Define 12h/24h and AM/PM in mycfg.h
- Define NTP discipline fetch frequency in mycfg.h, currently hourly
- On first boot, it will first show '-nTP' until it gets NTP time.
- On subsequent boots, it will first show RTC time, then display 'nTP' until it gets NTP time.
- Getting NTP time should be quick; if not, it's probably miconfigured 
- When it fetches NTP time hourly it displays 'nTP'

## Installation 
- See Maarten Pennings original GitHub for latest updates and matching hardware.  
- For programming, I just used jumper wires and a $2.00 USB-TTL device.
- I found I had to use erase-esp8266-flash.sh the first time, and again when I changed WiFi.
