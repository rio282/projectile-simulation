#include "ball.h"
#include "utils.h"
#include "window.h"
#include "world.h"
#include <math.h>


void UpdateBalls(Ball (*balls)[MAX_BALLS]) {
    for (size_t i = 0; i < MAX_BALLS; ++i) {
        Ball *ball = &(*balls)[i];
        if (!ball->visible) continue;

        // check if the ball is idle, if so: reduce its lifetime
        if (ball->idle) {
            ball->remaining_lifetime -= FRAME_DELAY_MS;

            if (ball->remaining_lifetime <= FRAME_DELAY_MS) {
                // reset
                ball->visible = false;
                ball->idle = false;
                ball->remaining_lifetime = BALL_IDLE_LIFETIME_MS;
            }

            continue;
        }

        // update the ball position
        ball->vel.y += SDL_STANDARD_GRAVITY * FRAME_TIME_S; // account for timeskip

        ball->pos.x += ball->vel.x;
        ball->pos.y += ball->vel.y;

        // collision check
        for (size_t j = 0; j < MAX_BALLS; ++j) {
            if (i == j) continue;

            Ball *other = &(*balls)[j];
            if (other->visible && !other->idle) {
                HandleCollision(ball, other);
            }
        }

        // boundary checking horizontal
        if (ball->pos.x < BALL_RADIUS || ball->pos.x > WIN_WIDTH - BALL_RADIUS) {
            ball->vel.x = -ball->vel.x * BALL_BOUNCE;
            ball->pos.x = clamp(ball->pos.x, BALL_RADIUS, WIN_WIDTH - BALL_RADIUS);
        }

        // boundary checking vertical
        if (ball->pos.y < BALL_RADIUS || ball->pos.y > WIN_HEIGHT - BALL_RADIUS) {
            ball->vel.y = -ball->vel.y * BALL_BOUNCE;
            ball->pos.y = clamp(ball->pos.y, BALL_RADIUS, WIN_HEIGHT - BALL_RADIUS);

            // if ball is almost at rest vertically
            if (fabsf(ball->vel.y) < 1.0f) {
                ball->vel.x *= FLOOR_FRICTION;

                if (fabsf(ball->vel.x) < 0.0125f) {
                    // stop ball completely if horizontal velocity is very small
                    ball->vel.x = 0;
                    ball->idle = true;
                }
            }
        }
    }
}

void ShootBall(Ball *ball, const SDL_Point *m_pos, const SDL_Point *anchor_point) {
    ball->idle = false;
    ball->visible = true;
    ball->remaining_lifetime = BALL_IDLE_LIFETIME_MS;

    ball->pos = (SDL_FPoint) {
            .x = (float) anchor_point->x,
            .y = (float) anchor_point->y
    };

    const float magnitude = hypotenuse(
            m_pos->x,
            m_pos->y,
            anchor_point->x,
            anchor_point->y
    );

    if (magnitude > 0) {
        const float powerScale = 1.0f + powf(
                fmaxf(magnitude - DISTANCE_SCALE_THRESHOLD, 0) / 100.0f,
                DISTANCE_SCALE_EXPONENT
        );

        ball->vel.x = (-((float) (m_pos->x - anchor_point->x) / magnitude) * BALL_SPEED) * powerScale;
        ball->vel.y = (-((float) (m_pos->y - anchor_point->y) / magnitude) * BALL_SPEED) * powerScale;
    }
}

void HandleCollision(Ball *a, Ball *b) {
    float dx = b->pos.x - a->pos.x;
    float dy = b->pos.y - a->pos.y;
    float distance = sqrtf(dx * dx + dy * dy);

    // check if balls are colliding
    if (distance < BALL_RADIUS * 2.0f) {  // or a.r + b.r
        // normalize collision vector
        float nx = dx / distance;
        float ny = dy / distance;

        // calculate relative velocity in direction of collision
        float dvx = b->vel.x - a->vel.x;
        float dvy = b->vel.y - a->vel.y;
        float dotProduct = dvx * nx + dvy * ny;

        // if the balls are moving apart -> no need for collision resolve
        if (dotProduct > 0) {
            return;
        }

        // calc impulse scalar with the coefficient of restitution
        // float impulse = (2.0f * dotProduct) / (a->mass + b->mass);
        float impulse = -(1 + BALL_BOUNCE) * dotProduct / 2.0f; // divided by 2 for equal mass assumption

        // update velocities based on impulse
        a->vel.x -= impulse * nx * 0.5f; // a->vel.x -= impulse * b->mass * nx;
        a->vel.y -= impulse * ny * 0.5f;
        b->vel.x += impulse * nx * 0.5f;
        b->vel.y += impulse * ny * 0.5f;

        // prevent sticking
        float overlap = 0.5f * (BALL_RADIUS * 2.0f - distance);
        a->pos.x -= overlap * nx;
        a->pos.y -= overlap * ny;
        b->pos.x += overlap * nx;
        b->pos.y += overlap * ny;
    }
}

size_t getNextAvailableBallIndex(const Ball (*balls)[MAX_BALLS]) {
    for (size_t i = 0; i < MAX_BALLS; ++i) if (!(*balls)[i].visible) return i;
    return -1;
}
