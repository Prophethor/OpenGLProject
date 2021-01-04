#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <GLAD/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

inline std::string readFileFromPath(std::string filePath);

class Shader {

public:

    Shader(std::string vertexShaderPath, std::string fragmentShaderPath);

    void useProgram();
    void deleteProgram();
    unsigned int GetID();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:

    unsigned int m_id;

    void checkCompileErrors(GLuint shader, std::string type);

};