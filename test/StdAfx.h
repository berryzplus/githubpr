// StdAfx.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#define WIN32_LEAN_AND_MEAN

#define NO_MINMAX

#define _WIN32_WINNT 0x0602

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <locale>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <regex>
#include <functional>

#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"
