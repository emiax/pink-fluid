#pragma once
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
 OrdinalGrid(unsigned int w, unsigned int h, unsigned int d) : Grid<T>(w, h, d) {};

 /**
   * Get the linearly interpolated value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   * @param k, the position along the y axis (d)
   */
  T getLerp(float i, float j, float k) const{
    if (i > this->w - 1) i = this->w - 1;
    if (j > this->h - 1) j = this->h - 1;
    if (k > this->d - 1) k = this->d - 1;
    if (i < 0) i = 0;
    if (j < 0) j = 0;
    if (k < 0) k = 0;
    unsigned int lowerI = floor(i);
    unsigned int upperI = ceil(i);
    unsigned int lowerJ = floor(j);
    unsigned int upperJ = ceil(j);
    unsigned int lowerK = floor(k);
    unsigned int upperK = ceil(k);
    
    float ti = i - (int)i;
    float tj = j - (int)j;
    float tk = k - (int)k;
    
    T v000 = this->get(lowerI, lowerJ, lowerK);
    T v001 = this->get(lowerI, lowerJ, upperK);
    T v010 = this->get(lowerI, upperJ, lowerK);
    T v011 = this->get(lowerI, upperJ, upperK);
    T v100 = this->get(upperI, lowerJ, lowerK);
    T v101 = this->get(upperI, lowerJ, upperK);
    T v110 = this->get(upperI, upperJ, lowerK);
    T v111 = this->get(upperI, upperJ, upperK);

    T v00 = lerp(v000, v001, tk);
    T v01 = lerp(v010, v011, tk);
    T v10 = lerp(v100, v101, tk);
    T v11 = lerp(v110, v111, tk);
    
    T v0 = lerp(v00, v01, tj);
    T v1 = lerp(v10, v11, tj);
    
    return lerp(v0, v1, ti);
  }

  /**
   * Get the Catmull-Rom interpolated value of the stored quantity.
   * @param i, the position along the x axis (w)
   * @param j, the position along the y axis (h)
   * @param k, the position along the z axis (d)
   */
  T getCrerp(float i, float j, float k) const{
    unsigned int i0 = floor(i)-1;
    unsigned int i1 = floor(i);
    unsigned int i2 = ceil(i);
    unsigned int i3 = ceil(i)+1;

    unsigned int j0 = floor(j)-1;
    unsigned int j1 = floor(j);
    unsigned int j2 = ceil(j);
    unsigned int j3 = ceil(j)+1;

    unsigned int k0 = floor(k)-1;
    unsigned int k1 = floor(k);
    unsigned int k2 = ceil(k);
    unsigned int k3 = ceil(k)+1;

    float ti = i - floor(i);
    float tj = j - floor(j);
    float tk = k - floor(k);
    
    T v000 = this->safeGet(i0, j0, k0);
    T v001 = this->safeGet(i0, j0, k1);
    T v002 = this->safeGet(i0, j0, k2);
    T v003 = this->safeGet(i0, j0, k3);

    T v010 = this->safeGet(i0, j1, k0);
    T v011 = this->safeGet(i0, j1, k1);
    T v012 = this->safeGet(i0, j1, k2);
    T v013 = this->safeGet(i0, j1, k3);

    T v020 = this->safeGet(i0, j2, k0);
    T v021 = this->safeGet(i0, j2, k1);
    T v022 = this->safeGet(i0, j2, k2);
    T v023 = this->safeGet(i0, j2, k3);

    T v030 = this->safeGet(i0, j3, k0);
    T v031 = this->safeGet(i0, j3, k1);
    T v032 = this->safeGet(i0, j3, k2);
    T v033 = this->safeGet(i0, j3, k3);
    
    T v100 = this->safeGet(i1, j0, k0);
    T v101 = this->safeGet(i1, j0, k1);
    T v102 = this->safeGet(i1, j0, k2);
    T v103 = this->safeGet(i1, j0, k3);

    T v110 = this->safeGet(i1, j1, k0);
    T v111 = this->safeGet(i1, j1, k1);
    T v112 = this->safeGet(i1, j1, k2);
    T v113 = this->safeGet(i1, j1, k3);

    T v120 = this->safeGet(i1, j2, k0);
    T v121 = this->safeGet(i1, j2, k1);
    T v122 = this->safeGet(i1, j2, k2);
    T v123 = this->safeGet(i1, j2, k3);

    T v130 = this->safeGet(i1, j3, k0);
    T v131 = this->safeGet(i1, j3, k1);
    T v132 = this->safeGet(i1, j3, k2);
    T v133 = this->safeGet(i1, j3, k3);
    
    T v200 = this->safeGet(i2, j0, k0);
    T v201 = this->safeGet(i2, j0, k1);
    T v202 = this->safeGet(i2, j0, k2);
    T v203 = this->safeGet(i2, j0, k3);

    T v210 = this->safeGet(i2, j1, k0);
    T v211 = this->safeGet(i2, j1, k1);
    T v212 = this->safeGet(i2, j1, k2);
    T v213 = this->safeGet(i2, j1, k3);

    T v220 = this->safeGet(i2, j2, k0);
    T v221 = this->safeGet(i2, j2, k1);
    T v222 = this->safeGet(i2, j2, k2);
    T v223 = this->safeGet(i2, j2, k3);

    T v230 = this->safeGet(i2, j3, k0);
    T v231 = this->safeGet(i2, j3, k1);
    T v232 = this->safeGet(i2, j3, k2);
    T v233 = this->safeGet(i2, j3, k3);
    
    T v300 = this->safeGet(i3, j0, k0);
    T v301 = this->safeGet(i3, j0, k1);
    T v302 = this->safeGet(i3, j0, k2);
    T v303 = this->safeGet(i3, j0, k3);

    T v310 = this->safeGet(i3, j1, k0);
    T v311 = this->safeGet(i3, j1, k1);
    T v312 = this->safeGet(i3, j1, k2);
    T v313 = this->safeGet(i3, j1, k3);

    T v320 = this->safeGet(i3, j2, k0);
    T v321 = this->safeGet(i3, j2, k1);
    T v322 = this->safeGet(i3, j2, k2);
    T v323 = this->safeGet(i3, j2, k3);

    T v330 = this->safeGet(i3, j3, k0);
    T v331 = this->safeGet(i3, j3, k1);
    T v332 = this->safeGet(i3, j3, k2);
    T v333 = this->safeGet(i3, j3, k3);

    T v00 = crer(v000,v001,v002,v003,tk);
    T v01 = crer(v010,v011,v012,v013,tk);
    T v02 = crer(v020,v021,v022,v023,tk);
    T v03 = crer(v030,v031,v032,v033,tk);

    T v10 = crer(v100,v101,v102,v103,tk);
    T v11 = crer(v110,v111,v112,v113,tk);
    T v12 = crer(v120,v121,v122,v123,tk);
    T v13 = crer(v130,v131,v132,v133,tk);

    T v20 = crer(v200,v201,v202,v203,tk);
    T v21 = crer(v210,v211,v212,v213,tk);
    T v22 = crer(v220,v221,v222,v223,tk);
    T v23 = crer(v230,v231,v232,v233,tk);

    T v30 = crer(v300,v301,v302,v303,tk);
    T v31 = crer(v310,v311,v312,v313,tk);
    T v32 = crer(v320,v321,v322,v323,tk);
    T v33 = crer(v330,v331,v332,v333,tk);

    T v0 = crer(v00,v01,v02,v03,tj);
    T v1 = crer(v10,v11,v12,v13,tj);
    T v2 = crer(v20,v21,v22,v23,tj);
    T v3 = crer(v30,v31,v32,v33,tj);
    
    return crer(v0,v1,v2,v3,ti);
  }
  ~OrdinalGrid(){
  }
  /**
   * A thin frontend for T getInterpolated(float i, float j)
   * @param p, A vector to interpolate from
   */
  T getLerp(glm::vec3 p) const{
    return getLerp(p.x, p.y, p.z);
  }

  T getCrerp(glm::vec3 p) const{
    return getCrerp(p.x, p.y, p.z);
  }

  T getInterpolated(float x, float y, float z) const {
    return getLerp(x, y, z);
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
