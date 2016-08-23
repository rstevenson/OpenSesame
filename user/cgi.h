#ifndef CGI_H
#define CGI_H

#include "httpd.h"

int cgiLed(HttpdConnData *connData);
int tplLed(HttpdConnData *connData, char *token, void **arg);
int tplCounter(HttpdConnData *connData, char *token, void **arg);
void tplNTP(HttpdConnData *connData, char *token, void **arg);
int cgiNTP(HttpdConnData *connData);
int cgiSchedule(HttpdConnData *connData);
void tplSchedule(HttpdConnData *connData, char *token, void **arg);

#endif
