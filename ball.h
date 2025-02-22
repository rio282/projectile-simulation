#ifndef BALL_H
#define BALL_H

#include <SDL.h>
#include <stdbool.h>

#define MAX_BALLS 16
#define BALL_RADIUS 12 // default is 12
#define BALL_SPEED 10.0f
#define BALL_BOUNCE 0.75f
#define BALL_IDLE_LIFETIME_MS 3000
#define DISTANCE_SCALE_THRESHOLD 200.0f // distance at which scaling kicks in
#define DISTANCE_SCALE_EXPONENT 1.25f // adjust this for more/less curvature

typedef struct {
    SDL_FPoint pos;
    SDL_FPoint vel;
    bool visible;
    bool idle;
    unsigned short remaining_lifetime;
} Ball;

void UpdateBalls(Ball (*balls)[MAX_BALLS]);

void ShootBall(Ball *ball, const SDL_Point *m_pos, const SDL_Point *anchor_point);

void HandleCollision(Ball *a, Ball *b);

size_t getNextAvailableBallIndex(const Ball (*balls)[MAX_BALLS]);

#endif
