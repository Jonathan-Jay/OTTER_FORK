#include "ObjLoader.h"

#define LOG_GL_NOTIFICATIONS

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
	case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
	case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
#ifdef LOG_GL_NOTIFICATIONS
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
#endif
	default: break;
	}
}

GLFWwindow* window;

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

bool initGLFW() {
	if (glfwInit() == GLFW_FALSE) {
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window
	window = glfwCreateWindow(800, 800, "Object Loader", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);

	return true;
}

bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
}

int main() {
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	ObjLoader::Init();

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GlDebugMessage, nullptr);


	ObjLoader blade("blade.obj", true);	//remove the true to render a black and white version.
	//ObjLoader diorama("Char.obj", true);

	Camera::sptr camera = Camera::Create();
	//camera->SetPosition(glm::vec3(0, 0, 15));
	camera->SetPosition(glm::vec3(0, 0, 3));
	camera->LookAt(glm::vec3(0.f));
	camera->SetFovDegrees(90);
	//camera->SetOrtho(25);

	// GL states
	glEnable(GL_DEPTH_TEST);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_FRONT); //GL_BACK, GL_FRONT_AND_BACK

	float oldFrame = 0;
	float currentFrame = glfwGetTime();
	float angle = 0;
	float deltaTime = currentFrame - oldFrame;

	bool spacePressed = false;
	bool currentCamera = CAMERA_PERSPECTIVE;
	currentCamera = !currentCamera;

	glm::mat4 transform = glm::mat4(1.f);
	glm::mat3 rotation = glm::mat4(1.f);
	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_SPACE)) 
		{
			if (!spacePressed)
			{
				//change ortho here
				camera->ChangePerspective(currentCamera);
				currentCamera = !currentCamera;
			}
			spacePressed = true;
			if (currentCamera)
				blade.LoadMesh("blade.obj");
			else
				blade.LoadMesh("blade.obj", true);
		}
		else
		{
			spacePressed = false;
		}

		oldFrame = currentFrame;
		currentFrame = glfwGetTime();
		
		deltaTime = currentFrame - oldFrame;

		angle += deltaTime;

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*rotation = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, -1.f, 0.f));

		transform = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -7.2f, 0.f));
		transform = glm::rotate(transform, angle, glm::vec3(0.f, -1.f, 0.f));
		diorama.Draw(camera, transform, rotation, glm::vec3(1.f, 1.f, 1.f), glm::vec3(3.f, 0.f, 3.f), glm::vec3(3.f));

		transform = glm::translate(transform, glm::vec3(0.f, 7.7f, 4.f));
		transform = glm::rotate(transform, angle, glm::vec3(0.f, -1.f, 0.f));

		rotation = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, -1.f, 0.f));

		blade.Draw(camera, transform, rotation, glm::vec3(1.f, 1.f, 1.f), glm::vec3(3.f, 0.f, 3.f), glm::vec3(3.f));*/

		rotation = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, 1.f, 0.f));

		transform = glm::translate(glm::mat4(1.f), glm::vec3(-1.5f, -1.2f, 0.f));
		transform = glm::rotate(transform, angle, glm::vec3(0.f, 1.f, 0.f));

		blade.Draw(camera, transform, rotation, glm::vec3(1.f, 1.f, 1.f), camera->GetPosition(), glm::vec3(1.f));

		rotation = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, -1.f, 0.f));

		transform = glm::translate(glm::mat4(1.f), glm::vec3(1.5f, -1.2f, 0.f));
		transform = glm::rotate(transform, angle, glm::vec3(0.f, -1.f, 0.f));

		blade.Draw(camera, transform, rotation, glm::vec3(1.f, 1.f, 1.f), camera->GetPosition(), glm::vec3(1.f));



		glfwSwapBuffers(window);
	}

	Shader::UnBind();
	VertexArrayObject::UnBind();

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
} 
