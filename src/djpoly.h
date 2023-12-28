/*
Copyright (C) 2023 David Joffe
djpoly.h

Template polygon class (ordered vector of points, for example to represent a polygon)
*/
#pragma once
#ifndef DJPOLY_H
#define DJPOLY_H

#include "djvec2d.h"
#include <vector>

template <typename T>
class djVecList {
public:
    std::vector<djVec2d<T>> vecList;

    djVecList() {}
};

#endif // DJPOLY_H
