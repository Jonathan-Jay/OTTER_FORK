#pragma once
#include "dependencies/entt.hpp"
#include "Transform.h"

class ECS abstract
{
public:
	/////	Managing the class	/////

	//Let the class edit the registry
	static void AttachRegistry(entt::registry* reg);

	//Make this class not attached to anything
	static void DettachRegistry();

	/////	Entity management	/////

	//Create an Entity and return the index, defaults with transform component
	static unsigned CreateEntity(bool hasTransform = true);

	//Delete an entity
	static void DestroyEntity(unsigned entity);

	//Attach a class (use the overload plz)
	template<typename T>
	static T& AttachComponent(unsigned entity);

	//Attach a class with existing data (use this)
	template<typename T>
	static T& AttachComponent(unsigned entity, T object);

	//Remove a class from an entity
	template<typename T>
	static void RemoveComponent(unsigned entity);

	//Get the class of an entity
	template<typename T>
	static T& GetComponent(unsigned entity);

private:
	static entt::registry* m_registry;
};

template<typename T>
inline T& ECS::AttachComponent(unsigned entity)
{
	m_registry->assign<T>(entity);

	return m_registry->get<T>(entity);
}

template<typename T>
inline T& ECS::AttachComponent(unsigned entity, T object)
{
	m_registry->assign<T>(entity);

	return (m_registry->get<T>(entity) = object);
}

template<typename T>
inline void ECS::RemoveComponent(unsigned entity)
{
	m_registry->remove<T>(entity);
}

template<typename T>
inline T& ECS::GetComponent(unsigned entity)
{
	return m_registry->get<T>(entity);
}
