#pragma once

namespace Hikari
{
    class Process
    {
    public:
        Process() = default;
        Process(const std::string& name);
        ~Process();

        std::vector<uint32_t>& GetList();
        void Attach(uint32_t index = 0);
        void Detach();

        std::vector<ModuleInfo>& GetModules_External();

#ifdef WINDOWS
        HANDLE& GetHandle();

        template <typename T>
        T Read(std::uintptr_t address);

        template <typename T>
        bool Write(std::uintptr_t address, T value);
#endif

    private:
        std::vector<uint32_t> _list {};
        std::vector<ModuleInfo> _modules {};
        uint32_t _pid {};
#ifdef WINDOWS
        HANDLE _handle {};
#endif
    };
}