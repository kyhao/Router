// W5500 wiring driver DHCP
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "Ethernet/socket.h"
#include "Internet/DHCP/dhcp.h"

/////////////////////////////////////////
// SOCKET NUMBER DEFINION for Examples //
/////////////////////////////////////////
#define SOCK_TCPS        0
#define SOCK_UDPS        1
#define SOCK_DHCP		 7

////////////////////////////////////////////////
// Shared Buffer Definition for LOOPBACK TEST //
////////////////////////////////////////////////
#define DATA_BUF_SIZE   2048
uint8_t gDATABUF[DATA_BUF_SIZE];

//////////
// DHCP //
//////////
uint8_t netconfig = NETINFO_DHCP;
//uint8_t netconfig = NETINFO_STATIC;

///////////////////////////
// Network Configuration //
///////////////////////////
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},
                            .ip = {192, 168, 1, 123},
                            .sn = {255,255,255,0},
                            .gw = {192, 168, 1, 1},
                            .dns = {0,0,0,0},
                            .dhcp = NETINFO_STATIC };

uint8_t data_buf[1];

//////////////////////////////////////////////////////////////////////////////////////////////
// Call back function for W5500 SPI - Theses used to parameter or reg_wizchip_xxx_cbfunc()  //
//////////////////////////////////////////////////////////////////////////////////////////////
void  wizchip_select(void);
void  wizchip_deselect(void);
void  wizchip_write(uint8_t wb);
uint8_t wizchip_read();


void platform_init(void);
void network_init(void);
void display_netinfo(void);


int32_t loopback_tcps(uint8_t, uint8_t*, uint16_t);
int32_t loopback_udps(uint8_t, uint8_t*, uint16_t);

////////////////////////////////////////////////////////
// SysTick Interrupt Handler for DHCP timeout counter //
////////////////////////////////////////////////////////
uint32_t systick_counter;

void SysTickIntHandler(void)
{
	if (systick_counter++ > 1000) { // 1m x 1000 = 1sec
		systick_counter = 0;
		DHCP_timerHandler(); // for DHCP timeout counter;
	}
}

//////////////////////////////////////////////////////////////////////////////
// spi使用前的准备 返回spi的文件描述符 //
//////////////////////////////////////////////////////////////////////////////
int initSPI()
{
    int spiFd; //spi的文件描述符
    //初始化所用到的IO引脚

    //初始化SPI通道0，并设置为最大速度32000000
    spiFd=wiringPiSPISetup(0,500000);
    if(spiFd==-1)
    {
        printf("init spi failed!\n");
    }  
}


//////////////////////////////////////////////////////////////////////////////
// Platform (ARM) initialization for peripherals as GPIO, SPI, UARTs //
//////////////////////////////////////////////////////////////////////////////
void platform_init(void)
{
    //初始化wiringPI的库函数
    if(wiringPiSetup()<0)
    {
        printf("init wiringPi error\n");
    }
    initSPI();
    pinMode(1, OUTPUT);

}

void network_init(void)
{
	// Set default Network information
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
}

// Display network information from WIZchip registers to serial terminal
void display_netinfo(void)
{
	uint8_t tmpstr[6] = {0,};
	wiz_NetInfo netinfo;
	ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);

	// Display Network Information
	ctlwizchip(CW_GET_ID,(void*)tmpstr);

	if(netinfo.dhcp == NETINFO_DHCP) printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
	else printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);

	printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],
			netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
	printf("SIP: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
	printf("===========================\r\n");
}


int main(void)
{
   uint8_t tmp;
   int32_t ret = 0;
   uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
   uint8_t dhcp_ret;

   platform_init();

   /////////////////////////////////////////////
   // Register SPI callback function of W5500 //
   /////////////////////////////////////////////

   /* Critical section callback - No use in this example */
   //reg_wizchip_cris_cbfunc(0, 0);

   /* Chip selection call back */
#if   _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_select);  // CS must be tried with LOW.
#else
   #if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
      #error "Unknown _WIZCHIP_IO_MODE_"
   #else
      reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
   #endif
#endif
    /* SPI Read & Write callback function */
    reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
    /////////////////////////////////////////////


    /* wizchip initialize*/
    if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1)
    {
       printf("WIZCHIP Initialized fail.\r\n");
       while(1);
    }

    /* PHY link status check */
    do
    {
       if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
          printf("Unknown PHY Link stauts.\r\n");
    }while(tmp == PHY_LINK_OFF);

    /* Network initialization */
    network_init();

    if(netconfig == NETINFO_STATIC) display_netinfo();
    /************************************************/
    /* >> DHCP Client 								*/
    /************************************************/
    /* DHCP Initialization */
    if(netconfig == NETINFO_DHCP) DHCP_init(SOCK_DHCP, gDATABUF);

    /*******************************/
   /* WIZnet W5500 Code Examples  */
   /* TCPS/UDPS Loopback test     */
   /*******************************/
    /* Main loop */
    while(1)
	{
        /* DHCP */
		/* DHCP IP allocation and check the DHCP lease time (for IP renewal) */
    	if(netconfig == NETINFO_DHCP) {

    		dhcp_ret = DHCP_run();

			if((dhcp_ret == DHCP_SUCCESS) || (dhcp_ret == DHCP_IP_CHANGED)) {
				getIPfromDHCP(gWIZNETINFO.ip);
				getGWfromDHCP(gWIZNETINFO.gw);
				getSNfromDHCP(gWIZNETINFO.sn);
				getDNSfromDHCP(gWIZNETINFO.dns);
				gWIZNETINFO.dhcp = NETINFO_DHCP;
				ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

				display_netinfo();
				printf("\r\n>> DHCP IP Leased Time : %ld Sec\r\n", getDHCPLeasetime());
			}

			if(dhcp_ret == DHCP_FAILED)	{
					printf(">> DHCP Failed\r\n");
					// User's parts : DHCP failed
					// ===== Example pseudo code =====
					// netconfig = NETINFO_STATIC;
					// set_netinfo_default();
			}
		}

        ///////////////////////////
        //   User's applaction   //
        ///////////////////////////
    	/* Loopback Test */
    	// TCP server loopback test
    	if( (ret = loopback_tcps(SOCK_TCPS, gDATABUF, 5000)) < 0) {
			printf("SOCKET ERROR : %ld\r\n", ret);
		}
        
    	// UDP server loopback test
		if( (ret = loopback_udps(SOCK_UDPS, gDATABUF, 3000)) < 0) {
			printf("SOCKET ERROR : %ld\r\n", ret);
		}
        //printf("%s\n", gDATABUF);
	} // end of Main loop



} // end of main()


void  wizchip_select(void)
{
   digitalWrite(1, LOW);
}

void  wizchip_deselect(void)
{
   digitalWrite(1, HIGH);
}

void  wizchip_write(uint8_t wb)
{
    data_buf[0] = wb;
    wiringPiSPIDataRW(0, data_buf, 1); //向总线中写入&个数据
}

uint8_t wizchip_read()
{
    data_buf[0] = 0x00;
    wiringPiSPIDataRW(0, data_buf, 1); //向总线中写入&个数据
    return data_buf[0];
}

int32_t loopback_tcps(uint8_t sn, uint8_t* buf, uint16_t port)
{
   int32_t ret;
   uint16_t size = 0, sentsize=0;
   switch(getSn_SR(sn))
   {
      case SOCK_ESTABLISHED :
         if(getSn_IR(sn) & Sn_IR_CON)
         {
            printf("%d:Connected\r\n",sn);
            setSn_IR(sn,Sn_IR_CON);
         }
         if((size = getSn_RX_RSR(sn)) > 0)
         {
            if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
            ret = recv(sn,buf,size);
            if(ret <= 0) return ret;
            sentsize = 0;
            while(size != sentsize)
            {
               ret = send(sn,buf+sentsize,size-sentsize);
               if(ret < 0)
               {
                  close(sn);
                  return ret;
               }
               sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
         }
         break;
      case SOCK_CLOSE_WAIT :
         printf("%d:CloseWait\r\n",sn);
         if((ret=disconnect(sn)) != SOCK_OK) return ret;
         printf("%d:Closed\r\n",sn);
         break;
      case SOCK_INIT :
    	  printf("%d:Listen, port [%d]\r\n",sn, port);
         if( (ret = listen(sn)) != SOCK_OK) return ret;
         break;
      case SOCK_CLOSED:
         printf("%d:LBTStart\r\n",sn);
         if((ret=socket(sn,Sn_MR_TCP,port,0x00)) != sn)
            return ret;
         printf("%d:Opened\r\n",sn);
         break;
      default:
         break;
   }
   return 1;
}

int32_t loopback_udps(uint8_t sn, uint8_t* buf, uint16_t port)
{
   int32_t  ret;
   uint16_t size, sentsize;
   uint8_t  destip[4];
   uint16_t destport;
   //uint8_t  packinfo = 0;
   switch(getSn_SR(sn))
   {
      case SOCK_UDP :
         if((size = getSn_RX_RSR(sn)) > 0)
         {
            if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
            ret = recvfrom(sn,buf,size,destip,(uint16_t*)&destport);
            if(ret <= 0)
            {
               printf("%d: recvfrom error. %ld\r\n",sn,ret);
               return ret;
            }
            size = (uint16_t) ret;
            sentsize = 0;
            while(sentsize != size)
            {
               ret = sendto(sn,buf+sentsize,size-sentsize,destip,destport);
               if(ret < 0)
               {
                  printf("%d: sendto error. %ld\r\n",sn,ret);
                  return ret;
               }
               sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
         }
         break;
      case SOCK_CLOSED:
         printf("%d:LBUStart\r\n",sn);
         if((ret=socket(sn,Sn_MR_UDP,port,0x00)) != sn)
            return ret;
         printf("%d:Opened, port [%d]\r\n",sn, port);
         break;
      default :
         break;
   }
   return 1;
}