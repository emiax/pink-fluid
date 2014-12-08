#pragma once

#include <string>
#include <vector>
#include <map>
#include <bubble.h>

class BubbleConfig {
 public:
  BubbleConfig();
  ~BubbleConfig();
  void read(std::istream& stream);
  void write(std::ostream& stream);
  void clear();
  void addBubbles(int frame, std::vector<Bubble> bubbles);
  std::vector<Bubble> getBubblesInFrame(int frame);
  std::map<int, std::vector<Bubble>> getMap();
 private:
  std::map<int, std::vector<Bubble>> config;
};
