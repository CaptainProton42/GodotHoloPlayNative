#include "godot_holoplay/HoloPlayVolume.h"

#include <ProjectSettings.hpp>
#include <VisualServer.hpp>
#include <OS.hpp>
#include <Engine.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <HoloPlayCore.h>

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	HoloPlayVolume::free_shaders();
	glfwTerminate();
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_gdnative_singleton() {
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *p_handle) {
	godot::Godot::nativescript_init(p_handle);

	if (!glfwInit()) {
		ERR_PRINT("Could not initialize GLFW!");
		return;
	}

	if (!gladLoadGL()) {
		ERR_PRINT("Failed to initialize GLAD");
		return;
	}

	hpc_client_error errco =
      hpc_InitializeApp("Godot HoloPlay Plugin", hpc_LICENSE_NONCOMMERCIAL);

	HoloPlayVolume::compile_shaders();

	godot::register_tool_class<godot::HoloPlayVolume>();
}