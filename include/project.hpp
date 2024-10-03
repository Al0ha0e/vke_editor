#ifndef PROJECT_H
#define PRIHECT_H

#include <string>
#include <fstream>
#include <unordered_map>
#include <iostream>
#include <asset.hpp>

namespace vke_editor
{
    class Project
    {
    public:
        std::string projectPath;
        std::unordered_map<AssetHandle, std::shared_ptr<Asset>> assets[ASSET_CNT_FLAG];
        int ids[ASSET_CNT_FLAG];

        Project(std::string &pth) : projectPath(pth)
        {
            for (int i = 0; i < ASSET_CNT_FLAG; i++)
                ids[i] = 1;
        }

        Project(std::string &pth, nlohmann::json &json) : projectPath(pth)
        {
            for (int i = 0; i < ASSET_CNT_FLAG; i++)
                ids[i] = 1;

            auto &as = json["assets"];
            for (auto &asset : as)
            {
                AssetType type = asset["type"];
                AssetHandle id = asset["id"];
                switch (type)
                {
                case ASSET_TEXTURE:
                    assets[type][id] = std::make_shared<TextureAsset>(id, asset);
                    ids[type] = std::max(ids[type], id + 1);
                    break;
                case ASSET_SHADER:
                    assets[type][id] = std::make_shared<ShaderAsset>(id, asset);
                    ids[type] = std::max(ids[type], id + 1);
                    break;
                case ASSET_MATERIAL:
                    assets[type][id] = std::make_shared<MaterialAsset>(id, asset);
                    ids[type] = std::max(ids[type], id + 1);
                    break;
                case ASSET_PHYSICS_MATERIAL:
                    assets[type][id] = std::make_shared<PhysicsMaterialAsset>(id, asset);
                    ids[type] = std::max(ids[type], id + 1);
                    break;
                case ASSET_SCENE:
                    assets[type][id] = std::make_shared<SceneAsset>(id, asset);
                    ids[type] = std::max(ids[type], id + 1);
                default:
                    break;
                }
            }
        }

        std::string ToJSON()
        {
            std::string ret = "{\n\"assets\": [ ";
            for (int i = 0; i < ASSET_CNT_FLAG; i++)
                for (auto &kv : assets[i])
                    ret += "\n" + kv.second->ToJSON() + ",";
            ret[ret.length() - 1] = ']';
            ret += "\n}";

            return ret;
        }

        void AddAsset(std::shared_ptr<Asset> asset)
        {
            AssetHandle id = ids[asset->type]++;
            asset->id = id;
            assets[asset->type][id] = asset;
        }
    };
}

#endif