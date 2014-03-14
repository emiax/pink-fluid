#include <levelSet.h>

namespace factory{
  namespace levelSet{
    LevelSet* ball(unsigned int w, unsigned int h, unsigned int d){
      return new LevelSet(w, h, d,
        [=](const unsigned int &i, const unsigned int &j, const unsigned int &k) {
          // distance function to circle with radius w/3, center in (w/2, h/2, d/2)
          const float x = (float)i - (float)w/2.0;
          const float y = (float)j - (float)h/2.0;
          const float z = (float)k - (float)d/2.0;
          std::cout << "wat in ctg init "<< w << " " <<  h<< " " << d << std::endl;

          return sqrt( x*x + y*y + z*z) - (float)w/2.5;
        }, [=](unsigned int i, unsigned int j, unsigned int k){
          CellType bt = CellType::EMPTY;

          if(i == 0){
            bt = CellType::SOLID;
          }
          else if(j == 0){
            bt = CellType::SOLID;
          }
          else if(k == 0){
            bt = CellType::SOLID;
          }
          else if(i == w - 1){
            bt = CellType::SOLID;
          }
          else if(j == h - 1){
            bt = CellType::SOLID;
          }
          else if(k == d - 1){
            bt = CellType::SOLID;
          }
          return bt;
        });
    }
  }
}
