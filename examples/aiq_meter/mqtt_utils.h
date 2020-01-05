// MQTT functions header
#ifndef __MQTT_UTILS_H__
#define __MQTT_UTILS_H__

#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>

#include <stdio.h>
#include <string.h>

#include <semphr.h>

#include "utils.h"

#define MQTT_HOST ("mqtt.beebotte.com")
#define MQTT_PORT 1883

#define MQTT_USER ("token_jxpORc4oWm878pUt")
#define MQTT_PASS NULL

#define PUB_MSG_LEN 64

void topic_received(mqtt_message_data_t *md);
mqtt_message_data_t *create_mqtt_msg(char *topic, void *payload, size_t payloadlen);
void delete_mqtt_msg(mqtt_message_data_t *mqtt_message_data);
void mqtt_task(void *pvParameters);

#endif
