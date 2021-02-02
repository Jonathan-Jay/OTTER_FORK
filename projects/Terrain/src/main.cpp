#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream> //03
#include <string> //03

#include <GLM/glm.hpp> //04
#include <glm/gtc/matrix_transform.hpp> //04

#define STB_IMAGE_IMPLEMENTATION //07
#include <stb_image.h> //07

GLFWwindow* window;

float spread = 0.75f, waterHeight = 2.f, tiling = 10;

//INPUT handling
float rx = 0.0f, ry = 0.0f,
tz = 0.0f, ty = 0.0f, tx = 0.0f;

unsigned char* loadImage(std::string filename, int &width, int &height) {
	int channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* image = stbi_load(filename.c_str(),
		&width,
		&height,
		&channels,
		0);
	if (image) {
		std::cout << "Image LOADED " << width << " " << height << " " << channels << std::endl;
	}
	else {
		std::cout << "Failed to load image!" << std::endl;
	}

	return image;
}

bool initGLFW() {
	if (glfwInit() == GLFW_FALSE) {
		std::cout << "Failed to Initialize GLFW" << std::endl;
		return false;
	}

	//Create a new GLFW window
	window = glfwCreateWindow(800, 800, "Window", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	return true;
}

bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		std::cout << "Failed to initialize Glad" << std::endl;
		return false;
	}

	return true;
}

GLuint loadShaders(std::string vert, std::string frag) {
	// Read Shaders from file
	std::string vert_shader_str;
	std::ifstream vs_stream(vert, std::ios::in);
	if (vs_stream.is_open()) {
		std::string Line = "";
		while (getline(vs_stream, Line))
			vert_shader_str += "\n" + Line;
		vs_stream.close();
	}
	else {
		printf("Could not open vertex shader!!\n");
		return -1;
	}
	const char* vs_str = vert_shader_str.c_str();

	std::string frag_shader_str;
	std::ifstream fs_stream(frag, std::ios::in);
	if (fs_stream.is_open()) {
		std::string Line = "";
		while (getline(fs_stream, Line))
			frag_shader_str += "\n" + Line;
		fs_stream.close();
	}
	else {
		printf("Could not open fragment shader!!\n");
		return -1;
	}
	const char* fs_str = frag_shader_str.c_str();
	
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_str, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_str, NULL);
	glCompileShader(fs);

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, fs);
	glAttachShader(shader_program, vs);
	glLinkProgram(shader_program);

	return shader_program;
}

void keyboard() {
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		waterHeight += 0.001f;
		if (waterHeight >= 3.5f)
			waterHeight = 3.5f;
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		waterHeight -= 0.001f;
		if (waterHeight <= 0.5f)
			waterHeight = 0.5f;
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		spread += 0.001f;
		if (spread >= 1.f)
			spread = 1.f;
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		spread -= 0.001f;
		if (spread <= 0)
			spread = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		tiling += 0.01f;
		if (tiling >= 100)
			tiling = 100;
	}
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		tiling -= 0.01f;
		if (tiling <= 1)
			tiling = 1;
	}


	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		ry += 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		ry -= 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		rx -= 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		rx += 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		tz -= 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		tz += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		tx += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		tx -= 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		ty += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		ty -= 0.001f;
	}
}

size_t genPlane(int d, GLfloat* points2, GLfloat* uvs2) {
	size_t numVertices = size_t(6 * (d * d));
	points2 = new GLfloat[3*numVertices];
	uvs2 = new GLfloat[2*numVertices];
	GLfloat x, y, z;
	int i = 0;
	x = -1.0;
	y = 0;
	z = 1.0;

	int uvcounter = 0;

	for (int col = 0; col < d; col++)
	for (int row = 0; row < d; row++) {

		points2[i++] = x + row * (2.f / d);
		//std::cout << "Px[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = y;
		//std::cout << "Py[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = z - col * (2.f / d);
		//std::cout << "Pz[" << i - 1 << "] = " << points2[i - 1] << "\n";

		uvs2[uvcounter++] = 0.f + row*(1.f/d);
		uvs2[uvcounter++] = 0.f + col*(1.f/d);

		points2[i++] = points2[i-3] + (2.f / d); //std::cout << points2[i-4] << " " << 2.0/d << "\n";
		//std::cout << "Px[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = y;
		//std::cout << "Py[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = points2[i-3];
		//std::cout << "Pz[" << i - 1 << "] = " << points2[i - 1] << "\n";

		uvs2[uvcounter++] = uvs2[uvcounter-2] + 1.f/d;
		uvs2[uvcounter++] = uvs2[uvcounter-2];

		points2[i++] = points2[i-6];
		//std::cout << "Px[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = y;
		//std::cout << "Py[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = points2[i-3] - (2.f / d);
		//std::cout << "Pz[" << i - 1 << "] = " << points2[i - 1] << "\n";

		uvs2[uvcounter++] = uvs2[uvcounter - 4];
		uvs2[uvcounter++] = uvs2[uvcounter - 2] + 1.f/d;

		points2[i++] = points2[i - 6];
		//std::cout << "Px[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = y;
		//std::cout << "Py[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = points2[i - 6];
		//std::cout << "Pz[" << i - 1 << "] = " << points2[i - 1] << "\n";

		uvs2[uvcounter++] = uvs2[uvcounter - 4];
		uvs2[uvcounter++] = uvs2[uvcounter - 4];

		points2[i++] = points2[i - 3];
		//std::cout << "Px[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = y;
		//std::cout << "Py[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = points2[i - 3] - (2.f / d);
		//std::cout << "Pz[" << i - 1 << "] = " << points2[i - 1] << "\n";

		uvs2[uvcounter++] = uvs2[uvcounter - 2];
		uvs2[uvcounter++] = uvs2[uvcounter - 4];


		points2[i++] = points2[i - 9];
		//std::cout << "Px[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = y;
		//std::cout << "Py[" << i - 1 << "] = " << points2[i - 1] << "\n";
		points2[i++] = points2[i - 3];
		//std::cout << "Pz[" << i - 1 << "] = " << points2[i - 1] << "\n";

		uvs2[uvcounter++] = uvs2[uvcounter - 6];
		uvs2[uvcounter++] = uvs2[uvcounter - 6];

	}


	// VAO
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//VBO
	GLuint pos_vbo = 0;
	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, (3 * numVertices) * sizeof(GLfloat), points2, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	GLuint uv_vbo = 1;
	glGenBuffers(1, &uv_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
	glBufferData(GL_ARRAY_BUFFER, (2 * numVertices) * sizeof(GLfloat), uvs2, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	return numVertices;
}

int main() {
	
	//Initialize GLFW
	if (!initGLFW())
		return 1;
	
	//Initialize GLAD
	if (!initGLAD())
		return 2;

	//Mesh data
	GLfloat* points2 = nullptr;
	GLfloat* uvs2 = nullptr;
	size_t numVertices = genPlane(250, points2, uvs2);

	GLuint terrainShader;
	GLuint waterShader;
	// Load the shaders
	if ((terrainShader = loadShaders("terrain.vert", "terrain.frag")) > -1)
		return 3;
	if ((waterShader = loadShaders("water.vert", "water.frag")) > -1)
		return 3;
	
	std::cout << "\nControls:\nWASD LShift Space to move the terain\nArrow keys to rotate\n"
		"IK to raise or lower water level\nJL to adjust smoothness\nUO to adjust tiling amount\n";

	//loading textures
	GLuint textureHandle[5];
	glGenTextures(5, textureHandle);
	for (int i(0); i < 5; ++i) {

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textureHandle[i]);

		int height, width;
		std::string filename = "";
		switch (i) {
		case 0:	filename = "heightmap.bmp";	break;
		case 1:	filename = "sand.png";		break;
		case 2:	filename = "grass.png";		break;
		case 3:	filename = "rock.png";		break;
		case 4:	filename = "water.png";		break;
		}
		unsigned char* image = loadImage(filename, width, height);
		if (i == 0)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		stbi_image_free(image);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	}

	///terrain uniforms		(TU stands for terrain uniform)
		glUseProgram(terrainShader);

		GLuint MatrixTU =		glGetUniformLocation(terrainShader, "MVP");
		GLuint spreadTU =		glGetUniformLocation(terrainShader, "spread");
		GLuint tilingTU =		glGetUniformLocation(terrainShader, "tiling");
		GLuint heightTU =		glGetUniformLocation(terrainShader, "waterHeight");

		/////// TEXTURE
		glUniform1i(glGetUniformLocation(terrainShader, "heightmap"), 0);
		glUniform1i(glGetUniformLocation(terrainShader, "sand"), 1);
		glUniform1i(glGetUniformLocation(terrainShader, "grass"), 2);
		glUniform1i(glGetUniformLocation(terrainShader, "rock"), 3);
	///End of terrain uniforms


	///Water uniforms		(WU stands for water uniform)
		glUseProgram(waterShader);

		GLuint MatrixWU =		glGetUniformLocation(waterShader, "MVP");
		GLuint TimeWU =			glGetUniformLocation(waterShader, "time");
		GLuint tilingWU =		glGetUniformLocation(waterShader, "tiling");
		GLuint heightWU =		glGetUniformLocation(waterShader, "waterHeight");

		/////// TEXTURE
		glUniform1i(glGetUniformLocation(waterShader, "water"), 4);
	///End of water uniforms


	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	glm::mat4 VP;	//VP calculations done once
	{
		glm::mat4 Projection =
			glm::perspective(glm::radians(45.0f),
				(float)width / (float)height, 0.0001f, 100.0f);

		// Camera matrix
		glm::mat4 View = glm::lookAt(
			glm::vec3(0, 2, 4),		//position
			glm::vec3(0, 0, 0),		//looking at
			glm::vec3(0, 1, 0)		//up
		);

		VP = Projection * View;
	}
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 mvp = VP * Model;
	
	// Enable depth buffer
	glEnable(GL_DEPTH_TEST);
	//let there be transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		keyboard();
		float time = glfwGetTime();
		
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Model = glm::translate(glm::mat4(1.f), glm::vec3(tx, ty, tz));
		Model = glm::rotate(Model, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
		Model = glm::rotate(Model, glm::radians(rx), glm::vec3(1.0f, 0.0f, 0.0f));
		mvp = VP * Model;
		
		//draw terrain
			glUseProgram(terrainShader);

			glUniformMatrix4fv(MatrixTU, 1, GL_FALSE, &mvp[0][0]);
			glUniform1f(spreadTU, spread);
			glUniform1f(tilingTU, tiling);
			glUniform1f(heightTU, waterHeight);
		
			glDrawArrays(GL_TRIANGLES, 0, numVertices);
		//end draw terrain


		//draw water
			glUseProgram(waterShader);

			glUniformMatrix4fv(MatrixWU, 1, GL_FALSE, &mvp[0][0]);
			glUniform1f(TimeWU, time);
			glUniform1f(tilingWU, tiling);
			glUniform1f(heightWU, waterHeight);

			glDrawArrays(GL_TRIANGLES, 0, numVertices);
		//end draw water

		glfwSwapBuffers(window);
	}

	delete points2;
	delete uvs2;

	return 0;
}