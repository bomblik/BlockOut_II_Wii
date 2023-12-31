// -------------------------------------
// 3x3 Matrix class
// -------------------------------------

#if defined(PLATFORM_PSP)
#include "SDL/SDL_opengl.h"
#elif defined(PLATFORM_PSVITA)
#include "GL/gl.h"
#elif defined(PLATFORM_WII)
#include "GL/gl.h"
#else
#include <SDL_opengl.h>
#endif

#ifndef _GLMATRIXH_
#define _GLMATRIXH_

class GLMatrix {

public:

  GLMatrix();

  void Init33(float _11,float _12,float _13,
              float _21,float _22,float _23,
              float _31,float _32,float _33);

  void Identity();
  void RotateX(float angle);
  void RotateY(float angle);
  void RotateZ(float angle);
  void Translate(float x,float y,float z);
  void Transpose();

  void Multiply(GLMatrix *m1,GLMatrix *m2);
  void Multiply(GLMatrix *m2);

  void TransfomVec(float x,float y,float z,float w,float *rx,float *ry,float *rz,float *rw);

  GLfloat *GetGL();
  void FromGL(GLfloat *m);

  float _11;float _12;float _13;float _14;
  float _21;float _22;float _23;float _24;
  float _31;float _32;float _33;float _34;
  float _41;float _42;float _43;float _44;

};

#endif /* _GLMATRIXH_ */
