#ifndef __MCAN_H
#define __MCAN_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32h5xx_hal.h"

typedef enum {
    MCAN_ENABLE,
    MCAN_DISABLE,
} MCAN_EN;

typedef enum {
    MCAN_EMERGENCY,
    MCAN_ERROR,
    MCAN_WARNING,
    MCAN_DEBUG,
} MCAN_PRI;

typedef enum {
    COMMAND,
    RESPONSE,
    VEHICLE_STATE,
    SENSORNODE,
    HEARTBEAT,
    DEBUG,
} MCAN_CAT;

typedef enum {
    DEV_POWER      = 1 << 0,
    DEV_COMPUTE    = 1 << 1,
    DEV_DEPLOYMENT = 1 << 2,
    DEV_MIO        = 1 << 3,
    DEV_MTUSC      = 1 << 4,
    DEV_DEBUG      = 1 << 5,
} MCAN_DEV;
static const MCAN_DEV ALL_DEVICES = 0x3F;

typedef struct{
    MCAN_PRI MCAN_PRIORITY;
    MCAN_CAT MCAN_CAT;
    MCAN_DEV MCAN_RX_Device; // Device that is receiving, to be received
    MCAN_DEV MCAN_TX_Device; // Device that is sending
    uint16_t MCAN_TimeStamp;
} sMCAN_ID;

typedef struct
{
    sMCAN_ID mcanID;
    uint8_t mcanData[64];
} sMCAN_Message;

// User can bitwise OR to configure device filter.
bool MCAN_Init( FDCAN_GlobalTypeDef* FDCAN_Instance, MCAN_DEV mcanRxFilter);

bool MCAN_SetEnableIT( MCAN_EN mcanEnable );
__weak void MCAN_RX_Handler( sMCAN_Message mcanRxMessage ); // Called by ISR 

bool MCAN_TX_Verbose( MCAN_PRI mcanPri, MCAN_CAT mcanType, MCAN_DEV mcanTxDevice, MCAN_DEV mcanRxDevice, uint8_t mcanData[64] );
bool MCAN_TX( MCAN_PRI mcanPri, MCAN_CAT mcanType, MCAN_DEV mcanRxDevice, uint8_t mcanData[64] );

void MCAN_EnableHeartBeats( uint32_t delay, uint8_t* heartbeatData);
void MCAN_DisableHeartBeats( void );

FDCAN_HandleTypeDef* MCAN_GetFDCAN_Handle( void );

#endif /* __MCAN_H */