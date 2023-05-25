#include <map>
#include <string>
#include <vector>
#pragma once;
using namespace std;
class Objectives
{
private:
    map<string, bool> objectives;

public:
    Objectives()
    {
    }
    ~Objectives()
    {
    }
    void AddObjective(string name, bool state)
    {
        objectives.insert(pair<string, bool>(name, state));
    }
    void removeObjective(string name)
    {
        objectives.erase(name);
    }
    bool getValue(string name)
    {
        return objectives.find(name)->second;
    }
    void setValue(string name, bool state)
    {
        objectives.find(name)->second = state;
    }
};
