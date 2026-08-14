#include "../public/Scene.h"
#include <Leadwort/components/public/Transform.h>
#include <stdexcept>

namespace Leadwort::Core {

	int Scene::s_IdSequence { 0 };

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

	void Scene::Serialize(Json& out) const {
		out["entities"] = Json::array();

		for (const auto& entity: m_EntityMap | std::views::values) {
			Json entityJson{};
			entity->Serialize(entityJson);
			out["entities"].push_back(entityJson);
		}
	}

	void Scene::Deserialize(const Json& in) {
		std::unordered_map<int, Entity*> idRemap;

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

			const Entity* self   { idRemap.at(entityJson["id"]) };
			const Entity* parent { idRemap.at(entityJson["parentId"]) };

			self->GetTransform().SetParent(&parent->GetTransform());
		}
	}

	void Scene::ResetSequence() {
		s_IdSequence = 0;
	}

	Entity* Scene::CreateEntity(const std::string& name) {
		const int entityID { GenerateNextEntityID() };
		auto entity { CreateUnique<Entity>(entityID, name) };

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
		auto* entity { CreateEntity(name) };

		if (configure) {
			configure(entity);
		}

		// ReSharper disable once CppDFALocalValueEscapesFunction
		return entity;
	}

	Entity* Scene::GetEntity(const EntityID entityID) const {
		const auto it { m_EntityMap.find(entityID) };

		if (it == m_EntityMap.end()) {
			throw std::runtime_error("Entity with id " + std::to_string(entityID) + " not found");
		}

		return it->second.get();
	}

} // namespace Engine::Core