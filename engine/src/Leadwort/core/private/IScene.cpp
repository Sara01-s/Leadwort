#include "Leadwort/core/public/IScene.h"

#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/components/public/Transform.h>
#include <Leadwort/rendering/public/Model.h>
#include <stdexcept>
#include <unordered_set>
#include <vector>

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
		if (!in.contains("entities") || !in["entities"].is_array()) {
			return;
		}

		// Keep every referenced model alive for the whole deserialization pass.
		// The model cache holds only weak refs, so without this pin each model is
		// dropped the moment its owning MeshRenderer::Deserialize returns, forcing
		// a full re-parse of the source file for every entity that references it.
		std::vector<Shared<Model>> pinnedModels{};
		{
			std::unordered_set<std::string> seenModelPaths{};

			for (const auto& entityJson : in["entities"]) {
				if (!entityJson.contains("components") || !entityJson["components"].is_array()) {
					continue;
				}

				for (const auto& compJson : entityJson["components"]) {
					if (!compJson.contains("fields") || !compJson["fields"].is_object()) {
						continue;
					}

					const std::string modelPath { compJson["fields"].value("modelPath", std::string{}) };

					if (modelPath.empty() || !seenModelPaths.insert(modelPath).second) {
						continue;
					}

					if (auto model { AssetManagement::EngineAssets::GetModel(modelPath) }) {
						pinnedModels.push_back(std::move(model));
					}
				}
			}
		}

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