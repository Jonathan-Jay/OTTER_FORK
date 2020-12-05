#include "Scene.h"

Scene::Scene(const std::string& name)
    : m_name(name)
{
}

Scene::~Scene()
{
}

entt::registry* Scene::GetRegistry()
{
    return &m_reg;
}

std::string Scene::GetName()
{
    return m_name;
}

void Scene::Init(int windowWidth, int windowHeight)
{
    //generic init, use as guidelines
    ECS::AttachRegistry(&m_reg);

    // for multi cam setup, change the m_camCount variable, and also spawn in reverse order (aka player 1 last)
    unsigned cameraEnt = ECS::CreateEntity();
    auto& camCam = ECS::AttachComponent<Camera>(cameraEnt);
    camCam.SetFovDegrees(60.f).ResizeWindow(windowWidth, windowHeight);
}

Scene* Scene::Reattach()
{
    ECS::AttachRegistry(&m_reg);

    m_lastClock = glfwGetTime();

    return this;
}

void Scene::BackEndUpdate()
{
    m_dt = glfwGetTime() - m_lastClock;
    m_lastClock = glfwGetTime();

    //always render
    if (m_camCount < 1)
        m_camCount = 1;
    else if (m_camCount > 4)
        m_camCount = 4;

    Collisions::Update(&m_reg, m_dt);
    Rendering::Update(&m_reg, m_camCount, m_dt);
}
