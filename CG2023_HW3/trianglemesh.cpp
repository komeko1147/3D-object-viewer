#include "trianglemesh.h"

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	// -------------------------------------------------------
	// Add your initialization code here.
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	vboId = 0;
	objExtent = glm::vec3(0.0f, 0.0f, 0.0f);
	// -------------------------------------------------------
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	// -------------------------------------------------------
	// Add your release code here.
	subMeshes.clear();
	pm.clear();
	// -------------------------------------------------------
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{	
	// Parse the OBJ file.
	// ---------------------------------------------------------------------------
    // Add your implementation here (HW1 + read *.MTL).
	std::ifstream ifs(filePath);
	std::string line;
	std::cout << "open the file\n";
	if (!ifs.is_open()) {
		std::cout << "failed to open the object file\n";
		exit(-1);
	}
	std::cout << "succeed\n";

	float maxx = -FLT_MAX, maxy = -FLT_MAX, maxz = -FLT_MAX;
	float minx = FLT_MAX, miny = FLT_MAX, minz = FLT_MAX;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::unordered_map<VertexPTN, int, hashVertex> record;
	int next = 0, last = -1;  //will be used when dataType == 'f'
	SubMesh* ts = new SubMesh;

	while (std::getline(ifs, line)) {
		std::stringstream iss(line);
		std::string dataType;
		iss >> dataType;
		//std::cout << line << std::endl;
		///// generate material of vertexes
		if (dataType == "mtllib") {
			std::string mtlName;
			iss >> mtlName;
			size_t part = filePath.rfind("\\");
			mtlName = filePath.substr(0, part + 1) + mtlName;
			if (!buildMtllib(mtlName)) {
				std::cout << "failed to open the material file\n";
				exit(-1);
			}
		}

		///// generate coordinate data of vertexes
		if (dataType == "v") { //position
			float xp, yp, zp;
			iss >> xp >> yp >> zp;

			// find two corners of bounding box
			maxx = std::max(maxx, xp), maxy = std::max(maxy, yp), maxz = std::max(maxz, zp);
			minx = std::min(minx, xp), miny = std::min(miny, yp), minz = std::min(minz, zp);
			//std::cout << xp << " " << yp << " " << zp << std::endl;
			positions.push_back({ xp, yp, zp });
		}
		else if (dataType == "vt") { //texture
			float xt, yt;
			iss >> xt >> yt;
			//std::cout << xt << " " << yt << " " << std::endl;
			uvs.push_back({ xt, yt });
		}
		else if (dataType == "vn") { //normal
			float xn, yn, zn;
			iss >> xn >> yn >> zn;
			//std::cout << xn << " " << yn << " " << zn << std::endl;
			normals.push_back({ xn, yn, zn });
		}
		else if (dataType == "usemtl") {
			std::string name;
			iss >> name;
			bool duplicate = false;

			for (SubMesh& sm : subMeshes)
			{
				if (sm.material->GetName() == name)
				{
					duplicate = 1;
					ts = &sm;
					break;
				}
			}
			if (!duplicate)
			{
				// Initialize

				subMeshes.push_back(SubMesh());
				ts = &subMeshes[subMeshes.size() - 1];
				for (PhongMaterial& material : pm)
				{
					if (material.GetName() == name)
					{
						ts->material = &material;
						break;
					}
				}
			}
		}
		else if (dataType == "f") {  //face
			std::string vertexInfo;
			int TriangleVertex_1st = -1;

			int cnt = 0;
			while (iss >> vertexInfo) {
				VertexPTN temp;
				cnt++;
				int head = 0, offset = 0, id = 0;

				///// make VertexPTN 
				// get position index
				while (vertexInfo[head + offset] != '/') offset++;
				std::string info = vertexInfo.substr(head, offset);
				std::istringstream ss1(info);
				//std::cout << info << "/";
				ss1 >> id;
				temp.position = positions[id - 1];
				head += offset + 1, offset = 0;
				// get texture index
				while (vertexInfo[head + offset] != '/') offset++;
				info = vertexInfo.substr(head, offset);
				std::istringstream ss2(info);
				//std::cout << info << "/";
				ss2 >> id;
				//id2 >> index;
				temp.texcoord = uvs[id - 1];
				head += offset + 1;
				// get normal index
				info = vertexInfo.substr(head, vertexInfo.size() - head);
				std::istringstream ss3(info);
				//std::cout << info <<" ";
				ss3 >> id;
				temp.normal = normals[id - 1];

				if (cnt == 1) {
					if (record.find(temp) != record.end()) {
						TriangleVertex_1st = record[temp];
						ts->vertexIndices.push_back(record[temp]);
					}
					else {
						record[temp] = next;
						TriangleVertex_1st = record[temp];
						vertices.push_back(temp);
						numVertices++;
						ts->vertexIndices.push_back(next);
						last = next;
						next++;
					}
				}
				else {
					if (cnt > 3) { // polygon subvision
						ts->vertexIndices.push_back(TriangleVertex_1st);
						ts->vertexIndices.push_back(last);
						//std::cout << next << " ";
						numTriangles++;
					}
					if (record.find(temp) != record.end()) {
						ts->vertexIndices.push_back(record[temp]);
						last = record[temp];
					}
					else {
						record[temp] = next;
						vertices.push_back(temp);
						numVertices++;
						ts->vertexIndices.push_back(next);
						last = next;
						next++;
					}
				}

			}
			numTriangles++;
		}
	}
    // ---------------------------------------------------------------------------

	// Normalize the geometry data.
	objExtent = { maxx - minx, maxy - miny , maxz - minz };
	objCenter = { (maxx + minx) / 2.0f, (maxy + miny) / 2.0f, (maxz + minz) / 2.0f };
	if (normalized) {
		// -----------------------------------------------------------------------
		// Add your normalization code here (HW1).
		float maximal_extent_axis = std::max(objExtent.x, std::max(objExtent.y, objExtent.z));
		for (int i = 0; i < vertices.size(); i++) {
			vertices[i].position[0] = (vertices[i].position[0] - objCenter[0]) / maximal_extent_axis;
			vertices[i].position[1] = (vertices[i].position[1] - objCenter[1]) / maximal_extent_axis;
			vertices[i].position[2] = (vertices[i].position[2] - objCenter[2]) / maximal_extent_axis;
		}
		// -----------------------------------------------------------------------
	}
	ifs.close();
	CreateBuffers();
	return true;
}

void TriangleMesh::CreateBuffers()
{
	// Add your code here.
	// Generate the vertex buffer.
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexPTN), &vertices[0], GL_STATIC_DRAW);

	// Generate the index buffer.
	for (SubMesh& SM : subMeshes) {
		glGenBuffers(1, &SM.iboId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SM.iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * SM.vertexIndices.size(), &(SM.vertexIndices[0]), GL_STATIC_DRAW);
	}
}	

bool TriangleMesh::buildMtllib(const std::string& mtlpath) {
	std::cout << mtlpath << std::endl;
	std::ifstream m(mtlpath);
	if (!m.is_open()) return false;

	PhongMaterial temp;
	std::string line;
	while (getline(m, line)) {
		std::stringstream s(line);
		std::string info;
		s >> info;
		if (info == "newmtl") {
			s >> info;
			if (temp.GetName() != "Default") {
				pm.push_back(temp);
			}
			temp.SetName(info);
		}
		else if (info == "Ns") {
			float f;
			s >> f;
			temp.SetNs(f);
		}
		else if (info == "Ka") {
			glm::vec3 coord;
			s >> coord.x;
			s >> coord.y;
			s >> coord.z;
			temp.SetKa(coord);
		}
		else if (info == "Kd") {
			glm::vec3 coord;
			s >> coord.x;
			s >> coord.y;
			s >> coord.z;
			temp.SetKd(coord);
		}
		else if (info == "Ks") {
			glm::vec3 coord;
			s >> coord.x;
			s >> coord.y;
			s >> coord.z;
			temp.SetKs(coord);
		}
		else if (info == "map_Kd") {
			std::string imageName;
			s >> imageName;
			size_t part = mtlpath.rfind("\\");
			std::string rp = mtlpath.substr(0, part + 1) + imageName;
			ImageTexture *im = new ImageTexture(rp);
			temp.SetMapKd(im);
		}
	}
	pm.push_back(temp);
	m.close();
	return true;
}


// Show model information.
void TriangleMesh::ShowInfo()
{
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Total " << subMeshes.size() << " subMeshes loaded" << std::endl;
	for (unsigned int i = 0; i < subMeshes.size(); ++i) {
		const SubMesh& g = subMeshes[i];
		std::cout << "SubMesh " << i << " with material: " << g.material->GetName() << std::endl;
		std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3 << std::endl;
	}
	std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", " << objCenter.z << std::endl;
	std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x " << objExtent.z << std::endl;
}

