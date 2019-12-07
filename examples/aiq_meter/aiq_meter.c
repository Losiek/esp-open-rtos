// This file contains main functionality of air quality meter
#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>

#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

#include <semphr.h>

SemaphoreHandle_t wifi_alive;

static void beat_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while(1) {
        vTaskDelayUntil(&xLastWakeTime, 10000/portTICK_PERIOD_MS);
        printf("beat\r\n");
    }
}

static const char *get_my_id(void)
{
    // Use MAC address for Station as unique ID
    static char my_id[13];
    static bool my_id_done = false;
    int8_t i;
    uint8_t x;
    if(my_id_done) {
        return my_id;
    }
    if(!sdk_wifi_get_macaddr(STATION_IF, (uint8_t*)my_id)) {
        return NULL;
    }
    for(i = 5; i >= 0; --i)
    {
        x = my_id[i] & 0x0F;
        if(x > 9) x += 7;
        my_id[i * 2 + 1] = x + '0';
        x = my_id[i] >> 4;
        if(x > 9) x += 7;
        my_id[i * 2] = x + '0';
    }
    my_id[12] = '\0';
    my_id_done = true;
    return my_id;
}

static void wifi_task(void *pvParameters)
{
    uint8_t status = 0;
    uint8_t retries = 10;
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    printf("WiFi: connecting to WiFi\n\r");
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    while(1)
    {
        while((status != STATION_GOT_IP) && retries) {
            status = sdk_wifi_station_get_connect_status();
            printf("%s: status = %d\n\r", __func__, status);
            if(status == STATION_WRONG_PASSWORD) {
                printf("WiFi: wrong password\n\r");
                break;
            } else if(status == STATION_NO_AP_FOUND) {
                printf("WiFi: AP not found\n\r");
                break;
            } else if(status == STATION_CONNECT_FAIL) {
                printf("WiFi: connection failed\n\r");
                break;
            } else {
                printf("WiFi: unknown error\n\r");
            }
            vTaskDelay(1000/portTICK_PERIOD_MS);
            --retries;
        }
        if(status == STATION_GOT_IP) {
            printf("WiFi: Connected\n\r");
            xSemaphoreGive(wifi_alive);
            taskYIELD();
        }
        while((status = sdk_wifi_station_get_connect_status()\
                    == STATION_GOT_IP)) {
            xSemaphoreGive(wifi_alive);
            taskYIELD();
        }
        printf("WiFi: disconnected\n\r");
        sdk_wifi_station_disconnect();
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    vSemaphoreCreateBinary(wifi_alive);
    xTaskCreate(&wifi_task, "wifi_task", 256, NULL, 2, NULL);
    xTaskCreate(&beat_task, "beat_task", 256, NULL, 3, NULL);
}
