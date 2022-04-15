#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "xclk.h"
#include "driver/i2s.h"
#include "main.h"
//static const char* TAG = "camera_xclk";

esp_err_t camera_enable_out_clock(camera_config_t* config) {

	const int i2s_num = 1; // i2s port number
	const i2s_config_t i2s_config = { .mode = I2S_MODE_MASTER | I2S_MODE_TX,
			.sample_rate = config->xclk_freq_hz/256, .bits_per_sample = 16, .channel_format =
					I2S_CHANNEL_FMT_RIGHT_LEFT, .communication_format =
							I2S_COMM_FORMAT_STAND_I2S, //I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
			.intr_alloc_flags = 0, // default interrupt priority
			.dma_buf_count = 8, .dma_buf_len = 64, .use_apll = 0 };

	const i2s_pin_config_t pin_config = { .bck_io_num = I2S_PIN_NO_CHANGE,
			.ws_io_num = I2S_PIN_NO_CHANGE, .data_out_num =
					I2S_PIN_NO_CHANGE, .data_in_num = I2S_PIN_NO_CHANGE };
	i2s_driver_install(i2s_num, &i2s_config, 0, NULL); //install and start i2s driver
	i2s_set_pin(i2s_num, &pin_config);
	uint32_t reg_val = REG_READ(PIN_CTRL);
	Debug("PIN_CTRL before:%x,%d", reg_val,config->xclk_freq_hz/256);
	REG_WRITE(PIN_CTRL, 0xFFFFFFFF);
	reg_val = REG_READ(PIN_CTRL);
	Debug("PIN_CTRL after:%x", reg_val);
	PIN_FUNC_SELECT(GPIO_PIN_REG_0, 1); //GPIO0 as CLK_OUT1
	i2s_start(i2s_num);

	return ESP_OK;
}

void camera_disable_out_clock() {
	periph_module_disable(PERIPH_LEDC_MODULE);
}
