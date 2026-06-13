#pragma once

#include "engine/asset-management/public/DefaultAssets.h"
#include "engine/components/behaviours/public/DirectionaLight.h"
#include "engine/components/behaviours/public/FirstPersonController.h"
#include "engine/components/public/Camera.h"
#include "engine/components/public/MeshRenderer.h"
#include "engine/core/public/Scene.h"

namespace Engine::Scenes {

class EmptyScene final : public Core::Scene {
public:
    void Create() override {
        // Default Cube
		Core::Entity* cube = CreateEntity("Default Cube");
        {
            cube->AddComponent<Components::MeshRenderer>();
        }

		const auto modelParent = CreateEntity("Model");
		const auto model = AssetManagement::EngineAssets::GetModel("models/model_mech_drone.glb");
		model->Instantiate(*modelParent);
    	modelParent->GetTransform().SetLocalPosition(Vec3(0.0f, 0.0f, -15.0f));
    	modelParent->GetTransform().SetLocalScale(Vec3(0.1f, 0.1f, 0.1f));
    	modelParent->GetTransform().SetLocalRotation(Quat::FromEuler(-90.0f, -180.0f, 0.0f));

        // Main Camera
        {
			Core::Entity* e = CreateEntity("Main Camera");
            e->tag = Core::Tags::MAIN_CAMERA;
            e->GetTransform().SetWorldPosition(Vec3(5.0f, 2.5f, -5.0f));
            e->GetTransform().LookAt(cube->GetTransform());

            auto* cam = e->AddComponent<Components::Camera>();
            cam->cullingMask = Utils::Layers::EVERYTHING & ~Utils::Layers::SCENE;
        }

        // Scene Camera
        {
			Core::Entity* e = CreateEntity("Scene Camera");
            e->tag = Core::Tags::SCENE_CAMERA;
            e->GetTransform().SetWorldPosition(Vec3(5.0f, 2.5f, -5.0f));
            e->GetTransform().LookAt(cube->GetTransform());

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
			e->GetTransform().SetLocalRotation(Quat::FromEuler(-45.0f, -45.0f, -45.0f));
        }
    }
};

} // namespace Engine::Scenes