#include "../public/Entity.h"

#include "engine/core/public/Scene.h"

#include <engine/components/public/Transform.h>
#include <ranges>

namespace Engine::Core {

Entity::Entity(const int id, std::string name)
	: id(id), name(std::move(name))
{
	transform = AddComponent<Components::Transform>();
}

Entity::~Entity() {
	m_Components.clear();
}

Entity* Entity::FindEntityByTag(const std::string& t) const {
	for (const auto& entity: scene->GetEntities() | std::views::values) {
		if (entity->CompareTag(t)) {
		    return entity;
        }
	}

	return nullptr;
}

Entity* Entity::CreateChild(const std::string& childName) const {
	Entity* child = scene->CreateEntity(childName);
	transform->AddChild(child->transform);

	return child;
}

} // namespace Engine::Core