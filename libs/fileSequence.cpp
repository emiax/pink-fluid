#include <fileSequence.h>

FileSequence::FileSequence(std::string originalFileName) {
  int i = originalFileName.size() - 1;

  int lastInteger = originalFileName.size();
  int firstInteger = 0;

  valid = true;
  for (; i >= 0; i--) {
    if (originalFileName[i] >= '0' && originalFileName[i] <= '9') {
      lastInteger = i;
      // break when first integer is found from end.
      break;
    }
  }
  for (; i >= 0; i--) {
    if (originalFileName[i] < '0' || originalFileName[i] > '9') {
      // break when first non-integer is found
      firstInteger = i + 1;
      break;
    }
  }

  // no integers found in filename.
  if (lastInteger < originalFileName.size()) {
    prefix = originalFileName.substr(0, firstInteger);

    std::string integer = originalFileName.substr(firstInteger, lastInteger - firstInteger + 1);
    originalIndex = std::stoi(integer);

    suffix = originalFileName.substr(lastInteger + 1, std::string::npos);
  } else {
    valid = false;
  }
}

std::string FileSequence::getFileNameRelative(int offset) {
  return prefix + std::to_string(originalIndex + offset) + suffix;
}


std::string FileSequence::getFileNameAbsolute(int position) {
  return prefix + std::to_string(position) + suffix;
}

int FileSequence::getOriginalIndex() {
  return originalIndex;
}

bool FileSequence::isValid() {
  return valid;
}
