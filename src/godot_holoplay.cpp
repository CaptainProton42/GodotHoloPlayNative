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

	hpc_CloseApp();

	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_gdnative_singleton() {
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *p_handle) {
	godot::Godot::nativescript_init(p_handle);

	hpc_client_error errco = hpc_InitializeApp("Godot HoloPlay Plugin", hpc_LICENSE_NONCOMMERCIAL);
	if (errco) {
		std::string errstr;
		switch (errco)
		{
			case hpc_CLIERR_NOSERVICE:
				errstr = "HoloPlay Service not running";
				break;
			case hpc_CLIERR_SERIALIZEERR:
				errstr = "Client message could not be serialized";
				break;
			case hpc_CLIERR_VERSIONERR:
				errstr = "Incompatible version of HoloPlay Service";
				break;
			case hpc_CLIERR_PIPEERROR:
				errstr = "Interprocess pipe broken";
				break;
			case hpc_CLIERR_SENDTIMEOUT:
				errstr = "Interprocess pipe send timeout";
				break;
			case hpc_CLIERR_RECVTIMEOUT:
				errstr = "Interprocess pipe receive timeout";
				break;
			default:
				errstr = "Unknown error";
				break;
		}
		ERR_PRINT("Could not connect to the HoloPlay service: " + String(errstr.c_str()) + " (" + String::num_int64(errco) + ").");
		return;
	}

	if (!glfwInit()) {
		ERR_PRINT("Could not initialize GLFW!");
		return;
	}

	if (!gladLoadGL()) {
		ERR_PRINT("Failed to initialize GLAD");
		return;
	}

	HoloPlayVolume::compile_shaders();

	godot::register_tool_class<godot::HoloPlayVolume>();
}