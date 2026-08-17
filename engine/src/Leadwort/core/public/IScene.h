#pragma once

#include "Entity.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace Leadwort::Core {

	class IScene : public Serialization::ISerializable {
	public:
		IScene();
		~IScene() override;

		IScene& operator=(const IScene&) = delete;

		virtual void Create() = 0;

		[[nodiscard]] std::size_t GetEntityCount() const noexcept { return m_EntityMap.size(); }
		[[nodiscard]] Entity* GetRootEntity() const noexcept { return m_RootEntity.get(); }
		[[nodiscard]] Entity* GetEntity(EntityID entityID) const;

		[[nodiscard]] const std::unordered_map<int, Unique<Entity>>& GetEntityMap() const noexcept { return m_EntityMap; }
		[[nodiscard]] const std::unordered_map<std::string, Entity*>& GetNamedRefs() const noexcept { return m_NamedRefs; }

		Entity* CreateEntity(const std::string& name = Entity::DEFAULT_NAME);

		[[nodiscard]] int GenerateNextEntityID() noexcept;

		void InitComponents() const noexcept;

	public:
		void Serialize(Json& out) const override;
		void Deserialize(const Json& in) override;
		std::string_view GetTypeName() const override { return "Scene"; }

	protected:
		Entity* AddEntity(
			const std::string& name,
			const std::function<void(Entity*)>& configure = {}
		);

	private:
		void ResetSequence();

	private:
		int m_IdSequence { 0 };
		Unique<Entity> m_RootEntity { nullptr };
		std::unordered_map<int, Unique<Entity>>  m_EntityMap{};
		std::unordered_map<std::string, Entity*> m_NamedRefs{};
	};

} // namespace Engine::Core