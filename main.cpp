/* Revision: 2.8.1 */

/******************************************************************************
* Copyright 1998-2016 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 5405 Morehouse Dr.
* San Diego, CA 92121
* www.netburner.com
******************************************************************************/

 /*******************************************************************************
 I2C_Scan sends out a call on the bus and waits for an ack, scanning each address 
 in the address space.  If a device responds, it reports that it's found a device 
 at xx address. This is really useful for figuring out what address you should 
 give to the sendbuf and readbuf commands to talk to your i2c devices.

 This should be helpful in two ways:
 1) It tells you that you have the hardware hooked up right.
 2) It avoids confusing datasheets. (Anyone familiar with i2c datasheets will know
  their address are written 3 different ways and 2 of them are wrong depending on
  who you ask.)
 ******************************************************************************/
#include "predef.h" 
#include <stdio.h>
#include <ctype.h>
#include <startnet.h>
#include <autoupdate.h>
#include <dhcpclient.h>
#include <smarttrap.h>
#include <taskmon.h>
#include <i2cmaster.h>
#include <pins.h>
#include "RtcPcf8563.h"
#include <sim.h>
#include <udp.h>
#include <multipartpost.h>
#include <StreamUpdate.h>
#include <constants.h>
#include <tcp.h>

//#include <multichanneli2c.h>
//#include <i2cmulti.h>

//extern "C"
//{
  // void UserMain(void * pd);
  // extern "C"  void WriteToPages( int sock, PCSTR url );
  // int MyDoPost( int sock, char *url, char *pData, char *rxBuffer );

//}


#define MAX_FILE_SIZE  10000

const char * AppName = "I2C_Scan";
static TwoPartUpdateStruct up_struct;
static DWORD dwWroteHeaderTime;


byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}


extern "C" void WriteToPages( int sock, PCSTR url )
{
        Rtc_Pcf8563 Rtc;
        char buffer[255];
        iprintf("cas: %s", url);
        siprintf( buffer, "%s \n", "zde bude cas");
        writestring(sock, Rtc.formatTime(RTCC_TIME_HMS)); // Data to output goes here );
}

int MyDoPost( int sock, char *url, char *pData, char *rxBuffer )
{
  if(dwWroteHeaderTime  < (TimeTick - 6))
  {
    writestring(sock, "<BR>File did not uploaded correctly<BR>");
    //return 0;
  }
  else
  {
      writestring(sock, "<BR>File uploaded correctly<BR>");
  }

  iprintf("post: %s, %s, %s \n\r" , url, pData, rxBuffer);
  if (up_struct.Result == STREAM_UP_OK)
  {
    clrsockoption(sock, SO_NOPUSH|SO_NONAGLE);
    iprintf("STREAM_UP_OK :\n\r" );
    iprintf((char *)up_struct.S0Record);
    iprintf("\n\r" );
    if (DoTwoPartAppUpdate(up_struct)==STREAM_UP_OK)
    {
      writestring(sock, "<BR>reboot for new application<BR>");
    }
    else
    {
      writestring(sock, "<BR>flash failure<BR>");
    }
  }
  else
  {
    iprintf("STREAM_UP_KO :\n\r" );
  }

  close (sock);
  AbortTwoPartAppUpdate(up_struct);
  return 0;
}


void fnFileIsHere(int fd, const char * url)
{
  SendHTMLHeader(fd);
  writestring(fd, "FileIsHere");
  writestring(fd, url);
  iprintf("file is here: %s \n\r", url);
  ReadTwoPartAppUdate(fd, up_struct);
  dwWroteHeaderTime = TimeTick;
  return;
}


void UserMain(void * pd)
{
    InitializeStack(); /* Setup the TCP/IP stack buffers */
    //GetDHCPAddressIfNecessary(); /* Get a DHCP address if needed */
    /*You may want to add a check for the return value from this function*/
    /*See the function definition in  \nburn\include\dhcpclient.h*/

    OSChangePrio(MAIN_PRIO); /* Change our priority from highest to something in the middle */
    EnableAutoUpdate(); /* Enable the ability to update code over the network */
    StartHTTP();
    EnableSmartTraps(); /* Enable the smart reporting of traps and faults */
    EnableTaskMonitor(); /*Enable the Task scan utility */

    //HTML Update
    if(EnableMultiPartForms(MAX_FILE_SIZE))
    {
      RegisterSpecialLongFile("myFile", fnFileIsHere);
      SetNewPostHandler( MyDoPost );
    }
    else
    {
      iprintf("MultiPart not allocated \n\r");
    }



    /* The I2C bus speed on the 5270 processor is set by a divider of the internal
     * clock frequency of 147.5MHz / 2 = 73.75MHz. The maximum I2C bus speed is
     * 100KHz. 73.75MHz/100KHz = 737.5. Referring to the I2C freq divider table
     * in the Freescale manual the closest divider is 768 (register value = 0x39).
     * 73.75MHz/768 = 95,703 bps.
     */

    // I2CInit(0x08,0x3C);  // I2C multi parameters are ( slave id, freq divider )

    //J2[39].function( PINJ2_39_I2C0_SDA  );
    //J2[42].function( PINJ2_42_I2C0_SCL   );


    I2CInit(0x3C);          // Approx 100Khz.  I2C master parameters are ( freq divider )

    iprintf("Scanning I2C \r\n");

    for (int x = 1; x < 0x80; x++)
    {
        int result = I2CStart(x, I2C_START_WRITE, 1);
        if (result < I2C_TIMEOUT)
        {
            iprintf("Found device at  0x%X, Result: %d\r\n", x, result);
            I2CStop();
        }
        else
        {
            I2CStop();
            I2CResetPeripheral();
        }
    }

    iprintf("Scan complete\r\n");

    Rtc_Pcf8563 Rtc;
    Rtc.setDate(2, 1, 1, 0, 19);
    Rtc.setTime(11, 23, 45);

    byte reg;
    byte status;
    //byte regBuff[7];
    while (1)
    {

        byte a[5];
        //iprintf("date: %s, time: %s \r\n", Rtc.formatDate(RTCC_DATE_WORLD), Rtc.formatTime(RTCC_TIME_HMS));
        iprintf("ver1 t: %s \r\n",  Rtc.formatTime(RTCC_TIME_HMS));


        {
          UDPPacket PacketToSend;
          PacketToSend.AddData(Rtc.formatTime(RTCC_TIME_HMS));
          PacketToSend.AddData("\r\n");
          PacketToSend.SetSourcePort(50005);
          PacketToSend.SetDestinationPort(50006);
          PacketToSend.Send(AsciiToIp("192.168.10.5"));
        }


/*

        status = I2CStart(0x51, I2C_START_WRITE);  //iprintf("Start write, s:%d\r\n", status);
        status = I2CSend(0); //iprintf("Send, s:%d\r\n", status);
        status = I2CStop(); //iprintf("Stop, s:%d\r\n", status);
        status = I2CStart(0x51, I2C_START_READ); //iprintf("Start read, s:%d\r\n", status);
        status = I2CRead(&reg); //dummy read
        status = I2CRead(&reg); a[0] = reg;//iprintf("r: %x \r\n",  reg);
        status = I2CRead(&reg); a[1] = reg;//iprintf("r: %x \r\n",  reg);
        status = I2CRead(&reg); a[2] = reg;//iprintf("r: %x \r\n",  reg);
        status = I2CRead(&reg); a[3] = reg;//iprintf("r: %x \r\n",  reg);
        I2C_SET_NO_ACK;
        status = I2CRead(&reg); a[4] = reg;//iprintf("r: %x \r\n",  reg);
        status = I2CStop(); //iprintf("Stop, s:%d\r\n", status);
        iprintf("%X,%X,%X,%X,%X\r\n",a[0],a[1],a[2],a[3],a[4]);
*/


        /*
        reg = 2;
        status = I2CSendBuf(0x51, &reg, 1); iprintf("Send, s:%d\r\n", status);
        status = I2CReadBuf(0x51, regBuff, sizeof(regBuff)); iprintf("Read, s:%d\r\n", status);

        iprintf("Value: ");
        for (int i = 0; i < (int)sizeof(regBuff); ++i)
        {
          iprintf("%d ", regBuff[i]);
        }
        iprintf(" sec: %d", bcdToDec(regBuff[0] & 0x7F));
        iprintf("\r\n");
        */

        OSTimeDly(TICKS_PER_SECOND);
    }
}
