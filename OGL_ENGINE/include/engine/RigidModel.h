#pragma once;
#include <RigidBox/RigidBox.h>
#include <engine/model.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class RigidModel
{
private:
    Model model;
    rbRigidBody rigidbox;
    glm::mat4 modelMatrix;

public:
    RigidModel()
    {
    }
    RigidModel(const char *name, const char *path, glm::vec3 position, float mass, float restitution, float friction, glm::vec3 size, int type)
    {
        rigidbox.name = name;
        rigidbox.SetPosition(position.x, position.y, position.z);
        rigidbox.SetShapeParameter(mass,
                                   size.x, size.y, size.z,
                                   restitution, friction);
        rigidbox.EnableAttribute(type);
        model = Model(name, path, position, glm::vec3(0, 0, 0), 0.0f, size.x);
        model.collbox.setScale(glm::vec3(0.5));
        model.collbox.defineCollisionBox();
    }
    void Draw(Shader shader)
    {
        float mtxRT[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        float mtxS[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

        rbVec3 pos = rigidbox.Position();
        rbMtx3 orn = rigidbox.Orientation();
        rbVec3 ext = rigidbox.HalfExtent();

        mtxS[0] = 2.0f * ext.e[0];
        mtxS[5] = 2.0f * ext.e[1];
        mtxS[10] = 2.0f * ext.e[2];

        mtxRT[0] = orn.Elem(0, 0);
        mtxRT[4] = orn.Elem(0, 1);
        mtxRT[8] = orn.Elem(0, 2);
        mtxRT[12] = pos.e[0];
        mtxRT[1] = orn.Elem(1, 0);
        mtxRT[5] = orn.Elem(1, 1);
        mtxRT[9] = orn.Elem(1, 2);
        mtxRT[13] = pos.e[1];
        mtxRT[2] = orn.Elem(2, 0);
        mtxRT[6] = orn.Elem(2, 1);
        mtxRT[10] = orn.Elem(2, 2);
        mtxRT[14] = pos.e[2];
        mtxRT[14] = pos.e[2];

        glm::mat4 rotMat(1);
        glm::mat4 scaleMat(1);
        modelMatrix = glm::mat4(1.0f);
        memcpy(glm::value_ptr(rotMat), mtxRT, sizeof(mtxRT));
        memcpy(glm::value_ptr(scaleMat), mtxS, sizeof(mtxS));

        modelMatrix = glm::translate(modelMatrix, glm::vec3(pos.x, pos.y, pos.z));
        modelMatrix = modelMatrix * rotMat;
        modelMatrix = modelMatrix * scaleMat;
        model.Draw(shader, modelMatrix);
    }
    ~RigidModel()
    {
    }

    Model getModel()
    {
        return this->model;
    }
    void setModel(Model model)
    {
        this->model = model;
    }

    string getName()
    {
        return this->model.name;
    }

    rbRigidBody *getRigidbox()
    {
        return &this->rigidbox;
    }
    void setRigidbox(rbRigidBody rigidbox)
    {
        this->rigidbox = rigidbox;
    }
    glm::mat4 getModelMatrix()
    {
        return this->modelMatrix;
    }
    void setModelMatrix(glm::mat4 modelMatrix)
    {
        this->modelMatrix = modelMatrix;
    }
};
