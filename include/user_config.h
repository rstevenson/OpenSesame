#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))
#define ICACHE_RODATA_ATTR __attribute__((section(".irom.text")))

#define CFG_HOLDER	0x00FF55A2
#define CFG_LOCATION	0x3C

#define NTP_ENABLE    1
#define NTP_TZ  	  2

#define HTTPD_PORT      80
#define HTTPD_AUTH      0
#define HTTPD_USER      "admin"
#define HTTPD_PASS      "pass"

#define AP_IP        "192.168.4.1"
#define AP_MASK      "255.255.255.0"
#define AP_GW        "192.168.4.1"

#define STA_MODE     "static"
#define STA_IP       "192.168.254.17"
#define STA_MASK     "255.255.255.0"
#define STA_GW       "192.168.254.254"
#define STA_SSID     "BUFFALO-289E04_G"
#define STA_PASS     "WAPpassword365"
#define STA_TYPE AUTH_WPA2_PSK

#endif
