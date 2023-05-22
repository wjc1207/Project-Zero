#include "driver/i2s.h"


#define BUFFER_SIZE 1024  // I2S缓冲区大小
const float amplitude = 0.2;  // 正弦波振幅，范围：0-1
float frequency = 600; // 正弦波频率，单位：Hz
float sample_rate = 44100;
float bit_depth = 16;
const float phase = 0; // 初始相位，范围：0-2*PI
int16_t samples[BUFFER_SIZE];  // 采样值数组
float t = 0; // 计算时间戳

void setup() {
  // 初始化I2S接口
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = sample_rate,
    .bits_per_sample = i2s_bits_per_sample_t(bit_depth),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_PCM),
    .intr_alloc_flags = 0,
    .dma_buf_count = 2,
    
    .dma_buf_len = 256,
    .use_apll = false
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = 25,
    .ws_io_num = 26,
    .data_out_num = 27,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_start(I2S_NUM_0);
}

void loop() {
  for (int i = 0; i < BUFFER_SIZE; i++)
  {
    float value = amplitude * sin(2 * PI * frequency * t);
    samples[i] = int16_t(value * 31767);
    t += 1/sample_rate;
    if (t > 1/frequency)
      t -= 1/frequency;
  }
  size_t buffer_write;
  i2s_write(I2S_NUM_0, &samples, BUFFER_SIZE, &buffer_write, portMAX_DELAY);
  
}
