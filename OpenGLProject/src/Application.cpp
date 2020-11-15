#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <lib/Shader.h>

void fb_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void update(GLFWwindow* window);


int main(void)
{
    //Initialize glfw
    if (!glfwInit())
        return EXIT_FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    //Initialize window
    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL Project", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create a window!\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, fb_size_callback);
    glfwSetKeyCallback(window, key_callback);

    //Initialize glad - load all glfw functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    Shader shader("resources/shaders/vertexShader.vsh", "resources/shaders/fragmentShader.fsh");

    //Shader data generation - vertex array, vertex buffer and element buffer

    float vertices[] = {
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f
    };
    unsigned indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    unsigned int VBO, VAO, EBO; 

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //Tell the shader where in the buffer is which attribute of an object

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //We can now unbind data since it's all setup in out variables
    //(we still need element buffer though)
 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Execute program until we recieve info that window should close

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.useProgram();
        glBindVertexArray(VAO); //which data
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //draw
        
        glfwSwapBuffers(window);
    }

    //Clear our gpu resources

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    shader.deleteProgram();
    glfwTerminate();
    return EXIT_SUCCESS;
}

void fb_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void update(GLFWwindow* window) {

}