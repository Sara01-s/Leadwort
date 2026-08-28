#include "../public/Entity.h"

#include <Leadwort/components/public/Transform.h>
#include <Leadwort/core/public/IScene.h>

#include <ranges>

namespace Leadwort::Core {

	Entity::Entity(const EntityID id, std::string name)
		: name(std::move(name)), m_Transform(AddComponent<Components::Transform>()), m_ID(id)
	{}

	Entity::~Entity() {
		// Components hand themselves to engine systems in OnAdded — a Behaviour to the
		// BehaviourSystem, and a Light on to the LightingSystem through OnDisable. Freeing
		// them without the matching OnRemoved leaves those systems iterating dangling
		// pointers on the next frame.
		for (const auto& component : m_OwnedComponents) {
			component->OnRemoved();
		}

		m_Components.clear();
	}

	Entity* Entity::FindEntityByTag(const std::string& t) const {
		LW_ASSERT(scene, "Entity::FindEntityByTag called on entity without a scene");

		for (const auto& entity: scene->GetEntityMap() | std::views::values) {
			if (entity->CompareTag(t)) {
			    return entity.get();
	        }
		}

		return nullptr;
	}

	Entity* Entity::CreateChild(const std::string& childName) const {
		Entity* child = scene->CreateEntity(childName);

		// A brand-new entity sits at the world origin with no parent, so AddChild would
		// dutifully "preserve" that world transform and bake inverse(parentWorld) into the
		// child's local one, pinning every child at the origin instead of at its parent.
		// A newly created child belongs in its parent's space.
		child->GetTransform().SetParentPreserveLocal(&GetTransform());

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

	    {
	       Json transformJson{};
	       transformJson["type"] = "Transform";

	       Json fieldsJson{};
	       GetTransform().Serialize(fieldsJson);
	       transformJson["fields"] = fieldsJson;

	       out["components"].push_back(transformJson);
	    }

	    for (const auto& component : m_Components | std::views::values) {
	       if (component->GetTypeName() == "Transform") {
	          continue;
	       }

	       Json componentJson{};
	       componentJson["type"] = component->GetTypeName();

	       Json fieldsJson{};
	       component->Serialize(fieldsJson);
	       componentJson["fields"] = fieldsJson;

	       out["components"].push_back(componentJson);
	    }
	}

	void Entity::Deserialize(const Json& in) {
	    if (in.is_null() || !in.is_object()) {
	       return;
	    }

	    name = in.value("name", DEFAULT_NAME);
	    tag  = in.value("tag", Tags::DEFAULT);
	    layerMask = in.value("layerMask", Utils::Layers::EVERYTHING);

	    if (in.contains("components") && in["components"].is_array()) {
	       for (const auto& compJson : in["components"]) {
	          if (compJson.is_null() || !compJson.contains("type")) {
	             continue;
	          }

	          const std::string typeName { compJson["type"] };

	          Components::Component* component{};
	          if (typeName == "Transform") {
	             component = &GetTransform();
	          }
	          else {
	             component = Components::ComponentRegistry::Get().CreateComponent(typeName, *this);
	          }

	          if (!component) {
	             continue;
	          }

	          if (compJson.contains("fields") && !compJson["fields"].is_null()) {
	             component->Deserialize(compJson["fields"]);
	          }
	       }
	    }
	}

} // namespace Leadwort::Core