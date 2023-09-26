#ifndef __MCAN_H
#define __MCAN_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32h5xx_hal.h"

typedef enum {
    FDCAN1_PERIPH,
    FDCAN2_PERIPH,
} FDCAN_INTERFACE;

typedef enum {
    POWER,
    MAIN_COMPUTE,
    DEPLOYMENT,
    MIO,
    CAMERA,
    ALL,
} MCAN_DEV;

typedef enum {
    COMMAND,
    RESPONSE,
    VEHICLE_STATE,
    SENSOR_DATA,
    LOG
} MCAN_TYPE;

typedef enum {
    MCAN_EMERGENCY,
    MCAN_ALERT,
    MCAN_CRITICAL,
    MCAN_ERROR,
    MCAN_WARNING,
    MCAN_NOTICE,
    MCAN_INFO,
    MCAN_DEBUG,
} MCAN_PRI;

typedef struct{
    MCAN_TYPE MCAN_TYPE;
    MCAN_DEV MCAN_TX_Device; // Device that is sending
    MCAN_DEV MCAN_RX_Device; // Device that is receiving, to be received
    MCAN_PRI MCAN_PRIORITY;
    uint32_t MCAN_MES_STAMP;
} sMCAN_ID;

typedef struct{
    sMCAN_ID* mcanID;
    uint8_t mcanData[64];
} sMCAN_Message;

// Caller must provide buffers for Rx and Tx.

// sMCAN_Message struct, provided by the caller, is populated with Rx content upon ISR firing
bool MCAN_PeriphConfig( FDCAN_INTERFACE eInterface, uint32_t uFilter );

void MCAN_RegisterRX_Buf( sMCAN_Message* mcanRxMessage ); 
bool MCAN_StartRX_IT( void );
__weak bool MCAN_RX_Handler( void ); // Called by ISR 

bool MCAN_TX( sMCAN_Message* mcanTxMessage );

#endif /* __MCAN_H */