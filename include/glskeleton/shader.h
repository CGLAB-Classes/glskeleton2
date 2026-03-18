/*
    glskeleton/shader.h -- Shader program loader and manager

    Inspired by the LearnOpenGL Shader class.
    Loads vertex and fragment shaders from files, compiles and links them,
    and provides uniform setter utilities with glm type support.

    Usage:
        Shader shader("resources/shaders/basic.vert",
                       "resources/shaders/basic.frag");
        shader.use();
        shader.setMat4("projection", projMatrix);
*/

#pragma once

#include <nanogui/opengl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

class Shader {
public:
    /// Shader program ID
    GLuint ID = 0;

    /// Load, compile, and link vertex and fragment shaders from file paths
    Shader(const std::string &vertexPath, const std::string &fragmentPath) {
        // 1. Read shader source code from files
        std::string vertexCode   = readFile(vertexPath);
        std::string fragmentCode = readFile(fragmentPath);

        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();

        // 2. Compile shaders
        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, nullptr);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, nullptr);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // 3. Link shader program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // 4. Clean up individual shaders (no longer needed after linking)
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    ~Shader() {
        if (ID != 0)
            glDeleteProgram(ID);
    }

    // Non-copyable
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    // Movable
    Shader(Shader &&other) noexcept : ID(other.ID) { other.ID = 0; }
    Shader &operator=(Shader &&other) noexcept {
        if (this != &other) {
            if (ID != 0) glDeleteProgram(ID);
            ID = other.ID;
            other.ID = 0;
        }
        return *this;
    }

    /// Activate this shader program
    void use() const { glUseProgram(ID); }

    // --- Uniform setters: basic types ----------------------------------------

    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    // --- Uniform setters: glm vector types -----------------------------------

    void setVec2(const std::string &name, const glm::vec2 &v) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v));
    }

    void setVec3(const std::string &name, const glm::vec3 &v) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v));
    }

    void setVec4(const std::string &name, const glm::vec4 &v) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v));
    }

    // --- Uniform setters: glm matrix types -----------------------------------

    void setMat2(const std::string &name, const glm::mat2 &m) const {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
    }

    void setMat3(const std::string &name, const glm::mat3 &m) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
    }

    void setMat4(const std::string &name, const glm::mat4 &m) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
    }

private:
    /// Read entire file contents as a string
    static std::string readFile(const std::string &path) {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            file.open(path);
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            return stream.str();
        } catch (const std::ifstream::failure &e) {
            std::cerr << "ERROR::SHADER::FILE_NOT_READ: " << path << "\n"
                      << e.what() << std::endl;
            throw std::runtime_error("Failed to read shader file: " + path);
        }
    }

    /// Check for compile/link errors and print diagnostics
    void checkCompileErrors(GLuint shader, const std::string &type) {
        GLint success;
        char infoLog[1024];

        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
                std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n"
                          << infoLog << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
                std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                          << infoLog << std::endl;
            }
        }
    }
};
