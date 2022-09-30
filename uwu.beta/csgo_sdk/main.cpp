#define NOMINMAX
#include <Windows.h>

#include "sdk/sdk.hpp"
#include "sdk/utils/utils.hpp"
#include "sdk/utils/input.hpp"

#include "hooks.hpp"
#include "config/config.h"

#include "functions/skins/kit_parser.h"
#include "functions/skins/skins.h"
#include "sdk/utils/anti_dbg.h"
#include "lua/Clua.h"
#include "sdk/mem_init.h"

#define CURL_STATICLIB
#include "curl/curl/curl.h"

#pragma comment(lib,"libcurl_a.lib")
#pragma comment(lib,"curl/libcurl_a.lib")

#include "sdk/utils/md.h"
#include "sdk/mem_init.h"

static bool g_Unload = false;

#pragma comment(lib,"wininet.lib")
#pragma comment(lib, "Advapi32.lib")
#include <WinInet.h>

#define HOST2 _("f0665949.xsph.ru")
#define PATH _("/")

size_t WriteToStr( void* ptr, size_t size, size_t count, void* stream ) {
    (( std::string* )stream)->append( ( char* )ptr, 0, size * count );
    return size * count;
}

std::string GET( std::string link ) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init( );
    std::string response;
    if ( curl ) {
        curl_easy_setopt( curl, CURLOPT_URL, link );

        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteToStr );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &response );

        res = curl_easy_perform( curl );
        curl_easy_cleanup( curl );
    }
    return response;
}

__forceinline std::string GetHwid( )
{
    DWORD dwVolumeSerialNumber;
    std::string ret;
    if ( GetVolumeInformationA( _( "C:\\" ), nullptr, 0, &dwVolumeSerialNumber, nullptr, nullptr, nullptr, 0 ) )
        ret = md5( md5( std::to_string( (dwVolumeSerialNumber << 3) - 4 ) ) );
    else
        ret = std::string( );

    return ret;
}

DWORD WINAPI OnDllAttach( LPVOID base )
{
    while ( !GetModuleHandleA( "serverbrowser.dll" ) )
        Sleep( 1000 );

#ifdef _DEBUG
    Utils::AttachConsole( );
#endif

    try
    {
        bool isSecured = true;

        if ( !isSecured )
        {
            std::string m_User;
            std::string m_Pass;

            std::ifstream in( _( "C:\\rec.ini" ) );
            std::string line;
            std::string dataf[ ] = { "", "", "" };

            if ( in.is_open( ) )
            {
                int i = 0;
                while ( getline( in, line ) )
                {
                    dataf[ i ] = line;
                    i++;
                }
                m_User = dataf[ 0 ];
                m_Pass = dataf[ 1 ];
            }
            in.close( );

            std::string m_ConvertedUser = m_User;
            std::string m_ConvertedPassword = m_Pass;

            std::string m_Valid = _( "https://uwucsgo.xyz/check.php?" );
            m_Valid.append( _( "username=" ) );
            m_Valid.append( m_ConvertedUser );
            m_Valid.append( _( "&password=" ) );
            m_Valid.append( m_ConvertedPassword );
            m_Valid.append( _( "&auth" ) );

            if ( GET( m_Valid ) == _( "valid" ) )
            {
                std::string m_NewRequest = _( "https://uwucsgo.xyz/check.php?" );
                m_NewRequest.append( _( "username=" ) );
                m_NewRequest.append( m_ConvertedUser );
                m_NewRequest.append( _( "&password=" ) );
                m_NewRequest.append( m_ConvertedPassword );
                m_NewRequest.append( _( "&hwid=" ) );
                m_NewRequest.append( GetHwid( ) );

                if ( GET( m_NewRequest ) == _( "success" ) )
                {
                    std::string m_LastRequest = _( "https://uwucsgo.xyz/check.php?" );
                    m_LastRequest.append( _( "username=" ) );
                    m_LastRequest.append( m_ConvertedUser );
                    m_LastRequest.append( _( "&password=" ) );
                    m_LastRequest.append( m_ConvertedPassword );
                    m_LastRequest.append( _( "&sub" ) );

                    if ( GET( m_LastRequest ) != _( "nosub" ) ) {
                        isSecured = true;
                    }
                    else
                    {
                        exit( EXIT_FAILURE );
                    }
                }
                else
                {
                    exit( EXIT_FAILURE );
                }
            }
            else {
                exit( EXIT_FAILURE );
            }
        }

        Interfaces::Initialize( );

        g_ConfigMaster.setup_config( );
        g_ConfigMaster.config_files( );

        NetvarSys::Get( ).Initialize( );
        InputSys::Get( ).Initialize( );

        c_lua::Get( ).initialize( );
        memory.initialize( );

        Hooks::Initialize( );
        initialize_kits( );

        g_ConfigMaster.setup_skin_clr( );
        g_ConfigMaster.skin_config_files( );
      
        InputSys::Get( ).RegisterHotkey( VK_DELETE, [ base ]( ) {
            g_Unload = true;
        } );

        while ( !g_Unload )
            std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

        FreeLibraryAndExitThread( static_cast< HMODULE >(base), 1 );
    }

    catch ( const std::exception& ex ) {
        FreeLibraryAndExitThread( static_cast< HMODULE >(base), 1 );
    }
}

BOOL WINAPI OnDllDetach( )
{
#ifdef _DEBUG
    Utils::DetachConsole( );
#endif

    Hooks::Shutdown( );

    return TRUE;
}

BOOL WINAPI DllMain( _In_ HINSTANCE hinstDll, _In_ DWORD fdwReason, _In_opt_ LPVOID lpvReserved )
{
    switch ( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls( hinstDll );
        CreateThread( nullptr, 0, OnDllAttach, hinstDll, 0, nullptr );
        return TRUE;
    case DLL_PROCESS_DETACH:
        if ( lpvReserved == nullptr )
            return OnDllDetach( );
        return TRUE;
    default:
        return TRUE;
    }
}
