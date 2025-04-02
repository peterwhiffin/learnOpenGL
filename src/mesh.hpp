
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "assimp/types.h"
#include "glm/ext/vector_float3.hpp"
#include "shader.hpp"

struct Vertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

struct Texture
{
  unsigned int id;
  std::string type;
  std::string path;
};

class Mesh
{
public:
  // mesh data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  glm::vec3 baseColor;

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, aiColor3D baseColor)
  {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->baseColor.x = baseColor.r;
    this->baseColor.y = baseColor.g;
    this->baseColor.z = baseColor.b;

    // if (textures.size() == 0) {
    //   Texture defaultTex;
    //   defaultTex.id = 1;
    //   defaultTex.type = "texture_diffuse";
    //   this->textures.push_back(defaultTex);
    //   Texture defaultSpec;
    //   defaultSpec.id = 1;
    //   defaultSpec.type = "texture_specular";
    //   this->textures.push_back(defaultSpec);
    // }
    //
    setupMesh();
  }

  void Draw(const Shader *shader)
  {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    bool hasSpecular = false;

    for (unsigned int i = 0; i < textures.size(); i++)
    {
      glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
                                        // retrieve texture number (the N in diffuse_textureN)
      std::string number;
      std::string name = textures[i].type;

      if (name == "texture_diffuse")
        number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
      {
        number = std::to_string(specularNr++);
        hasSpecular = true;
      }

      shader->setInt(("material." + name + number).c_str(), i);
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    if (!hasSpecular)
    {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, 1);
    }

    // shader->setVec3("baseColor", baseColor);
    glActiveTexture(GL_TEXTURE0);
    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

private:
  //  render data
  unsigned int VAO, VBO, EBO;

  void setupMesh()
  {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
  }
};
