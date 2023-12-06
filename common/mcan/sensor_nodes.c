#include <stdint.h> 

#include "mcan.h"
#include "tx_api.h"
#include "sensor_nodes.h"

// Static variables
MCAN_DEV _rxDevice;
static uint16_t _nodePeriod_MS;
static void (*_sensorFunc)(uint8_t *sensorData);
static SENSOR_NODE_EN _nodeEn;
static uint8_t sensorData[8];

// Threads
#define THREAD_SENSOR_NODE_STACK_SIZE 2048
static TX_THREAD stThreadSensorNode;
static uint8_t auThreadSensorNodeStack[THREAD_SENSOR_NODE_STACK_SIZE];
static const uint16_t NODE_THREAD_DELAY_MS = 5;
void thread_sensor_node(ULONG ctx);

void SensorNodeRegister( MCAN_DEV rxDevice, uint16_t nodePeriod_MS, void (*sensorFunc)(uint8_t *sensorData), SENSOR_NODE_EN nodeEn)
{
    // Register device;
    _rxDevice = rxDevice;

    // Register period to delay between sample sends
    _nodePeriod_MS = nodePeriod_MS;

    // Register sensor function;
    _sensorFunc = sensorFunc;

    // Set node state
    _nodeEn = nodeEn;

    tx_thread_create( &stThreadSensorNode, 
        "thread_sensor_node", 
        thread_sensor_node, 
        0, 
        auThreadSensorNodeStack, 
        THREAD_SENSOR_NODE_STACK_SIZE, 
        3,
        3, 
        0,  
        TX_AUTO_START);
}

void SensorNodeEnable(void)
{
    _nodeEn = SENSOR_NODE_ENABLE;
}

void SensorNodeDisable(void)
{
    _nodeEn = SENSOR_NODE_DISABLE;
}

void thread_sensor_node(ULONG ctx)
{
    while(true)
    {
        // Loiter while disabled
        while(_nodeEn == SENSOR_NODE_DISABLE)
        {
            tx_thread_sleep(NODE_THREAD_DELAY_MS);
        }

        // Popuate sensor data array by calling the registered func
        _sensorFunc(sensorData);

        // Transmit the sensor data
        MCAN_TX(PRI_DEBUG, CAT_SENSOR_NODE, _rxDevice, sensorData);

        // Wait for registered period
        tx_thread_sleep(_nodePeriod_MS);
    }
}