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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>


int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow *glWindow = NULL;

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


gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, 5.5f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.01f;

bool pressedKeys[1024];
float angleY = 0.0f;
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

bool showDepthMap;

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

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized to width: %d, height: %d\n", width, height);
    glWindowWidth = width;
    glWindowHeight = height;

    // Update viewport and projection matrix for new window dimensions
    glViewport(0, 0, retina_width, retina_height);
    projection = glm::perspective(glm::radians(45.0f),
                                (float)retina_width / (float)retina_height,
                                0.1f, 1000.0f);
    myCustomShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
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
    //glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
}

void initObjects() {
    std::string modelPath = "models/mirror-edge-bake_final.obj";
    std::cout << "Loading model from: " << modelPath << std::endl;
    map.LoadModel(modelPath);
}

void initShaders() {
    myCustomShader.loadShader("shaders/shader.vert", "shaders/shader.frag");
    myCustomShader.useShaderProgram();
    // lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    // lightShader.useShaderProgram();
    // screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    // screenQuadShader.useShaderProgram();
    // skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    // skyboxShader.useShaderProgram();
}

void initUniforms() {
    myCustomShader.useShaderProgram();

    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(45.0f), (float) retina_width / (float) retina_height, 0.1f, 1000.0f);
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

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
}

void initSkyBox() {
    std::vector<const GLchar *> faces, faces2;
    // faces.push_back("skybox/right.tga");
    // faces.push_back("skybox/left.tga");
    // faces.push_back("skybox/top.tga");
    // faces.push_back("skybox/bottom.tga");
    // faces.push_back("skybox/back.tga");
    // faces.push_back("skybox/front.tga");
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
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    const GLfloat near_plane = 1.0f, far_plane = 10.0f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

    glm::vec3 lightDirRotated = glm::vec3(lightRotation * glm::vec4(lightDir, 0.0f));
    glm::mat4 lightView = glm::lookAt(lightDirRotated * -2.0f,  // Light position
                                     glm::vec3(0.0f),           // Look at origin
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
    // 1. First render to depth map
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Use custom shader for depth pass
    myCustomShader.useShaderProgram();
    glm::mat4 lightSpaceMatrix = computeLightSpaceTrMatrix();
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
                       1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    drawObjects(myCustomShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Render scene normally with shadow mapping
    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    } else {
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myCustomShader.useShaderProgram();

        // Update view matrix and light direction
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

        // Bind shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

        // Send light space matrix to shader
        glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
                          1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        drawObjects(myCustomShader, false);

        // Draw light cube
        lightShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"),
                          1, GL_FALSE, glm::value_ptr(view));

        model = lightRotation;
        model = glm::translate(model, 1.0f * lightDir);
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"),
                          1, GL_FALSE, glm::value_ptr(model));

        lightCube.Draw(lightShader);
    }
}
void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    glfwDestroyWindow(glWindow);
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
    initSkyBox();
    initFBO();
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