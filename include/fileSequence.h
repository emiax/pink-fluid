#pragma once

#include <string>

class FileSequence {
 public:
  FileSequence(std::string originalFileName);
  std::string getFileNameRelative(int offset);
  std::string getFileNameAbsolute(int index);
  int getOriginalIndex();
  bool isValid();

 private:
  std::string prefix;
  int originalIndex;
  std::string suffix;
  bool valid = false;
};
