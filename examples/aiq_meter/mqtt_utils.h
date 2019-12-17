// MQTT functions header
#ifndef __MQTT_UTILS_H__
#define __MQTT_UTILS_H__

#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>

#include <stdio.h>
#include <string.h>

#include <semphr.h>

#include "utils.h"

#define MQTT_HOST ("test.mosquitto.org")
#define MQTT_PORT 1883

#define MQTT_USER NULL
#define MQTT_PASS NULL

#define PUB_MSG_LEN 16

void topic_received(mqtt_message_data_t *md);
void mqtt_task(void *pvParameters);

#endif
