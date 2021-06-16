#ifndef HOLOPLAYVOLUME_H_
#define HOLOPLAYVOLUME_H_

#include <Environment.hpp>
#include <Godot.hpp>
#include <Input.hpp>
#include <Rect2.hpp>
#include <Spatial.hpp>
#include <Math.hpp>

#include <vector>

#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#include "Shader.h"

namespace godot {

class HoloPlayVolume : public Spatial {
    GODOT_CLASS(HoloPlayVolume, Spatial)

public:
    enum QuiltPreset {
        LOW_QUALITY, // 24 views
        MEDIUM_QUALITY, // 32 views
        HIGH_QUALITY, // 45 views
        VERY_HIGH_QUALITY // 8k
    };

private:
    ////////////////////////////
    // API Device Properties. //
    ////////////////////////////

    // Window position of the display in window coordinates.
    int win_x;
    int win_y;

    // Screen width and height in pixels.
    int screen_w;
    int screen_h;

    // Display aspect ratio.
    float aspect;

    // Whether the lenticular shader should be inverted.
    bool inv_view;

    // Red and blue lenticular sub pixel indices.
    int red_index;
    int blue_index;

    // Lenticular lens pitch.
    float lent_pitch;
    
    // Lenticular center offset.
    float lent_offset;

    // Lenticular tilt angle.
    float lent_tilt;

    // Fringe correction value.
    float fringe;

    // Display subpixel size.
    float subpixel;

    /////////////
    // Camera. //
    /////////////

    uint32_t layers = 0xffffff;
    Ref<Environment> environment;

    ///////////////////////
    // Other properties. //
    ///////////////////////

    int device_index = 0;

    bool dummy = false;

    float near_clip = 0.2f;
    float far_clip = 0.5f;

    float size = 1.0f;

    ////////////////////////
    // Utility functions. //
    ////////////////////////

    void update_device_properties();

    ////////////////
    // Rendering. //
    ////////////////

    static Shader* blit_shader;
    static Shader* lightfield_shader;
    static const GLfloat tri_verts[6];

    bool in_world = false;
    
    HDC hdc;
    GLFWwindow* window = nullptr;
    
    std::vector<RID> viewports;
    std::vector<RID> cameras;

    float view_dist = 7.5f; // Distance between focus plane and viewer.
    float view_cone = 70.0f; // Viewing cone in degrees.

    QuiltPreset quilt_preset = QuiltPreset::MEDIUM_QUALITY;
    int tex_width = 2048;
    int tex_height = 2048;
    int total_views = 32;
    int num_cols = 4;
    int num_rows = 8;

    GLuint quilt_tex = 0;
    GLuint quilt_fbo = 0; // quilt fbo
    GLuint tri_vbo = 0;

    bool debug_view = false;

    void create_viewports();
    void update_cameras();
    
    void create_window();
    void destroy_window();
    
    void create_quilt_tex();
    void update_quilt_tex();

    void render_frame();

    ////////////
    // Mouse. //
    ////////////

    static HoloPlayVolume *grabbed_display;

    static void static_window_focus_callback(GLFWwindow* window, int focused);
    void window_focus_callback(bool focused);

    Vector2 mouse_pos;
    Input::MouseMode orig_mouse_mode;
    bool wait_for_active = false;

public:
    static void _register_methods();

    static void compile_shaders();
    static void free_shaders();

    HoloPlayVolume();
    ~HoloPlayVolume();

    void _init();

    void _process(float delta);
    void _input(const Ref<InputEvent> event);

    void _notification(int what);

    void frame_drawn_callback(int data);

    float get_aspect() const;
    Rect2 get_rect() const;

    uint32_t get_cull_mask() const;
    void set_cull_mask(uint32_t p_layers);

    void set_cull_mask_bit(int p_layer, bool p_enable);
    bool get_cull_mask_bit(int p_layer) const;

    Ref<Environment> get_environment() const;
    void set_environment(const Ref<Environment> p_environment);

    int get_device_index() const;
    void set_device_index(int p_device_index);

    bool is_dummy() const;
    void set_dummy(bool p_dummy);

    bool get_debug_view() const;
    void set_debug_view(bool p_debug_view);

    float get_near_clip() const;
    void set_near_clip(float p_near_clip);

    float get_far_clip() const;
    void set_far_clip(float p_far_clip);

    float get_view_dist() const;
    void set_view_dist(float p_view_dist);

    float get_view_cone() const;
    void set_view_cone(float p_view_cone);

    float get_size() const;
    void set_size(float p_size);

    int get_quilt_preset() const;
    void set_quilt_preset(int p_quilt_preset);

    void grab_mouse();
    void release_mouse();

    Vector2 get_mouse_position() const;

    Vector3 project_position(Vector2 screen_point, float z_depth) const;
    Vector3 project_ray_normal(Vector2 screen_point) const;
    Vector3 project_ray_origin(Vector2 screen_point) const;
};

}

#endif // HOLOPLAYVOLUME_H_