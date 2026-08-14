#pragma once

#include <filesystem>
#include <string>

namespace Leadwort::Core {

	class Path {
	public:
		explicit Path(const std::string& path);

		[[nodiscard]] bool Exists() const;
		[[nodiscard]] std::filesystem::path GetRawPath() const { return m_Path; }

		[[nodiscard]] std::string GetGenericString() const { return m_Path.generic_string(); }
		[[nodiscard]] std::wstring GetWString() const { return m_Path.wstring(); }
		[[nodiscard]] std::u8string GetU8String() const { return m_Path.generic_u8string(); }

		Path operator/(const Path& other) const;
		Path operator/(const std::string& subPath) const;

	private:
		std::filesystem::path m_Path{};
	};

} // namespace Engine::Core