#include "../include/Hikari.h"
#include <iostream>

int main()
{
    try
    {
        std::cout << "Hello world" << std::endl;

        auto temp_utf8 = Hikari::String::ToUtf8(L"Hello from wstring");
        std::cout << temp_utf8 << std::endl;

        auto temp_unicode = Hikari::String::ToUnicode("Hello from utf8");
        std::wcout << temp_unicode << std::endl;

        std::string example_string = "The quick brown fox jumps over the lazy dog";
        {
            auto splited = Hikari::String::Split(example_string, " ");
            std::cout << "Splited size: " << splited.size() << " | Strings: ";
            for (auto&& s : splited)
            {
                std::cout << s << " ";
            }
            std::cout << std::endl;

            std::cout << "Lowercased: " << Hikari::String::Lowercase(example_string) << std::endl;
            std::cout << "Uppercased: " << Hikari::String::Uppercase(example_string) << std::endl;
        }

        auto target_process = Hikari::Process("csgo.exe");
        std::cout << "ProcessSize: " << target_process.GetList().size() << std::endl;
        target_process.Attach();
        auto process_modules = target_process.GetModules_External();
        std::cout << "ModuleList: " << process_modules.size() << std::endl;
        for (auto&& mod : process_modules)
        {
            auto name = mod.Name();
            std::cout << "    " << name;

            if (name == "client.dll" || name == "engine.dll")
            {
                auto segments = mod.Segments();
                std::cout << " | segments.size(): " << segments.size() << ", segments.front().name: " << segments.front().name << ", segments.front().address: 0x" << std::hex
                          << segments.front().address;
                std::cout << ", segments.front().size: " << segments.front().size;
            }

            std::cout << std::endl;
        }
    }
    catch (std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        exit(1);
    }

    return 0;
}