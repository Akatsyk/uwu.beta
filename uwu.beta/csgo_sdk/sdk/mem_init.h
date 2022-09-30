#pragma once

#include "sdk.hpp"
#include "utils/utils.hpp"

template <typename First, typename Second>
struct NodeBrk {
    int previousId;
    int nextId;
    void* _unknownPtr;
    int _unknown;
    First key;
    Second value;
};

template <typename First, typename Second>
struct HeadQua {
    NodeBrk<First, Second>* memory;
    int allocationCount;
    int growSize;
    int startElement;
    int nextAvailable;
    int _unknown;
    int lastElement;
};

class String_t
{
public:
    char* m_pszString;
    int m_iUnknown0, m_iUnknown1;
    int m_iLength;
};

class CPaintKit
{
public:
    int nID;
    String_t sName;
    String_t sDescriptionString;
    String_t sDescriptionTag;
    String_t pad;
    String_t pattern;
    String_t pad1;
    String_t sLogoMaterial;
    int bBaseDiffuseOverride;
    int rarity;
    int nStyle;
    int color1;
    int color2;
    int color3;
    int color4;
    int logoColor1;
    int logoColor2;
    int logoColor3;
    int logoColor4;
    float flWearDefault;
    float flWearRemapMin;
    float flWearRemapMax;
    char nFixedSeed;
    char uchPhongExponent;
    char uchPhongAlbedoBoost;
    char uchPhongIntensity;
    float flPatternScale;
    float flPatternOffsetXStart;
    float flPatternOffsetXEnd;
    float flPatternOffsetYStart;
    float flPatternOffsetYEnd;
    float flPatternRotateStart;
    float flPatternRotateEnd;
    float flLogoScale;
    float flLogoOffsetX;
    float flLogoOffsetY;
    float flLogoRotation;
    int bIgnoreWeaponSizeScale;
    int nViewModelExponentOverrideSize;
    int bOnlyFirstMaterial;
    float pearlescent;
    int sVmtPath[ 4 ];
    int kvVmtOverrides;
};

class ItemSchema {
    void* vmt;
    std::byte pad[ 0x28C ];
public:
    HeadQua<int, CPaintKit*> paintKits;
private:
    std::byte pad1[ 0x8 ];
public:
    int stickerKits;
};

class Memory {
public:

    void initialize() noexcept;
    std::add_pointer_t<ItemSchema* __cdecl()> itemSchema;

private:
    static std::uintptr_t findPattern( const char* module, const char* pattern, size_t offset = 0 ) noexcept
    {
        static auto id = 0;
        ++id;

        if ( MODULEINFO moduleInfo; GetModuleInformation( GetCurrentProcess(), GetModuleHandle( module ), &moduleInfo, sizeof( moduleInfo ) ) ) {
            auto start = static_cast< const char* >(moduleInfo.lpBaseOfDll);
            const auto end = start + moduleInfo.SizeOfImage;

            auto first = start;
            auto second = pattern;

            while ( first < end && *second ) {
                if ( *first == *second || *second == '?' ) {
                    ++first;
                    ++second;
                }
                else {
                    first = ++start;
                    second = pattern;
                }
            }

            if ( !*second )
                return reinterpret_cast< std::uintptr_t >(const_cast< char* >(start) + offset);
        }

        return 0;
    }
};

extern Memory memory;