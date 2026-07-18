#pragma once

#include <Leadwort/asset-management/public/DefaultAssets.h>
#include <Leadwort/components/behaviours/public/DirectionaLight.h>
#include <Leadwort/components/behaviours/public/FirstPersonController.h>
#include <Leadwort/components/public/Camera.h>
#include <Leadwort/components/public/MeshRenderer.h>
#include <Leadwort/core/public/Scene.h>

namespace Leadwort::Scenes {

class EmptyScene final : public Core::Scene {
public:
    void Create() override {
        // Default Cube
		Core::Entity* sphere = CreateEntity("Default Sphere");
        {
            auto* mr = sphere->AddComponent<Components::MeshRenderer>();
			mr->mesh = Utils::PrimitiveMeshes::Get().Sphere();
        }

    	sphere->GetTransform().SetLocalScale(Vec3(7.0f));

		const auto modelParent = CreateEntity("Model");
		const auto model = AssetManagement::EngineAssets::GetModel("models/model_mech_drone.glb");
		model->Instantiate(*modelParent);
    	modelParent->GetTransform().SetLocalPosition(Vec3(0.0f, 0.0f, -15.0f));
    	modelParent->GetTransform().SetLocalRotation(Quat::FromEuler(0.0f, -180.0f, 0.0f));

        // Main Camera
        {
			Core::Entity* e = CreateEntity("Main Camera");
            e->tag = Core::Tags::MAIN_CAMERA;
            e->GetTransform().SetWorldPosition(Vec3(5.0f, 2.5f, -5.0f));
            e->GetTransform().LookAt(sphere->GetTransform());

            auto* cam = e->AddComponent<Components::Camera>();
            cam->cullingMask = Utils::Layers::EVERYTHING & ~Utils::Layers::SCENE;
        }

        // Scene Camera
        {
			Core::Entity* e = CreateEntity("Scene Camera");
            e->tag = Core::Tags::SCENE_CAMERA;
            e->GetTransform().SetWorldPosition(Vec3(5.0f, 2.5f, -5.0f));
            e->GetTransform().LookAt(sphere->GetTransform());

            e->AddComponent<Components::Behaviours::FirstPersonController>();
			auto* a = e->AddComponent<Components::Camera>();
			a->background = Components::Camera::SkyBox {
				.skybox = CreateUnique<Rendering::Skybox>()
			};
        }

        // Directional Light
        {
			Core::Entity* e = CreateEntity("Directional Light");
            e->AddComponent<Components::Behaviours::DirectionalLight>();
			e->GetTransform().SetLocalRotation(Quat::FromEuler(135.0f, -45.0f, -45.0f));
        }
    }
};

} // namespace Engine::Scenes