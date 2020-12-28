#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <GLAD/glad.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

string readFileFromPath(string filePath) {
    ifstream in(filePath);
    stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

class Shader {

    unsigned int m_id;

public:

    Shader(string vertexShaderPath, string fragmentShaderPath) {

        //Shader testing variables
        int success = 0;
        char infoLog[512];

        //Read shader code from files and convert them to C strings for later use
        string vsFileContent = readFileFromPath(vertexShaderPath);
        string fsFileContent = readFileFromPath(fragmentShaderPath);
        const char* vertexShaderSource = vsFileContent.c_str();
        const char* fragmentShaderSource = fsFileContent.c_str();

        //Initialize, compile and test vertex shader
        unsigned vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                << infoLog << '\n';
        }

        //Initialize, compile and test fragment shader

        unsigned fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                << infoLog << '\n';
        }

        //Create and test shader program from given shaders

        unsigned shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                << infoLog << '\n';
        }

        //We don't need the shaders once program is created

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        //Finally we save the ID of the created shader program
        m_id = shaderProgram;
    }

    void useProgram() {
        glUseProgram(m_id);
    }

    void deleteProgram() {
        glDeleteProgram(m_id);
        m_id = 0;
    }

    unsigned int GetID() {
        return m_id;
    }

    void setInt(const string& name, float x) {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), x);
    }

    void setFloat(const string& name, float x) {
        glUniform1f(glGetUniformLocation(m_id, name.c_str()), x);
    }

    void set4Float(const string& name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(m_id,name.c_str()), x, y, z, w);
    }

    void setVec3(const string& name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
    }

    void setVec3(const string& name, glm::vec3& vector) {
        glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &vector[0]);
    }

    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

};