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

        auto csgo_process = Hikari::Process("csgo.exe");
        auto process_list = csgo_process.GetList();
        std::cout << "ProcessSize: " << process_list.size() << std::endl;

        if (!process_list.empty())
        {
            csgo_process.Attach();
            auto process_modules = csgo_process.GetModules_External();
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
                    std::cout << ", segments.front().size: " << std::dec << segments.front().size;

                    auto create_interface = mod.GetExport_External("CreateInterface");
                    std::cout << ", CreateInterface: 0x" << std::hex << create_interface << ", baseAddress: 0x" << mod.BaseAddress() << ", endAddress: 0x" << mod.BaseAddress() + mod.Size();
                }

                std::cout << std::endl;
            }
        }
    }
    catch (std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        exit(1);
    }

    return 0;
}