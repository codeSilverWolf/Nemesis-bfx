#include "Global.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>

#include "utilities/algorithm.h"

using namespace std;

namespace nemesis
{
    //TODO: optimize iequals and to_*_copy functions
    // remove no longer needed overloads

    // const char* to_lower_copy(const char* data)
    // {
    //     char* temp = _strdup(data);
    //     int size   = strlen(data);
    //     int i      = 0;

    //     while (i < size)
    //     {
    //         temp[i] = tolower(temp[i]);
    //         i++;
    //     }

    //     return temp;
    // }

    // const wchar_t* to_lower_copy(const wchar_t* data)
    // {
    //     wchar_t* temp = _wcsdup(data);
    //     int size      = wcslen(data);
    //     int i         = 0;

    //     while (i < size)
    //     {
    //         temp[i] = tolower(temp[i]);
    //         i++;
    //     }

    //     return temp;
    // }

    string to_lower_copy(const string& data)
    {
        string::size_type str_len = data.length();

        if(str_len == 0) return {};
        
        string tmpstr = data;
        for( string::size_type i = 0; i < str_len; i++)
        {
            tmpstr[i] = tolower(data[i]);
        }
        return tmpstr;
    }

    wstring to_lower_copy(const wstring& data)
    {
        wstring::size_type str_len = data.length();

        if(str_len == 0) return {};
        
        wstring tmpstr = data;
        for( wstring::size_type i = 0; i < str_len; i++)
        {
            tmpstr[i] = towlower(data[i]);
        }
        return tmpstr;
    }

    void to_lower(string& data)
    {
        data = to_lower_copy(data);
    }

    void to_lower(wstring& data)
    {
        data = to_lower_copy(data);
    }

    // const char* to_upper_copy(const char* data)
    // {
    //     char* temp = _strdup(data);
    //     int size   = strlen(data);
    //     int i      = 0;

    //     while (i < size)
    //     {
    //         temp[i] = toupper(temp[i]);
    //         i++;
    //     }

    //     return temp;
    // }

    // const wchar_t* to_upper_copy(const wchar_t* data)
    // {
    //     wchar_t* temp = _wcsdup(data);
    //     int size      = wcslen(data);
    //     int i         = 0;

    //     while (i < size)
    //     {
    //         temp[i] = toupper(temp[i]);
    //         i++;
    //     }

    //     return temp;
    // }

    string to_upper_copy(const string& data)
    {
        string::size_type str_len = data.length();

        if(str_len == 0) return {};
        
        string tmpstr = data;
        for( string::size_type i = 0; i < str_len; i++)
        {
            tmpstr[i] = toupper(data[i]);
        }
        return tmpstr;
    }

    wstring to_upper_copy(const wstring& data)
    {
        wstring::size_type str_len = data.length();

        if(str_len == 0) return {};
        
        wstring tmpstr = data;
        for( wstring::size_type i = 0; i < str_len; i++)
        {
            tmpstr[i] = towlower(data[i]);
        }
        return tmpstr;
    }

    void to_upper(string& data)
    {
        data = to_upper_copy(data);
    }

    void to_upper(wstring& data)
    {
        data = to_upper_copy(data);
    }

    // bool iequals(const char* l, const char* r)
    // {
    //     return strcmp(to_lower_copy(l).c_str(), to_lower_copy(r).c_str()) == 0;
    // }

    // bool iequals(const wchar_t* l, const wchar_t* r)
    // {
    //     return wcscmp(to_lower_copy(l).c_str(), to_lower_copy(r)) == 0;
    // }

    bool iequals(const string& l, const string& r)
    {
        string::size_type l_length = l.length(), r_length = r.length();

        if(l_length == r_length)
        {
            string::size_type i = 0;
            while(i < l_length)
            {
                if (tolower(l[i]) != tolower(r[i]))
                    return false;
                i++;
            }
            return true; // both strings equal
        }
        else
        {
            return false;   // strings of different length can not be equal
        }
    }

    bool iequals(const wstring& l, const wstring& r)
    {
        wstring::size_type l_length = l.length(), r_length = r.length();

        if(l_length == r_length)
        {
            wstring::size_type i = 0;
            while(i < l_length)
            {
                if (tolower(l[i]) != tolower(r[i]))
                    return false;
                i++;
            }
            return true; // both strings equal
        }
        else
        {
            return false;   // strings of different length can not be equal
        }
    }

    // bool iequals(const char* l, const string& r)
    // {
    //     return iequals(string(l), r);
    // }

    // bool iequals(const wchar_t* l, const wstring& r)
    // {
    //     return iequals(wstring(l), r);
    // }

    // bool iequals(const string& l, const char* r)
    // {
    //     return iequals(l, string(r));
    // }

    // bool iequals(const wstring& l, const wchar_t* r)
    // {
    //     return iequals(l, wstring(r));
    // }

} // namespace nemesis
