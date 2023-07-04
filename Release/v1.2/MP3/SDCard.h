#define MAX_FILES 655
uint16_t readConfig(fs::FS &fs)
{
  File file = fs.open("/config.txt");
  if (!fs.exists("/config.txt"))
  {
    File file = fs.open("/config.txt", FILE_WRITE);
    file.print("0");
  }
  
  String lastWavNumStr = "";
  while (file.available()) {
    lastWavNumStr += (char)file.read();
  }
  file.close();
  Serial.println(lastWavNumStr);
  uint16_t value = static_cast<uint16_t>(lastWavNumStr.toInt());
  return value;
}

void writeConfig(fs::FS &fs, uint16_t lastWavNum)
{
  File file = fs.open("/config.txt", FILE_WRITE);
  file.print(String(lastWavNum));
  file.close();
}

String* listDir(fs::FS &fs, const char * dirname, uint16_t wavNum[1]) {
  //return all .wav file names
  static String wavList[MAX_FILES]; // Make this static to keep its value between function calls
  static String error[1] = {"ERROR"};
  uint8_t counter = 0;

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return error;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return error;
  }

  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    if (fileName.endsWith(".wav") && counter < MAX_FILES) {
      wavList[counter] = fileName;
      counter++;
    }
    file = root.openNextFile();
  }
  wavNum[0] = counter;
  return wavList;
}
