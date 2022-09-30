#pragma once

#define NOMINMAX

#include <Windows.h>
#include <string>
#include <initializer_list>
#include <TlHelp32.h>
#include "../sdk.hpp"
#include <thread>
#include <Psapi.h>

#define INRANGE(x,a,b)  (x >= a && x <= b) 
#define GETBITS( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define GETBYTE( x )    (GETBITS(x[0]) << 4 | GETBITS(x[1]))

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) (g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5 + (float)(dt) / g_GlobalVars->interval_per_tick ) )

namespace Utils 
{
	std::vector<char> HexToBytes(const std::string& hex);

	std::string BytesToString(unsigned char* data, int len);

	std::vector<std::string> Split(const std::string& str, const char* delim);

	unsigned int FindInDataMap(datamap_t * pMap, const char * name);
    unsigned long FindSig(const char* module_name, const char* signature);

	void AttachConsole();
    void DetachConsole();

    bool ConsolePrint(const char* fmt, ...);
    
    char ConsoleReadKey();

    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules);

    std::uint8_t* PatternScan(void* module, const char* signature);

    void SetClantag(const char* tag);
    void SetName(const char* name);
}
