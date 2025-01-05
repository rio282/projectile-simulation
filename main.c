#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define SDL_MAIN_HANDLED // needs to be set before SDL.h is imported

#include <SDL.h>

// window
#define WIN_TITLE "Projectile Simulation"
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define TARGET_FPS 60
#define FRAME_DELAY_MS (1000 / TARGET_FPS)
#define FRAME_TIME_S (1.0f / TARGET_FPS)

// world
#define MAX_BALLS 16
#define BALL_RADIUS 12
#define BALL_SPEED 10.0f
#define BALL_IDLE_LIFETIME_MS 3000
#define GRAVITY 9.81f
#define BOUNCE 0.75f
#define FLOOR_FRICTION 0.95f
#define DISTANCE_SCALE_THRESHOLD 200.0f // distance at which scaling kicks in
#define DISTANCE_SCALE_EXPONENT 1.25f // adjust this for more/less curvature


typedef struct m_State {
    SDL_Point m_pos;
    bool m_down;
} m_State;

typedef struct Ball {
    SDL_FPoint pos;
    SDL_FPoint vel;

    bool visible;
    bool idle;
    unsigned short remaining_lifetime;
} Ball;

float clamp(const float current, const float lower, const float upper) {
    return fmaxf(lower, fminf(current, upper));
}

float hypotenuse(const int x1, const int y1, const int x2, const int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return (float) sqrt(dx * dx + dy * dy);
}

float normalizeScalar(const float dst, const float max_dst) {
    return fminf(dst / max_dst, 1.0f);
}

Uint32 ndstToGradientColor(const float normalized_dst) {
    Uint8 r = (Uint8) (255 * normalized_dst);
    Uint8 g = (Uint8) (255 * (1.0f - normalized_dst));
    Uint8 b = 0;
    Uint8 a = 255;
    return (r << 24) | (g << 16) | (b << 8) | a;
}

void SetRenderColor(SDL_Renderer *renderer, const Uint32 color) {
    Uint8 r = (color >> 24) & 0xFF;
    Uint8 g = (color >> 16) & 0xFF;
    Uint8 b = (color >> 8) & 0xFF;
    Uint8 a = (color) & 0xFF;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void FillCircle(SDL_Renderer *rend, const SDL_Point p, const int r) {
    int r_sq = r * r;
    for (int x = p.x - r; x <= p.x + r; ++x) {
        for (int y = p.y - r; y <= p.y + r; ++y) {
            int dst_sq = (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y);
            if (dst_sq < r_sq) {
                SDL_RenderDrawPoint(rend, x, y);
            }
        }
    }
}

void DrawDottedCircleLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, const int step, const int r) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
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
            ball->vel.x = -ball->vel.x * BOUNCE;
            ball->pos.x = clamp(ball->pos.x, BALL_RADIUS, WIN_WIDTH - BALL_RADIUS);
        }

        // boundary checking vertical
        if (ball->pos.y < BALL_RADIUS || ball->pos.y > WIN_HEIGHT - BALL_RADIUS) {
            ball->vel.y = -ball->vel.y * BOUNCE;
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

void RenderBalls(SDL_Renderer *renderer, Ball (*balls)[MAX_BALLS]) {
    for (size_t i = 0; i < MAX_BALLS; ++i) {
        Ball *ball = &(*balls)[i];
        if (!ball->visible)continue;

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

void RenderBallShooter(SDL_Renderer *renderer, const SDL_Point *m_pos, SDL_Point *anchor_point) {
    float dst = hypotenuse(
            m_pos->x, m_pos->y,
            anchor_point->x, anchor_point->y
    );
    float normalized_dst = normalizeScalar(dst, WIN_HEIGHT);
    float smooth_dst = normalized_dst * normalized_dst;
    Uint32 dst_indication_color = ndstToGradientColor(smooth_dst);

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

size_t getNextAvailableBallIndex(const Ball (*balls)[MAX_BALLS]) {
    for (size_t i = 0; i < MAX_BALLS; ++i) if (!(*balls)[i].visible) return i;
    return -1;
}

void shootBall(Ball *ball, const SDL_Point *m_pos, const SDL_Point *anchor_point) {
    ball->pos = (SDL_FPoint) {
            .x = (float) anchor_point->x,
            .y = (float) anchor_point->y
    };

    float magnitude = hypotenuse(
            m_pos->x,
            m_pos->y,
            anchor_point->x,
            anchor_point->y
    );

    if (magnitude > 0) {
        ball->vel.x = -((float) (m_pos->x - anchor_point->x) / magnitude) * BALL_SPEED;
        ball->vel.y = -((float) (m_pos->y - anchor_point->y) / magnitude) * BALL_SPEED;

        float powerScale = 1.0f + powf(
                fmaxf(magnitude - DISTANCE_SCALE_THRESHOLD, 0) / 100.0f,
                DISTANCE_SCALE_EXPONENT
        );
        ball->vel.x *= powerScale;
        ball->vel.y *= powerScale;

        ball->idle = false;
        ball->visible = true;
    }
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow(
            WIN_TITLE,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WIN_WIDTH,
            WIN_HEIGHT,
            SDL_WINDOW_BORDERLESS | SDL_WINDOW_KEYBOARD_GRABBED
    );

    if (!window) {
        SDL_Log("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool paused = false;

    m_State mouse_state = {};
    SDL_Point anchor_point = {};

    Ball balls[MAX_BALLS];
    for (size_t i = 0; i < MAX_BALLS; ++i) {
        balls[i] = (Ball) {
                .visible = false,
                .idle = false,
                .remaining_lifetime = BALL_IDLE_LIFETIME_MS
        };
    }

    SDL_Log("Init complete.\n");
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (paused) break;
                    mouse_state.m_down = true;
                    anchor_point.x = mouse_state.m_pos.x;
                    anchor_point.y = mouse_state.m_pos.y;
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (paused) break;
                    mouse_state.m_down = false;

                    const size_t next_available = getNextAvailableBallIndex(&balls);
                    if (next_available != -1) {
                        shootBall(&balls[next_available], &mouse_state.m_pos, &anchor_point);
                    }

                    break;

                case SDL_MOUSEMOTION:
                    if (paused) break;
                    mouse_state.m_pos.x = event.motion.x;
                    mouse_state.m_pos.y = event.motion.y;
                    break;

                case SDL_KEYUP:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_Q:
                            running = false;
                            break;
                        case SDL_SCANCODE_SPACE:
                            paused = !paused;
                            break;
                        default:
                            break;
                    }
            }
        }

        if (!paused) {
            // --- UPDATE
            UpdateBalls(&balls);

            // --- DRAW
            SDL_SetRenderDrawColor(renderer, 64, 63, 64, 255);
            SDL_RenderClear(renderer);

            RenderBalls(renderer, &balls);
            if (mouse_state.m_down && getNextAvailableBallIndex(&balls) != -1) {
                // if mouse down and if there's a ball available
                RenderBallShooter(renderer, &mouse_state.m_pos, &anchor_point);
            }

            SDL_RenderPresent(renderer);
        }
        SDL_Delay(FRAME_DELAY_MS);
    }

    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;
}
