// wifi.cpp - control the wifi

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "wifi.h"


ESP8266WiFiMulti wifi;


// Sets host name, based on MAC address
static void wifi_sethostname(int len=WL_MAC_ADDR_LENGTH) {
  const char prefix[]= "nCLC-";
  char hname[sizeof(prefix)+2*WL_MAC_ADDR_LENGTH]; // sizeof(prefix) includes terminating 0
  char * p= (char*)&hname;
  for( const char * q=prefix; *q!=0; ) *p++= *q++;
  uint8_t macbuf[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(macbuf);
  for(int i=WL_MAC_ADDR_LENGTH-len; i<WL_MAC_ADDR_LENGTH; i++ ) {
    uint8 d1=(macbuf[i]>>4)&0x0f;
    uint8 d0=(macbuf[i]>>0)&0x0f;
    *p++= d1>=10 ? d1+'A'-10 : d1+'0';
    *p++= d0>=10 ? d0+'A'-10 : d0+'0';
  }
  *p= '\0';
  WiFi.hostname(hname);
  Serial.printf("wifi: host: %s\n", hname);
}


// Initializes the WiFi driver.
// Sets up WiFi for the three SSIDs the user configured.
void wifi_init(int n, const char* ssids[], const char * passwords[]) {
  wifi_sethostname(3);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  Serial.printf("wifi: APs:");
  for (int i = 0; i < n; i++) {
    wifi.addAP(ssids[i], passwords[i]);
    Serial.printf(" %s",ssids[i]);
  }
  Serial.printf("\n");
  wifi_isconnected();
}


// Prints WiFi status to the user (over Serial, only when changed), and returns true iff connected
bool wifi_isconnected() {
  static bool wifi_on= false;
  if( wifi.run()==WL_CONNECTED ) { // Unfortunately, this is a blocking call
    if( !wifi_on ) Serial.printf("wifi: connected to %s, IP address %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str() );
    wifi_on= true;
  } else {
    if( wifi_on ) Serial.printf("wifi: disconnected\n" );
    wifi_on= false;    
  }
  return wifi_on;
}
