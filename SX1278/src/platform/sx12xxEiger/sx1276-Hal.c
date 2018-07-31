/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1276-Hal.c
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include <stdint.h>
#include <stdbool.h> 

#include "platform.h"

#if defined( USE_SX1276_RADIO )

#include "spi.h"
#include "../../radio/sx1276-Hal.h"

void SX1276InitIo( void )
{
    // 配置 NSS DIO REST引脚
}

void SX1276SetReset( uint8_t state )
{
    // 控制 REST 引脚
}

void SX1276Write( uint8_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

void SX1276Read( uint8_t addr, uint8_t *data )
{
    SX1276ReadBuffer( addr, data, 1 );
}

void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_RESET );

    SpiInOut( addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }

    //NSS = 1;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );
}

void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_RESET );

    SpiInOut( addr & 0x7F );

    for( i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }

    //NSS = 1;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );
}

void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1276WriteBuffer( 0, buffer, size );
}

void SX1276ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1276ReadBuffer( 0, buffer, size );
}

// 读中断
inline uint8_t SX1276ReadDio0( void )
{
    return GPIO_ReadInputDataBit( DIO0_IOPORT, DIO0_PIN );
}

// 只有 DIO0 做为中断
inline uint8_t SX1276ReadDio1( void )
{
    return 0;
}

inline uint8_t SX1276ReadDio2( void )
{
    return 0;
}

inline uint8_t SX1276ReadDio3( void )
{
    return 0;
}

inline uint8_t SX1276ReadDio4( void )
{
    return 0;
}

inline uint8_t SX1276ReadDio5( void )
{
    return 0;
}

inline void SX1276WriteRxTx( uint8_t txEnable )
{
    // 控制函数 貌似无用
    // if( txEnable != 0 )
    // {
    //     IoePinOn( FEM_CTX_PIN );
    //     IoePinOff( FEM_CPS_PIN );
    // }
    // else
    // {
    //     IoePinOff( FEM_CTX_PIN );
    //     IoePinOn( FEM_CPS_PIN );
    // }
}

#endif // USE_SX1276_RADIO
