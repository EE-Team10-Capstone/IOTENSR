#include <stdint.h>
#include <inttypes.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_types.h"
#include "esp_log.h"
#include "driver/i2c.h"

// SCD-4x I2C Address
#define SCD_4X_ADDR 0x62

#define SCD41_SDA_PIN 1 // Blue Wire
#define SCD41_SCL_PIN 2 // Yellow Wire

// SCD-4x command hex codes
#define xCMD_STRT_PRDC_MSRMNT   0x21b1  //start periodic measurement
#define xCMD_STP_PRDC_MSRMNT    0x3f86  //stop periodic measurement
#define xCMD_RD_MSRMNT          0xec05  //read measurment
#define xCMD_GT_DT_RDY_STTS     0xe4b8  //get data ready status 
#define xCMD_PRFRM_SLF_TST      0x3639  //perform self test
#define xCMD_ST_TEMP_OFS        0x241d  //set temperature offset
#define xCMD_PR_ST              0x3615  //persist settings
#define xCMD_STRT_L_PR_PER_MS   0x21ac  //start low power periodic measurement
#define xCMD_GT_TEMP_OFS        0x2318  //get temperature offset
#define xCMD_GT_SENS_ALT        0x2322  //get sensor altitude
#define xCMD_ST_SENS_ALT        0x2427  //set sensor altitude
#define xCMD_ST_AMB_PRS         0xe000  //set ambient pressure
#define xCMD_ST_SLF_CAL         0x2416  //set automatic calibration enabled
#define xCMD_GT_SLF_CAL         0x2313  //get automatic calibration enabled
#define xCMD_MSR_SS             0x219d  //measure single shot
#define xCMD_MSR_SS_RHT_ONLY    0x219d  //measure single shot rht only
#define xCMD_FORCE_RECAL        0x362f  //perform forced recalibration
#define xCMD_GT_SERIAL_NUM      0x3682  //get serial number
#define xCMD_PRFRM_FAC_RESET    0x3632  //perform factory reset
#define xCMD_REINIT             0x3646  //reinitalize

// Misc. Sensor Defines
#define SGNL_UPDT_INTRVL    5000    //Signal Update Interval (5s)
#define SGNL_UPDT_INTRVL_L  30000   //Singal Update Interval (30s)
#define SGNL_UPDT_INTRVL_3M 90000   //Signal Update Interval (3mins)

// Misc. Functions
uint8_t crc8(const uint8_t *data, size_t count);
uint16_t swap(uint16_t data);

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