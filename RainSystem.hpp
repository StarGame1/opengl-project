#ifndef RainSystem_hpp
#define RainSystem_hpp

#include <vector>
#include <random>
#include <glm/glm.hpp>
#include "Shader.hpp"

class RainParticle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float life;

    RainParticle() {
        reset();
    }

    void reset() {
        // Generăm o poziție random deasupra camerei într-o zonă largă
        position.x = ((float)(rand() % 2000 - 1000)) * 0.1f;  // -100 to 100
        position.y = 50.0f + ((float)(rand() % 100)) * 0.1f;    // 50 to 60
        position.z = ((float)(rand() % 2000 - 1000)) * 0.1f;  // -100 to 100

        // Viteză cu variație mică pentru efect natural
        velocity = glm::vec3(
            ((float)(rand() % 100 - 50)) * 0.01f,  // mică variație pe x
            -20.0f - ((float)(rand() % 100)) * 0.05f, // viteză în jos
            ((float)(rand() % 100 - 50)) * 0.01f    // mică variație pe z
        );

        life = 1.0f;
    }

    void update(float deltaTime) {
        position += velocity * deltaTime;
        if (position.y < -2.0f) {
            reset();
        }
    }
};

class RainSystem {
private:
    std::vector<RainParticle> particles;
    GLuint VAO, VBO;
    bool enabled;
    bool initialized;

public:
    RainSystem(int count = 10000) : enabled(false), initialized(false) {
        particles.resize(count);
        for(auto& particle : particles) {
            particle.reset();
        }
    }

    void init() {
        if (!initialized) {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            // Alocăm spațiu pentru toate particulele (2 vertices per particle * 3 coordinates)
            glBufferData(GL_ARRAY_BUFFER, particles.size() * 6 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            glBindVertexArray(0);
            initialized = true;
        }
    }

    void update(float deltaTime) {
        if (!enabled || !initialized) return;

        std::vector<float> vertexData;
        vertexData.reserve(particles.size() * 6); // 2 vertices per particle * 3 coordinates

        for(auto& particle : particles) {
            particle.update(deltaTime);

            // Pentru fiecare particulă, adăugăm o linie verticală
            glm::vec3 bottomPos = particle.position;
            glm::vec3 topPos = bottomPos + glm::vec3(0.0f, 0.3f, 0.0f); // Lungimea picăturii

            // Bottom vertex
            vertexData.push_back(bottomPos.x);
            vertexData.push_back(bottomPos.y);
            vertexData.push_back(bottomPos.z);

            // Top vertex
            vertexData.push_back(topPos.x);
            vertexData.push_back(topPos.y);
            vertexData.push_back(topPos.z);
        }

        // Update VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(float), vertexData.data());
    }

    void draw(gps::Shader& shader) {
        if (!enabled || !initialized) return;

        shader.useShaderProgram();
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, particles.size() * 2);
        glBindVertexArray(0);
    }

    void toggle() {
        enabled = !enabled;
    }

    bool isEnabled() const {
        return enabled;
    }

    ~RainSystem() {
        if (initialized) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }
    }
};

#endif