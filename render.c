#include "render.h"
#include "utils.h"
#include "window.h"


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
        SetRenderColor(renderer, color);
        FillCircle(renderer, (SDL_Point) {.x = (int) ball->pos.x, .y = (int) ball->pos.y}, BALL_RADIUS);
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

    FillCircle(renderer, *anchor_point, BALL_RADIUS);

    SetRenderColor(renderer, 0xFFFFFFFF);
    FillCircle(renderer, *m_pos, BALL_RADIUS * 0.75);
}
