#pragma once

#include <Leadwort/asset-management/public/DefaultAssets.h>
#include <Leadwort/components/behaviours/public/FirstPersonController.h>
#include <Leadwort/components/behaviours/public/Light.h>
#include <Leadwort/components/public/Camera.h>
#include <Leadwort/components/public/MeshRenderer.h>
#include <Leadwort/core/public/IScene.h>

namespace Leadwort::Scenes {

	class DefaultScene final : public Core::IScene {
	public:
	    void Create() override {
	        // Default Cube
			Core::Entity* cube { CreateEntity("Default Cube") };
	        {
	            auto* mr { cube->AddComponent<Components::MeshRenderer>() };
				mr->mesh = Utils::PrimitiveMeshes::Get().Cube();
	        }

			Core::Entity* water { CreateEntity("Water") };
	    	water->GetTransform().SetLocalScale(Vec3(100.0f, 1.0f, 100.0f));
	    	water->GetTransform().SetLocalPosition(Vec3(0.0f, -3.0f, 0.0f));
			auto* mr { water->AddComponent<Components::MeshRenderer>() };
			const auto waterShader { AssetManagement::EngineAssets::GetShader("shaders/shd_water.glsl") };
			const auto waterMaterial { AssetManagement::EngineAssets::CreateMaterial(waterShader) };
			mr->SetSubdividedPlane(1.0f, 1.0f, 100, 100);
			mr->mesh->SetMaterial(waterMaterial);

			const auto modelParent { CreateEntity("Scenery") };
			const auto model { AssetManagement::EngineAssets::GetModel("models/model_scene.glb") };
			model->Instantiate(*modelParent);

	        // Main Camera
	        {
				Core::Entity* e { CreateEntity("Main Camera") };
	            e->tag = Core::Tags::MAIN_CAMERA;
	            e->GetTransform().SetLocalPosition(Vec3(-5.0f, 2.0f, -2.0f));
				e->GetTransform().SetLocalRotation(Quat::FromEuler(360.0f, 45.0f, 0.0f));

	            auto* cam = e->AddComponent<Components::Camera>();
	            cam->CullingMask = Utils::Layers::EVERYTHING & ~Utils::Layers::SCENE;
				cam->Background = Components::Camera::SkyBox {
					.Sky = CreateUnique<Rendering::Skybox>("textures/skyboxes/tex_sky.exr")
				};
	        }

	        // Scene Camera
	        {
				Core::Entity* e { CreateEntity("Scene Camera") };
	            e->tag = Core::Tags::SCENE_CAMERA;
	            e->GetTransform().SetWorldPosition(Vec3(5.0f, 2.5f, -5.0f));
	            e->GetTransform().LookAt(cube->GetTransform());

	            e->AddComponent<Components::Behaviours::FirstPersonController>();
				auto* a { e->AddComponent<Components::Camera>() };
				// a->Background = Components::Camera::SolidColor {
				// 	.Color = Color::Gray35()
				// };
				a->Background = Components::Camera::SkyBox {
					.Sky = CreateUnique<Rendering::Skybox>("textures/skyboxes/tex_sky.exr")
				};
	        }

	        // Directional Light
	        {
				Core::Entity* e { CreateEntity("Directional Light") };
	            e->AddComponent<Components::Behaviours::Light>();
				e->GetTransform().SetLocalRotation(Quat::FromEuler(170.0f, -45.0f, -45.0f));
	        }
	    }
	};

} // namespace Engine::Scenes