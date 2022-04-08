#include "microOLED.h"
#include "font8x8_basic.h"
// Following definitions are bollowed from 
// http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html

#define OLEDTAG "OLED"

#define CO2_ch "CO2:"
#define TEMP_ch "\n\nTEMP:"
#define HUMID_ch "\n\nHUMID:"

#define OLED_I2C_ADDRESS   0x3D

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1    
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8    
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14



static void task_ssd1306_display_pattern(void *ignore) {
	i2c_cmd_handle_t cmd;

	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		for (uint8_t j = 0; j < 128; j++) {
			i2c_master_write_byte(cmd, 64 >> j, true);
		}
		
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		vTaskDelay(pdMS_TO_TICKS(1000));
		
	}

	vTaskDelete(NULL);
}

static void task_OLEDClear(void *ignore) {
	i2c_cmd_handle_t cmd;

	uint8_t zero[128]={0};
	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, zero, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

	vTaskDelete(NULL);
}

static void task_OLEDContrast(void *ignore) {
	i2c_cmd_handle_t cmd;

	uint8_t contrast = 0;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
    i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
    i2c_master_write_byte(cmd, contrast, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(1/portTICK_PERIOD_MS);

	vTaskDelete(NULL);
}

static void task_ssd1306_scroll(void *ignore) {
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	// i2c_master_write_byte(cmd, 0x29, true); // vertical and horizontal scroll (p29)
	// i2c_master_write_byte(cmd, 0x00, true);
	// i2c_master_write_byte(cmd, 0x00, true);
	// i2c_master_write_byte(cmd, 0x07, true);
	// i2c_master_write_byte(cmd, 0x01, true);
	// i2c_master_write_byte(cmd, 0x3F, true);

	i2c_master_write_byte(cmd, 0xA3, true); // set vertical scroll area (p30)
	i2c_master_write_byte(cmd, 0x20, true);
	i2c_master_write_byte(cmd, 0x7f, true);

	i2c_master_write_byte(cmd, 0x2F, true); // activate scroll (p29)

	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(OLEDTAG, "Scroll command succeeded");
	} else {
		ESP_LOGE(OLEDTAG, "Scroll command failed. code: 0x%.2X", espRc);
	}

	i2c_cmd_link_delete(cmd);

	vTaskDelete(NULL);
}

static void task_displayText(void *arg_text) {
	char *text = (char*)arg_text;
	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	uint8_t cur_page = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x00, true); // reset column
	i2c_master_write_byte(cmd, 0x10, true);
	i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
			i2c_master_write_byte(cmd, 0x00, true); // reset column
			i2c_master_write_byte(cmd, 0x10, true);
			i2c_master_write_byte(cmd, 0xB0 | ++cur_page, true); // increment page

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		} else {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[i]], 8, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}

	vTaskDelete(NULL);
}



void initializeOLED() 
{
    esp_err_t espRc;
	
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);

    i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_CLK_DIV, true);
    i2c_master_write_byte(cmd, 0x80, true);

    i2c_master_write_byte(cmd, 0xA8, true);
    i2c_master_write_byte(cmd, 0x3F, true);
    
    
    i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_OFFSET, true);
    i2c_master_write_byte(cmd, 0x00, true);// 0x20

    i2c_master_write_byte(cmd, 0x40, true);//set start line

    i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
    i2c_master_write_byte(cmd, 0x14, true);// enable charge pump

    
    i2c_master_write_byte(cmd, OLED_CMD_SET_MEMORY_ADDR_MODE, true);
    
    i2c_master_write_byte(cmd, 0x00, true);
    
    i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
    i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

    i2c_master_write_byte(cmd, 0xDA, true); //Com pins
    i2c_master_write_byte(cmd, 0x12, true); //

    i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true); //Set contrast pins
    i2c_master_write_byte(cmd, 0xCF, true); //

    i2c_master_write_byte(cmd, 0xD9, true); //Pre charge
    i2c_master_write_byte(cmd, 0xF1, true); //

    i2c_master_write_byte(cmd, 0xDB, true); //Set VCOMH Deselect LEVEL
    i2c_master_write_byte(cmd, 0x40, true); //RST LEVL

    //i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ALLON, true); //DISP ALL ON
    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_RAM, true); //DISP ON

    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_NORMAL, true); //
    i2c_master_write_byte(cmd, 0x2E, true); //SCROLL OFF
    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true); //DISP ON
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(OLEDTAG, "OLED configured successfully");
	} else {
		ESP_LOGE(OLEDTAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}

void printOLED(char *string)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreate(&task_OLEDClear, 
                "task_OLEDClear",  
                2048, NULL, 6, NULL);

    vTaskDelay(pdMS_TO_TICKS(100));

	xTaskCreate(&task_displayText, 
                "task_displayText",  
                2048, 
                (void *)string, 
                6, NULL);
}

void printSample(uint16_t *co2, float *temperature, float *humidity)
{
    char buffer[5];
    sprintf(buffer, "%hu", *co2);
    puts(buffer);

    uint16_t formlen;
    formlen = strlen(TEMP_ch);
    formlen += strlen(HUMID_ch);
    formlen += strlen(CO2_ch);
    formlen += 3*strlen(buffer)+3; //lenght of digits added together

    char *formstr = (char *)malloc(formlen);
    printf("%d\n",formlen);
    strcpy(formstr, CO2_ch);
    strcat(formstr, buffer);
    strcat(formstr, TEMP_ch);
    sprintf(buffer, "%2.1f", *temperature);
    strcat(formstr, buffer);
    strcat(formstr, HUMID_ch);
    sprintf(buffer, "%2.1f", *humidity);
    strcat(formstr, buffer);
    puts(formstr);

    xTaskCreate(&task_OLEDClear, 
                "task_OLEDClear",  
                2048, NULL, 6, NULL);

    vTaskDelay(pdMS_TO_TICKS(100));

	xTaskCreate(&task_displayText, 
                "task_displayText",  
                2048, 
                (void *)formstr, 
                6, NULL);

}