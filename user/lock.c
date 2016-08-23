/*
 * lock.c
 *
 *  Created on: Aug 7, 2016
 *      Author: Ryan
 */

#include "espmissingincludes.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "gpio.h"
#include "io.h"
#include <stdlib.h>
#include "config.h"
#include "sntp.h"
#include "time_utils.h"
#include "stdout.h"

static int ICACHE_FLASH_ATTR wd(int year, int month, int day) {
  size_t JND =                                                     \
          day                                                      \
        + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) \
        + (365 * (year + 4800 - ((14 - month) / 12)))              \
        + ((year + 4800 - ((14 - month) / 12)) / 4)                \
        - ((year + 4800 - ((14 - month) / 12)) / 100)              \
        + ((year + 4800 - ((14 - month) / 12)) / 400)              \
        - 32045;
  return (int)JND % 7;
}

void ICACHE_FLASH_ATTR lock(int setpoint)
{

}

static  void ICACHE_FLASH_ATTR pollLockCb(void * arg)
{
	unsigned long epoch = sntp_time+(sntp_tz*3600);
	int year=get_year(&epoch);
	int month=get_month(&epoch,year);
	int day=day=1+(epoch/86400);
	int dow=wd(year,month,day);
	epoch=epoch%86400;
	unsigned int hour=epoch/3600;
	epoch%=3600;
	unsigned int min=epoch/60;
	int minadj = (min*100/60);
	int currtime = hour*100+minadj;


	if(year<2015) { // Something is wrong with the NTP time, maybe not enabled?
		os_printf("NTP time seems incorrect!\n");
		return;
	}

	for(int sched=0; sched<8 && sysCfg.Lockschedule.weekSched[dow].daySched[sched].active==1; sched++) {
			if(currtime >= sysCfg.Lockschedule.weekSched[dow].daySched[sched].start && currtime < sysCfg.Lockschedule.weekSched[dow].daySched[sched].end) {
				os_printf("Current schedule (%d) setpoint is: %d\n",sched,sysCfg.Lockschedule.weekSched[dow].daySched[sched].setpoint);
				lock(sysCfg.Lockschedule.weekSched[dow].daySched[sched].setpoint);
			}
	}
}

void ICACHE_FLASH_ATTR lock_init(uint32_t polltime){
	os_printf("Lock daemon init; poll interval of %d sec\n", (int)polltime/1000);

	static ETSTimer lockTimer;
	os_timer_setfn(&lockTimer, pollLockCb, NULL);
	os_timer_arm(&lockTimer, polltime, 1);
}

