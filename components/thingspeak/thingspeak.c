/*
 thingspeak.c - Routines to post data to ThingSpeak.com

 This file is adapted from the ESP32 Everest Run project
 https://github.com/krzychb/esp32-everest-run

 Copyright (c) 2016 Krzysztof Budzynski <krzychb@gazeta.pl>
 This work is licensed under the Apache License, Version 2.0, January 2004
 See the file LICENSE for details.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include <stdio.h>
#include <string.h>

#include "thingspeak.h"
#include "http.h"

static const char* TAG = "ThingSpeak";

//static char *THINGSPEAK_WRITE_KEY;

#define WEB_SERVER "api.thingspeak.com"

#define THINGSPEAK_WRITE_KEY "RSZX2RN2OL6CDCNU"

static const char* get_request_start =
    "GET /update?key="
    THINGSPEAK_WRITE_KEY;

static const char* get_request_end =
    " HTTP/1.1\n"
    "Host: "WEB_SERVER"\n"
    "Connection: close\n"
    "User-Agent: esp32 / esp-idf\n"
    "\n";

static http_client_data http_client = {0};

/* Collect chunks of data received from server
   into complete message and save it in proc_buf
 */
static void process_chunk(uint32_t *args)
{
    http_client_data* client = (http_client_data*)args;

    int proc_buf_new_size = client->proc_buf_size + client->recv_buf_size;
    char *copy_from;

    if (client->proc_buf == NULL){
        client->proc_buf = malloc(proc_buf_new_size);
        copy_from = client->proc_buf;
    } else {
        proc_buf_new_size -= 1; // chunks of data are '\0' terminated
        client->proc_buf = realloc(client->proc_buf, proc_buf_new_size);
        copy_from = client->proc_buf + proc_buf_new_size - client->recv_buf_size;
    }
    if (client->proc_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory");
    }
    client->proc_buf_size = proc_buf_new_size;
    memcpy(copy_from, client->recv_buf, client->recv_buf_size);
}

static void disconnected(uint32_t *args)
{
    http_client_data* client = (http_client_data*)args;

    free(client->proc_buf);
    client->proc_buf = NULL;
    client->proc_buf_size = 0;

    ESP_LOGD(TAG, "Free heap %u", xPortGetFreeHeapSize());
}

esp_err_t ThingSpeakPostData(uint16_t *co2, float *temperature, float *humidity)
{
    int n;

    // 1
    n = snprintf(NULL, 0, "%u", *co2);
    char field1[n+1];
    sprintf(field1, "%u", *co2);
    // 2
    n = snprintf(NULL, 0, "%2.1f", *temperature);
    char field2[n+1];
    sprintf(field2, "%2.1f", *temperature);
    // 3
    n = snprintf(NULL, 0, "%2.1f", *humidity);
    char field3[n+1];
    sprintf(field3, "%2.1f", *humidity);

    // request string size calculation
    int string_size = strlen(get_request_start);
    //string_size += strlen(THINGSPEAK_WRITE_KEY);
    string_size += strlen("&fieldN=") * 3;  // number of fields
    string_size += strlen(field1);
    string_size += strlen(field2);
    string_size += strlen(field3);
    string_size += strlen(get_request_end);
    string_size += 1;  // '\0' - space for string termination character

    // request string assembly / concatenation
    char * get_request = malloc(string_size);
    strcpy(get_request, get_request_start);
    //strcpy(get_request, THINGSPEAK_WRITE_KEY);
    strcat(get_request, "&field1=");
    strcat(get_request, field1);
    strcat(get_request, "&field2=");
    strcat(get_request, field2);
    strcat(get_request, "&field3=");
    strcat(get_request, field3);
    strcat(get_request, get_request_end);

    puts(get_request);

    esp_err_t err = http_client_request(&http_client, WEB_SERVER, get_request);

    free(get_request);
    return err;
}

void initializeThingSpeak()
{
    http_client_on_process_chunk(&http_client, process_chunk);
    http_client_on_disconnected(&http_client, disconnected);
}

void get_writekey(char *provisioned_writekey)
{
    // Free this at some point lmao
    // THINGSPEAK_WRITE_KEY = (char *)malloc((WRITE_KEY_MAX_LEN + 1) * sizeof(char));
    // memcpy(THINGSPEAK_WRITE_KEY, provisioned_writekey, WRITE_KEY_MAX_LEN + 1);
    // puts(THINGSPEAK_WRITE_KEY);
}
