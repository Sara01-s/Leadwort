#pragma once
#include <string_view>

namespace Leadwort::Rendering {
	// Reserved texture units for engine-wide "global" samplers.
	// Materials must never bind into this range.
	namespace GlobalTextureSlots {
		static constexpr int ShadowMapSlot     { 15 };
		static constexpr int IBLIrradianceSlot { 14 };
		static constexpr int IBLPrefilterSlot  { 13 };
		static constexpr int IBLBrdfLUTSlot    { 12 };

		constexpr int GetReservedSlotForGlobalSampler(const std::string_view& name) noexcept {
			if (name == "_ShadowMap")     return ShadowMapSlot;
			if (name == "_IBLIrradiance") return IBLIrradianceSlot;
			if (name == "_IBLPrefilter")  return IBLPrefilterSlot;
			if (name == "_IBLBrdfLUT")    return IBLBrdfLUTSlot;
			return -1;
		}
	}
}