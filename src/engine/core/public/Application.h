#pragma once

#include "engine/utils/public/Singleton.h"
#include <filesystem>

namespace Engine::Core {

class Application : public Utils::Singleton<Application> {
	friend class Singleton;

public:
	[[nodiscard]] const std::filesystem::path& GetRootPath()           const { return m_RootPath; }
	[[nodiscard]] const std::filesystem::path& GetAssetsPath()         const { return m_AssetsPath; }
	[[nodiscard]] const std::filesystem::path& GetPersistentDataPath() const { return m_PersistentDataPath; }
	[[nodiscard]] const std::filesystem::path& GetTempPath()           const { return m_TempPath; }

private:
	Application();

	std::filesystem::path m_RootPath;
	std::filesystem::path m_AssetsPath;
	std::filesystem::path m_PersistentDataPath;
	std::filesystem::path m_TempPath;
};

} // namespace Engine