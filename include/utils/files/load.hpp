#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace utils::files
{
    /**
     * Прочесть файл как текст
     * @param path Путь к файлу
     * @return Строка с содержимым файла
     */
    inline std::string load_as_text(const std::string& path)
    {
        if(!std::filesystem::exists(path))
        {
            throw std::runtime_error("Cant open file \"" + path + "\"");
        }

        std::ifstream ifstream;
        ifstream.open(path);

        if (!ifstream.fail())
        {
            std::stringstream ss;
            ss << ifstream.rdbuf();
            ifstream.close();

            return ss.str();
        }

        return "";
    }

    /**
     * Прочесть файл в бинарном виде
     * @param path Путь к файлу
     * @return Массив байт
     */
    inline std::vector<unsigned char> load_as_bytes(const std::string& path)
    {
        if(!std::filesystem::exists(path))
        {
            throw std::runtime_error("Cant open file \"" + path + "\"");
        }

        if (std::ifstream is(path.c_str(), std::ios::binary | std::ios::in | std::ios::ate); is.is_open())
        {
            is.seekg(0,std::ios::end);
            const auto size = static_cast<int>(is.tellg());
            const auto data = new char[size];
            is.seekg(0, std::ios::beg);
            is.read(data, size);
            is.close();
            std::vector<unsigned char> result(data, data + size);
            return result;
        }

        return {};
    }
}