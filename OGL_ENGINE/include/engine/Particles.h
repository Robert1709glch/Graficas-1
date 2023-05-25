#define GLM_ENABLE_EXPERIMENTAL
#ifndef PARTICLES_H
#define PARTICLES_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <engine/Random.h>
#include <engine/shader_m.h>


#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>


struct ParticleProps
{
    glm::vec3 Position;
    glm::vec3 Velocity, VelocityVariation;
    glm::vec4 ColorBegin, ColorEnd;
    float SizeBegin, SizeEnd, SizeVariation;
    float LifeTime = 1.0f;
};

struct Particle
{
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec4 ColorBegin, ColorEnd;
    float Rotation = 0.0f;
    float SizeBegin, SizeEnd;

    float LifeTime = 1.0f;
    float LifeRemaining = 0.0f;

    bool Active = false;
};

class Particles
{
private:
    /* data */
    std::vector<Particle> m_ParticlePool;
    int m_PoolIndex = 999;

    Shader* ParticlesShader;
    unsigned int diffuseMap;
    unsigned int ParticlesVBO, ParticlesVAO, ParticlesEBO;
    float sheetWidth,  sheetHeight,  spriteWidth, spriteHeight;



public:
    Particles()
    {

    }
    Particles(const char* filePath)
    {
        // build and compile our shader zprogram
        // ------------------------------------
         m_ParticlePool.resize(1000);
         ParticlesShader = new Shader("shaders/particles.vs", "shaders/particles.fs");
         int _sheetWidth, _sheetHeight, nrChannels;
         unsigned char* data = stbi_load(filePath, &_sheetWidth, &_sheetHeight, &nrChannels, 0);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------


         unsigned int indices[] = {
           0, 1, 3, // first triangle
           1, 2, 3  // second triangle
         };


         float vertices[] = {
             // positions        // texture coords
              -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, //bottom left
               0.5f, -0.5f, 0.0f,  1.0f, 0.0f,   //bottom right
               0.5f,  0.5f, 0.0f,  1.0f, 1.0f, //top right
              -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,//top left
         };

        if (data)
        {         
            glGenVertexArrays(1, &ParticlesVAO);
            glGenBuffers(1, &ParticlesVBO);
            glGenBuffers(1, &ParticlesEBO);

            glBindVertexArray(ParticlesVAO);

            glBindBuffer(GL_ARRAY_BUFFER, ParticlesVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ParticlesEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // texture coord attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);


            glGenTextures(1, &diffuseMap);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // load image, create texture and generate mipmaps


            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, _sheetWidth, _sheetHeight, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        ParticlesShader->use();
        ParticlesShader->setInt("texture1", 0);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
  
    void Draw(glm::vec3 position,glm::mat4 view, glm::mat4 projection)
    {
       
        for (auto& particle : m_ParticlePool)
        {
            if (!particle.Active)
                continue;

            // Fade away particles
            float life = particle.LifeRemaining / particle.LifeTime;
            glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
            color.a = color.a * life;

            float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

            // Render
            // bind textures on corresponding texture units
            // render container
          
            ParticlesShader->use();

            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, particle.Position);
            transform = glm::translate(transform, position);
            transform = glm::rotate(transform, particle.Rotation, { 0.0f, 0.0f, 1.0f });
            transform = glm::scale(transform, { size, size, size });

            glEnable(GL_BLEND);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            glBindVertexArray(ParticlesVAO);
            ParticlesShader->setMat4("model", transform);
            ParticlesShader->setMat4("view", view);
            ParticlesShader->setMat4("projection", projection);
            ParticlesShader->setVec4("color",  color);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glDisable(GL_BLEND);     
        }     
      
    }
   

    void OnUpdate(float velocity)
    {
      
        for (auto& particle : m_ParticlePool)
        {
            if (!particle.Active)
                continue;

            if (particle.LifeRemaining <= 0.0f)
            {
                particle.Active = false;
                continue;
            }

            particle.LifeRemaining -= velocity;
            particle.Position += particle.Velocity * velocity;
            particle.Rotation += 0.01f * velocity;
        }
    }


    void Emit(const ParticleProps& particleProps)
    {
        
        Particle& particle = m_ParticlePool[m_PoolIndex];
        particle.Active = true;
        particle.Position = particleProps.Position;
        particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

        // Velocity
        particle.Velocity = particleProps.Velocity;
        particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
        particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);
        particle.Velocity.z += particleProps.VelocityVariation.z * (Random::Float() - 0.5f);

        // Color
        particle.ColorBegin = particleProps.ColorBegin;
        particle.ColorEnd = particleProps.ColorEnd;

        particle.LifeTime = particleProps.LifeTime;
        particle.LifeRemaining = particleProps.LifeTime;
        particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
        particle.SizeEnd = particleProps.SizeEnd;

        m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();


    }



    void Release()
    {
        glDeleteVertexArrays(1, &ParticlesVAO);
        glDeleteBuffers(1, &ParticlesVBO);
        glDeleteBuffers(1, &ParticlesEBO);
        delete ParticlesShader;
    }
  
    ~Particles()
    {
    }
   };
#endif
