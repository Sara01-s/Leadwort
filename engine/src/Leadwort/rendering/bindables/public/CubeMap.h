#pragma once

#include <Leadwort/asset-management/private/AssetKey.h>
#include <Leadwort/rendering/bindables/public/IBindable.h>

#include <array>
#include <string>

namespace Leadwort::Rendering::Bindables {

class CubeMap final : public IBindable {
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