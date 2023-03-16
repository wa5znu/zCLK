// dCLC.ino - alternative firmware for 303WIFILC01: NTP clock with DS1302
// board: Generic ESP8266 module
#define VERSION "1.2-WA5ZNU-1"

#include <pgmspace.h>

#include <coredecls.h> // Only needed for settimeofday_cb()
#include <core_version.h> // ARDUINO_ESP8266_RELEASE
#include <sys/time.h> 
#include <time.h>
#include <TZ.h>
#include "led.h"
#include "but.h"
#include "disp.h"
#include "rtc.h"
#include "wifi.h"

// mycfg.h is secret: copy from cfg.h.sample and edit
// #include "mycfg.h"
#include "mycfg.h"
#include "CREDITS.h"
Cfg cfg;

volatile bool ntp_sync = false; // flag NTP synchronized
volatile int from_sntp = -2; // arg received by callback function

// for testing set short NTP delay
extern "C" {
  uint32_t sntp_update_delay_MS_rfc_not_less_than_15000 () {
    // return 15 * 1000UL; // 15s
    return 1 * 60 * 60 * 1000UL; // 1h
  }
}

void setup() {
  Serial.begin(115200);
  Serial.printf("zCLK: Welcome to WA5ZNU zCLK %s\n",VERSION);
  Serial.printf("%s\n\n", CREDITS_STRING);  

  Serial.printf("main: Arduino ESP32 " ARDUINO_ESP8266_RELEASE "\n" );
  Serial.printf("main: compiler " __VERSION__ "\n" );
  Serial.printf("main: arduino %d\n",ARDUINO );
  Serial.printf("main: compiled " __DATE__ ", " __TIME__ "\n" );

  // Identify ourselves, regardless if we go into config mode or the real app
  disp_init();
  disp_power_set(1);
  disp_brightness_set(1);

  disp_show("zclc");
  delay(2000);
  

  // Initialize RTC
  rtc_init();

  // LED on
  led_init();
  led_on(); 
  but_init();

  // if RTC have valid time use it

  if( rtc_check() ) {
    struct tm utc;

    // do this all in UTC
    rtc_get(&utc);
    time_t t = mktime(&utc);	// ergardless of setTZ, rtc is UTC
    timeval tv = { t, 0 };
    settimeofday(&tv, nullptr);

    // now switch to local time
    Serial.printf("rtc: cfg.timezone=%s\n", cfg.timezone);
    setTZ(cfg.timezone);
    tzset();

    struct tm t_local;
    localtime_r(&t, &t_local);

    char bnow[5];
    sprintf(bnow,"%02d%02d", t_local.tm_hour, t_local.tm_min );
    disp_show(bnow,DISP_DOTNO);

    Serial.printf("rtc: utc=%s\n", asctime(&utc));
    Serial.printf("rtc: cfg.timezone=%s t_local=%s t_local.tm_isdst=%d\n", cfg.timezone, asctime(&t_local), t_local.tm_isdst );
  } else {
    Serial.printf("rtc: no rtc; cfg.timezone=%s\n", cfg.timezone);
    setTZ(cfg.timezone);
    tzset();
    disp_show("-ntP");
  }

  // WiFi and NTP
  wifi_init(AP_COUNT, cfg.ssids, cfg.passwords);
  Serial.printf("clk : timezone: %s; NTP %s %s %s\n", cfg.timezone, cfg.ntp_server_1, cfg.ntp_server_2, cfg.ntp_server_3);
  configTime(cfg.timezone, cfg.ntp_server_1, cfg.ntp_server_2, cfg.ntp_server_3);
  settimeofday_cb( [](bool f){from_sntp = f; ntp_sync = true; } );  // Pass lambda function to print SET when time is set

  // App starts running
  Serial.printf("dCLC\n");
}


// Record last received seconds, print
int       tm_sec_prev = -1;

// Showing time or date
int       show_date; 

void loop() {
  struct tm snow;
  struct tm gnow;

  // if in config mode, do config loop (when config completes, it restarts the device)
  // In normal application mode
  led_set( !wifi_isconnected() );     // LED is on when not connected
  but_scan();

  if( but_wentdown(BUT3) ) disp_brightness_set( disp_brightness_get()%8 + 1 );
  if( but_wentdown(BUT2) ) show_date = !show_date;

  // Returns seconds (and writes to the passed pointer, when not NULL) - note `time_t` is just a `long`.
  time_t tnow = time(NULL);

  // Returns a struct with time fields (https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm)
  localtime_r(&tnow, &snow); 

  // UTC version, for RTC
  gmtime_r(&tnow, &gnow);    

  bool sync = snow.tm_year>120;// We miss-use "old" time as indication of "time not yet set" (year is 1900 based)

  // When NTP synchronized - update RTC DS1302
  if( ntp_sync ) {
    ntp_sync = false;
    Serial.printf("ntp : nyp_sync from_sntp = %d\n", from_sntp);
    if (!sync) {
      Serial.printf("ERROR: ntp sync with bad data\n");
    } else {
      rtc_set(&gnow);
      disp_show("ntP");
      Serial.printf("ntp : ntp = %s", asctime(&gnow));
      Serial.printf("ntp : loc = %s", asctime(&snow));
      delay(1000);
    }
  }

  //   struct tm localtime_r(const time_t* timer, struct tm* timeptr)
  //   struct tm tm_out;
  // localtime_r(&ts, &tm_out);
  // problem how to convert UTC to a timezone other than current timezone
  

  // If seconds changed:
  if( snow.tm_sec != tm_sec_prev ) {
    // once a minute:  print
    if (snow.tm_sec == 0) {
      // In `snow` the `tm_year` field is 1900 based, `tm_month` is 0 based, rest is as expected
      Serial.printf("time=%s dst=%d sync=%s\n", asctime(&snow), snow.tm_isdst, sync?"NTP":"RTC" );
    }
    tm_sec_prev = snow.tm_sec; 
  }

  if( sync && show_date ) {
    char bnow[5];
    sprintf(bnow,"%2d%02d", 1+snow.tm_mon, snow.tm_mday );
    disp_show(bnow);
  }
  
  if( sync && !show_date ) {
    char bnow[5];
    bool pm = snow.tm_hour >= 12;
    int  hr = snow.tm_hour % cfg.hours;
    int sec = snow.tm_sec;
    sprintf(bnow,"%02d%02d", hr, snow.tm_min );
    int dots = (sec % 2 == 0) ? DISP_DOTCOLON : DISP_DOTNO;
    if( cfg.ampm_dot==DOT_FLAG_AM && !pm ) dots |= DISP_DOT4;
    if( cfg.ampm_dot==DOT_FLAG_PM &&  pm ) dots |= DISP_DOT4;
    disp_show(bnow,dots);
  }
}
