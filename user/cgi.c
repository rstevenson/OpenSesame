/*
Some random cgi routines. Used in the LED example and the page that returns the entire
flash as a binary. Also handles the hit counter on the main page.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <esp8266.h>
#include "cgi.h"
#include "io.h"
#include "jsmn.h"
#include "sntp.h"
#include "time_utils.h"
#include "config.h"

//cause I can't be bothered to write an ioGetLed()
static char currLedState=1;

//Cgi that turns the LED on or off according to the 'led' param in the POST data
int ICACHE_FLASH_ATTR cgiLed(HttpdConnData *connData) {
	int len;
	char buff[1024];
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	len=httpdFindArg(connData->post->buff, "led", buff, sizeof(buff));
	if (len!=0) {
		currLedState=atoi(buff);
		ioLed(!currLedState);
	}

	httpdRedirect(connData, "led.tpl");
	return HTTPD_CGI_DONE;
}



//Template code for the led page.
int ICACHE_FLASH_ATTR tplLed(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return HTTPD_CGI_DONE;

	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "ledstate")==0) {
		if (currLedState) {
			os_strcpy(buff, "on");
		} else {
			os_strcpy(buff, "off");
		}
	}
	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}

static long hitCounter=0;

//Template code for the counter on the index page.
int ICACHE_FLASH_ATTR tplCounter(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return HTTPD_CGI_DONE;

	if (os_strcmp(token, "counter")==0) {
		hitCounter++;
		os_sprintf(buff, "%ld", hitCounter);
	}
	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplNTP(HttpdConnData *connData, char *token, void **arg) {

	char buff[128];
	if (token==NULL) return;

	os_strcpy(buff, "Unknown");

	if (os_strcmp(token, "ntp-enable")==0) {
			os_sprintf(buff, "%d", (int)sysCfg.ntp_enable);
	}

	if (os_strcmp(token, "ntp-tz")==0) {
			os_sprintf(buff, "%d", (int)sysCfg.ntp_tz);
	}

	if (os_strcmp(token, "NTP")==0) {
		os_sprintf(buff,"Time: %s GMT%s%02d\n",epoch_to_str(sntp_time+(sntp_tz*3600)),sntp_tz > 0 ? "+" : "",sntp_tz);
	}


	httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiNTP(HttpdConnData *connData) {
	char buff[128];
	int len;

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	len=httpdFindArg(connData->post->buff, "ntp-enable", buff, sizeof(buff));
	sysCfg.ntp_enable = len > 0 ? 1:0;

	len=httpdFindArg(connData->post->buff, "ntp-tz", buff, sizeof(buff));
	if (len>0) {
		sysCfg.ntp_tz=atoi(buff);
		sntp_tz=sysCfg.ntp_tz;
	}

	CFG_Save();


	httpdRedirect(connData, "/ntp.tpl");
	return HTTPD_CGI_DONE;
}


void ICACHE_FLASH_ATTR tplSchedule(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return;
	os_strcpy(buff, "Unknown");
	httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiSchedule(HttpdConnData *connData) {
	char buff[2048];
	char temp[128];
	char humi[32];
	int len=0;

	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "application/json");
	httpdHeader(connData, "Access-Control-Allow-Origin", "*");
	httpdEndHeaders(connData);

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	os_strcpy(buff, "Unknown");
	os_strcpy(temp, "N/A");
	os_strcpy(humi, "N/A");

	len=httpdFindArg(connData->getArgs, "param", buff, sizeof(buff));
	if (len>0) {

		if(os_strcmp(buff,"state")==0) {

			/*if(sysCfg.sensor_dht22_enable && (sysCfg.Lock_input == 1 || sysCfg.Lock_input == 2)) {
				dht_temp_str(temp);
				dht_humi_str(humi);
			}
			else if (sysCfg.sensor_ds18b20_enable && sysCfg.Lock_input == 0) {
				ds_str(temp,0);
			}

			else if (sysCfg.Lock_input == 4) { 		//Serial
				os_sprintf(temp,"%d.%d",(int)serialTreading/100,serialTreading-((int)serialTreading/100)*100);
			}

			else if (sysCfg.Lock_input == 5) { 		//Fixed value
				os_strcpy(temp,"10");
			}


			os_sprintf(buff, "{\"temperature\": \"%s\"\n,\"humidity\": \"%s\"\n,\"humidistat\": %d\n,\"relay1state\": %d\n,\"relay1name\":\"%s\",\n\"opmode\":%d\n,\"state\":%d,\n\"manualsetpoint\": %d\n,\"mode\":%d }\n",
				temp, humi, (int)sysCfg.Lock_input==2?1:0, currGPIO12State,(char *)sysCfg.relay1name,(int)sysCfg.Lockopmode, (int)sysCfg.Lockstate, (int)sysCfg.Lockmanualsetpoint,(int)sysCfg.Lockmode);

			*/
			os_sprintf(buff,"{\"manualsetpoint\":\"%d\"\n,\"mode\":\"%d\"\n,\"curLockState\":\"%d\"}\n",(int)sysCfg.Lockmanualsetpoint,(int)sysCfg.Lockmode, (int)sysCfg.Lockstate);
		}

		if(os_strcmp(buff,"temperature")==0) {
			//if(sysCfg.sensor_dht22_enable) {
			//	dht_temp_str(temp);
			}
			else {
				//ds_str(temp,0);
			}
			//os_sprintf(buff, "%s", temp);

		}


		if(os_strcmp(buff,"thermostat_opmode")==0) {
			//if(connData->post->len>0) {
			//	sysCfg.Lockopmode=(int)atoi(connData->post->buff);
			//	CFG_Save();
			//	os_printf("Handle thermostat opmode (%d) saved\n",(int)sysCfg.Lockopmode);
			//} else {
			//	os_sprintf(buff, "%d", (int)sysCfg.Lockopmode);
			//}
		}


		if(os_strcmp(buff,"thermostat_relay1state")==0) {
			if(connData->post->len>0) {
				//os_printf("N/A\n");
			} else {
			//	os_sprintf(buff, "%d", currGPIO12State);
			}
		}

		if(os_strcmp(buff,"lock_state")==0) {
			if(connData->post->len>0) {
			//	sysCfg.Lockstate=(int)atoi(connData->post->buff);

			//	currGPIO12State=0;
			//	ioGPIO(currGPIO12State,12);

			//	CFG_Save();
			//	os_printf("Handle thermostat state (%d) saved\n",(int)sysCfg.Lockstate);
			} else {
			//	os_sprintf(buff, "%d", (int)sysCfg.Lockstate);
			}
		}

		if(os_strcmp(buff,"manualsetpoint")==0) {
			if(connData->post->len>0) {
				sysCfg.Lockmanualsetpoint=(int)atoi(connData->post->buff);
				CFG_Save();
				os_printf("Handle thermostat manual setpoint save (%d)\n",(int)sysCfg.Lockmanualsetpoint);
			} else {
				os_sprintf(buff, "%d", (int)sysCfg.Lockmanualsetpoint);
			}
		}

		if(os_strcmp(buff,"lock_mode")==0) {
			if(connData->post->len>0) {
				sysCfg.Lockmode=(int)atoi(connData->post->buff);
				CFG_Save();
				os_printf("Handle lock mode save (%d)\n",(int)sysCfg.Lockmode);
			} else {
				os_sprintf(buff, "%d", (int)sysCfg.Lockmode);
			}

		}

		if(os_strcmp(buff,"schedule")==0) {
			char * days[7] = {"mon","tue","wed","thu","fri","sat","sun"};
			if(connData->post->len>0) {
				os_printf("Handle Lock schedule save\n");

					int r;
					jsmn_parser p;
					jsmntok_t t[128]; /* We expect no more than 64 tokens per day*/

					jsmn_init(&p);
					r = jsmn_parse(&p, connData->post->buff, strlen(connData->post->buff) , t, sizeof(t)/sizeof(t[0]));

					if (r < 0) {
						os_printf("Failed to parse JSON: %d\n", r);
						return HTTPD_CGI_DONE;
					}

					/* Assume the top-level element is an object */
					if (r < 1 || t[0].type != JSMN_OBJECT) {
						os_printf("Object expected\n");
						return HTTPD_CGI_DONE;
					}

					buff[0]=0x0;

					int found=-1;
					for(int i=0;i<7;i++) {
						if(os_memcmp(connData->post->buff + t[1].start,days[i],3)==0)
							found=i;
					}

					if(found<0) {
						os_printf("Could not find day schedule in JSON\n");
						return HTTPD_CGI_DONE;
					}


					os_printf("Schedule for %s found\n",days[found]);

					int sched=0;
					for (int i = 3; i < r && sched <8; i+=11) {	//skip the day and day array strings

					//Number of tokens will be 1 for the day+1 for the day data + (number of schedules * 9 tokens in a schedule element (one for the schedule itself then 8 tokens for start:val,end:val,setpoint:val,autolock:val)

						os_memcpy(temp, connData->post->buff + t[i+2].start,t[i+2].end - t[i+2].start);
						temp[t[i+2].end - t[i+2].start]=0x0;
						os_sprintf(buff+strlen(buff),"Start = %s\n", temp);
						sysCfg.Lockschedule.weekSched[found].daySched[sched].start=atoi(temp);

						os_memcpy(temp, connData->post->buff + t[i+4].start,t[i+4].end - t[i+4].start);
						temp[t[i+4].end - t[i+4].start]=0x0;
						os_sprintf(buff+strlen(buff),"End = %s\n", temp);
						sysCfg.Lockschedule.weekSched[found].daySched[sched].end=atoi(temp);

						os_memcpy(temp, connData->post->buff + t[i+6].start,t[i+6].end - t[i+6].start);
						temp[t[i+6].end - t[i+6].start]=0x0;
						os_sprintf(buff+strlen(buff),"Setpoint = %s\n", temp);
						sysCfg.Lockschedule.weekSched[found].daySched[sched].setpoint=atoi(temp);

						os_memcpy(temp, connData->post->buff + t[i+8].start,t[i+8].end - t[i+8].start);
						temp[t[i+8].end - t[i+8].start]=0x0;
						os_sprintf(buff+strlen(buff),"AutoLock = %s\n", temp);
						sysCfg.Lockschedule.weekSched[found].daySched[sched].autolock=atoi(temp);

						os_memcpy(temp, connData->post->buff + t[i+10].start,t[i+10].end - t[i+10].start);
						temp[t[i+10].end - t[i+10].start]=0x0;
						os_sprintf(buff+strlen(buff),"AutoTimeout = %s\n", temp);
						sysCfg.Lockschedule.weekSched[found].daySched[sched].autotimeout=atoi(temp);

						sysCfg.Lockschedule.weekSched[found].daySched[sched].active=1;

						sched++;
					}
					if(sched<8)
						sysCfg.Lockschedule.weekSched[found].daySched[sched].active=0;	//mark the next schedule as inactive

					if(sched>=8) {
						os_printf("Too many elements in Schedule\n");
						return HTTPD_CGI_DONE;
					}

					os_printf(buff);
					CFG_Save();

			} else {
				// Build schedule JSON from the config structure. Keeping the JSON string in memory is quite heavy on the RAM/heap, so we re-construct it
				os_strcpy(buff, "{");
				for(int dow=0; dow<7; dow++) {
					os_sprintf(buff+strlen(buff),"\"%s\":[",days[dow]);
					for(int sched=0; sched<8 && sysCfg.Lockschedule.weekSched[dow].daySched[sched].active==1; sched++) {
						os_sprintf(buff+strlen(buff),"{\"s\":%d,\"e\":%d,\"sp\":%d,\"al\":%d,\"at\":%d}",sysCfg.Lockschedule.weekSched[dow].daySched[sched].start,sysCfg.Lockschedule.weekSched[dow].daySched[sched].end,sysCfg.Lockschedule.weekSched[dow].daySched[sched].setpoint,sysCfg.Lockschedule.weekSched[dow].daySched[sched].autolock,sysCfg.Lockschedule.weekSched[dow].daySched[sched].autotimeout);
							if(sched<7 && sysCfg.Lockschedule.weekSched[dow].daySched[sched+1].active==1)
							os_sprintf(buff+strlen(buff),",");
					}
					os_sprintf(buff+strlen(buff),"]%s\n",dow<6 ? ",":"");
				}
				os_sprintf(buff+strlen(buff),"}");
			}
		}

	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}
