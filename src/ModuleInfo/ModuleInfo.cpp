#include "Hikari.h"

namespace Hikari
{
    ModuleInfo::ModuleInfo(const std::string& name)
    {
        Get(name);
    }

    ModuleInfo::ModuleInfo(const std::string& name, std::uintptr_t baseAddress, std::size_t size, void* handle)
    {
        this->_name        = name;
        this->_baseAddress = baseAddress;
        this->_size        = size;
        this->_handle      = handle;

        const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);

        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
            throw std::runtime_error(std::format("Invalid dos magic for {}", name));

        const auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(handle) + dos_header->e_lfanew);

        if (nt_header->Signature != IMAGE_NT_SIGNATURE)
            throw std::runtime_error(std::format("Invalid nt signature for {}", name));

        auto section = IMAGE_FIRST_SECTION(nt_header);

        for (auto i = 0; i < nt_header->FileHeader.NumberOfSections; i++, section++)
        {
            // Basically getting .text section, just in case some sections are rx but its name is not .text
            const auto is_executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
            const auto is_readable   = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;

            if (is_executable && is_readable)
            {
                // Here we are using baseAddress instead of the address of handle
                // because we are getting module bytes by allocating new memory area and reading from target process
                const auto start = baseAddress + section->VirtualAddress;
                const auto size  = (std::min)(section->SizeOfRawData, section->Misc.VirtualSize);
                this->_segments.emplace_back(start, reinterpret_cast<std::uint8_t*>(start), size, reinterpret_cast<const char*>(section->Name));
            }
        }
    }

    void ModuleInfo::Get(const std::string& name)
    {
#ifdef WINDOWS
        const auto handle = GetModuleHandleA(name.c_str());
        if (!handle)
            throw std::runtime_error(std::format("Failed to get module info for {}", name));

        const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);

        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
            throw std::runtime_error(std::format("Invalid dos magic for {}", name));

        const auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(handle) + dos_header->e_lfanew);

        if (nt_header->Signature != IMAGE_NT_SIGNATURE)
            throw std::runtime_error(std::format("Invalid nt signature for {}", name));

        this->_name        = name;
        this->_handle      = handle;
        this->_baseAddress = reinterpret_cast<std::uintptr_t>(handle);
        this->_size        = nt_header->OptionalHeader.SizeOfImage;

        auto section = IMAGE_FIRST_SECTION(nt_header);

        for (auto i = 0; i < nt_header->FileHeader.NumberOfSections; i++, section++)
        {
            // Basically getting .text section, but just in case some sections are rx but its name is not .text
            const auto is_executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
            const auto is_readable   = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;

            if (is_executable && is_readable)
            {
                const auto start = reinterpret_cast<std::uintptr_t>(handle) + section->VirtualAddress;
                const auto size  = (std::min)(section->SizeOfRawData, section->Misc.VirtualSize);
                this->_segments.emplace_back(start, reinterpret_cast<std::uint8_t*>(start), size);
            }
        }
#else
        // TODO: Linux
#endif
    }

    void* ModuleInfo::GetExport(std::string_view name)
    {
        if (!this->_handle)
            throw std::runtime_error(std::format("Invalid module handle for {} when calling ModuleInfo::GetExport", this->_name));

#ifdef WINDOWS
        if (const auto address = GetProcAddress(static_cast<HMODULE>(this->_handle), name.data()); address)
        {
            return address;
        }

        return nullptr;
#else
        // TODO: Linux
        return nullptr;
#endif
    }

    std::string ModuleInfo::Name()
    {
        return this->_name;
    }

    std::uintptr_t ModuleInfo::BaseAddress() const
    {
        return this->_baseAddress;
    }

    std::size_t ModuleInfo::Size() const
    {
        return this->_size;
    }

    std::vector<ModuleInfo::Segment_t>& ModuleInfo::Segments()
    {
        return this->_segments;
    }
}