
#ifndef COLLISIONBOX_H
#define COLLISIONBOX_H

#include <GLFW/glfw3.h>
#include <RigidBox/RigidBox.h>
#include <engine/shader_m.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class CollisionBox
{
private:
    /* data */
    unsigned int VBO, VAO;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 angles;
    glm::vec3 rotationAxis;
    float angle;
    rbRigidBody rigidbox;
    bool withBox;

public:
    glm::vec3 min;
    glm::vec3 max;
    Shader *shader;
    CollisionBox()
    {
    }
    CollisionBox(glm::vec3 position, glm::vec3 scale, glm::vec4 color, bool withRigidBox = true, bool withBox = true)
    {
        shader = new Shader("shaders/basic.vs", "shaders/basic.fs");
        this->position = position;
        this->scale = scale;
        angles = glm::vec3(0.0f);
        defineCollisionBox();
        if (withRigidBox)
        {
            rigidbox.SetPosition(position.x, position.y, position.z);
            rigidbox.SetShapeParameter(10.0,
                                       scale.x, scale.y, scale.z,
                                       0.0, 0.5);
            rigidbox.EnableAttribute(rbRigidBody::Attribute_Fixed);
        }
        this->withBox = withBox;
        if (withBox)
        {
            float vertices[] = {
                -0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, 0.5f, -0.5f,
                0.5f, 0.5f, -0.5f,
                -0.5f, 0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,

                -0.5f, -0.5f, 0.5f,
                0.5f, -0.5f, 0.5f,
                0.5f, 0.5f, 0.5f,
                0.5f, 0.5f, 0.5f,
                -0.5f, 0.5f, 0.5f,
                -0.5f, -0.5f, 0.5f,

                -0.5f, 0.5f, 0.5f,
                -0.5f, 0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, 0.5f,
                -0.5f, 0.5f, 0.5f,

                0.5f, 0.5f, 0.5f,
                0.5f, 0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, 0.5f,
                0.5f, 0.5f, 0.5f,

                -0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, 0.5f,
                0.5f, -0.5f, 0.5f,
                -0.5f, -0.5f, 0.5f,
                -0.5f, -0.5f, -0.5f,

                -0.5f, 0.5f, -0.5f,
                0.5f, 0.5f, -0.5f,
                0.5f, 0.5f, 0.5f,
                0.5f, 0.5f, 0.5f,
                -0.5f, 0.5f, 0.5f,
                -0.5f, 0.5f, -0.5f};

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);

            shader->use();
            shader->setVec4("color", color);
        }
    }

    ~CollisionBox()
    {
    }
    void draw(glm::mat4 view, glm::mat4 projection)
    {
        if (withBox)
        {
            shader->use();
            glLineWidth(3.0);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position); // translate it down so it's at the center of the scene
            model = glm::rotate(model, glm::radians(angles.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(angles.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(angles.z), glm::vec3(0, 0, 1));
            model = glm::scale(model, scale);

            shader->setMat4("model", model);
            shader->setMat4("view", view);
            shader->setMat4("projection", projection);
            glBindVertexArray(VAO);
            glDrawArrays(GL_LINE_STRIP, 0, 36);
        }        
    }

    void draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
    {
        if (withBox)
        {
            shader->use();
            glLineWidth(3.0);
            shader->setMat4("model", model);
            shader->setMat4("view", view);
            shader->setMat4("projection", projection);
            glBindVertexArray(VAO);
            glDrawArrays(GL_LINE_STRIP, 0, 36);
        }
    }

    float getAngle()
    {
        return this->angle;
    }
    void setAngle(float angle)
    {
        this->angle = angle;
    }

    glm::vec3 getAngles()
    {
        return this->angles;
    }
    void setAngles(glm::vec3 angles)
    {
        this->angles = angles;
    }

    void defineCollisionBox()
    {
        max = glm::vec3(position.x + scale.x, position.y + scale.y, position.z + scale.z);
        min = glm::vec3(position.x - scale.x, position.y - scale.y, position.z - scale.z);
    }
    
    glm::vec3 getPosition()
    {
        return this->position;
    }
    void setPosition(glm::vec3 position)
    {
        this->position = position;
    }

    glm::vec3 getScale()
    {
        return this->scale;
    }
    void setScale(glm::vec3 scale)
    {
        this->scale = scale;
    }

    void setRotation(glm::vec3 rotationAxis)
    {
        this->rotationAxis = rotationAxis;
    }
    glm::vec3 getRotation()
    {
        return this->rotationAxis;
    }
    rbRigidBody *getRigidbox()
    {
        return &this->rigidbox;
    }
    void setRigidbox(rbRigidBody rigidbox)
    {
        this->rigidbox = rigidbox;
    }

    void Release()
    {
        delete shader;
    }
};
#endif // !COLLISIONBOX_H