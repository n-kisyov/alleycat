#include "game.h"
#include "render.h"
#include "input.h"
#include "sound.h"
#include "scene.h"
#include "scenes/title.h"
#include <SDL.h>

#define WINDOW_SCALE 3

/*
 * Logic runs on a fixed step so the game plays the same on any display.  The
 * old loop threw dt away and throttled twice -- vsync plus a manual delay --
 * which tied every speed in the game to whatever frame rate happened to land.
 */
#define STEP_MS (1000.0 / LOGIC_HZ)

static void toggle_fullscreen(void)
{
	Uint32 flags = SDL_GetWindowFlags(g_state.window);
	int on = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;

	SDL_SetWindowFullscreen(g_state.window,
		on ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
}

static void fail(const char *what)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Alley Cat",
	                         what ? what : SDL_GetError(), NULL);
}

int main(int argc, char **argv)
{
	double accumulator = 0.0;
	Uint64 last;

	(void)argc;
	(void)argv;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		fail(NULL);
		return 1;
	}

	/* Windowed by default; Alt+Enter switches to borderless desktop
	 * fullscreen rather than changing the display mode. */
	g_state.window = SDL_CreateWindow("Alley Cat",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_W * WINDOW_SCALE, SCREEN_H * WINDOW_SCALE,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!g_state.window) {
		fail(NULL);
		SDL_Quit();
		return 1;
	}

	if (!render_init(SCREEN_W, SCREEN_H)) {
		fail("Could not create the renderer.");
		render_shutdown();
		SDL_DestroyWindow(g_state.window);
		SDL_Quit();
		return 1;
	}

	input_init();
	sound_init();
	game_init();

	scene_request_replace(title_create());
	scene_apply_pending();

	last = SDL_GetPerformanceCounter();

	while (g_state.running) {
		SDL_Event e;
		Uint64 now;
		double elapsed_ms;

		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				g_state.running = 0;
				break;
			case SDL_KEYDOWN:
				if (e.key.repeat)
					break;
				if (e.key.keysym.sym == SDLK_RETURN &&
				    (SDL_GetModState() & KMOD_ALT)) {
					toggle_fullscreen();
					break;
				}
				scene_keydown(e.key.keysym.sym);
				break;
			case SDL_KEYUP:
				scene_keyup(e.key.keysym.sym);
				break;
			default:
				break;
			}
		}
		scene_apply_pending();

		now = SDL_GetPerformanceCounter();
		elapsed_ms = (double)(now - last) * 1000.0 /
		             (double)SDL_GetPerformanceFrequency();
		last = now;

		/* Cap the catch-up so a stall does not spiral into a long run of
		 * logic steps. */
		if (elapsed_ms > 250.0)
			elapsed_ms = 250.0;
		accumulator += elapsed_ms;

		while (accumulator >= STEP_MS) {
			accumulator -= STEP_MS;
			g_state.ticks++;
			input_update();
			scene_update();
			game_flash_update();
			/* Applied inside the step loop: a scene that asked to be
			 * replaced must not receive another update. */
			scene_apply_pending();
			if (!g_state.running)
				break;
		}

		scene_render();
		render_present();

		if (!scene_current())
			g_state.running = 0;
	}

	scene_clear();
	sound_stop();
	sound_close();
	render_shutdown();
	SDL_DestroyWindow(g_state.window);
	SDL_Quit();
	return 0;
}
