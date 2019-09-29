/*
 * RtcPcf8563.h
 *
 *  Created on: 2.1.2019
 *      Author: Svoboda
 */

#ifndef RTCPCF8563_H_
#define RTCPCF8563_H_

//#define I2C_DEBUG_MESSAGES

#include <stdint.h>
#include <i2cmaster.h>
#include <stdio.h>
#include <ucos.h>


#define byte uint8_t


#define RTCC_VERSION  "Pcf8563 v1.0.3"
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/* the read and write values for pcf8563 rtcc */
/* these are adjusted for arduino */
#define RTCC_R  0xa3
#define RTCC_W  0xa2

#define RTCC_SEC                        1
#define RTCC_MIN                        2
#define RTCC_HR                         3
#define RTCC_DAY                        4
#define RTCC_WEEKDAY    5
#define RTCC_MONTH              6
#define RTCC_YEAR               7
#define RTCC_CENTURY    8

/* register addresses in the rtc */
#define RTCC_STAT1_ADDR                 0x0
#define RTCC_STAT2_ADDR                 0x01
#define RTCC_SEC_ADDR                   0x02
#define RTCC_MIN_ADDR                   0x03
#define RTCC_HR_ADDR                    0x04
#define RTCC_DAY_ADDR                   0x05
#define RTCC_WEEKDAY_ADDR               0x06
#define RTCC_MONTH_ADDR                 0x07
#define RTCC_YEAR_ADDR                  0x08
#define RTCC_ALRM_MIN_ADDR              0x09
#define RTCC_SQW_ADDR                   0x0D

/* setting the alarm flag to 0 enables the alarm.
 * set it to 1 to disable the alarm for that value.
 */
#define RTCC_ALARM                      0x80
#define RTCC_ALARM_AIE                  0x02
#define RTCC_ALARM_AF                   0x08 // 0x08 : not 0x04!!!!
/* optional val for no alarm setting */
#define RTCC_NO_ALARM                   99

#define RTCC_CENTURY_MASK               0x80

/* date format flags */
#define RTCC_DATE_WORLD                 0x01
#define RTCC_DATE_ASIA                  0x02
#define RTCC_DATE_US                    0x04
/* time format flags */
#define RTCC_TIME_HMS                   0x01
#define RTCC_TIME_HM                    0x02

/* square wave contants */
#define SQW_DISABLE                     0x00 //B0000 0000
#define SQW_32KHZ                       0x80 //B1000 0000
#define SQW_1024HZ                      0x81 //B1000 0001
#define SQW_32HZ                        0x82 //B1000 0010
#define SQW_1HZ                         0x83 //B1000 0011

class ArduinoI2CWrapper
{
public:
  ArduinoI2CWrapper();
  byte reg;
  int status;
  int toRead;
  int toSend;

  void beginTransmission(byte addr);
  void send(byte tx);
  void endTransmission(void);
  void requestFrom(byte addr, int size);
  byte read(void);
  byte readBuff[16];
  byte sendBuff[16];
  byte slaveAddr;
};


class Rtc_Pcf8563
{
public:
        Rtc_Pcf8563();

        void initClock();               /* zero out all values, disable all alarms */
        void clearStatus();     /* set both status bytes to zero */

        void getDate();                 /* get date vals to local vars */
        void setDate(byte day, byte weekday, byte month, byte century, byte year);
        void getTime();    /* get time vars + 2 status bytes to local vars */
        void getAlarm();
        void setTime(byte sec, byte minute, byte hour);
        byte readStatus2();
        bool alarmEnabled();
        bool alarmActive();

        void enableAlarm(); /* activate alarm flag and interrupt */
        void setAlarm(byte min, byte hour, byte day, byte weekday); /* set alarm vals, 99=ignore */
        void clearAlarm();      /* clear alarm flag and interrupt */
        void resetAlarm();  /* clear alarm flag but leave interrupt unchanged */
        void setSquareWave(byte frequency);
        void clearSquareWave();

        byte getSecond();
        byte getMinute();
        byte getHour();
        byte getDay();
        byte getMonth();
        byte getYear();
        byte getWeekday();
        byte getStatus1();
        byte getStatus2();

        byte getAlarmMinute();
        byte getAlarmHour();
        byte getAlarmDay();
        byte getAlarmWeekday();

        /*get a output string, these call getTime/getDate for latest vals */
        char *formatTime(byte style=RTCC_TIME_HMS);
        /* date supports 3 styles as listed in the wikipedia page about world date/time. */
        char *formatDate(byte style=RTCC_DATE_US);

        char *version();

private:
        /* methods */
        byte decToBcd(byte value);
        byte bcdToDec(byte value);
        /* time variables */
        byte hour;
        byte minute;
        byte sec;
        byte day;
        byte weekday;
        byte month;
        byte year;
        /* alarm */
        byte alarm_hour;
        byte alarm_minute;
        byte alarm_weekday;
        byte alarm_day;
        /* support */
        byte status1;
        byte status2;
        byte century;

        char strOut[9];
        char strDate[11];

        int Rtcc_Addr;

        ArduinoI2CWrapper Wire;

};








#endif /* RTCPCF8563_H_ */
