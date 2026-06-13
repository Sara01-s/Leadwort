#include "../public/Scene.h"
#include <engine/components/public/Transform.h>
#include <stdexcept>

namespace Engine::Core {

int Scene::s_IdSequence = 0;

Scene::Scene() {
	m_RootEntity = CreateUnique<Entity>(GenerateNextEntityID(), "Root");
	m_RootEntity->scene = this;
}

Scene::~Scene() {
	m_EntityMap.clear();
	m_NamedRefs.clear();
	ResetSequence();
}

// ─────────────────────────────────────────────
//  Entity creation
// ─────────────────────────────────────────────

int Scene::GenerateNextEntityID() {
	return s_IdSequence++;
}

void Scene::ResetSequence() {
	s_IdSequence = 0;
}

Entity* Scene::CreateEntity(const std::string& name) {
	const int entityID = GenerateNextEntityID();
	auto entity = CreateUnique<Entity>(entityID, name);

	Entity* rawPtr = entity.get();
	rawPtr->scene = this;

	m_RootEntity->GetTransform().AddChild(rawPtr->GetTransform());
	m_EntityMap[entityID] = std::move(entity);

	if (name != Entity::DEFAULT_NAME) {
		m_NamedRefs[name] = rawPtr;
	}

	// ReSharper disable once CppDFALocalValueEscapesFunction
	return rawPtr;
}

Entity* Scene::AddEntity(
	const std::string& name,
	const std::function<void(Entity*)>& configure
) {
	auto* entity = CreateEntity(name);

	if (configure) {
		configure(entity);
	}

	// ReSharper disable once CppDFALocalValueEscapesFunction
	return entity;
}

Entity* Scene::GetEntity(const int id) const {
	const auto it = m_EntityMap.find(id);

	if (it == m_EntityMap.end()) {
		throw std::runtime_error("Entity with id " + std::to_string(id) + " not found");
	}

	return it->second.get();
}

} // namespace Engine::Core