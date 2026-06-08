#pragma once

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <cstdlib>
	#include <unistd.h>
#endif

namespace Engine::Utils {

class Log {
public:
	// #define LOG_TO_FILE

	static constexpr char EOL{'\n'};
	static constexpr auto SUB_LOG_SYMBOL{"╰>"};

	Log() = delete;
	~Log() = delete;
	Log(const Log&) = delete;

	// Colors.
	static constexpr auto COLOR_CLEAR{"\033[0m"};
	static constexpr auto COLOR_BLACK{"\033[30m"};
	static constexpr auto COLOR_RED{"\033[31m"};
	static constexpr auto COLOR_GREEN{"\033[32m"};
	static constexpr auto COLOR_YELLOW{"\033[33m"};
	static constexpr auto COLOR_BLUE{"\033[34m"};
	static constexpr auto COLOR_MAGENTA{"\033[35m"};
	static constexpr auto COLOR_CYAN{"\033[36m"};
	static constexpr auto COLOR_WHITE{"\033[37m"};
	static constexpr auto COLOR_GRAY{"\033[90m"};

	static constexpr auto LIGHT_BLACK{"\033[90m"};
	static constexpr auto LIGHT_RED{"\033[91m"};
	static constexpr auto LIGHT_GREEN{"\033[92m"};
	static constexpr auto LIGHT_YELLOW{"\033[93m"};
	static constexpr auto LIGHT_BLUE{"\033[94m"};
	static constexpr auto LIGHT_MAGENTA{"\033[95m"};
	static constexpr auto LIGHT_CYAN{"\033[96m"};
	static constexpr auto LIGHT_WHITE{"\033[97m"};
	static constexpr auto LIGHT_GRAY{"\033[97m"};

	static constexpr auto STYLE_BOLD{"\033[1m"};
	static constexpr auto STYLE_UNDERLINE{"\033[4m"};
	static constexpr auto STYLE_INVERT{"\033[7m"};

	static void Initialize() noexcept {
#ifdef _WIN32
		const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		if (GetConsoleMode(hOut, &dwMode)) {
			SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
#endif
	}

	// Logging.
	template <typename... Args>
	static void Info(Args&&... args) noexcept {
		LogMessage("[INFO]: ", COLOR_WHITE, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void SubInfo(Args&&... args) noexcept {
		SubInfoImpl(1, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void SubInfo(const size_t level, Args&&... args) noexcept {
		SubInfoImpl(level, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void Header(Args&&... args) noexcept {
		std::ostringstream oss{};
		(oss << ... << args);

		constexpr std::size_t totalWidth = 50;
		const std::string message = oss.str();
		const std::size_t padding = totalWidth > message.length() ? (totalWidth - message.length()) / 2 : 0;

		std::string padded_message = std::string(padding, ' ') + message + std::string(padding, ' ');
		if (padded_message.length() < totalWidth) {
			padded_message += ' ';
		}

		std::cout << "########################################################" << EOL;
		std::cout << "## " << padded_message << " ##" << EOL;
		std::cout << "########################################################" << EOL;
	}

	template <typename... Args>
	static void Warn(Args&&... args) noexcept {
		LogMessage("[WARNING]: ", COLOR_YELLOW, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void SubWarn(Args&&... args) noexcept {
		SubWarnImpl(1, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void SubWarn(const size_t level, Args&&... args) noexcept {
		SubWarnImpl(level, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void Error(Args&&... args) noexcept {
		LogMessage("[ERROR]: ", COLOR_RED, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void SubError(Args&&... args) noexcept {
		SubErrorImpl(1, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void SubError(const size_t level, Args&&... args) noexcept {
		SubErrorImpl(level, std::forward<Args>(args)...);
	}

	// Cursor.
	static void MoveCursor(const int x, const int y) noexcept { std::cout << "\033[" << x << ";" << y << "H"; }
	static void MoveCursorUp(const int n) noexcept { std::cout << "\033[" << n << "A"; }
	static void MoveCursorDown(const int n) noexcept { std::cout << "\033[" << n << "B"; }
	static void MoveCursorRight(const int n) noexcept { std::cout << "\033[" << n << "C"; }
	static void MoveCursorLeft(const int n) noexcept { std::cout << "\033[" << n << "D"; }
	static void MoveCursorToColumn(const int n) noexcept { std::cout << "\033[" << n << "G"; }
	static void MoveCursorLineStart() noexcept { std::cout << "\r"; }
	static void SaveCursorPosition() noexcept { std::cout << "\033[s"; }
	static void LoadCursorPosition() noexcept { std::cout << "\033[u"; }
	static void ShowCursor() noexcept { std::cout << "\033[?25h"; }
	static void HideCursor() noexcept { std::cout << "\033[?25l"; }

	// Clearing.
	static void ClearLine() noexcept { std::cout << "\033[K"; }
	static void ClearScreen() noexcept { std::cout << "\033[2J"; }
	static void ClearFromCursor() noexcept { std::cout << "\033[0K"; }
	static void ClearFromStartToCursor() noexcept { std::cout << "\033[1K"; }

	// Convert.
	static std::string ToString(const bool value) {
		return value ? std::string(LIGHT_GREEN) + "true" : std::string(LIGHT_RED) + "false";
	}

private:
	static bool SupportsColor() noexcept {
#ifdef _WIN32
		const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		return GetConsoleMode(hOut, &dwMode) && (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
		const char* term = std::getenv("TERM");
		return isatty(fileno(stdout)) && term != nullptr && std::string(term) != "dumb";
#endif
	}

	template <typename... Args>
	static void SubInfoImpl(const size_t level, Args&&... args) noexcept {
		std::ostringstream oss{};
		oss << "[INFO]: ";

		for (size_t i{}; i < level; ++i) {
			oss << "   ";
		}

		oss << SUB_LOG_SYMBOL << " ";
		LogMessage(oss.str().c_str(), COLOR_WHITE, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void SubWarnImpl(const size_t level, Args&&... args) noexcept {
		std::ostringstream oss{};
		oss << "[WARNING]: ";

		for (size_t i{}; i < level; ++i) {
			oss << "   ";
		}

		oss << SUB_LOG_SYMBOL << " ";
		LogMessage(oss.str().c_str(), COLOR_YELLOW, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void SubErrorImpl(const size_t level, Args&&... args) noexcept {
		std::ostringstream oss{};
		oss << "[ERROR]: ";

		for (size_t i{}; i < level; ++i) {
			oss << "   ";
		}

		oss << SUB_LOG_SYMBOL << " ";
		LogMessage(oss.str().c_str(), COLOR_RED, std::forward<Args>(args)...);
	}

	template <typename... Args>
	static void LogMessage(const char* prefix, const char* color, Args&&... args) noexcept {
		std::ostringstream oss{};
		(oss << ... << args);
		const std::string message = oss.str();

#ifdef LOG_TO_FILE
		if (!std::filesystem::exists("../logs/")) {
			std::filesystem::create_directories("../logs/");
		}

		std::ofstream log_file{"../logs/app.log", std::ios_base::app};
		if (log_file.is_open()) {
			log_file << prefix << message << EOL;
		}
#else
		if (SupportsColor()) {
			std::cout << color << prefix << message << COLOR_CLEAR << EOL;
		}
		else {
			std::cout << prefix << message << EOL;
		}
#endif
	}
};

#define CORE_ENABLE_ASSERTS_IN_RELEASE 1

#ifndef CORE_ENABLE_ASSERTS_IN_RELEASE
	#define CORE_ENABLE_ASSERTS_IN_RELEASE 0
#endif

#if !defined(NDEBUG) || CORE_ENABLE_ASSERTS_IN_RELEASE
	#define CORE_ASSERT(statement, message)                                                                            \
		if (!(static_cast<bool>(statement))) {                                                                         \
			Engine::Utils::Log::Error(__FILE__, ":", __LINE__, ":\nAssertion failed: ", #statement, ": ", message);    \
			assert(0);                                                                                                 \
		}

	#define CORE_LOG(...)   Engine::Utils::Log::Info(__VA_ARGS__)
	#define CORE_WARN(...)  Engine::Utils::Log::Warn(__VA_ARGS__)
	#define CORE_ERROR(...) Engine::Utils::Log::Error(__VA_ARGS__)
#else
	#define CORE_ASSERT(statement, message)
	#define CORE_LOG(...)
	#define CORE_WARN(...)
	#define CORE_ERROR(...)
#endif

} // namespace Engine::Utils