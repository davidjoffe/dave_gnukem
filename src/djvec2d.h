/*
Copyright (C) 2023 David Joffe
djvec2d.h

Template 2D vector class
*/
#pragma once
#ifndef _DJVEC2D_H_
#define _DJVEC2D_H_

template <typename T>
class djVec2d {
public:
    T x;
    T y;

    djVec2d() : x(0), y(0) {}
    djVec2d(T x, T y) : x(x), y(y) {}
};

typedef djVec2d<int> djVec2dI;
typedef djVec2d<float> djVec2dF;
typedef djVec2d<double> djVec2dD;

#endif // _DJVEC2D_H_
