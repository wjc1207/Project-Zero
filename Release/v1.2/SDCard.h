#define MAX_FILES 655

String* listDir(fs::FS &fs, const char * dirname, uint8_t levels, uint8_t wavNum[1]) { 
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
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1, wavNum); // Recursive call to list files in subdirectories
      }
    } else {
      String fileName = file.name();
      String fileExt = "";
      int lastDotIndex = fileName.lastIndexOf('.');
      if (lastDotIndex > 0) {
        fileExt = fileName.substring(lastDotIndex + 1);
      }
      if (fileExt.equalsIgnoreCase("wav") && counter < MAX_FILES) { // Ignore case when comparing file extension, and limit the number of files to MAX_FILES
        wavList[counter] = file.name();
        counter++;
      }
    }
    file = root.openNextFile();
  }
  wavNum[0] = counter;
  return wavList;
}
