#include "shader.hpp"

using namespace squi;

Shader::Shader(const char *vertexShaderSource, const char *fragmentShaderSource) {
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED %s\n", infoLog);
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED %s\n", infoLog);
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED %s\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(shaderProgram);
}

void Shader::use() {
    glUseProgram(shaderProgram);
}

void Shader::setUniform(const char *name, int value) {
    glUniform1i(glGetUniformLocation(shaderProgram, name), value);
}

void Shader::setUniform(const char *name, float value) {
    glUniform1f(glGetUniformLocation(shaderProgram, name), value);
}

void Shader::setUniform(const char *name, glm::vec2 value) {
    glUniform2fv(glGetUniformLocation(shaderProgram, name), 1, &value[0]);
}

void Shader::setUniform(const char *name, glm::vec3 value) {
    glUniform3fv(glGetUniformLocation(shaderProgram, name), 1, &value[0]);
}

void Shader::setUniform(const char *name, glm::vec4 value) {
    glUniform4fv(glGetUniformLocation(shaderProgram, name), 1, &value[0]);
}

void Shader::setUniform(const char *name, glm::mat3 value) {
    glUniformMatrix3fv(glGetUniformLocation(shaderProgram, name), 1, GL_FALSE, &value[0][0]);
}

void Shader::setUniform(const char *name, glm::mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name), 1, GL_FALSE, &value[0][0]);
}