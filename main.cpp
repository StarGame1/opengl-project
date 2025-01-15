//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB

#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include "RainSystem.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <vector>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>



int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow *glWindow = NULL;

GLuint motionBlurFBO, motionBlurTexture;
gps::Shader motionBlurShader;
unsigned int quadVAO = 0;
unsigned int quadVBO;

GLuint postProcessFBO, postProcessTexture;
GLuint velocityFBO, velocityTexture;
gps::Shader fxaaShader;

glm::mat4 previousViewProjection;

enum RenderMode {
    SOLID,
    WIREFRAME,
    POLYGONAL,
    SMOOTH
};

RenderMode currentRenderMode = SOLID;

gps::Shader rainShader;
RainSystem rainSystem(20000); // 20000 de picături
float lastFrame = 0.0f;

glm::vec3 pointLightPos = glm::vec3(-0.10f, 7.59f, 3.23f);
float pointLightMovementSpeed = 0.1f;

// Add these variables at the top of main.cpp with other global variables
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;

float currentYaw = 0.0f;
float currentPitch = 0.0f;


// Valorile fixe pentru rotație
float startYaw = -20.0f;    // Modifică aceste valori cu cele dorite
float startPitch = 10.0f;    // pentru poziția de start
float endYaw = -250.0f;       // și pentru poziția finală
float endPitch = 0.0f;

// Camera animation variables
bool isAnimating = false;
float animationTime = 0.0f;
const float ANIMATION_DURATION = 10.0f; // Animation will take 2 seconds

glm::vec3 startPosition(8.91f, 1.56f, -6.35f);
glm::vec3 endPosition(-0.96f, 2.52f, -3.24f);



const float WIND_MAX_STRENGTH = 2.0f;  // Mărește range-ul vântului


bool fogEnabled = false;
glm::vec3 fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
bool rainEnabled = false;
float windStrength = 0.0f;
glm::vec3 windDirection = glm::vec3(1.0f, 0.0f, 0.0f);


const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

GLuint pointLightPosLoc;
glm::vec3 pointLightColor;
GLuint pointLightColorLoc;

gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, 5.5f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.01f;

bool pressedKeys[1024];
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;

float angleY = 0.0f;

float lastX = glWindowWidth / 2.0f;
float lastY = glWindowHeight / 2.0f;
// float yaw = -90.0f;
// float pitch = 0.0f;
// bool firstMouse = true;

GLfloat lightAngle;

gps::Model3D map;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;
gps::Shader skyboxShader;

float skyboxVertices[] = {
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f
};
bool showDepthMap;


void renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int loadCubemap(std::vector<std::string>);

GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM: error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE: error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION: error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow *window, int width, int height) {
    fprintf(stdout, "Window resized to width: %d, height: %d\n", width, height);
    glWindowWidth = width;
    glWindowHeight = height;

    // Update viewport and projection matrix for new window dimensions
    glViewport(0, 0, retina_width, retina_height);
    projection = glm::perspective(glm::radians(80.0f),
                                  (float) retina_width / (float) retina_height,
                                  0.1f, 1000.0f);
    myCustomShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

// Update the keyboard callback function
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        isAnimating = true;
        animationTime = 0.0f;

        // Force the initial values
        yaw = startYaw;
        pitch = startPitch;

        // Debug print
        printf("Starting animation with yaw=%.2f, pitch=%.2f\n", yaw, pitch);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction = glm::normalize(direction);

        myCamera = gps::Camera(
            startPosition,
            startPosition + direction,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
    }

    // Controale pentru modurile de vizualizare
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        myCustomShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "renderMode"), 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        myCustomShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "renderMode"), 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        myCustomShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "renderMode"), 2);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        myCustomShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "renderMode"), 3);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        fogEnabled = !fogEnabled;
        myCustomShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "fogEnabled"), fogEnabled);
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        rainSystem.toggle();
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        windStrength = (windStrength == 0.0f) ? WIND_MAX_STRENGTH : 0.0f;
        myCustomShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "windStrength"), windStrength);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}
void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_Q]) {
        angleY -= 1.0f;
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angleY += 1.0f;
    }

    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 1.0f;
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 1.0f;
    }

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_W] || pressedKeys[GLFW_KEY_S] || pressedKeys[GLFW_KEY_A] || pressedKeys[GLFW_KEY_D]) {
        glm::vec3 cameraPos = myCamera.getCameraPosition();
        printf("Camera position - X: %.2f, Y: %.2f, Z: %.2f\n", cameraPos.x, cameraPos.y, cameraPos.z);
        if (pressedKeys[GLFW_KEY_UP]) {
            pointLightPos.z -= pointLightMovementSpeed;
            printf("Light pos: X=%.2f, Y=%.2f, Z=%.2f\n", pointLightPos.x, pointLightPos.y, pointLightPos.z);
        }
        if (pressedKeys[GLFW_KEY_DOWN]) {
            pointLightPos.z += pointLightMovementSpeed;
            printf("Light pos: X=%.2f, Y=%.2f, Z=%.2f\n", pointLightPos.x, pointLightPos.y, pointLightPos.z);
        }
        if (pressedKeys[GLFW_KEY_LEFT]) {
            pointLightPos.x -= pointLightMovementSpeed;
            printf("Light pos: X=%.2f, Y=%.2f, Z=%.2f\n", pointLightPos.x, pointLightPos.y, pointLightPos.z);
        }
        if (pressedKeys[GLFW_KEY_RIGHT]) {
            pointLightPos.x += pointLightMovementSpeed;
            printf("Light pos: X=%.2f, Y=%.2f, Z=%.2f\n", pointLightPos.x, pointLightPos.y, pointLightPos.z);
        }
        if (pressedKeys[GLFW_KEY_PAGE_UP]) {
            pointLightPos.y += pointLightMovementSpeed;
            printf("Light pos: X=%.2f, Y=%.2f, Z=%.2f\n", pointLightPos.x, pointLightPos.y, pointLightPos.z);
        }
        if (pressedKeys[GLFW_KEY_PAGE_DOWN]) {
            pointLightPos.y -= pointLightMovementSpeed;
            printf("Light pos: X=%.2f, Y=%.2f, Z=%.2f\n", pointLightPos.x, pointLightPos.y, pointLightPos.z);
        }

        // Update light position in shader
        myCustomShader.useShaderProgram();
        glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));
    }
}

bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);

    glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    // get version info
    const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte *version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    //for RETINA display
    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, retina_width, retina_height);

    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

    glEnable(GL_FRAMEBUFFER_SRGB);
    rainSystem.init();
}

void initObjects() {
    std::string modelPath = "models/mirror.obj";
    std::cout << "Loading model from: " << modelPath << std::endl;
    map.LoadModel(modelPath);
}

void initShaders() {
    myCustomShader.loadShader("shaders/shader.vert", "shaders/shader.frag");
    myCustomShader.useShaderProgram();
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
     rainShader.loadShader("shaders/rain.vert", "shaders/rain.frag");
    fxaaShader.loadShader("shaders/fxaa.vert", "shaders/fxaa.frag");
    motionBlurShader.loadShader("shaders/motionBlur.vert", "shaders/motionBlur.frag");

}

void initUniforms() {
    myCustomShader.useShaderProgram();

    // Set PBR parameters
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "metallic"), 0.0f);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "roughness"), 0.5f);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "ao"), 1.0f);

    // Set visual enhancement parameters
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "exposure"), 15.0f);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "saturation"), 1.5f);
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "contrast"), 1.5f);


    // Model matrix
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // View matrix
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // Create projection matrix
    projection = glm::perspective(glm::radians(60.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // Point light setup
    pointLightPos = glm::vec3(-0.10f, 7.59f, 3.23f);
    pointLightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos");
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPos));

    pointLightColor = glm::vec3(1.0f, 0.95f, 0.8f); // Warm white light
    pointLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor");
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    // Set point light intensity
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightIntensity"), 50.0f);
}

void initFBO() {
    // Create the FBO for depth map
    glGenFramebuffers(1, &shadowMapFBO);

    // Create depth texture
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Create motion blur FBO and texture
    glGenFramebuffers(1, &motionBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, motionBlurFBO);

    glGenTextures(1, &motionBlurTexture);
    glBindTexture(GL_TEXTURE_2D, motionBlurTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, retina_width, retina_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, motionBlurTexture, 0);

    // Create and attach depth buffer for motion blur
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, retina_width, retina_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkybox() {
    // Generare VAO și VBO pentru skybox
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

    // Încărcare texturi pentru skybox
    std::vector<std::string> faces = {
        "skybox/negx.jpg",
        "skybox/negy.jpg",
        "skybox/negz.jpg",
        "skybox/posx.jpg",
        "skybox/posy.jpg",
        "skybox/posz.jpg"
    };
    cubemapTexture = loadCubemap(faces);

    // Inițializare shader pentru skybox
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(skyboxShader.shaderProgram, "skybox"), 0);
}

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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

glm::mat4 computeLightSpaceTrMatrix() {
    const GLfloat near_plane = 1.0f, far_plane = 10.0f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

    glm::vec3 lightDirRotated = glm::vec3(lightRotation * glm::vec4(lightDir, 0.0f));
    glm::mat4 lightView = glm::lookAt(lightDirRotated * -2.0f, // Light position
                                      glm::vec3(0.0f), // Look at origin
                                      glm::vec3(0.0f, 1.0f, 0.0f)); // Up vector

    return lightProjection * lightView;
}

void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    map.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    ground.Draw(shader);
}

void renderScene() {
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;

    // First render pass (render to motion blur FBO)
    glBindFramebuffer(GL_FRAMEBUFFER, motionBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Animation code
    if (isAnimating) {
        animationTime += deltaTime;
        float t = glm::clamp(animationTime / ANIMATION_DURATION, 0.0f, 1.0f);
        float smoothT = (1.0f - cos(t * glm::pi<float>())) * 0.5f;

        glm::vec3 newPosition = glm::mix(startPosition, endPosition, smoothT);
        float interpolatedYaw = glm::mix(startYaw, endYaw, smoothT);
        yaw = interpolatedYaw;
        pitch = glm::mix(startPitch, endPitch, smoothT);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction = glm::normalize(direction);

        myCamera = gps::Camera(
            newPosition,
            newPosition + direction,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        if (t >= 1.0f) {
            isAnimating = false;
        }
    }

    // Shadow mapping pass
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    myCustomShader.useShaderProgram();
    glm::mat4 lightSpaceMatrix = computeLightSpaceTrMatrix();
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
                       1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    drawObjects(myCustomShader, true);

    // Main rendering pass
    glViewport(0, 0, retina_width, retina_height);
    glBindFramebuffer(GL_FRAMEBUFFER, motionBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myCustomShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    // Update all your regular uniforms
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "time"), currentTime);

    if (fogEnabled) {
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "fogEnabled"), fogEnabled);
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "fogColor"), 1, glm::value_ptr(fogColor));
    }

    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "windStrength"), windStrength);
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "windDirection"), 1, glm::value_ptr(windDirection));

    // Bind shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
                       1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    // Update point light
    glm::vec3 pointLightPosView = glm::vec3(view * glm::vec4(pointLightPos, 1.0));
    glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(pointLightPosView));

    // Draw scene
    drawObjects(myCustomShader, false);

    // Draw lights
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"),
                       1, GL_FALSE, glm::value_ptr(view));

    // Draw directional light cube
    model = lightRotation;
    model = glm::translate(model, 1.0f * lightDir);
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"),
                       1, GL_FALSE, glm::value_ptr(model));
    lightCube.Draw(lightShader);

    // Draw point light cube
    model = glm::mat4(1.0f);
    model = glm::translate(model, pointLightPos);
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"),
                       1, GL_FALSE, glm::value_ptr(model));
    lightCube.Draw(lightShader);

    // Draw rain
    if (rainSystem.isEnabled()) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        rainSystem.update(deltaTime);
        rainShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "projection"),
                          1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"),
                          1, GL_FALSE, glm::value_ptr(view));
        rainSystem.draw(rainShader);

        glDisable(GL_BLEND);
    }

    // Draw skybox
    glDepthFunc(GL_LEQUAL);
    skyboxShader.useShaderProgram();
    view = glm::mat4(glm::mat3(myCamera.getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"),
                       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"),
                       1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    // Final pass: render to screen with motion blur
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    motionBlurShader.useShaderProgram();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, motionBlurTexture);
    glUniform1i(glGetUniformLocation(motionBlurShader.shaderProgram, "screenTexture"), 0);

    renderQuad();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    glfwDestroyWindow(glWindow);
    glDeleteFramebuffers(1, &motionBlurFBO);
    glDeleteTextures(1, &motionBlurTexture);
    if(quadVAO != 0) {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
    //close GL context and any other GLFW resources
    glfwTerminate();
}

int main(int argc, const char *argv[]) {
    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initOpenGLState();
    initObjects();
    initShaders();
    initUniforms();
    initFBO();
    initSkybox();

    glCheckError();

    while (!glfwWindowShouldClose(glWindow)) {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();

    return 0;
}
