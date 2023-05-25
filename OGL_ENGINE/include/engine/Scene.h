#pragma once;
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;
const enum TypeObjective { COLLISION,
                           EXIST,   //DETERMINA SI UN OBJETO EXISTE SEGÚN EL NOMBRE DE UN OBJETO
                           COUNT,   //CUENTA UNA CANTIDAD EXACTA DE OBJETOS EN UN ARREGLO
                           GREATER, //DETERMINA SI UNA CANTIDAD DE OBJETOS ES MAYOR QUE UN NÚMERO
                           SMALLER  //DETERMINA SI UNA CANTIDAD DE OBJETOS ES MENOR QUE UN NÚMERO
};

class Scene
{
private:
public:
    map<string, pair<TypeObjective, bool>> objectives;
    Scene()
    {
    }
    ~Scene()
    {
    }

    void AddObjective(string name, TypeObjective type, bool state)
    {
        objectives.insert(pair<string, pair<TypeObjective, bool>>(name, pair<TypeObjective, bool>(type, state)));
    }
    void RemoveObjective(string name)
    {
        objectives.erase(name);
    }
    bool CheckBy(string name)
    {
        objectives.erase(name);
    }
};
