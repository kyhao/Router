/****************************************************************************    
* Ӳ�����ӣ�
* 
* LoRaģ����������
*  1--VDD--3.3V
*  3--GND--GND
*  7--PD4--DIO0 TxDone
*  11--PD7--DIO2  FHSS Changed Channel
*  2--PA5--SCK
*  4--PB7--NSS
*  6--PA7--MOSI
*  8--PA6--MISO
*  24--GND--GND
*  ------------------------
* | PA9  - USART1(Tx)      |
* | PA10 - USART1(Rx)      |
*  ------------------------    

*******************************************************************************/
 
#include "sys_config.h"

#define BUFFER_SIZE     80                          // Define the payload size here

static uint16_t BufferSize = BUFFER_SIZE;			// RF buffer size
static uint8_t  Buffer[BUFFER_SIZE];				// RF buffer

//static uint8_t EnableMaster = true; 				// Master/Slave selection

tRadioDriver *Radio = NULL;

u16 len;//��¼���ݳ���


void OnMaster( void );
//void OnSlave( void );
/*
 * Manages the master operation
 */
void OnMaster( void )
{
    uint8_t i;
	switch( Radio->Process( ) )
    {
    case RF_RX_TIMEOUT:
				if(USART_RX_STA&0x8000)//bit15Ϊ1���������
				{					   
					while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
					len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���,bit14��bit15���Ǳ�־λ�������ڼ���			
					Radio->SetTxPacket( USART_RX_BUF, len );   //RFLR_STATE_TX_INIT������ط����������strlen(USART_RX_BUF)�������⣬Ӧ��strlen���ִ���
					USART_RX_STA=0;
				}
				break;
    case RF_RX_DONE:
        Radio->GetRxPacket( Buffer, ( uint16_t* )&BufferSize );
		    if( BufferSize > 0 )
        {
					for(i=0;i<BufferSize;i++)
					{
						USART_putchar(USART1,Buffer[i]);
					}
					printf("\n");
				}
				break;
    case RF_TX_DONE:
        // Indicates on a LED that we have sent a PING
				LED0_TOGGLE;
        Radio->StartRx( );   //RFLR_STATE_RX_INIT
        break;
    default:
        break;
    }
}


/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{	
	//stm32 config
	sys_Configuration();
	//printf("RF_LoRa_TX_OK�� \n");
	BoardInit( );
    
    Radio = RadioDriverInit( );
    
    Radio->Init( );
	
    Radio->StartRx( );   //RFLR_STATE_RX_INIT
	//Radio->SetTxPacket( MY_TEST_Msg, 18 );
	
	while(1)
	{		
		OnMaster();	
	}
}



/*********************************************END OF FILE**********************/
