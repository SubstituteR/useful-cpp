#pragma once
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <Psapi.h>

inline auto find_pattern(unsigned char* start, const std::size_t length, const std::initializer_list<unsigned char> pattern, const std::string& mask) -> unsigned char*
{
    if (pattern.size() != mask.length())
        return nullptr;
    size_t position = 0;
    for (auto* i = start; i < start + length - mask.length(); ++i)
    {
        if (mask[position] == '*' || *i == pattern.begin()[position])
        {
            if (++position == mask.length())
                return (++i - mask.length());
        }
        else
        {
            position = 0;
        }
    }
    return nullptr;
}

inline auto find_pattern(const HMODULE mod, const std::initializer_list<unsigned char> pattern, const std::string& mask) -> unsigned char*
{
    MODULEINFO info = { };
    GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(MODULEINFO));
    return find_pattern(reinterpret_cast<unsigned char*>(mod), info.SizeOfImage, pattern, mask);
}
