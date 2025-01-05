#include "utils.h"
#include <math.h>

float clamp(const float current, const float lower, const float upper) {
    return fmaxf(lower, fminf(current, upper));
}

float hypotenuse(const int x1, const int y1, const int x2, const int y2) {
    const int dx = x2 - x1;
    const int dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

Uint32 ndstToGradientColor(const float normalized_dst) {
    Uint8 r = (Uint8) (255 * normalized_dst);
    Uint8 g = (Uint8) (255 * (1.0f - normalized_dst));
    return (r << 24) | (g << 16) | (255);
}

float normalizeScalar(const float dst, const float max_dst) {
    return fminf(dst / max_dst, 1.0f);
}
