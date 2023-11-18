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
    COMMAND,
    RESPONSE,
    VEHICLE_STATE,
    SENSOR_DATA,
    LOG,
    HEARTBEAT,
} MCAN_CAT;

typedef enum {
    DEV_POWER,
    DEV_MAIN_COMPUTE,
    DEV_DEPLOYMENT,
    DEV_MIO,
    DEV_MTUSC,
    DEV_ALL,
    DEV_HEARTBEAT
} MCAN_DEV;

typedef enum {
    MCAN_EMERGENCY,
    MCAN_ERROR,
    MCAN_WARNING,
    MCAN_DEBUG,
} MCAN_PRI;

typedef struct{
    MCAN_CAT MCAN_CAT;
    MCAN_DEV MCAN_TX_Device; // Device that is sending
    MCAN_DEV MCAN_RX_Device; // Device that is receiving, to be received
    MCAN_PRI MCAN_PRIORITY;
    uint32_t MCAN_TIME_STAMP;
} sMCAN_ID;

typedef struct
{
    sMCAN_ID mcanID;
    uint8_t mcanData[64];
} sMCAN_Message;

// Caller must provide bufferssS for Rx and Tx.

// sMCAN_Message struct, provided by the caller, is populated with Rx content upon ISR firing
bool MCAN_Init( FDCAN_GlobalTypeDef* FDCAN_Instance, MCAN_DEV currentDevice, sMCAN_Message* mcanRxMessage);

bool MCAN_SetEnableIT( MCAN_EN mcanEnable );
__weak void MCAN_RX_Handler( void ); // Called by ISR 

bool MCAN_TX( MCAN_PRI mcanPri, MCAN_CAT mcanType, MCAN_DEV mcanRxDevice, uint8_t mcanData[64] );

void MCAN_EnableHeartBeats( uint32_t delay, uint8_t* heartbeatData);
void MCAN_DisableHeartBeats( void );

FDCAN_HandleTypeDef* MCAN_GetFDCAN_Handle( void );

#endif /* __MCAN_H */