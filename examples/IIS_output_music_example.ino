#include "driver/i2s.h"
#include "FS.h"
#include "SD.h"、
#include "SPI.h"

#define BUFFER_SIZE 256  // I2S缓冲区大小
float sample_rate;
float bit_depth;
int16_t samples[BUFFER_SIZE];  // 采样值数组
float t = 0; // 计算时间戳
File file;

void setup() {
  pinMode(23, OUTPUT);
  pinMode(16,INPUT);
  pinMode(17,INPUT);
  Serial.begin(115200);
  SPIClass spi = SPIClass(HSPI);
  spi.begin(14 /* SCK */, 2 /* MISO */, 15 /* MOSI */, 13 /* SS */);
  if (!SD.begin(13 /* SS */, spi, 80000000)) {
    Serial.println("Card Mount Failed");
    return;
  }
  file = SD.open("/Moon Halo.wav");
  if (!file)
  {
    Serial.println("Failed to open file.");
    delay(1000);
    return;
  }

  // 读取wav文件头
  uint8_t header[44];
  file.read(header, 44);
  sample_rate = header[24] + (header[25] << 8) + (header[26] << 16) + (header[27] << 24);
  bit_depth = header[34] + (header[35] << 8);
  uint16_t num_channels = header[22] + (header[23] << 8);
  float data_size = header[4] + (header[5] << 8) + (header[6] << 16) + (header[7] << 24);
  Serial.println(sample_rate);
  Serial.println(bit_depth);
  Serial.print(data_size / 1024 / 1024);
  Serial.println("MB");

  /*  字节偏移量  内容  大小
    0-3 Chunk ID  "RIFF"
    4-7 Chunk Size  文件总大小-8
    8-11  Format  "WAVE"
    12-15 Subchunk 1 ID "fmt "
    16-19 Subchunk 1 Size 16
    20-21 Audio Format  1
    22-23 Num Channels  1或2
    24-27 Sample Rate 采样率
    28-31 Byte Rate 采样率 × 采样位数 × 通道数 / 8
    32-33 Block Align 采样位数 × 通道数 / 8
    34-35 Bits Per Sample 采样位数
    36-39 Subchunk 2 ID "data"
    40-43 Subchunk 2 Size 音频数据大小
    int16 有符号格式
    */

  // 初始化I2S接口
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = sample_rate,
    .bits_per_sample = i2s_bits_per_sample_t(bit_depth),
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_PCM),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
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

  Serial.println("Begin reading file.");

  if (bit_depth == 16)
  {
    while (1) {
      uint8_t buffer[BUFFER_SIZE];
      if (file.available() > 0)
      {
        size_t bytes_read = file.read(buffer, BUFFER_SIZE);
        for (size_t i = 0; i < bytes_read; i += 2) {
          int16_t sample = (buffer[i + 1] << 8) | (buffer[i]);
          // 应用均衡器
          sample = apply_equalizer(sample);

          size_t bytes_written;
          i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);

          // 检查错误
          if (bytes_written != sizeof(sample)) {
            ESP_LOGE(TAG, "Failed to write audio data: error code %d", bytes_written);
            // 处理错误情况
          }
        }
      }
      else
        break;
    }
    Serial.println("Finished reading file.");
  }
  else if (bit_depth == 8)
  {
    while (1) {
      uint8_t buffer[BUFFER_SIZE];
      if (file.available() > 0)
      {
        size_t bytes_read = file.read(buffer, BUFFER_SIZE);
        for (size_t i = 0; i < bytes_read; i += 1) {
          uint8_t sample = buffer[i];
          size_t bytes_written;
          i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, portMAX_DELAY);
        }
      }
      else
        break;
    }
    Serial.println("Finished reading file.");
  }
  digitalWrite(23, HIGH);

}
void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();

}
int16_t apply_equalizer(int16_t sample) {
  // 增加 6 dB 的增益
  sample = sample >> 1;
  
  // 截断到最大值 65535
  if (sample > (65535)) {
    sample = (65535);
  }
  
  return sample;
}
void loop() {

}
