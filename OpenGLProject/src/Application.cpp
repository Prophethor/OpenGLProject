#include <lib/Application.h>

ProgramState* programState;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

int main() {

    //Initialize GLFW

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Generate window and context

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGLProject", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    //Tell GL which functions to call when certain events happen

    glfwSetFramebufferSizeCallback(window, fb_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //Initialize GLAD

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return EXIT_FAILURE;
    }

    //Initialize new program state and if there is a file containing previous one read from it

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");

    //Hide/show cursor depending on if console mode is active 

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (programState->imGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    //Initialize ImGui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    //Initialize all of our shader programs

    Shader cubeShader("resources/shaders/cubeVertexShader.vs.glsl", "resources/shaders/cubeFragmentShader.fs.glsl");
    Shader lightShader("resources/shaders/lightVertexShader.vs.glsl", "resources/shaders/lightFragmentShader.fs.glsl");
    Shader modelShader("resources/shaders/modelVertexShader.vs.glsl", "resources/shaders/modelFragmentShader.fs.glsl");
    Shader screenShader("resources/shaders/screenVertexShader.vs.glsl", "resources/shaders/screenFragmentShader.fs.glsl");
    Shader planeShader("resources/shaders/planeVertexShader.vs.glsl", "resources/shaders/planeFragmentShader.fs.glsl");

    //Load a model from given location

    Model myModel("resources/objects/cyborg/cyborg.obj");

    //Declare all needed VBOs and VAOs

    unsigned int cubeVBO, cubeVAO, lightVAO, planeVBO, planeVAO, quadVBO, quadVAO;

    //Generate data needed to draw a cube

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

    //Generate data needed to draw a light source cube

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Generate data needed to draw a ground plane

    glGenBuffers(1, &planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &planeVAO);
    glBindVertexArray(planeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Generate data needed to draw a screen texture for a custom framebuffer

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    //Configure post-processing framebuffer

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //Create a multisampled color attachment texture

    unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, screenTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, screenTexture, 0);

    //Create a (also multisampled) renderbuffer object for depth and stencil attachments

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Load needed textures for drawing a cube

    unsigned int containerDiffuse = loadTexture("resources/textures/container2.png");
    unsigned int containerSpecular = loadTexture("resources/textures/container2_specular.png");
    
    //Execute this loop until window is given a signal to close

    while (!glfwWindowShouldClose(window)) {

        //Get current viewport dimensions

        GLint viewPortDim[4];
        glGetIntegerv(GL_VIEWPORT, viewPortDim);

        //Configure variables to make movement framerate-independent

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //Process user input
        
        update(window);

        //Bind our framebuffer and clear the screen

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        //Those transformation matrices are universal and used by all the shaders

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //Configure cube drawing

        cubeShader.useProgram();
        cubeShader.setVec3("viewPos", camera.Position);
        cubeShader.setVec3("lightColor", programState->lightColor);
        cubeShader.setFloat("material.shininess", 32.0f);
        cubeShader.setInt("material.diffuse", 0);
        cubeShader.setInt("material.specular", 1);
        cubeShader.setVec3("dirLight.direction", programState->dirLight.direction);
        cubeShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        cubeShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        cubeShader.setVec3("dirLight.specular", programState->dirLight.specular);
        cubeShader.setVec3("pointLight.position", programState->pointLight.position);
        cubeShader.setVec3("pointLight.ambient", programState->pointLight.ambient);
        cubeShader.setVec3("pointLight.diffuse", programState->pointLight.diffuse);
        cubeShader.setVec3("pointLight.specular", programState->pointLight.specular);
        cubeShader.setFloat("pointLight.constant", programState->pointLight.constant);
        cubeShader.setFloat("pointLight.linear", programState->pointLight.linear);
        cubeShader.setFloat("pointLight.quadratic", programState->pointLight.quadratic);
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, containerSpecular);

        //Draw multiple cubes
        
        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 6; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * (i+1);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            cubeShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //Configure light source cube drawing
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, programState->pointLight.position);
        model = glm::scale(model, glm::vec3(0.2f));
        lightShader.useProgram();
        lightShader.setMat4("model", model);
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);
        lightShader.setVec3("lightColor", programState->lightColor);

        //Draw a light source cube

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //Configure model drawing

        model = glm::mat4(1.0f); //model transformation matrix
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, -5.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        modelShader.useProgram();
        modelShader.setMat4("model", model);
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", projection);
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setVec3("lightColor", programState->lightColor);
        modelShader.setFloat("shininess", 32.0f);
        modelShader.setVec3("dirLight.direction", programState->dirLight.direction);
        modelShader.setVec3("dirLight.ambient", programState->dirLight.ambient);
        modelShader.setVec3("dirLight.diffuse", programState->dirLight.diffuse);
        modelShader.setVec3("dirLight.specular", programState->dirLight.specular);
        modelShader.setVec3("pointLight.position", programState->pointLight.position);
        modelShader.setVec3("pointLight.ambient", programState->pointLight.ambient);
        modelShader.setVec3("pointLight.diffuse", programState->pointLight.diffuse);
        modelShader.setVec3("pointLight.specular", programState->pointLight.specular);
        modelShader.setFloat("pointLight.constant", programState->pointLight.constant);
        modelShader.setFloat("pointLight.linear", programState->pointLight.linear);
        modelShader.setFloat("pointLight.quadratic", programState->pointLight.quadratic);

        //Draw a model

        myModel.Draw(modelShader);

        //Configure ground plane drawing

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
        model = glm::scale(model, glm::vec3(200.0f));
        planeShader.useProgram();
        planeShader.setMat4("model", model);
        planeShader.setMat4("view", view);
        planeShader.setMat4("projection", projection);
        planeShader.setVec3("myColor", programState->planeColor);
        planeShader.setVec3("lightColor", programState->lightColor);
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

        //Draw a ground plane 


        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //If user pressed F1 enter console mode

        if(programState->imGuiEnabled)
            DrawImGui(programState);

        //Unbind our framebuffer
       
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        //Configure and draw our screen texture

        screenShader.useProgram();
        screenShader.setInt("screenTexture", 0);
        screenShader.setBool("shouldAA", programState->antiAliasing);
        screenShader.setBool("shouldGrayscale", programState->grayScale);
        glUniform2iv(glGetUniformLocation(screenShader.GetID(),"viewPortDim"), 1, &viewPortDim[2]); //TODO: Implement integer Vec2 uniform setter in Shader.h
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, screenTexture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Check events and swap buffers

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    //Save program state and free all GPU resources to avoid memory leaks


    programState->SaveToFile("resources/program_state.txt");

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
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


//Update window state by processing user input

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

//Whenever window is resized adjust the viewport

void fb_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

//Callback function for when keyboard event is triggered

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

//Callback function for when mouse movement event is triggered

bool firstMouse = true; //true only in first frame in application that mouse moves in

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

//Callback function for when mouse scroll event is triggered

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

//Texture loading function

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

//Save program state to a file

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
        << lightColor.r << '\n'
        << lightColor.g << '\n'
        << lightColor.b << '\n'
        << dirLight.direction.x << '\n'
        << dirLight.direction.y << '\n'
        << dirLight.direction.z << '\n'
        << pointLight.position.x << '\n'
        << pointLight.position.y << '\n'
        << pointLight.position.z << '\n';
}

//If there is a file containing program state read from it

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> antiAliasing
            >> grayScale
            >> imGuiEnabled
            >> planeColor.r
            >> planeColor.g
            >> planeColor.b
            >> clearColor.r
            >> clearColor.g
            >> clearColor.b
            >> lightColor.r
            >> lightColor.g
            >> lightColor.b
            >> dirLight.direction.x
            >> dirLight.direction.y
            >> dirLight.direction.z
            >> pointLight.position.x
            >> pointLight.position.y
            >> pointLight.position.z;
    }
}

//Draw graphical interface in console mode

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
        ImGui::ColorEdit3("Light color", (float*)&programState->lightColor);
        ImGui::DragFloat3("Point light position", (float*)value_ptr(programState->pointLight.position), 0.05, -100, 100);
        ImGui::DragFloat3("Directional light direction", (float*)value_ptr(programState->dirLight.direction), 0.01, -1, 1);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}