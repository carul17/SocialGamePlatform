#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include "InterpretJson.h"
#include "game.h"

using namespace std;
using json = nlohmann::json;
using namespace ns;


    InterpretJson::InterpretJson(string path){
        ifstream f(path);
        json jData = json::parse(f);
        data = jData;
    }

   
    void InterpretJson::interpret(Game& obj){
        //obj.from_json(data, obj);
        obj = data["configuration"].get<ns::Game>();
    }

