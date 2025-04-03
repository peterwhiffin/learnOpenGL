#ifndef MODEL_H
#define MODEL_H
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "assimp/material.h"
#include "assimp/types.h"
#include "camera.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "stb_image.h"

unsigned int TextureFromFile(const char *path, const std::string &directory,
                             bool gamma = false);

class Model
{
public:
  std::vector<Texture> textures_loaded;
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(3.0f, 3.0f, 3.0f);

  Model(char *path, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
  {
    this->position = position;
    loadModel(path);
  }

  void Draw(const Shader *shader)
  {
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
    trans *= glm::mat4_cast(rotation);
    trans = glm::scale(trans, scale);

    shader->setMat4("normalMat", glm::transpose(glm::inverse(glm::mat3(trans))));
    shader->setMat4("model", trans);

    for (unsigned int i = 0; i < meshes.size(); i++)
      meshes[i].Draw(shader);
  }

  void rotate(float degrees, glm::vec3 axis)
  {
    rotation *= glm::angleAxis(glm::radians(degrees), axis);
  }

private:
  // model data
  std::vector<Mesh> meshes;
  std::string directory;

  void loadModel(std::string path)
  {
    Assimp::Importer importer;
    // const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate |
                                                       aiProcess_FlipUVs |
                                                       aiProcess_JoinIdenticalVertices |
                                                       aiProcess_OptimizeMeshes |
                                                       aiProcess_OptimizeGraph |
                                                       aiProcess_ImproveCacheLocality);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
      std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
      return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene, glm::mat4(1.0f));
  }

  void processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform)
  {
    // process all the node's meshes (if any)
    // Convert Assimp transformation matrix to GLM
    glm::mat4 nodeTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    // Accumulate transformation
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(processMesh(mesh, scene, globalTransform));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      processNode(node->mChildren[i], scene, globalTransform);
    }
  }

  Mesh processMesh(aiMesh *mesh, const aiScene *scene, const glm::mat4 &transform)
  {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      Vertex vertex;
      // process vertex positions, normals and texture coordinates
      // glm::vec3 vector;
      // vector.x = mesh->mVertices[i].x;
      // vector.y = mesh->mVertices[i].y;
      // vector.z = mesh->mVertices[i].z;
      // vertex.Position = vector;

      // vector.x = mesh->mNormals[i].x;
      // vector.y = mesh->mNormals[i].y;
      // vector.z = mesh->mNormals[i].z;
      // vertex.Normal = vector;
      glm::vec4 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
      vertex.Position = glm::vec3(transform * position);

      glm::vec4 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f);
      vertex.Normal = glm::normalize(glm::vec3(transform * normal));

      vertex.TexCoords = glm::vec2(0.0f, 0.0f);

      if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
      {
        glm::vec2 vec;
        vec.x = mesh->mTextureCoords[0][i].x;
        vec.y = mesh->mTextureCoords[0][i].y;
        vertex.TexCoords = vec;
      }

      vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
      aiFace face = mesh->mFaces[i];

      for (unsigned int j = 0; j < face.mNumIndices; j++)
        indices.push_back(face.mIndices[j]);
    }

    aiColor3D baseColor(1.0f, 1.0f, 1.0f);
    // process material
    std::string name;
    if (mesh->mMaterialIndex >= 0)
    {
      aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
      std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", true);
      material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
      textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
      std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
      textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
      name = material->GetName().C_Str();
    }

    return Mesh(vertices, indices, textures, baseColor, name);
  }

  std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, bool gamma = false)
  {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
      aiString str;
      mat->GetTexture(type, i, &str);
      bool skip = false;

      for (unsigned int j = 0; j < textures_loaded.size(); j++)
      {
        if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
        {
          textures.push_back(textures_loaded[j]);
          skip = true;
          break;
        }
      }

      if (!skip)
      { // if texture hasn't been loaded already, load it
        Texture texture;
        texture.id = TextureFromFile(str.C_Str(), directory, gamma);
        texture.type = typeName;
        texture.path = str.C_Str();
        textures.push_back(texture);
        textures_loaded.push_back(texture); // add to loaded textures
      }
    }

    return textures;
  }
};

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
  std::string filename = std::string(path);
  filename = directory + '/' + filename;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

  if (data)
  {
    GLenum format;
    GLenum internalFormat;
    if (nrComponents == 1)
    {
      format = GL_RED;
      internalFormat = GL_RED;
    }
    else if (nrComponents == 3)
    {
      format = GL_RGB;
      internalFormat = gamma ? GL_SRGB : GL_RGB;
    }
    else if (nrComponents == 4)
    {
      format = GL_RGBA;
      internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}
#endif
