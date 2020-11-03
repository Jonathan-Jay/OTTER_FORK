#include "ObjLoader.h"

#include <string>
#include <sstream>

// Borrowed from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// Thank you Shawn
#pragma region String Trimming

// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

#pragma endregion 

Shader::sptr ObjLoader::m_shader = nullptr;
Shader::sptr ObjLoader::m_matShader = nullptr;


struct Materials
{
	std::string name;
	glm::vec3 colours;
	glm::vec2 specStrength;
};

ObjLoader::ObjLoader(const std::string& fileName, bool usingMaterial)
{
	LoadMesh(fileName, usingMaterial);
}

ObjLoader::~ObjLoader()
{
	m_vao.resize(0);
	m_numberOfVertices.resize(0);
}

void ObjLoader::Init()
{
	m_matShader = Shader::Create();
	m_matShader->LoadShaderPartFromFile("shaders/mat_vertex_shader.glsl", GL_VERTEX_SHADER);
	m_matShader->LoadShaderPartFromFile("shaders/mat_frag_shader.glsl", GL_FRAGMENT_SHADER);
	m_matShader->Link();

	m_shader = Shader::Create();
	m_shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
	m_shader->LoadShaderPartFromFile("shaders/frag_shader.glsl", GL_FRAGMENT_SHADER);
	m_shader->Link();
}

void ObjLoader::LoadMesh(const std::string& fileName, bool usingMaterial)
{
	m_vao.resize(0);
	m_numberOfVertices.resize(0);
	std::ifstream file;
	file.open(fileName, std::ios::binary);

	if (!file)
	{
		throw std::runtime_error("Failed to open file\nError 69: " + fileName);
	}

	std::vector<Materials> materials;

	if (usingMaterial)
	{
		std::ifstream materialFile;
		std::string materialFileName = fileName.substr(0, fileName.size() - 3) + "mtl";
		materialFile.open(materialFileName, std::ios::binary);
		if (!materialFile)
		{
			throw std::runtime_error("Failed to open material file, maybe it doesn't have one\nError 420: " + materialFileName);
		}

		std::string matLine;
		size_t matIndex = 0;
		float tempExponent = 1.f;
		while (std::getline(materialFile, matLine))
		{
			ltrim(matLine);
			if (matLine.substr(0, 6) == "newmtl")
			{
				materials.push_back({ matLine.substr(7), glm::vec3(1.f), glm::vec2(1.f) });
				matIndex = materials.size() - 1;
			}
			else if (matLine.substr(0, 2) == "Ns")
			{
				std::istringstream ss = std::istringstream(matLine.substr(2));
				ss >> tempExponent;
			}
			else if (matLine.size() > 1 && matLine[0] == 'K')
			{
				//only colour data is taken for now, textures later
				if (matLine[1] == 'd')
				{
					//diffuse Colour, aka object colour
					std::istringstream ss = std::istringstream(matLine.substr(2));
					glm::vec3 colour;
					ss >> colour.x >> colour.y >> colour.z;

					materials[matIndex].colours = colour;
				}
				else if (matLine[1] == 's')
				{
					//specular Colour
					//diffuse Colour, aka object colour
					std::istringstream ss = std::istringstream(matLine.substr(2));
					glm::vec3 colour;
					ss >> colour.x >> colour.y >> colour.z;

					materials[matIndex].specStrength = glm::vec2(colour.length(), tempExponent);
				}
				else if (matLine[1] == 'a')
				{
					//ambient Colour, ignored
				}
			}
			else if (matLine[0] == '#')
			{
				//it's a comment lol i cant even, tho maybe i can
			}
		}
		materialFile.close();

		m_usingMaterial = true;
	}
	else
	{
		m_usingMaterial = false;
	}


	std::vector<glm::vec3> vertex = { glm::vec3() };
	std::vector<glm::vec2> UV = { glm::vec3() };
	std::vector<glm::vec3> normals = { glm::vec3() };

	std::vector<size_t> bufferVertex;
	std::vector<size_t> bufferUV;
	std::vector<size_t> bufferNormals;

	std::vector<size_t> bufferColours;	//index of material vector

	// stringStream was learnt throught the NotObjLoader, thank you Shawn once again
	std::string line;
	glm::size_t currentColour = 0;
	size_t vectorIndex = 0;
	while (std::getline(file, line))
	{
		trim(line);

		if (line[0] == 'v')
		{
			if (line[1] == ' ')
			{
				//Vertices			v xvalue yvalue zvalue
				std::istringstream ss = std::istringstream(line.substr(1));
				glm::vec3 pos;
				ss >> pos.x >> pos.y >> pos.z;

				vertex.push_back(pos);
			}
			else if (line[1] == 't')
			{
				//UVs, we do care rn			vt xvalue yvalue
				std::istringstream ss = std::istringstream(line.substr(2));
				glm::vec2 uv;
				ss >> uv.x >> uv.y;

				UV.push_back(uv);
			}
			else if (line[1] == 'n')
			{
				//Normals
				std::istringstream ss = std::istringstream(line.substr(2));
				//is Kinda cringe
				glm::vec3 n;
				ss >> n.x >> n.y >> n.z;

				normals.push_back(n);
			}

		}
		else if (line[0] == 'f')
		{
			//faces
			//indices, seperated by /
			size_t offset = 2;

			//cause 3 vertices times 3 values
			for (int i = 0; i < 9; ++i)
			{
				size_t lengthIndex = 0;
				for (; isalnum(line[offset + lengthIndex]); ++lengthIndex);
				size_t index = stoi(line.substr(offset, offset + lengthIndex));

				offset += 1 + lengthIndex;

				if (i % 3 == 0)
				{
					//it's the vertex index
					bufferVertex.push_back(index);
					if (usingMaterial)
						bufferColours.push_back(currentColour);
				}
				else if (i % 3 == 1)
				{
					//it's the UV index, we do care
					bufferUV.push_back(index);
				}
				else if (i % 3 == 2)
				{
					//it's the normals index
					bufferNormals.push_back(index);
				}
			}

			if (offset < line.size())
			{

				bufferVertex.push_back(bufferVertex[bufferVertex.size() - 3]);
				bufferVertex.push_back(bufferVertex[bufferVertex.size() - 2]);
				bufferUV.push_back(bufferUV[bufferUV.size() - 3]);
				bufferUV.push_back(bufferUV[bufferUV.size() - 2]);
				bufferNormals.push_back(bufferNormals[bufferNormals.size() - 3]);
				bufferNormals.push_back(bufferNormals[bufferNormals.size() - 2]);
				if (usingMaterial) {
					bufferColours.push_back(currentColour);
					bufferColours.push_back(currentColour);
				}

				for (int j(0); j < 3; ++j)
				{
					size_t lengthIndex = 0;
					for (; isalnum(line[offset + lengthIndex]); ++lengthIndex);
					size_t index = stoi(line.substr(offset, offset + lengthIndex));

					offset += 1 + lengthIndex;

					if (j == 0)
					{
						//it's the vertex index
						bufferVertex.push_back(index);
						if (usingMaterial)
							bufferColours.push_back(currentColour);

					}
					else if (j == 1)
					{
						//it's the UV index, we do care
						bufferUV.push_back(index);

					}
					else if (j == 2)
					{
						//it's the normals index
						bufferNormals.push_back(index);

					}
				}

			}
		}
		else if (usingMaterial && line.substr(0, 6) == "usemtl")
		{
			//usemtl
			std::string materialName = line.substr(7);
			for (int j = 0; j < materials.size(); ++j)
			{
				if (materialName == materials[j].name) {
					currentColour = j;
					break;
				}
			}
		}
		else if (line[0] == 'o')
		{
			//it's a new object, push_back the vao vector
			m_vao.push_back(VertexArrayObject::Create());
			if (vectorIndex > 0)
			{
				/*size_t size = bufferVertex.size() * (usingMaterial ? 8 : 3) + bufferUV.size() * 2 + bufferNormals.size() * 3;
				float* interleaved = new float[size];
				size_t currentIndex = 0;*/

				std::vector<float> interleaved;

				for (size_t i = 0; i < bufferVertex.size(); ++i) {
					/*interleaved[currentIndex++] = vertex[bufferVertex[i]].x;
					interleaved[currentIndex++] = vertex[bufferVertex[i]].y;
					interleaved[currentIndex++] = vertex[bufferVertex[i]].z;
					if (usingMaterial) {
						interleaved[currentIndex++] = materials[bufferColours[i]].colours.x;
						interleaved[currentIndex++] = materials[bufferColours[i]].colours.y;
						interleaved[currentIndex++] = materials[bufferColours[i]].colours.z;
						interleaved[currentIndex++] = materials[bufferColours[i]].specStrength.x;
						interleaved[currentIndex++] = materials[bufferColours[i]].specStrength.y;
					}
					interleaved[currentIndex++] = UV[bufferUV[i]].x;
					interleaved[currentIndex++] = UV[bufferUV[i]].y;
					interleaved[currentIndex++] = normals[bufferNormals[i]].x;
					interleaved[currentIndex++] = normals[bufferNormals[i]].y;
					interleaved[currentIndex++] = normals[bufferNormals[i]].z;*/
					interleaved.push_back(vertex[bufferVertex[i]].x);
					interleaved.push_back(vertex[bufferVertex[i]].y);
					interleaved.push_back(vertex[bufferVertex[i]].z);
					if (usingMaterial) {
						interleaved.push_back(materials[bufferColours[i]].colours.x);
						interleaved.push_back(materials[bufferColours[i]].colours.y);
						interleaved.push_back(materials[bufferColours[i]].colours.z);
						interleaved.push_back(materials[bufferColours[i]].specStrength.x);
						interleaved.push_back(materials[bufferColours[i]].specStrength.y);
					}
					interleaved.push_back(UV[bufferUV[i]].x);
					interleaved.push_back(UV[bufferUV[i]].y);
					interleaved.push_back(normals[bufferNormals[i]].x);
					interleaved.push_back(normals[bufferNormals[i]].y);
					interleaved.push_back(normals[bufferNormals[i]].z);
				}

				VertexBuffer::sptr interleaved_vbo = VertexBuffer::Create();
				interleaved_vbo->LoadData(interleaved.data(), interleaved.size());

				//change this once UVs are added
				if (usingMaterial) {
					size_t stride = sizeof(float) * 13;
					m_vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
						BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
						BufferAttribute(1, 3, GL_FLOAT, false, stride, sizeof(float) * 3),
						BufferAttribute(4, 2, GL_FLOAT, false, stride, sizeof(float) * 6),
						BufferAttribute(2, 2, GL_FLOAT, false, stride, sizeof(float) * 8),
						BufferAttribute(3, 3, GL_FLOAT, false, stride, sizeof(float) * 10)
						});
				}
				else {
					size_t stride = sizeof(float) * 8;
					m_vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
						BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
						BufferAttribute(1, 2, GL_FLOAT, false, stride, sizeof(float) * 3),
						BufferAttribute(2, 3, GL_FLOAT, false, stride, sizeof(float) * 5)
						});
				}

				m_numberOfVertices.push_back(interleaved.size());

				//delete[] interleaved;

			}
			++vectorIndex;
		}
		else if (line[0] == '#')
		{
			//it's a comment lol i cant even
		}
	}

	file.close();

	std::vector<float> interleaved;

	for (size_t i = 0; i < bufferVertex.size(); ++i) {
		interleaved.push_back(vertex[bufferVertex[i]].x);
		interleaved.push_back(vertex[bufferVertex[i]].y);
		interleaved.push_back(vertex[bufferVertex[i]].z);
		if (usingMaterial) {
			interleaved.push_back(materials[bufferColours[i]].colours.x);
			interleaved.push_back(materials[bufferColours[i]].colours.y);
			interleaved.push_back(materials[bufferColours[i]].colours.z);
			interleaved.push_back(materials[bufferColours[i]].specStrength.x);
			interleaved.push_back(materials[bufferColours[i]].specStrength.y);
		}
		interleaved.push_back(UV[bufferUV[i]].x);
		interleaved.push_back(UV[bufferUV[i]].y);
		interleaved.push_back(normals[bufferNormals[i]].x);
		interleaved.push_back(normals[bufferNormals[i]].y);
		interleaved.push_back(normals[bufferNormals[i]].z);
	}

	VertexBuffer::sptr interleaved_vbo = VertexBuffer::Create();
	interleaved_vbo->LoadData(interleaved.data(), interleaved.size());

	//change this once UVs are added
	if (usingMaterial) {
		size_t stride = sizeof(float) * 13;
		m_vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
			BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
			BufferAttribute(1, 3, GL_FLOAT, false, stride, sizeof(float) * 3),
			BufferAttribute(4, 2, GL_FLOAT, false, stride, sizeof(float) * 6),
			BufferAttribute(2, 2, GL_FLOAT, false, stride, sizeof(float) * 8),
			BufferAttribute(3, 3, GL_FLOAT, false, stride, sizeof(float) * 10)
			});
	}
	else {
		size_t stride = sizeof(float) * 8;
		m_vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
			BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
			BufferAttribute(1, 2, GL_FLOAT, false, stride, sizeof(float) * 3),
			BufferAttribute(2, 3, GL_FLOAT, false, stride, sizeof(float) * 5)
			});
	}

	m_numberOfVertices.push_back(interleaved.size());
}

void ObjLoader::Draw(Camera::sptr camera, glm::mat4 transform, glm::mat3 rotation, glm::vec3 colour,
	glm::vec3 lightPos, glm::vec3 lightColour, float specularStrength, float shininess,
	float ambientLightStrength, glm::vec3 ambientColour, float ambientStrength)
{
	//Draw
	if (m_usingMaterial) {
		m_matShader->Bind();
		m_matShader->SetUniformMatrix("MVP", camera->GetViewProjection() * transform);
		m_matShader->SetUniformMatrix("transform", transform);
		m_matShader->SetUniformMatrix("rotation", rotation);
		m_matShader->SetUniform("camPos", camera->GetPosition());

		m_matShader->SetUniform("lightPos", lightPos);
		m_matShader->SetUniform("lightColour", lightColour);

		m_matShader->SetUniform("ambientLightStrength", ambientLightStrength);
		m_matShader->SetUniform("ambientColour", ambientColour);
		m_matShader->SetUniform("ambientStrength", ambientStrength);
	}
	else {
		m_shader->Bind();
		m_shader->SetUniformMatrix("MVP", camera->GetViewProjection() * transform);
		m_shader->SetUniformMatrix("transform", transform);
		m_shader->SetUniformMatrix("rotation", rotation);
		m_shader->SetUniform("camPos", camera->GetPosition());

		m_shader->SetUniform("colour", colour);
		m_shader->SetUniform("specularStrength", specularStrength);
		m_shader->SetUniform("shininess", shininess);

		m_shader->SetUniform("lightPos", lightPos);
		m_shader->SetUniform("lightColour", lightColour);

		m_shader->SetUniform("ambientLightStrength", ambientLightStrength);
		m_shader->SetUniform("ambientColour", ambientColour);
		m_shader->SetUniform("ambientStrength", ambientStrength);
	}

	for (int i(0); i < m_vao.size(); ++i)
	{
		m_vao[i]->Bind();
		glDrawArrays(GL_TRIANGLES, 0, m_numberOfVertices[i]);
	}
}
