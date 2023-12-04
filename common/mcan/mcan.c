#include <string.h>

#include "stm32h5xx_hal.h"

#if defined(STM32H503)
#include "stm32h503xx.h"
#elif defined(STM32H563)
#include "stm32h563xx.h"
#endif

#include "tx_api.h"
#include "mcan.h"

#define UINT12_MAX 4096
#define MCAN_QUEUE_SIZE 20
#define MCAN_PRI_COUNT 4

/********** Static Data Structures ********/
typedef enum {
    kMCAN_SHIFT_Priority  = 27,
    kMCAN_SHIFT_Cat       = 24,
    kMCAN_SHIFT_RxDevice  = 18,
    kMCAN_SHIFT_TxDevice  = 12,
    kMCAN_SHIFT_TimeStamp = 0,
} MCAN_ID_SHIFTS;

typedef enum {
    mMCAN_Priority  = 0x0003 << kMCAN_SHIFT_Priority, 
    mMCAN_Cat       = 0x0007 << kMCAN_SHIFT_Cat,
    mMCAN_RxDevice  = 0x003F << kMCAN_SHIFT_RxDevice,
    mMCAN_TxDevice  = 0x003F << kMCAN_SHIFT_TxDevice,
    mMCAN_TimeStamp = 0x0FFF << kMCAN_SHIFT_TimeStamp,
} MCAN_ID_MASK;

typedef struct {
    sMCAN_Message array[MCAN_QUEUE_SIZE];
    uint8_t front;
    uint8_t rear;
    uint8_t size;
} MCAN_Queue;

// Priority queue holds 4 buckets organized by priority
typedef struct {
    MCAN_Queue queues[MCAN_PRI_COUNT];
} MCAN_PriQueue;

/********** Static Variables ********/
static const uint8_t MCAN_MAX_FILTERS = 2;
static MCAN_DEV _mcanCurrentDevice;
static FDCAN_HandleTypeDef _hfdcan ;
static TX_MUTEX mcanTxMutex;
static sMCAN_Message* _mcanRxMessage;
static uint8_t* heartbeatDataBuf;

// Queue Variables
static MCAN_PriQueue _mcanPriQueue;

// Thread Variables
#define THREAD_HEARTBEAT_STACK_SIZE 256
static TX_THREAD stThreadHeartbeat;
static uint8_t auThreadHeartbeatStack[THREAD_HEARTBEAT_STACK_SIZE];
static uint32_t heartbeatPeriod;


/********** Static Function Declarations ********/
static bool _MCAN_ConfigInterface ( FDCAN_GlobalTypeDef* FDCAN_Instance );
static bool _MCAN_ConfigFilter( MCAN_DEV mcanRxFilter );
static inline void _MCAN_Conv_ID_To_Uint32( sMCAN_ID* mcanID, uint32_t* uIdentifier );
static inline void _MCAN_Conv_Uint32_To_ID( uint32_t uIdentifier, sMCAN_ID* mcanID);
static uint16_t _MCAN_GetTimestamp( void );

// Queue Functions
void _MCAN_QueueInit(MCAN_Queue *q);
void _MCAN_QueueEmpty(MCAN_Queue *q);
void _MCAN_QueueFull(MCAN_Queue *q);
void _MCAN_Enqueue(MCAN_Queue, sMCAN_Message message);
sMCAN_Message _MCAN_Dequeue(MCAN_Queue *q);

void _MCAN_PriQueueInit(void);
void _MCAN_PriEnqueue(sMCAN_Message mcanMessage);
sMCAN_Message _MCAN_Dequeue(void);

// Threads 
static void thread_heartbeat( ULONG ctx );


/***************************** Static Function Definitions *****************************/

/************************************************************
    Name: _MCAN_ConfigInterface
    
    Description:
        Configures the FDCAN interface based on input. Assumes
        all modules use the same clockspeed, resulting in the
        same time quanta config. Assumes FD in BRS mode.

    Arguments:
        eInterface = FDCAN interface that is selected ( 1 or 2 )

    Returns:
        True  = succesful config init
        False = config failure 
*************************************************************/
static bool _MCAN_ConfigInterface( FDCAN_GlobalTypeDef* FDCAN_Instance )
{
    // Configure for no BRS, 1MHz Nominal and 1MHz Data
    _hfdcan.Instance = FDCAN_Instance;
    _hfdcan.Init.ClockDivider = FDCAN_CLOCK_DIV1;
    _hfdcan.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
    _hfdcan.Init.Mode = FDCAN_MODE_NORMAL;
    _hfdcan.Init.AutoRetransmission = ENABLE;
    _hfdcan.Init.TransmitPause = DISABLE;
    _hfdcan.Init.ProtocolException = DISABLE;
    _hfdcan.Init.NominalPrescaler = 1;
    _hfdcan.Init.NominalSyncJumpWidth = 2;
    _hfdcan.Init.NominalTimeSeg1 = 29;
    _hfdcan.Init.NominalTimeSeg2 = 2;
    _hfdcan.Init.DataPrescaler = 1;
    _hfdcan.Init.DataSyncJumpWidth = 15;
    _hfdcan.Init.DataTimeSeg1 = 16;
    _hfdcan.Init.DataTimeSeg2 = 15;
    _hfdcan.Init.StdFiltersNbr = 0;
    _hfdcan.Init.ExtFiltersNbr = MCAN_MAX_FILTERS;
    _hfdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
 
 if (HAL_FDCAN_Init(&_hfdcan) != HAL_OK)
    {
        return false;
    }

    return true;
}

/********************************************************************
    Name: _MCAN_ConfigFilter
    
    Description:
        Filters CAN messages based on the current device.
        Does not consider priority, or messages destined
        for other devices. Exception is the ALL_DEVICES
        selection. Assumes Extended CAN2.0B style IDs.

    Arguments:
        rxDevice = current module expecting reception

    Returns:
        True  = succesful config init
        False = config failure 
*********************************************************************/
static bool _MCAN_ConfigFilter( MCAN_DEV mcanRxFilter )
{
    uint8_t filterIndex = 0;
    FDCAN_FilterTypeDef sFilterConfig =
    {
        .IdType        = FDCAN_EXTENDED_ID,
        .FilterType    = FDCAN_FILTER_MASK,
        .FilterConfig  = FDCAN_FILTER_TO_RXFIFO0,
    };

    // Config global filters to reject incorrect IDs
    if ( HAL_FDCAN_ConfigGlobalFilter(&_hfdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK )
    {
        return false;
    }

    // Bitmask to configure selected filters
    for(MCAN_DEV dev = DEV_POWER; dev <= DEV_DEBUG; dev = dev << 1)
    {
        if (mcanRxFilter& dev)
        {
            sFilterConfig.FilterIndex = filterIndex++;
            sFilterConfig.FilterID1   = mcanRxFilter << kMCAN_SHIFT_RxDevice;

            if ( HAL_FDCAN_ConfigFilter(&_hfdcan, &sFilterConfig) != HAL_OK )
            {
                return false;
            }
        }
    }

    return true; 
}

/*********************************************************************************
    Name: _MCAN_Conv_Uint32_To_ID
    
    Description:
        Helper to convert uint32 style HAL identifiers to MCAN identifiers.

    Arguments:
        uIdentifier = identifier from HAL FDCAN API
        mcanID      = pointer to an MCAN style ID where conversion is stored

    Returns:
        None
***********************************************************************************/
static inline void _MCAN_Conv_Uint32_To_ID(uint32_t uIdentifier, sMCAN_ID* mcanID )
{
    mcanID->MCAN_PRIORITY  |= (uIdentifier & mMCAN_Priority)  >> kMCAN_SHIFT_Priority;
    mcanID->MCAN_CAT       |= (uIdentifier & mMCAN_Cat)       >> kMCAN_SHIFT_Cat;
    mcanID->MCAN_RX_Device |= (uIdentifier & mMCAN_RxDevice)  >> kMCAN_SHIFT_RxDevice;
    mcanID->MCAN_TX_Device |= (uIdentifier & mMCAN_TxDevice)  >> kMCAN_SHIFT_TxDevice;
    mcanID->MCAN_TimeStamp |= (uIdentifier & mMCAN_TimeStamp) >> kMCAN_SHIFT_TimeStamp;
}

/*********************************************************************************
    Name: _MCAN_Conv_ID_To_Uint32
    
    Description:
        Helper to convert MCAN identifiers to uint32 style HAL identifiers,

    Arguments:
        mcanID      = pointer to an MCAN style ID, 
        uIdentifier = pointer to identifier from HAL FDCAN API where
                      conversion is stored
        
    Returns:
        None
***********************************************************************************/
static inline void _MCAN_Conv_ID_To_Uint32( sMCAN_ID* mcanID, uint32_t* uIdentifier )
{
    *uIdentifier = 0;
    *uIdentifier |= (mcanID->MCAN_PRIORITY  << kMCAN_SHIFT_Priority);
    *uIdentifier |= (mcanID->MCAN_CAT       << kMCAN_SHIFT_Cat);
    *uIdentifier |= (mcanID->MCAN_RX_Device << kMCAN_SHIFT_RxDevice);
    *uIdentifier |= (mcanID->MCAN_TX_Device << kMCAN_SHIFT_TxDevice);
    *uIdentifier |= (mcanID->MCAN_TimeStamp << kMCAN_SHIFT_TimeStamp);
}

static inline uint16_t _MCAN_GetTimestamp( void )
{
    return (HAL_GetTick() / 1000) % UINT12_MAX;
}


// Queue functions

// Initialize the queue
void _MCAN_QueueInit(MCAN_Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

// Check if the queue is empty
bool _MCAN_QueueEmpty(MCAN_Queue *q) {
    return q->size == 0;
}

// Check if the queue is full
bool _MCAN_QueueFull(MCAN_Queue *q) {
    return q->size == MCAN_QUEUE_SIZE;
}

// Enqueue an element
void _MCAN_Enqueue(MCAN_Queue *q, sMCAN_Message message) {
    if (MCAN_QueueFull(q)) {
        return;
    }
    
    q->array[q->rear] = message;
    q->rear = (q->rear + 1) % MCAN_QUEUE_SIZE;
    q->size++;
}

// Dequeue an element
sMCAN_Message _MCAN_Dequeue(MCAN_Queue *q) {
    if (MCAN_QueueEmpty(q)) {
        sMCAN_Message empty = {0}; // Initialize to your default empty value
        return empty;
    }
    sMCAN_Message element = q->array[q->front];
    q->front = (q->front + 1) % MCAN_QUEUE_SIZE;
    q->size--;
    return element;
}


// Priority Queue Functions
void _MCAN_PriQueueInit(void) {
    for (int i = 0; i < MCAN_PRI_COUNT; i++) {
        _MCAN_QueueInit(&_mcanPriQueue->queues[i]);
    }
}

// Enqueue an element into the correct queue based on priority
void _MCAN_PriEnqueue(sMCAN_Message message) {
    MCAN_PRI pri = message.mcanID.MCAN_PRIORITY; 

    // Check priority bounds 
    if (pri < MCAN_EMERGENCY || pri > MCAN_DEBUG) 
    {
        return; // Invalid priority
    }

    // Insert message into appropriate queue 
    _MCAN_Enqueue(_mcanPriQueue->queues[pri], message);
}

// Dequeue an element, starting from the highest priority
sMCAN_Message _MCAN_PriDequeue(void) {

    // Iterate through queues in order of priority 
    for (uint8_t i = 0; i < MCAN_PRI_COUNT; i++) 
    {
        // If queue is not empty, return
        if (!MCAN_QueueEmpty(_mcanPriQueue->queues[i])) 
        {
            return MCAN_Dequeue(_mcanPriQueue->queues[i]);
        }
    }

    // If all queues are empty, return
    sMCAN_Message empty = {0}; 
    return empty;
}


/***************************** Public Function Definitions *****************************/

/*********************************************************************************
    Name: MCAN_Init
    
    Description:
        Configure FDCAN interface and filtering. Caller is fully responsible for 
        configuring and enabling the FDCAN interface that is passed.

        NOTE: MCAN Does not support registering two interfaces currently. 

    Arguments:
        FDCAN_Instance = pointer to FDCAN_GlobalTypeDef instance
        rxDevice       = current module expecting reception
        sMCAN_Message  = pointer to MCAN Rx message buffer for reception

    Returns:
        True  = succesful interface and filter configuration
        False = failed interface or filter configuration 
***********************************************************************************/
bool MCAN_Init( FDCAN_GlobalTypeDef* FDCAN_Instance, MCAN_DEV mcanCurrentDevice, MCAN_DEV mcanRxFilter, sMCAN_Message* mcanRxMessage )
{
    _mcanRxMessage = mcanRxMessage;
    _mcanCurrentDevice = mcanCurrentDevice;

    if ( !_MCAN_ConfigInterface( FDCAN_Instance ) )
    {
        return false;
    }

   if ( !_MCAN_ConfigFilter( mcanRxFilter ) )
    {
        return false;
    }

   return true;
}


/*********************************************************************************
    Name: MCAN_StartRX_IT
    
    Description:
        Start the FDCAN interface in interrupt mode, given current configs.
        Will always use RX FIFO0.

    Arguments:
        mcanEnable = enables or disables the interrupt

    Returns:
        True  = succesful activation of peripheral and interrupt notifs
        False = failure to activate peripheral or interrupt notifs
***********************************************************************************/
bool MCAN_SetEnableIT( MCAN_EN mcanEnable )
{
    switch(mcanEnable)
    {
        case MCAN_ENABLE:

            if( HAL_FDCAN_Start( &_hfdcan ) != HAL_OK)
            {
                return false;
            }

            if ( HAL_FDCAN_ActivateNotification( &_hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 ) != HAL_OK)
            {
                return false;
            }

            break;

        case MCAN_DISABLE:
        
            if ( HAL_FDCAN_DeactivateNotification(&_hfdcan, 0) != HAL_OK)
            {
                return false;
            }
            
            break;

        default:
            return false;
    }
    
    return true;
}

/*********************************************************************************
    Name: MCAN_Rx_Handler
    
    Description:
        Weak function to be overriden by a module, that is called in the FDCAN
        ISR Callback.

        Module has full responsibility for RX processing of the buffer provided
        during registration.

    Arguments:
        None

    Returns:
        None
***********************************************************************************/
__weak void MCAN_Rx_Handler()
{
    // NOP
}

/*********************************************************************************
    Name: MCAN_TX
    
    Description:
        Transmit an MCAN message, given an MCAN message struct. Struct timestamp
        is always overwritten.

    Arguments:
        mcanTxMessage = pointer to an MCAN message struct which specifies message
                        data and MCAN ID configs. 

    Returns:
        True  = successful transmission of message
        False = failed tranmission of message
***********************************************************************************/
bool MCAN_TX( MCAN_PRI mcanPri, MCAN_CAT mcanType, MCAN_DEV mcanRxDevice, uint8_t mcanData[64])
{
    int status = 0;
    sMCAN_ID mcanID = {
            .MCAN_PRIORITY = mcanPri,
            .MCAN_CAT = mcanType, 
            .MCAN_RX_Device = mcanRxDevice,
            .MCAN_TX_Device = _mcanCurrentDevice,
            .MCAN_TimeStamp = _MCAN_GetTimestamp(),
    };
    
    // Interpret 32 bit idenfitier from MCAN message struct ID
    static uint32_t uIdentifier;
    _MCAN_Conv_ID_To_Uint32(&mcanID, &uIdentifier);

    // Format header: assume 64 byte CANFD with flexible data rate
    FDCAN_TxHeaderTypeDef TxHeader = {
        .Identifier = uIdentifier,
        .IdType = FDCAN_EXTENDED_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = FDCAN_DLC_BYTES_64,
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_CLASSIC_CAN,
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
        .MessageMarker = 0,
    };

    // Add frame to TX FIFO -> Transmit
    tx_mutex_get(&mcanTxMutex, TX_WAIT_FOREVER); // enter critical section, suspend if mutex is locked
    status = HAL_FDCAN_AddMessageToTxFifoQ(&_hfdcan, &TxHeader, mcanData );
    tx_mutex_put(&mcanTxMutex);                  // exit critical section

    if (status == HAL_OK)
    {
        return true;
    }

    return false;
}


/********************************************************************************
 //TODO DOCUMENTATION UPDATE
*/
void MCAN_EnableHeartBeats( uint32_t delay, uint8_t* heartbeatData )
{
    static bool heartBeatThreadCreated = false;
    heartbeatDataBuf = heartbeatData;
    heartbeatPeriod = delay;

    // If first time calling, create the thread but don't call it
    if ( !heartBeatThreadCreated )
    {
        tx_thread_create( &stThreadHeartbeat, 
                    "thread_heartbeat", 
                    thread_heartbeat, 
                    0, 
                    auThreadHeartbeatStack, 
                    THREAD_HEARTBEAT_STACK_SIZE, 
                    6,
                    6, 
                    0, // Time slicing unused if all threads have unique priorities     
                    TX_DONT_START); 
        heartBeatThreadCreated = true;
    }

    // Enable the heartbeat thread
    tx_thread_resume( &stThreadHeartbeat );
}

void MCAN_DisableHeartBeats( void )
{
    tx_thread_suspend( &stThreadHeartbeat );
}

/*********************************************************************************
    Name: MCAN_GetFDCAN_Handler 
    
    Description:
        Simple getter which returns the applicable FDCAN_HandleTypeDef of
        the selected interface.

        Used solely for context in the ISR processing.

    Arguments:
        None

    Returns:
        A pointer to the FDCAN_HandleTypeDef.
***********************************************************************************/
FDCAN_HandleTypeDef* MCAN_GetFDCAN_Handle( void )
{
    return &_hfdcan;
}


/***************************** External Overrides *****************************/

/*********************************************************************************
    Name: HAL_FDCAN_RxFifo0Callback
    
    Description:
        HAL Callback that is overriden to update the register MCAN Rx buf.

    Arguments:
        hfdcan     = pointer to an FDCAN_HandleTypeDef, handled by ISR context
        RxFifoOITs = used for interrupt configuration, handled by ISR context
    
    Returns:
        None
***********************************************************************************/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    // Allocate Rx Header to be populated with message data
    FDCAN_RxHeaderTypeDef _RxHeader = { 0 };
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        // Populate header and MCAN data 
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &_RxHeader, _mcanRxMessage->mcanData ) != HAL_OK)
        {
        /* Reception Error */
        }

        // Insert ID and timestamp into the message
        _MCAN_Conv_Uint32_To_ID(_RxHeader.Identifier, &_mcanRxMessage->mcanID);
        _mcanRxMessage->mcanID.MCAN_TimeStamp= _MCAN_GetTimestamp();

        // Add message to queue
        _MCAN_PriEnqueue(_mcanRxMessage);

        // Signal queue processing thread

        // Enable interrupts to receive new messages
        if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
        {
        /* Notification Error */
        }
    }
}

/***************************** Threads *****************************/
void thread_heartbeat(ULONG ctx)
{
    while( true )
    {
       MCAN_TX( MCAN_DEBUG, HEARTBEAT, _mcanCurrentDevice, heartbeatDataBuf);
       tx_thread_sleep(heartbeatPeriod);
    }
}
