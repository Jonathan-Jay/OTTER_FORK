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

#include "Graphics/Post/GreyscaleEffect.h"
#include "Graphics/Post/SepiaEffect.h"
#include "Graphics/Post/BloomEffect.h"

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
		bool usingTextures = true;

		bool lightCamera = true;

		int maxEffectCount = 5;

		// These are our application / scene level uniforms that don't necessarily update
		// every frame
		shader->SetUniform("u_LightPos", lightPos);
		shader->SetUniform("u_LightCol", lightCol);
		shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
		shader->SetUniform("u_AmbientLightStrength", lightAmbientPow);
		shader->SetUniform("u_AmbientCol", ambientCol);
		shader->SetUniform("u_AmbientStrength", ambientPow);
		shader->SetUniform("u_LightAttenuationConstant", 1.0f);
		shader->SetUniform("u_LightAttenuationLinear", lightLinearFalloff);
		shader->SetUniform("u_LightAttenuationQuadratic", lightQuadraticFalloff);

		// We'll add some ImGui controls to control our shader
		BackendHandler::imGuiCallbacks.push_back([&]() {
			if (ImGui::CollapsingHeader("not my stuff lol")) {
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
			}

			if (ImGui::CollapsingHeader("Assignment toggles"))
			{
				if (ImGui::TreeNode("Templates")) {
					if (ImGui::Button("1: No Lighting")) {
						usingLightingOnly = true;
						usingAmbient = false;
						usingDiffuse = true;
						usingSpecular = false;
					}
					if (ImGui::Button("2: Only Ambient")) {
						usingLightingOnly = false;
						usingAmbient = true;
						usingDiffuse = false;
						usingSpecular = false;
					}
					if (ImGui::Button("3: Only Specular")) {
						usingLightingOnly = false;
						usingAmbient = false;
						usingDiffuse = false;
						usingSpecular = true;
					}
					if (ImGui::Button("4: Ambient + Specular + diffuse aka normal")) {
						usingLightingOnly = false;
						usingAmbient = true;
						usingDiffuse = true;
						usingSpecular = true;
					}
					ImGui::Text("Bloom can be added via frame effects options");
					ImGui::Checkbox("6: texures", &usingTextures);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Individual toggles")) {
					ImGui::Text("Lighting only overwrites everything else");
					ImGui::Checkbox("Lighting Only", &usingLightingOnly);
					ImGui::Checkbox("Ambient", &usingAmbient);
					ImGui::Checkbox("Diffuse", &usingDiffuse);
					ImGui::Checkbox("Specular", &usingSpecular);
					ImGui::Checkbox("Textures", &usingTextures);

					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Things that help")) {
					ImGui::Checkbox("Light Following Camera", &lightCamera);
					if (!lightCamera) {
						if (ImGui::SliderFloat3("Light Pos", glm::value_ptr(lightPos), -12.f, 12.f)) {
							shader->SetUniform("u_LightPos", lightPos);
						}
					}
					if (ImGui::SliderFloat("Light Power", &lightSpecularPow, 0.0f, 5.0f)) {
						shader->SetUniform("u_SpecularLightStrength", lightSpecularPow);
					}

					ImGui::TreePop();
				}
			}
			
			if (ImGui::CollapsingHeader("Post Processing Effects"))
			{
				ImGui::Text(("Number of effects: " + std::to_string(FrameEffects::Constant->size())).c_str());
				ImGui::SliderInt("max amt of effects", &maxEffectCount, 0, 5);
				for (int i(0); i < FrameEffects::Constant->size(); ++i) {
					std::string name = (*FrameEffects::Constant)[i]->Info();
					if (ImGui::TreeNode((std::to_string(i + 1) + ": " + name).c_str())) {
						if (name == "Bloom") {
							BloomEffect* effect = (BloomEffect*)((*FrameEffects::Constant)[i]);
							float threshold = effect->GetTreshold();
							if (ImGui::SliderFloat("Treshold", &threshold, 0.f, 1.f)) {
								effect->SetTreshold(threshold);
							}
							float radius = effect->GetRadius();
							if (ImGui::SliderFloat("Radius", &radius, 0.f, 15.f)) {
								effect->SetRadius(radius);
							}
							int blur = effect->GetBlurCount();
							if (ImGui::SliderInt("Blur Passes", &blur, 1, 15)) {
								effect->SetBlurCount(blur);
							}
						}
						else if (name == "Greyscale") {
							GreyscaleEffect* effect = (GreyscaleEffect*)((*FrameEffects::Constant)[i]);
							float intensity = effect->GetIntensity();
							if (ImGui::SliderFloat("Intensity", &intensity, 0.f, 1.f)) {
								effect->SetIntensity(intensity);
							}
						}
						else if (name == "Sepia") {
							SepiaEffect* effect = (SepiaEffect*)((*FrameEffects::Constant)[i]);
							float intensity = effect->GetIntensity();
							if (ImGui::SliderFloat("Intensity", &intensity, 0.f, 1.f)) {
								effect->SetIntensity(intensity);
							}
						}
						else if (name == "Toon") {
							ImGui::Text("Toon bands based on average of rgb");
							ToonEffect* effect = (ToonEffect*)((*FrameEffects::Constant)[i]);
							int bands = effect->GetBands();
							if (ImGui::SliderInt("Bands", &bands, 3, 15)) {
								effect->SetBands(bands);
							}
						}
						else if (name == "Pixel") {
							ImGui::Text("Pixels done by drawing to a smaller buffer, which means no new shader!");
							PixelEffect* effect = (PixelEffect*)((*FrameEffects::Constant)[i]);
							int pixels = effect->GetPixelCount();
							if (ImGui::SliderInt("PixelCount", &pixels, 4, BackendHandler::height)) {
								effect->SetPixelCount(pixels);
							}
						}
						if (ImGui::Button("Remove")) {
							FrameEffects::Constant->RemoveEffect(i);
							//ImGui::SetNextItemOpen(false);
						}
						ImGui::TreePop();
					}
				}
				if (FrameEffects::Constant->size() < maxEffectCount) {
					if (ImGui::Button("Greyscale")) {
						GreyscaleEffect* effect = new GreyscaleEffect();
						effect->Init(BackendHandler::width, BackendHandler::height);
						effect->SetInfo("Greyscale");
						FrameEffects::Constant->AddEffect(effect);
						effect = nullptr;
					}
					ImGui::SameLine();
					if (ImGui::Button("Sepia")) {
						SepiaEffect* effect = new SepiaEffect();
						effect->Init(BackendHandler::width, BackendHandler::height);
						effect->SetInfo("Sepia");
						FrameEffects::Constant->AddEffect(effect);
						effect = nullptr;
					}
					ImGui::SameLine();
					if (ImGui::Button("Bloom")) {
						BloomEffect* effect = new BloomEffect();
						effect->Init(BackendHandler::width, BackendHandler::height);
						effect->SetInfo("Bloom");
						FrameEffects::Constant->AddEffect(effect);
						effect = nullptr;
					}
					ImGui::SameLine();
					if (ImGui::Button("\"Toon Shading\"")) {
						ToonEffect* effect = new ToonEffect();
						effect->Init(BackendHandler::width, BackendHandler::height);
						effect->SetInfo("Toon");
						FrameEffects::Constant->AddEffect(effect);
						effect = nullptr;
					}
					ImGui::SameLine();
					if (ImGui::Button("Pixelataion")) {
						PixelEffect* effect = new PixelEffect();
						effect->Init(BackendHandler::width, BackendHandler::height);
						effect->SetInfo("Pixel");
						FrameEffects::Constant->AddEffect(effect);
						effect = nullptr;
					}
				}
				else {
					ImGui::Text("Max effects added");
				}
			}

			});

		#pragma endregion 

		// GL states
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); // New 

		#pragma region TEXTURE LOADING


		//All textures done by me


		// Load some textures from files
		Texture2D::sptr allTextures = Texture2D::LoadFromFile("images/alltextures.png");

		// Load the cube map
		//TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/sample.jpg");
		TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/ToonSky.jpg"); 

		// Creating an empty texture
		Texture2DDescription desc = Texture2DDescription();
		desc.Width = 1;
		desc.Height = 1;
		desc.Format = InternalFormat::RGB8;
		Texture2D::sptr blankTexture = Texture2D::Create(desc);
		// Clear it with a white colour
		blankTexture->Clear();

		#pragma endregion

		///////////////////////////////////// Scene Generation //////////////////////////////////////////////////
		#pragma region Scene Generation
		
		FrameEffects::Constant = new FrameEffects();
		FrameEffects::Constant->Init(BackendHandler::width, BackendHandler::height);

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
		floorMat->Set("s_Diffuse", blankTexture);
		floorMat->Set("s_Specular", blankTexture);
		floorMat->Set("u_Shininess", 1.0f);
		floorMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr texturedMat = ShaderMaterial::Create();  
		texturedMat->Shader = shader;
		texturedMat->Set("s_Diffuse", allTextures);
		texturedMat->Set("s_Specular", blankTexture);
		texturedMat->Set("u_Shininess", 1.0f);
		texturedMat->Set("u_TextureMix", 0.0f);


		GameObject obj1 = scene->CreateEntity("Ground"); 
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj", glm::vec4(0.5f, 0.5f, 0.5f, 1.f));
			obj1.emplace<RendererComponent>().SetMesh(vao).SetMaterial(floorMat);
			obj1.get<Transform>().SetLocalPosition(0.0f, 0.f, -0.4f);
		}

		GameObject obj2 = scene->CreateEntity("Train station");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/station.obj");
			obj2.emplace<RendererComponent>().SetMesh(vao).SetMaterial(texturedMat);
			obj2.get<Transform>().SetLocalPosition(-3.25f, 0.0f, 0.0f);
		}

		VertexArrayObject::sptr trainVao = ObjLoader::LoadFromFile("models/train.obj");
		GameObject obj3 = scene->CreateEntity("Train1");
		{
			obj3.emplace<RendererComponent>().SetMesh(trainVao).SetMaterial(texturedMat);
			obj3.get<Transform>().SetLocalPosition(-8.0f, 6.0f, 0.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj3);
			path->Points.push_back({ glm::vec3(-8, -8, 0), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(8, -8, 0), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(8, 8, 0), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(-8, 8, 0), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) });
			path->Speed = 4.f;
		}
		GameObject obj4 = scene->CreateEntity("Train2");
		{
			obj4.emplace<RendererComponent>().SetMesh(trainVao).SetMaterial(texturedMat);
			obj4.get<Transform>().SetLocalPosition(-8.0f, 2.0f, 0.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj4);
			path->Points.push_back({ glm::vec3(-8, -8, 0), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(8, -8, 0), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(8, 8, 0), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(-8, 8, 0), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) });
			path->Speed = 4.f;
		}
		GameObject obj5 = scene->CreateEntity("Train3");
		{
			obj5.emplace<RendererComponent>().SetMesh(trainVao).SetMaterial(texturedMat);
			obj5.get<Transform>().SetLocalPosition(-8.0f, -2.0f, 0.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj5);
			path->Points.push_back({ glm::vec3(-8, -8, 0), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(8, -8, 0), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(8, 8, 0), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(-8, 8, 0), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) });
			path->Speed = 4.f;
		}
		GameObject obj6 = scene->CreateEntity("Train4");
		{
			obj6.emplace<RendererComponent>().SetMesh(trainVao).SetMaterial(texturedMat);
			obj6.get<Transform>().SetLocalPosition(-8.0f, -6.0f, 0.f);

			auto path = BehaviourBinding::Bind<FollowPathBehaviour>(obj6);
			path->Points.push_back({ glm::vec3(-8, -8, 0), glm::angleAxis(glm::radians(180.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(8, -8, 0), glm::angleAxis(glm::radians(270.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(8, 8, 0), glm::angleAxis(glm::radians(0.f), glm::vec3(0, 0, 1)) });
			path->Points.push_back({ glm::vec3(-8, 8, 0), glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1)) });
			path->Speed = 4.f;
		}

		// Create an object to be our camera
		GameObject cameraObject = scene->CreateEntity("Camera");
		{
			cameraObject.get<Transform>().SetLocalPosition(-8, 0, 1.5f).LookAt(glm::vec3(0, 0, 1.5f));

			// We'll make our camera a component of the camera object
			Camera& camera = cameraObject.emplace<Camera>();// Camera::Create();
			camera.SetPosition(glm::vec3(-8, 0, 1.5f));
			camera.SetUp(glm::vec3(0, 0, 1));
			camera.LookAt(glm::vec3(0, 0, 1.5f));
			camera.SetFovDegrees(90.0f); // Set an initial FOV
			camera.SetOrthoHeight(3.0f);
			BehaviourBinding::Bind<CameraControlBehaviour>(cameraObject)->_moveSpeed = 3.f;
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
			FrameEffects::Constant->Clear();

			glClearColor(0.08f, 0.17f, 0.31f, 1.0f);
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

			FrameEffects::Constant->Bind();

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
					shader->SetUniform("u_usingTextures", (int)usingTextures);
					if (lightCamera)
						shader->SetUniform("u_LightPos", lightPos = camTransform.GetLocalPosition());
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

			FrameEffects::Constant->UnBind();

			FrameEffects::Constant->Draw();

			// Draw our ImGui content
			BackendHandler::RenderImGui();

			scene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}

		FrameEffects::Constant->RemoveAllEffects();
		delete FrameEffects::Constant;
		FrameEffects::Constant = nullptr;

		// Nullify scene so that we can release references
		Application::Instance().ActiveScene = nullptr;
		BackendHandler::ShutdownImGui();
		FrameEffects::Unload();
	}	

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}