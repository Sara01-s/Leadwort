#pragma once

namespace Engine::Utils::Layers {

inline constexpr uint32_t NONE = 0;
inline constexpr uint32_t DEFAULT = 1 << 0;
inline constexpr uint32_t SCENE = 1 << 1;
inline constexpr uint32_t UI = 1 << 2;
inline constexpr uint32_t EVERYTHING = ~0u;

} // namespace Engine::Utils::Layers