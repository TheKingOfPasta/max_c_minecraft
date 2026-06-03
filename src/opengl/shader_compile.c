#include "shader_compile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* read_all_file(char* file)
{
    FILE* f = fopen(file, "r");
    if (f == NULL)
    {
        printf("%s does not exist\n", file);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    int eof = ftell(f);
    rewind(f);

    char* res = calloc(eof + 1, sizeof(char));
    fread(res, eof, sizeof(char), f);
    fclose(f);

    return res;
}

static char* read_shader_includes(char* file)
{
    char* f = read_all_file(file);
    size_t result_len = 1;
    char* result = calloc(result_len, sizeof(char));

    const char* include_text = "#include \"";
    size_t include_len = strlen(include_text);
    size_t f_i = 0;
    size_t len = strlen(f);

    while (f_i < len)
    {
        if (strncmp(f + f_i, include_text, include_len) == 0)
        {
            f_i += include_len;

            char file_name[150] = { 0 };
            size_t file_name_size = 0;
            while (f[f_i] != '"')
                file_name[file_name_size++] = f[f_i++];
            f_i++;

            char* included = read_shader_includes(file_name);
            size_t included_len = strlen(included);

            result = realloc(result, result_len + included_len + 1 + len);
            strcat(result, included);
            result_len += included_len;

            free(included);
        }
        else
        {
            result = realloc(result, result_len + len);
            result[result_len - 1] = f[f_i++];
            result[result_len] = '\0';
            result_len++;
        }
    }

    free(f);
    return result;
}

static void print_error(const char* error_name, const char* src, const char* error_text)
{
    printf("%s:\n%s\n%s\n\n", error_name, src, error_text);

    size_t line_count = 0;
    for (; *src; src++)
    {
        printf("%c", *src);
        if (*src == '\n')
            printf("%5zu | ", line_count++);
    }
    printf("%s\n", error_text);
}

static GLuint create_compute_program(GLuint s, const char* src)
{
    GLuint p = glCreateProgram();
    glAttachShader(p, s);
    glLinkProgram(p);

    int ok;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(p, 1024, NULL, log);
        print_error("program link error", src, log);
        exit(1);
    }
    return p;
}

GLuint compile_shader(GLenum type, char* file_name)
{
    const char* src = read_shader_includes(file_name);

    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, 1024, NULL, log);
        print_error("shader error", src, log);
        exit(1);
    }

    if (type == GL_COMPUTE_SHADER)
        return create_compute_program(s, src);

    return s;
}

GLuint create_program(GLuint s1, GLuint s2)
{
    GLuint p = glCreateProgram();
    glAttachShader(p, s1);
    glAttachShader(p, s2);
    glLinkProgram(p);
    return p;
}

void bind_uniform_buffer(GLuint* ubo, GLuint binding, void* ptr, size_t elt_size)
{
    glGenBuffers(1, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, *ubo);
    glBufferData(GL_UNIFORM_BUFFER, elt_size, ptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, *ubo);
}

GLFWwindow* init_window(AppState* state)
{
    if (!glfwInit())
    {
        printf("glfwInit() failed\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(1920, 1080, "Max C Minecraft", NULL, NULL);
    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("glad failed to load\n");
        exit(1);
    }

    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(0);

    glfwSetWindowUserPointer(win, state);
    glfwSetKeyCallback(win, key_callback);
    glfwSetCursorPosCallback(win, cursor_callback);

    return win;
}
