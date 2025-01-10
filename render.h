#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>
#include "ball.h"

#define DRAW_TRAJECTORY_PREVIEW true

void RenderBalls(SDL_Renderer *renderer, Ball (*balls)[MAX_BALLS]);

void RenderBallShooter(SDL_Renderer *renderer, const SDL_Point *m_pos, const SDL_Point *anchor_point);

void FillCircle(SDL_Renderer *renderer, SDL_Point p, int r);

void SetRenderColor(SDL_Renderer *renderer, Uint32 color);

void DrawDottedCircleLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int step, int r);

#endif
