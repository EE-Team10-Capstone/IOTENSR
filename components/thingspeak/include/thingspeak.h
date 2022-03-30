/* 
 thingspeak.h - Routines to post data to ThingSpeak.com

 This file is part of the ESP32 Everest Run project
 https://github.com/krzychb/esp32-everest-run

 Copyright (c) 2016 Krzysztof Budzynski <krzychb@gazeta.pl>
 This work is licensed under the Apache License, Version 2.0, January 2004
 See the file LICENSE for details.
*/
#ifndef THINGSPEAK_H
#define THINGSPEAK_H


#include "cloud_data.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_ERR_THINGSPEAK_BASE 0x60000
#define ESP_ERR_THINGSPEAK_POST_FAILED          (ESP_ERR_THINGSPEAK_BASE + 1)

#define WRITE_KEY_MAX_LEN   16

esp_err_t thinkgspeak_post_data(uint16_t *co2, float *temperature, float *humidity);
void thingspeak_initialise();
void get_writekey(char *provisioned_writekey);
// char write_key[17];

#ifdef __cplusplus
}
#endif

#endif  // THINGSPEAK_H
