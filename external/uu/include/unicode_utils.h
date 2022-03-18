/* Unicode utility functions 

Copyright (C) 2022  codeSilverWolf  https://github.com/codeSilverWolf

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <clocale>
#include <cstddef>
#include <cstdint>
#include <cwctype>
#include <string>
// for TextFile
#include <filesystem>
#include <cstdio>
#include <cwchar>
#include <vector>

namespace uu {

    // struct describing how to fold code points in whole block of 256 characters
    struct FoldBlockInfo {
        enum FoldBlockType : uint8_t {NO_CHANGE = 0, CODEPOINTS_TABLE, SEQUENCES};
        FoldBlockType type;
        uint8_t length;
        uint16_t index;  // index in codepoints block table (first codepoint offset = index*256 !!!) OR sequence table (first sequence offset = index !!!)
    };

    struct FoldSequence {
        char32_t first_code;    // first code point in sequence
        char32_t last_code;     // last code point in sequence
        int32_t code_shift;         // difference between folded codepoint and it's unfolded codepoint for characters in sequence
        uint32_t distance;      // distance between characters in sequence
    };

    enum Status   {OK = 0,
                        CONVERSION_ERROR, // there was incorrectly encoded UTF characters in string
                        INTERNAL_ERROR  // logic error conversion is likely incorrect
                        };

    int utf16cToUtf32(const wchar_t *utf16str, char32_t& utf32c);
    
    int utf8cToUtf32(const char *utf8str, char32_t& utf32c);

    int utf32cToUtf8(char32_t u32c, char *u8str, size_t buf_size);
    
    // string conversions
    std::wstring utf8StrToUtf16(const std::string &str, uu::Status *status = nullptr);
    std::string  utf16StrToUtf8(const std::wstring &str, uu::Status *status = nullptr);

    char32_t foldUtf32c(char32_t c);

    bool foldedEquals(const std::string& s1, const std::string& s2);
    bool foldedEquals(const std::wstring& s1, const std::wstring& s2);

    void foldString(std::string &str);
    void foldString(std::wstring &str);

    // returns true when utf-8 string contains alphabetic character
    bool hasAlpha(const std::string& str);
    // returns true when utf-16 string contains alphabetic character
    bool hasAlpha(const std::wstring& str);

    // utf-8 encoded text file
    class TextFile
    {
        std::filesystem::path fname;
        FILE *file;
        std::vector<char> buf;

        enum struct file_mode_t {INVALID, READING, WRITING};
        file_mode_t file_mode;

    public:
        TextFile() : fname{}, file {nullptr}, buf{}, file_mode(file_mode_t::INVALID) {};

        template<typename T>
        TextFile(const T &path) : file {nullptr}, buf{}, file_mode(file_mode_t::INVALID)
        {
            fname = std::filesystem::path(path);
        }

        ~TextFile()
        {
            if (file) std::fclose(file);
        }

        bool OpenForReading()
        {
            errno_t err;
            err = _wfopen_s(&file, fname.wstring().c_str(), L"rb"); // windows only, but needed for UTF-16 file path
            if (err == 0)
            {
                file_mode = file_mode_t::READING;
                return true;
            }
            else
            {
                file = nullptr;
                return false;
            }
        }

        bool OpenForWriting(bool append = false)
        {
            errno_t err;
            err = _wfopen_s(&file, fname.wstring().c_str(), append ? L"w+b" : L"wb"); // windows only, but needed for UTF-16 file path
            if (err == 0)
            {
                file_mode = file_mode_t::WRITING;
                return true;
            }
            else
            {
                file = nullptr;
                return false;
            }
        }

        bool Close()
        {
            if (file == nullptr)
                return true;
            else if(fclose(file) == 0)
                return true;
            else
                return false;
        }

        bool Flush()
        {
            if (file != nullptr)
            {
                if(fflush(file) == 0)
                    return true;
            }
            return false;
        }

        bool GetLine(std::string& line);
        bool GetLine(std::wstring& line);

        bool GetLines(std::vector<std::string>& lines);
        bool GetLines(std::vector<std::wstring>& lines);

        bool PutString(const std::string& line);
        bool PutString(const std::wstring& line);

        bool PutLine(const std::string& line);
        bool PutLine(const std::wstring& line);

        bool PutLines(std::vector<std::string>& lines);
        bool PutLines(std::vector<std::wstring>& lines);

    }; // TextFile

} // namespace uu