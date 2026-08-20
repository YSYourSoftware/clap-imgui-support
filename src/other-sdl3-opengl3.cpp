/* SPDX-License-Identifier: MIT */

/*
 * SDL3 + OpenGL3 Support for ImGui with CLAP
 * GloriousGlider8 18/08/2026 (DD/MM/YYYY)
 *
 * Copyright 2026 YourSoftware Foundation <https://yoursoftware.org>
 * Licensed under the MIT license.
 */

// TODO: Make this file conform with CLAP code style standards (i.e. m_window -> mWindow)

#include "imgui-clap-support/imgui-clap-editor.h"

#include <SDL3/SDL.h>
#ifdef IMGUI_IMPL_OPENGL_ES2
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>

#include <stdexcept>

#define CIMGUI_RTASSERT(condition)                                                                                      \
	if (!(condition)) throw std::runtime_error("Assertion failed")

// Some code below is taken from https://github.com/ocornut/imgui/blob/master/examples/example_sdl3_opengl3/main.cpp
class OpenGLSupport {
public:
	OpenGLSupport() {
		CIMGUI_RTASSERT(SDL_Init(SDL_INIT_VIDEO));

		const char *glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
		// GL ES 2.0 + GLSL 100 (WebGL 1.0)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
		// GL ES 3.0 + GLSL 300 es (WebGL 2.0)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
		// GL 3.2 Core + generally GLSL 150
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
		// GL 3.0 + generally GLSL 130
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		constexpr SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
		m_window_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
		m_window = SDL_CreateWindow("CLAP Plugin", 0, 0, window_flags);

		CIMGUI_RTASSERT(m_window != nullptr);

		m_gl_context = SDL_GL_CreateContext(m_window);
		CIMGUI_RTASSERT(m_gl_context != nullptr);

		SDL_GL_MakeCurrent(m_window, m_gl_context);
		SDL_GL_SetSwapInterval(1);
		SDL_SetWindowPosition(m_window, );

		ImGui_ImplSDL3_InitForOpenGL();
	}
private:
	SDL_Window *m_window = nullptr;
	SDL_GLContext m_gl_context = nullptr;

	float m_window_scale = 0.f;
};