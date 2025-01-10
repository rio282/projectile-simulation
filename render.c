#include <math.h>
#include "render.h"
#include "utils.h"
#include "window.h"
#include "world.h"


void SetRenderColor(SDL_Renderer *renderer, const Uint32 color) {
    Uint8 r = (color >> 24) & 0xFF;
    Uint8 g = (color >> 16) & 0xFF;
    Uint8 b = (color >> 8) & 0xFF;
    Uint8 a = (color) & 0xFF;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void FillCircle(SDL_Renderer *renderer, const SDL_Point p, const int r) {
    const int r_sq = r * r;
    int dst_sq;

    for (int x = p.x - r; x <= p.x + r; ++x) {
        for (int y = p.y - r; y <= p.y + r; ++y) {
            dst_sq = (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y);
            if (dst_sq < r_sq) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

void RenderBalls(SDL_Renderer *renderer, Ball (*balls)[MAX_BALLS]) {
    for (size_t i = 0; i < MAX_BALLS; ++i) {
        Ball *ball = &(*balls)[i];
        if (!ball->visible) continue;

        Uint32 color = 0xFFFFFFFF;
        if (ball->remaining_lifetime != BALL_IDLE_LIFETIME_MS) {
            const float alpha = 1.0f - normalizeScalar(BALL_IDLE_LIFETIME_MS - ball->remaining_lifetime,
                                                       BALL_IDLE_LIFETIME_MS);
            color = (0xFF << 24) | (0xFF << 16) | (0xFF << 8) | (Uint8) (alpha * 255);
        }

        SetRenderColor(renderer, color);
        FillCircle(
                renderer,
                (SDL_Point) {.x = (int) ball->pos.x, .y = (int) ball->pos.y},
                BALL_RADIUS
        );
    }
}

void DrawDottedCircleLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, const int step, const int r) {
    const int dx = abs(x2 - x1);
    const int dy = abs(y2 - y1);
    const int sx = (x1 < x2) ? 1 : -1;
    const int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int s_count = 0;

    while (true) {
        if (s_count % step == 0) {
            // only draw if the s_count is a multiple of step
            FillCircle(
                    renderer,
                    (SDL_Point) {.x = x1, .y = y1},
                    r
            );
        }

        s_count++;

        if (x1 == x2 && y1 == y2) break;

        int e2 = err * 2;

        // update x and/or y based on the error value
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void RenderBallShooter(SDL_Renderer *renderer, const SDL_Point *m_pos, const SDL_Point *anchor_point) {
    // before
    const float dst = hypotenuse(
            m_pos->x, m_pos->y,
            anchor_point->x, anchor_point->y
    );
    const float normalized_dst = normalizeScalar(dst, WIN_HEIGHT);
    const float smooth_dst = normalized_dst * normalized_dst;
    const Uint32 dst_indication_color = ndstToGradientColor(smooth_dst);

    SetRenderColor(renderer, dst_indication_color);
    DrawDottedCircleLine(
            renderer,
            m_pos->x,
            m_pos->y,
            anchor_point->x,
            anchor_point->y,
            BALL_RADIUS * 2,
            BALL_RADIUS / 2
    );

    // TODO: refactor this part
    // after
    if (DRAW_TRAJECTORY_PREVIEW) {
        const float steps_calc_limit = 248;
        const int max_steps = (int) clamp(steps_calc_limit * smooth_dst, 0.0f, steps_calc_limit);

        SDL_FPoint current_position = {
                .x = (float) anchor_point->x,
                .y = (float) anchor_point->y
        };
        SDL_FPoint last_position = current_position;
        float velocity_x = (float) (anchor_point->x - m_pos->x) * smooth_dst;
        float velocity_y = (float) (anchor_point->y - m_pos->y) * smooth_dst;

        for (int steps = 0; steps < max_steps; ++steps) {
            float alpha = 1.0f - normalizeScalar((float) steps, (float) max_steps);
            Uint32 color = (0xE8 << 24) | (0xE8 << 16) | (0xE8 << 8) | (Uint8) (alpha * 255);
            SetRenderColor(renderer, color);

            // ripped straight from UpdateBalls()
            velocity_y += SDL_STANDARD_GRAVITY * FRAME_TIME_S;

            current_position.x += velocity_x * FRAME_TIME_S;
            current_position.y += velocity_y * FRAME_TIME_S;

            if (current_position.x < BALL_RADIUS || current_position.x > WIN_WIDTH - BALL_RADIUS) {
                velocity_x = -velocity_x * BALL_BOUNCE;
                current_position.x = clamp(current_position.x, BALL_RADIUS, WIN_WIDTH - BALL_RADIUS);
            }

            if (current_position.y < BALL_RADIUS || current_position.y > WIN_HEIGHT - BALL_RADIUS) {
                velocity_y = -velocity_y * BALL_BOUNCE;
                current_position.y = clamp(current_position.y, BALL_RADIUS, WIN_HEIGHT - BALL_RADIUS);
                if (fabsf(velocity_y) < 1.0f) {
                    velocity_x *= FLOOR_FRICTION;
                    if (fabsf(velocity_x) < 0.0125f) {
                        velocity_x = 0;
                        steps = max_steps; // break on next iteration
                    }
                }
            }

            // slow!!!
//            FillCircle(
//                    renderer,
//                    (SDL_Point) {
//                            .x  = (int) current_position.x,
//                            .y  = (int) current_position.y
//                    },
//                    BALL_RADIUS
//            );

            // boring!!!
//        SDL_RenderDrawPointF(renderer, current_position.x, current_position.y);

            // whatever man...
            SDL_RenderDrawLineF(
                    renderer,
                    last_position.x, last_position.y,
                    current_position.x, current_position.y
            );

            last_position = current_position;
        }
    }

    // draw on top
    SetRenderColor(renderer, 0xFFFFFFFF);
    FillCircle(renderer, *m_pos, BALL_RADIUS * 0.75);

    SetRenderColor(renderer, dst_indication_color);
    FillCircle(renderer, *anchor_point, BALL_RADIUS);
}
