#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>

unsigned int loadTexture(const char *path);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(const char *path);
void renderQuad();


// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;
bool hdr = true;
bool hdrKeyPressed = false;

bool bloom = true;
bool bloomKeyPressed = false;

//grayscale effect
bool grayEffect = false;

//parralax mapping height ()
float heightScale = 0.045;


//normal/parallax rendering flag
bool normalON = true;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};



struct DirLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 lampLightPosition = glm::vec3(0.0f);

    float mountainScale = 1.0f;
    float mountainScale2 = 1.0f;
    float mountainScale3 = 1.0f;
    glm::vec3 mountainPosition = glm::vec3(0.0f);
    glm::vec3 mountainPosition2 = glm::vec3(0.0f);
    glm::vec3 mountainPosition3 = glm::vec3(0.0f);
    float exposure = 1.0f;
    glm::vec3 modelPosition = glm::vec3(0.0f);


    PointLight pointLight;
    SpotLight spotLight;
    PointLight lampPointLight;
    DirLight dirLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n'
        << mountainScale << '\n'
        << mountainScale2 << '\n'
        << mountainScale3 << '\n'
        << mountainPosition.x << '\n'
        << mountainPosition.y << '\n'
        << mountainPosition.z << '\n'
        << mountainPosition2.x << '\n'
        << mountainPosition2.y << '\n'
        << mountainPosition2.z << '\n'
        << mountainPosition3.x << '\n'
        << mountainPosition3.y << '\n'
        << mountainPosition3.z << '\n'
        << exposure << '\n'
        << lampLightPosition.x << '\n'
        << lampLightPosition.y << '\n'
        << lampLightPosition.z << '\n'
        << modelPosition.x << '\n'
        << modelPosition.y << '\n'
        << modelPosition.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z
           >> mountainScale
           >> mountainScale2
           >> mountainScale3
           >> mountainPosition.x
           >> mountainPosition.y
           >> mountainPosition.z
           >> mountainPosition2.x
           >> mountainPosition2.y
           >> mountainPosition2.z
           >> mountainPosition3.x
           >> mountainPosition3.y
           >> mountainPosition3.z
           >> exposure
           >> lampLightPosition.x
           >> lampLightPosition.y
           >> lampLightPosition.z
           >> modelPosition.x
           >> modelPosition.y
           >> modelPosition.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    //enable frag blending and setup blending function:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // -------------------------
    Shader modelShader("resources/shaders/modelLightingShader.vs", "resources/shaders/modelLightingShader.fs");
    Shader antiAliasingShader("resources/shaders/antial.vs","resources/shaders/antial.fs");
    Shader hdrShader("resources/shaders/hdr.vs", "resources/shaders/hdr.fs");
    Shader blurShader("resources/shaders/blur.vs", "resources/shaders/blur.fs");


    Shader rugShader("resources/shaders/rugShader.vs", "resources/shaders/rugShader.fs");
    unsigned int rugTextureDiff = loadTexture("resources/textures/rug.png");
    unsigned int rugTextureNormal = loadTexture("resources/textures/rugNormal.png");
    rugShader.use();
    rugShader.setInt("diffuseMap", 0);
    rugShader.setInt("normalMap", 1);


    // load models
    // -----------


    //load house model
    Model houseModel("resources/objects/house/highpoly_town_house_01.obj");
    houseModel.SetShaderTextureNamePrefix("material.");


    //load snow model
    Model snowModel("resources/objects/snow model/terrain5.obj");
    snowModel.SetShaderTextureNamePrefix("material.");
    Model snowModel2("resources/objects/snow model/terrain3.obj");
    snowModel.SetShaderTextureNamePrefix("material.");
    Model snowModel3("resources/objects/snow model/terrain4.obj");
    snowModel.SetShaderTextureNamePrefix("material.");

    //load furniture models(bed,table,bookcase...)
    Model bedModel("resources/objects/bed/untitled.obj");
    bedModel.SetShaderTextureNamePrefix("material.");
    Model tableModel("resources/objects/table/Table_Chair.obj");
    tableModel.SetShaderTextureNamePrefix("material.");

    //load tree models
    Model modelTree("resources/objects/tree/3d-model.obj");
    modelTree.SetShaderTextureNamePrefix("material.");
    Model modelTree2("resources/objects/tree/3d-model.obj");
    modelTree2.SetShaderTextureNamePrefix("material.");
    Model modelTree3("resources/objects/tree/3d-model.obj");
    modelTree3.SetShaderTextureNamePrefix("material.");

    //load fence
    Model modelFence("resources/objects/fence/untitled.obj");
    modelFence.SetShaderTextureNamePrefix("material.");
    Model modelFence2("resources/objects/fence/untitled.obj");
    modelFence2.SetShaderTextureNamePrefix("material.");
    Model modelFence3("resources/objects/fence/untitled.obj");
    modelFence3.SetShaderTextureNamePrefix("material.");
    Model modelFence4("resources/objects/fence/untitled.obj");
    modelFence4.SetShaderTextureNamePrefix("material.");

    //load rock
    Model modelRock("resources/objects/rock/untitled.obj");
    modelFence4.SetShaderTextureNamePrefix("material.");

    //load sled
    Model modelSled("resources/objects/sled/Sled01Old.obj");
    modelSled.SetShaderTextureNamePrefix("material.");
    //Model shelfModel("resources/objects/shelf/BOOKS OBJ.obj");
    //shelfModel.SetShaderTextureNamePrefix("material.");
    //Model rugModel("resources/objects/rug/Fine Persian Esfahan Carpet.obj");
    //rugModel.SetShaderTextureNamePrefix("material.");


    //load mt model
    Model modelMountain("resources/objects/great_mountain/untitled.obj");
    modelMountain.SetShaderTextureNamePrefix("material.");
    Model modelMountain2("resources/objects/great_mountain/untitled.obj");
    modelMountain2.SetShaderTextureNamePrefix("material.");
    Model modelMountain3("resources/objects/great_mountain/untitled.obj");
    modelMountain3.SetShaderTextureNamePrefix("material.");
    Model modelLamp("resources/objects/lamp/Lamp Old Street.obj");
    modelLamp.SetShaderTextureNamePrefix("material.");



    //load bell model
    Shader reflectShader("resources/shaders/reflectShader.vs", "resources/shaders/reflectShader.fs");
    Model bellModel("resources/objects/bell/bell.obj");


    //skybox vertices/cubemapping
    Shader skyShader("resources/shaders/skyShader.vs", "resources/shaders/skyShader.fs");

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };



    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    //vector of cubemap component paths
    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/right.jpg").c_str(),
                    FileSystem::getPath("resources/textures/left.jpg").c_str(),
                    FileSystem::getPath("resources/textures/bottom.jpg").c_str(),
                    FileSystem::getPath("resources/textures/top.jpg").c_str(),
                    FileSystem::getPath("resources/textures/front.jpg").c_str(),
                    FileSystem::getPath("resources/textures/back.jpg").c_str()
            };
    //load maps
    unsigned int cubemapTexture = loadCubemap(faces);

    skyShader.use();
    skyShader.setInt("skybox", 0);



    //plane
    float planeVertices[] = {
            // positions            // normals         // texcoords
            10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
            -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
            -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

            10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
            -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
            10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
    };

    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int planeTexture = loadTexture("resources/textures/Snow1Albedo.png");
    modelShader.use();
    modelShader.setInt("material.texture_diffuse1", 0);






    //directional light
    DirLight& dirLight = programState->dirLight;
    dirLight.direction = glm::vec3(0.0, -5.0, 0.0);
    dirLight.ambient = glm::vec3(0.05, 0.05, 0.05);
    //dirLight.diffuse = glm::vec3(0.4, 0.4, 0.4);
    //dirLight.specular = glm::vec3(0.4, 0.4, 0.4);
    dirLight.diffuse = glm::vec3(0.1, 0.1, 0.1);
    dirLight.specular = glm::vec3(0.1, 0.1, 0.1);

    //point light
    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    //spotlight lamp
    SpotLight& spotLight = programState->spotLight;
    spotLight.position = glm::vec3(4.0f, 4.0, 0.0);
    spotLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    spotLight.diffuse = glm::vec3(0.9f, 0.25f, 0.1f);
    spotLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09f;
    spotLight.quadratic = 0.032f;
    spotLight.cutOff = glm::cos(glm::radians(20.0f));
    spotLight.outerCutOff = glm::cos(glm::radians(35.0f));

    //lamp
    PointLight& lampPointLight = programState->lampPointLight;
    lampPointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    lampPointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    lampPointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    lampPointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    lampPointLight.constant = 1.0f;
    lampPointLight.linear = 0.09f;
    lampPointLight.quadratic = 0.032f;



    //transparent window vertices
    Shader windowShader("resources/shaders/transparentShader.vs", "resources/shaders/transparentShader.fs");
    float windowVertices[] = {
            4.0f, -0.4f,  4.0f,  1.0f, 0.0f,
            -4.0f, -0.4f,  4.0f,  0.0f, 0.0f,
            -4.0f, -0.4f, -4.0f,  0.0f, 1.0f,

            4.0f, -0.4f,  4.0f,  1.0f, 0.0f,
            -4.0f, -0.4f, -4.0f,  0.0f, 1.0f,
            4.0f, -0.4f, -4.0f,  1.0f, 1.0f
    };

    unsigned int windowVAO, windowVBO;
    glGenVertexArrays(1, &windowVAO);
    glGenBuffers(1, &windowVBO);
    glBindVertexArray(windowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(windowVertices), windowVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);


    unsigned int windowTexture = loadTexture(FileSystem::getPath("resources/textures/window.png").c_str());

    windowShader.use();
    windowShader.setInt("diffTex", 0);




    //stone wall shader and tex
    Shader brickShader("resources/shaders/normalShader.vs", "resources/shaders/normalShader.fs");

    unsigned int brickTextureDiff = loadTexture(FileSystem::getPath("resources/textures/brickWallDiff.jpg").c_str());
    //unsigned int brickTextureSpec = loadTexture(FileSystem::getPath("resources/textures/brickWallSpec.jpg").c_str());
    unsigned int brickTextureNormal = loadTexture(FileSystem::getPath("resources/textures/brickWallNormal.jpg").c_str());
    unsigned int brickTextureDisp = loadTexture(FileSystem::getPath("resources/textures/brickWallDisp.jpg").c_str());

    brickShader.use();
    brickShader.setInt("diffuseMap", 0);
    brickShader.setInt("normalMap", 1);
    brickShader.setInt("depthMap", 2);
    //brickShader.setInt("specularMap", 3);


    //screen vertices
    float quadVertices[] = {
            // positions        // texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));



    //hdr and bloom
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // renderbuffer
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    unsigned int pingpongFBO[2];
    unsigned int pingpongColorBuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorBuffers);
    for (unsigned int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorBuffers[i], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //MSAA
    unsigned int msFBO;
    glGenFramebuffers(1, &msFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, msFBO);

    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCR_WIDTH,SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    //napravili smo tekstura obj i treba da ga nakacimo sad
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

    unsigned  int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "ERROR: FRAMEBUFFER\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    antiAliasingShader.use();
    antiAliasingShader.setInt("screenTex", 0);

    hdrShader.use();
    hdrShader.setInt("scene", 0);
    hdrShader.setInt("bloomBlur", 1);

    blurShader.use();
    blurShader.setInt("image", 0);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    float h = 0;
    typedef struct{
        glm::vec3 trans;
        glm::vec3 skal;
        glm::vec3 rotatV;
        float rotatU;
    }triD;
    vector<triD>winPos(3);
    winPos.push_back({glm::vec3(-1.25f,1.75f,-3.25f), glm::vec3(0.39f,0.45f,0.4f), glm::vec3(1.0, 0, 0) ,43.0f});
    winPos.push_back({glm::vec3(-1.25f,3.05f,2.35f), glm::vec3(0.39f,0.45f,0.4f), glm::vec3(1.0, 0, 0) ,43.0f});
    winPos.push_back({glm::vec3(3.2753f,1.72f,1.35f), glm::vec3(0.39f,0.45f,0.4f), glm::vec3(0.0f, 0.0f, 1.0f) ,43.0f});
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(0.1,0.1,0.1, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
        glClearColor(0.1,0.1,0.1, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);*/


        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();


        //skybox rendering
        glDepthMask(GL_FALSE);

        skyShader.use();

        glm::mat4 viewCube = glm::mat4(glm::mat3(view));

        //glm::mat4 skyModel = glm::mat4(1.0f);
        //skyModel = glm::translate(skyModel, glm::vec3(0.0f, 0.0f, 0.0f));

        glm::mat4 skyModel = glm::mat4(1.0f);
        skyModel = glm::rotate(skyModel, glm::radians(0.01f * (h)), glm::vec3(1.0f, 1.0f, 0.0f));
        h++;
        if(h > 36000){
            h = 0;
        }

        skyShader.setMat4("view", viewCube);
        skyShader.setMat4("projection", projection);
        skyShader.setMat4("model", skyModel);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);





        // house rendering
        modelShader.use();

        // directional light
        modelShader.setVec3("dirLight.direction", dirLight.direction);
        modelShader.setVec3("dirLight.ambient", dirLight.ambient);
        modelShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        modelShader.setVec3("dirLight.specular", dirLight.specular);

        //point light
        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        modelShader.setVec3("pointLight.position", pointLight.position);
        modelShader.setVec3("pointLight.ambient", glm::vec3(0.85f, 0.25f, 0.0f));
        modelShader.setVec3("pointLight.diffuse", glm::vec3(0.65f, 0.25f, 0.1f));
        //modelShader.setVec3("pointLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        modelShader.setVec3("pointLight.specular", glm::vec3(1.0f, 0.45f, 0.4f));
        modelShader.setFloat("pointLight.constant", pointLight.constant);
        modelShader.setFloat("pointLight.linear", pointLight.linear);
        modelShader.setFloat("pointLight.quadratic", pointLight.quadratic);


        //lamp point light
        lampPointLight.position = glm::vec3(programState->lampLightPosition);
        modelShader.setVec3("lampPointLight.position", lampPointLight.position);
        modelShader.setVec3("lampPointLight.ambient", glm::vec3(0.85f, 0.25f, 0.0f));
        modelShader.setVec3("lampPointLight.diffuse", glm::vec3(0.65f, 0.25f, 0.1f));
        modelShader.setVec3("lampPointLight.specular", glm::vec3(1.0f, 0.35, 0.35));
        modelShader.setFloat("lampPointLight.constant", lampPointLight.constant);
        modelShader.setFloat("lampPointLight.linear", lampPointLight.linear);
        modelShader.setFloat("lampPointLight.quadratic", lampPointLight.quadratic);
        modelShader.setVec3("viewPosition", programState->camera.Position);
        modelShader.setFloat("material.shininess", 32.0f);

        //spot light
        modelShader.setVec3("spotLight.direction", glm::vec3(0.0f,-1.0f,0.0f));
        modelShader.setVec3("spotLight.position", glm::vec3(10.25, 7.25,13.25));
        modelShader.setVec3("spotLight.ambient", spotLight.ambient);
        modelShader.setVec3("spotLight.diffuse", glm::vec3(0.85f, 0.25f, 0.0f));
        modelShader.setVec3("spotLight.specular", spotLight.specular);
        modelShader.setFloat("spotLight.constant", spotLight.constant);
        modelShader.setFloat("spotLight.linear", spotLight.linear);
        modelShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        modelShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        modelShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        //transforming models
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f));
        model = glm::scale(model, glm::vec3(0.8f));

        modelShader.setMat4("model", model);
        houseModel.Draw(modelShader);

        //lamp
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.f, -1.0f, 12.0f));
        model = glm::scale(model, glm::vec3(0.3f));
        model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        modelShader.setMat4("model", model);
        modelLamp.Draw(modelShader);

        //snow pile rendering
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.f, 0.0f, -20.0f));
        model = glm::scale(model, glm::vec3(4.0f));
        modelShader.setMat4("model", model);
        snowModel.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.f, 0.0f, -20.0f));
        model = glm::scale(model, glm::vec3(4.0f));
        modelShader.setMat4("model", model);
        snowModel2.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.f, 0.0f, -20.0f));
        model = glm::scale(model, glm::vec3(4.0f));
        modelShader.setMat4("model", model);
        snowModel3.Draw(modelShader);



        //mountains
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->mountainPosition);
        model = glm::scale(model, glm::vec3(programState->mountainScale));

        modelShader.setMat4("model", model);
        modelMountain.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->mountainPosition2);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(programState->mountainScale2));

        modelShader.setMat4("model", model);
        modelMountain2.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->mountainPosition3);
        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(programState->mountainScale3));

        modelShader.setMat4("model", model);
        modelMountain3.Draw(modelShader);

        //trees
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 15.0f));
        model = glm::scale(model, glm::vec3(0.09));
        modelShader.setMat4("model",model);
        modelTree.Draw(modelShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f, 0.0f, 25.0f));
        model = glm::scale(model, glm::vec3(0.09));
        modelShader.setMat4("model",model);
        modelTree2.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(8.0f, 0.0f, -9.0f));
        model = glm::scale(model, glm::vec3(0.09));
        modelShader.setMat4("model",model);
        modelTree2.Draw(modelShader);



        //rock
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-14.0f,1.0f,-10.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        modelShader.setMat4("model",model);
        modelRock.Draw(modelShader);


        //sled
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f, 0.2f, 11.0f));
        model = glm::scale(model, glm::vec3(5.0f));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelShader.setMat4("model",model);
        modelSled.Draw(modelShader);

        //fence

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f,0.0f,23.0f));
        model = glm::scale(model, glm::vec3(4.0));
        modelShader.setMat4("model",model);
        modelFence2.Draw(modelShader);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -15.0f));
        model = glm::scale(model, glm::vec3(4.0));
        modelShader.setMat4("model",model);
        modelFence.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-16.0f, 0.0f, -5.0f));
        model = glm::scale(model, glm::vec3(4.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelShader.setMat4("model",model);
        modelFence3.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-16.0f, 0.0f, 13.0f));
        model = glm::scale(model, glm::vec3(4.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelShader.setMat4("model",model);
        modelFence3.Draw(modelShader);

        //plane rendering
        glDisable(GL_CULL_FACE);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));
        model = glm::translate(model, glm::vec3(4.0f, 0.505f, 0.0f));
        modelShader.setMat4("model", model);
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planeTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        //std::cout << programState->lampScale;




        //bed rendering
        modelShader.use();
        modelShader.setVec3("pointLight.position", programState->pointLight.position);
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0f, 2.0f, 2.1f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
        modelShader.setFloat("material.shininess", 5);
        modelShader.setMat4("model", model);
        bedModel.Draw(modelShader);



        //table rendering
        modelShader.use();
        modelShader.setVec3("pointLight.position", programState->pointLight.position);
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.5f, 2.45f, 2.0f));
        model = glm::rotate(model, glm::radians(-9.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.014f));
        modelShader.setFloat("material.shininess", 48);
        modelShader.setMat4("model", model);
        tableModel.Draw(modelShader);


        /*
        //shelf rendering
        modelShader.use();
        modelShader.setVec3("pointLight.position", programState->pointLight.position);
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.67f, 2.5f, 1.0f));
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
        //modelShader.setFloat("material.shininess", 48);
        modelShader.setMat4("model", model);
        shelfModel.Draw(modelShader);





        //rug/carpet rendering
        modelShader.use();
        modelShader.setVec3("pointLight.position", programState->pointLight.position);
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.67f, 2.5f, 1.0f));
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
        //modelShader.setFloat("material.shininess", 48);
        modelShader.setMat4("model", model);
        rugModel.Draw(modelShader);
        */





        //bell rendering (reflective surface)
        reflectShader.use();

        reflectShader.setMat4("projection", projection);
        reflectShader.setMat4("view", view);

        glm::mat4 rot = glm::mat4(1.0f);
        rot = glm::rotate(rot, glm::radians(0.01f * (h)), glm::vec3(1.0f, 1.0f, 1.0f));

        reflectShader.setMat4("rot", rot);

        reflectShader.setVec3("cameraPos", programState->camera.Position);

        model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(4.85f, 4.5f, -2.55f));
        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));

        reflectShader.setMat4("model", model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        bellModel.Draw(reflectShader);




        //normal/parallax supported rendering
        brickShader.use();
        //brickShader.setVec3("lightPos", programState->pointLight.position);
        brickShader.setVec3("lightPos", glm::vec3( 1.3f, 2.85f, -3.85f));
        brickShader.setVec3("viewPos", programState->camera.Position);

        glm::mat4 brickModel = glm::mat4(1.0f);
        brickModel = glm::translate(brickModel, glm::vec3( 1.5f, 2.85f, -3.65f));
        brickModel = glm::scale(brickModel ,glm::vec3(2.3f, 1.279f, 1.0f));
        //brickModel = glm::rotate(brickModel, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        brickModel = glm::rotate(brickModel, glm::radians((float)90.0f * -10.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)));

        //brickShader.setVec3("diffuseL", pointLight.diffuse);
        brickShader.setVec3("diffuseL", glm::vec3(1.0f, 0.45f, 0.15f));
        brickShader.setVec3("ambientL", pointLight.ambient);

        brickShader.setMat4("projection", projection);
        brickShader.setMat4("view", view);
        brickShader.setMat4("model", brickModel);

        /* brickShader.setFloat("constant", pointLight.constant);
         brickShader.setFloat("linear", pointLight.linear);
         brickShader.setFloat("quadratic", pointLight.quadratic);*/

        brickShader.setFloat("heightScale", heightScale); // adjust with Q and E

        brickShader.setFloat("factorD", 1.1f);
        brickShader.setFloat("factorL", 1.4f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickTextureDiff);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brickTextureNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brickTextureDisp);
        //glActiveTexture(GL_TEXTURE3);
        //glBindTexture(GL_TEXTURE_2D, brickTextureSpec);

        renderQuad();

        brickShader.setVec3("lightPos", glm::vec3( 1.3f, 2.85f, 3.85f));
        brickModel = glm::translate(brickModel, glm::vec3( -0.12f, 0.0f, 0.255f));
        brickModel = glm::scale(brickModel ,glm::vec3(.97f, 1.06f, 1.0f));
        brickShader.setMat4("model", brickModel);

        /* brickShader.setVec3("diffuseL", pointLight.diffuse);
         brickShader.setVec3("ambientL", pointLight.ambient);

         brickShader.setFloat("factorD", 0.5f);
         brickShader.setFloat("factorL", 0.7f);

         renderQuad(); */

        brickShader.setVec3("lightPos", glm::vec3( 1.3f, 2.85f, 3.65f));
        brickModel = glm::translate(brickModel, glm::vec3( 0.5f, 0.0f, -7.6f));
        brickModel = glm::scale(brickModel ,glm::vec3(0.7f, 1.06f, 0.5f));
        brickShader.setMat4("model", brickModel);

        brickShader.setVec3("diffuseL", glm::vec3(1.0f, 0.45f, 0.2f));
        brickShader.setVec3("ambientL", pointLight.ambient);

        brickShader.setFloat("factorD", 1.1f);
        brickShader.setFloat("factorL", 1.4f);

        renderQuad();





        //rug rendering with normal maps
        rugShader.use();
        //rugShader.setVec3("lightPos", programState->pointLight.position);
        rugShader.setVec3("lightPos", glm::vec3( 1.5f, 0.84f, -1.65f));
        rugShader.setVec3("viewPos", programState->camera.Position);

        glm::mat4 rugModel = glm::mat4(1.0f);

        rugModel = glm::rotate(rugModel, glm::radians(90.0f), glm::vec3( 1.0f, 0.0f, 0.0f));

        rugModel = glm::translate(rugModel, glm::vec3( 0.5f, 0.44f, -1.65f));

        rugShader.setMat4("projection", projection);
        rugShader.setMat4("view", view);
        rugShader.setMat4("model", rugModel);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rugTextureDiff);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rugTextureNormal);

        renderQuad();

        glEnable(GL_CULL_FACE);



        //transparent objects are rendered last
        //windows rendering

        std::sort(winPos.begin(), winPos.end(), [](triD a, triD b){
                float d1 = glm::distance(a.trans, programState->camera.Position);
                float d2 = glm::distance(b.trans, programState->camera.Position);
                return d1 > d2;
        });

        //this goes before window implementation
        glDisable(GL_CULL_FACE);
        windowShader.use();
        glBindVertexArray(windowVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, windowTexture);

        windowShader.setMat4("view", view);
        windowShader.setMat4("projection", projection);

        for(int i = 0 ;i < winPos.size(); i++){

            model = glm::mat4(1.0f);

            model = glm::translate(model, winPos[i].trans);
            model = glm::scale(model, winPos[i].skal);
            model = glm::rotate(model, glm::radians(winPos[i].rotatU), winPos[i].rotatV);

            /*if(glm::vec3(3.0f,2.7f,1.0f) == winPos[i].trans){
                model = glm::rotate(model, glm::radians(winPos[i].rotatU), glm::vec3(1.0f,0.0f,0.0f));
            }*/

            windowShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 6);

        }

        /*model = glm::rotate(model, glm::radians(-43.0f), glm::vec3(1.0, 0, 0));
        model = glm::translate(model, glm::vec3(0.03f,2.87f,13.91f));
        model = glm::rotate(model, glm::radians(43.0f), glm::vec3(1.0, 0, 0));
        windowShader.setMat4("model", model);*/

        //this goes after window implementation
        glEnable(GL_CULL_FACE);




        //POST PROCESSING
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorBuffers[!horizontal]);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        hdrShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[!horizontal]);
        hdrShader.setBool("hdr", hdr);
        hdrShader.setBool("bloom", bloom);
        hdrShader.setFloat("exposure", programState->exposure);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        antiAliasingShader.use();
        antiAliasingShader.setInt("grayEffect", grayEffect);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);


        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime * 2);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime * 2);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime * 2);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime * 2);



    //changing height scale of parralax using Q and E
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hdrKeyPressed)
    {
        hdr = !hdr;
        hdrKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
    {
        hdrKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        if (programState->exposure > 0.0f)
            programState->exposure -= 0.001f;
        else
            programState->exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        programState->exposure += 0.001f;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        ImGui::Begin("Hello!");
        ImGui::SliderFloat("Exposure", &programState->exposure, 0.0, 2.0);
        ImGui::DragFloat3("Mountain position", (float*)&programState->mountainPosition);
        ImGui::DragFloat3("Mountain position - 2", (float*)&programState->mountainPosition2);
        ImGui::DragFloat3("Mountain position - 3", (float*)&programState->mountainPosition3);
        ImGui::DragFloat("Mountain scale", &programState->mountainScale, 0.005, 0.005, 0.5);
        ImGui::DragFloat("Mountain scale", &programState->mountainScale2, 0.005, 0.005, 0.5);
        ImGui::DragFloat("Mountain scale", &programState->mountainScale3, 0.005, 0.005, 0.5);
        ImGui::DragFloat3("Point light position", (float*)&programState->lampLightPosition);
        ImGui::DragFloat3("Model position", (float*)&programState->modelPosition);

        
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if(key == GLFW_KEY_G && action == GLFW_PRESS){
        grayEffect = !grayEffect;
    }
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
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





unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at this path : " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}




unsigned int sqVAO = 0;
unsigned int sqVBO;
void renderQuad() {
    if (sqVAO == 0) {
        // positions
        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float sqVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z,
                bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z,
                bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &sqVAO);
        glGenBuffers(1, &sqVBO);
        glBindVertexArray(sqVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sqVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sqVertices), &sqVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) (6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) (8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) (11 * sizeof(float)));
    }
    glBindVertexArray(sqVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

