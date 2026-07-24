#include "../public/Entity.h"

#include <Leadwort/core/public/Scene.h>
#include <Leadwort/components/public/Transform.h>

#include <ranges>

namespace Leadwort::Core {

Entity::Entity(const int id, std::string name)
	: name(std::move(name)), m_Transform(AddComponent<Components::Transform>()), m_ID(id)
{}

Entity::~Entity() {
	m_Components.clear();
}

Entity* Entity::FindEntityByTag(const std::string& t) const {
	for (const auto& entity: scene->GetEntityMap() | std::views::values) {
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

void Entity::Serialize(Json& out) const {
	out["id"] = m_ID;
	out["name"] = name;
	out["tag"] = tag;
	out["layerMask"] = layerMask;

	if (const Components::Transform* parentTransform = GetTransform().GetParent()) {
		out["parentId"] = parentTransform->GetEntity().GetID();
	}

	out["components"] = Json::array();
	for (const auto& component : m_Components | std::views::values) {
		Json componentJson{};
		componentJson["type"] = component->GetTypeName();

		Json fieldsJson{};
		component->Serialize(fieldsJson);
		componentJson["fields"] = fieldsJson;

		out["components"].push_back(componentJson);
	}
}

void Entity::Deserialize(const Json& in) {
	name = in.value("name", DEFAULT_NAME);
	tag  = in.value("tag", Tags::DEFAULT);
	layerMask = in.value("layerMask", Utils::Layers::EVERYTHING);

	for (const auto& compJson : in["components"]) {
		const std::string typeName = compJson["type"];

		Components::Component* comp{};
		if (typeName == "Transform") {
			comp = &GetTransform();
		}
		else {
			comp = Components::ComponentRegistry::Get().CreateComponent(typeName, *this);
		}

		comp->Deserialize(compJson["fields"]);
	}
}

} // namespace Leadwort::Core