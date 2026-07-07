#include "game.h"
#include "render.h"
#include "input.h"
#include "sound.h"
#include "scene.h"
#include "scenes/title.h"
#include <SDL.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
	(void)hInst; (void)hPrev; (void)lpCmdLine; (void)nCmdShow;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return 1;
	}

	g_state.window = SDL_CreateWindow("Alley Cat",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_W * SCALE, SCREEN_H * SCALE,
		SDL_WINDOW_SHOWN);
	if (!g_state.window) {
		fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	g_state.renderer = SDL_CreateRenderer(g_state.window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!g_state.renderer) {
		fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
		SDL_DestroyWindow(g_state.window);
		SDL_Quit();
		return 1;
	}

	SDL_RenderSetLogicalSize(g_state.renderer, SCREEN_W, SCREEN_H);
	SDL_SetRenderDrawColor(g_state.renderer, 0, 0, 0, 255);

	render_init(g_state.renderer);
	input_init();
	sound_init();

	game_init();

	scene_push(title_create());

	Uint32 last_time = SDL_GetTicks();
	while (g_state.running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				g_state.running = 0;
				break;
			case SDL_KEYDOWN:
				scene_keydown(event.key.keysym.sym);
				if (event.key.keysym.sym == SDLK_ESCAPE)
					g_state.running = 0;
				if (event.key.keysym.sym == SDLK_RETURN &&
				    (SDL_GetModState() & KMOD_ALT))
					SDL_SetWindowFullscreen(g_state.window,
						SDL_GetWindowFlags(g_state.window) & SDL_WINDOW_FULLSCREEN ?
						0 : SDL_WINDOW_FULLSCREEN);
				break;
			case SDL_KEYUP:
				scene_keyup(event.key.keysym.sym);
				break;
			}
		}

		Uint32 now = SDL_GetTicks();
		float dt = (now - last_time) / 1000.0f;
		last_time = now;
		if (dt > 0.1f) dt = 0.1f;

		g_state.ticks++;

		input_update();
		scene_update(dt);

		render_begin();
		SDL_SetRenderDrawColor(g_state.renderer, 0, 0, 0, 255);
		SDL_RenderClear(g_state.renderer);
		scene_render();
		render_end();
		SDL_RenderPresent(g_state.renderer);

		sound_update();
	}

	while (scene_current()) scene_pop();

	sound_close();
	if (g_state.framebuffer) SDL_DestroyTexture(g_state.framebuffer);
	SDL_DestroyRenderer(g_state.renderer);
	SDL_DestroyWindow(g_state.window);
	SDL_Quit();

	return 0;
}
