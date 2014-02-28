#pragma once
#define GLM_FORCE_RADIANS
#include <grid.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <algorithm>
#include <iostream>


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
  T getLerp(float i, float j) const{
    if (i > this->w - 1) i = this->w - 1;
    if (j > this->h - 1) j = this->h - 1;
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    unsigned int lowerI = floor(i);
    unsigned int upperI = ceil(i);
    unsigned int lowerJ = floor(j);
    unsigned int upperJ = ceil(j);
    
    float ti = i - (int)i;
    float tj = j - (int)j;
    
    T v00 = this->get(lowerI, lowerJ);
    T v01 = this->get(lowerI, upperJ);
    T v10 = this->get(upperI, lowerJ);
    T v11 = this->get(upperI, upperJ);
    
    T v0 = lerp(v00, v01, tj);
    T v1 = lerp(v10, v11, tj);
    
    return lerp(v0, v1, ti);
  }

  /**
   * Get the Catmull-Rom interpolated value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   */
  T getCrerp(float i, float j) const{
    unsigned int i0 = floor(i)-1;
    unsigned int i1 = floor(i);
    unsigned int i2 = ceil(i);
    unsigned int i3 = ceil(i)+1;

    unsigned int j0 = floor(j)-1;
    unsigned int j1 = floor(j);
    unsigned int j2 = ceil(j);
    unsigned int j3 = ceil(j)+1;

    float ti = i - floor(i);
    float tj = j - floor(j);
    
    T v00 = this->safeGet(i0, j0);
    T v01 = this->safeGet(i0, j1);
    T v02 = this->safeGet(i0, j2);
    T v03 = this->safeGet(i0, j3);
    
    T v10 = this->safeGet(i1, j0);
    T v11 = this->safeGet(i1, j1);
    T v12 = this->safeGet(i1, j2);
    T v13 = this->safeGet(i1, j3);

    T v20 = this->safeGet(i2, j0);
    T v21 = this->safeGet(i2, j1);
    T v22 = this->safeGet(i2, j2);
    T v23 = this->safeGet(i2, j3);

    T v30 = this->safeGet(i3, j0);
    T v31 = this->safeGet(i3, j1);
    T v32 = this->safeGet(i3, j2);
    T v33 = this->safeGet(i3, j3);

    T v0 = crer(v00,v10,v20,v30,ti);
    T v1 = crer(v01,v11,v21,v31,ti);
    T v2 = crer(v02,v12,v22,v32,ti);
    T v3 = crer(v03,v13,v23,v33,ti);
    
    return crer(v0,v1,v2,v3,tj);
    /*
      if (i > this->w - 1) i = this->w - 1;
      if (j > this->h - 1) j = this->h - 1;
      if (i < 0) i = 0;
      if (j < 0) j = 0;
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
    
    return lerp(v0, v1, ti);*/
  }
  ~OrdinalGrid(){
  }
  /**
   * A thin frontend for T getInterpolated(float i, float j)
   * @param p, A vector to interpolate from
   */
  T getLerp(glm::vec2 p) const{
    return getLerp(p.x,p.y);
  }

  T getCrerp(glm::vec2 p) const{
    return getCrerp(p.x,p.y);
  }

  T getInterpolated(float x, float y) const {
    return getLerp(x, y);
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
  

  /**
   * 1D Catmull-Rom Interpolation
   * 
   *
   *
   *
   */
  T crer(T x1, T x2, T x3, T x4, float t) const{
    T minT = glm::min(x2,x3);
    T maxT = glm::max(x2,x3);
    float t2 = t*t;
    float t3 = t2*t;
    return glm::clamp(0.5f * (
                              (2.0f * x2) + 
                              (-x1 + x3) * t + 
                              (2.0f * x1 - 5.0f*x2 + 4.0f*x3 - x4) * t2 + 
                              (-x1 + 3.0f*x2 - 3.0f*x3 + x4) * t3
                              ), 
                      minT, maxT);
  }
};
