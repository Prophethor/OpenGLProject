#include <lib/Shader.h>

std::string readFileFromPath(std::string filePath) {
 std::ifstream in(filePath);
 std::stringstream buffer;
 buffer << in.rdbuf();
 return buffer.str();
}



Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath) {

    //Shader testing variables
    int success = 0;
    char infoLog[512];

    //Read shader code from files and convert them to C strings for later use
    std::string vsFileContent = readFileFromPath(vertexShaderPath);
    std::string fsFileContent = readFileFromPath(fragmentShaderPath);
    const char* vertexShaderSource = vsFileContent.c_str();
    const char* fragmentShaderSource = fsFileContent.c_str();

    //Initialize, compile and test vertex shader
    unsigned vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
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

void Shader::useProgram() {
    glUseProgram(m_id);
}

void Shader::deleteProgram() {
    glDeleteProgram(m_id);
    m_id = 0;
}

unsigned int Shader::GetID() {
    return m_id;
}

void Shader::setBool(const std::string& name, bool value) const  {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const  {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const  {
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const  {
    glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string& name, float x, float y) const  {
    glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const  {
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const  {
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const  {
    glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w)  {
    glUniform4f(glGetUniformLocation(m_id, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const  {
    glUniformMatrix2fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const  {
    glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const  {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}