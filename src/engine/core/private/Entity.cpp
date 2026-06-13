#include "../public/Entity.h"

#include "engine/core/public/Scene.h"

#include <engine/components/public/Transform.h>
#include <ranges>

namespace Engine::Core {

Entity::Entity(const int id, std::string name)
	: name(std::move(name)), m_Transform(AddComponent<Components::Transform>()), m_ID(id)
{}

Entity::~Entity() {
	m_Components.clear();
}

Entity* Entity::FindEntityByTag(const std::string& t) const {
	for (const auto& entity: scene->GetEntities() | std::views::values) {
		if (entity->CompareTag(t)) {
		    return entity.get();
        }
	}

	return nullptr;
}

Entity* Entity::CreateChild(const std::string& childName) const {
	Entity* child = scene->CreateEntity(childName);
	GetTransform().AddChild(child->GetTransform());

	return child;
}

} // namespace Engine::Core