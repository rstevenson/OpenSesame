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

#define LOCKGPIO	14
#define UNLOCKGPIO	12
#define LOCKEDGPIO	0
#define UNLOCKEDGPIO	2

#define FAILTIMEOUT	5000 //failed to lock/unlock time in ms
#define FAILTIMEOUTTMR	(FAILTIMEOUT/10)

static ETSTimer lockDwellTimer;
static ETSTimer unlockDwellTimer;

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

static void ICACHE_FLASH_ATTR lockDwellTimerCb(void *arg) {
	static int cnt=0;
	if (GPIO_INPUT_GET(LOCKEDGPIO)) {
		cnt++;
	} else {
		sysCfg.Lockstate = LOCKED;
		gpio_output_set(0, (1<<LOCKGPIO), (1<<LOCKGPIO), 0);
		os_timer_disarm(&lockDwellTimer);
		cnt = 0;
	}
	if (cnt > 10) {
		sysCfg.Lockstate = FAILEDTOLOCK;
		gpio_output_set(0, (1<<LOCKGPIO), (1<<LOCKGPIO), 0);
		os_timer_disarm(&lockDwellTimer);
		cnt = 0;
	}
}

static void ICACHE_FLASH_ATTR unlockDwellTimerCb(void *arg) {
	static int cnt=0;
	if (GPIO_INPUT_GET(UNLOCKEDGPIO)) {
		cnt++;
	} else {
		sysCfg.Lockstate = UNLOCKED;
		gpio_output_set(0, (1<<UNLOCKGPIO), (1<<UNLOCKGPIO), 0);
		os_timer_disarm(&unlockDwellTimer);
		cnt = 0;
	}
	if (cnt > 10) {
		sysCfg.Lockstate = FAILEDTOUNLOCK;
		gpio_output_set(0, (1<<UNLOCKGPIO), (1<<UNLOCKGPIO), 0);
		os_timer_disarm(&unlockDwellTimer);
		cnt = 0;
	}
}

void ICACHE_FLASH_ATTR lock(int lock, int autoRetry, int retryTimeout)
{
	if (lock) {
		if (GPIO_INPUT_GET(LOCKEDGPIO)) {
			os_timer_disarm(&unlockDwellTimer);
			gpio_output_set((1<<LOCKGPIO), 0, (1<<LOCKGPIO), 0);
			gpio_output_set(0, (1<<UNLOCKGPIO), (1<<UNLOCKGPIO), 0);
			os_timer_arm(&lockDwellTimer, FAILTIMEOUTTMR, 1);
		}
	} else {
		if (GPIO_INPUT_GET(UNLOCKEDGPIO)) {
			os_timer_disarm(&lockDwellTimer);
			gpio_output_set(0, (1<<LOCKGPIO), (1<<LOCKGPIO), 0);
			gpio_output_set((1<<UNLOCKGPIO), 0, (1<<UNLOCKGPIO), 0);
			os_timer_arm(&unlockDwellTimer, FAILTIMEOUTTMR, 1);
		}
	}
}

void ICACHE_FLASH_ATTR manualLock(int lock)
{
	if (lock) {
		if (GPIO_INPUT_GET(LOCKEDGPIO)) {
			os_timer_disarm(&unlockDwellTimer);
			gpio_output_set((1<<LOCKGPIO), 0, (1<<LOCKGPIO), 0);
			gpio_output_set(0, (1<<UNLOCKGPIO), (1<<UNLOCKGPIO), 0);
			os_timer_arm(&lockDwellTimer, FAILTIMEOUTTMR, 1);
		} else
			sysCfg.Lockstate = LOCKED;
	} else {
		if (GPIO_INPUT_GET(UNLOCKEDGPIO)) {
			os_timer_disarm(&lockDwellTimer);
			gpio_output_set(0, (1<<LOCKGPIO), (1<<LOCKGPIO), 0);
			gpio_output_set((1<<UNLOCKGPIO), 0, (1<<UNLOCKGPIO), 0);
			os_timer_arm(&unlockDwellTimer, FAILTIMEOUTTMR, 1);
		} else
			sysCfg.Lockstate = UNLOCKED;
	}
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
	unsigned int setpoint;


	if(year<2015) { // Something is wrong with the NTP time, maybe not enabled?
		os_printf("NTP time seems incorrect!\n");
		return;
	}
	if (sysCfg.Lockmode == LOCK_AUTO){
		for(int sched=0; sched<8 && sysCfg.Lockschedule.weekSched[dow].daySched[sched].active==1; sched++) {
				if(currtime >= sysCfg.Lockschedule.weekSched[dow].daySched[sched].start && currtime < sysCfg.Lockschedule.weekSched[dow].daySched[sched].end) {
					os_printf("Current schedule (%d) setpoint is: %d\n",sched,sysCfg.Lockschedule.weekSched[dow].daySched[sched].setpoint);
					setpoint = sysCfg.Lockschedule.weekSched[dow].daySched[sched].setpoint;
					lock(setpoint,sysCfg.Lockschedule.weekSched[dow].daySched[sched].autolock,sysCfg.Lockschedule.weekSched[dow].daySched[sched].autotimeout);
					sysCfg.Lockmanualsetpoint = setpoint;
					sysCfg.LockLastSP = setpoint;
					sysCfg.LockLastDaySched = sched;
				}
		}
	} else{
			manualLock(sysCfg.Lockmanualsetpoint);
	}
}

void ICACHE_FLASH_ATTR lock_init(uint32_t polltime){
	os_printf("Lock daemon init; poll interval of %d sec\n", (int)polltime/1000);
	sysCfg.Lockstate = 99;

	static ETSTimer lockTimer;
	os_timer_setfn(&lockTimer, pollLockCb, NULL);
	os_timer_arm(&lockTimer, polltime, 1);

	os_timer_disarm(&lockDwellTimer);
	os_timer_setfn(&lockDwellTimer, lockDwellTimerCb, NULL);

	os_timer_disarm(&unlockDwellTimer);
	os_timer_setfn(&unlockDwellTimer, unlockDwellTimerCb, NULL);

	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
	//PIN_PULLDOWN_EN(PERIPHS_IO_MUX_MTCK_U);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
	//PIN_PULLDOWN_EN(PERIPHS_IO_MUX_MTDO_U);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);


	gpio_output_set(0, 0, (1<<LOCKGPIO)|(1<<UNLOCKGPIO), (1<<LOCKEDGPIO)|(1<<UNLOCKEDGPIO));

}

