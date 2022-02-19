#pragma once

#if _WIN32 || _WIN64
    #define WINDOWS
    #if _WIN64
        #define ENVIRONMENT64
    #else
        #define ENVIRONMENT32
    #endif
// Check GCC
#elif __GNUC__
    #if __x86_64__ || __ppc64__
         #define ENVIRONMENT64
    #else
         #define ENVIRONMENT32
    #endif
#endif

#ifdef WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <TlHelp32.h>
    #include <Psapi.h>
#endif

#include <iostream>

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <algorithm>
#include <format>

#include "String/String.h"
#include "Address/Address.h"
#include "ModuleInfo/ModuleInfo.h"
#include "Process/Process.h"