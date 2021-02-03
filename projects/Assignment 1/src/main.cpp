//Just a simple handler for simple initialization stuffs
#include "Utilities/BackendHandler.h"
#include "Utilities/Util.h"

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <Texture2D.h>
#include <Texture2DData.h>
#include <MeshBuilder.h>
#include <MeshFactory.h>
#include <NotObjLoader.h>
#include <ObjLoader.h>
#include <VertexTypes.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <TextureCubeMap.h>
#include <TextureCubeMapData.h>

#include <Timing.h>
#include <GameObjectTag.h>
#include <InputHelpers.h>

#include <IBehaviour.h>
#include <CameraControlBehaviour.h>
#include <FollowPathBehaviour.h>
#include <SimpleMoveBehaviour.h>


int main() {
	int frameIx = 0;
	float fpsBuffer[128];
	float minFps, maxFps, avgFps;
	int selectedVao = 0; // select cube by default
	std::vector<GameObject> controllables;

	BackendHandler::InitAll();

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(BackendHandler::GlDebugMessage, nullptr);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Push another scope so most memory should be freed *before* we exit the app
	{
		#pragma region Shader and ImGui

		// Load our shaders
		Shader::sptr shader = Shader::Create();
		shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		//shader->LoadShaderPartFromFile("shaders/frag_blinn_phong_textured.glsl", GL_FRAGMENT_SHADER);
		shader->LoadShaderPartFromFile("shaders/frag_phong_textured_toggleable.glsl", GL_FRAGMENT_SHADER);
		shader->Link();

		glm::vec3 lightPos = glm::vec3(0.0f, 2.0f, 2.0f);
		glm::vec3 lightCol = glm::vec3(0.85f, 0.75f, 0.9f);
		float     lightAmbientPow = 0.05f;
		float     lightSpecularPow = 1.0f;
		glm::vec3 ambientCol = glm::vec3(1.0f);
		float     ambientPow = 0.1f;
		float     lightLinearFalloff = 0.09f;
		float     lightQuadraticFalloff = 0.032f;

		bool usingLightingOnly = false;
		bool usingAmbient = true;
		bool usingDiffuse = true;
		bool usingSpecular = true;
		bool usingToon = true;


		// These are our application / scene level uniforms that don't necessarily update
		// every frame
		shader->SetUniform("u_LightPos", lightPos);
		shader->SetUniform("u_LightCol", lightCol);
		shader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
		shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
		shader->SetUniform("u_AmbientCol", ambientCol);
		shader->SetUniform("u_AmbientStrength", ambientPow);
		shader->SetUniform("u_LightAttenuationConstant", 1.0f);
		shader->SetUniform("u_LightAttenuationLinear", lightLinearFalloff);
		shader->SetUniform("u_LightAttenuationQuadratic", lightQuadraticFalloff);

		// We'll add some ImGui controls to control our shader
		BackendHandler::imGuiCallbacks.push_back([&]() {
			if (ImGui::CollapsingHeader("Scene Level Lighting Settings"))
			{
				if (ImGui::ColorPicker3("Ambient Color", glm::value_ptr(ambientCol))) {
					shader->SetUniform("u_AmbientCol", ambientCol);
				}
				if (ImGui::SliderFloat("Fixed Ambient Power", &ambientPow, 0.01f, 1.0f)) {
					shader->SetUniform("u_AmbientStrength", ambientPow);
				}
			}
			if (ImGui::CollapsingHeader("Light Level Lighting Settings"))
			{
				if (ImGui::DragFloat3("Light Pos", glm::value_ptr(lightPos), 0.01f, -10.0f, 10.0f)) {
					shader->SetUniform("u_LightPos", lightPos);
				}
				if (ImGui::ColorPicker3("Light Col", glm::value_ptr(lightCol))) {
					shader->SetUniform("u_LightCol", lightCol);
				}
				if (ImGui::SliderFloat("Light Ambient Power", &lightAmbientPow, 0.0f, 1.0f)) {
					shader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
				}
				if (ImGui::SliderFloat("Light Specular Power", &lightSpecularPow, 0.0f, 1.0f)) {
					shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
				}
				if (ImGui::DragFloat("Light Linear Falloff", &lightLinearFalloff, 0.01f, 0.0f, 1.0f)) {
					shader->SetUniform("u_LightAttenuationLinear", lightLinearFalloff);
				}
				if (ImGui::DragFloat("Light Quadratic Falloff", &lightQuadraticFalloff, 0.01f, 0.0f, 1.0f)) {
					shader->SetUniform("u_LightAttenuationQuadratic", lightQuadraticFalloff);
				}
			}
			if (ImGui::CollapsingHeader("Assignment toggles"))
			{
				if (ImGui::CollapsingHeader("Templates"))
				{
					if (ImGui::Button("1: No Lighting")) {
						usingLightingOnly = true;
						usingAmbient = false;
						usingDiffuse = true;
						usingSpecular = false;
						usingToon = false;
					}
					if (ImGui::Button("2: Only Ambient")) {
						usingLightingOnly = false;
						usingAmbient = true;
						usingDiffuse = false;
						usingSpecular = false;
						usingToon = false;
					}
					if (ImGui::Button("3: Only Specular")) {
						usingLightingOnly = false;
						usingAmbient = false;
						usingDiffuse = false;
						usingSpecular = true;
						usingToon = false;
					}
					if (ImGui::Button("4: Ambient + Specular")) {
						usingLightingOnly = false;
						usingAmbient = true;
						usingDiffuse = true;
						usingSpecular = true;
						usingToon = false;
					}
					if (ImGui::Button("5: Ambient + Specular + Toon aka all")) {
						usingLightingOnly = false;
						usingAmbient = true;
						usingDiffuse = true;
						usingSpecular = true;
						usingToon = true;
					}
				}
				if (ImGui::CollapsingHeader("Individual"))
				{
					ImGui::Checkbox("Lighting Only", &usingLightingOnly);
					ImGui::Checkbox("Ambient", &usingAmbient);
					ImGui::Checkbox("Diffuse", &usingDiffuse);
					ImGui::Checkbox("Specular", &usingSpecular);
					ImGui::Checkbox("Toon Shading", &usingToon);
				}
			}

			auto name = controllables[selectedVao].get<GameObjectTag>().Name;
			ImGui::Text(name.c_str());
			//auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
			//ImGui::Checkbox("Relative Rotation", &behaviour->Relative);

			ImGui::Text("Q/E -> Yaw\nLeft/Right -> Roll\nUp/Down -> Pitch\nY -> Toggle Mode");
		
			minFps = FLT_MAX;
			maxFps = 0;
			avgFps = 0;
			for (int ix = 0; ix < 128; ix++) {
				if (fpsBuffer[ix] < minFps) { minFps = fpsBuffer[ix]; }
				if (fpsBuffer[ix] > maxFps) { maxFps = fpsBuffer[ix]; }
				avgFps += fpsBuffer[ix];
			}
			ImGui::PlotLines("FPS", fpsBuffer, 128);
			ImGui::Text("MIN: %f MAX: %f AVG: %f", minFps, maxFps, avgFps / 128.0f);
			});

		#pragma endregion 

		// GL states
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); // New 

		#pragma region TEXTURE LOADING

		// Load some textures from files
		Texture2D::sptr DioramaTex = Texture2D::LoadFromFile("images/DioramaTexture.png");

		// Load the cube map
		//TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/sample.jpg");
		TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/ToonSky.jpg"); 

		// Creating an empty texture
		Texture2DDescription desc = Texture2DDescription();
		desc.Width = 1;
		desc.Height = 1;
		desc.Format = InternalFormat::RGB8;
		Texture2D::sptr texture2 = Texture2D::Create(desc);
		// Clear it with a white colour
		texture2->Clear();

		#pragma endregion

		///////////////////////////////////// Scene Generation //////////////////////////////////////////////////
		#pragma region Scene Generation
		
		// We need to tell our scene system what extra component types we want to support
		GameScene::RegisterComponentType<RendererComponent>();
		GameScene::RegisterComponentType<BehaviourBinding>();
		GameScene::RegisterComponentType<Camera>();

		// Create a scene, and set it to be the active scene in the application
		GameScene::sptr scene = GameScene::Create("test");
		Application::Instance().ActiveScene = scene;

		// We can create a group ahead of time to make iterating on the group faster
		entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, RendererComponent> renderGroup =
			scene->Registry().group<RendererComponent>(entt::get_t<Transform>());

		// Create a material and set some properties for it
		ShaderMaterial::sptr floorMat = ShaderMaterial::Create();  
		floorMat->Shader = shader;
		floorMat->Set("s_Diffuse", texture2);
		floorMat->Set("s_Specular", texture2);
		floorMat->Set("u_Shininess", 1.0f);
		floorMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr dioramaMat = ShaderMaterial::Create();  
		dioramaMat->Shader = shader;
		dioramaMat->Set("s_Diffuse", DioramaTex);
		dioramaMat->Set("s_Specular", texture2);
		dioramaMat->Set("u_Shininess", 12.0f);
		dioramaMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr tankMat = ShaderMaterial::Create();  
		tankMat->Shader = shader;
		tankMat->Set("s_Diffuse", texture2);
		tankMat->Set("s_Specular", texture2);
		tankMat->Set("u_Shininess", 2.0f);
		tankMat->Set("u_TextureMix", 0.0f);

		GameObject obj1 = scene->CreateEntity("Ground"); 
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj", glm::vec4(0.5f, 0.5f, 0.5f, 1.f));
			obj1.emplace<RendererComponent>().SetMesh(vao).SetMaterial(floorMat);
		}

		GameObject obj2 = scene->CreateEntity("Diorama");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Diorama.obj");
			obj2.emplace<RendererComponent>().SetMesh(vao).SetMaterial(dioramaMat);
			obj2.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj2.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			obj2.get<Transform>().SetLocalScale(0.25f, 0.25f, 0.25f);
		}

		GameObject obj3 = scene->CreateEntity("Tank1");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Tank.obj", glm::vec4(1, 0, 0, 1));
			obj3.emplace<RendererComponent>().SetMesh(vao).SetMaterial(tankMat);
			obj3.get<Transform>().SetLocalPosition(-4.0f, -4.0f, 0.0f);
			obj3.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			obj3.get<Transform>().SetLocalScale(2.f, 2.f, 2.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj3);
			path->Points.push_back({ glm::vec3(-4, -4, 1), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(4, -4, 1), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(4, 4, 1), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-4, 4, 1), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Speed = 4.f;
		}
		GameObject obj4 = scene->CreateEntity("Tank2");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Tank.obj", glm::vec4(0, 0, 1, 1));
			obj4.emplace<RendererComponent>().SetMesh(vao).SetMaterial(tankMat);
			obj4.get<Transform>().SetLocalPosition(7.0f, -7.0f, 0.0f);
			obj4.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			obj4.get<Transform>().SetLocalScale(2.f, 2.f, 2.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj4);
			path->Points.push_back({ glm::vec3(7, -7, 1), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(7, 7, 1), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-7, 7, 1), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-7, -7, 1), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Speed = 7.f;
		}
		GameObject obj5 = scene->CreateEntity("Tank3");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Tank.obj", glm::vec4(0, 1, 0, 1));
			obj5.emplace<RendererComponent>().SetMesh(vao).SetMaterial(tankMat);
			obj5.get<Transform>().SetLocalPosition(10.0f, 10.0f, 0.0f);
			obj5.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			obj5.get<Transform>().SetLocalScale(2.f, 2.f, 2.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj5);
			path->Points.push_back({ glm::vec3(10, 10, 1), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-10, 10, 1), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-10, -10, 1), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(10, -10, 1), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Speed = 15.f;
		}

		GameObject obj6 = scene->CreateEntity("Tank4");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Tank.obj", glm::vec4(0, 1, 0, 1));
			obj6.emplace<RendererComponent>().SetMesh(vao).SetMaterial(tankMat);
			obj6.get<Transform>().SetLocalPosition(4.0f, 4.0f, 0.0f);
			obj6.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			obj6.get<Transform>().SetLocalScale(2.f, 2.f, 2.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj6);
			path->Points.push_back({ glm::vec3(4, 4, 1), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-4, 4, 1), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-4, -4, 1), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(4, -4, 1), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Speed = 4.f;
		}
		GameObject obj7 = scene->CreateEntity("Tank5");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Tank.obj", glm::vec4(0, 0, 1, 1));
			obj7.emplace<RendererComponent>().SetMesh(vao).SetMaterial(tankMat);
			obj7.get<Transform>().SetLocalPosition(-7.0f, 7.0f, 0.0f);
			obj7.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			obj7.get<Transform>().SetLocalScale(2.f, 2.f, 2.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj7);
			path->Points.push_back({ glm::vec3(-7, 7, 1), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-7, -7, 1), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(7, -7, 1), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(7, 7, 1), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Speed = 7.f;
		}
		GameObject obj8 = scene->CreateEntity("Tank6");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Tank.obj", glm::vec4(1, 0, 0, 1));
			obj8.emplace<RendererComponent>().SetMesh(vao).SetMaterial(tankMat);
			obj8.get<Transform>().SetLocalPosition(-10.0f, -10.0f, 0.0f);
			obj8.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			obj8.get<Transform>().SetLocalScale(2.f, 2.f, 2.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj8);
			path->Points.push_back({ glm::vec3(-10, -10, 1), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(10, -10, 1), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(10, 10, 1), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Points.push_back({ glm::vec3(-10, 10, 1), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) });
			path->Speed = 15.f;
		}

		// Create an object to be our camera
		GameObject cameraObject = scene->CreateEntity("Camera");
		{
			cameraObject.get<Transform>().SetLocalPosition(0, 3, 3).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& camera = cameraObject.emplace<Camera>();// Camera::Create();
			camera.SetPosition(glm::vec3(0, 3, 3));
			camera.SetUp(glm::vec3(0, 0, 1));
			camera.LookAt(glm::vec3(0));
			camera.SetFovDegrees(90.0f); // Set an initial FOV
			camera.SetOrthoHeight(3.0f);
			BehaviourBinding::Bind<CameraControlBehaviour>(cameraObject)->_moveSpeed = 3.f;
		}

		Framebuffer* testBuffer;
		GameObject framebufferObject = scene->CreateEntity("Basic Buffer");
		{
			int width, height;
			glfwGetWindowSize(BackendHandler::window, &width, &height);

			testBuffer = &framebufferObject.emplace<Framebuffer>();
			testBuffer->AddDepthTarget();
			testBuffer->AddColorTarget(GL_RGBA8);
			testBuffer->Init(width, height);
		}
		#pragma endregion 
		//////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////// SKYBOX ///////////////////////////////////////////////
		{
			// Load our shaders
			Shader::sptr skybox = std::make_shared<Shader>();
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.vert.glsl", GL_VERTEX_SHADER);
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.frag.glsl", GL_FRAGMENT_SHADER);
			skybox->Link();

			ShaderMaterial::sptr skyboxMat = ShaderMaterial::Create();
			skyboxMat->Shader = skybox;  
			skyboxMat->Set("s_Environment", environmentMap);
			skyboxMat->Set("u_EnvironmentRotation", glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))));
			skyboxMat->RenderLayer = 100;

			MeshBuilder<VertexPosNormTexCol> mesh;
			MeshFactory::AddIcoSphere(mesh, glm::vec3(0.0f), 1.0f);
			MeshFactory::InvertFaces(mesh);
			VertexArrayObject::sptr meshVao = mesh.Bake();
			
			GameObject skyboxObj = scene->CreateEntity("skybox");  
			skyboxObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			skyboxObj.get_or_emplace<RendererComponent>().SetMesh(meshVao).SetMaterial(skyboxMat);
		}
		////////////////////////////////////////////////////////////////////////////////////////


		// We'll use a vector to store all our key press events for now (this should probably be a behaviour eventually)
		std::vector<KeyPressWatcher> keyToggles;
		{
			// This is an example of a key press handling helper. Look at InputHelpers.h an .cpp to see
			// how this is implemented. Note that the ampersand here is capturing the variables within
			// the scope. If you wanted to do some method on the class, your best bet would be to give it a method and
			// use std::bind
			keyToggles.emplace_back(GLFW_KEY_T, [&]() { cameraObject.get<Camera>().ToggleOrtho(); });

			controllables.push_back(obj2);

			keyToggles.emplace_back(GLFW_KEY_KP_ADD, [&]() {
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
				selectedVao++;
				if (selectedVao >= controllables.size())
					selectedVao = 0;
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
				});
			keyToggles.emplace_back(GLFW_KEY_KP_SUBTRACT, [&]() {
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
				selectedVao--;
				if (selectedVao < 0)
					selectedVao = controllables.size() - 1;
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
				});

			keyToggles.emplace_back(GLFW_KEY_Y, [&]() {
				auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
				behaviour->Relative = !behaviour->Relative;
				});
		}

		// Initialize our timing instance and grab a reference for our use
		Timing& time = Timing::Instance();
		time.LastFrame = glfwGetTime();

		///// Game loop /////
		while (!glfwWindowShouldClose(BackendHandler::window)) {
			glfwPollEvents();

			// Update the timing
			time.CurrentFrame = glfwGetTime();
			time.DeltaTime = static_cast<float>(time.CurrentFrame - time.LastFrame);

			time.DeltaTime = time.DeltaTime > 1.0f ? 1.0f : time.DeltaTime;

			// Update our FPS tracker data
			fpsBuffer[frameIx] = 1.0f / time.DeltaTime;
			frameIx++;
			if (frameIx >= 128)
				frameIx = 0;

			// We'll make sure our UI isn't focused before we start handling input for our game
			if (!ImGui::IsAnyWindowFocused()) {
				// We need to poll our key watchers so they can do their logic with the GLFW state
				// Note that since we want to make sure we don't copy our key handlers, we need a const
				// reference!
				for (const KeyPressWatcher& watcher : keyToggles) {
					watcher.Poll(BackendHandler::window);
				}
			}

			// Iterate over all the behaviour binding components
			scene->Registry().view<BehaviourBinding>().each([&](entt::entity entity, BehaviourBinding& binding) {
				// Iterate over all the behaviour scripts attached to the entity, and update them in sequence (if enabled)
				for (const auto& behaviour : binding.Behaviours) {
					if (behaviour->Enabled) {
						behaviour->Update(entt::handle(scene->Registry(), entity));
					}
				}
			});

			// Clear the screen
			testBuffer->Clear();

			glClearColor(0.08f, 0.17f, 0.31f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Update all world matrices for this frame
			scene->Registry().view<Transform>().each([](entt::entity entity, Transform& t) {
				t.UpdateWorldMatrix();
			});
			
			// Grab out camera info from the camera object
			Transform& camTransform = cameraObject.get<Transform>();
			glm::mat4 view = glm::inverse(camTransform.LocalTransform());
			glm::mat4 projection = cameraObject.get<Camera>().GetProjection();
			glm::mat4 viewProjection = projection * view;
						
			// Sort the renderers by shader and material, we will go for a minimizing context switches approach here,
			// but you could for instance sort front to back to optimize for fill rate if you have intensive fragment shaders
			renderGroup.sort<RendererComponent>([](const RendererComponent& l, const RendererComponent& r) {
				// Sort by render layer first, higher numbers get drawn last
				if (l.Material->RenderLayer < r.Material->RenderLayer) return true;
				if (l.Material->RenderLayer > r.Material->RenderLayer) return false;

				// Sort by shader pointer next (so materials using the same shader run sequentially where possible)
				if (l.Material->Shader < r.Material->Shader) return true;
				if (l.Material->Shader > r.Material->Shader) return false;

				// Sort by material pointer last (so we can minimize switching between materials)
				if (l.Material < r.Material) return true;
				if (l.Material > r.Material) return false;
				
				return false;
			});

			// Start by assuming no shader or material is applied
			Shader::sptr current = nullptr;
			ShaderMaterial::sptr currentMat = nullptr;

			testBuffer->Bind();

			// Iterate over the render group components and draw them
			renderGroup.each( [&](entt::entity e, RendererComponent& renderer, Transform& transform) {
				// If the shader has changed, set up it's uniforms
				if (current != renderer.Material->Shader) {
					current = renderer.Material->Shader;
					current->Bind();
					shader->SetUniform("u_usingLighting", (int)usingLightingOnly);
					shader->SetUniform("u_usingAmbient", (int)usingAmbient);
					shader->SetUniform("u_usingDiffuse", (int)usingDiffuse);
					shader->SetUniform("u_usingSpecular", (int)usingSpecular);
					shader->SetUniform("u_usingToon", (int)usingToon);
					BackendHandler::SetupShaderForFrame(current, view, projection);
				}
				// If the material has changed, apply it
				if (currentMat != renderer.Material) {
					currentMat = renderer.Material;
					currentMat->Apply();
				}
				// Render the mesh
				BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform);
			});

			testBuffer->Unbind();

			testBuffer->DrawToBackbuffer();

			// Draw our ImGui content
			BackendHandler::RenderImGui();

			scene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}

		// Nullify scene so that we can release references
		Application::Instance().ActiveScene = nullptr;
		BackendHandler::ShutdownImGui();
	}	

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}