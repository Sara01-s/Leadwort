#pragma once

#include "engine/asset-management/public/DefaultAssets.h"
#include "engine/components/behaviours/public/DirectionaLight.h"
#include "engine/components/behaviours/public/FirstPersonController.h"
#include "engine/components/public/Camera.h"
#include "engine/components/public/MeshRenderer.h"
#include "engine/core/public/Scene.h"
#include "engine/rendering/public/Skybox.h"
#include "engine/utils/public/PrimitiveMeshes.h"

#include <glm/gtc/quaternion.hpp>

namespace Engine::Scenes {

class EmptyScene final : public Core::Scene {
public:
    void Create() override {
        // Default Cube
		Core::Entity* cube = CreateEntity("Default Cube");
        {
            auto* mr = cube->AddComponent<Components::MeshRenderer>();
            mr->mesh = Utils::PrimitiveMeshes::Get().Cube();
            mr->SetMaterial(AssetManagement::DefaultAssets::GetLitMaterial());
        }

        // Main Camera
        {
			Core::Entity* e = CreateEntity("Main Camera");
            e->tag = Core::Tags::MAIN_CAMERA;
            e->transform->SetWorldPosition({ 5.0f, 2.5f, -5.0f });
            e->transform->LookAt(cube->transform);

            auto* cam = e->AddComponent<Components::Camera>();
            cam->cullingMask = Utils::Layers::EVERYTHING & ~Utils::Layers::SCENE;
        }

        // Scene Camera
        {
			Core::Entity* e = CreateEntity("Scene Camera");
            e->tag = Core::Tags::SCENE_CAMERA;
            e->transform->SetWorldPosition({ 5.0f, 2.5f, -5.0f });
            e->transform->LookAt(cube->transform);

            e->AddComponent<Components::Behaviours::FirstPersonController>();
        	e->AddComponent<Components::Camera>();
        }

        // Directional Light
        {
			Core::Entity* e = CreateEntity("Directional Light");
            e->AddComponent<Components::Behaviours::DirectionalLight>();

            const auto qX = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            const auto qY = glm::angleAxis(glm::radians(-50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            e->transform->SetLocalRotation(qX * qY);
        }
    }
};

} // namespace Engine::Scenes