/*
Copyright (C) 1995-2023 David Joffe
djrect.h

Template rectangle class
*/
#pragma once
#ifndef _DJRECT_H_
#define _DJRECT_H_

// For performance reasons we may want an initialized version of this for graphics stuff? Should we (for safety so programmers are aware) give it a name that indicates such? [low]
template <typename T>
class djRect {
public:
    T x;
    T y;
    T w;
    T h;

    djRect() : x(0), y(0), w(0), h(0) {}
    djRect(T x, T y, T w, T h) : x(x), y(y), w(w), h(h) {}
};

typedef djRect<int> djRectI;
typedef djRect<float> djRectF;
typedef djRect<double> djRectD;

#endif // _DJRECT_H_
