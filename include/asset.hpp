#ifndef ASSET_H
#define ASSET_H

#include <nlohmann/json.hpp>
#include <string>

namespace vke_editor
{
    enum AssetType
    {
        ASSET_TEXTURE,
        ASSET_SHADER,
        ASSET_MATERIAL,
        ASSET_PHYSICS_MATERIAL,
        ASSET_SCENE,
        ASSET_CNT_FLAG
    };

    const std::string AssetTypeToName[] = {"Texture", "Shader", "Material", "PhysicalMaterial", "Scene"};

    typedef int AssetHandle;

    class Asset
    {
    public:
        AssetType type;
        AssetHandle id;
        std::string name;
        std::string path;

        Asset() : type(ASSET_CNT_FLAG), id(0) {}

        Asset(AssetType tp, AssetHandle id, nlohmann::json &json) : type(tp), id(id), name(json["name"]), path(json["path"]) { std::cout << path << "\n"; }
        Asset(AssetType tp, AssetHandle id, std::string &nm, std::string &pth) : type(tp), id(id), name(nm), path(pth) {}

        virtual std::string ToJSON()
        {
            std::string ret = "{\"type\": " + std::to_string(type) + ", ";
            ret += "\"id\": " + std::to_string(id) + ", ";
            ret += "\"name\": \"" + name + "\", ";
            ret += "\"path\": \"" + path + "\"}";
            return ret;
        }

        virtual std::string ToEngineJSON() { return ""; }
    };

#define DEFAULT_CONSTRUCTOR(type, tp) \
    type(AssetHandle id, nlohmann::json &json) : Asset(tp, id, json) {}

#define DEFAULT_CONSTRUCTOR2(type, tp) \
    type(AssetHandle id, std::string &nm, std::string &pth) : Asset(tp, id, nm, pth) {}

#define LEAF_ASSET_TYPE(type, tp)      \
    class type : public Asset          \
    {                                  \
    public:                            \
        DEFAULT_CONSTRUCTOR(type, tp)  \
        DEFAULT_CONSTRUCTOR2(type, tp) \
    };

    LEAF_ASSET_TYPE(TextureAsset, ASSET_TEXTURE)
    LEAF_ASSET_TYPE(ShaderAsset, ASSET_SHADER)
    LEAF_ASSET_TYPE(PhysicsMaterialAsset, ASSET_PHYSICS_MATERIAL)
    LEAF_ASSET_TYPE(SceneAsset, ASSET_SCENE)

    class MaterialAsset : public Asset
    {
    public:
        AssetHandle vertexShader;
        AssetHandle fragmentShader;

        MaterialAsset(AssetHandle id, nlohmann::json &json)
            : vertexShader(json["vertex"]), fragmentShader(json["fragment"]), Asset(ASSET_MATERIAL, id, json) {}

        DEFAULT_CONSTRUCTOR2(MaterialAsset, ASSET_MATERIAL)

        virtual std::string ToJSON()
        {
            std::string ret = "{\"type\": " + std::to_string(type) + ", ";
            ret += "\"id\": " + std::to_string(id) + ", ";
            ret += "\"name\": \"" + name + "\", ";
            ret += "\"path\": \"" + path + "\", \n";
            ret += "\"vertex\": " + std::to_string(vertexShader) + ", ";
            ret += "\"fragment\": " + std::to_string(fragmentShader) + "}";
            return ret;
        }
    };

}

#endif