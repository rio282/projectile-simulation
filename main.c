#include <stdlib.h>
#include <stdbool.h>


#define SDL_MAIN_HANDLED // needs to be set before SDL.h is imported

#include <SDL.h>

// window
#define WIN_TITLE "Projectile Simulation"
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define TARGET_FPS 60

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
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
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    // before enter loop
    const int frame_delay_ms = 1000 / TARGET_FPS;
    bool running = true;

    SDL_Log("Init complete.\n");

    // update loop
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_Q:
                            running = false;
                            break;
                        case SDL_SCANCODE_SPACE:
                            break;
                        default:
                            break;
                    }
            }
        }

        SDL_SetRenderDrawColor(renderer, 128, 127, 128, 255);
        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(frame_delay_ms);
    }

    // cleanup
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;
}
