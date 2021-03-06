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
    // int size = this->size();
    quantities = new T[w*h*d];
    for(auto i = 0u; i < size(); i++){
      quantities[i] = T(0);
    }
  };

  Grid(const Grid& origin) {
    this->w = origin.w;
    this->h = origin.h;
    this->d = origin.d;

    quantities = new T[w*h*d];
    for (auto i = 0u; i < size(); i++) {
      quantities[i] = origin.quantities[i];
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
  void setForEach(const std::function< T (unsigned int i, unsigned int j, unsigned int k)> func){
    #pragma omp parallel for collapse(3)
    for(auto k = 0u; k < d; k++){
      for(auto j = 0u; j < h; j++){
        for(auto i = 0u; i < w; i++){
          set(i, j, k, func(i, j, k));
        }
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

  T get(unsigned int i, unsigned int j, unsigned int k) const{
    assert(i >= 0);
    assert(j >= 0);
    assert(k >= 0);
    assert(i < w);
    assert(j < h);
    assert(k < d);
    return quantities[k*w*h + j*w + i];
  };

  inline T get(GridCoordinate c) const{
    return this->get(c.x, c.y, c.z);
  };

  inline T safeGet(GridCoordinate c) const {
    return this->safeGet(c.x, c.y, c.z);
  };

  inline T clampGet(GridCoordinate c) const {
    return this->clampGet(c.x, c.y, c.z);
  };

  inline unsigned int indexTranslation(unsigned int i, unsigned int j, unsigned int k) const{
    return k*w*h + j*w + i;
  }

  T clampGet(int i, int j, int k) const {
    i = (i < 0) ? 0 : (i >= w) ? w-1 : i;
    j = (j < 0) ? 0 : (j >= h) ? h-1 : j;
    k = (k < 0) ? 0 : (k >= d) ? d-1 : k;
    return this->get(i, j, k);
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
    return this->get(i, j, k);
  };


  inline bool isValid(int i, int j, int k) const{
    return (
      i >= 0 && i < w &&
      j >= 0 && j < h && 
      k >= 0 && k < d);
  }

  inline bool isValid(GridCoordinate c) const{
    return isValid(c.x, c.y, c.z);
  }


  /**
   * Set value of the stored quantity.
   */
  void set(unsigned int i, unsigned int j, unsigned int k, T value) {
    quantities[k*w*h + j*w + i] = value;
  };

  inline void set(GridCoordinate c, T value) {
    this->set(c.x, c.y, c.z, value);
  };

  unsigned int getW() const{
    return w;
  }
  unsigned int getH() const{
    return h;
  }

  unsigned int getD() const{
    return d;
  }

  std::ostream& write(std::ostream& stream){
    stream.write(reinterpret_cast<char*>(&w), sizeof(w));
    stream.write(reinterpret_cast<char*>(&h), sizeof(h));
    stream.write(reinterpret_cast<char*>(&d), sizeof(d));
    long dataLength = w * h * d;
    stream.write(reinterpret_cast<char*>(quantities), sizeof(T)*dataLength);
    return stream;
  }

  std::istream& read(std::istream& stream){
    stream.read(reinterpret_cast<char*>(&w), sizeof(w));
    stream.read(reinterpret_cast<char*>(&h), sizeof(h));
    stream.read(reinterpret_cast<char*>(&d), sizeof(d));
    long dataLength = w * h * d;
    stream.read(reinterpret_cast<char*>(quantities), sizeof(T)*dataLength);
    return stream;
  }
  
 protected:
  unsigned int w, h, d;
  T *quantities;
  
};
