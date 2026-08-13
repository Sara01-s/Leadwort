// pch.h
#pragma once

#ifdef _WIN32
	#define _WIN32_WINNT 0x0A00 // Windows 10
	#define WINVER       0x0A00
	#include <sdkddkver.h>

	#ifdef FULL_WINDOWS
		#define WIN32_LEAN_AND_MEAN
		#define NOGDICAPMASKS
		#define NOSYSMETRICS
		#define NOMENUS
		#define NOICONS
		#define NOSYSCOMMANDS
		#define NORASTEROPS
		#define OEMRESOURCE
		#define NOATOM
		#define NOCLIPBOARD
		#define NOCOLOR
		#define NOCTLMGR
		#define NODRAWTEXT
		#define NOKERNEL
		#define NONLS
		#define NOMEMMGR
		#define NOMETAFILE
		#define NOOPENFILE
		#define NOSCROLL
		#define NOSERVICE
		#define NOSOUND
		#define NOTEXTMETRIC
		#define NOWH
		#define NOCOMM
		#define NOKANJI
		#define NOHELP
		#define NOPROFILER
		#define NODEFERWINDOWPOS
		#define NOMCX
		#define NORPC
		#define NOPROXYSTUB
		#define NOIMAGE
		#define NOTAPE
	#endif

	#define NOMINMAX
	#define STRICT

	#undef far
	#undef near

	#include <windows.h>
#endif

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <format>
#include <optional>
#include <filesystem>
#include <ranges>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <any>
