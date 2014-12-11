#include <bubbleConfig.h>
#include <iostream>


BubbleConfig::BubbleConfig() {

}

BubbleConfig::~BubbleConfig() {

}

void BubbleConfig::read(std::istream& stream) {
  int nFrameBuckets;
  stream.read(reinterpret_cast<char*>(&nFrameBuckets), sizeof(nFrameBuckets));
  this->clear();
  for (int i = 0; i < nFrameBuckets; i++) {
    int frame, nBubbles;

    stream.read(reinterpret_cast<char*>(&frame), sizeof(frame));
    stream.read(reinterpret_cast<char*>(&nBubbles), sizeof(nBubbles));
    std::vector<Bubble> bubbles(nBubbles);

    stream.read(reinterpret_cast<char*>(bubbles.data()), sizeof(Bubble)*nBubbles);

    this->addBubbles(frame, bubbles);

  }
}

void BubbleConfig::write(std::ostream& stream) {
  int nFrameBuckets = config.size();
  stream.write(reinterpret_cast<char*>(&nFrameBuckets), sizeof(nFrameBuckets));

  for (auto it : config) {
    int frame = it.first;
    std::vector<Bubble> &bubbles = it.second;
    int nBubbles = bubbles.size();

    stream.write(reinterpret_cast<char*>(&frame), sizeof(frame));
    stream.write(reinterpret_cast<char*>(&nBubbles), sizeof(nBubbles));
    stream.write(reinterpret_cast<char*>(bubbles.data()), sizeof(Bubble)*nBubbles);
  }
}


void BubbleConfig::clear() {
  config.clear();
}


void BubbleConfig::addBubbles(int frame, std::vector<Bubble> bubbles) {
  std::vector<Bubble> &bucket = config[frame];

  bucket.reserve(bucket.size() + bubbles.size());
  for (Bubble &b : bubbles) {
    bucket.push_back(b);
  }
}


std::vector<Bubble> BubbleConfig::getBubblesInFrame(int frame) {
  if (config.count(frame) > 0) {
    return config[frame];
  } else {
    return std::vector<Bubble>();
  }
}
