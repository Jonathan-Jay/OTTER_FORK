#include "ObjLoader.h"
#include "Utilities/stringTrimming.h"

std::vector<ObjLoader::DrawData> ObjLoader::m_matQueue = {};
std::vector<ObjLoader::DrawData> ObjLoader::m_texQueue = {};
std::vector<ObjLoader::DrawData> ObjLoader::m_defaultQueue = {};
std::vector<ObjLoader::Models> ObjLoader::m_models = {};
std::vector<ObjLoader::Texture> ObjLoader::m_textures = {};
Shader::sptr ObjLoader::m_shader = nullptr;
Shader::sptr ObjLoader::m_matShader = nullptr;
Shader::sptr ObjLoader::m_texShader = nullptr;

struct Materials
{
	std::string name;
	glm::vec3 colours;
	glm::vec3 specStrength;
};

ObjLoader::ObjLoader(const std::string& fileName, bool usingMaterial)
{
	LoadMesh(fileName, usingMaterial);
}

ObjLoader::~ObjLoader()
{
}

ObjLoader& ObjLoader::LoadMesh(const std::string& fileName, bool usingMaterial)
{
	for (int count(0); count < m_models.size(); ++count) {
		if (m_models[count].fileName == fileName && m_models[count].mat == usingMaterial) {
			m_index = count;

			return *this;
		}
	}

	size_t ind = m_models.size();
	m_models.push_back({ fileName, usingMaterial });

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
		float tempExponent = 1.f, tempTrans = 1;
		while (std::getline(materialFile, matLine))
		{
			stringTrimming::ltrim(matLine);
			if (matLine.substr(0, 6) == "newmtl")
			{
				materials.push_back({ matLine.substr(7), glm::vec3(1.f), glm::vec3(1.f) });
				tempExponent = tempTrans = 1;
				matIndex = materials.size() - 1;
			}
			else if (matLine.substr(0, 6) == "map_Kd" && m_models[ind].texture == INT_MAX)
			{
				std::string textureName = matLine.substr(7);
				bool dupt = false;
				for (int i(0); i < m_textures.size(); ++i) {
					if (m_textures[i].fileName == textureName) {
						m_models[ind].texture = i;
						dupt = true;
						break;
					}
				}
				if (!dupt) {
					if (textureName == ".") {
						Texture2DDescription desc = Texture2DDescription();
						desc.Width = 1;
						desc.Height = 1;
						desc.Format = InternalFormat::RGBA8;
						Texture2D::sptr texture = Texture2D::Create(desc);
						texture->Clear(glm::vec4(0.5f, 0.5f, 0.5f, 0.75f));
						m_models[ind].texture = m_textures.size();
						m_textures.push_back({ textureName, texture });
					}
					else {
						Texture2D::sptr tex = Texture2D::LoadFromFile("textures/" + textureName);
						if (!tex)
						{
							throw std::runtime_error("Failed to open texture\nError 0: " + textureName);
						}
						m_models[ind].texture = m_textures.size();
						m_textures.push_back({ textureName, tex });
					}
				}
				m_models[ind].text = true;
			}
			else if (matLine.substr(0, 2) == "Ns")
			{
				std::istringstream ss = std::istringstream(matLine.substr(2));
				ss >> tempExponent;
			}
			else if (matLine.substr(0, 5) == "trans")
			{
				std::istringstream ss = std::istringstream(matLine.substr(5));
				ss >> tempTrans;
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

					materials[matIndex].specStrength = glm::vec3(colour.length(), tempExponent, tempTrans);
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

		m_models[ind].mat = true;
	}
	else
	{
		m_models[ind].mat = false;
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
	//size_t vectorIndex = 0;

	bool noDraw = false, usingTexture = m_models[ind].text;
	while (std::getline(file, line))
	{
		stringTrimming::trim(line);

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
			else if (line[1] == 't' && usingTexture)
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
		else if (line[0] == 'f' && !noDraw)
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
				else if (i % 3 == 1 && usingTexture)
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

			while (offset < line.size())
			{

				bufferVertex.push_back(bufferVertex[bufferVertex.size() - 3]);
				bufferVertex.push_back(bufferVertex[bufferVertex.size() - 2]);
				if (usingTexture) {
					bufferUV.push_back(bufferUV[bufferUV.size() - 3]);
					bufferUV.push_back(bufferUV[bufferUV.size() - 2]);
				}
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
					else if (j == 1 && usingTexture)
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
			noDraw = (materialName == "tools/toolsnodraw");
		}
		else if (line[0] == 'o')
		{
			//it's a new object, push_back the vao vector < dropped
			/*
			m_models[ind].vao.push_back(VertexArrayObject::Create());
			std::cout << line.substr(1) << '\n';
			if (vectorIndex > 0)
			{
				size_t size = bufferVertex.size() * (usingMaterial ? 8 : 3) + bufferUV.size() * 2 + bufferNormals.size() * 3;
				float* interleaved = new float[size];
				size_t currentIndex = 0;* /

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
					if (usingTexture) {
						interleaved.push_back(UV[bufferUV[i]].x);
						interleaved.push_back(UV[bufferUV[i]].y);
					}
					interleaved.push_back(normals[bufferNormals[i]].x);
					interleaved.push_back(normals[bufferNormals[i]].y);
					interleaved.push_back(normals[bufferNormals[i]].z);
				}

				VertexBuffer::sptr interleaved_vbo = VertexBuffer::Create();
				interleaved_vbo->LoadData(interleaved.data(), interleaved.size());

				//change this once UVs are added
				if (usingTexture) {
					size_t stride = sizeof(float) * 13;
					m_models[ind].vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
						BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
						BufferAttribute(1, 3, GL_FLOAT, false, stride, sizeof(float) * 3),
						BufferAttribute(4, 2, GL_FLOAT, false, stride, sizeof(float) * 6),
						BufferAttribute(2, 2, GL_FLOAT, false, stride, sizeof(float) * 8),
						BufferAttribute(3, 3, GL_FLOAT, false, stride, sizeof(float) * 10)
						});
				}
				else if (usingMaterial) {
					size_t stride = sizeof(float) * 11;
					m_models[ind].vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
						BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
						BufferAttribute(1, 3, GL_FLOAT, false, stride, sizeof(float) * 3),
						BufferAttribute(3, 2, GL_FLOAT, false, stride, sizeof(float) * 6),
						BufferAttribute(2, 3, GL_FLOAT, false, stride, sizeof(float) * 8)
						});
				}
				else {
					size_t stride = sizeof(float) * 6;
					m_models[ind].vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
						BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
						BufferAttribute(1, 3, GL_FLOAT, false, stride, sizeof(float) * 3)
						});
				}

				m_models[ind].verts.push_back(interleaved.size());

				//delete[] interleaved;

			}
			++vectorIndex;*/
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
			interleaved.push_back(materials[bufferColours[i]].specStrength.z);
		}
		if (usingTexture) {
			interleaved.push_back(UV[bufferUV[i]].x);
			interleaved.push_back(UV[bufferUV[i]].y);
		}
		interleaved.push_back(normals[bufferNormals[i]].x);
		interleaved.push_back(normals[bufferNormals[i]].y);
		interleaved.push_back(normals[bufferNormals[i]].z);
	}

	VertexBuffer::sptr interleaved_vbo = VertexBuffer::Create();
	interleaved_vbo->LoadData(interleaved.data(), interleaved.size());

	//change this once UVs are added
	if (usingTexture) {
		size_t stride = sizeof(float) * 14;
		//m_models[ind].vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
		m_models[ind].vao->AddVertexBuffer(interleaved_vbo, {
			BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
			BufferAttribute(1, 3, GL_FLOAT, false, stride, sizeof(float) * 3),
			BufferAttribute(4, 3, GL_FLOAT, false, stride, sizeof(float) * 6),
			BufferAttribute(2, 2, GL_FLOAT, false, stride, sizeof(float) * 9),
			BufferAttribute(3, 3, GL_FLOAT, false, stride, sizeof(float) * 11)
			});
	}
	else if (usingMaterial) {
		size_t stride = sizeof(float) * 12;
		//m_models[ind].vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
		m_models[ind].vao->AddVertexBuffer(interleaved_vbo, {
			BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
			BufferAttribute(1, 3, GL_FLOAT, false, stride, sizeof(float) * 3),
			BufferAttribute(3, 3, GL_FLOAT, false, stride, sizeof(float) * 6),
			BufferAttribute(2, 3, GL_FLOAT, false, stride, sizeof(float) * 9)
			});
	}
	else {
		size_t stride = sizeof(float) * 6;
		//m_models[ind].vao[vectorIndex - 1]->AddVertexBuffer(interleaved_vbo, {
		m_models[ind].vao->AddVertexBuffer(interleaved_vbo, {
			BufferAttribute(0, 3, GL_FLOAT, false, stride, 0),
			BufferAttribute(1, 3, GL_FLOAT, false, stride, sizeof(float) * 3)
			});
	}

	//m_models[ind].verts.push_back(interleaved.size());
	m_models[ind].verts = interleaved.size();

	m_index = ind;

	return *this;
}

void ObjLoader::Init()
{
	m_matShader = Shader::Create();
	m_matShader->LoadShaderPartFromFile("shaders/mat_vertex_shader.glsl", GL_VERTEX_SHADER);
	m_matShader->LoadShaderPartFromFile("shaders/mat_frag_shader.glsl", GL_FRAGMENT_SHADER);
	m_matShader->Link();

	m_texShader = Shader::Create();
	m_texShader->LoadShaderPartFromFile("shaders/tex_vertex_shader.glsl", GL_VERTEX_SHADER);
	m_texShader->LoadShaderPartFromFile("shaders/tex_frag_shader.glsl", GL_FRAGMENT_SHADER);
	m_texShader->Link();

	m_shader = Shader::Create();
	m_shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
	m_shader->LoadShaderPartFromFile("shaders/frag_shader.glsl", GL_FRAGMENT_SHADER);
	m_shader->Link();

	m_matQueue.clear();
	m_texQueue.clear();
	m_defaultQueue.clear();
}

void ObjLoader::Unload()
{
	for (int i(0); i < m_textures.size(); ++i) {
		m_textures[i].texture = nullptr;
	}
	m_textures.clear();
	for (int i(0); i < m_models.size(); ++i) {
		m_models[i].vao = nullptr;
	}
	m_models.clear();

	m_shader = nullptr;
	m_matShader = nullptr;
	m_texShader = nullptr;
}

void ObjLoader::BeginDraw()
{
	m_matQueue.resize(0);
	m_texQueue.resize(0);
	m_defaultQueue.resize(0);
}

void ObjLoader::Draw(const glm::mat4& model)
{
	if (!m_enabled)
		return;

	if (m_models[m_index].text) {
		m_texQueue.push_back({ m_index, model });
	}
	else if (m_models[m_index].mat) {
		m_matQueue.push_back({ m_index, model });
	}
	else {
		m_defaultQueue.push_back({ m_index, model });
	}

	return;
}

void ObjLoader::DrawSkipQueue(const glm::mat4& model, const glm::mat4& view, const Camera& camera, const glm::vec3& colour, const glm::vec3& lightPos1, const glm::vec3& lightPos2, const glm::vec3& lightColour, float specularStrength, float shininess, float ambientLightStrength, const glm::vec3& ambientColour, float ambientStrength)
{
	glm::mat4 VP = camera.GetProjection() * view;
	if (!m_enabled)
		return;

	if (m_models[m_index].text) {
		m_texShader->Bind();

		m_texShader->SetUniform("lightPos1", lightPos1);
		m_texShader->SetUniform("lightPos2", lightPos2);
		m_texShader->SetUniform("lightColour", lightColour);

		m_texShader->SetUniform("ambientLightStrength", ambientLightStrength);
		m_texShader->SetUniform("ambientColour", ambientColour);
		m_texShader->SetUniform("ambientStrength", ambientStrength);

		m_texShader->SetUniform("s_texture", 0);

		m_texShader->SetUniformMatrix("MVP", VP * model);
		m_texShader->SetUniformMatrix("transform", model);

		m_textures[m_models[m_index].texture].texture->Bind(0);

		m_models[m_index].vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, m_models[m_index].verts);
	}

	else if (m_models[m_index].mat) {
		//global stuff
		m_matShader->Bind();

		m_matShader->SetUniform("lightPos1", lightPos1);
		m_matShader->SetUniform("lightPos2", lightPos2);
		m_matShader->SetUniform("lightColour", lightColour);

		m_matShader->SetUniform("ambientLightStrength", ambientLightStrength);
		m_matShader->SetUniform("ambientColour", ambientColour);
		m_matShader->SetUniform("ambientStrength", ambientStrength);

		m_matShader->SetUniformMatrix("MVP", VP * model);
		m_matShader->SetUniformMatrix("transform", model);

		m_models[m_index].vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, m_models[m_index].verts);
	}

	else {
		//global stuff
		m_shader->Bind();

		m_shader->SetUniform("colour", colour);
		m_shader->SetUniform("specularStrength", specularStrength);
		m_shader->SetUniform("shininess", shininess);

		m_shader->SetUniform("lightPos1", lightPos1);
		m_shader->SetUniform("lightPos2", lightPos2);
		m_shader->SetUniform("lightColour", lightColour);

		m_shader->SetUniform("ambientLightStrength", ambientLightStrength);
		m_shader->SetUniform("ambientColour", ambientColour);
		m_shader->SetUniform("ambientStrength", ambientStrength);

		m_shader->SetUniformMatrix("MVP", VP * model);
		m_shader->SetUniformMatrix("transform", model);

		m_models[m_index].vao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, m_models[m_index].verts);
	}
}

void ObjLoader::PerformDraw(const glm::mat4& view, const Camera& camera, const glm::vec3& colour,
	const glm::vec3& lightPos1, const glm::vec3& lightPos2, const glm::vec3& lightColour,
	float specularStrength, float shininess,
	float ambientLightStrength, const glm::vec3& ambientColour, float ambientStrength)
{
	glm::mat4 VP = camera.GetProjection() * view;

	//attempted toon shading: didn't come out great, here's the shader code:
	/*
	if (dot(viewDir, inNormal) < 0.1) {
		frag_color = vec4(0, 0, 0, 1);
		return;
	}
	*/

	if (m_defaultQueue.size() != 0) {
		//global stuff
		m_shader->Bind();

		m_shader->SetUniform("colour", colour);
		m_shader->SetUniform("specularStrength", specularStrength);
		m_shader->SetUniform("shininess", shininess);

		m_shader->SetUniform("lightPos1", lightPos1);
		m_shader->SetUniform("lightPos2", lightPos2);
		m_shader->SetUniform("lightColour", lightColour);

		m_shader->SetUniform("ambientLightStrength", ambientLightStrength);
		m_shader->SetUniform("ambientColour", ambientColour);
		m_shader->SetUniform("ambientStrength", ambientStrength);

		for (int i(0); i < m_defaultQueue.size(); ++i) {
			m_shader->SetUniformMatrix("MVP", VP * m_defaultQueue[i].model);
			m_shader->SetUniformMatrix("transform", m_defaultQueue[i].model);

			m_models[m_defaultQueue[i].modelIndex].vao->Bind();
			glDrawArrays(GL_TRIANGLES, 0, m_models[m_defaultQueue[i].modelIndex].verts);
		}
	}

	if (m_texQueue.size() != 0) {
		m_texShader->Bind();

		m_texShader->SetUniform("lightPos1", lightPos1);
		m_texShader->SetUniform("lightPos2", lightPos2);
		m_texShader->SetUniform("lightColour", lightColour);

		m_texShader->SetUniform("ambientLightStrength", ambientLightStrength);
		m_texShader->SetUniform("ambientColour", ambientColour);
		m_texShader->SetUniform("ambientStrength", ambientStrength);

		m_texShader->SetUniform("s_texture", 0);

		for (int i(0); i < m_texQueue.size(); ++i) {
			m_texShader->SetUniformMatrix("MVP", VP * m_texQueue[i].model);
			m_texShader->SetUniformMatrix("transform", m_texQueue[i].model);

			m_textures[m_models[m_texQueue[i].modelIndex].texture].texture->Bind(0);

			m_models[m_texQueue[i].modelIndex].vao->Bind();
			glDrawArrays(GL_TRIANGLES, 0, m_models[m_texQueue[i].modelIndex].verts);
		}
	}

	if (m_matQueue.size() != 0) {
		//global stuff
		m_matShader->Bind();

		m_matShader->SetUniform("lightPos1", lightPos1);
		m_matShader->SetUniform("lightPos2", lightPos2);
		m_matShader->SetUniform("lightColour", lightColour);

		m_matShader->SetUniform("ambientLightStrength", ambientLightStrength);
		m_matShader->SetUniform("ambientColour", ambientColour);
		m_matShader->SetUniform("ambientStrength", ambientStrength);

		for (int i(0); i < m_matQueue.size(); ++i) {
			m_matShader->SetUniformMatrix("MVP", VP * m_matQueue[i].model);
			m_matShader->SetUniformMatrix("transform", m_matQueue[i].model);

			m_models[m_matQueue[i].modelIndex].vao->Bind();
			glDrawArrays(GL_TRIANGLES, 0, m_models[m_matQueue[i].modelIndex].verts);
		}
	}

	Shader::UnBind();
	VertexArrayObject::UnBind();
}
