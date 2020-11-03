#include "ECS.h"

entt::registry* ECS::m_registry = nullptr;

void ECS::AttachRegistry(entt::registry* reg)
{
    m_registry = reg;
}

void ECS::DettachRegistry()
{
    m_registry = nullptr;
}

unsigned ECS::CreateEntity(bool hasPhysics)
{
    unsigned entity = m_registry->create();

    if (hasPhysics)
        m_registry->assign<Transform>(entity);

    return entity;
}

void ECS::DestroyEntity(unsigned entity)
{
    m_registry->destroy(entity);
}
