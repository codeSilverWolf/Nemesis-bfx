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

#include "unicode_utils.h"
#include <cctype>
#include <corecrt.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cwctype>
#include <iterator>
#include <locale>
#include <string>
#include <iostream>

namespace uu {

#include "uu_case_folding.inc"

// Function that converts one UTF-16 encoded character (at most 2 wchar / utf16 code units)
// to UTF32 from wchar NULL terminated string.
// Converted character is returned by utf32c parameter!.
// Function returns number of characters used for conversion.
// If string is malformed (unmatched surrogates) function
// sets returned char to current code unit casted to char32_t and returns -1

int utf16cToUtf32(const wchar_t *utf16str, char32_t& utf32c)
{
    if ( (*utf16str & 0xF800) != 0xD800)  // utf16 code unit is NOT in U+D800-U+DFFF surrogate range.
                                               // 0xF800 = 1111 1000 0000 0000b is a mask for significant bits in this range
    {
        utf32c = *utf16str; 
        return 1;
    }
    else 
    {
        // surrogate pair spH(igh)and spL(ow)
        // spH needs to be from 0xD800–0xDBFF , and spL 0xDC00–0xDFFF.
        char32_t spH = *utf16str, spL = *(utf16str + 1);
        if (spH <= 0xDBFFu && spL >= 0xDC00u && spL <= 0xDFFFu)
        {
            utf32c = ((spH-0xD800u) << 10) + (spL - 0xDC00u) + 0x10000u;
            return 2;
        }
        else  // error - two utf-16 code units are not in High and Low surrogate range
        {
            utf32c = spH; 
            return -1;
        }
    }
}

int utf32cToUtf16(char32_t u32c, wchar_t *u16buf, size_t buf_size)
{
    if (u32c < 0x10000) // 1 wchar_t
    {
        if (buf_size >= 1)
        {
            *u16buf = u32c;
            return 1;
        }
    }
    else if (u32c <= 0x10FFFF) // 2 wchar_t surrogate pair
    {
        if (buf_size >= 2)
        {
            u32c -= 0x10000;
            *u16buf = 0xD800 + (u32c >> 10);
            *(u16buf+1) = 0xDC00 + (u32c & 0x3FF);
            return 2;
        }
    }
    else
    {
        // bad character error. return U+FFFD and signalize error
        if (buf_size >= 1)
        {
            *u16buf = 0xFFFD;
            return -1;
        }
    }

    return 0;  // buffer too small
}

// no checking for supplied buffer size. Must be at least 2 wchar_t characters long.
int utf32cToUtf16Fast(char32_t u32c, wchar_t *u16buf)
{
    if (u32c < 0x10000) // 1 wchar_t
    {
        *u16buf = u32c;
        return 1;
    }
    else if (u32c <= 0x10FFFF) // 2 wchar_t surrogate pair
    {
        u32c -= 0x10000;
        *u16buf = 0xD800 + (u32c >> 10);
        *(u16buf+1) = 0xDC00 + (u32c & 0x3FF);
        return 2;
    }
    else
    {
        // bad character error. return U+FFFD and signalize error
        *u16buf = 0xFFFD;
        return -1;
    }
}

// no checking for supplied buffer size. Must be at least 2 wchar_t characters long.
// sets returns 0xFFFD for 
int utf32cToUtf16Fast(char32_t u32c, std::wstring::iterator &u16buf)
{
    if (u32c < 0x10000) // 1 wchar_t
    {
        *u16buf = u32c;
        return 1;
    }
    else if (u32c <= 0x10FFFF) // 2 wchar_t surrogate pair
    {
        u32c -= 0x10000;
        *u16buf = 0xD800 + (u32c >> 10);
        *(u16buf+1) = 0xDC00 + (u32c & 0x3FF);
        return 2;
    }
    else
    {
        // bad character error. return U+FFFD and signalize error
        *u16buf = 0xFFFD;
        return -1;
    }
}


// Function that converts one UTF-8 encoded character (at most 4 char / utf-8 code units)
// to UTF32 from char BUFFER.
// Converted character is returned by utf32c parameter!
// Function returns number of characters used for conversion.
// If string is malformed (1st code unit is wrong) sets returned character to current char and
// returns: -1
// If error is in next code units, sets returned utf32 character to 0 (ZERO), because correct conversion is impossible,
// and returns: -(number of correct chars in current codepoint).
// this allows skipping over bad sequence. 

int utf8cToUtf32(const char *utf8str, char32_t& utf32c)
{
    unsigned char u8c = *utf8str;
    if (!(u8c & 0x80))  // char is ASCII
    {
        utf32c = u8c;    
        return 1;
    }
    else  // multi utf-8 code unit character
    {
        char32_t u32c = 0;
        int extra_code_points = 0; // how many additional code units we expect

        if ( (u8c & 0xE0) == 0xC0)  // 2 code units
        {
            extra_code_points = 1;
            u32c = u8c & 0x1F;
        } else if ( (u8c & 0xF0) == 0xE0)  // 3 code units
        {
            extra_code_points = 2;
            u32c = u8c & 0x0F;
        } else if ( (u8c & 0xF8) == 0xF0)  // 4 code units
        {
            extra_code_points = 3;
            u32c = u8c & 0x07;
        }
        else  // error - incorrect 1st utf-8 code unit
        {
            utf32c = u8c;            
            return -1;
        }

        // next we expect 'extra_code_points' more characters in bit format 10xxxxxx
        int counter;
        for (counter = 0; counter < extra_code_points; counter++)
        {
            u8c = *(++utf8str);  // get next code unit
            if ( (u8c & 0xC0) == 0x80 )  // ok. character is in format 10xxxxxx b
            {
                u32c = (u32c << 6) + (u8c & 0x3F);  // shift 6 bits to left and add 6 least significant bytes form extra code unit
            }
            else // charcter is not correct n-th utf8 code unit or premature end of string
            {
                utf32c = 0;
                return -1-counter;
            }
        }
        utf32c = u32c;
        return 1+counter;
    }
}


int utf32cToUtf8(char32_t u32c, char *u8buf, size_t buf_size)
{
    if (u32c < 0x80) // ascii
    {
        if (buf_size >= 1)
        {
            *u8buf = u32c;
            return 1;
        }
    }
    else if (u32c < 0x800) // 2 bytes
    {
        if (buf_size >= 2)
        {
            *u8buf = (u32c >> 6) + 0b11000000;
            *(u8buf+1) = (u32c & 0b00111111) + 0b10000000;
            return 2;
        }
    }
    else if (u32c < 0x10000) // 3 bytes
    {
        if (buf_size >= 3)
        {
            *u8buf = (u32c >> 12) + 0b11100000;
            *(u8buf+1) = ((u32c >> 6) & 0b00111111) + 0b10000000;
            *(u8buf+2) = (u32c & 0b00111111) + 0b10000000;
            return 3;
        }
    }
    else if (u32c <= 0x10FFFF) // 4 bytes
    {
        if (buf_size >= 4)
        {
            *u8buf = (u32c >> 18) + 0b11110000;
            *(u8buf+1) = ((u32c >> 12) & 0b00111111) + 0b10000000;
            *(u8buf+2) = ((u32c >> 6) & 0b00111111) + 0b10000000;
            *(u8buf+3) = (u32c & 0b00111111) + 0b10000000;
            return 4;
        }
    }
    else
    {
        // bad character error. return '�' character ( U+FFFD (0xEF 0xBF 0xBD) in utf8 ) and signalize error
        if (buf_size >= 3)
        {
            *u8buf = 0xEF;
            *(u8buf+1) = 0xBF;
            *(u8buf+2) = 0xBD;
            return -3;
        }
    }

    return 0;  // buffer too small
}

// no checking for buffer size. Must be at least 4 char long.
int utf32cToUtf8Fast(char32_t u32c, char *u8buf)
{
    if (u32c < 0x80) // ascii
    {
        *u8buf = u32c;
        return 1;
    }
    else if (u32c < 0x800) // 2 bytes
    {
        *u8buf = (u32c >> 6) + 0b11000000;
        *(u8buf+1) = (u32c & 0b00111111) + 0b10000000;
        return 2;
    }
    else if (u32c < 0x10000) // 3 bytes
    {
        *u8buf = (u32c >> 12) + 0b11100000;
        *(u8buf+1) = ((u32c >> 6) & 0b00111111) + 0b10000000;
        *(u8buf+2) = (u32c & 0b00111111) + 0b10000000;
        return 3;
    }
    else if (u32c <= 0x10FFFF) // 4 bytes
    {
        *u8buf = (u32c >> 18) + 0b11110000;
        *(u8buf+1) = ((u32c >> 12) & 0b00111111) + 0b10000000;
        *(u8buf+2) = ((u32c >> 6) & 0b00111111) + 0b10000000;
        *(u8buf+3) = (u32c & 0b00111111) + 0b10000000;
        return 4;
    }
    else
    {
        // bad character error. return '�' character ( U+FFFD (0xEF 0xBF 0xBD) in utf8 ) and signalize error
        *u8buf = 0xEF;
        *(u8buf+1) = 0xBF;
        *(u8buf+2) = 0xBD;
        return -3;
    }
}

// no checking for buffer size. Must be at least 4 char long.
// iterator version
int utf32cToUtf8Fast(char32_t u32c, std::string::iterator &u8buf)
{
    if (u32c < 0x80) // ascii
    {
        *u8buf = u32c;
        return 1;
    }
    else if (u32c < 0x800) // 2 bytes
    {
        *u8buf = (u32c >> 6) + 0b11000000;
        *(u8buf+1) = (u32c & 0b00111111) + 0b10000000;
        return 2;
    }
    else if (u32c < 0x10000) // 3 bytes
    {
        *u8buf = (u32c >> 12) + 0b11100000;
        *(u8buf+1) = ((u32c >> 6) & 0b00111111) + 0b10000000;
        *(u8buf+2) = (u32c & 0b00111111) + 0b10000000;
        return 3;
    }
    else if (u32c <= 0x10FFFF) // 4 bytes
    {
        *u8buf = (u32c >> 18) + 0b11110000;
        *(u8buf+1) = ((u32c >> 12) & 0b00111111) + 0b10000000;
        *(u8buf+2) = ((u32c >> 6) & 0b00111111) + 0b10000000;
        *(u8buf+3) = (u32c & 0b00111111) + 0b10000000;
        return 4;
    }
    else
    {
        // bad character error. return '�' character ( U+FFFD (0xEF 0xBF 0xBD) in utf8 ) and signalize error
        *u8buf = 0xEF;
        *(u8buf+1) = 0xBF;
        *(u8buf+2) = 0xBD;
        return -3;
    }
}


// string lenght calculations

// validate string and calculate length as replacing each incorrect sequence with '�' character ( U+FFFD (0xEF 0xBF 0xBD) in utf8 ) 
// returns true if ok
// if validated_str_len != nullptr, then string length is returned via this pointer
bool utf8StrValidate(const char *utf8str, std::size_t *validated_str_len_p = nullptr)
{
    std::size_t len = 0;
    unsigned char u8c;
    bool valid = true;

    while ( (u8c = *utf8str) != 0)
    {
        if (!(u8c & 0x80))  // char is ASCII
        {
            ++len;
            ++utf8str;
        }
        else  // multi utf-8 code unit character
        {
            int extra_code_points = 0; // how many additional code units we expect

            if ( (u8c & 0xE0) == 0xC0)  // 2 code units
            {
                extra_code_points = 1;
            } else if ( (u8c & 0xF0) == 0xE0)  // 3 code units
            {
                extra_code_points = 2;
            } else if ( (u8c & 0xF8) == 0xF0)  // 4 code units
            {
                extra_code_points = 3;
            }
            else  // error - incorrect 1st utf-8 code unit
            {
                // replace with U+FFFD - 3 utf-8 code units
                len += 3;
                ++utf8str;
                valid = false;
                continue;
            }

            // next we expect 'extra_code_points' more characters in bit format 10xxxxxx
            int counter;
            for (counter = 0; counter < extra_code_points; counter++)
            {
                u8c = *(++utf8str);  // get next code unit
                if ( !((u8c & 0xC0) == 0x80) )  // character is not in correct format 10xxxxxx b
                {
                    valid = false;
                    len += 3; // replace with U+FFFD - 3 utf-8 code units

                    break;
                }
            }

            if(counter == extra_code_points) // all code units in charcter are valid 
            {
                len += 1+counter;
                ++utf8str;
            }
        }
    }

    if (validated_str_len_p != nullptr)
        *validated_str_len_p = len;
    return valid;
}

std::string utf8BufToValidStr(const char *utf8str, std::size_t validated_str_len = (-1))
{
    if (validated_str_len == -1) // get correct str length if it was not passed in argument
    {
        if (utf8StrValidate(utf8str, &validated_str_len)) 
        {
            // buffer is valid 
            return std::string(utf8str);
        }
        if (validated_str_len == 0)
            return {};
    }

    std::string str;
    unsigned char u8c;

    str.resize(validated_str_len);
    auto str_it = str.begin();


    while ( (u8c = *utf8str) != 0)
    {
        if (!(u8c & 0x80))  // char is ASCII
        {
            *str_it = u8c;
            ++str_it;
            ++utf8str;
        }
        else  // multi utf-8 code unit character
        {
            int extra_code_points = 0; // how many additional code units we expect

            if ( (u8c & 0xE0) == 0xC0)  // 2 code units
            {
                extra_code_points = 1;
            } else if ( (u8c & 0xF0) == 0xE0)  // 3 code units
            {
                extra_code_points = 2;
            } else if ( (u8c & 0xF8) == 0xF0)  // 4 code units
            {
                extra_code_points = 3;
            }
            else  // error - incorrect 1st utf-8 code unit
            {
                // replace with U+FFFD - 3 utf-8 code units
                *str_it = 0xEF;
                ++str_it;
                *str_it = 0xBF;
                ++str_it;
                *str_it = 0xBD;
                ++str_it;

                ++utf8str;
                continue;
            }

            // copy leading character
            *str_it = u8c;
            ++str_it;

            // next we expect 'extra_code_points' more characters in bit format 10xxxxxx
            int counter;
            for (counter = 0; counter < extra_code_points; counter++)
            {
                u8c = *(++utf8str);  // get next code unit
                if ( ((u8c & 0xC0) == 0x80) )  // character is in correct format 10xxxxxx b
                {
                    *str_it = u8c;
                    ++str_it;
                }
                else
                {
                    // rewind iterator to first code unit in current character
                    str_it -= counter+1;
                    *str_it = 0xEF;
                    ++str_it;
                    *str_it = 0xBF;
                    ++str_it;
                    *str_it = 0xBD;
                    ++str_it;

                    break;
                }
            }

            if(counter == extra_code_points)
            {
                ++utf8str;
            }
        }
    }
    return str;
}


// Calculate null terminated string length
// Assumed wors cese when incorrect UTF-8 code units are treated as 1 UTF32 code point (U+FFFD).
// And incomplete sequence (1st correct, but to few additionl code units) is treated as 1 UTF32 code point (U+FFFD)
std::size_t strLenInUtf32(const char *utf8str)
{
    unsigned char u8c;
    std::size_t len = 0;

    while ( (u8c = *utf8str) != 0)
    {
        if (!(u8c & 0x80))  // char is ASCII
        {
            ++len;
            ++utf8str;
        }
        else  // multi utf-8 code unit character
        {
            int extra_code_points = 0; // how many additional code units we expect

            if ( (u8c & 0xE0) == 0xC0)  // 2 code units
            {
                extra_code_points = 1;
            } else if ( (u8c & 0xF0) == 0xE0)  // 3 code units
            {
                extra_code_points = 2;
            } else if ( (u8c & 0xF8) == 0xF0)  // 4 code units
            {
                extra_code_points = 3;
            }
            else  // error - incorrect 1st utf-8 code unit
            {
                // replace with U+FFFD  so 1 codepoint
                ++len;
                ++utf8str;
                continue;
            }
            ++utf8str;

            // next we expect 'extra_code_points' more characters in bit format 10xxxxxx
            int counter;
            for (counter = 0; counter < extra_code_points; counter++)
            {
                u8c = *(utf8str);  // get next code unit
                if ( !((u8c & 0xC0) == 0x80) )  // character is in not in correct format 10xxxxxx b
                {
                    break;
                }
                ++utf8str;
            }
            ++len;
        }
    }
    return len;
}

// Calculate null terminated string length
// Assumed that incorrect UTF-8 code units or partially correct sequences are treated as 1 UTF32 code point (U+FFFD).
std::size_t strLenInUtf16(const char *utf8str)
{
    unsigned char u8c;
    std::size_t len = 0;

    while ( (u8c = *utf8str) != 0)
    {
        if (!(u8c & 0x80))  // char is ASCII - 1 UTF-16 code unit
        {
            ++len;
            ++utf8str;
        }
        else  // multi utf-8 code unit character
        {
            int extra_code_points = 0; // how many additional code units we expect

            if ( (u8c & 0xE0) == 0xC0)  // 2 code units
            {
                extra_code_points = 1;
            } else if ( (u8c & 0xF0) == 0xE0)  // 3 code units
            {
                extra_code_points = 2;
            } else if ( (u8c & 0xF8) == 0xF0)  // 4 code units
            {
                extra_code_points = 3;
                // this encodes codepoints above U+FFFF - add additional utf-16 code unit to length
                ++len;
            }
            else  // error - incorrect 1st utf-8 code unit
            {
                // replace with U+FFFD  so 1 codepoint
                ++len;
                ++utf8str;
                continue;
            }
            ++utf8str;

            // next we expect 'extra_code_points' more characters in bit format 10xxxxxx
            int counter;
            for (counter = 0; counter < extra_code_points; counter++)
            {
                u8c = *(utf8str);  // get next code unit
                if ( !((u8c & 0xC0) == 0x80) )  // character is in not in correct format 10xxxxxx b
                {
                    if (extra_code_points == 3) --len; // incorrectly encoded code unit will be converted to 1 not 2 code units!!
                    break;
                }
                ++utf8str;
            }
            ++len;
        }
    }
    return len;
}


// Length of utf-8 string when converted from source UTF-16 string
// incorrect codepoints are assumed to be converted to U+FFFD in utf-8 (so 3 chars)

std::size_t strLenInUtf8(const wchar_t *utf16str)
{
    std::size_t len = 0;
    int consumed;
    char32_t u32c;

    do
    {
        consumed = utf16cToUtf32(utf16str, u32c);
        if (consumed > 0)
        {
            if (u32c == 0)
                break;
            else if (u32c < 0x80) // ascii - 1 byte
                len += 1;
            else if (u32c < 0x800) // 2 bytes
                len += 2;
            else if (u32c < 0x10000) // 3 bytes
                len += 3;
            else if (u32c <= 0x10FFFF) // 4 bytes
                len +=  4;
            else // error utf32 value outside valid range
            {
                len += 3; // returning '?' character
            }

            utf16str += consumed;
        }
        else // incorrect utf16 code unit
        {
            len += 3;
            utf16str += std::abs(consumed);
        }
    } while (true);
    
    return len;
}


// performs simple (without increasing character count) case folding for single UTF32 encoded character
char32_t foldUtf32c(char32_t c)
{
    std::size_t block_nr = c >> 8;

    if (block_nr < sizeof(fold_blocks)/sizeof(FoldBlockInfo))
    {
        switch (fold_blocks[block_nr].type) {
            case FoldBlockInfo::CODEPOINTS_TABLE : // most common case as first 5 block use this method
                return fold_codes[ fold_blocks[block_nr].index*256u + (c & 0xFFu)];
            case FoldBlockInfo::NO_CHANGE :
                return c;
            case FoldBlockInfo::SEQUENCES : // check all sequences in block
                FoldSequence *seq = &fold_sequences[fold_blocks[block_nr].index];
                for(unsigned i = 0; i < fold_blocks[block_nr].length; i++)
                {
                    if(c >= seq->first_code && c <= seq->last_code)
                    {
                        if ((c - seq->first_code) % seq->distance == 0) 
                        {
                            return static_cast<int32_t>(c)+seq->code_shift;  // code shift can be a negative number !
                        }
                    }
                    ++seq;
                }
                break;

        }

    }
    // outside our folding table range - return same character as passed to function
    return c;
}

// Caseless strings comparision
// returns true if strings are equal, false otherwise
bool foldedEquals(const std::string& s1, const std::string& s2)
{
    const char *s1p = s1.c_str(), *s2p = s2.c_str();
    char32_t c1,c2;
    int consumed1,consumed2;

    // utf-8 encoded character can change size after being folded so simple size compare can not rule out folded string equality
    // do codepoint by dodepoint compare
    do {
        consumed1 = uu::utf8cToUtf32(s1p, c1);
        consumed2 = uu::utf8cToUtf32(s2p, c2);
        if (c1 && c2)  // both characters are not null and are good
        {
            if (uu::foldUtf32c(c1) == uu::foldUtf32c(c2))
            {
                // advance to next codepoints. abs is required since in case of errors in first utf8 code unit consumed it will -1 to pass through errors
                // TODO: error checking not only skipping
                s1p += std::abs(consumed1);
                s2p += std::abs(consumed2);                
            }
            else
            {
                return false;
            }
        }
        else  // NULL char(s) or error encountered
        {
            if (consumed1 > 0 && consumed2 > 0) // no errors
            {
                if (c1+c2 == 0) // both strings ended - they are equal
                    return true;
                else
                    return false;
            }
            else // errors 
            {
                // TODO: throw exception maybe??
                std::cout << "ERROR: bad UTF-8 encoding in foldedEquals. s1 " << (consumed1 > 0 ? "GOOD":"BAD") << " s2 " << (consumed2 > 0 ? "GOOD":"BAD") << "\n";
                std::cout << "s1 = '" << s1 << "'\n";
                std::cout << "s2 = '" << s2 << "'\n";
                
                return false;
            }
        }

    } while(true);
}

// Caseless wstrings comparision
// returns true if strings are equal, false otherwise
bool foldedEquals(const std::wstring& s1, const std::wstring& s2)
{
    const wchar_t *s1p = s1.c_str(), *s2p = s2.c_str();
    char32_t c1,c2;
    int consumed1,consumed2;

    // utf-8 encoded character can change size after being folded so simple size compare can not rule out folded string equality
    // do codepoint by dodepoint compare
    do {
        consumed1 = uu::utf16cToUtf32(s1p, c1);
        consumed2 = uu::utf16cToUtf32(s2p, c2);
        if (c1 && c2)  // both characters are not null and are good
        {
            if (uu::foldUtf32c(c1) == uu::foldUtf32c(c2))
            {
                // advance to next codepoints. abs is required since in case of errors utf16 code unit consumed it will -1 to pass through errors
                // TODO: error checking not only skipping
                s1p += std::abs(consumed1);
                s2p += std::abs(consumed2);                
            }
            else
            {
                return false;
            }
        }
        else  // NULL char(s) or error encountered
        {
            if (consumed1 > 0 && consumed2 > 0) // no errors
            {
                if (c1+c2 == 0) // both strings ended - they are equal
                    return true;
                else
                    return false;
            }
            else // errors 
            {
                // TODO: throw exception maybe??
                std::wcout << L"ERROR: bad UTF-16 encoding in foldedEquals. s1 " << (consumed1 > 0 ? L"GOOD":L"BAD") << L" s2 " << (consumed2 > 0 ? L"GOOD":L"BAD") << L"\n";
                std::wcout << L"s1 = '" << s1 << L"'\n";
                std::wcout << L"s2 = '" << s2 << L"'\n";
                
                return false;
            }
        }

    } while(true);
}

void foldString(std::string &str)
{
    if(str.empty())
        return;

    auto s_it = str.begin();
    auto d_it = s_it;
    auto s_endit = str.end();
    std::string::size_type f_size = 0;  // size of folded string - can be different than source in utf-8 strings
    char buf[4];
    char32_t c32;
    std::string src_copy; // will contain copy of rest of string in case of enlargement


    while (s_it != s_endit)
    {
        if(!(*s_it & 0x80)) // ascii
        {
            *d_it = uu::fold_codes[*s_it]; // works only for ascii
            ++s_it;
            ++d_it;
            ++f_size;
        }
        else
        {
            int consumed, needed;
            consumed = uu::utf8cToUtf32(&(*s_it), c32); 
            if ( consumed > 0)
            {
                needed = uu::utf32cToUtf8Fast(uu::foldUtf32c(c32), buf);
                if (needed > consumed)
                {
                    // simple folding utf-8 needs enlargement for two rare code points U+023A b'\xc8\xba' and U+023E b'\xc8\xbe' which increase in length by 1
                    if (src_copy.empty())
                    {
                        src_copy = str.substr(std::distance(str.begin(),s_it),str.length()); // copy rest of string starting from current! position
                        // point src iterators to new string
                        s_it = src_copy.begin();
                        s_endit = src_copy.end();
                    }

                    str.resize(str.length()+(needed-consumed));
                    d_it = str.begin()+f_size;  // renew iterator as increasing str length can invalidate it
                }
                // copy folded utf-8 character to string
                switch (needed)
                {
                    case 1:
                        *d_it = buf[0];
                        break;
                    case 2:
                        *d_it = buf[0];
                        *(d_it+1) = buf[1];
                        break;
                    case 3:
                        *d_it = buf[0];
                        *(d_it+1) = buf[1];
                        *(d_it+2) = buf[2];
                        break;
                    case 4:
                        *d_it = buf[0];
                        *(d_it+1) = buf[1];
                        *(d_it+2) = buf[2];
                        *(d_it+3) = buf[3];
                }
                s_it += consumed;
                d_it += needed;
                f_size += needed;

            }
            else // skip converting incorrect characters
            {
                std::string::size_type skip = std::abs(consumed);
                
                for (auto s_skipit_end = s_it+skip; s_it != s_skipit_end; ++s_it)
                {
                    *d_it = *s_it;
                    ++d_it;
                }
                f_size += skip;
            }
        }
    }


    if (str.length() != f_size)
        str.resize(f_size);

}


void foldString(std::wstring &str)
{
    if(str.empty())
        return;

    auto s_it = str.begin();
    auto s_endit = str.end();
    char32_t c32;

    while (s_it != s_endit)
    {
        if(*s_it < 0x80) // ascii
        {
            *s_it = uu::fold_codes[*s_it]; // TODO: check if more codes apply to this optimization
            ++s_it;
        }
        else
        {
            int consumed;
            consumed = uu::utf16cToUtf32(&(*s_it), c32); 
            if ( consumed > 0)
            {
                uu::utf32cToUtf16Fast(uu::foldUtf32c(c32), &(*s_it));
                s_it += consumed;
            }
            else // skip converting incorrect characters
            {
                s_it += std::abs(consumed);
            }
        }
    }
}



// returns true when utf-8 string contains alphabetic character
// FIXME: uses std::iswalpha function which works only for unicode standard plane (code point < 0xFFFF)
bool hasAlpha(const std::string& str)
{
    const char *str_p = str.c_str();
    uint32_t c;


    while ((c = *str_p) != 0)
    {
        if (!(c & 0x80)) // ASCII
        {
            if ( (static_cast<uint32_t>(c-'A') <= static_cast<uint32_t>('Z'-'A')) || (static_cast<uint32_t>(c-'a') <= static_cast<uint32_t>('z'-'a')) )  // same as (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') when all values are unsigned !!!
                return true;
        }
        else
        {
            // convert to utf 32
            char32_t c32;
            int consumed;

            consumed = uu::utf8cToUtf32(str_p, c32);
            if (consumed > 0)
            {
                if (c32 <= 0xFFFFu && std::iswalpha(static_cast<wchar_t>(c32)))  // same as UTF-16 for this range
                    return true;

                str_p += consumed;
                continue;
            }
            else if (consumed < 0) // premature end of string or utf8 decoding error
            {
                // TODO : maybe skip bad characters ???
                return false;
            }
            else  // consumed < 0 - error
            {
                str_p += consumed;
                continue;
            }
        }

        str_p++;
    }
    return false;

}

// returns true when utf-16 string contains alphabetic character
// FIXME: uses std::iswalpha function which works only for unicode standard plane (code point < 0xFFFF)
bool hasAlpha(const std::wstring& str)
{
    const wchar_t *str_p = str.c_str();
    wchar_t c;

    while ((c = *str_p) != 0)
    {
        if (c < 0x0080) // ASCII
        {
            if ( (static_cast<uint32_t>(c - L'A') <= static_cast<uint32_t>(L'Z'-L'A')) || (static_cast<uint32_t>(c - L'a') <= static_cast<uint32_t>(L'z'-L'a')) ) // same as (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z')
                return true;
        }
        else
        {
            if(c >= 0xD800 && c <= 0xDFFF) // character is HI or low surrogate
            {
                // currently not supported due to iswalpha not supporting multibyte UTF-16 characters
            }
            else 
            {
                if(std::iswalpha(c)) return true;
            }
        }

        str_p++;
    }
    return false;
}

std::wstring utf8StrToUtf16(const std::string &str, uu::Status *status)
{
    std::wstring wstr{};

    if (!str.empty())
    {
        char32_t c32;
        int consumed, converted;
        const char *str_p = str.c_str();
        std::wstring::size_type wstr_calc_size = strLenInUtf16(str_p), size = 0;

        wstr.resize(wstr_calc_size);

        auto wstr_it = wstr.begin();
        bool valid = true;

        do {
            consumed = utf8cToUtf32(str_p, c32);
            if (consumed > 0)
            {
                if (c32 != 0)
                {
                    converted = abs( utf32cToUtf16Fast(c32, wstr_it) ); // ignore errors
                    str_p += consumed;
                    wstr_it += converted;
                    size += converted;
                }
                else  // null terminating character encountered
                {
                    break;
                }
            }
            else
            {
                *wstr_it = 0xFFFD;
                str_p += abs(consumed);
                ++wstr_it;
                ++size;

                valid = false;
            }
            
                        
        } while (true);

        if (status != nullptr)
        {
            if (size != wstr_calc_size)
                *status = INTERNAL_ERROR;
            else if (!valid)
                *status = CONVERSION_ERROR;
            else
                *status = OK;
        }

    }

    return wstr;
}


std::string utf16StrToUtf8(const std::wstring &str, uu::Status *status)
{
    std::string u8str{};
    if (!str.empty())
    {
        char32_t c32;
        int consumed, converted;
        const wchar_t *str_p = str.c_str();
        std::string::size_type wstr_calc_size = strLenInUtf8(str_p), size = 0;

        u8str.resize(wstr_calc_size);

        auto str_it = u8str.begin();
        bool valid = true;

        do {
            consumed = utf16cToUtf32(str_p, c32);
            if (consumed > 0)
            {
                if (c32 != 0)
                {
                    converted = abs( utf32cToUtf8Fast(c32, str_it) ); // ignore errors. conversion to utf32 cannot exceed UTF32 range if UTF-16 was valid
                    str_p += consumed;
                    str_it += converted;
                    size += converted;
                }
                else  // null terminating character encountered
                {
                    break;
                }
            }
            else // bad character error. return '�' character ( U+FFFD (0xEF 0xBF 0xBD) in utf8 )
            {
                *str_it = 0xEF;
                ++str_it;
                *str_it = 0xBF;
                ++str_it;
                *str_it = 0xBD;
                ++str_it;

                str_p += abs(consumed);
                ++size;

                valid = false;
            }
            
                        
        } while (true);

        if (status != nullptr)
        {
            if (size != wstr_calc_size)
                *status = INTERNAL_ERROR;
            else if (!valid)
                *status = CONVERSION_ERROR;
            else
                *status = OK;
        }
    }
    return u8str;
}




// uu::TextFile

bool TextFile::GetLine(std::string& line)
{
    char *buf_p;

    if (!file)  // file is not opened
    {
        if (!OpenForReading())
            return false;
    }

    if (feof(file))
        return false;

    if ( buf.empty())
        buf.resize(1024);

    line.clear();

    do
    {
        buf_p = fgets(buf.data(), buf.size(), file);
        if (buf_p != nullptr)
        {
            line.append(buf.data());
            
            std::string::size_type str_len = line.length();
            if (str_len > 0)
            {
                if ( line[str_len-1] == '\n' ) // str[len] points to NULL terminating character!
                {
                    line.pop_back();
                    --str_len;
                    if (str_len > 0)
                    {
                        if ( line[str_len-1] == '\r' )
                            line.pop_back();
                    }
                    break;
                }
            }
        }
        else // error
        {
            if (ferror(file))
            {
                return false;
            }
        }


    } while (!feof(file));

    // validate str
    std::size_t validated_length = -1;
    if (!utf8StrValidate(line.c_str(), &validated_length))
    {
        line = utf8BufToValidStr(line.c_str(), validated_length);
    }

    return true;            
}

bool TextFile::GetLine(std::wstring& line)
{
    std::string str;
    if (GetLine(str))
    {
        line = utf8StrToUtf16(str);
        return true;
    }
    else
        return false;
}

bool TextFile::GetLines(std::vector<std::string>& lines)
{
    std::string str;
    bool got_line = false;
  
    while (GetLine(str))
    {
        lines.emplace_back(std::move(str));
        str = {};
        got_line = true;
    };

    return true;            
}

bool TextFile::GetLines(std::vector<std::wstring>& lines)
{
    std::string str;
    bool got_line = false;
  
    while (GetLine(str))
    {
        lines.emplace_back(utf8StrToUtf16(str));
        str.clear();
        got_line = true;
    };

    return true;            
}

// write a string to file
bool TextFile::PutString(const std::string& line)
{
    if (!(file_mode == file_mode_t::WRITING)) // since writing can be destructive explicitly require explicit opening
        return false;
    if (fputs(line.c_str(), file) == EOF)
        return false;
    return true;
}

bool TextFile::PutString(const std::wstring& line)
{
    std::string out_str;
    uu::Status conv_status;
    bool status;

    out_str = utf16StrToUtf8(line, &conv_status);
    if (conv_status != uu::INTERNAL_ERROR)
    {
        status = PutString(out_str);
        return status;        
    }
    else {
        return false;
    }
}


// write a string to file
// if string is not terminated by "\n" or "\r\n" , "\r\n" will be added
bool TextFile::PutLine(const std::string& line)
{
    int rv;
    if (!(file_mode == file_mode_t::WRITING)) // since writing can be destructive explicitly require explicit opening
    {
        return false;
    }

    if (fputs(line.c_str(), file) == EOF)
        return false;

    if (line.length() > 0 )
    {
        if ( !(line[line.length()-1] == '\n'))
        {
            if (fputs("\r\n", file) == EOF)
                return false;
        }
    }
    else
    {
        if (fputs("\r\n", file) == EOF)
            return false;
    }

    return true;
}

bool TextFile::PutLine(const std::wstring& line)
{
    std::string out_str;
    uu::Status conv_status;
    bool status;

    out_str = utf16StrToUtf8(line, &conv_status);
    if (conv_status != uu::INTERNAL_ERROR)
    {
        status = PutLine(out_str);
        return status;        
    }
    else {
        return false;
    }
}

// put content of vector of strings in file terminating all lines except last one with "\r\n"
bool TextFile::PutLines(std::vector<std::string>& lines)
{
    std::size_t i,len = lines.size();
    bool rv,status;

    for ( i = 0; i < len; ++i)
    {
        if (i < len-1)
            status = PutLine(lines[i]);
        else
            status = PutString(lines[i]);

        if (!status)
            rv = false;
    }
    return rv;
}

// put content of vector of wstrings in file terminating all lines except last one with "\r\n"
bool TextFile::PutLines(std::vector<std::wstring>& lines)
{
    std::size_t i,len = lines.size();
    bool rv,status;

    for ( i = 0; i < len; ++i)
    {
        if (i < len-1)
            status = PutLine(lines[i]);
        else
            status = PutString(lines[i]);

        if (!status)
            rv = false;
    }
    return rv;
}


}; // namespace uu