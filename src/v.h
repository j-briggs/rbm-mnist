#pragma once

#include <vector>
#include <numeric>
#include <random>
#include <float.h>

typedef std::vector<float> Vector;

namespace v {
  struct LightVector: public std::pair<float *, float *>
  {
    typedef std::pair<float *, float *> Parent;

    LightVector(){}

    template <class Arg> 
    LightVector(Arg&& arg1, Arg&& arg2): Parent(std::forward<Arg>(arg1), std::forward<Arg>(arg2)) 
    {}

    LightVector(float* arg1, float* arg2): Parent(arg1, arg2) 
    {}

    float *data() const { return first; }
    size_t size() const { return std::distance(first, second); }
    bool empty() const  { return first == second; }

    float& operator[](size_t i) { return *(first + i); }
    float operator[](size_t i) const { return *(first + i); }
  };

  template <class Vector1, class Vector2> inline float dot(const Vector1&x, const Vector2& y) { 
    int m = x.size(); const float *xd = x.data(), *yd = y.data();
    float sum = 0.0;
    while (--m >= 0) sum += (*xd++) * (*yd++);
    return sum;
  }

  // saxpy: x = x + g * y; x = a * x + g * y
  inline void saxpy(Vector& x, float g, const Vector& y) {
    int m = x.size(); float *xd = x.data(); const float *yd = y.data();
    while (--m >= 0) (*xd++) += g * (*yd++);
  }

  inline void saxpy(float a, Vector& x, float g, const Vector& y) {
    int m = x.size(); float *xd = x.data(); const float *yd = y.data();
    while (--m >= 0) { (*xd) = a * (*xd) + g * (*yd); ++xd; ++yd; }
  }

  inline void saxpy2(Vector& x, float g, const Vector& y, float h, const Vector& z) {
    int m = x.size(); float *xd = x.data(); const float *yd = y.data(); const float *zd = z.data();
    while (--m >= 0) { (*xd++) +=  (g * (*yd++)  + h * (*zd++)); }
  }

  inline void scale(Vector& x, float g) {
    int m = x.size(); float *xd = x.data();
    while (--m >= 0) (*xd++) *= g;
  }

#if 0
  inline void addsub(Vector& x, const Vector& y, const Vector& z) {
    int m = x.size(); float *xd = x.data(); const float *yd = y.data(); const float *zd = z.data();
    while (--m >= 0) (*xd++) += ((*yd++) - (*zd++));
  }
#endif

  inline void unit(Vector& x) {
    float len = ::sqrt(dot(x, x));
    if (len == 0) return;

    int m = x.size(); float *xd = x.data();
    while (--m >= 0) (*xd++) /= len;
  }

  inline bool isfinite(const Vector& x) { 
    Vector::const_iterator iStart = x.begin(), iEnd = x.end();
    while (iStart != iEnd)
    {
      if (! _finite(*iStart)) return false;
      ++iStart;
    }
    return true;
  }

}

