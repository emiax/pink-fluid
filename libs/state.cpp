#include <state.h>
#include <velocityGrid.h>
#include <ordinalGrid.h>
#include <iostream>
#include <levelSet.h>

/**
 * Constructor.
 */
State::State(unsigned int width, unsigned int height, unsigned int depth) : 
  w(width), h(height), d(depth) {

  velocityGrid = new VelocityGrid(w, h, d);
  inkGrid = new OrdinalGrid<glm::vec3>(w, h, d);

  resetVelocityGrids();
}

/**
 * Destructor.
 */
State::~State() {
  delete velocityGrid;
  delete inkGrid;
  if (levelSet) {
    delete levelSet;
  }
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
 * Set ink grid
 * @param ink grid to copy concentration values from
 */
void State::setInkGrid(OrdinalGrid<glm::vec3> const* const ink) {
  for (unsigned k = 0; k < d; ++k) {
    for (unsigned int j = 0; j < h; ++j) {
      for (unsigned int i = 0; i < w; ++i) {
        this->inkGrid->set(i, j, k, ink->get(i, j, k));
      }
    } 
  }
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
  levelSet = new LevelSet(w, h, d, *(ls->initSDF), ls->getCellTypeGrid() );
};

/**
 * Get cell type grid
 */
Grid<CellType> const *const State::getCellTypeGrid() const {
  return levelSet->getCellTypeGrid();
}

/**
 * Get ink grid
 * @return const pointer to ink grid.
 */
OrdinalGrid<glm::vec3> const *const State::getInkGrid() const {
  return inkGrid;
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


std::ostream& State::write(std::ostream& stream){
  stream.write(reinterpret_cast<char*>(&w), sizeof(w));
  stream.write(reinterpret_cast<char*>(&h), sizeof(h));
  stream.write(reinterpret_cast<char*>(&d), sizeof(d));
  velocityGrid->write(stream);
  inkGrid->write(stream);
  levelSet->write(stream);
  // Write bubble state to the stream
  auto bubbles = bubbleTracker->getBubbles();
  int nBubbles = bubbles.size();
  stream.write(reinterpret_cast<char*>(&nBubbles), sizeof(nBubbles));
  stream.write(reinterpret_cast<char*>(bubbles.data()), sizeof(Bubble)*nBubbles);
  return stream;
}

std::istream& State::read(std::istream& stream){
  stream.read(reinterpret_cast<char*>(&w), sizeof(w));
  stream.read(reinterpret_cast<char*>(&h), sizeof(h));
  stream.read(reinterpret_cast<char*>(&d), sizeof(d));
  velocityGrid->read(stream);
  inkGrid->read(stream);
  levelSet->read(stream);

  //Read bubbles from stream
  int nBubbles;
  stream.read(reinterpret_cast<char*>(&nBubbles), sizeof(nBubbles));
  std::vector<Bubble> bubbles(nBubbles);
  stream.read(reinterpret_cast<char*>(bubbles.data()), sizeof(Bubble)*nBubbles);

  bubbleTracker->setBubbles(bubbles);
  
  return stream;
}

void State::setBubbleTracker(BubbleTracker *bTracker){
  this->bubbleTracker = bTracker;
}

void State::setParticleTracker(ParticleTracker *pTracker){
  this->particleTracker = pTracker;
  
}
