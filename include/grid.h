#pragma once
#include <functional>
#include <glm/glm.hpp>
#include <iostream>

typedef glm::i32vec3 GridCoordinate;

template <class T>
class Grid {
 public:

  /**
   * Constructor.
   * @param w width
   * @param h height
   */
  Grid(unsigned int w, unsigned int h, unsigned int d) {
    this->w = w;
    this->h = h;
    this->d = d;
    std::cout << w << "," << h << "," << d << std::endl;
    // int size = this->size();
    quantities = new T[w*h*d];
    for(auto i = 0u; i < size(); i++){
      quantities[i] = T(0);
    }
  };
  
  ~Grid(){
    delete[] quantities;
  }

  /**
   * Size.
   * @returns the total number of cells
   */
  unsigned int size() const{
    return w*h*d;
  };


  /**
   * Function in order to set each cell in a grid using a lambda.
   * @param func Function to apply for each cell
   */
  void setForEach(std::function< T (unsigned int i, unsigned int j, unsigned int k)> func){
    for(auto k = 0u; k < d; k++){
      for(auto j = 0u; j < h; j++){
        for(auto i = 0u; i < w; i++){
          set(i, j, k, func(i, j, k));
        }
      }
    }
  }


  /**
   * Get value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T get(unsigned int i, unsigned int j, unsigned int k) const{
    return quantities[k*w*h + j*w + i];
  };

  inline T get(GridCoordinate c) const{
    return get(c.x, c.y, c.z);
  };

  inline T safeGet(GridCoordinate c) const {
    return safeGet(c.x, c.y, c.z);
  };

  inline T clampGet(GridCoordinate c) const {
    return clampGet(c.x, c.y, c.z);
  };

  T clampGet(int i, int j, int k) const {
    i = (i < 0) ? 0 : (i >= w) ? w-1 : i;
    j = (j < 0) ? 0 : (j >= h) ? h-1 : j;
    k = (k < 0) ? 0 : (k >= d) ? d-1 : j;
    return get(i, j, k);
  };

  /**
   * Gets the value of a stored quantity, sets the value to T(0) if indicies are outside bounds
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T safeGet(int i, int j, int k) const{
    if(i < 0 || j < 0 || k < 0 || i > int(w - 1) || j > int(h - 1) || k > int(d - 1)){
      return T(0);
    }
    return get(i, j, k);
  };

  /**
   * Set value of the stored quantity.
   */
  void set(unsigned int i, unsigned int j, unsigned int k, T value) {
    quantities[k*w*h + j*w + i] = value;
  };

  inline void set(GridCoordinate c, T value) {
    set(c.x, c.y, c.z, value);
  };


 protected:
  unsigned int w, h, d;
  T *quantities;
  
};
