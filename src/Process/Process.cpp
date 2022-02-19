#include "Hikari.h"

namespace Hikari
{
    Process::Process(const std::string& name)
    {
#ifdef WINDOWS
        auto ProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (ProcessSnap == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Failed to crate snapshot handle");

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(ProcessSnap, &entry))
        {
            if (entry.szExeFile == name)
                this->_list.push_back(entry.th32ProcessID);

            while (Process32Next(ProcessSnap, &entry))
            {
                if (entry.szExeFile == name)
                    this->_list.push_back(entry.th32ProcessID);
            }
        }

        CloseHandle(ProcessSnap);
#elif
        // TODO: Linux
#endif
    }

    Process::~Process()
    {
    }

    std::vector<uint32_t>& Process::GetList()
    {
        return this->_list;
    }

    void Process::Attach(uint32_t index)
    {
        if (index > this->_list.size())
            throw std::runtime_error("Index is larger than the size of process list");

#ifdef WINDOWS
        this->_handle = OpenProcess(PROCESS_ALL_ACCESS, false, this->_list.at(index));
        if (!this->_handle)
            throw std::runtime_error("Failed to create process handle");

        this->_pid = this->_list.at(index);

        std::cout << "Process attached." << std::endl;
#endif
    }

    void Process::Detach()
    {
#ifdef WINDOWS
        CloseHandle(this->_handle);
        this->_handle = INVALID_HANDLE_VALUE;
#endif
    }

    std::vector<ModuleInfo>& Process::GetModules_External()
    {
        if (this->_modules.empty())
        {
#ifdef WINDOWS
            auto Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->_pid);
            if (Snapshot == INVALID_HANDLE_VALUE)
                throw std::runtime_error("Failed to crate snapshot handle when getting modules");

            MODULEENTRY32 entry;
            entry.dwSize = sizeof(MODULEENTRY32);

            while (Module32Next(Snapshot, &entry))
            {
                try
                {
                    const auto bytes = VirtualAlloc(nullptr, entry.modBaseSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                    ReadProcessMemory(this->_handle, entry.modBaseAddr, bytes, entry.modBaseSize, nullptr);

                    this->_modules.push_back(ModuleInfo(entry.szModule, reinterpret_cast<std::uintptr_t>(entry.modBaseAddr), entry.modBaseSize, static_cast<void*>(bytes)));
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }

            CloseHandle(Snapshot);
#else
#endif
        }

        return this->_modules;
    }

#ifdef WINDOWS
    HANDLE& Process::GetHandle()
    {
        return this->_handle;
    }

    template <typename T>
    T Process::Read(std::uintptr_t address)
    {
        T value;
        ReadProcessMemory(this->_handle, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
        return value;
    }

    template <typename T>
    bool Process::Write(std::uintptr_t address, T value)
    {
        return WriteProcessMemory(this->_handle, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
    }
#endif
}