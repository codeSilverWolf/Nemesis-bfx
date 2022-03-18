#pragma once

#include <iterator>
#include <string>
#include <cwchar>
#include <vector>

#include "unicode_utils.h"

namespace nemesis
{
    //TODO: remove unneded declarations

    //const char* to_lower_copy(const char* data);
    //const wchar_t* to_lower_copy(const wchar_t* data);

    inline std::string to_lower_copy(const std::string& data)
    {
        std::string l_copy(data);
        uu::foldString(l_copy);
        return l_copy;
    }
    inline std::wstring to_lower_copy(const std::wstring& data)
    {
        std::wstring l_copy(data);
        uu::foldString(l_copy);
        return l_copy;
    }
    

    inline void to_lower(std::string& data)
    {
        uu::foldString(data);
    }
    inline void to_lower(std::wstring& data)
    {
        uu::foldString(data);
    }

    //const char* to_upper_copy(const char* data);
    //const wchar_t* to_upper_copy(const wchar_t* data);

    std::string to_upper_copy(const std::string& data);
    std::wstring to_upper_copy(const std::wstring& data);

    void to_upper(std::string& data);
    void to_upper(std::wstring& data);

    // bool iequals(const char* l, const char* r);
    // bool iequals(const wchar_t* l, const wchar_t* r);
    inline bool iequals(const std::string& l, const std::string& r)
    {
        return uu::foldedEquals(l,r);
    };
    inline bool iequals(const std::wstring& l, const std::wstring& r)
    {
        return uu::foldedEquals(l,r);
    };
    // bool iequals(const char* l, const std::string& r);
    // bool iequals(const wchar_t* l, const std::wstring& r);
    // bool iequals(const std::string& l, const char* r);
    // bool iequals(const std::wstring& l, const wchar_t* r);

    // template <typename T, typename F>
    // inline T transform_to(const F& str) noexcept
    // {
    //     if (str.empty()) return {};

    //     return {std::begin(str), std::end(str)};
    // };

    // TODO: generic transform_to will mangle non ascii ancoded strings!
    // Add check for this situation and throw exception. 

    template <typename T, typename F>
    inline T transform_to(const F& str) noexcept
    {
        if (str.empty()) return {};
        T str_out;
        auto iter_end = std::end(str);
        for(auto i = std::begin(str); i != iter_end; ++i)
            str_out.push_back(*i);
        return str_out;
    };

    // UTF-8 -> UTF-16 version
    // TODO: see if we can optimize memory allocation by using, for small strings,
    // static buffer instead of std::vector. Maybe array on stack?

    template <>
        inline std::wstring transform_to(const std::string& str) noexcept
        {
            if (str.empty()) return {};
            std::mbstate_t shift_state{};
            const char *input_buf = str.c_str();
            std::size_t output_len = mbsrtowcs(nullptr, &input_buf, 0, &shift_state) + 1;

            std::vector<wchar_t> out_buffer(output_len);
            mbsrtowcs(&out_buffer[0], &input_buf, output_len, &shift_state);
            return std::wstring(out_buffer.data());
        };

    // UTF-16 -> UTF-8 version

    template <>
        inline std::string transform_to(const std::wstring& str) noexcept
        {
            if (str.empty()) return {};
            std::mbstate_t shift_state{};
            const wchar_t *input_buf = str.c_str();
            std::size_t output_len = wcsrtombs(nullptr, &input_buf, 0, &shift_state) + 1;

            std::vector<char> out_buffer(output_len);
            wcsrtombs(&out_buffer[0], &input_buf, output_len, &shift_state);
            
            return std::string(out_buffer.data());
        };


} // namespace nemesis
