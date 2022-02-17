#ifndef READTEXTFILE_H_
#define READTEXTFILE_H_

#include <QtCore\QFile.h>
#include <filesystem>

struct FileReader
{
    QFile file;

    FileReader(const char* filename)
    {
        file.setFileName(QString::fromStdString(filename));
    }

    FileReader(std::filesystem::path filename)
    {
        file.setFileName(QString::fromStdWString(filename.wstring()));
    }

    std::filesystem::path GetFilePath() const
    {
        return std::filesystem::path(file.fileName().toStdWString());
    }

    bool GetFile()
    {
        return file.open(QIODevice::ReadOnly);
    }

    bool GetLines(std::string& line)
    {
        while (!file.atEnd())
        {
            QString qline = file.readLine();
            line          = qline.toStdString();

            while (line.length() > 0 && (line.back() == '\n' || line.back() == '\r'))
            {
                line.pop_back();
            }

            return true;
        }

        return false;
    }

    // disable std::string_view& line version
    // clangd issues warning that templine pointer is backed by object that will be soon destroyed.
    // gcc is silent so maybe it extends life of object?
    
    // bool GetLines(std::string_view& line)
    // {
    //     while (!file.atEnd())
    //     {
    //         QString qline        = file.readLine();
    //         const char* templine = qline.toStdString().c_str();
    //         line                 = std::string_view(templine);

    //         while (line.length() > 0 && (line.back() == '\n' || line.back() == '\r'))
    //         {
    //             line = std::string_view(templine, line.length() - 1);
    //         }

    //         return true;
    //     }

    //     return false;
    // }

    bool GetLines(std::wstring& line)
    {
        while (!file.atEnd())
        {
            QString qline = file.readLine();
            line          = qline.toStdWString();

            while (line.length() > 0 && (line.back() == '\n' || line.back() == '\r'))
            {
                line.pop_back();
            }

            return true;
        }

        return false;
    }

    // bool GetLines(std::wstring_view& line)
    // {
    //     while (!file.atEnd())
    //     {
    //         QString qline           = file.readLine();
    //         const wchar_t* templine = qline.toStdWString().c_str();
    //         line                    = std::wstring_view(templine);

    //         while (line.length() > 0 && (line.back() == '\n' || line.back() == '\r'))
    //         {
    //             line = std::wstring_view(templine, line.length() - 1);
    //         }

    //         return true;
    //     }

    //     return false;
    // }
};

#endif
