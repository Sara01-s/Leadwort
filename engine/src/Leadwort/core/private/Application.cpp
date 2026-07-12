#include "../public/Application.h"
#include <Leadwort/utils/public/Logger.h>

namespace Leadwort::Core {

Application::Application() {
	m_RootPath = std::filesystem::current_path();
	m_AssetsPath = m_RootPath / "assets";
	m_PersistentDataPath = m_RootPath / "persistentData";
	m_TempPath = m_RootPath / ".temp";

	LW_LOG("Application root path: ", m_RootPath.string());

	if (!std::filesystem::exists(m_AssetsPath)) {
		LW_ERROR("Assets folder not found: ", m_AssetsPath.string());
		LW_WARN("Create an 'assets' folder in the project root.");
	}
	else {
		LW_LOG("Assets folder found: ", m_AssetsPath.string());
	}

	std::filesystem::create_directories(m_PersistentDataPath);
	std::filesystem::create_directories(m_TempPath);
}

} // namespace Leadwort::Core