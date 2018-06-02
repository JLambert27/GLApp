// NxN-dimensional matrix/inverse pair transformation operations
// inline functions and function declarations
#ifndef Xform_inl
#define Xform_inl

#include "Xform.hpp"
#include "Mat.inl"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// matrix size
template <typename T, int N>
inline int dimensions(const Xform<T,N> &p) {
    return N;
}

//////////////////////////////////////////////////////////////////////
// addition and subtraction
template <typename T, int N>
inline Xform<T,N> operator+(const Xform<T,N> &m1, const Xform<T,N> &m2) {
    Xform<T,N> result;
    result.matrix = m1.matrix + m2.matrix;
    result.inverse = m1.inverse + m2.inverse;
}

template <typename T, int N>
inline Xform<T,N> operator-(const Xform<T,N> &m1, const Xform<T,N> &m2) {
    Xform<T,N> result;
    result.matrix = m1.matrix - m2.matrix;
    result.inverse = m1.inverse - m2.inverse;
}

//////////////////////////////////////////////////////////////////////
// scalar multiplication and division
template <typename T, int N>
inline Xform<T,N> operator*(T s, const Xform<T,N> &m) {
    Xform<T,N> result;
    result.matrix = s*m.matrix;
    result.inverse = m.inverse / s;
    return result;
}

template <typename T, int N>
inline Xform<T,N> operator*(const Xform<T,N> &m, T s) {
    Xform<T,N> result;
    result.matrix = m.matrix * s;
    result.inverse = m.inverse / s;
    return result;
}

template <typename T, int N>
inline Xform<T,N> operator/(const Xform<T,N> &m, T s) {
    Xform<T,N> result;
    result.matrix = m.matrix / s;
    result.inverse = m.inverse * s;
    return result;
}

//////////////////////////////////////////////////////////////////////
// matrix*matrix
template <typename T, int N>
inline Xform<T,N> operator*(const Xform<T,N> &m1, const Xform<T,N> &m2) {
    Xform<T,N> result;
    result.matrix = m1.matrix * m2.matrix;
    result.inverse = m2.inverse * m1.inverse;
    return result;
}


//////////////////////////////////////////////////////////////////////
// Build functions. Not done as constructors, so the matrix will be a
// POD (plain old data) type for pre-C++11. Among other things,
// guarantees the type will be exactly equivalent to an array in
// memory.

// note that OpenGL uses matrices in column-major format, so each
// row in the source code is one COLUMN in the matrix

// build 4x4 perspective from field of view (in radians), 
// x/y aspect ratio, and near/far clipping planes
Xform4f perspective4fp(float fov, float aspect, float near, float far);

// build x, y, or z axis rotation for angle (in radians)
Xform4f xrotate4fp(float angle);
Xform4f yrotate4fp(float angle);
Xform4f zrotate4fp(float angle);

// build 4x4 scale matrix
Xform4f scale4fp(Vec3f s);

// build 4x4 translation matrix
Xform4f translate4fp(Vec3f t);

#endif
