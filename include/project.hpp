#ifndef PROJECT_H
#define PRIHECT_H

#include <string>
#include <fstream>
#include <unordered_map>
#include <iostream>

namespace vke_editor
{
    class Project
    {
    public:
        std::string projectPath;
        std::string assetLUTPath;

        Project(std::string &pth) : projectPath(pth), assetLUTPath(pth + "/asset_lut.json") {}

        Project(std::string &pth, nlohmann::json &json) : projectPath(pth), assetLUTPath(json["asset_lut_path"]) {}

        std::string ToJSON() { return "{\"asset_lut_path\":\"" + assetLUTPath + "\"}"; }
    };
}

#endif