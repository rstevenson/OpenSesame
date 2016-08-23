/* config.h
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

#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_
#include "os_type.h"
#include "user_config.h"

#define LOCK_MANUAL 0
#define LOCK_AUTO 1

#define LOCKED 0
#define UNLOCKED 1

typedef struct {
 uint16_t start;
 uint16_t end;
 uint16_t setpoint;  // Locked=1, Unlocked=0
 uint16_t active; 	 // pad to 4 byte boundary
 } dayScheduleElement;
 
typedef struct {
dayScheduleElement daySched[8]; // Max 8 schedules per day
}  daySchedule;
 
typedef struct {
daySchedule weekSched[7]; // 7 days per week
}  weekSchedule;


typedef struct{
	
//4 byte alignment, hence uint32_t
uint32_t cfg_holder;

uint8_t sta_mode[8];
uint8_t sta_ip[16];
uint8_t sta_mask[16];
uint8_t sta_gw[16];
uint8_t sta_ssid[32];
uint8_t sta_pass[32];
uint32_t sta_type;

uint8_t ap_ip[32];
uint8_t ap_mask[32];
uint8_t ap_gw[32];

uint32_t httpd_port;
uint32_t httpd_auth;
uint8_t httpd_user[16];
uint8_t httpd_pass[16];

uint32_t ntp_enable;
int32_t ntp_tz;

	
uint32_t Lockstate;
uint32_t Lockmanualsetpoint;
uint32_t Lockmode;
uint32_t Lockopmode;
weekSchedule Lockschedule;

	
} SYSCFG;

typedef struct {
    uint8 flag;
    uint8 pad[3];
} SAVE_FLAG;

void CFG_Save();
void CFG_Load();

 
 
extern SYSCFG sysCfg;

#endif /* USER_CONFIG_H_ */
