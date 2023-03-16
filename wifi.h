// wifi.h - interface to control the wifi
#ifndef _WIFI_H_
#define _WIFI_H_


void wifi_init(int n, const char* ssids[], const char* passwords[]); // Initializes the WiFi driver
bool wifi_isconnected(); // Prints WiFi status to the user (over Serial, only when changed), and returns true iff connected


#endif
