#include "../renderer.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    GLuint programs[NUM_SHADER_PROGRAMS];
} ShaderContext;

static ShaderContext shader_context;

static u32 compile(GLenum type, const char* shader_code)
{
    u32 shader;
    char info_log[512];
    i32 success;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_code, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        log_write(FATAL, "Failed to compile shader: ", info_log);
    }

    return shader;
}

static void link(ShaderProgramEnum id)
{
    char info_log[512];
    i32 success;
    glLinkProgram(shader_context.programs[id]);
    glGetProgramiv(shader_context.programs[id], GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shader_context.programs[id], 512, NULL, info_log);
        log_write(FATAL, "Failed to link shader %d\n%s", id, info_log);
    }
}

static void attach(ShaderProgramEnum id, u32 shader)
{
    glAttachShader(shader_context.programs[id], shader);
}

static void detach(ShaderProgramEnum id, u32 shader)
{
    glDetachShader(shader_context.programs[id], shader);
}

static void delete(u32 shader)
{
    glDeleteShader(shader);
}

static char* vertex_shader = 
    "#version 460 core\n                                                                                          "
    "layout (location = 0) in vec2 aPosition;\n                                                                   "
    "layout (location = 1) in vec4 aPosOffset;\n                                                                  "
    "layout (location = 2) in vec4 aColor;\n                                                                      "
    "layout (location = 3) in vec4 aTexOffset;\n                                                                  "
    "layout (location = 4) in float aTexLocation;\n                                                               "
    "layout (std140) uniform Window {\n                                                                           "
    "    int width;\n                                                                                             "
    "    int height;\n                                                                                            "
    "    float aspect_ratio;\n                                                                                    "
    "} window;\n                                                                                                  "
    "out vec2 TexCoord;\n                                                                                         "
    "out vec4 Color;\n                                                                                            "
    "out flat int TexLocation;\n                                                                                  "
    "void main() {\n                                                                                              "
    "    vec2 position;\n                                                                                         "
    "    position.x = ((aPosOffset.x + aPosition.x * aPosOffset.z) - window.width / 2) / (window.width / 2);\n    "
    "    position.y = ((aPosOffset.y + aPosition.y * aPosOffset.w) - window.height / 2) / (window.height / 2);\n  "
    "    gl_Position = vec4(position, 0.0f, 1.0f);\n                                                              "
    "    TexCoord.x = (1 - aPosition.x) * aTexOffset.x + aPosition.x * aTexOffset.z;\n                            "
    "    TexCoord.y = (1 - aPosition.y) * aTexOffset.w + aPosition.y * aTexOffset.y;\n                            "
    "    TexLocation = int(round(aTexLocation));\n                                                                "
    "    Color = aColor / 255;\n                                                                                  "
    "}                                                                                                            ";

static char* fragment_shader =
    "#version 460 core\n                                                "
    "uniform sampler2D textures[16];\n                                  "
    "out vec4 FragColor;\n                                              "
    "in vec2 TexCoord;\n                                                "
    "in vec4 Color;\n                                                   "
    "in flat int TexLocation;\n                                         "
    "void main() {\n                                                    "
    "    FragColor = texture(textures[TexLocation], TexCoord) * Color;\n"
    "}                                                                  ";


static void compile_shader_program_gui(void)
{
    u32 vert, frag;
    vert = compile(GL_VERTEX_SHADER, vertex_shader);
    frag = compile(GL_FRAGMENT_SHADER, fragment_shader);
    attach(SHADER_PROGRAM_GUI, vert);
    attach(SHADER_PROGRAM_GUI, frag);
    link(SHADER_PROGRAM_GUI);
    detach(SHADER_PROGRAM_GUI, vert);
    detach(SHADER_PROGRAM_GUI, frag);
    delete(vert);
    delete(frag);
    i32 texs[NUM_TEXTURE_UNITS];
    for (i32 i = 0; i < NUM_TEXTURE_UNITS; ++i)
        texs[i] = i;
    shader_use(SHADER_PROGRAM_GUI);
    glUniform1iv(shader_get_uniform_location(SHADER_PROGRAM_GUI, "textures"), NUM_TEXTURE_UNITS, texs);
    shader_bind_uniform_block(SHADER_PROGRAM_GUI, UBO_INDEX_WINDOW, "Window");
}
void shader_init(void)
{
    shader_context.programs[0] = glCreateProgram();
    compile_shader_program_gui();
}

void shader_use(ShaderProgramEnum id)
{
    glUseProgram(shader_context.programs[id]);
}

void shader_cleanup(void)
{
    for (i32 i = 0; i < NUM_SHADER_PROGRAMS; i++)
        if (shader_context.programs[i] != 0)
            glDeleteProgram(shader_context.programs[i]);
}

GLuint shader_get_uniform_location(ShaderProgramEnum program, const char* identifier)
{
    return glGetUniformLocation(shader_context.programs[program], identifier);
}

void shader_bind_uniform_block(ShaderProgramEnum program, u32 index, const char* identifier)
{
    glUniformBlockBinding(shader_context.programs[program], 
            glGetUniformBlockIndex(shader_context.programs[program], identifier), 
            index);
}
