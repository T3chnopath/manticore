#ifndef __SENSOR_NODES_H
#define __SENSOR_NODES_H

#include "mcan.h"

typedef enum
{
    SENSOR_NODE_ENABLE,
    SENSOR_NODE_DISABLE,
} SENSOR_NODE_EN;

void SensorNodeRegister( MCAN_DEV rxDevice, uint16_t nodePeriod_MS, void (*sensorFunc)(uint8_t *sensorData), SENSOR_NODE_EN nodeEnable);
void SensorNodeEnable(void);
void SensorNodeDisable(void);

#endif 