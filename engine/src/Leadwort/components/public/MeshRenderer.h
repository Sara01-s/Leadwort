#pragma once

#include "Leadwort/asset-management/public/AssetDatabase.h"
#include "Leadwort/components/ComponentRegistry.h"
#include "Leadwort/serialization/AssetSerializer.h"

#include <Leadwort/components/public/IRenderer.h>
#include <Leadwort/rendering/bindables/public/Mesh.h>
#include <Leadwort/systems/public/RenderSystem.h>
#include <Leadwort/utils/public/PrimitiveMeshes.h>

namespace Leadwort::Components { class Camera; }
namespace Leadwort::Components::Behaviours { class Light; }

namespace Leadwort::Components {

	class MeshRenderer final : public IRenderer {
	public:
		using Mesh = Rendering::Bindables::Mesh;

		LW_REFLECT(MeshRenderer,
			LW_FIELD(AssetRef, mesh, "Mesh"),
			LW_FIELD(Bool, castShadows, "Cast Shadows")
		)
	public:
		Shared<Mesh> mesh { Utils::PrimitiveMeshes::Get().Cube() };
		bool castShadows { true };

		std::string modelPath {};
		int meshIndex { -1 };
		std::string primitiveType { "cube" };
		bool isPrimitive { true };

		float planeSizeX     { 1.0f };
		float planeSizeZ     { 1.0f };
		int   planeSegmentsX { 1 };
		int   planeSegmentsZ { 1 };

		// Builds a subdivided-plane primitive and tags the renderer so that the
		// mesh identity survives serialization.
		void SetSubdividedPlane(const float sizeX, const float sizeZ, const int segmentsX, const int segmentsZ) {
			isPrimitive = true;
			primitiveType = "subdivided_plane";
			planeSizeX = sizeX;
			planeSizeZ = sizeZ;
			planeSegmentsX = segmentsX;
			planeSegmentsZ = segmentsZ;
			mesh = Utils::PrimitiveMeshes::Get().SubdividedPlane(sizeX, sizeZ, segmentsX, segmentsZ);
		}

		// The material carries the queue (glTF alphaMode), but the scene collector reads
		// the renderer's own field, so it has to be pulled across whenever the mesh
		// changes: on import and after deserializing.
		void SyncRenderQueueFromMaterial() noexcept {
			if (mesh && mesh->GetMaterial()) {
				renderQueue = mesh->GetMaterial()->renderQueue;
			}
		}

		void Serialize(Json& out) const override {
			out["isPrimitive"] = isPrimitive;
			out["castShadows"] = castShadows;

			if (isPrimitive) {
				out["primitiveType"] = primitiveType;

				if (primitiveType == "subdivided_plane") {
					out["planeSizeX"] = planeSizeX;
					out["planeSizeZ"] = planeSizeZ;
					out["planeSegmentsX"] = planeSegmentsX;
					out["planeSegmentsZ"] = planeSegmentsZ;
				}
			}
			else {
				out["modelPath"] = modelPath;
				out["meshIndex"] = meshIndex;
			}

			// Only primitives carry a scene-authored material override. Model meshes
			// own their material (potentially with embedded textures) and must not be
			// rebuilt from a lossy JSON round-trip.
			if (isPrimitive && mesh && mesh->GetMaterial()) {
				out["material"] = AssetManagement::AssetSerializer<Rendering::Bindables::Material>::Serialize(*mesh->GetMaterial());
			}
		}

		void Deserialize(const Json& in) override {
			isPrimitive = in.value("isPrimitive", true);
			castShadows = in.value("castShadows", true);

			if (isPrimitive) {
				primitiveType = in.value("primitiveType", "cube");

				if (primitiveType == "subdivided_plane") {
					planeSizeX = in.value("planeSizeX", 1.0f);
					planeSizeZ = in.value("planeSizeZ", 1.0f);
					planeSegmentsX = in.value("planeSegmentsX", 1);
					planeSegmentsZ = in.value("planeSegmentsZ", 1);
					mesh = Utils::PrimitiveMeshes::Get().SubdividedPlane(planeSizeX, planeSizeZ, planeSegmentsX, planeSegmentsZ);
				}
				else {
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

			if (isPrimitive && mesh && in.contains("material")) {
				if (auto material { AssetManagement::AssetSerializer<Rendering::Bindables::Material>::Deserialize(in.at("material")) }) {
					mesh->SetMaterial(material);
				}
			}

			SyncRenderQueueFromMaterial();
		}

	public:
		void EmitDrawCommand(Rendering::DrawCommandBuffer& drawCmdBuffer, const Camera& camera) const override;
		[[nodiscard]] std::optional<AABB> GetAABB() const override { return mesh->GetAABB(); }
	};

	LW_REGISTER_COMPONENT(MeshRenderer)
} // namespace Engine::Components