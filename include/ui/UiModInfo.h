#pragma once

#include <string>

class UiModInfo
{
    std::wstring name;
    std::wstring author;
    std::wstring automatic;

public:
    UiModInfo(const std::wstring& _name, const std::wstring& _author, const std::wstring& _auto);

    std::string getNameA() const;
    std::string getAuthorA() const;
    std::string getAutomaticA() const;

    const std::wstring& getName() const;
    const std::wstring& getAuthor() const;
    const std::wstring& getAutomatic() const;
};
