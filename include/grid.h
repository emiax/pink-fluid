#pragma once
#include <functional>
#include <glm/glm.hpp>

typedef glm::i32vec2 GridCoordinate;

template <class T>
class Grid {
 public:

  /**
   * Constructor.
   * @param w width
   * @param h height
   */
  Grid(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
    // int size = this->size();
    quantities = new T[w*h];
    setForEach([](unsigned int, unsigned int){
      return T(0);
    });
  };
  
  ~Grid(){
    delete[] quantities;
  }

  /**
   * Size.
   * @returns the total number of cells
   */
  unsigned int size() const{
    return w*h;
  };


  /**
   * Function in order to set each cell in a grid using a lambda.
   * @param func Function to apply for each cell
   */
  void setForEach(const std::function< T (unsigned int i, unsigned int j)> func){
    #pragma omp parallel for collapse(2)
    for(auto j = 0u; j < h; j++){
      for(auto i = 0u; i < w; i++){
        set(i,j, func(i,j));
      }
    }
  }


  T get(unsigned int i) const{
    return quantities[i];
  }

  /**
   * Get value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T get(unsigned int i, unsigned int j) const{
    return quantities[indexTranslation(i,j)];
  };

  inline T get(GridCoordinate c) const{
    return get(c.x, c.y);
  };

  inline T safeGet(GridCoordinate c) const {
    return safeGet(c.x, c.y);
  };

  inline T clampGet(GridCoordinate c) const {
    return clampGet(c.x, c.y);
  };

  inline unsigned int indexTranslation(unsigned int i, unsigned int j) const{
    return j*w + i;
  }

  T clampGet(int i, int j) const {
    i = (i < 0) ? 0 : (i >= w) ? w-1 : i;
    j = (j < 0) ? 0 : (j >= h) ? h-1 : j;
    return get(i, j);
  };

  /**
   * Gets the value of a stored quantity, sets the value to T(0) if indicies are outside bounds
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T safeGet(int i, int j) const{
    if(i < 0 || j < 0 || i > int(w - 1) || j > int(h - 1) ){
      return T(0);
    }
    return get(i,j);
  };


  inline bool isValid(int i, int j) const{
    return (
      i >= 0 && i < w &&
      j >= 0 && j < h);
  }

  inline bool isValid(GridCoordinate c) const{
    return isValid(c.x, c.y);
  }


  /**
   * Set value of the stored quantity.
   */
  void set(unsigned int i, unsigned int j, T value) {
    quantities[j*w + i] = value;
  };

  inline void set(GridCoordinate c, T value) {
    set(c.x, c.y, value);
  };

  unsigned int getW() const{
    return w;
  }
  unsigned int getH() const{
    return h;
  }

 protected:
  unsigned int w, h;
  T *quantities;
  
};
