// SceneSerializer.h
#pragma once
#include <Leadwort/core/public/IScene.h>
#include <filesystem>

namespace Leadwort::Serialization {

	class SceneSerializer {
	public:
		static bool SaveToFile(const Core::IScene& scene, const std::filesystem::path& path) {
			Json out{};
			scene.Serialize(out);

			std::ofstream file(path);

			if (!file.is_open()) {
				LW_ERROR("SceneSerializer: Could not open a file by writing: ", path.string());
				return false;
			}

			LW_LOG("SceneSerializer: Saved scene to file: ", path.string());

			file << out.dump(2);
			return true;
		}

		static bool LoadFromFile(Core::IScene& scene, const std::filesystem::path& path) {
			if (!std::filesystem::exists(path)) {
				LW_ERROR("SceneSerializer: File does not exist: ", path.string());
				return false;
			}

			std::ifstream file(path);
			Json in{};

			try {
				file >> in;
			}
			catch (const std::exception& e) {
				LW_ERROR("SceneSerializer: Failed to parse JSON: ", e.what());
				return false;
			}

			scene.Deserialize(in);
			return true;
		}
	};

} // namespace Leadwort::Core