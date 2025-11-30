#include "Model.hpp"

#include "TextureLoader.hpp"

#include <cstddef>
#include <unordered_map>

Model::~Model() {
    Destroy();
}

bool Model::LoadFromObj(const std::filesystem::path& objPath, std::string* errorMessage) {
    ObjMesh mesh;
    if (!LoadObjMesh(objPath, mesh, errorMessage)) {
        return false;
    }

    if (mesh.vertices.empty() || mesh.indices.empty()) {
        if (errorMessage) {
            *errorMessage = "OBJ file does not contain any drawable geometry.";
        }
        return false;
    }

    Destroy();

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.vertices.size() * sizeof(VertexPNT),
                 mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(uint32_t),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    constexpr GLsizei stride = sizeof(VertexPNT);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(VertexPNT, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(VertexPNT, normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(VertexPNT, texCoord)));

    glBindVertexArray(0);

    draws_.clear();
    textures_.clear();
    std::unordered_map<std::string, GLuint> textureCache;

    auto acquireTexture = [&](const std::filesystem::path& texPath, std::string& lastError) -> GLuint {
        auto canonical = texPath.lexically_normal();
        std::string key = canonical.string();
        auto cacheIt = textureCache.find(key);
        if (cacheIt != textureCache.end()) {
            return cacheIt->second;
        }

        GLuint texture = 0;
        if (!gfx::LoadTexture2D(canonical, texture, &lastError)) {
            return 0;
        }

        textureCache.emplace(key, texture);
        textures_.push_back(texture);
        return texture;
    };

    std::string textureError;
    for (const auto& chunk : mesh.chunks) {
        if (chunk.indexCount == 0) {
            continue;
        }
        MeshDrawCall draw;
        draw.startIndex = chunk.startIndex;
        draw.indexCount = chunk.indexCount;
        draw.diffuseColor = chunk.material.diffuseColor;
        draw.shininess = chunk.material.shininess;

        if (!chunk.material.diffuseTexture.empty()) {
            GLuint tex = acquireTexture(chunk.material.diffuseTexture, textureError);
            if (tex != 0) {
                draw.diffuseTexture = tex;
                draw.hasDiffuse = true;
            } else if (errorMessage && !textureError.empty()) {
                *errorMessage = "Failed to load texture " + chunk.material.diffuseTexture.string() + ": " + textureError;
            }
        }

        draws_.push_back(draw);
    }

    if (draws_.empty()) {
        MeshDrawCall fallback;
        fallback.startIndex = 0;
        fallback.indexCount = static_cast<uint32_t>(mesh.indices.size());
        draws_.push_back(fallback);
    }

    indexCount_ = mesh.indices.size();
    return true;
}

void Model::Draw(const ShaderProgram& shader) const {
    if (vao_ == 0 || indexCount_ == 0) {
        return;
    }

    glBindVertexArray(vao_);
    for (const auto& draw : draws_) {
        shader.SetVec3("uMaterial.diffuseColor", draw.diffuseColor);
        shader.SetFloat("uMaterial.shininess", draw.shininess);
        shader.SetInt("uMaterial.hasDiffuseMap", draw.hasDiffuse ? 1 : 0);
        if (draw.hasDiffuse) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, draw.diffuseTexture);
        }

        const void* offsetPtr = reinterpret_cast<const void*>(static_cast<uintptr_t>(draw.startIndex) * sizeof(uint32_t));
        glDrawElements(GL_TRIANGLES, draw.indexCount, GL_UNSIGNED_INT, offsetPtr);

        if (draw.hasDiffuse) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    glBindVertexArray(0);
}

void Model::Destroy() {
    for (GLuint tex : textures_) {
        glDeleteTextures(1, &tex);
    }
    textures_.clear();
    draws_.clear();
    indexCount_ = 0;

    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
}

