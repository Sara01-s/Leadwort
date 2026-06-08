#include "../public/Scene.h"
#include <engine/components/public/Transform.h>
#include <stdexcept>

namespace Engine::Core {

int Scene::s_IdSequence = 0;

Scene::Scene() {
	m_RootEntity = new Entity(GenerateNextId(), "Root");
	m_RootEntity->scene = this;
	m_EntityMap[m_RootEntity->id] = m_RootEntity;
}

Scene::~Scene() {
	m_EntityMap.clear();
	m_NamedRefs.clear();
	delete m_RootEntity;
	ResetSequence();
}

// ─────────────────────────────────────────────
//  Entity creation
// ─────────────────────────────────────────────

int Scene::GenerateNextId() {
	return s_IdSequence++;
}

void Scene::ResetSequence() {
	s_IdSequence = 0;
}

Entity* Scene::CreateEntity(const std::string& name) {
	const int id = GenerateNextId();
	auto* entity = new Entity(id, name);
	entity->scene = this;

	m_RootEntity->transform->AddChild(entity->transform);
	m_EntityMap[id] = entity;

	if (name != Entity::DEFAULT_NAME) {
		m_NamedRefs[name] = entity;
	}

	return entity;
}

Entity* Scene::AddEntity(
	const std::string& name,
	const std::function<void(Entity*)>& configure
) {
	auto* entity = CreateEntity(name);

	if (configure) {
		configure(entity);
	}

	return entity;
}

Entity* Scene::GetEntity(const int id) const {
	const auto it = m_EntityMap.find(id);

	if (it == m_EntityMap.end()) {
		throw std::runtime_error("Entity with id " + std::to_string(id) + " not found");
	}

	return it->second;
}

} // namespace Engine::Core