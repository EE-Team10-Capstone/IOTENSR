#include <stdint.h>
#include <inttypes.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_types.h"
#include "esp_log.h"
#include "driver/i2c.h"

// Misc. Sensor Defines
#define SGNL_UPDT_INTRVL    5000    //Signal Update Interval (5s)
#define SGNL_UPDT_INTRVL_L  30000   //Singal Update Interval (30s)
#define SGNL_UPDT_INTRVL_3M 90000   //Signal Update Interval (3mins)

// Sensor Commands
esp_err_t start_periodic_measurement();                                         //start periodic measurement
esp_err_t stop_periodic_measurement();                                          //stop periodic measurement
esp_err_t read_measurement(uint16_t* co2, float* temperature, float* humidity); //read measurment
esp_err_t get_data_ready_status(bool *data_ready);                              //get data ready status 
esp_err_t perform_self_test();                                                  //perform self test
esp_err_t get_temperature_offset(float *temp_offset);                           //get temperature offset
esp_err_t persist_settings();                                                   //persist settings
esp_err_t start_low_power_periodic_measurement();                               //start low power periodic measurement
esp_err_t set_temperature_offset(float *ref_temperature);                       //set temperature offset
esp_err_t get_sensor_altitude(uint16_t* altitude);                              //get sensor altitude
esp_err_t set_sensor_altitude(uint16_t* altitude);                              //set sensor altitude
esp_err_t set_ambient_pressure(uint16_t* pressure);                             //set ambient pressure
esp_err_t set_automatic_self_calibration_enabled(uint16_t* state);              //set automatic calibration enabled
esp_err_t get_automatic_self_calibration_enabled(uint16_t* state);              //get automatic calibration enabled
esp_err_t measure_single_shot();                                                //measure single shot
esp_err_t measure_single_shot_rht_only();                                       //measure single shot rht only
esp_err_t perform_forced_recalibration(uint16_t* target_conc);                  //perform forced recalibration
esp_err_t get_serial_number(uint64_t* serial);                                                  //get serial number
esp_err_t perform_factory_reset();                                              //perform factory reset
esp_err_t reinit();                                                             //reinitalize
// Condensed commands
esp_err_t periodic_measurement(uint16_t* co2, float* temperature, float* humidity);