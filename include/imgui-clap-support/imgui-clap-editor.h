//
// Created by Paul Walker on 7/13/22.
// Modified by GloriousGlider8 on 20/08/2026 (DD/MM/YYYY).
//

#pragma once

#include <memory>

#include "clap/ext/gui.h"
#include "clap/ext/timer-support.h"

// Surely we need a backend render function, ImGui has all sorts of graphics library-specific functions for that.
struct imgui_clap_editor {
	virtual ~imgui_clap_editor() = default;
	virtual void onGuiCreate() {}
	virtual void onGuiDestroy() {}
	virtual void onRender() {}

	void *ctx = nullptr;
};

/**
 * To render an image, use `ImGui::Image((ImTextureID)(texture.texture), {texture.width, texture.height})`
 */
struct imgui_clap_texture {
	const int width;
	const int height;
	void *texture;
};

// I'm no expert in metal (and don't own any Apple devices yet), so if anybody could implement the following
// functions with the metal backend, it would be greatly appreciated:
// imgui_clap_guiSetWindowTitleWith
// imgui_clap_guiLoadImageWith
// imgui_clap_guiDestroyImageWith

bool imgui_clap_guiCreateWith(imgui_clap_editor *, const clap_host_timer_support_t *);
void imgui_clap_guiDestroyWith(imgui_clap_editor *, const clap_host_timer_support_t *);
bool imgui_clap_guiSetParentWith(imgui_clap_editor *, const clap_window *);
bool imgui_clap_guiSetSizeWith(imgui_clap_editor *, int width, int height);
bool imgui_clap_guiSetWindowTitleWith(imgui_clap_editor *, const char *);

/**
 * The image data **must** be in `R8G8B8A8` format.
 * You can use [`stb_image.h`](https://github.com/nothings/stb/blob/master/stb_image.h) to load an image from a file.
 */
imgui_clap_texture *imgui_clap_guiCreateTextureWith(imgui_clap_editor *, const uint8_t *, size_t, int width, int height);
void imgui_clap_guiDestroyTextureWith(imgui_clap_editor *, const imgui_clap_texture *);