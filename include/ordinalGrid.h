#include <grid.h>
#include <cmath>
#include <glm/glm.hpp>
template <class T>
class OrdinalGrid : public Grid<T> {
 public:
  /**
   * Constructor.
   * @param w width
   * @param h height
   */
   OrdinalGrid(unsigned int w, unsigned int h) : Grid<T>(w, h) {};


  /**
   * Get the linearly interpolated value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T getInterpolated(float i, float j) const{
    unsigned int lowerI = floor(i);
    unsigned int upperI = ceil(i);
    unsigned int lowerJ = floor(j);
    unsigned int upperJ = ceil(j);

    float ti = fmod(i, 1.0);
    float tj = fmod(j, 1.0);

    T v00 = this->get(lowerI, lowerJ);
    T v01 = this->get(lowerI, upperJ);
    T v10 = this->get(upperI, lowerJ);
    T v11 = this->get(upperI, upperJ);

    T v0 = lerp(v00, v01, tj);
    T v1 = lerp(v10, v11, tj);

    return lerp(v0, v1, ti);
  }
  
  /**
   * A thin frontend for T getInterpolated(float i, float j)
   * @param p, A vector to interpolate from
   */
  T getInterpolated(glm::vec2 p) const{
    return getInterpolated(p.x,p.y);
  }

 private:
  /**
   * Linear interpolation.
   * @param a The first value
   * @param b The second value
   * @param t A number between 0 and 1.
   */
  T lerp(T a, T b, float t) const{
    return a*(1.0f - t) + b*t;
  }

};
