#ifndef QUADTEXTURE_H
#define QUADTEXTURE_H
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb/stb_image.h>

#include <engine/shader_m.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class QuadTexture
{
private:
    /* data */
    Shader *QuadTextureShader;
    float vertices[20];
    unsigned int diffuseMap;
    unsigned int QuadTextureVBO, QuadTextureVAO, QuadTextureEBO;
    float sheetWidth, sheetHeight, spriteWidth, spriteHeight;

    void loadTexImageData(float *vertices, float _sheetWidth, float _sheetHeight, float _spriteWidth, float _spriteHeight, float x, float y)
    {

        float tc[4][2] = {
            {((x + 1) * _spriteWidth) / _sheetWidth, ((y + 1) * _spriteHeight) / _sheetHeight}, // top right
            {(x * _spriteWidth) / _sheetWidth, ((y + 1) * _spriteHeight) / _sheetHeight},       // top left
            {(x * _spriteWidth) / _sheetWidth, (y * _spriteHeight) / _sheetHeight},             // bottom left
            {((x + 1) * _spriteWidth) / _sheetWidth, (y * _spriteHeight) / _sheetHeight}        // bottom right
        };
        float verts[] = {
            // positions        // texture coords
            -0.5f, 0.5f, 0.0f, tc[2][0], tc[2][1],  //top right
            -0.5f, -0.5f, 0.0f, tc[1][0], tc[1][1], //bottom right
            0.5f, -0.5f, 0.0f, tc[0][0], tc[0][1],  //bottom left
            0.5f, 0.5f, 0.0f, tc[3][0], tc[3][1],   //top left
        };
        for (int i = 0; i < 20; i++)
        {
            *(vertices + i) = *(verts + i);
        }
    }

public:
    QuadTexture()
    {

    }
    QuadTexture(const char *filePath, float _spriteWidth, float _spriteHeight, float x, float y)
    {
        // build and compile our shader zprogram
        // ------------------------------------
        QuadTextureShader = new Shader("shaders/gui.vs", "shaders/gui.fs");
        int _sheetWidth, _sheetHeight, nrChannels;
        // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
        unsigned char *data = stbi_load(filePath, &_sheetWidth, &_sheetHeight, &nrChannels, 0);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        sheetWidth = _sheetWidth;
        sheetHeight = _sheetHeight;

        spriteWidth = _spriteWidth * 2;
        spriteHeight = _spriteHeight * 2;

        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };
        loadTexImageData(vertices, _sheetWidth, _sheetHeight, _spriteWidth * 2, _spriteHeight * 2, x, y);
        if (data)
        {
            glGenVertexArrays(1, &QuadTextureVAO);
            glGenBuffers(1, &QuadTextureVBO);
            glGenBuffers(1, &QuadTextureEBO);

            glBindVertexArray(QuadTextureVAO);

            glBindBuffer(GL_ARRAY_BUFFER, QuadTextureVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QuadTextureEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);

            // texture coord attribute
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glGenTextures(1, &diffuseMap);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // load image, create texture and generate mipmaps

            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, sheetWidth, sheetHeight, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        QuadTextureShader->use();
        QuadTextureShader->setInt("texture1", 0);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    void changeSprite(float x, float y)
    {
        loadTexImageData(vertices, sheetWidth, sheetHeight, spriteWidth, spriteHeight, x, y);
        glBindBuffer(GL_ARRAY_BUFFER, QuadTextureVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    }
    void Draw(glm::vec2 QuadTexturePos, float scale)
    {

        QuadTextureShader->use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(QuadTexturePos, 0.0f));
        model = glm::scale(model, glm::vec3(scale)); // a smaller cube
        QuadTextureShader->setMat4("model", model);

        glEnable(GL_BLEND);
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        // render container
        QuadTextureShader->use();
        glBindVertexArray(QuadTextureVAO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);
    }

    void Release()
    {
        glDeleteVertexArrays(1, &QuadTextureVAO);
        glDeleteBuffers(1, &QuadTextureVBO);
        glDeleteBuffers(1, &QuadTextureEBO);
        delete QuadTextureShader;
    }

    ~QuadTexture()
    {
    }
};
#endif
