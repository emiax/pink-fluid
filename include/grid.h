#pragma once
#include <functional>
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
    for(auto i = 0; i < size(); i++){
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
    return w*h;
  };


  /**
   * 
   * 
   * 
   */
  void setForEach(std::function< T (unsigned int i, unsigned int j)> func){
    for(unsigned int i =  0; i < w; i++){
      for(unsigned int j = 0; j < h; j++){
        set(i,j, func(i,j));
      }
    }
  }

  /**
   * Get value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T get(unsigned int i, unsigned int j) const{
    return quantities[j*w + i];
  };

  /**
   * Gets the value of a stored quantity, sets the value to T(0) if indicies are outside bounds
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  
  T safeGet(int i, int j) const{
    if(i < 0 || j < 0 || i > w - 1 || j > h - 1 ){
      return T(0);
    }
    return get(i,j);
  }
  /**
   * Set value of the stored quantity.
   */
  void set(unsigned int i, unsigned int j, T value) {
    quantities[j*w + i] = value;
  };


 protected:
  unsigned int w, h;
  T *quantities;
  
};
