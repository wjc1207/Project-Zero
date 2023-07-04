/*
  ESP32 I2S Amplifier Sample
*/

// Include I2S driver
#include <driver/i2s.h>

// Connections to INMP441 I2S microphone
#define I2S_IN_WS 18
#define I2S_IN_SD 19
#define I2S_IN_SCK 5

#define I2S_OUT_SCK 26
#define I2S_OUT_WS 27
#define I2S_OUT_SD 15

// Use I2S Processor 0
#define I2S_IN_PORT I2S_NUM_0
#define I2S_OUT_PORT I2S_NUM_1

// Define input buffer length
#define bufferLen 64
int16_t sInBuffer[bufferLen];
int16_t sOutBuffer[bufferLen * 2];

void i2s_in_install() {
  // Set up I2S Processor configuration
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };

  i2s_driver_install(I2S_IN_PORT, &i2s_config, 0, NULL);
}

void i2s_out_install() {
  // Set up I2S Processor configuration
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen * 2,
    .tx_desc_auto_clear   = true,
  };

  i2s_driver_install(I2S_OUT_PORT, &i2s_config, 0, NULL);

  REG_WRITE(PIN_CTRL, 0xF00);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_CLK_OUT2);
}

void i2s_in_setpin() {
  // Set I2S pin configuration
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_IN_SCK,
    .ws_io_num = I2S_IN_WS,
    .data_out_num = -1,
    .data_in_num = I2S_IN_SD
  };

  i2s_set_pin(I2S_IN_PORT, &pin_config);
}
void i2s_out_setpin() {
  // Set I2S pin configuration
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_OUT_SCK,
    .ws_io_num = I2S_OUT_WS,
    .data_out_num = I2S_OUT_SD,
    .data_in_num = -1
  };

  i2s_set_pin(I2S_OUT_PORT, &pin_config);
}

void setup() {

  // Set up Serial Monitor
  Serial.begin(115200);
  Serial.println(" ");

  delay(1000);

  // Set up I2S
  i2s_in_install();
  i2s_out_install();
  i2s_in_setpin();
  i2s_out_setpin();
  i2s_start(I2S_IN_PORT);
  i2s_start(I2S_OUT_PORT);

  delay(500);
}

void loop() {
  size_t bytes_read;
  size_t bytes_written;
  i2s_read(I2S_IN_PORT, &sInBuffer, bufferLen, &bytes_read, 1000);
  for (int i = 0; i < bufferLen; i += 1)
    sInBuffer[i] = sInBuffer[i] << 4;
  for (int i = 0; i < (bufferLen * 2); i += 1)
    sOutBuffer[i] = sInBuffer[i / 2];
  i2s_write(I2S_OUT_PORT, &sOutBuffer, bufferLen * 2, &bytes_written, 1000); //播放

}
