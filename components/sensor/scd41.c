#include "scd41.h"

/////////////////////////////////////////////////////////////////////
// Misc. Functions
/////////////////////////////////////////////////////////////////////

// ECC function from datasheet
uint8_t crc8(const uint8_t *data, size_t count)
{
    uint8_t res = 0xff;

    for (size_t i = 0; i < count; ++i)
    {
        res ^= data[i];
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            if (res & 0x80)
                res = (res << 1) ^ 0x31;
            else
                res = (res << 1);
        }
    }
    return res;
}


// Byte-swap function for swapping cmd hex code in memory for sending/recieving to/from SCD-4x
uint16_t swap(uint16_t data)
{
    return (data << 8) | (data >> 8);
}


/////////////////////////////////////////////////////////////////////
// SCD-4x Command Functions
/////////////////////////////////////////////////////////////////////

// Start periodic measurement command
esp_err_t start_periodic_measurement(){
    printf("\nStarting periodic measurement!\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_STRT_PRDC_MSRMNT);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);    

    return ESP_OK;
}


// Stop periodic measurement command
esp_err_t stop_periodic_measurement(){
    // int8_t execute_time = 500;
    printf("\nStopping periodic measurement!\n");

    uint8_t buf[2];
    *(uint16_t*) buf = swap(xCMD_STP_PRDC_MSRMNT);
    printf("Sending command with hex code: 0x%x%x\n", buf[0], buf[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buf, sizeof(buf), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 500 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);    
    return ESP_OK;
}


// Read measurement command
esp_err_t read_measurement(uint16_t *co2, float *temperature, float *humidity){
    printf("\nReading measurement:\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_RD_MSRMNT);
    printf("Sending command with hex code: 0x%x%.2x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle1 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle1);
    i2c_master_write_byte(cmd_handle1, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle1, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle1);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle1, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle1);

    ets_delay_us(1000);

    uint8_t buffer2[9];
    i2c_cmd_handle_t cmd_handle2 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle2);
    i2c_master_write_byte(cmd_handle2, (SCD_4X_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle2, (void *) buffer2, sizeof(buffer2), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle2);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle2);

    uint16_t data[3];
    for (size_t i = 0; i < 3; i++)
    {
        uint8_t *p = buffer2 + i * 3;
        uint8_t crc = crc8(p, 2);
        if (crc != *(p + 2))
        {
            ESP_LOGE("i2cdev", "Invalid CRC 0x%02x, expected 0x%02x", crc, *(p + 2));
            return ESP_ERR_INVALID_CRC;
        }
        data[i] = swap(*(uint16_t *)p);
    }

    uint16_t rh_raw     = data[2];
    uint16_t temp_raw   = data[1];
    *co2                = data[0]; // No formatting needed for CO2 data

    // Convert raw values
    *temperature    = (float)temp_raw * 175.0f / 65536.0f - 45.0f;
    *humidity       = (float)rh_raw * 100.0f / 65536.0f;

    return ESP_OK;
}


// Data status command
esp_err_t get_data_ready_status(bool *data_ready){

    printf("\nFetching data status.\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_GT_DT_RDY_STTS);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);    

    ets_delay_us(1000);
    
    uint8_t buf2[3];    // Buffer for storing 3-byte data in
    // Send read command
    printf("Sending read command\n");
    i2c_cmd_handle_t cmd_handle2 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle2);
    i2c_master_write_byte(cmd_handle2, (SCD_4X_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle2, (void *) buf2, sizeof(buf2), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd_handle2);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle2);

    uint16_t data;

    uint8_t *p = buf2;
    uint8_t crc = crc8(p, 2);
    if (crc != *(p + 2))
    {
        ESP_LOGE("i2cdev", "Invalid CRC 0x%02x, expected 0x%02x", crc, *(p + 2));
        return ESP_ERR_INVALID_CRC;
    }
    printf("Passed crc8 check.\n");

    data = swap(*(uint16_t*)p);     // Byte-swap response after checking CRC
    data = (data << 5);             // Move 11 LSB to MSB -- last 5 bits now 0

    if (data & 0xffe0){             // Check 11 MSB of data
        printf("Data ready!\n");
        *data_ready = true;
    }
    else{ printf("Data not yet ready.\n"); }

    return ESP_OK;
}


// Self Test command
esp_err_t perform_self_test(){
    uint8_t max_command_duration = 10;  // Max Command Duration 10s for perform_self_test()
    printf("\nPerforming Self-Test!\n");

    uint8_t buf[2];                                 // Buffer for storing hex command code
    *(uint16_t*) buf = swap(xCMD_PRFRM_SLF_TST);    // Perform byte-swap for sending

    // Send write command 
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buf, sizeof(buf), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    // Wait for command duration
    printf("Awaiting Max Command Duration (10s)\n");
    for(int i = 0; i < max_command_duration; i++){
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf(".\n");
    }    
    printf("Max Command Duration Elapsed\n");


    uint8_t buf2[3];    // Buffer for storing 3-byte data in
    // Send read command
    printf("Fetching response...\n");
    i2c_cmd_handle_t cmd_handle2 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle2);
    i2c_master_write_byte(cmd_handle2, (SCD_4X_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle2, (void *) buf2, sizeof(buf2), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd_handle2);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle2);


    // CRC8 Check
    printf("Performing crc8 check...\n");
    uint16_t data;
    for (size_t i = 0; i < 1; i++)
    {
        uint8_t *p = buf2 + i * 3;
        uint8_t crc = crc8(p, 2);
        if (crc != *(p + 2))
        {
            ESP_LOGE("i2cdev", "Invalid CRC 0x%02x, expected 0x%02x", crc, *(p + 2));
            return ESP_ERR_INVALID_CRC;
        }
        data = swap(*(uint16_t*)p);
    }
    printf("Passed crc8 check. Response: 0x%.4x\n", data);

    // if the data is all 0's -> sensor malfunctioned
    if (!(data & 0xff)){printf("Passed Self-Test!\n");}
    else{printf("Malfunction detected.\n");}
    

    return ESP_OK;
}


// Persist settings command
esp_err_t persist_settings(){
    printf("\nPersist settings for next cycle!\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_PR_ST);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 800 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);    

    return ESP_OK;
}


// Start low power periodic measurement command
esp_err_t start_low_power_periodic_measurement(){
     printf("\nStarting low power periodic measuremnet!\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_STRT_L_PR_PER_MS);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);    

    return ESP_OK;
}


// Get temperature offset command
esp_err_t get_temperature_offset(float *temp_offset){
    
    printf("\nFetching temperature offset!\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_GT_TEMP_OFS);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    ets_delay_us(1000);

    uint8_t buffer2[3]; // Buffer for storing 3-byte data in
    printf("Sending read command\n");
    i2c_cmd_handle_t cmd_handle2 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle2);
    i2c_master_write_byte(cmd_handle2, (SCD_4X_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle2, (void *) buffer2, sizeof(buffer2), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle2);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle2);

    uint16_t data;

    uint8_t *p = buffer2;
    uint8_t crc = crc8(p, 2);
    if (crc != *(p + 2))
    {
        ESP_LOGE("i2cdev", "Invalid CRC 0x%02x, expected 0x%02x", crc, *(p + 2));
        return ESP_ERR_INVALID_CRC;
    }
    printf("Passed crc8 check.\n");

    data = swap(*(uint16_t*)p);     // Byte-swap response after checking CRC
    *temp_offset = 175.0f*(float)data/(65536.0f);         // Signal conversion to get temp offset
    printf("Temp offset is %f\n",*temp_offset);
    return ESP_OK;
}


// Set temperature offset command
esp_err_t set_temperature_offset(float *ref_temperature){

    uint16_t co2;
    float temp_offset_prev, temp_offset_actual, temp_scd4x, humidity;

    ESP_ERROR_CHECK(periodic_measurement(&co2, &temp_scd4x, &humidity));
    ESP_ERROR_CHECK(get_temperature_offset(&temp_offset_prev));

    uint8_t buffer[5];

    *(uint16_t*) buffer = swap(xCMD_ST_TEMP_OFS);
    temp_offset_actual = temp_scd4x - *ref_temperature + temp_offset_prev;
    uint16_t signal = (uint16_t)(temp_offset_actual * 65536.0f/175.0f);
    uint8_t *t_offset_pointer = buffer + 2;
    *(uint16_t*) t_offset_pointer = swap(signal); 
    buffer[4] = crc8(t_offset_pointer, 2);

    printf("\ntemp off set: %1.1f\n",temp_offset_actual);
    printf("\nSetting temperature offset!\n");
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);

    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    
    // Send write command
    printf("Writing...\nVerifying offset\n");
    get_temperature_offset(&temp_offset_actual);
    return ESP_OK;
}


// Get sensor altitude command
esp_err_t get_sensor_altitude(uint16_t* altitude){
    printf("\nGetting altitude measurement:\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_GT_SENS_ALT);
    printf("Sending command with hex code: 0x%x%.2x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle1 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle1);
    i2c_master_write_byte(cmd_handle1, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle1, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle1);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle1, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle1);

    ets_delay_us(1000);

    uint8_t buffer2[3];
    i2c_cmd_handle_t cmd_handle2 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle2);
    i2c_master_write_byte(cmd_handle2, (SCD_4X_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle2, (void *) buffer2, sizeof(buffer2), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle2);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle2);


    uint8_t *p = buffer2;
    uint8_t crc = crc8(p, 2);
    if (crc != *(p + 2))
    {
        ESP_LOGE("i2cdev", "Invalid CRC 0x%02x, expected 0x%02x", crc, *(p + 2));
        return ESP_ERR_INVALID_CRC;
    }
    printf("Passed crc8 check.\n");

    *altitude = swap(*(uint16_t*)p);     // Byte-swap response after checking CRC
             // Signal conversion to get temp offset
    printf("Altitude offset is %d\n",*altitude);
    return ESP_OK;
}


// Set sensor altitude command
esp_err_t set_sensor_altitude(uint16_t* altitude){

    uint8_t buffer[5];

    *(uint16_t*) buffer = swap(xCMD_ST_SENS_ALT);
    uint8_t *alt = buffer + 2;
    *(uint16_t*) alt = swap(*altitude); 
    buffer[4] = crc8(alt, 2);

    printf("\nAltitude: %d\n",*altitude);
    printf("\nSetting altitude!\n");
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);

    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    
    // Send write command
    printf("Writing...\nVerifying altitude\n");
    get_sensor_altitude(altitude);
    return ESP_OK;
}


// Set ambient pressure
esp_err_t set_ambient_pressure(uint16_t* pressure){
    uint8_t buffer[5];
    *(uint16_t*) buffer = swap(xCMD_ST_AMB_PRS);
    uint8_t *alt = buffer + 2;
    *(uint16_t*) alt = swap(*pressure); 
    buffer[4] = crc8(alt, 2);

    printf("\nPressure: %d\n",*pressure);
    printf("\nSetting Pressure!\n");
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);

    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    
    // Send write command
    printf("Writing...\n");
    
    return ESP_OK;
}


// Set automatic self calibration enabled
esp_err_t set_automatic_self_calibration_enabled(uint16_t* state){
    uint8_t buffer[5];

    *(uint16_t*) buffer = swap(xCMD_ST_SLF_CAL);
    uint8_t *alt = buffer + 2;
    *(uint16_t*) alt = swap(*state); 
    buffer[4] = crc8(alt, 2);

    printf("\nState: %d\n",*state);
    printf("\nSetting State!\n");
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);

    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);
    
    // Send write command
    printf("Writing...\n");
    get_automatic_self_calibration_enabled(state);
    return ESP_OK;
}


// Get automatic self calibration enabled
esp_err_t get_automatic_self_calibration_enabled(uint16_t* state){
    printf("\nGetting self calibration status:\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_GT_SLF_CAL);
    printf("Sending command with hex code: 0x%x%.2x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle1 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle1);
    i2c_master_write_byte(cmd_handle1, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle1, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle1);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle1, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle1);

    ets_delay_us(1000);

    uint8_t buffer2[2];
    i2c_cmd_handle_t cmd_handle2 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle2);
    i2c_master_write_byte(cmd_handle2, (SCD_4X_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle2, (void *) buffer2, sizeof(buffer2), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle2);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle2);
    printf("Recived transmission hex:\n");

    uint8_t *p = buffer2;
    uint8_t crc = crc8(p, 2);
    printf("%x %x %x \n", buffer2[0],buffer2[1],buffer2[2]);
    
    

    if (crc != *(p + 2))
    {
        ESP_LOGE("i2cdev", "Invalid CRC 0x%02x, expected 0x%02x", crc, *(p + 2));
        return ESP_ERR_INVALID_CRC;
    }
    printf("Passed crc8 check.\n");

    *state = swap(*(uint16_t*)p);     // Byte-swap response after checking CRC
             // Signal conversion to get temp offset
    printf("State: %d\n",*state);
    return ESP_OK;
}


// Measure single shot
esp_err_t measure_single_shot(){
    printf("\nStarting single shot measure!\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_MSR_SS);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 5000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);    

    return ESP_OK;
}


// Measure single shot rht only
esp_err_t measure_single_shot_rht_only(){
    printf("\nStarting single shot measure!\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_MSR_SS_RHT_ONLY);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);    

    return ESP_OK;
}


// Perform forced recalibration
esp_err_t perform_forced_recalibration(uint16_t* target_conc){
    printf("\nTarget concentration: %d\n",*target_conc);
    uint16_t co2; float temperature; float humididty;
    ESP_ERROR_CHECK(start_periodic_measurement());
    vTaskDelay(pdMS_TO_TICKS(SGNL_UPDT_INTRVL_3M+10000));
    ESP_ERROR_CHECK(read_measurement(&co2,&temperature,&humididty));
    ESP_ERROR_CHECK(stop_periodic_measurement());
    vTaskDelay(pdMS_TO_TICKS(500));

    uint8_t buffer[5];

    printf("\nSetting calibration!\n");

    *(uint16_t*) buffer = swap(xCMD_FORCE_RECAL);
    uint8_t *alt = buffer + 2;
    *(uint16_t*) alt = swap(*target_conc); 
    buffer[4] = crc8(alt, 2);

    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);

    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    ets_delay_us((400)*1000);

    uint8_t buffer2[3];
    i2c_cmd_handle_t cmd_handle2 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle2);
    i2c_master_write_byte(cmd_handle2, (SCD_4X_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle2, (void *) buffer2, sizeof(buffer2), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle2);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle2);

    printf("Recived transmission hex:\n");
    
    uint8_t *p = buffer2;
    uint8_t crc = crc8(p, 2);
    printf("%x %x %x \n", buffer2[0],buffer2[1],buffer2[2]);
    
    *target_conc = swap(*(uint16_t*)p);     // Byte-swap response after checking CRC
    printf("FRC correction is dec:%d, hex:%x\n",*target_conc-32768, *target_conc);
    if ((*target_conc==0xffff)){
        printf("Error forced recalibration has failed\n");
    }
    else{
        if (crc != *(p + 2))
        {
            ESP_LOGE("i2cdev", "Invalid CRC 0x%02x, expected 0x%02x\n", crc, *(p + 2));
            return ESP_ERR_INVALID_CRC;
        }
        printf("FRC correction is %d\n",*target_conc-32768);
    }
    
    printf("Passed crc8 check.\n");
    
    // Signal conversion to get temp offset
    persist_settings();
    
    return ESP_OK;
}


// Get serial number
esp_err_t get_serial_number(uint64_t* serial){
    printf("\nGetting serial number:\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_GT_SERIAL_NUM);
    printf("Sending command with hex code: 0x%x%.2x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle1 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle1);
    i2c_master_write_byte(cmd_handle1, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle1, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle1);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle1, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle1);

    ets_delay_us(400);

    uint8_t buffer2[8];
    i2c_cmd_handle_t cmd_handle2 = i2c_cmd_link_create();
    i2c_master_start(cmd_handle2);
    i2c_master_write_byte(cmd_handle2, (SCD_4X_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle2, (void *) buffer2, sizeof(buffer2), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle2);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle2);

    uint8_t *p = buffer2;
    uint8_t crc = crc8(p, 2);

    for (uint8_t n=0; n<3;n++){
        crc = crc8(p+n*3, 2);
        if (crc != *(p+n*3 + 2))
        {
            ESP_LOGE("i2cdev", "Invalid CRC 0x%02x, expected 0x%02x", crc, *(p+n*3 + 2));
            return ESP_ERR_INVALID_CRC;
        }
    }

    printf("Passed crc8 check.\n");
    for (uint8_t n=0; n<3;n++){
        *serial = ((uint64_t)(swap(buffer2[n])) << (32-16*n)) | *serial;    // Byte-swap  and signal conversion after checking CRC
    }

    printf("%" PRIu64 "\n", *serial);

    return ESP_OK;
}


// Perform factory reset
esp_err_t perform_factory_reset(){
    printf("\nPerforming factory reset!\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_PRFRM_FAC_RESET);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 800 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);    

    vTaskDelay(pdMS_TO_TICKS(1200));
    return ESP_OK;

}


// Reinit
esp_err_t reinit(){
    printf("\nReinitalizing the sensor!\n");

    uint8_t buffer[2];
    *(uint16_t*) buffer = swap(xCMD_REINIT);
    printf("Sending command with hex code: 0x%x%x\n", buffer[0], buffer[1]);
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SCD_4X_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, (void *) buffer, sizeof(buffer), I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 800 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    vTaskDelay(pdMS_TO_TICKS(20));


    return ESP_OK;

}

// Condesed commands
esp_err_t periodic_measurement(uint16_t* co2, float* temperature, float* humidity){
    ESP_ERROR_CHECK(start_periodic_measurement());
    vTaskDelay(pdMS_TO_TICKS(SGNL_UPDT_INTRVL));

    bool data_ready = false;
    while(!data_ready)
    {
        vTaskDelay(pdMS_TO_TICKS(SGNL_UPDT_INTRVL));
        ESP_ERROR_CHECK(get_data_ready_status(&data_ready));
    }

  
    ESP_ERROR_CHECK(read_measurement(co2, temperature, humidity));
    printf("CO2: %uppm\n", *co2);
    printf("Temp: %1.1f° Celcius\n", *temperature);
    printf("Humidity: %1.1f %%RH\n", *humidity);

    ESP_ERROR_CHECK(stop_periodic_measurement());
    return ESP_OK;
}


//