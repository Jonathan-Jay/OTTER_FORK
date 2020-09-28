#include <glad/glad.h> //has to be before GLFW
#include <GLFW/glfw3.h>
#include <iostream>

class vec3 {
public:
	vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
	float x = 0, y = 0, z = 0;

	vec3 operator+(const vec3& other) { return vec3(this->x + other.x, this->y + other.y, this->z + other.z); }
	vec3 operator-(const vec3& other) { return vec3(this->x - other.x, this->y - other.y, this->z - other.z); }
	vec3 operator*(float f) { return vec3(this->x * f, this->y * f, this->z * f); }
};

vec3 LERP(vec3 p0, vec3 p1, float t) {
	return p0 * (1.f - t) + p1 * t;
}

int main() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		std::cout << "Failed to initialize Glad" << std::endl;
		return 1;
	}
	// Create a new GLFW window
	GLFWwindow* window = glfwCreateWindow(300, 300, "Jonathan Jay - 100743575", nullptr, nullptr);
	// We want GL commands to be executed for our window, so we make our window's context the current one
	glfwMakeContextCurrent(window);

	// Let glad know what function loader we are using (will call gl commands via glfw)
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		std::cout<< "Failed to initialize Glad" << std::endl;
		return 2;
	}

	// Display our GPU and OpenGL version
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;

	vec3 p0(0, 0, 0);
	vec3 p1((rand() % 11) / 10, (rand() % 11) / 10, (rand() % 11) / 10);

	float t(0);

	// Run as long as the window is open
	while (!glfwWindowShouldClose(window)) {
		// Poll for events from windows (clicks, keypressed, closing, all that)
		glfwPollEvents();

		t += 0.001;
		if (t >= 1) {
			t = 0;
			p0 = p1;
			p1 = vec3((rand() % 11) / 10.f, (rand() % 11) / 10.f, (rand() % 11) / 10.f);
		}

		vec3 temp = LERP(p0, p1, t);
		// Clear our screen every frame
		glClearColor(temp.x, temp.y, temp.z, 1);
		//glClearColor(float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX);
		glClear(GL_COLOR_BUFFER_BIT);

		// Present our image to windows
		glfwSwapBuffers(window);
	}

	return 0;
}