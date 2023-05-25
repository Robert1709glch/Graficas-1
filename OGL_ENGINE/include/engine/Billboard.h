#ifndef BILLBOARD_H
#define BILLBOARD_H
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb/stb_image.h>

#include <engine/camera.h>
#include <engine/shader_m.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Billboard
{
private:
    /* data */
    Shader *BillboardShader;
    unsigned int diffuseMap;
    unsigned int BillboardVBO, BillboardVAO, BillboardEBO;
    glm::vec3 position, origin;
    float scale, rotationAngle, sWidth, sHeight, sheetWidth, sheetHeight, spriteWidth, spriteHeight;
    float vertices[20];

    void reloadData(float *vertices, Camera camera, float width, float height, float x, float y)
    {

        glm::vec3 billcam = glm::vec3(position.x - camera.Position.x,
                                      position.y - camera.Position.y,
                                      position.z - camera.Position.z);

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, billcam));
        right.x *= width / 2.0;
        right.z *= width / 2.0;

        float tc[4][2] = {
            {((x + 1) * spriteWidth) / sheetWidth, ((y + 1) * spriteHeight) / sheetHeight}, // top right
            {(x * spriteWidth) / sheetWidth, ((y + 1) * spriteHeight) / sheetHeight},       // top left

            {(x * spriteWidth) / sheetWidth, (y * spriteHeight) / sheetHeight},             // bottom left
            {((x + 1) * spriteWidth) / sheetWidth, (y * spriteHeight) / sheetHeight},       // bottom right
        };

        float verts[] = {
            // positions        // texture coords
            origin.x - right.x, origin.y, origin.z - right.z, tc[0][0], tc[0][1],          //bottom left
            origin.x + right.x, origin.y, origin.z + right.z, tc[1][0], tc[1][1],          //bottom right
            
            origin.x + right.x, origin.y + height, origin.z + right.z, tc[2][0], tc[2][1], //top right
            origin.x - right.x, origin.y + height, origin.z - right.z, tc[3][0], tc[3][1], //top left
        };

        for (int i = 0; i < 20; i++)
        {
            *(vertices + i) = *(verts + i);
        }
    }

    void reloadData(float *vertices)
    {

        float verts[] = {
            // positions        // texture coords
            -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, //bottom left
            0.5f, -0.5f, 0.0f, 0.0f, 1.0f,  //bottom right
            0.5f, 0.5f, 0.0f, 0.0f, 0.0f,   //top right
            -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  //top left
        };

        for (int i = 0; i < 20; i++)
        {
            *(vertices + i) = *(verts + i);
        }
    }

public:
    Billboard()
    {

    }
    Billboard(const char *filePath, float screenWidth, float screenHeight, float _spriteWidth, float _spriteHeight)
    {

        sWidth = screenWidth;
        sHeight = screenHeight;

        origin = glm::vec3(0.0f, 0.0f, 0.0f);
        // build and compile our shader zprogram
        // ------------------------------------
        BillboardShader = new Shader("shaders/billboard.vs", "shaders/billboard.fs");
        int Width, Height, nrChannels;
        // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
        unsigned char *data = stbi_load(filePath, &Width, &Height, &nrChannels, 0);

        sheetWidth = Width;
        sheetHeight = Height;
        spriteWidth = _spriteWidth * 2;
        spriteHeight = _spriteHeight * 2;

        reloadData(vertices);
        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };

        if (data)
        {
            glGenVertexArrays(1, &BillboardVAO);
            glGenBuffers(1, &BillboardVBO);
            glGenBuffers(1, &BillboardEBO);

            glBindVertexArray(BillboardVAO);

            glBindBuffer(GL_ARRAY_BUFFER, BillboardVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BillboardEBO);
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

            glTexImage2D(GL_TEXTURE_2D, 0, format, Width, Height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
        // -------------------------------------------------------------------------------------------
        BillboardShader->use();
        BillboardShader->setInt("texture1", 0);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    void Draw(Camera camera, float x, float y)
    {

        BillboardShader->use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                sWidth / sHeight, 0.1f, 100.0f);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(scale));

        BillboardShader->setMat4("projection", projection);
        BillboardShader->setMat4("view", view);
        BillboardShader->setMat4("model", model);

        glEnable(GL_BLEND);
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        // render container
        glBindVertexArray(BillboardVAO);
        reloadData(vertices, camera, 1, 1, x, y);
        glBindBuffer(GL_ARRAY_BUFFER, BillboardVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);
    }

    void Release()
    {
        glDeleteVertexArrays(1, &BillboardVAO);
        glDeleteBuffers(1, &BillboardVBO);
        glDeleteBuffers(1, &BillboardEBO);
        delete BillboardShader;
    }

    ~Billboard()
    {
    }

    glm::vec3 getPosition()
    {
        return this->position;
    }
    void setPosition(glm::vec3 position)
    {
        this->position = position;
    }

    float getScale()
    {
        return this->scale;
    }
    void setScale(float scale)
    {
        this->scale = scale;
    }

    float getRotationAngle()
    {
        return this->rotationAngle;
    }
    void setRotationAngle(float rotationAngle)
    {
        this->rotationAngle = rotationAngle;
    }
};
#endif
