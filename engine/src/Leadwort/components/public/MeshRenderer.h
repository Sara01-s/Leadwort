#pragma once

#include "Leadwort/asset-management/public/AssetDatabase.h"
#include "Leadwort/components/ComponentRegistry.h"

#include <Leadwort/components/public/IRenderer.h>
#include <Leadwort/rendering/bindables/public/Mesh.h>
#include <Leadwort/systems/public/RenderSystem.h>
#include <Leadwort/utils/public/PrimitiveMeshes.h>

namespace Leadwort::Components { class Camera; }
namespace Leadwort::Components::Behaviours { class DirectionalLight; }

namespace Leadwort::Components {

	class MeshRenderer final : public IRenderer {
	public:
		using Mesh = Rendering::Bindables::Mesh;

		LW_REFLECT(MeshRenderer,
			LW_FIELD(AssetRef, mesh, "Mesh")
		)
	public:
		Shared<Mesh> mesh { Utils::PrimitiveMeshes::Get().Cube() };
		std::string modelPath {};
		int meshIndex { -1 };
		std::string primitiveType { "cube" };
		bool isPrimitive { true };

		void Serialize(Json& out) const override {
			out["isPrimitive"] = isPrimitive;

			if (isPrimitive) {
				out["primitiveType"] = primitiveType;
			}
			else {
				out["modelPath"] = modelPath;
				out["meshIndex"] = meshIndex;
			}
		}

		void Deserialize(const Json& in) override {
			isPrimitive = in.value("isPrimitive", true);

			if (isPrimitive) {
				primitiveType = in.value("primitiveType", "cube");
				if (primitiveType == "cube") {
					mesh = Utils::PrimitiveMeshes::Get().Cube();
				}
				// TODO: Add other primitives later
			}
			else {
				modelPath = in.value("modelPath", "");
				meshIndex = in.value("meshIndex", -1);

				if (!modelPath.empty() && meshIndex >= 0) {
					const auto model = AssetManagement::EngineAssets::GetModel(modelPath);
					if (model && meshIndex < model->GetMeshCount()) {
						mesh = model->GetMesh(meshIndex);
					}
				}
			}
		}

	public:
		void EmitDrawCommand(Rendering::DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const override;
		[[nodiscard]] std::optional<AABB> GetAABB() const override { return mesh->GetAABB(); }
	};

	LW_REGISTER_COMPONENT(MeshRenderer)
} // namespace Engine::Components