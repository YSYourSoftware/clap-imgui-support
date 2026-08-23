/* SPDX-License-Identifier: MIT */

/*
 * SDL3 + OpenGL3 Support for ImGui with CLAP
 * GloriousGlider8 18/08/2026 (DD/MM/YYYY)
 *
 * Copyright 2026 YourSoftware Foundation <https://yoursoftware.org>
 * Licensed under the MIT license.
 */

#if defined(__APPLE__)
#error macOS is not supported with the OpenGL3 backend.
#endif

#include "imgui-clap-support/imgui-clap-editor.h"

#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>

#include <stdexcept>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOCRYPT
#include <Windows.h>
#elif defined(__linux__) && !defined(__ANDROID__)
#include <X11/Xlib.h>
#endif

#define CIMGUI_RTASSERT(condition)                                                                                     \
	if (!(condition)) throw std::runtime_error("Assertion failed")

// Some code below is taken from https://github.com/ocornut/imgui/blob/master/examples/example_sdl3_opengl3/main.cpp
class SDL3OpenGL3Context {
public:
	SDL3OpenGL3Context() {
		CIMGUI_RTASSERT(SDL_Init(SDL_INIT_VIDEO));

		const char *glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
		// GL ES 2.0 + GLSL 100 (WebGL 1.0)
		glsl_version = "#version 100";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
		// GL ES 3.0 + GLSL 300 es (WebGL 2.0)
		glsl_version = "#version 300 es";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
		// GL 3.0 + generally GLSL 130
		glsl_version = "#version 130";
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
		SDL_ShowWindow(m_window);

		m_imgui_context = ImGui::CreateContext();

		ImGui::StyleColorsDark();
		ImGuiStyle &style = ImGui::GetStyle();
		style.ScaleAllSizes(m_window_scale);
		style.FontScaleDpi = m_window_scale;

		ImGui_ImplSDL3_InitForOpenGL(m_window, m_gl_context);
		ImGui_ImplOpenGL3_Init(glsl_version);

		ImGui::GetIO().IniFilename = nullptr;
	}

	~SDL3OpenGL3Context() {
		ImGui::SetCurrentContext(m_imgui_context);

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext(m_imgui_context);

		SDL_GL_DestroyContext(m_gl_context);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	void render(imgui_clap_editor *editor) {
		ImGui::SetCurrentContext(m_imgui_context);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			// Quit logic needed?
		}

		if (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
			return;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);

		editor->onRender();

		ImGui::End();

		ImGui::Render();

		constexpr ImVec4 clear_colour = {0.f, 0.f, 0.f, 1.f};
		const ImGuiIO &io = ImGui::GetIO();
		glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
		glClearColor(clear_colour.x * clear_colour.w, clear_colour.y * clear_colour.w, clear_colour.z * clear_colour.w,
					 clear_colour.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(m_window);
	}

	bool set_size(const int width, const int height) const { return SDL_SetWindowSize(m_window, width, height); }
	bool set_title(const char *title) const { return SDL_SetWindowTitle(m_window, title); }

	bool set_parent(const clap_window *parent) {
#if defined(_WIN32)
		if (std::strcmp(parent->api, CLAP_WINDOW_API_WIN32) != 0) return false;

		const auto parent_hwnd = static_cast<HWND>(parent->win32);

		const SDL_PropertiesID window_props = SDL_GetWindowProperties(m_window);
		const auto child_hwnd =
			static_cast<HWND>(SDL_GetPointerProperty(window_props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

		if (!parent_hwnd || !child_hwnd) return false;

		SetParent(child_hwnd, parent_hwnd);

		LONG_PTR style = GetWindowLongPtr(child_hwnd, GWL_STYLE);
		style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		style |= WS_CHILD;
		SetWindowLongPtr(child_hwnd, GWL_STYLE, style);

		SetWindowPos(child_hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		return true;
#elif defined(__linux__) && !defined(__ANDROID__)
		if (std::strcmp(parent->api, CLAP_WINDOW_API_X11) != 0) return false;

		// I haven't had the opportunity to test with linux yet.
		::Window parent_xwnd = static_cast<::Window>(window->x11);

		if (!parent_xwnd) return false;

		const SDL_PropertiesID window_props = SDL_GetWindowProperties(m_window);
		Display *display =
			static_cast<Display *>(SDL_GetPointerProperty(window_props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
		::Window child_xwnd =
			static_cast<::Window>(SDL_GetNumberProperty(window_props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));

		if (!parent_xwnd || !display || !child_xwnd) return false;

		XReparentWindow(display, child_xwnd, parent_xwnd, 0, 0);
		XMapWindow(display, child_xwnd);
		XFlush(display);

		return true;
#endif
		return false;
	}

	imgui_clap_texture *create_texture(const uint8_t *data, const int width, const int height) const {
		GLuint image_texture;
		glGenTextures(1, &image_texture);
		glBindTexture(GL_TEXTURE_2D, image_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);

		return new imgui_clap_texture{width, height, reinterpret_cast<void *>(static_cast<uintptr_t>(image_texture))};
	}

	void destroy_texture(const imgui_clap_texture *texture) const {
		if (!texture) return;

		const auto image_texture = static_cast<GLuint>(reinterpret_cast<uintptr_t>(texture->texture));
		glDeleteTextures(1, &image_texture);

		delete texture;
	}
private:
	SDL_Window *m_window = nullptr;
	SDL_GLContext m_gl_context = nullptr;

	ImGuiContext *m_imgui_context = nullptr;

	float m_window_scale = 0.f;
};

bool imgui_clap_guiCreateWith(imgui_clap_editor *editor, const clap_host_timer_support_t *timer_support) {
	try {
		editor->ctx = new SDL3OpenGL3Context();
	} catch (std::exception &e) { return false; }

	return true;
}

void imgui_clap_guiDestroyWith(imgui_clap_editor *editor, const clap_host_timer_support_t *timer_support) {
	delete static_cast<SDL3OpenGL3Context *>(editor->ctx);
}

bool imgui_clap_guiSetParentWith(imgui_clap_editor *editor, const clap_window *window) {
	return static_cast<SDL3OpenGL3Context *>(editor->ctx)->set_parent(window);
}

bool imgui_clap_guiSetSizeWith(imgui_clap_editor *editor, const int width, const int height) {
	return static_cast<SDL3OpenGL3Context *>(editor->ctx)->set_size(width, height);
}

bool imgui_clap_guiSetWindowTitleWith(imgui_clap_editor *editor, const char *title) {
	return static_cast<SDL3OpenGL3Context *>(editor->ctx)->set_title(title);
}

imgui_clap_texture *imgui_clap_guiCreateTextureWith(imgui_clap_editor *editor, const uint8_t *data, const size_t size,
													const int width, const int height) {
	return static_cast<SDL3OpenGL3Context *>(editor->ctx)->create_texture(data, width, height);
}

void imgui_clap_guiDestroyTextureWith(imgui_clap_editor *editor, const imgui_clap_texture *texture) {
	return static_cast<SDL3OpenGL3Context *>(editor->ctx)->destroy_texture(texture);
}
