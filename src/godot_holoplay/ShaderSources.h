#ifndef SHADERS_H_
#define SHADERS_H_

#include <string>

static std::string gl_version_string = "#version 330 core\n";

static const char *blit_vert_shader_code = 
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(aPos, 0.0, 1.0);\n"
"};";

static const char *blit_frag_shader_code =
"#version 330 core\n"
"uniform sampler2D tex;\n"
"uniform float x;\n"
"uniform float y;\n"
"uniform float width;\n"
"uniform float height;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = texture(tex, (gl_FragCoord.xy - vec2(x, y)) / vec2(width, height));\n"
"}";


#endif // SHADERS_H_