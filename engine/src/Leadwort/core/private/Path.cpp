#include "../public/Path.h"
#include <Leadwort/utils/public/Logger.h>

namespace Leadwort::Core {

namespace fs = std::filesystem;

Path::Path(const std::string& path) {
	fs::path fsPath(path);

	if (fsPath.is_relative() && fsPath.has_root_directory()) {
		fsPath = fsPath.relative_path();
	}

	if (fs::exists(fsPath)) {
		m_Path = fs::canonical(fsPath);
	}
	else {
		m_Path = fs::absolute(fsPath).lexically_normal();
	}
}

bool Path::Exists() const {
	return fs::exists(m_Path);
}

Path Path::operator/(const Path& other) const {
	return Path((m_Path / other.m_Path).generic_string());
}

Path Path::operator/(const std::string& subPath) const {
	return Path((m_Path / subPath).generic_string());
}

} // namespace Engine::Core