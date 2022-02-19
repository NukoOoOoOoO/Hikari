#pragma once

namespace Hikari
{
    class ModuleInfo
    {
    public:
        struct Segment_t
        {
            std::uintptr_t address {};
            std::uint8_t* data {};
            std::size_t size {};
            const char* name {};
        };

        ModuleInfo() = default;
        ModuleInfo(const std::string& name);
        ModuleInfo(const std::string& name, std::uintptr_t baseAddress, std::size_t size, void* handle, void* imageBytes);

        std::string Name();
        std::uintptr_t BaseAddress() const;
        std::vector<Segment_t>& Segments();
        std::size_t Size() const;

        void* GetExport(std::string_view name);
        std::uintptr_t GetExport_External(std::string_view name);

    private:
        void Get(const std::string& name);

        std::vector<Segment_t> _segments {};
        std::size_t _size {};
        std::uintptr_t _baseAddress {};
        std::string _name {};
        void* _imageBytes {};
        void* _handle {};

#ifdef WINDOWS
        PIMAGE_NT_HEADERS _ntHeader;
#endif
    };
}