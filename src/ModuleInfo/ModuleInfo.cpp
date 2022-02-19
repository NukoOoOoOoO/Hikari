#include "Hikari.h"

namespace Hikari
{
    ModuleInfo::ModuleInfo(const std::string& name)
    {
        Get(name);
    }

    ModuleInfo::ModuleInfo(const std::string& name, std::uintptr_t baseAddress, std::size_t size, void* handle, void* imageBytes)
    {
#ifdef WINDOWS
        this->_name        = name;
        this->_baseAddress = baseAddress;
        this->_size        = size;
        this->_handle      = handle;
        this->_imageBytes  = imageBytes;

        const auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(imageBytes);

        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
            throw std::runtime_error(std::format("Invalid dos magic for {}", name));

        const auto ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(imageBytes) + dosHeader->e_lfanew);

        if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
            throw std::runtime_error(std::format("Invalid nt signature for {}", name));

        this->_ntHeader = ntHeader;

        auto section = IMAGE_FIRST_SECTION(ntHeader);

        for (auto i = 0; i < ntHeader->FileHeader.NumberOfSections; i++, section++)
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
#else
        // TODO: Linux
#endif
    }

    void ModuleInfo::Get(const std::string& name)
    {
#ifdef WINDOWS
        const auto handle = GetModuleHandleA(name.c_str());
        if (!handle)
            throw std::runtime_error(std::format("Failed to get module info for {}", name));

        const auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);

        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
            throw std::runtime_error(std::format("Invalid dos magic for {}", name));

        const auto ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(handle) + dosHeader->e_lfanew);

        if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
            throw std::runtime_error(std::format("Invalid nt signature for {}", name));

        this->_name        = name;
        this->_handle      = handle;
        this->_baseAddress = reinterpret_cast<std::uintptr_t>(handle);
        this->_size        = ntHeader->OptionalHeader.SizeOfImage;

        auto section = IMAGE_FIRST_SECTION(ntHeader);

        for (auto i = 0; i < ntHeader->FileHeader.NumberOfSections; i++, section++)
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
        const auto address = GetProcAddress(static_cast<HMODULE>(this->_handle), name.data());
        if (address)
            return address;

        return nullptr;
#else
        // TODO: Linux
        return nullptr;
#endif
    }

    std::uintptr_t ModuleInfo::GetExport_External(std::string_view name)
    {
#ifdef WINDOWS

        PIMAGE_EXPORT_DIRECTORY exportDir {};
        std::uintptr_t exportBaseAddress {};
        std::uintptr_t exportSize {};

        // We treat things differerntly ok? lmao
        if (this->_ntHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            auto nt32         = reinterpret_cast<PIMAGE_NT_HEADERS32>(_ntHeader);
            exportBaseAddress = nt32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
            exportSize        = nt32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

            exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<std::uintptr_t>(this->_imageBytes) + exportBaseAddress);
        }
        else if (this->_ntHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            auto nt64         = reinterpret_cast<PIMAGE_NT_HEADERS64>(_ntHeader);
            exportBaseAddress = nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
            exportSize        = nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

            exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<std::uintptr_t>(this->_imageBytes) + exportBaseAddress);
        }
        else
        {
            return 0;
        }

        auto funcs    = reinterpret_cast<std::uint32_t*>(exportDir->AddressOfFunctions + reinterpret_cast<std::uintptr_t>(this->_imageBytes));
        auto names    = reinterpret_cast<std::uint32_t*>(exportDir->AddressOfNames + reinterpret_cast<std::uintptr_t>(this->_imageBytes));
        auto ordinals = reinterpret_cast<std::uint16_t*>(exportDir->AddressOfNameOrdinals + reinterpret_cast<std::uintptr_t>(this->_imageBytes));

        for (auto i = 0u; i < exportDir->NumberOfNames; i++)
        {
            const auto exportName = std::string_view(reinterpret_cast<char*>(names[i] + reinterpret_cast<std::uintptr_t>(this->_imageBytes)));
            if (name == exportName)
            {
                const auto exportAddress = funcs[ordinals[i]];

                if (reinterpret_cast<std::uintptr_t>(this->_imageBytes) + exportAddress >= reinterpret_cast<std::uintptr_t>(exportDir)
                    && reinterpret_cast<std::uintptr_t>(this->_imageBytes) + exportAddress <= reinterpret_cast<std::uintptr_t>(exportDir) + exportSize)
                    return 0;

                return this->_baseAddress + exportAddress;
            }
        }

        return 0;
#else
        // TODO: Linux
        return 0;
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