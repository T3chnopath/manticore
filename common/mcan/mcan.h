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
    PRI_EMERGENCY,
    PRI_ERROR,
    PRI_WARNING,
    PRI_DEBUG,
} MCAN_PRI;

typedef enum {
    CAT_COMMAND,
    CAT_RESPONSE,
    CAT_VEHICLE_STATE,
    CAT_SENSOR_NODE,
    CAT_HEARTBEAT,
    CAT_DEBUG,
} MCAN_CAT;

typedef enum {
    DEV_POWER      = 1 << 0,
    DEV_COMPUTE    = 1 << 1,
    DEV_DEPLOYMENT = 1 << 2,
    DEV_MIO        = 1 << 3,
    DEV_MTUSC      = 1 << 4,
    DEV_DEBUG      = 1 << 5,
} MCAN_DEV;
static const MCAN_DEV DEV_ALL = 0x3F;

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
bool MCAN_Init( FDCAN_GlobalTypeDef* FDCAN_Instance, MCAN_DEV mcanRxFilterm, MCAN_EN mcanEnable);

bool MCAN_SetEnableIT( MCAN_EN mcanEnable );

__weak void MCAN_RX_GetLatest( sMCAN_Message mcanRxMessage ); // Get the latest MCAN message in the arg
__weak void MCAN_RX_Handler( sMCAN_Message mcanRxMessage );   // Called by ISR 

bool MCAN_TX_Verbose( MCAN_PRI mcanPri, MCAN_CAT mcanType, MCAN_DEV mcanTxDevice, MCAN_DEV mcanRxDevice, uint8_t mcanData[64] );
bool MCAN_TX( MCAN_PRI mcanPri, MCAN_CAT mcanType, MCAN_DEV mcanRxDevice, uint8_t mcanData[64] );

void MCAN_EnableHeartBeats( uint32_t delay, uint8_t* heartbeatData);
void MCAN_DisableHeartBeats( void );

// Helper function for conversion
void MCAN_Conv_ID_To_Uint32( sMCAN_ID* mcanID, uint32_t* uIdentifier );
void MCAN_Conv_Uint32_To_ID( uint32_t uIdentifier, sMCAN_ID* mcanID);

const char * MCAN_Pri_String( MCAN_PRI priority);
const char * MCAN_Cat_String( MCAN_CAT category);
const char * MCAN_Dev_String( MCAN_DEV device);

FDCAN_HandleTypeDef* MCAN_GetFDCAN_Handle( void );

#endif /* __MCAN_H */