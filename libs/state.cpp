#include <state.h>
#include <velocityGrid.h>
#include <ordinalGrid.h>
#include <iostream>
#include <levelSet.h>
#include <bubble.h>

/**
 * Constructor.
 */
State::State(unsigned int width, unsigned int height, unsigned int depth) :
  w(width), h(height), d(depth) {
  velocityGrid = new VelocityGrid(w, h, d);
  levelSet = new LevelSet(w, h, d);
  resetVelocityGrids();
  frameNumber = 0;
  bubbles = std::vector<Bubble>();
  deadBubbleIndices = std::stack<int>();
}


State::State(const State& origin) {
  w = origin.w;
  h = origin.h;
  d = origin.d;

  velocityGrid = new VelocityGrid(*origin.velocityGrid);
  levelSet = new LevelSet(*origin.levelSet);
}

/**
 * Destructor.
 */
State::~State() {
  delete velocityGrid;
  if (levelSet) {
    delete levelSet;
  }
  delete levelSet;
}

/**
 * Reset velocity grids
 */
void State::resetVelocityGrids() {
  for(unsigned k = 0; k < d; ++k) {
    for(unsigned j = 0; j < h; j++){
      for(unsigned i = 0; i <= w; i++){
        velocityGrid->u->set(i, j, k, 0.0f);
      }
    }
  }
  for(unsigned k = 0; k < d; ++k) {
    for(unsigned j = 0; j <= h; j++){
      for(unsigned i = 0; i < w; i++){
        velocityGrid->v->set(i, j, k, 0.0f);
      }
    }
  }
  for(unsigned k = 0; k <= d; ++k) {
    for(unsigned j = 0; j < h; j++){
      for(unsigned i = 0; i < w; i++){
        velocityGrid->w->set(i, j, k, 0.0f);
      }
    }
  }
}


/**
 * Copy velocity grid to internal velocity grid
 */
void State::setVelocityGrid(VelocityGrid const* const velocity){
  for(unsigned k = 0; k < d; ++k) {
    for(unsigned j = 0; j < h; j++){
      for(unsigned i = 0; i <= w; i++){
        velocityGrid->u->set(i, j, k, velocity->u->get(i, j, k));
      }
    }
  }
  for(unsigned k = 0; k < d; ++k) {
    for(unsigned j = 0; j <= h; j++){
      for(unsigned i = 0; i < w; i++){
        velocityGrid->v->set(i, j, k, velocity->v->get(i, j, k));
      }
    }
  }
  for(unsigned k = 0; k <= d; ++k) {
    for(unsigned j = 0; j < h; j++){
      for(unsigned i = 0; i < w; i++){
        velocityGrid->w->set(i, j, k, velocity->w->get(i, j, k));
      }
    }
  }
}

/**
 * Get velocity grid
 */
VelocityGrid const *const State::getVelocityGrid() const{
  return velocityGrid;
};

/**
 * Get width
 */
unsigned int State::getW() const{
  return w;
}


/**
 * Get height
 */
unsigned int State::getH() const{
  return h;
}

/**
 * Get depth
 */
unsigned int State::getD() const {
  return d;
}

/**
 * Set cell type grid
 */
void State::setCellTypeGrid(Grid<CellType>const* const ctg) {
  levelSet->setCellTypeGrid(ctg);
}

/**
 * Set signed distance grid
 * @param sdg grid to copy signed distance from
 */
// void State::setSignedDistanceGrid(OrdinalGrid<float> const* const sdg) {
//   for (unsigned int j = 0; j < h; ++j) {
//     for (unsigned int i = 0; i < w; ++i) {
//       this->signedDistanceGrid->set( i, j, sdg->get(i, j) );
//     }
//   }
// }

/**
 * Set level set
 * @param levelSet pointer to LevelSet object
 */
void State::setLevelSet(LevelSet *ls) {
  if(levelSet){
    delete levelSet;
  }
  levelSet = new LevelSet(w, h, d, *(ls->initSDF), ls->getCellTypeGrid() );
};

/**
 * Get cell type grid
 */
Grid<CellType> const *const State::getCellTypeGrid() const {
  return levelSet->getCellTypeGrid();
}


/**
 * Get signed distance grid
 * @return const pointer to signed distance grid.
 */
OrdinalGrid<float> const *const State::getSignedDistanceGrid() const {
  return levelSet->getDistanceGrid();
}


/**
 * Get closest point grid
 * @return const pointer to closest point grid.
 */
Grid<glm::vec3> const *const State::getClosestPointGrid() const {
  return levelSet->getClosestPointGrid();
}

/**
 * Get bubbles
 */
std::vector<Bubble> State::getBubbles() const {
  std::vector<Bubble> validBubbles;
  int nAliveBubbles = bubbles.size() - deadBubbleIndices.size();

  if (nAliveBubbles > 0) {
    validBubbles.reserve(nAliveBubbles);
  }

  for (auto &b : bubbles) {
    if (b.alive) {
      validBubbles.push_back(b);
    }
  }
  return validBubbles;
}


void State::setBubbles(std::vector<Bubble> &pBubbles){
  bubbles.clear();
  //STL is silly and does not provide clear on stacks
  deadBubbleIndices = std::stack<int>();
  for (int idx = 0; idx < pBubbles.size(); idx++) {
    Bubble b = pBubbles[idx];

    bubbles.push_back(b);
    if (!b.alive) {
      deadBubbleIndices.push(idx);
    }
  }
}

void State::addBubble(Bubble &b) {
  if (b.alive) {
    if (deadBubbleIndices.size() > 0) {
      int idx = deadBubbleIndices.top();
      bubbles[idx] = b;
      // todo: find out what to do with old id?
      b.id = nextBubbleId++;
      deadBubbleIndices.pop();
    } else {
      bubbles.push_back(b);
    }
  }
}

void State::addBubbles(std::vector<Bubble>& newBubbles) {
  for (int i = 0; i < newBubbles.size(); i++) {
    addBubble(newBubbles[i]);
  }
}

/**
 * Write to stream
 */
std::ostream& State::write(std::ostream& stream){
  stream.write(reinterpret_cast<char*>(&frameNumber), sizeof(frameNumber));
    
  stream.write(reinterpret_cast<char*>(&w), sizeof(w));
  stream.write(reinterpret_cast<char*>(&h), sizeof(h));
  stream.write(reinterpret_cast<char*>(&d), sizeof(d));
  velocityGrid->write(stream);
  levelSet->write(stream);
  // Write bubble state to the stream
  int nBubbles = bubbles.size();

  stream.write(reinterpret_cast<char*>(&nBubbles), sizeof(nBubbles));
  stream.write(reinterpret_cast<char*>(bubbles.data()), sizeof(Bubble)*nBubbles);

  stream.write(reinterpret_cast<char*>(&nextBubbleId), sizeof(nextBubbleId));

  return stream;
}

/**
 * Read from stream
 */
std::istream& State::read(std::istream& stream){
  stream.read(reinterpret_cast<char*>(&frameNumber), sizeof(frameNumber));
  
  stream.read(reinterpret_cast<char*>(&w), sizeof(w));
  stream.read(reinterpret_cast<char*>(&h), sizeof(h));
  stream.read(reinterpret_cast<char*>(&d), sizeof(d));

  delete velocityGrid;
  velocityGrid = new VelocityGrid(w,h,d);
  velocityGrid->read(stream);

  delete levelSet;
  levelSet = new LevelSet(w,h,d);
  levelSet->read(stream);

  //Read bubbles from stream
  int nBubbles;
  bubbles.clear();
  stream.read(reinterpret_cast<char*>(&nBubbles), sizeof(nBubbles));
  bubbles.reserve(nBubbles);
  stream.read(reinterpret_cast<char*>(bubbles.data()), sizeof(Bubble)*nBubbles);

  deadBubbleIndices = std::stack<int>();
  for (int i = 0; i < nBubbles; i++) {
    if (!bubbles[i].alive) {
      deadBubbleIndices.push(i);
    }
  }

  stream.read(reinterpret_cast<char*>(&nextBubbleId), sizeof(nextBubbleId));

  return stream;
}


unsigned int State::getFrameNumber() const{
  return frameNumber;
}
