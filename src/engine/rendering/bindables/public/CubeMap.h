#pragma once

#include "engine/rendering/bindables/public/Bindable.h"
#include "engine/asset-management/private/AssetKey.h"

#include <array>
#include <string>

namespace Engine::Rendering::Bindables {

class CubeMap final : public Bindable {
public:
	explicit CubeMap(const std::array<std::string, 6>& paths, AssetManagement::AssetKey<CubeMap>);
	~CubeMap() override;

	CubeMap(const CubeMap&)            = delete;
	CubeMap& operator=(const CubeMap&) = delete;
	CubeMap(CubeMap&&)                 = default;
	CubeMap& operator=(CubeMap&&)      = default;

	void Bind(int slot) const;
	void Bind()   const noexcept override;
	void Unbind() const noexcept override;
};

} // namespace Engine::Rendering::Bindables