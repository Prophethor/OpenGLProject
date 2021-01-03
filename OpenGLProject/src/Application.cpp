#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
#include <lib/Shader.h>
#include <lib/Camera.h>
#include <lib/Model.h>
#include <lib/Error.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void fb_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void update(GLFWwindow* window);
unsigned int loadTexture(const char* path);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    DirLight(vec3 dir) {
        direction = dir;
        ambient = vec3(0.1, 0.1, 0.1);
        diffuse = vec3(0.6, 0.6, 0.6);
        specular = vec3(1.0, 1.0, 1.0);
    }             
};

struct PointLight {

    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    PointLight(vec3 pos) {
        position = pos;
        ambient = vec3(0.1, 0.1, 0.1);
        diffuse = vec3(0.6, 0.6, 0.6);
        specular = vec3(1.0, 1.0, 1.0);
        constant = 1.0f;
        linear = 0.09f;
        quadratic = 0.032f;
    }
};

struct ProgramState {
    bool antiAliasing = false;
    bool grayScale = false;
    bool imGuiEnabled = false;
    vec3 planeColor = vec3(0);
    vec3 clearColor = vec3(0);
    DirLight dirLight;
    PointLight pointLight;
    ProgramState()
        :
        dirLight(vec3(0.0f,0.0f,0.0f)),
        pointLight(vec3(0.0f,0.0f,-3.0f))
    {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << antiAliasing << '\n'
        << grayScale << '\n'
        << imGuiEnabled << '\n'
        << planeColor.r << '\n'
        << planeColor.g << '\n'
        << planeColor.b << '\n'
        << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << dirLight.direction.x << '\n'
        << dirLight.direction.y << '\n'
        << dirLight.direction.z << '\n'
        << pointLight.position.x << '\n'
        << pointLight.position.y << '\n'
        << pointLight.position.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in  >> antiAliasing
            >> grayScale
            >> imGuiEnabled
            >> planeColor.r
            >> planeColor.g
            >> planeColor.b
            >> clearColor.r
            >> clearColor.g
            >> clearColor.b
            >> dirLight.direction.x
            >> dirLight.direction.y
            >> dirLight.direction.z
            >> pointLight.position.x
            >> pointLight.position.y
            >> pointLight.position.z;
    }
}

ProgramState* programState;

Camera camera(vec3(0.0f,0.0f,3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

void DrawImGui(ProgramState* programState);

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGLProject", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, fb_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return EXIT_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->imGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    Shader cubeShader("resources/shaders/vertexShader.vs.glsl", "resources/shaders/fragmentShader.fs.glsl");
    Shader lightShader("resources/shaders/lightVertexShader.vs.glsl", "resources/shaders/lightFragmentShader.fs.glsl");
    Shader modelShader("resources/shaders/modelVertexShader.vs.glsl", "resources/shaders/modelFragmentShader.fs.glsl");
    Shader screenShader("resources/shaders/screenVertexShader.vs.glsl", "resources/shaders/screenFragmentShader.fs.glsl");
    Shader planeShader("resources/shaders/planeVertexShader.vs.glsl", "resources/shaders/planeFragmentShader.fs.glsl");

    Model myModel("resources/objects/cyborg/cyborg.obj");

    float planeVertices[] = {
        -0.5f, 0.0f, -0.5f,
         0.5f, 0.0f, -0.5f,
         0.5f, 0.0f,  0.5f,
         0.5f, 0.0f,  0.5f,
        -0.5f, 0.0f,  0.5f,
        -0.5f, 0.0f, -0.5f
    };

    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    vec3 cubePositions[] = {
        vec3(-1.5f, -2.2f, -2.5f),
        vec3(2.4f, -0.4f, -3.5f),
        vec3(1.3f, -2.0f, -2.5f),
        vec3(1.5f, 2.0f, -2.5f),
        vec3(1.5f, 0.2f, -1.5f),
        vec3(-1.3f, 1.0f, -1.5f)
    };


    

    float quadVertices[] = {

        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int cubeVBO, cubeVAO, lightVAO, planeVBO, planeVAO, quadVAO, quadVBO;

    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &planeVAO);
    glBindVertexArray(planeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // configure MSAA framebuffer
    // --------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a multisampled color attachment texture
    unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, screenTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, screenTexture, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    unsigned int containerDiffuse = loadTexture("resources/textures/container2.png");
    unsigned int containerSpecular = loadTexture("resources/textures/container2_specular.png");
    
    //Execute this loop until window should close

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        update(window);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);


        cubeShader.useProgram();
        cubeShader.setFloat("material.shininess", 32.0f);
        cubeShader.setInt("material.diffuse", 0);
        cubeShader.setInt("material.specular", 1);
        // directional light
        cubeShader.setVec3("dirLight.direction", programState->dirLight.direction);
        cubeShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        cubeShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        cubeShader.setVec3("dirLight.specular", programState->dirLight.specular);
        // point light
        cubeShader.setVec3("viewPos", camera.Position);
        cubeShader.setVec3("pointLight.position", programState->pointLight.position);
        cubeShader.setVec3("pointLight.ambient", programState->pointLight.ambient);
        cubeShader.setVec3("pointLight.diffuse", programState->pointLight.diffuse);
        cubeShader.setVec3("pointLight.specular", programState->pointLight.specular);
        cubeShader.setFloat("pointLight.constant", programState->pointLight.constant);
        cubeShader.setFloat("pointLight.linear", programState->pointLight.linear);
        cubeShader.setFloat("pointLight.quadratic", programState->pointLight.quadratic);

        // view/projection transformations
        mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        mat4 view = camera.GetViewMatrix();
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, containerSpecular);
        
        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 6; i++)
        {
            mat4 model = mat4(1.0f);
            model = translate(model, cubePositions[i]);
            float angle = 20.0f * (i+1);
            model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
            cubeShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // also draw the lamp object(s)
        lightShader.useProgram();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        mat4 model = mat4(1.0f);
        model = translate(model, programState->pointLight.position);
        model = scale(model, vec3(0.2f)); // Make it a smaller cube
        lightShader.setMat4("model", model);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        modelShader.useProgram();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setFloat("shininess", 32.0f);
        // directional light
        modelShader.setVec3("dirLight.direction", programState->dirLight.direction);
        modelShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        modelShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        modelShader.setVec3("dirLight.specular", programState->dirLight.specular);
        // point light
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setVec3("pointLight.position", programState->pointLight.position);
        modelShader.setVec3("pointLight.ambient", programState->pointLight.ambient);
        modelShader.setVec3("pointLight.diffuse", programState->pointLight.diffuse);
        modelShader.setVec3("pointLight.specular", programState->pointLight.specular);
        modelShader.setFloat("pointLight.constant", programState->pointLight.constant);
        modelShader.setFloat("pointLight.linear", programState->pointLight.linear);
        modelShader.setFloat("pointLight.quadratic", programState->pointLight.quadratic);
        model = mat4(1.0f);
        model = translate(model, vec3(0.0f, -3.0f, -5.0f));
        model = scale(model, vec3(1.0f));
        modelShader.setMat4("model", model);

        myModel.Draw(modelShader);

        planeShader.useProgram();
        model = mat4(1.0f);
        model = translate(model, vec3(0.0f, -3.0f, 0.0f));
        model = scale(model, vec3(200.0f));
        planeShader.setMat4("model", model);
        planeShader.setMat4("view", view);
        planeShader.setMat4("projection", projection);
        planeShader.setVec3("myColor", programState->planeColor);
        planeShader.setFloat("shininess", 32.0f);
        planeShader.setVec3("viewPos", camera.Position);
        planeShader.setVec3("dirLight.direction", programState->dirLight.direction);
        planeShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        planeShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        planeShader.setVec3("dirLight.specular", programState->dirLight.specular);
        planeShader.setVec3("pointLight.position", programState->pointLight.position);
        planeShader.setVec3("pointLight.ambient", programState->pointLight.ambient);
        planeShader.setVec3("pointLight.diffuse", programState->pointLight.diffuse);
        planeShader.setVec3("pointLight.specular", programState->pointLight.specular);
        planeShader.setFloat("pointLight.constant", programState->pointLight.constant);
        planeShader.setFloat("pointLight.linear", programState->pointLight.linear);
        planeShader.setFloat("pointLight.quadratic", programState->pointLight.quadratic);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        if(programState->imGuiEnabled)
            DrawImGui(programState);
       
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        screenShader.useProgram();
        glBindVertexArray(quadVAO);
        screenShader.setInt("screenTexture", 0);
        screenShader.setBool("shouldAA", programState->antiAliasing);
        screenShader.setBool("shouldGrayscale", programState->grayScale);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, screenTexture); // use the now resolved color attachment as the quad's texture
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //check events, swap buffers

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Free all GPU resources

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    programState->SaveToFile("resources/program_state.txt");
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &quadVBO);
    cubeShader.deleteProgram();
    lightShader.deleteProgram();
    planeShader.deleteProgram();
    screenShader.deleteProgram();
    glfwTerminate();
    return EXIT_SUCCESS;
}

// Update window state

void update(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Whenever window is resized adjust the viewport

void fb_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Register keypresses and act accordingly

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    } else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->imGuiEnabled = !programState->imGuiEnabled;
        if (programState->imGuiEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if(!programState->imGuiEnabled)camera.ProcessMouseMovement(xpos - lastX, lastY - ypos);
    lastX = xpos;
    lastY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void DrawImGui(ProgramState* programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        static float f = 0.0f;
        ImGui::Begin("Rendering options");
        ImGui::Checkbox("Anti-Aliasing MSAAx8", &programState->antiAliasing);
        ImGui::Checkbox("Grayscale", &programState->grayScale);
        ImGui::ColorEdit3("Background color", (float*)&programState->clearColor);
        ImGui::ColorEdit3("Ground color", (float*)&programState->planeColor);
        ImGui::DragFloat3("Point light position", (float*)value_ptr(programState->pointLight.position), 0.05, -100, 100);
        ImGui::DragFloat3("Directional light direction", (float*)value_ptr(programState->dirLight.direction), 0.01, -1, 1);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}