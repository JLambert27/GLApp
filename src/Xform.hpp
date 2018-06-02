// NxN-dimensional matrix and its inverse for transforms
#ifndef Xform_hpp
#define Xform_hpp

#include "Mat.hpp"

//////////////////////////////////////////////////////////////////////
// pair of NxN matrices (matrix and inverse)
template <typename T, int N>
struct Xform {
    typedef T value_type;

    Mat<T,N> matrix;
    Mat<T,N> inverse;
};

//////////////////////////////////////////////////////////////////////
// 2x2, 3x3 and 4x4 float specializations
typedef Xform<float,2> Xform2f;
typedef Xform<float,3> Xform3f;
typedef Xform<float,4> Xform4f;

#endif
