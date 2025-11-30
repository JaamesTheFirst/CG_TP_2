#pragma once

#include "ObjLoader.hpp"
#include "ShaderProgram.hpp"

#include <vector>

struct MeshDrawCall {
    uint32_t startIndex = 0;
    uint32_t indexCount = 0;
    glm::vec3 diffuseColor{0.8f};
    float shininess = 32.0f;
    GLuint diffuseTexture = 0;
    bool hasDiffuse = false;
};

class Model {
public:
    Model() = default;
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    bool LoadFromObj(const std::filesystem::path& objPath, std::string* errorMessage = nullptr);
    void Draw(const ShaderProgram& shader) const;
    void Destroy();

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    std::vector<MeshDrawCall> draws_;
    std::vector<GLuint> textures_;
    std::size_t indexCount_ = 0;
};

