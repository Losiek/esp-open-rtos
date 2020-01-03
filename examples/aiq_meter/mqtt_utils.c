// MQTT functions
#include "mqtt_utils.h"
#include "dbg.h"

#undef NDEBUG

QueueHandle_t publish_queue;

void mqtt_beat_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char msg[PUB_MSG_LEN];
    memset(msg, 0, sizeof(msg));
    uint32_t beat_cnt = 0;
    mqtt_message_data_t *mqtt_msg_p = create_mqtt_msg("aiq_meter/log", (void*)&msg, PUB_MSG_LEN);

    while(1) {
        ++beat_cnt;
        sprintf((char*)&msg, "[%010d] %s [%d]", xTaskGetTickCount(), __func__, beat_cnt);
        xQueueSend(publish_queue, (void*)&mqtt_msg_p, 0);
        vTaskDelayUntil(&xLastWakeTime, 60*1000/portTICK_PERIOD_MS);
    }
    delete_mqtt_msg(mqtt_msg_p);
}

void topic_received(mqtt_message_data_t *md)
{
    int i;
    mqtt_message_t *message = md->message;
    printf("Received: ");
    for(i = 0; i < md->topic->lenstring.len; ++i) {
        printf("%c", md->topic->lenstring.data[i]);
    }
    printf(" = ");
    for(i = 0; i < (int)message->payloadlen; ++i) {
        printf("%c", ((char* )(message->payloadlen))[i]);
    }
    printf("\r\n");
}

mqtt_message_data_t *create_mqtt_msg(char *topic, void *payload, size_t payloadlen)
{
    mqtt_string_t        *mqtt_topic_str;
    mqtt_message_data_t  *mqtt_message_data;
    mqtt_message_t       *mqtt_message;

    mqtt_topic_str      = (mqtt_string_t*)malloc(sizeof(mqtt_string_t));
    mqtt_message_data   = (mqtt_message_data_t*)malloc(sizeof(mqtt_message_handler_t));
    mqtt_message        = (mqtt_message_t*)malloc(sizeof(mqtt_message_t));

    check_mem(mqtt_topic_str)
    check_mem(mqtt_message_data)
    check_mem(mqtt_message)

    mqtt_topic_str->lenstring.len = 0;
    mqtt_topic_str->lenstring.data = NULL;
    mqtt_topic_str->cstring = (char*)topic;

    mqtt_message->payload = payload;
    mqtt_message->payloadlen = payloadlen;
    mqtt_message->dup = 0;
    mqtt_message->qos = MQTT_QOS1;
    mqtt_message->retained = 0;

    mqtt_message_data->topic = mqtt_topic_str;
    mqtt_message_data->message = mqtt_message;

    return mqtt_message_data;

error:
    return NULL;
}

void delete_mqtt_msg(mqtt_message_data_t *mqtt_message_data)
{
    free(mqtt_message_data->message);
    free(mqtt_message_data);
}

void mqtt_task(void *pvParameters)
{
    SemaphoreHandle_t *wifi_alive = (SemaphoreHandle_t*)pvParameters;
    publish_queue = xQueueCreate(32, sizeof(mqtt_message_data_t*));

    int ret = 0;
    struct mqtt_network network;
    mqtt_client_t client = mqtt_client_default;

    char mqtt_client_id[20];
    uint8_t mqtt_buf[100];
    uint8_t mqtt_readbuf[100];
    mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;

    mqtt_network_new(&network);
    memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
    strcpy(mqtt_client_id, "AIQM-");
    strcat(mqtt_client_id, get_my_id());

    xTaskCreate(&mqtt_beat_task, "mqtt_beat_task", 256, NULL, 3, NULL);

    while(1) {
        xQueueReset(publish_queue);
        xSemaphoreTake(*wifi_alive, portMAX_DELAY);
        printf("%s: started\n\r", __func__);
        printf("%s: (Re)connecting to MQTT server %s ... ", __func__,
                MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
        if(ret) {
            printf("error: %d\n\r", ret);
            taskYIELD();
            continue;
        }
        printf("done\n\r");
        mqtt_client_new(&client, &network, 5000, mqtt_buf, 100,
                        mqtt_readbuf, 100);

        data.willFlag = 0;
        data.MQTTVersion = 3;
        data.clientID.cstring = mqtt_client_id;
        data.username.cstring = MQTT_USER;
        data.password.cstring = MQTT_PASS;
        data.keepAliveInterval = 10;
        data.cleansession = 0;
        printf("Send MQTT connect ... ");
        ret = mqtt_connect(&client, &data);
        if(ret) {
            printf("error: %d\n\e\r", ret);
            mqtt_network_disconnect(&network);
            taskYIELD();
            continue;
        }
        printf("done\r\n");
        mqtt_subscribe(&client, "aiq_meter/cmd", MQTT_QOS1, topic_received);

        mqtt_message_data_t *mqtt_message_data_p;

        while(1) {
            while(xQueueReceive(publish_queue, (mqtt_message_data_t*)&mqtt_message_data_p, 0) == pdTRUE) {
                printf("got message to publish\r\n");
                ret = mqtt_publish(&client, mqtt_message_data_p->topic->cstring, mqtt_message_data_p->message);
                ret = MQTT_SUCCESS;
                if(ret != MQTT_SUCCESS) {
                    printf("error while publishing message: %d\n", ret);
                    break;
                }
            }

            ret = mqtt_yield(&client, 1000);
            if(ret == MQTT_DISCONNECTED) {
                break;
            }
        }

        printf("Connection dropped, request restart\n\r");
        mqtt_network_disconnect(&network);
        taskYIELD();

    }
}
