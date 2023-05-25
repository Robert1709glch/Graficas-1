#ifndef OBJECTIVES_H
#define OBJECTIVES_H

#include <RigidBox/RigidBox.h>
#include <engine/CollisionBox.h>
#include <engine/utils.h>
#include <iostream>
#include <string>
using namespace std;
enum TypeObjectives
{
    COLLISION,
    EXIST,   //DETERMINA SI UN OBJETO EXISTE SEGÚN EL NOMBRE DE UN OBJETO
    COUNT,   //CUENTA UNA CANTIDAD EXACTA DE OBJETOS EN UN ARREGLO
    GREATER, //DETERMINA SI UNA CANTIDAD DE OBJETOS ES MAYOR QUE UN NÚMERO
    SMALLER  //DETERMINA SI UNA CANTIDAD DE OBJETOS ES MENOR QUE UN NÚMERO
};

template <typename T>
class Objectives
{
private:
    TypeObjectives type;
    bool state = false;

public:
    Objectives()
    {
    }
    ~Objectives()
    {
    }

    bool checkByCollision(CollisionBox *a, CollisionBox *b)
    {
        state = intersect(*a, *b);
        return state;
    }
    void checkByGreater(vector<T> array, int count)
    {
        state = array.size() > count;
        return state;
    }
    void checkBySmaller(vector<T> array, int count)
    {
        state = array.size() < count;
        return state;
    }
    void checkBySmaller(vector<T> array, int count)
    {
        state = array.size() == count;
        return state;
    }

    void checkByExist(vector<T> array, int name)
    {

        auto it = find_if(array.begin(), array.end(), [&name](const T &obj)
                          { return obj.name == name; });
        state = it != v.end();
        return state;
    }

    TypeObjectives getType()
    {
        return this->type;
    }
    void setType(TypeObjectives type)
    {
        this->type = type;
    }

    bool getState()
    {
        return this->state;
    }
    void setState(bool state)
    {
        this->state = state;
    }

    string getTag()
    {
        return this->tag;
    }
    void setTag(string tag)
    {
        this->tag = tag;
    }
};
#endif // !OBJECTIVES_H