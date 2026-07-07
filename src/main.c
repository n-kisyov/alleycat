#include "game.h"
#include "render.h"
#include "input.h"
#include "sound.h"
#include "scene.h"
#include "scenes/title.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char **argv)
{
	(void)argc; (void)argv;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
			SDL_GetError(), NULL);
		return 1;
	}

	g_state.window = SDL_CreateWindow("Alley Cat",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_W * 3, SCREEN_H * 3,
		SDL_WINDOW_FULLSCREEN);

	if (!g_state.window) {
		g_state.window = SDL_CreateWindow("Alley Cat",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			SCREEN_W * 3, SCREEN_H * 3,
			SDL_WINDOW_SHOWN);
	}
	if (!g_state.window) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
			SDL_GetError(), NULL);
		SDL_Quit();
		return 1;
	}

	render_init(SCREEN_W, SCREEN_H);
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
				if (event.key.keysym.sym == SDLK_ESCAPE)
					g_state.running = 0;
				if (event.key.keysym.sym == SDLK_RETURN &&
				    (SDL_GetModState() & KMOD_ALT)) {
					Uint32 f = SDL_GetWindowFlags(g_state.window);
					SDL_SetWindowFullscreen(g_state.window,
						(f & SDL_WINDOW_FULLSCREEN) ? 0 : SDL_WINDOW_FULLSCREEN);
				}
				scene_keydown(event.key.keysym.sym);
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
		scene_render();
		render_present();

		sound_update();
	}

	while (scene_current()) scene_pop();

	sound_close();
	SDL_DestroyWindow(g_state.window);
	SDL_Quit();

	return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
	(void)hInst; (void)hPrev; (void)lpCmdLine; (void)nCmdShow;
	return main(0, NULL);
}
#endif
