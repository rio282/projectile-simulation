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

        // update the ball
        ball->vel.y += GRAVITY * FRAME_TIME_S; // account for timeskip

        ball->pos.x += ball->vel.x;
        ball->pos.y += ball->vel.y;

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

size_t getNextAvailableBallIndex(const Ball (*balls)[MAX_BALLS]) {
    for (size_t i = 0; i < MAX_BALLS; ++i) if (!(*balls)[i].visible) return i;
    return -1;
}
