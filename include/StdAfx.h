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

#define THROW_APP_EXCEPTION(message) \
	throw std::runtime_error(message)


std::wstring getCurrentDirectory();
std::wstring loadString(_In_ WORD wStringResourceId);
std::wstring getEnvStr(_In_z_ LPCWSTR pszVarName);
std::wstring convertMbsToWString(_In_reads_z_(cchMbString) LPCSTR pszMbString, _In_ SIZE_T cchMbString);
