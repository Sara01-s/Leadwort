#include "Leadwort/core/public/IScene.h"

#include <Leadwort/components/public/Transform.h>
#include <stdexcept>

namespace Leadwort::Core {

    IScene::IScene() {
		m_RootEntity = CreateUnique<Entity>(GenerateNextEntityID(), "Root");
		m_RootEntity->scene = this;
    }

    IScene::~IScene() {
		LW_LOG("Scene Closed, there was ", m_EntityMap.size(), " entities in the scene");
		m_EntityMap.clear();
		m_NamedRefs.clear();
    }

    int IScene::GenerateNextEntityID() noexcept {
		return m_IdSequence++;
    }

	void IScene::InitComponents() const noexcept {
	    for (const auto& entity: m_EntityMap | std::views::values) {
		    for (const auto& component : entity->GetAllComponents()) {
			    component->OnAdded();
		    }
	    }
    }

	void IScene::Serialize(Json& out) const {
		out["entities"] = Json::array();

		for (const auto& entity: m_EntityMap | std::views::values) {
			Json entityJson{};
			entity->Serialize(entityJson);
			out["entities"].push_back(entityJson);
		}
    }

    void IScene::Deserialize(const Json& in) {
		std::unordered_map<int, Entity*> idRemap{};

		idRemap[m_RootEntity->GetID()] = m_RootEntity.get();

		for (const auto& entityJson : in["entities"]) {
			const std::string entityName { entityJson.value("name", Entity::DEFAULT_NAME) };
			Entity* entity { CreateEntity(entityName) };

			entity->Deserialize(entityJson);

			idRemap[entityJson["id"]] = entity;
		}

		for (const auto& entityJson : in["entities"]) {
			if (!entityJson.contains("parentId")) {
			 continue;
			}

			const Entity* self { idRemap.at(entityJson["id"]) };

			const int parentIdFromFile { entityJson["parentId"] };
			const auto parentIt { idRemap.find(parentIdFromFile) };

			if (parentIt == idRemap.end()) {
			 continue;
			}

			const Entity* parent { parentIt->second };
			self->GetTransform().SetParentPreserveLocal(&parent->GetTransform());
		}
    }

	void IScene::ResetSequence() {
		m_IdSequence = 0;
	}

    Entity* IScene::CreateEntity(const std::string& name) {
		const int entityID { GenerateNextEntityID() };
		auto entity { CreateUnique<Entity>(entityID, name) };

		Entity* rawPtr = entity.get();
		rawPtr->scene = this;

		m_RootEntity->GetTransform().AddChild(rawPtr->GetTransform());
		m_EntityMap[entityID] = std::move(entity);

		if (name != Entity::DEFAULT_NAME) {
			m_NamedRefs[name] = rawPtr;
		}

		return rawPtr;
    }

    Entity* IScene::AddEntity(
       const std::string& name,
       const std::function<void(Entity*)>& configure
    ) {
		auto* entity { CreateEntity(name) };

		if (configure) {
			configure(entity);
		}

		return entity;
    }

    Entity* IScene::GetEntity(const EntityID entityID) const {
		const auto it { m_EntityMap.find(entityID) };

		if (it == m_EntityMap.end()) {
		  throw std::runtime_error("Entity with id " + std::to_string(entityID) + " not found");
		}

		return it->second.get();
    }

} // namespace Leadwort::Core