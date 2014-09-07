#include <bubbleMaxExporter.h>
#include <bubbleTracker.h>
#include <fstream>
#include <sstream>

void BubbleMaxExporter::update(int frame, const BubbleTracker* bt) {
  const std::vector<Bubble*> *bubbles = bt->getBubbles();
  for (const Bubble *b : *bubbles) {
    updateBubble(frame, b);
  }
}

void BubbleMaxExporter::exportBubbles(std::string filename) {
    std::ofstream f(filename);

    f << "with animate on (" << std::endl;
    
    std::map<int,BubbleLife>::iterator it;
    for (it = bubbleLives.begin(); it != bubbleLives.end(); ++it) {
      BubbleLife bl = it->second;
      int i = it->first;

      f << "s" << i << " = sphere visibility:false radius:" << bl.radius <<
        " pos:" << serialize(bl.positions[0]) << " wirecolor:yellow;" << std::endl;
      
      // hide and position bubble before spawning
      f << "at time " << (bl.spawnFrame - 1) << "(" << std::endl; 
      f << "s" << i << ".visibility = false;" << std::endl;
      //      f << "s" << i << ".pos = " << serialize(bl.positions[0]) << ";" << std::endl;
      f << ")" << std::endl;


      // show bubble when spawning
      f << "at time " << (bl.spawnFrame) << "(" << std::endl; 
      f << "s" << i << ".visibility = true;" << std::endl;
      f << ")" << std::endl;


      // show bubble before popping
      f << "at time " << (bl.popFrame - 1) << "(" << std::endl; 
      f << "s" << i << ".visibility = true;" << std::endl;
      f << ")" << std::endl;


      // hide bubble when popped
      f << "at time " << (bl.popFrame) << "(" << std::endl; 
      f << "s" << i << ".visibility = false;" << std::endl;
      f << ")" << std::endl;


      // update position of bubble every timeframe
      for (int j = 0; j < bl.positions.size(); j++) {
        f << "at time " << (bl.spawnFrame + j) << "(" << std::endl; 
        f << "s" << i << ".pos:" << serialize(bl.positions[j]) << ";" << std::endl;
        f << ")" << std::endl;
        }
    }

    f << ")" << std::endl;

    f.close();
}


void BubbleMaxExporter::exportSnapshot(int frame, std::string filename, float threshold) {
    std::ofstream f(filename);

    std::map<int,BubbleLife>::iterator it;
    for (it = bubbleLives.begin(); it != bubbleLives.end(); ++it) {
      BubbleLife bl = it->second;
      int i = it->first;
      
      if (frame >= bl.spawnFrame && frame < bl.popFrame && bl.radius >= threshold) {
        f << "s" << i << " = sphere radius:" << bl.radius <<
          " pos = " << serialize(bl.positions[frame - bl.spawnFrame]) << " wirecolor:yellow;" << std::endl;
      }
    }
    f.close();
}



void BubbleMaxExporter::updateBubble(int frame, const Bubble *b) {
  int id = b->id;

  if (bubbleLives.count(id) == 0) {
    // bubble does not exist.
    BubbleLife bf(frame, b->radius);
    bubbleLives.insert(std::pair<int, BubbleLife>(id, bf));
  }
  BubbleLife *bl = &bubbleLives.at(id);
  
  bl->positions.push_back(b->position);
  bl->popFrame = frame + 1;
}


std::string BubbleMaxExporter::serialize(glm::vec3 pos) {
  std::stringstream s;
  s << "[" << pos.x << "," << pos.y << "," << pos.z << "]";
  return s.str();
}
