#ifndef SKYBOX_H
#define SKYBOX_H

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>

using std::cerr;
using std::endl;
using std::string;
using std::vector;

class SkyBox
{
    Shader *shader;

    unsigned int VAO, VBO, EBO;
    unsigned int cubemapTexture;
    unsigned int loadCubemap(vector<string> faces, const string sky)
    {
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

        string path = string("textures/skybox/") + sky + string("/");
        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            string file = path + faces[i];
            cerr << file << endl;

            unsigned char *data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                // glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

                stbi_image_free(data);
                // glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
            else
            {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return texture;
    }

public:
  
    SkyBox(const float size = 1, const string skybox = "1")
    {
        shader = new Shader("shaders/skyBox.vs", "shaders/skyBox.fs");
        GLfloat sky_vertices[] = {
            size, size, size,
            size, -size, size,
            size, size, -size,
            size, -size, -size,
            -size, -size, -size,
            -size, size, -size,
            -size, -size, size,
            -size, size, size};

        unsigned int sky_indices[] = {
            0, 1, 3,
            3, 2, 0,
            0, 1, 7,
            7, 6, 1,
            1, 3, 6,
            6, 4, 3,
            3, 2, 4,
            4, 2, 5,
            5, 4, 6,
            6, 5, 7,
            7, 5, 2,
            2, 0, 7};
        vector<string> faces{
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "back.jpg",
            "front.jpg"};
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        cubemapTexture = loadCubemap(faces, skybox);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sky_vertices), sky_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sky_indices), sky_indices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    };
    SkyBox()
    {

    }
    ~SkyBox()
    {

    }
    void reloadTexture(const string sky) //NOMBRE DE LA CARPETA
    {
        vector<string> faces{
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "back.jpg",
            "front.jpg"};

        string path = string("textures/skybox/") + sky + string("/");
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            string file = path + faces[i];
            cerr << file << endl;

            unsigned char *data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                // glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

                stbi_image_free(data);
                // glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
            else
            {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    void Release()
    {
        delete shader;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
    void draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection, glm::vec3 camPos)
    {
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", model);
        shader->setVec3("campos", camPos);

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubemapTexture);
        glDrawElements(GL_TRIANGLE_STRIP, 36, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);
        //shader->notUse();
    }
    Shader *getShader()
    {
        return this->shader;
    }
    void setShader(Shader *shader)
    {
        this->shader = shader;
    }
};

#endif