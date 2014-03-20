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

    LevelSet* halfContainerBox(unsigned int w, unsigned int h, unsigned int d){
      return new LevelSet(w, h, d,
        [=](const unsigned int &i, const unsigned int &j, const unsigned int &k) {
          // fill half the container with fluid
          return (i > 0 && i < w/2 && j > 0 && j < h - 1 && k > 0 && k < d - 1) ? -0.5 : 0.5;

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

    LevelSet* fourthContainerBox(unsigned int w, unsigned int h, unsigned int d){
      return new LevelSet(w, h, d,
        [=](const unsigned int &i, const unsigned int &j, const unsigned int &k) {
          // fill half the container with fluid
          return (i > 0 && i < w/2 && j > 0 && j < h/2 && k > 0 && k < d - 1) ? -0.5 : 0.5;

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

    LevelSet* twoPillars(unsigned int w, unsigned int h, unsigned int d){
      return new LevelSet(w, h, d,
        [=](const unsigned int &i, const unsigned int &j, const unsigned int &k) {
          // fill half the container with fluid
          if(i > w/8 && i < 3*w/8) {
            return (j > h/6 && j < 5*h/6 && k > d/4+3 && k < 3*d/4+3) ? -0.5 : 0.5;
          } else if (i > 5*w/8 && i < 7*w/8) {
            return (j > h/6 && j < 5*h/6 && k > d/4 && k < 3*d/4) ? -0.5 : 0.5;
          }
          return 0.5;

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

    LevelSet* stairs(unsigned int w, unsigned int h, unsigned int d){
      return new LevelSet(w, h, d,
        [=](const unsigned int &i, const unsigned int &j, const unsigned int &k) {
          // fill half the container with fluid
          return (i > 0 && i < w/3 && j > h/2 && j < h - 2 && k > 0 && k < d - 1) ? -0.5 : 0.5;

        }, [=](unsigned int i, unsigned int j, unsigned int k){
          CellType bt = CellType::EMPTY;

          // two stair steps
          if(i < w/3 && j < h/2) {
            bt = CellType::SOLID;
          } else if (i < 2*w/3 && j < 1*h/4) {
            bt = CellType::SOLID;
          }

          // the usual container
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
