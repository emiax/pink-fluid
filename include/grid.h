#include <cmath>

/*******************
 * Class definition.
 *******************/

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
    int size = this->size();
    quantities = new T[w*h];
    for (unsigned int i = 0; i < size; ++i) {
      quantities[i] = 0;
    }
  };


  /**
   * Size.
   * @returns the total number of cells
   */
  unsigned int size() {
    return w*h;
  };


  /**
   * Get value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T get(unsigned int i, unsigned int j) {
    return quantities[j*w + i];
  };


  /**
   * Get the linearly interpolated value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T get(float i, float j) {
    unsigned int lowerI = floor(i);
    unsigned int upperI = ceil(i);
    unsigned int lowerJ = floor(j);
    unsigned int upperJ = ceil(j);
    
    float ti = fmod(i, 1.0);
    float tj = fmod(j, 1.0);
    
    T v00 = get(lowerI, lowerJ);
    T v01 = get(lowerI, upperJ);
    T v10 = get(upperI, lowerJ);
    T v11 = get(upperI, upperJ);

    T v0 = lerp(v00, v01, tj);
    T v1 = lerp(v10, v11, tj);

    return lerp(v0, v1, ti);
  }

  
  /**
   * Set value of the stored quantity.
   */
  T set(unsigned int i, unsigned int j, T value) {
    quantities[j*w + i] = value;
  };


 private:
  unsigned int w, h;
  T *quantities;
  

  /**
   * Linear interpolation.
   * @param a The first value
   * @param b The second value
   * @param t A number between 0 and 1.
   */
  T lerp(T a, T b, float t) {
    return a*(1.0 - t) + b*t;
  }

};
