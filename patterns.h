#pragma once
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <Psapi.h>

inline auto find_pattern(std::byte* start, const std::size_t length, const char* pattern, const std::string& mask) -> std::byte* //C++20 pattern -> span, check pattern.len == mask.len
{
    size_t position = 0;
    for (auto* i = start; i < start + length - mask.length(); ++i)
    {
        if (mask[position] == '*' || *reinterpret_cast<char*>(i) == pattern[position])
        {
            if (++position == mask.length())
                return ++i - mask.length();
        }
        else
        {
            position = 0;
        }
    }
    return nullptr;
}

inline auto find_pattern(const HMODULE module, const char* pattern, const std::string& mask) -> std::byte*
{
    MODULEINFO info = { };
    GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(MODULEINFO));
    return find_pattern(reinterpret_cast<std::byte*>(module), info.SizeOfImage, pattern, mask);
}