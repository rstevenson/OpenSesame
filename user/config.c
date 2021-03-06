/*
 config.c
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "config.h"
#include "user_config.h"
#include "debug.h"

SYSCFG sysCfg;
SAVE_FLAG saveFlag;

void ICACHE_FLASH_ATTR CFG_Save()
{
	 spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
	                   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));

	if (saveFlag.flag == 0) {
		spi_flash_erase_sector(CFG_LOCATION + 1);
		spi_flash_write((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&sysCfg, sizeof(SYSCFG));
		saveFlag.flag = 1;
		spi_flash_erase_sector(CFG_LOCATION + 3);
		spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	} else {
		spi_flash_erase_sector(CFG_LOCATION + 0);
		spi_flash_write((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&sysCfg, sizeof(SYSCFG));
		saveFlag.flag = 0;
		spi_flash_erase_sector(CFG_LOCATION + 3);
		spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	}
}

void ICACHE_FLASH_ATTR CFG_Load()
{

	os_printf("\r\nload cfg...(%d bytes)\r\n", sizeof(SYSCFG) );
	spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
				   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	if (saveFlag.flag == 0) {
		spi_flash_read((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
					   (uint32 *)&sysCfg, sizeof(SYSCFG));
	} else {
		spi_flash_read((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
					   (uint32 *)&sysCfg, sizeof(SYSCFG));
	}
	if(sysCfg.cfg_holder != CFG_HOLDER){
		os_memset(&sysCfg, 0x00, sizeof sysCfg);

		sysCfg.cfg_holder = CFG_HOLDER;
	
		os_sprintf((char *)sysCfg.sta_mode, "%s", STA_MODE);
		os_sprintf((char *)sysCfg.sta_ip, "%s", STA_IP);
		os_sprintf((char *)sysCfg.sta_mask, "%s", STA_MASK);
		os_sprintf((char *)sysCfg.sta_gw, "%s", STA_GW);
		os_sprintf((char *)sysCfg.sta_ssid, "%s", STA_SSID);
		os_sprintf((char *)sysCfg.sta_pass, "%s", STA_PASS);
		sysCfg.sta_type=STA_TYPE;

		os_sprintf((char *)sysCfg.ap_ip, "%s", AP_IP);
		os_sprintf((char *)sysCfg.ap_mask, "%s", AP_MASK);
		os_sprintf((char *)sysCfg.ap_gw, "%s", AP_GW);

		sysCfg.httpd_port=HTTPD_PORT;
		sysCfg.httpd_auth=HTTPD_AUTH;
		os_sprintf((char *)sysCfg.httpd_user, "%s", HTTPD_USER);
		os_sprintf((char *)sysCfg.httpd_pass, "%s", HTTPD_PASS);

		sysCfg.ntp_enable=NTP_ENABLE;
		sysCfg.ntp_tz=NTP_TZ;
		
		sysCfg.Lockstate=0;
		sysCfg.Lockmanualsetpoint=2100;
		sysCfg.Lockmode=LOCK_MANUAL;
		sysCfg.Lockopmode=UNLOCKED;
		
		//Build default schedule for the Lock
		for(int dow=0; dow<7; dow++) {
			sysCfg.Lockschedule.weekSched[dow].daySched[0].start=0; //0am
			sysCfg.Lockschedule.weekSched[dow].daySched[0].end=600; //6am, hours are * 100
			sysCfg.Lockschedule.weekSched[dow].daySched[0].setpoint=1; //Lock
			sysCfg.Lockschedule.weekSched[dow].daySched[0].autolock=1; //Auto Locked
			sysCfg.Lockschedule.weekSched[dow].daySched[0].autotimeout=30; //Auto Timeout, for relocking
			sysCfg.Lockschedule.weekSched[dow].daySched[0].active=1;
			
			sysCfg.Lockschedule.weekSched[dow].daySched[1].start=600;
			sysCfg.Lockschedule.weekSched[dow].daySched[1].end=900;
			sysCfg.Lockschedule.weekSched[dow].daySched[1].setpoint=1;
			sysCfg.Lockschedule.weekSched[dow].daySched[1].autolock=1;
			sysCfg.Lockschedule.weekSched[dow].daySched[1].autotimeout=30;
			sysCfg.Lockschedule.weekSched[dow].daySched[1].active=1;
			
			sysCfg.Lockschedule.weekSched[dow].daySched[2].start=900;
			sysCfg.Lockschedule.weekSched[dow].daySched[2].end=1700;
			sysCfg.Lockschedule.weekSched[dow].daySched[2].setpoint=1;
			sysCfg.Lockschedule.weekSched[dow].daySched[2].autolock=1;
			sysCfg.Lockschedule.weekSched[dow].daySched[2].autotimeout=30;
			sysCfg.Lockschedule.weekSched[dow].daySched[2].active=1;
			
			sysCfg.Lockschedule.weekSched[dow].daySched[3].start=1700;
			sysCfg.Lockschedule.weekSched[dow].daySched[3].end=2200;
			sysCfg.Lockschedule.weekSched[dow].daySched[3].setpoint=1;
			sysCfg.Lockschedule.weekSched[dow].daySched[3].autolock=1;
			sysCfg.Lockschedule.weekSched[dow].daySched[3].autotimeout=30;
			sysCfg.Lockschedule.weekSched[dow].daySched[3].active=1;

			sysCfg.Lockschedule.weekSched[dow].daySched[4].start=2200;
			sysCfg.Lockschedule.weekSched[dow].daySched[4].end=2400;
			sysCfg.Lockschedule.weekSched[dow].daySched[4].setpoint=1;
			sysCfg.Lockschedule.weekSched[dow].daySched[4].autolock=1;
			sysCfg.Lockschedule.weekSched[dow].daySched[4].autotimeout=30;
			sysCfg.Lockschedule.weekSched[dow].daySched[4].active=1;
			
			sysCfg.Lockschedule.weekSched[dow].daySched[5].active=0; //Terminate
		}

		os_printf(" default configurations\r\n");

		CFG_Save();
	}

}
