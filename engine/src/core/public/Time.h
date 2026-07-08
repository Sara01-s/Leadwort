#pragma once

namespace Engine::Core {

class Time {
public:
	Time() = delete;

	static inline float timeScale = 1.0f;
	static constexpr float FIXED_DELTA_TIME = 1.0f / 60.0f;

	static float GetDeltaTime() { return m_DeltaTime; }
	static float GetTime()      { return m_Time; }

	static void Update(const double currentTime) {
		if (m_LastTime <= 0.01) {
			m_LastTime = currentTime;
		}

		const float frameTime = static_cast<float>(currentTime - m_LastTime) * timeScale;

		m_LastTime    = currentTime;
		m_DeltaTime   = frameTime;
		m_Time       += frameTime;
		m_Accumulator += frameTime;
	}

	static bool ShouldRunFixedUpdate() { return m_Accumulator >= FIXED_DELTA_TIME; }
	static void ConsumeFixedUpdate()    { m_Accumulator -= FIXED_DELTA_TIME; }

	static void Reset() {
		m_LastTime    = 0.0;
		m_DeltaTime   = 0.0f;
		m_Time        = 0.0f;
		m_Accumulator = 0.0f;
	}

private:
	static inline double m_LastTime    = 0.0;
	static inline float  m_DeltaTime   = 0.0f;
	static inline float  m_Time        = 0.0f;
	static inline float  m_Accumulator = 0.0f;
};

} // namespace Engine::Core