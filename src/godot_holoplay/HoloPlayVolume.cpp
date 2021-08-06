#include "HoloPlayVolume.h"
#include "ShaderSources.h"

#include <windows.h>
#undef min
#undef max

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#include <HoloPlayCore.h>
#include <HoloPlayShadersOpen.h>

#include <GlobalConstants.hpp>
#include <Input.hpp>
#include <InputEvent.hpp>
#include <InputEventMouseMotion.hpp>
#include <InputEventMouseButton.hpp>
#include <Math.hpp>
#include <OS.hpp>
#include <ProjectSettings.hpp>
#include <Rect2.hpp>
#include <RID.hpp>
#include <Transform.hpp>
#include <VisualServer.hpp>
#include <World.hpp>

#include <vector>
#include <string>

using namespace godot;

Shader* HoloPlayVolume::blit_shader = nullptr;
Shader* HoloPlayVolume::lightfield_shader = nullptr;
const GLfloat HoloPlayVolume::tri_verts[6] = {
        -1.0, -1.0,
        -1.0,  3.0,
        3.0, -1.0
};
HoloPlayVolume* HoloPlayVolume::grabbed_display = nullptr;

void HoloPlayVolume::compile_shaders() {
    // Compile shaders.
    blit_shader = new Shader(blit_vert_shader_code, blit_frag_shader_code);
    lightfield_shader = new Shader((gl_version_string + hpc_LightfieldVertShaderGLSL).c_str(), (gl_version_string + hpc_LightfieldFragShaderGLSL).c_str());
};

void HoloPlayVolume::free_shaders() {
    // Free shaders.
    delete blit_shader;
    delete lightfield_shader;
}

void HoloPlayVolume::_register_methods() {
    register_method("_process", &HoloPlayVolume::_process);
    register_method("_input", &HoloPlayVolume::_input);
    register_method("_notification", &HoloPlayVolume::_notification);
    register_method("get_aspect", &HoloPlayVolume::get_aspect);
    register_method("get_rect", &HoloPlayVolume::get_rect);
    register_method("grab_mouse", &HoloPlayVolume::grab_mouse);
    register_method("release_mouse", &HoloPlayVolume::release_mouse);
    register_method("get_mouse_position", &HoloPlayVolume::get_mouse_position);
    register_method("project_position", &HoloPlayVolume::project_position);
    register_method("project_ray_origin", &HoloPlayVolume::project_ray_origin);
    register_method("project_ray_normal", &HoloPlayVolume::project_ray_normal);
    register_method("get_cull_mask", &HoloPlayVolume::get_cull_mask);
    register_method("set_cull_mask", &HoloPlayVolume::set_cull_mask);
    register_method("get_environment", &HoloPlayVolume::get_environment);
    register_method("set_cull_mask", &HoloPlayVolume::set_cull_mask);
    register_method("get_device_index", &HoloPlayVolume::get_device_index);
    register_method("set_device_index", &HoloPlayVolume::set_device_index);
    register_method("is_dummy", &HoloPlayVolume::is_dummy);
    register_method("set_dummy", &HoloPlayVolume::set_dummy);
    register_method("get_debug_view", &HoloPlayVolume::get_debug_view);
    register_method("set_debug_view", &HoloPlayVolume::set_debug_view);
    register_method("get_near_clip", &HoloPlayVolume::get_near_clip);
    register_method("set_near_clip", &HoloPlayVolume::set_near_clip);
    register_method("get_far_clip", &HoloPlayVolume::get_far_clip);
    register_method("set_far_clip", &HoloPlayVolume::set_far_clip);
    register_method("get_view_dist", &HoloPlayVolume::get_view_dist);
    register_method("set_view_dist", &HoloPlayVolume::set_view_dist);
    register_method("get_view_cone", &HoloPlayVolume::get_view_cone);
    register_method("set_view_cone", &HoloPlayVolume::set_view_cone);
    register_method("get_size", &HoloPlayVolume::get_size);
    register_method("set_size", &HoloPlayVolume::set_size);
    register_method("get_quilt_preset", &HoloPlayVolume::get_quilt_preset);
    register_method("set_quilt_preset", &HoloPlayVolume::set_quilt_preset);
    register_property("cull_mask", &HoloPlayVolume::set_cull_mask, &HoloPlayVolume::get_cull_mask, (uint32_t)0xffffff, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT, GODOT_PROPERTY_HINT_LAYERS_3D_RENDER);
    register_property("environment", &HoloPlayVolume::set_environment, &HoloPlayVolume::get_environment, Ref<Environment>(),  GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT, GODOT_PROPERTY_HINT_RESOURCE_TYPE, "Environment");
    register_property("device_index", &HoloPlayVolume::set_device_index, &HoloPlayVolume::get_device_index, 0);
    register_property("dummy", &HoloPlayVolume::set_dummy, &HoloPlayVolume::is_dummy, false);
    register_property("debug_view", &HoloPlayVolume::set_debug_view, &HoloPlayVolume::get_debug_view, false);
    register_property("near_clip", &HoloPlayVolume::set_near_clip, &HoloPlayVolume::get_near_clip, 0.2f);
    register_property("far_clip", &HoloPlayVolume::set_far_clip, &HoloPlayVolume::get_far_clip, 0.5f);
    register_property("view_dist", &HoloPlayVolume::set_view_dist, &HoloPlayVolume::get_view_dist, 1.0f);
    register_property("view_cone", &HoloPlayVolume::set_view_cone, &HoloPlayVolume::get_view_cone, 80.0f);
    register_property("size", &HoloPlayVolume::set_size, &HoloPlayVolume::get_size, 1.0f);
    register_property("quilt_preset", &HoloPlayVolume::set_quilt_preset, &HoloPlayVolume::get_quilt_preset, (int)QuiltPreset::MEDIUM_QUALITY,  GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT, GODOT_PROPERTY_HINT_ENUM, "Low Quality, Medium Quality, High Quality, Very High Quality");

    // This needs to be registered for VisualServer::request_frame_drawn_callback but shouldn't be directly called by the user.
    register_method("frame_drawn_callback", &HoloPlayVolume::frame_drawn_callback);
}

HoloPlayVolume::HoloPlayVolume() { }

HoloPlayVolume::~HoloPlayVolume() { }

float HoloPlayVolume::get_aspect() const {
    return aspect;
}

Rect2 HoloPlayVolume::get_rect() const {
    return Rect2(Vector2(win_x, win_y), Vector2(screen_w, screen_h));
}

Ref<Environment> HoloPlayVolume::get_environment() const {
    return environment;
};

void HoloPlayVolume::set_environment(const Ref<Environment> p_environment) {
    environment = p_environment;

    VisualServer *vs = VisualServer::get_singleton();
    for (int i = 0; i < cameras.size(); ++i) {
        RID camera = cameras[i];
        if (environment.is_valid()) {
            vs->camera_set_environment(camera, environment->get_rid());
        } else {
            vs->camera_set_environment(camera, RID());
        }
    }
};

int HoloPlayVolume::get_device_index() const {
    return device_index;
}

uint32_t HoloPlayVolume::get_cull_mask() const {
    return layers;
};

void HoloPlayVolume::set_cull_mask(uint32_t p_layers) {
    layers = p_layers;
    VisualServer *vs = VisualServer::get_singleton();
    for (int i = 0; i < cameras.size(); ++i) {
        RID camera = cameras[i];
        vs->camera_set_cull_mask(camera, layers);
    }
};

void HoloPlayVolume::set_cull_mask_bit(int p_layer, bool p_enable) {
    ERR_FAIL_INDEX(p_layer, 32);
    if (p_enable) {
        set_cull_mask(layers | (1 << p_layer));
    } else {
        set_cull_mask(layers & (~(1 << p_layer)));
    }
};

bool HoloPlayVolume::get_cull_mask_bit(int p_layer) const {
    ERR_FAIL_INDEX_V(p_layer, 32, false);
    return (layers & (1 << p_layer));
};

void HoloPlayVolume::set_device_index(int p_device_index) {
    device_index = p_device_index;
    update_device_properties();
}

bool HoloPlayVolume::is_dummy() const {
    return dummy;
}

void HoloPlayVolume::set_dummy(bool p_dummy) {
    dummy = p_dummy;
    update_device_properties();
}

bool HoloPlayVolume::get_debug_view() const {
    return debug_view;
}

void HoloPlayVolume::set_debug_view(bool p_debug_view) {
    debug_view = p_debug_view;
    lightfield_shader->use();
    lightfield_shader->setInt("debug", (int)debug_view);
}

float HoloPlayVolume::get_near_clip() const {
    return near_clip;
}

void HoloPlayVolume::set_near_clip(float p_near_clip) {
    near_clip = Math::max(p_near_clip, 0.0f);
    update_gizmo();
    update_cameras();
}

float HoloPlayVolume::get_far_clip() const {
    return far_clip;
}

void HoloPlayVolume::set_far_clip(float p_far_clip) {
    far_clip = Math::max(p_far_clip, 0.0f);
    update_gizmo();
    update_cameras(); 
}

float HoloPlayVolume::get_view_dist() const {
    return view_dist;
}

void HoloPlayVolume::set_view_dist(float p_view_dist) {
    view_dist = Math::max(p_view_dist, 0.0f);
    update_cameras();
}

float HoloPlayVolume::get_view_cone() const {
    return view_cone;
}

void HoloPlayVolume::set_view_cone(float p_view_cone) {
    view_cone = p_view_cone;
    update_cameras();
}

float HoloPlayVolume::get_size() const {
    return size;
}

void HoloPlayVolume::set_size(float p_size) {
    size = p_size;
    update_gizmo();
    update_cameras();
}

int HoloPlayVolume::get_quilt_preset() const {
    return quilt_preset;
}

void HoloPlayVolume::set_quilt_preset(int p_quilt_preset) { 
    quilt_preset = (QuiltPreset)p_quilt_preset;
    switch (quilt_preset) {
        case QuiltPreset::LOW_QUALITY:
            tex_width = 1024;
            tex_height = 1024;
            total_views = 21;
            num_cols = 3;
            num_rows = 7;
            break;
        case QuiltPreset::MEDIUM_QUALITY:
            tex_width = 2048;
            tex_height = 2048;
            total_views = 32;
            num_cols = 4;
            num_rows = 8;
            break;
        case QuiltPreset::HIGH_QUALITY:
            tex_width = 4096;
            tex_height = 4096;
            total_views = 45;
            num_cols = 5;
            num_rows = 9;
            break;
        case QuiltPreset::VERY_HIGH_QUALITY:
            tex_width = 8192;
            tex_height = 8192;
            total_views = 45;
            num_cols = 5;
            num_rows = 9;
            break;
    }
    create_viewports();

    // Resize quilt texture.
    update_quilt_tex();

    lightfield_shader->setVec3("tile", (float)num_cols, (float)num_rows, (float)total_views);
}

void HoloPlayVolume::_init() {
        update_device_properties();

        set_notify_transform(true);
}

void HoloPlayVolume::_notification(int what) {
    if (what == Spatial::NOTIFICATION_ENTER_WORLD) {
        in_world = true;
        create_viewports();

        // Create our GLFW window without a context since we will be re-using
        // Godot's context.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(300, 400, "Godot HoloPlay Output", NULL, NULL);
        if (window == NULL)
        {
            ERR_PRINT("Could not create GLFW window!");
            return;
        }
        glfwSetWindowSize(window, screen_w, screen_h);
        glfwSetWindowPos(window, win_x+1, win_y+1);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwSetWindowUserPointer(window, (void *)this);
        glfwSetWindowFocusCallback(window, HoloPlayVolume::static_window_focus_callback);

        // Get the windows's device context.
        HWND hwnd = glfwGetWin32Window(window);
        hdc = GetDC(hwnd);
        // Not using async causes issues with Godot.
        ShowWindowAsync(hwnd, SW_SHOWNOACTIVATE);
        //SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_NOACTIVATE);
        if (!hdc) {
            ERR_PRINT("Could not retrieve device context!");
        }

        // Set correct pixel format for the device context.
        HGLRC hglrc = (HGLRC)OS::get_singleton()->get_native_handle(OS::OPENGL_CONTEXT);
        HDC hdc_gd = (HDC)OS::get_singleton()->get_native_handle(OS::WINDOW_VIEW);

        PIXELFORMATDESCRIPTOR ppfd;
        int pxf = GetPixelFormat(hdc_gd);
        SetPixelFormat(hdc, pxf, &ppfd);

        // Create quilt texture.
        create_quilt_tex();

        // Create quilt quilt_fbo.
        glGenFramebuffers(1, &quilt_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, quilt_fbo);

        // Set quilt texture as render target of quilt quilt_fbo.
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               quilt_tex, 0);

        // Unbind frame buffer.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create VBO.
        glGenBuffers(1, &tri_vbo);
	    glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(tri_verts), tri_verts, GL_STATIC_DRAW);

    } else if (what == Spatial::NOTIFICATION_EXIT_WORLD) {
        in_world = false;
        glfwDestroyWindow(window);
    } else if (what == Spatial::NOTIFICATION_TRANSFORM_CHANGED) {
        // Update only the camera transforms but not the projections.
        VisualServer *vs = VisualServer::get_singleton();
        float s_view_dist = size * view_dist;

        for (int i = 0; i < cameras.size(); ++i) {
            RID camera = cameras[i];
            // Shift the camera horizontaly inside the view cone.
            float shift_angle = (i / (total_views - 1.0f) - 0.5f) * Math::deg2rad(view_cone);  
            float shift = s_view_dist * Math::tan(shift_angle);

            Transform t = get_global_transform(); // Should still be anchored to "unscaled" world
            t = t.translated(Vector3(shift, 0.0, s_view_dist));
            vs->camera_set_transform(camera, t);
        }
    }
}

void HoloPlayVolume::_process(float delta) {
    VisualServer* vs = VisualServer::get_singleton();
    if (vs->has_changed()) {
        vs->request_frame_drawn_callback(this, "frame_drawn_callback", NULL);
    }

    if (wait_for_active) {
        HWND hwnd = (HWND)godot::OS::get_singleton()->get_native_handle(godot::OS::WINDOW_HANDLE);
        SetActiveWindow(hwnd);
        if (hwnd == GetActiveWindow()) {
            wait_for_active = false;
        }
    }
}

void HoloPlayVolume::update_device_properties() {
    if (!dummy) {
        win_x = hpc_GetDevicePropertyWinX(device_index);
        win_y = hpc_GetDevicePropertyWinY(device_index);

        screen_w = hpc_GetDevicePropertyScreenW(device_index);
        screen_h = hpc_GetDevicePropertyScreenH(device_index);

        aspect = hpc_GetDevicePropertyDisplayAspect(device_index);

        inv_view = (bool)hpc_GetDevicePropertyInvView(device_index);

        red_index = hpc_GetDevicePropertyRi(device_index);
        blue_index = hpc_GetDevicePropertyBi(device_index);

        lent_pitch = hpc_GetDevicePropertyPitch(device_index);
        lent_offset = hpc_GetDevicePropertyCenter(device_index);
        lent_tilt = hpc_GetDevicePropertyTilt(device_index);

        fringe = hpc_GetDevicePropertyFringe(device_index);

        subpixel = hpc_GetDevicePropertySubp(device_index);
    } else {
        win_x = 0;
        win_y = 0;
        
        screen_w = 300;
        screen_h = 400;

        aspect = 0.75;

        inv_view = false;

        red_index = 0;
        blue_index = 2;

        lent_pitch = 0.0;
        lent_offset = 0.0;
        lent_tilt = 0.0;

        fringe = 0.0;
        subpixel = 0.0;
    }

    Godot::print("Updated device properties: ");
    Godot::print("win_x: " +  String::num_int64(win_x));
    Godot::print("win_y:" + String::num_int64(win_y));
    Godot::print("screen_w:" + String::num_int64(screen_w));
    Godot::print("screen_h:" + String::num_int64(screen_h));
    Godot::print("aspect:" + String::num(aspect));
    Godot::print("inv_view:" + String::num_int64(inv_view));
    Godot::print("red_index:" + String::num_int64(red_index));
    Godot::print("blue_index:" + String::num_int64(blue_index));
    Godot::print("lent_pitch:" + String::num(lent_pitch));
    Godot::print("lent_offset:" + String::num(lent_offset));
    Godot::print("lent_tilt:" + String::num(lent_tilt));
    Godot::print("fringe:" + String::num(fringe));
    Godot::print("subpixel:" + String::num(subpixel));

    // Update uniforms.
    if (lightfield_shader) {
        lightfield_shader->use();

        lightfield_shader->setFloat("pitch", lent_pitch);
        lightfield_shader->setFloat("tilt", lent_tilt);
        lightfield_shader->setFloat("center", lent_offset);
        lightfield_shader->setInt("invView", (int)inv_view);
        lightfield_shader->setFloat("subp", subpixel);
        lightfield_shader->setFloat("displayAspect", aspect);
        lightfield_shader->setFloat("quiltAspect", aspect);
        lightfield_shader->setInt("ri", red_index);
        lightfield_shader->setInt("bi", blue_index);

        lightfield_shader->setInt("overscan", 0);
        lightfield_shader->setInt("quiltInvert", 0);
        lightfield_shader->setVec3("tile", (float)num_cols, (float)num_rows, (float)total_views);
        lightfield_shader->setVec2("viewPortion", 1.0, 1.0);
    }

    // Update the gizmo with the new data.
    update_gizmo();

    create_viewports();

    if (window) {
        glfwSetWindowSize(window, screen_w, screen_h);
        glfwSetWindowPos(window, win_x+1, win_y+1);
    }

    // Resize quilt texture.
    update_quilt_tex();
}

void HoloPlayVolume::create_viewports() {
    if (!in_world) return;

    VisualServer *vs = VisualServer::get_singleton();
    for (int i = 0; i < viewports.size(); ++i) {
        RID viewport = viewports[i];
        RID camera = cameras[i];

        vs->viewport_detach(viewport);
        vs->free_rid(viewport);
        vs->free_rid(camera);
    }

    viewports.resize(0);
    cameras.resize(0);

    // Projection matrices correlate to viewport size in Godot
    // so we need to find the smallest viewport size with correct
    // aspect ratio. This sadly renders some unnecessary pixels :(
    int view_width = tex_width / num_cols;
    int view_height = tex_height / num_rows;
    int view_aspect = view_width / view_height;
    if (view_aspect > aspect) {
        view_height = view_width / aspect;
    } else {
        view_width = view_height * aspect;
    }

    for (int i = 0; i < total_views; ++i) {
        RID viewport = vs->viewport_create();
        RID camera = vs->camera_create();

        vs->viewport_set_size(viewport, view_width, view_height);
        vs->viewport_attach_camera(viewport, camera);
        vs->viewport_set_active(viewport, true);
        vs->viewport_set_update_mode(viewport, VisualServer::VIEWPORT_UPDATE_ALWAYS);
        vs->viewport_set_scenario(viewport, get_world()->get_scenario());

        viewports.push_back(viewport);
        cameras.push_back(camera);
    }

    update_cameras();
}

void HoloPlayVolume::update_cameras() {
    VisualServer *vs = VisualServer::get_singleton();
    float s_view_dist = size * view_dist;
    float s_near_clip = size * near_clip;
    float s_far_clip = size * far_clip;

    float znear = s_view_dist - s_near_clip;
    float zfar = s_view_dist + s_far_clip;
    float near_size = size * znear / s_view_dist / aspect; // Width of near plane (in global units)

    for (int i = 0; i < cameras.size(); ++i) {
        RID camera = cameras[i];
        // Shift the camera horizontaly inside the view cone.
        float shift_angle = (i / (total_views - 1.0f) - 0.5f) * Math::deg2rad(view_cone);  
        float shift = s_view_dist * Math::tan(shift_angle);

        Transform t = get_global_transform(); // Should still be anchored to "unscaled" world
        t = t.translated(Vector3(shift, 0.0, s_view_dist));
        vs->camera_set_transform(camera, t);

        Vector2 offset = Vector2(-shift * znear / s_view_dist, 0.0); // Tilt frustum near plane opposite to shift.
        vs->camera_set_frustum(camera, near_size, offset, znear, zfar);

        vs->camera_set_cull_mask(camera, layers);
        if (environment.is_valid()) {
            vs->camera_set_environment(camera, environment->get_rid());
        } else {
            vs->camera_set_environment(camera, RID());
        }
    }
}

void HoloPlayVolume::frame_drawn_callback(int data) {
    render_frame();
}

void HoloPlayVolume::render_frame() {
    if (!hdc) return;
    VisualServer *vs = VisualServer::get_singleton();

    HGLRC hglrc = (HGLRC)OS::get_singleton()->get_native_handle(OS::OPENGL_CONTEXT);
    HDC hdc_gd = (HDC)OS::get_singleton()->get_native_handle(OS::WINDOW_VIEW);

    wglMakeCurrent(hdc, hglrc);

    int view_width = tex_width / num_cols;
    int view_height = tex_height / num_rows;

    blit_shader->use();
    blit_shader->setInt("tex", 0);
    blit_shader->setFloat("width", (GLfloat)view_width);
    blit_shader->setFloat("height", (GLfloat)view_height);
    
    glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, quilt_fbo); // Bind target quilt_fbo.

    glActiveTexture(GL_TEXTURE0);

    for (int view_index = 0; view_index < total_views; ++view_index) {
        int x = (view_index % num_cols) * view_width;
        int y = (view_index / num_cols) * view_height;

        glViewport(x, y, view_width, view_height);

        RID view_tex = vs->viewport_get_texture(viewports[view_index]);
        glBindTexture(GL_TEXTURE_2D, (GLuint)vs->texture_get_texid(view_tex));
        
        blit_shader->setFloat("x", (GLfloat)x);
        blit_shader->setFloat("y", (GLfloat)y);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind quilt_fbo.

    glViewport(0, 0, screen_w, screen_h);

    // Create light field.
    glBindTexture(GL_TEXTURE_2D, quilt_tex);
    
    lightfield_shader->use();
    lightfield_shader->setInt("screenTex", 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    SwapBuffers(hdc);
    
    wglMakeCurrent(hdc_gd, hglrc); // Move context back to main window.
}

void HoloPlayVolume::create_quilt_tex() {
    glGenTextures(1, &quilt_tex);
    glBindTexture(GL_TEXTURE_2D, quilt_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB,
        GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void HoloPlayVolume::update_quilt_tex() {
    if (quilt_tex) {
        glBindTexture(GL_TEXTURE_2D, quilt_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB,
                GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void HoloPlayVolume::static_window_focus_callback(GLFWwindow *window, int focused) {
    HoloPlayVolume *hpv = (HoloPlayVolume *)glfwGetWindowUserPointer(window);
    hpv->window_focus_callback((bool)focused); 
}

void HoloPlayVolume::window_focus_callback(bool focused) {
    if (focused) {
        HWND hwnd = (HWND)OS::get_singleton()->get_native_handle(OS::WINDOW_HANDLE);
        SetActiveWindow(hwnd);
    }
}

void HoloPlayVolume::grab_mouse() {
    if (!grabbed_display) { // if no display has grabbed the mouse, change mouse mode
        Input *input = Input::get_singleton();
        orig_mouse_mode = input->get_mouse_mode();
        input->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
    }
    grabbed_display = this;
}

void HoloPlayVolume::release_mouse() {
    if (grabbed_display == this) {
        Input::get_singleton()->set_mouse_mode(orig_mouse_mode);
        grabbed_display = nullptr;
    } else {
        ERR_PRINT("Window belonging to this HoloPlayVolume has not currently grabbed the mouse.");
    }
}

Vector2 HoloPlayVolume::get_mouse_position() const {
    return mouse_pos;
}

Vector3 HoloPlayVolume::project_position(Vector2 screen_point, float z_depth) const {
    real_t rel_x = screen_point.x / (real_t)screen_w - 0.5;
    real_t rel_y = 0.5 - screen_point.y / (real_t)screen_h;
    Vector3 local_pos = size * Vector3(rel_x, rel_y / aspect, -z_depth);
    return get_global_transform() * local_pos;
}

Vector3 HoloPlayVolume::project_ray_normal(Vector2 screen_point) const {
    return -get_global_transform().basis.z;
}

Vector3 HoloPlayVolume::project_ray_origin(Vector2 screen_point) const {
    return project_position(screen_point, -near_clip);
}
void HoloPlayVolume::_input(const Ref<InputEvent> event) {
    if (grabbed_display == this) {
        InputEventMouseMotion *mm = Object::cast_to<InputEventMouseMotion>(event.ptr());
        if (mm) {
            mouse_pos += mm->get_relative();
            mouse_pos.x = Math::clamp(mouse_pos.x, 0.0f, (real_t)screen_w);
            mouse_pos.y = Math::clamp(mouse_pos.y, 0.0f, (real_t)screen_h);
        }
    }
}