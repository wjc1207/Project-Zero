#include "driver/i2s.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "SDCard.h"
#include "Pattern.h"
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#define BUFFER_SIZE 256  // I2S缓冲区大小
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
#define ERRORCODE 0
#define SUCCESSCODE 1
#define MINUISTATE (0)
#define MAXUISTATE (5)
#define UISTATE_VOLUP (1)
#define UISTATE_VOLDOWN (5)
#define UISTATE_MUSICBACK (2)
#define UISTATE_MUSICNEXT (4)
#define UISTATE_PLAYPAUSE (3)
#define MINMUSICVOL (1)
#define MAXMUSICVOL (5)


// Define proper RST_PIN if required.
#define RST_PIN -1
#define bufferSize 16
#define SW1 (33)
#define SW2 (16)
#define SW3 (32)
#define SHUTDOWN (23)
#define SCREEN_WIDTH (128)
#define SCREEN_HEIGHT (64)

// declare an SSD1306 display object connected to I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
File file;
uint16_t musicIndex = 0;
uint8_t continueSwitchCounter = 0;
uint8_t triggerDelayCounter = 0;
uint8_t UIstate = 0;
uint8_t musicPlayState = 0; //0: 正在播放 1: 播放完成
uint8_t musicVolume = 3;
uint8_t playPauseState = 0; //0: 播放 1：暂停

String* wavList;
uint16_t wavNum[1] = { -1};
uint16_t lastWavNum = -1;

void setup() {
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(SHUTDOWN, OUTPUT);
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(1000000L);

  oled.begin();
  oled.enableUTF8Print();
  oled.setFont(u8g2_font_wqy12_t_gb2312);
  oled.setFontDirection(0);

  //SD initial
  static SPIClass spi = SPIClass(HSPI);
  spi.begin(14 /* SCK */, 2 /* MISO */, 15 /* MOSI */, 13 /* SS */);
  if (!SD.begin(13 /* SS */, spi, 80000000)) {
    Serial.println("Card Mount Failed");
    return;
  }


  //get music name list
  wavList = listDir(SD, "/", wavNum);
  lastWavNum = readConfig(SD);
  musicIndex = lastWavNum;
  if (musicIndex > wavNum[0])
  {
    musicIndex = 0;
  }
  playMusic("/" + wavList[musicIndex]);
}



void playMusic(String musicName)
{
  uint16_t sample_rate;
  uint8_t bit_depth;
  int16_t samples[BUFFER_SIZE];  // 采样值数组
  uint8_t musicNameLen = musicName.length();
  String musicNameBuffer = musicName;
  uint16_t scrollCounter = 0;
  musicPlayState = 0; //0: 正在播放 1：播放完成

  triggerDelayCounter = 0;

  file = SD.open(musicName);
  writeConfig(SD, musicIndex);
  displayUI(musicNameBuffer);
  if (!file)
  {
    Serial.println("Failed to open file.");
    delay(1000);
    return;
  }
  uint8_t header[44];
  file.read(header, 44);
  sample_rate = header[24] + (header[25] << 8) + (header[26] << 16) + (header[27] << 24);
  bit_depth = header[34] + (header[35] << 8);
  uint16_t num_channels = header[22] + (header[23] << 8);
  float data_size = header[4] + (header[5] << 8) + (header[6] << 16) + (header[7] << 24);
  Serial.println(num_channels);
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
    .channel_format = num_channels == 1 ? I2S_CHANNEL_FMT_ONLY_LEFT : I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 6,
    .dma_buf_len = 512,                                                   // int                    DMA Buffer Length
    .tx_desc_auto_clear   = true,                                                     // bool                   auto clear tx descriptor if there is underflow condition (helps in avoiding noise in case of data unavailability)
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = 26,
    .ws_io_num = 27,
    .data_out_num = 25,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_start(I2S_NUM_0);

  //enable MCLK on GPIO0
  REG_WRITE(PIN_CTRL, 0xF00);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_CLK_OUT2);

  Serial.println("Begin reading file.");


  while (1) {
    uint8_t buffer[BUFFER_SIZE];
    if (file.available() > 0) //music haven't played yet
    {
      if (playPauseState == 0)//播放
      {
        digitalWrite(SHUTDOWN, LOW);
        size_t bytes_read = file.read(buffer, BUFFER_SIZE);
        if (bit_depth == 16)
        {
          for (size_t i = 0; i < bytes_read; i += 2) {

            int16_t sample = (buffer[i + 1] << 8) | (buffer[i]);
            // 应用均衡器
            sample = apply_equalizer(sample);

            size_t bytes_written;
            i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, 1000); //播放

            // 检查错误
            if (bytes_written != sizeof(sample)) {
              ESP_LOGE(TAG, "Failed to write audio data: error code %d", bytes_written);
              // 处理错误情况
            }
          }
        }
        else if (bit_depth == 32)
        {
          for (size_t i = 0; i < bytes_read; i += 4) {
            int32_t sample = (buffer[i + 3] << 24) | (buffer[i + 2] << 16) | (buffer[i + 1] << 8) | (buffer[i]);
            // 应用均衡器
            sample = apply_equalizer_32(sample);

            size_t bytes_written;
            i2s_write(I2S_NUM_0, &sample, sizeof(sample), &bytes_written, 1000); // 播放

            // 检查错误
            if (bytes_written != sizeof(sample)) {
              ESP_LOGE(TAG, "Failed to write audio data: error code %d", bytes_written);
              // 处理错误情况
            }
          }
        }
      }
      else //暂停
      {
        //高电平->关闭
        digitalWrite(SHUTDOWN, HIGH);
      }
      if ((UIstate < MAXUISTATE) and (digitalRead(SW2) == HIGH) and (triggerDelayCounter > 180))
      {
        triggerDelayCounter = 0;
        UIstate += 1;
        displayUI(musicNameBuffer);
      }
      else if ((UIstate > MINUISTATE) and (digitalRead(SW1) == HIGH) and (triggerDelayCounter > 180)) //延迟触发
      {
        triggerDelayCounter = 0;
        UIstate -= 1;
        displayUI(musicNameBuffer);
      }
      if (((UIstate == UISTATE_MUSICBACK) or (UIstate == UISTATE_MUSICNEXT)) and digitalRead(SW3) == HIGH and (triggerDelayCounter > 180)) //触发中断
      {
        //中断 1
        triggerDelayCounter = 0;
        i2s_driver_uninstall(I2S_NUM_0);
        file.close();
        return;
      }
      if ((UIstate == UISTATE_VOLUP) and digitalRead(SW3) == HIGH  and (triggerDelayCounter > 180))
      {
        triggerDelayCounter = 0;
        if (musicVolume < MAXMUSICVOL)
          musicVolume += 1;
        displayUI(musicNameBuffer);
      }
      else if ((UIstate == UISTATE_VOLDOWN) and digitalRead(SW3) == HIGH  and (triggerDelayCounter > 180))
      {
        triggerDelayCounter = 0;
        if (musicVolume > MINMUSICVOL)
          musicVolume -= 1;
        displayUI(musicNameBuffer);
      }
      if ((UIstate == UISTATE_PLAYPAUSE) and (digitalRead(SW3) == HIGH) and (playPauseState == 0) and (triggerDelayCounter > 180)) //播放->暂停
      {
        triggerDelayCounter = 0;
        playPauseState = 1;
        displayUI(musicNameBuffer);
        Serial.println(playPauseState);
      }
      if (UIstate == UISTATE_PLAYPAUSE and (digitalRead(SW3) == HIGH) and (playPauseState == 1) and (triggerDelayCounter > 180))
      {
        triggerDelayCounter = 0;
        playPauseState = 0;
        displayUI(musicNameBuffer);
        Serial.println(playPauseState);
      }
      if (scrollCounter == 180)
      {
        if (musicName.length() > 15) //scrolling
        {
          musicNameBuffer = musicNameBuffer.substring(2);
          if (musicNameBuffer.length() < 15)
          {
            musicNameBuffer = musicName;
          }
          displayUI(musicNameBuffer);
        }
      }

      if (triggerDelayCounter < 255)
      {
        triggerDelayCounter += 1;
        if (playPauseState == 1)
          delay(3);
      }
      if (scrollCounter < 1024)
      {
        scrollCounter += 1;
        if (playPauseState == 1)
          delay(1);
      }
      else
      {
        scrollCounter = 0;
      }
    }
    else
    {
      break;
    }
  }


  //中断 2
  musicPlayState = 1;
  triggerDelayCounter = 0;
  file.close();
  i2s_driver_uninstall(I2S_NUM_0);

  Serial.println("Finished reading file.");
}



void loop() {
  if ((UIstate == UISTATE_MUSICBACK)) //delay 100T
  {
    if (musicIndex > 0)
      musicIndex -= 1;
    playMusic("/" + wavList[musicIndex]);
  }
  if ((UIstate == UISTATE_MUSICNEXT)) //delay 100T
  {
    if (musicIndex < (wavNum[0] - 1))
      musicIndex += 1;
    playMusic("/" + wavList[musicIndex]);
  }
  if (musicPlayState == 1)
  {
    musicPlayState = 0;
    if (musicIndex < (wavNum[0] - 1))
      musicIndex += 1;
    playMusic("/" + wavList[musicIndex]);
  }
}






void displayUI(String musicName)
{
  oled.clearBuffer();

  oled.setDrawColor(1);
  oled.setCursor(20, 10);
  oled.print(musicName);

  oled.setCursor(80, 32);
  oled.print(String(musicIndex + 1) + "/" + String(wavNum[0]));

  oled.setCursor(80, 48);
  oled.print(String(musicVolume) + "/" + String(5));
  if (UIstate == 0) //被选中
    drawPattern(LEFTARROW, 0, 0, 1);
  else
    drawPattern(LEFTARROW, 0, 0, 0);
  if (UIstate == 1)
    drawPattern(UPARROW, 50, 16, 1);
  else
    drawPattern(UPARROW, 50, 16, 0);
  if (UIstate == 2)
    drawPattern(LEFTARROW, 30, 32, 1);
  else
    drawPattern(LEFTARROW, 30, 32, 0);
  if (UIstate == 3)
  {
    if (playPauseState == 0)
      drawPattern(PAUSESYMBOL, 50, 32, 1);
    else
      drawPattern(PLAYSYMBOL, 50, 32, 1);
  }
  else
  {
    if (playPauseState == 0)
      drawPattern(PAUSESYMBOL, 50, 32, 0);
    else
      drawPattern(PLAYSYMBOL, 50, 32, 0);
  }

  if (UIstate == 4)
    drawPattern(RIGHTARROW, 70, 32, 1);
  else
    drawPattern(RIGHTARROW, 70, 32, 0);
  if (UIstate == 5)
    drawPattern(DOWNARROW, 50, 48, 1);
  else
    drawPattern(DOWNARROW, 50, 48, 0);

  oled.sendBuffer();
}
int16_t apply_equalizer(int16_t sample) {

  sample = sample >> (5 - musicVolume);

  // 截断到最大值 65535
  if (sample > (65535)) {
    sample = (65535);
  }

  return sample;
}
int32_t apply_equalizer_32(int32_t sample) {

  sample = sample >> (5 - musicVolume);

  // 截断到最大值 65535
  if (sample > (65535)) {
    sample = (65535);
  }

  return sample;
}
void drawPixel(uint8_t X, uint8_t Y, uint8_t Color)
{
  oled.setDrawColor(Color);
  oled.drawPixel(X, Y);
}

uint8_t drawPattern(const uint8_t bitMap[8][8], uint8_t offsetX, uint8_t offsetY, uint8_t displayMode)
{
  if ((offsetX < 0) or (offsetX > 120) or (offsetY < 0) or (offsetY > 56))
  {
    return ERRORCODE;
  }
  for (int Y = 0; Y < 8; Y += 1)
  {
    for (int X = 0; X < 8; X += 1)
    {
      if (displayMode == 0) //正色
      {
        if (bitMap[Y][X] == 1)
          drawPixel(X + offsetX, Y + offsetY, 1);
        else
          drawPixel(X + offsetX, Y + offsetY, 0);
      }
      else //反色
      {
        if (bitMap[Y][X] == 1)
          drawPixel(X + offsetX, Y + offsetY, 0);
        else
          drawPixel(X + offsetX, Y + offsetY, 1);
      }
    }
  }

  return SUCCESSCODE;
}
