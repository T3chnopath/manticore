#include "stm32h5xx_hal.h"
#include "mcan.h"

volatile uint16_t MCAN_TimeStamp = 0;

static FDCAN_HandleTypeDef *_hfdcan;
static sMCAN_Message* _mcanRxMessage;

static bool _MCAN_ConfigInterface ( FDCAN_INTERFACE eInterface );
static bool _MCAN_ConfigFilter( MCAN_DEV currentDevice);
static inline void _MCAN_Conv_ID_To_Uint32( sMCAN_ID* mcanID, uint32_t* uIdentifier );
static inline void _MCAN_Conv_Uint32_To_ID( uint32_t uIdentifier, sMCAN_ID* mcanID);

typedef enum {
    mMCAN_MessageStamp = ( 0x7FFF << 0  ), 
    mMCAN_Priority     = (   0x07 << 15 ), 
    mMCAN_RxDevice     = (   0x0F << 16 ),
    mMCAN_TxDevice     = (   0x0F << 20 ),
    mMCAN_Type         = (   0x07 << 24 ),
} MCAN_ID_MASK;

/*
    Static Function Declarations
*/
bool _MCAN_ConfigInterface( FDCAN_INTERFACE eInterface )
{
    FDCAN_GlobalTypeDef* FDCAN_Instance;

    switch( eInterface )
    {
        #ifdef FDCAN1 
        case FDCAN1_PERIPH:
            FDCAN_Instance = FDCAN1;
            break;
        #endif

        #ifdef FDCAN2
        case FDCAN2_PERIPH:
            FDCAN_Instance = FDCAN2;
            break;
        #endif

        #if ( !defined(FDCAN1) && !defined(FDCAN2) )
            #error "No FDCAN peripheral is defined!"
        #endif

        default:
            FDCAN_Instance = FDCAN1;
            break;
    }

    _hfdcan->Instance = FDCAN_Instance;
    _hfdcan->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    _hfdcan->Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
    _hfdcan->Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK;
    _hfdcan->Init.AutoRetransmission = ENABLE;
    _hfdcan->Init.TransmitPause = DISABLE;
    _hfdcan->Init.ProtocolException = DISABLE;
    _hfdcan->Init.NominalPrescaler = 1;
    _hfdcan->Init.NominalSyncJumpWidth = 13;
    _hfdcan->Init.NominalTimeSeg1 = 86;
    _hfdcan->Init.NominalTimeSeg2 = 13;
    _hfdcan->Init.DataPrescaler = 25;
    _hfdcan->Init.DataSyncJumpWidth = 1;
    _hfdcan->Init.DataTimeSeg1 = 2;
    _hfdcan->Init.DataTimeSeg2 = 1;
    _hfdcan->Init.StdFiltersNbr = 0;
    _hfdcan->Init.ExtFiltersNbr = 0;
    _hfdcan->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

    if (HAL_FDCAN_Init(_hfdcan) != HAL_OK)
    {
        return false;
    }

    return true;
}

bool _MCAN_ConfigFilter( MCAN_DEV currentDevice)
{
    FDCAN_FilterTypeDef sFilterConfig =
    {
        .IdType        = FDCAN_EXTENDED_ID,
        .FilterIndex   = 0,
        .FilterType    = FDCAN_FILTER_MASK,
        .FilterConfig  = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1     = currentDevice || ALL_DEVICES,  // Filter will trigger if intended for current or all devices
        .FilterID2     = mMCAN_RxDevice,                // Mask receive device
    };

    if ( HAL_FDCAN_ConfigFilter(_hfdcan, &sFilterConfig) != HAL_OK )
    {
        return false;
    }

    return true; 
}

inline void _MCAN_Conv_Uint32_To_ID(uint32_t uIdentifier, sMCAN_ID* mcanID )
{
    mcanID->MCAN_TIME_STAMP = ( uIdentifier & mMCAN_MessageStamp );
    mcanID->MCAN_PRIORITY  = ( uIdentifier & mMCAN_Priority );
    mcanID->MCAN_RX_Device = ( uIdentifier & mMCAN_RxDevice );
    mcanID->MCAN_TX_Device = ( uIdentifier & mMCAN_TxDevice );
    mcanID->MCAN_TYPE      = ( uIdentifier & mMCAN_Type );
}

inline void _MCAN_Conv_ID_To_Uint32( sMCAN_ID* mcanID, uint32_t* uIdentifier )
{
    *uIdentifier = 0;
    *uIdentifier = (*uIdentifier & mMCAN_MessageStamp) | mcanID->MCAN_TIME_STAMP;
    *uIdentifier = (*uIdentifier & mMCAN_Priority    ) | mcanID->MCAN_PRIORITY;
    *uIdentifier = (*uIdentifier & mMCAN_RxDevice    ) | mcanID->MCAN_RX_Device;
    *uIdentifier = (*uIdentifier & mMCAN_TxDevice    ) | mcanID->MCAN_TX_Device;
    *uIdentifier = (*uIdentifier & mMCAN_Type        ) | mcanID->MCAN_TYPE;
}



/*
    Public Function Declarations
*/
bool MCAN_PeriphConfig( FDCAN_INTERFACE eInterface, MCAN_DEV currentDevice )
{
  
    if ( !_MCAN_ConfigInterface( eInterface ) )
    {
        return false;
    }

    if ( !_MCAN_ConfigFilter( currentDevice ) )
    {
        return false;
    }

   return true;
}

void MCAN_RegisterRX( sMCAN_Message* mcanRxMessage )
{
    _mcanRxMessage = mcanRxMessage;
} 

bool MCAN_StartRX_IT( void )
{
    if( HAL_FDCAN_Start( _hfdcan ) != HAL_OK)
    {
        return false;
    }

    if ( HAL_FDCAN_ActivateNotification( _hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 ) != HAL_OK)
    {
        return false;
    }
    
    return true;
}

// Rx Handler to be overwritten by the caller
__weak bool MCAN_Rx_Handler()
{
    return true;
}

bool MCAN_TX( sMCAN_Message* mcanTxMessage )
{
    // Interpret 32 bit idenfitier from MCAN message struct ID
    uint32_t uIdentifier = 0;
    _MCAN_Conv_ID_To_Uint32(mcanTxMessage->mcanID, &uIdentifier);

    // Format header: assume 64 byte CANFD with flexible data rate
    FDCAN_TxHeaderTypeDef TxHeader = {
        .Identifier = uIdentifier,
        .IdType = FDCAN_EXTENDED_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = FDCAN_DLC_BYTES_64,
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_FD_CAN,
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
        .MessageMarker = 0,
    };

    // Add frame to TX FIFO -> Transmit
    if( HAL_FDCAN_AddMessageToTxFifoQ(_hfdcan, &TxHeader, mcanTxMessage->mcanData ) != HAL_OK )
    {
        return false;
    }

    return true;
}

FDCAN_HandleTypeDef* MCAN_GetFDCAN_Handle( void )
{
    return _hfdcan;
}

// Weak callback overriden for MCAN
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    // Allocate Rx Header to be populated with message data
    FDCAN_RxHeaderTypeDef _RxHeader = { 0 };
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        // Populate header and MCAN data 
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &_RxHeader, _mcanRxMessage->mcanData) != HAL_OK)
        {
        /* Reception Error */
        }

        // Enable interrupts
        if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
        {
        /* Notification Error */
        }

        // Interpret ID for MCAN struct from uint32 identifier
        _MCAN_Conv_Uint32_To_ID(_RxHeader.Identifier, _mcanRxMessage->mcanID);
        MCAN_Rx_Handler();
    }
}
