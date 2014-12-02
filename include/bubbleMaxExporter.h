#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <string>

class BubbleTracker;
class Bubble;

struct BubbleLife {
  BubbleLife(int sf, float r) {
    this->spawnFrame = sf;
    this->radius = r;
  };

  int spawnFrame;
  int popFrame;
  std::vector<glm::vec3> positions;
  float radius;
};


class BubbleMaxExporter {
 public:
  void exportBubbles(std::string filename);
  void exportSnapshot(int frame, std::string filename, float threshold=0);
  void update(int frame, const BubbleTracker* p);
  void updateBubble(int frame, const Bubble p);
 private:
  std::string serialize(glm::vec3);
  std::map<int, BubbleLife> bubbleLives;
};
