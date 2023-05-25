#ifndef PLANE_H
#define PLANE_H
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb/stb_image.h>

#include <engine/shader_m.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Plane
{
private:
    Shader *shader;
    float vertices[20];
    unsigned int diffuseMap;
    unsigned int VAO, VBO, EBO;
    float sheetWidth, sheetHeight, spriteWidth, spriteHeight;
    glm::vec3 position;
    glm::vec3 angles;
    glm::vec3 scale;

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
    Plane()
    {
    }
    Plane(const char *filePath, float _spriteWidth, float _spriteHeight, float x, float y)
    {
        position = glm::vec3(0.0f);
        angles = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);

        shader = new Shader("shaders/figures.vs", "shaders/figures.fs");
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
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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
        shader->use();
        shader->setInt("texture1", 0);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    ~Plane()
    {
    }
    void draw(float x, float y, glm::mat4 view, glm::mat4 projection)
    {
        shader->use();

        loadTexImageData(vertices, sheetWidth, sheetHeight, spriteWidth, spriteHeight, x, y);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, this->position);
        model = glm::rotate(model, glm::radians(this->angles.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(this->angles.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(this->angles.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, this->scale); // a smaller cube

        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
        glEnable(GL_BLEND);
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);
    }

    glm::vec3 getPosition()
    {
        return this->position;
    }
    void setPosition(glm::vec3 position)
    {
        this->position = position;
    }

    glm::vec3 getAngles()
    {
        return this->angles;
    }
    void setAngles(glm::vec3 angles)
    {
        this->angles = angles;
    }

    glm::vec3 getScale()
    {
        return this->scale;
    }
    void setScale(glm::vec3 scale)
    {
        this->scale = scale;
    }
    void Release()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        delete shader;
    }
};

#endif // !PLANE_H
