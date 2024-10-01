#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "headers.h"
#include "material.h"

// VertexPTN Declarations.
struct VertexPTN
{
	VertexPTN() {
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		normal = glm::vec3(0.0f, 1.0f, 0.0f);
		texcoord = glm::vec2(0.0f, 0.0f);
	}
	VertexPTN(glm::vec3 p, glm::vec3 n, glm::vec2 uv) {
		position = p;
		normal = n;
		texcoord = uv;
	}
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
	bool operator==(const VertexPTN& v2) const {
		return (position == v2.position) && (normal == v2.normal) && (texcoord == v2.texcoord);
	}
};

// SubMesh Declarations.
struct SubMesh
{
	SubMesh() {
		material = nullptr;
		iboId = 0;
	}
	PhongMaterial* material;
	GLuint iboId;
	std::vector<unsigned int> vertexIndices;
};


// TriangleMesh Declarations.
class TriangleMesh
{
public:
	// TriangleMesh Public Methods.
	TriangleMesh();
	~TriangleMesh();
	
	// Load the model from an *.OBJ file.
	bool LoadFromFile(const std::string& filePath, const bool normalized = true);
	
	// Show model information.
	void ShowInfo();

	// -------------------------------------------------------
	// Feel free to add your methods or data here.
	void CreateBuffers();
	GLuint Get_vbo() const { return vboId; }
	// -------------------------------------------------------

	int GetNumVertices() const { return numVertices; }
	int GetNumTriangles() const { return numTriangles; }
	int GetNumSubMeshes() const { return (int)subMeshes.size(); }

	glm::vec3 GetObjCenter() const { return objCenter; }
	glm::vec3 GetObjExtent() const { return objExtent; }
	std::vector<SubMesh>& GetsubMeshes() { return subMeshes; }

private:
	// -------------------------------------------------------
	// Feel free to add your methods or data here.
	bool buildMtllib(const std::string& mtlpath);
	// -------------------------------------------------------

	// TriangleMesh Private Data.
	GLuint vboId;
	
	std::vector<VertexPTN> vertices;
	// For supporting multiple materials per object, move to SubMesh.
	// GLuint iboId;
	// std::vector<unsigned int> vertexIndices;
	std::vector<SubMesh> subMeshes;
	std::vector<PhongMaterial> pm;
	int numVertices;
	int numTriangles;
	glm::vec3 objCenter;
	glm::vec3 objExtent;
};

//write a special hash function to hash VertexPTN in unordered_map; 
struct hashVertex
{
	size_t operator()(const VertexPTN& v) const {
		return std::hash<float>() (v.position[0]) ^ std::hash<float>() (v.position[1]) ^ std::hash<float>() (v.position[2])
			^ std::hash<float>() (v.normal[0]) ^ std::hash<float>() (v.normal[1]) ^ std::hash<float>() (v.normal[2])
			^ std::hash<float>() (v.texcoord[0]) ^ std::hash<float>() (v.texcoord[1]);
	}
};

#endif
