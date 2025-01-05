#include <stdbool.h>

#define SDL_MAIN_HANDLED // needs to be set before SDL.h is imported

#include <SDL.h>
#include "ball.h"
#include "render.h"
#include "window.h"


Ball balls[MAX_BALLS] = {0};
SDL_Point anchor_point = {};


int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
            WIN_TITLE,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WIN_WIDTH,
            WIN_HEIGHT,
            0
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool running = true;
    bool paused = false;
    SDL_Event event;

    bool m_down = false;
    SDL_Point mouse_pos = {};

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                anchor_point = mouse_pos;
                m_down = true;
            }
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                m_down = false;
                ShootBall(&balls[getNextAvailableBallIndex(&balls)], &mouse_pos, &anchor_point);
            }
            if (event.type == SDL_MOUSEMOTION) {
                mouse_pos.x = event.button.x;
                mouse_pos.y = event.button.y;
            }
            if (event.type == SDL_KEYUP) {
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
            UpdateBalls(&balls);

            SDL_SetRenderDrawColor(renderer, 64, 63, 64, 255);
            SDL_RenderClear(renderer);

            SetRenderColor(renderer, 0xFFFFFFFF);
            RenderBalls(renderer, &balls);

            if (m_down && getNextAvailableBallIndex(&balls) != -1) {
                RenderBallShooter(renderer, &mouse_pos, &anchor_point);
            }

            SDL_RenderPresent(renderer);
        }

        SDL_Delay(FRAME_DELAY_MS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
