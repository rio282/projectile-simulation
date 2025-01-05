#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>

float clamp(const float current, const float lower, const float upper);

float hypotenuse(const int x1, const int y1, const int x2, const int y2);

Uint32 ndstToGradientColor(const float normalized_dst);

float normalizeScalar(const float dst, const float max_dst);


#endif
