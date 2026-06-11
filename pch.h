// pch.h
#pragma once

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#define NOGDI
	#define NOMCX
	#define NOSERVICE
	#include <windows.h>
#endif