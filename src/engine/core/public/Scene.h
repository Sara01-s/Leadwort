#pragma once

#include "Entity.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace Engine::Core {

class Scene {
public:
	Scene();
	virtual ~Scene();

	Scene& operator=(const Scene&) = delete;

	virtual void Create() = 0;

	[[nodiscard]] std::size_t GetEntityCount() const { return m_EntityMap.size(); }
	[[nodiscard]] Entity* GetRootEntity() const { return m_RootEntity.get(); }
	[[nodiscard]] Entity* GetEntity(int id) const;

	[[nodiscard]] const std::unordered_map<int, Unique<Entity>>& GetEntities() const { return m_EntityMap; }
	[[nodiscard]] const std::unordered_map<std::string, Entity*>& GetNamedRefs() const { return m_NamedRefs; }

	Entity* CreateEntity(const std::string& name = Entity::DEFAULT_NAME);

	static int GenerateNextEntityID();

protected:
	Entity* AddEntity(
		const std::string& name,
		const std::function<void(Entity*)>& configure = {}
	);

private:
	static int s_IdSequence;

	static void ResetSequence();

	Unique<Entity> m_RootEntity { nullptr };
	std::unordered_map<int, Unique<Entity>>  m_EntityMap{};
	std::unordered_map<std::string, Entity*> m_NamedRefs{};
};

} // namespace Engine::Core