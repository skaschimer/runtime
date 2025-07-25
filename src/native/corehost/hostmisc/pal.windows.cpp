// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#include "pal.h"
#include "trace.h"
#include "utils.h"
#include "longfile.h"
#include <windows.h>
#include <cassert>
#include <ShlObj.h>
#include <ctime>

void pal::file_vprintf(FILE* f, const pal::char_t* format, va_list vl)
{
    // String functions like vfwprintf convert wide to multi-byte characters as if wcrtomb were called - that is, using the current C locale (LC_TYPE).
    // In order to properly print UTF-8 and GB18030 characters, we need to use the version of vfwprintf that takes a locale.
    _locale_t loc = _create_locale(LC_ALL, ".utf8");
    ::_vfwprintf_l(f, format, loc, vl);
    ::fputwc(_X('\n'), f);
    _free_locale(loc);
}

namespace
{
    void print_line_to_handle(const pal::char_t* message, HANDLE handle, FILE* fallbackFileHandle) {
        // String functions like vfwprintf convert wide to multi-byte characters as if wcrtomb were called - that is, using the current C locale (LC_TYPE).
        // In order to properly print UTF-8 and GB18030 characters to the console without requiring the user to use chcp to a compatible locale, we use WriteConsoleW.
        // However, WriteConsoleW will fail if the output is redirected to a file - in that case we will write to the fallbackFileHandle
        DWORD output;
        // GetConsoleMode returns FALSE when the output is redirected to a file, and we need to output to the fallback file handle.
        BOOL isConsoleOutput = ::GetConsoleMode(handle, &output);
        if (isConsoleOutput == FALSE)
        {
            // We use file_vprintf to handle UTF-8 formatting. The WriteFile api will output the bytes directly with Unicode bytes,
            // while pal::file_vprintf will convert the characters to UTF-8.
            pal::file_vprintf(fallbackFileHandle, message, va_list());
        }
        else {
            ::WriteConsoleW(handle, message, (int)pal::strlen(message), NULL, NULL);
            ::WriteConsoleW(handle, _X("\n"), 1, NULL, NULL);
        }
    }
}

void pal::err_print_line(const pal::char_t* message)
{
    // Forward to helper to handle UTF-8 formatting and redirection
    print_line_to_handle(message, ::GetStdHandle(STD_ERROR_HANDLE), stderr);
}

void pal::out_vprint_line(const pal::char_t* format, va_list vl)
{
    va_list vl_copy;
    va_copy(vl_copy, vl);
    // Get the length of the formatted string + 1 for null terminator
    int len = 1 + pal::strlen_vprintf(format, vl_copy);
    if (len < 0)
    {
        return;
    }
    std::vector<pal::char_t> buffer(len);
    int written = pal::str_vprintf(&buffer[0], len, format, vl);
    if (written != len - 1)
    {
        return;
    }
    // Forward to helper to handle UTF-8 formatting and redirection
    print_line_to_handle(&buffer[0], ::GetStdHandle(STD_OUTPUT_HANDLE), stdout);
}

namespace
{
    typedef DWORD(WINAPI *get_temp_path_func_ptr)(DWORD buffer_len, LPWSTR buffer);
    static volatile get_temp_path_func_ptr s_get_temp_path_func = nullptr;

    DWORD get_temp_path(DWORD buffer_len, LPWSTR buffer)
    {
        if (s_get_temp_path_func == nullptr)
        {
            HMODULE kernel32 = ::LoadLibraryExW(L"kernel32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);

            get_temp_path_func_ptr get_temp_path_func_local = NULL;
            if (kernel32 != NULL)
            {
                // store to thread local variable to prevent data race
                get_temp_path_func_local = (get_temp_path_func_ptr)::GetProcAddress(kernel32, "GetTempPath2W");
            }

            if (get_temp_path_func_local == NULL) // method is only available with Windows 10 Creators Update or later
            {
                s_get_temp_path_func = &GetTempPathW;
            }
            else
            {
                s_get_temp_path_func = get_temp_path_func_local;
            }
        }

        return s_get_temp_path_func(buffer_len, buffer);
    }

    bool GetModuleFileNameWrapper(HMODULE hModule, pal::string_t* recv)
    {
        pal::string_t path;
        size_t dwModuleFileName = MAX_PATH / 2;

        do
        {
            path.resize(dwModuleFileName * 2);
            dwModuleFileName = GetModuleFileNameW(hModule, (LPWSTR)path.data(), static_cast<DWORD>(path.size()));
        } while (dwModuleFileName == path.size());

        if (dwModuleFileName == 0)
            return false;

        path.resize(dwModuleFileName);
        recv->assign(path);
        return true;
    }

    bool GetModuleHandleFromAddress(void *addr, HMODULE *hModule)
    {
        BOOL res = ::GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(addr),
            hModule);

        return (res != FALSE);
    }
}

pal::string_t pal::get_timestamp()
{
    std::time_t t = std::time(nullptr);
    const std::size_t elems = 100;
    char_t buf[elems];

    tm tm_l{};
    ::gmtime_s(&tm_l, &t);
    std::wcsftime(buf, elems, _X("%c GMT"), &tm_l);

    return pal::string_t(buf);
}

bool pal::touch_file(const pal::string_t& path)
{
    HANDLE hnd = ::CreateFileW(path.c_str(), 0, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hnd == INVALID_HANDLE_VALUE)
    {
        trace::verbose(_X("Failed to leave breadcrumb, HRESULT: 0x%X"), HRESULT_FROM_WIN32(GetLastError()));
        return false;
    }
    ::CloseHandle(hnd);
    return true;
}

static void* map_file(const pal::string_t& path, size_t *length, DWORD mapping_protect, DWORD view_desired_access)
{
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
    {
        trace::error(_X("Failed to map file. CreateFileW(%s) failed with error %d"), path.c_str(), GetLastError());
        return nullptr;
    }

    if (length != nullptr)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(file, &fileSize) == 0)
        {
            trace::error(_X("Failed to map file. GetFileSizeEx(%s) failed with error %d"), path.c_str(), GetLastError());
            CloseHandle(file);
            return nullptr;
        }
        *length = (size_t)fileSize.QuadPart;
    }

    HANDLE map = CreateFileMappingW(file, NULL, mapping_protect, 0, 0, NULL);

    if (map == NULL)
    {
        trace::error(_X("Failed to map file. CreateFileMappingW(%s) failed with error %d"), path.c_str(), GetLastError());
        CloseHandle(file);
        return nullptr;
    }

    void *address = MapViewOfFile(map, view_desired_access, 0, 0, 0);

    if (address == NULL)
    {
        trace::error(_X("Failed to map file. MapViewOfFile(%s) failed with error %d"), path.c_str(), GetLastError());
    }

    // The file-handle (file) and mapping object handle (map) can be safely closed
    // once the file is mapped. The OS keeps the file open if there is an open mapping into the file.

    CloseHandle(map);
    CloseHandle(file);

    return address;
}

const void* pal::mmap_read(const string_t& path, size_t* length)
{
    return map_file(path, length, PAGE_READONLY, FILE_MAP_READ);
}

void* pal::mmap_copy_on_write(const string_t& path, size_t* length)
{
    return map_file(path, length, PAGE_WRITECOPY, FILE_MAP_READ | FILE_MAP_COPY);
}

bool pal::getcwd(pal::string_t* recv)
{
    recv->clear();

    pal::char_t buf[MAX_PATH];
    DWORD result = GetCurrentDirectoryW(MAX_PATH, buf);
    if (result < MAX_PATH)
    {
        recv->assign(buf);
        return true;
    }
    else if (result != 0)
    {
        std::vector<pal::char_t> str;
        str.resize(result);
        result = GetCurrentDirectoryW(static_cast<uint32_t>(str.size()), str.data());
        assert(result <= str.size());
        if (result != 0)
        {
            recv->assign(str.data());
            return true;
        }
    }
    assert(result == 0);
    trace::error(_X("Failed to obtain working directory, HRESULT: 0x%X"), HRESULT_FROM_WIN32(GetLastError()));
    return false;
}

bool pal::get_loaded_library(
    const char_t *library_name,
    const char *symbol_name,
    /*out*/ dll_t *dll,
    /*out*/ pal::string_t *path)
{
    dll_t dll_maybe = ::GetModuleHandleW(library_name);
    if (dll_maybe == nullptr)
        return false;

    *dll = dll_maybe;
    return pal::get_module_path(*dll, path);
}

bool pal::load_library(const string_t* in_path, dll_t* dll)
{
    string_t path = *in_path;

    // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR:
    //   In framework-dependent apps, coreclr would come from another directory than the host,
    //   so make sure coreclr dependencies can be resolved from coreclr.dll load dir.

    if (LongFile::IsPathNotFullyQualified(path))
    {
        if (!pal::fullpath(&path))
        {
            trace::error(_X("Failed to load [%s], HRESULT: 0x%X"), path.c_str(), HRESULT_FROM_WIN32(GetLastError()));
            return false;
        }
    }

    //Adding the assert to ensure relative paths which are not just filenames are not used for LoadLibrary Calls
    assert(!LongFile::IsPathNotFullyQualified(path) || !LongFile::ContainsDirectorySeparator(path));

    *dll = ::LoadLibraryExW(path.c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (*dll == nullptr)
    {
        int error_code = ::GetLastError();
        trace::error(_X("Failed to load [%s], HRESULT: 0x%X"), path.c_str(), HRESULT_FROM_WIN32(error_code));
        if (error_code == ERROR_BAD_EXE_FORMAT)
        {
            trace::error(_X("  - Ensure the library matches the current process architecture: ") _STRINGIFY(CURRENT_ARCH_NAME));
        }

        return false;
    }

    // Pin the module
    HMODULE dummy_module;
    if (!::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, path.c_str(), &dummy_module))
    {
        trace::error(_X("Failed to pin library [%s] in [%s]"), path.c_str(), _STRINGIFY(__FUNCTION__));
        return false;
    }

    if (trace::is_enabled())
    {
        string_t buf;
        GetModuleFileNameWrapper(*dll, &buf);
        trace::info(_X("Loaded library from %s"), buf.c_str());
    }

    return true;
}

pal::proc_t pal::get_symbol(dll_t library, const char* name)
{
    auto result = ::GetProcAddress(library, name);
    if (result == nullptr)
    {
        trace::info(_X("Probed for and did not resolve library symbol %S"), name);
    }

    return result;
}

void pal::unload_library(dll_t library)
{
    // No-op. On windows, we pin the library, so it can't be unloaded.
}

static
bool get_wow_mode_program_files(pal::string_t* recv)
{
#if defined(TARGET_AMD64)
    const pal::char_t* env_key = _X("ProgramFiles(x86)");
#else
    const pal::char_t* env_key = _X("ProgramFiles");
#endif

    return get_file_path_from_env(env_key,recv);
}

bool pal::get_default_breadcrumb_store(string_t* recv)
{
    recv->clear();

    pal::string_t prog_dat;
    if (!get_file_path_from_env(_X("ProgramData"), &prog_dat))
    {
        // We should have the path in prog_dat.
        trace::verbose(_X("Failed to read default breadcrumb store [%s]"), prog_dat.c_str());
        return false;
    }
    recv->assign(prog_dat);
    append_path(recv, _X("Microsoft"));
    append_path(recv, _X("NetFramework"));
    append_path(recv, _X("BreadcrumbStore"));
    return true;
}

bool pal::get_default_servicing_directory(string_t* recv)
{
    if (!get_wow_mode_program_files(recv))
    {
        return false;
    }
    append_path(recv, _X("coreservicing"));
    return true;
}

namespace
{
    bool is_supported_multi_arch_install(pal::architecture arch)
    {
#if defined(TARGET_AMD64)
        // x64, looking for x86 install or emulating x64, looking for arm64 install
        return arch == pal::architecture::x86
            || (arch == pal::architecture::arm64 && pal::is_emulating_x64());
#elif defined(TARGET_ARM64)
        // arm64, looking for x64 install
        return arch == pal::architecture::x64;
#elif defined(TARGET_X86)
        // x86 running in WoW64, looking for x64 install
        return arch == pal::architecture::x64 && pal::is_running_in_wow64();
#else
        // Others do not support default install locations on a different architecture
        return false;
#endif
    }
}

bool pal::get_default_installation_dir(pal::string_t* recv)
{
    return get_default_installation_dir_for_arch(get_current_arch(), recv);
}

bool pal::get_default_installation_dir_for_arch(pal::architecture arch, pal::string_t* recv)
{
    //  ***Used only for testing***
    pal::string_t environmentOverride;
    if (test_only_getenv(_X("_DOTNET_TEST_DEFAULT_INSTALL_PATH"), &environmentOverride))
    {
        recv->assign(environmentOverride);
        return true;
    }
    //  ***************************

    bool is_current_arch = arch == get_current_arch();

    // Bail out early for unsupported requests for different architectures
    if (!is_current_arch && !is_supported_multi_arch_install(arch))
        return false;

    const pal::char_t* program_files_dir;
    if (is_current_arch)
    {
        program_files_dir = _X("ProgramFiles");
    }
#if defined(TARGET_AMD64)
    else if (arch == pal::architecture::x86)
    {
        // Running x64, looking for x86 install
        program_files_dir = _X("ProgramFiles(x86)");
    }
#endif
#if defined(TARGET_X86)
    else if (pal::is_running_in_wow64() && arch == pal::architecture::x64)
    {
        // Running x86 on x64, looking for x64 install
        program_files_dir = _X("ProgramW6432");
    }
#endif
    else
    {
        // Running arm64/x64, looking for x64/arm64.
        // Other cases should have bailed out based on is_supported_multi_arch_install
        program_files_dir = _X("ProgramFiles");
    }

    if (!get_file_path_from_env(program_files_dir, recv))
    {
        return false;
    }

    append_path(recv, _X("dotnet"));
    if (is_current_arch && pal::is_emulating_x64())
    {
        // Install location for emulated x64 should be %ProgramFiles%\dotnet\x64.
        append_path(recv, get_arch_name(arch));
    }
#if defined(TARGET_ARM64)
    else if (!is_current_arch)
    {
        // Running arm64, looking for x64 install
        assert(arch == pal::architecture::x64);
        append_path(recv, get_arch_name(arch));
    }
#endif

    return true;
}

namespace
{
    void get_dotnet_install_location_registry_path(pal::architecture arch, HKEY * key_hive, pal::string_t * sub_key, const pal::char_t ** value)
    {
        *key_hive = HKEY_LOCAL_MACHINE;
        // The registry search occurs in the 32-bit registry in all cases.
        pal::string_t dotnet_key_path = pal::string_t(_X("SOFTWARE\\dotnet"));

        pal::string_t environmentRegistryPathOverride;
        if (test_only_getenv(_X("_DOTNET_TEST_REGISTRY_PATH"), &environmentRegistryPathOverride))
        {
            pal::string_t hkcuPrefix = _X("HKEY_CURRENT_USER\\");
            if (environmentRegistryPathOverride.substr(0, hkcuPrefix.length()) == hkcuPrefix)
            {
                *key_hive = HKEY_CURRENT_USER;
                environmentRegistryPathOverride = environmentRegistryPathOverride.substr(hkcuPrefix.length());
            }

            dotnet_key_path = environmentRegistryPathOverride;
        }

        *sub_key = dotnet_key_path + pal::string_t(_X("\\Setup\\InstalledVersions\\")) + get_arch_name(arch);
        *value = _X("InstallLocation");
    }

    pal::string_t registry_path_as_string(const HKEY& key_hive, const pal::string_t& sub_key, const pal::char_t* value)
    {
        assert(key_hive == HKEY_CURRENT_USER || key_hive == HKEY_LOCAL_MACHINE);
        return (key_hive == HKEY_CURRENT_USER ? _X("HKCU\\") : _X("HKLM\\")) + sub_key + _X("\\") + value;
    }
}

pal::string_t pal::get_dotnet_self_registered_config_location(pal::architecture arch)
{
    HKEY key_hive;
    pal::string_t sub_key;
    const pal::char_t* value;
    get_dotnet_install_location_registry_path(arch, &key_hive, &sub_key, &value);

    return registry_path_as_string(key_hive, sub_key, value);
}

bool pal::get_dotnet_self_registered_dir(pal::string_t* recv)
{
    //  ***Used only for testing***
    pal::string_t environmentOverride;
    if (test_only_getenv(_X("_DOTNET_TEST_GLOBALLY_REGISTERED_PATH"), &environmentOverride))
    {
        recv->assign(environmentOverride);
        return true;
    }
    //  ***************************

    return get_dotnet_self_registered_dir_for_arch(get_current_arch(), recv);
}

bool pal::get_dotnet_self_registered_dir_for_arch(pal::architecture arch, pal::string_t* recv)
{
    recv->clear();

    HKEY hkeyHive;
    pal::string_t sub_key;
    const pal::char_t* value;
    get_dotnet_install_location_registry_path(arch, &hkeyHive, &sub_key, &value);

    if (trace::is_enabled())
        trace::verbose(_X("Looking for architecture-specific registry value in '%s'."), registry_path_as_string(hkeyHive, sub_key, value).c_str());

    // Must use RegOpenKeyEx to be able to specify KEY_WOW64_32KEY to access the 32-bit registry in all cases.
    // The RegGetValue has this option available only on Win10.
    HKEY hkey = NULL;
    LSTATUS result = ::RegOpenKeyExW(hkeyHive, sub_key.c_str(), 0, KEY_READ | KEY_WOW64_32KEY, &hkey);
    if (result != ERROR_SUCCESS)
    {
        if (result == ERROR_FILE_NOT_FOUND)
        {
            trace::verbose(_X("The registry key ['%s'] does not exist."), sub_key.c_str());
        }
        else
        {
            trace::verbose(_X("Failed to open the registry key. Error code: 0x%X"), result);
        }

        return false;
    }

    // Determine the size of the buffer
    DWORD size = 0;
    result = ::RegGetValueW(hkey, nullptr, value, RRF_RT_REG_SZ, nullptr, nullptr, &size);
    if (result != ERROR_SUCCESS || size == 0)
    {
        trace::verbose(_X("Failed to get the size of the install location registry value or it's empty. Error code: 0x%X"), result);
        ::RegCloseKey(hkey);
        return false;
    }

    // Get the key's value
    std::vector<pal::char_t> buffer(size/sizeof(pal::char_t));
    result = ::RegGetValueW(hkey, nullptr, value, RRF_RT_REG_SZ, nullptr, &buffer[0], &size);
    if (result != ERROR_SUCCESS)
    {
        trace::verbose(_X("Failed to get the value of the install location registry value. Error code: 0x%X"), result);
        ::RegCloseKey(hkey);
        return false;
    }

    recv->assign(buffer.data());
    ::RegCloseKey(hkey);
    trace::verbose(_X("Found registered install location '%s'."), recv->c_str());
    return true;
}

bool pal::get_global_dotnet_dirs(std::vector<pal::string_t>* dirs)
{
    pal::string_t default_dir;
    pal::string_t custom_dir;
    bool dir_found = false;
    if (pal::get_dotnet_self_registered_dir(&custom_dir))
    {
        remove_trailing_dir_separator(&custom_dir);
        dirs->push_back(custom_dir);
        dir_found = true;
    }
    if (get_default_installation_dir(&default_dir))
    {
        remove_trailing_dir_separator(&default_dir);

        // Avoid duplicate global dirs.
        if (!dir_found || !are_paths_equal_with_normalized_casing(custom_dir, default_dir))
        {
            dirs->push_back(default_dir);
            dir_found = true;
        }
    }
    return dir_found;
}

// To determine the OS version, we are going to use RtlGetVersion API
// since GetVersion call can be shimmed on Win8.1+.
typedef LONG (WINAPI *pFuncRtlGetVersion)(RTL_OSVERSIONINFOW *);

pal::string_t pal::get_current_os_rid_platform()
{
    pal::string_t ridOS;

    RTL_OSVERSIONINFOW osinfo;

    // Init the buffer
    ZeroMemory(&osinfo, sizeof(osinfo));
    osinfo.dwOSVersionInfoSize = sizeof(osinfo);
    HMODULE hmodNtdll = LoadLibraryA("ntdll.dll");
    if (hmodNtdll != NULL)
    {
        pFuncRtlGetVersion pRtlGetVersion = (pFuncRtlGetVersion)GetProcAddress(hmodNtdll, "RtlGetVersion");
        if (pRtlGetVersion)
        {
            if ((*pRtlGetVersion)(&osinfo) == 0)
            {
                // Win7 RID is the minimum supported version.
                uint32_t majorVer = 6;
                uint32_t minorVer = 1;

                if (osinfo.dwMajorVersion > majorVer)
                {
                    majorVer = osinfo.dwMajorVersion;

                    // Reset the minor version since we picked a different major version.
                    minorVer = 0;
                }

                if (osinfo.dwMinorVersion > minorVer)
                {
                    minorVer = osinfo.dwMinorVersion;
                }

                if (majorVer == 6)
                {
                    switch(minorVer)
                    {
                        case 1:
                            ridOS.append(_X("win7"));
                            break;
                        case 2:
                            ridOS.append(_X("win8"));
                            break;
                        case 3:
                        default:
                            // For unknown version, we will support the highest RID that we know for this major version.
                            ridOS.append(_X("win81"));
                            break;
                    }
                }
                else if (majorVer >= 10)
                {
                    // Return the major version for use in RID computation without applying any cap.
                    ridOS.append(_X("win"));
                    ridOS.append(pal::to_string(majorVer));
                }
            }
        }
    }

    return ridOS;
}

namespace
{
    bool is_directory_separator(pal::char_t c)
    {
        return c == DIR_SEPARATOR || c == L'/';
    }
}

bool pal::is_path_rooted(const string_t& path)
{
    return (path.length() >= 1 && is_directory_separator(path[0])) // UNC or device paths
        || (path.length() >= 2 && path[1] == L':'); // Drive letter paths
}

bool pal::is_path_fully_qualified(const string_t& path)
{
    if (path.length() < 2)
        return false;

    // Check for UNC and DOS device paths
    if (is_directory_separator(path[0]))
        return path[1] == L'?' || is_directory_separator(path[1]);

    // Check for drive absolute path - for example C:\.
    return path.length() >= 3 && path[1] == L':' && is_directory_separator(path[2]);
}

// Returns true only if an env variable can be read successfully to be non-empty.
bool pal::getenv(const char_t* name, string_t* recv)
{
    recv->clear();

    auto length = ::GetEnvironmentVariableW(name, nullptr, 0);
    if (length == 0)
    {
        auto err = GetLastError();
        if (err != ERROR_ENVVAR_NOT_FOUND)
        {
            trace::warning(_X("Failed to read environment variable [%s], HRESULT: 0x%X"), name, HRESULT_FROM_WIN32(err));
        }
        return false;
    }
    std::vector<pal::char_t> buffer(length);
    if (::GetEnvironmentVariableW(name, &buffer[0], length) == 0)
    {
        auto err = GetLastError();
        if (err != ERROR_ENVVAR_NOT_FOUND)
        {
            trace::warning(_X("Failed to read environment variable [%s], HRESULT: 0x%X"), name, HRESULT_FROM_WIN32(err));
        }
        return false;
    }

    recv->assign(buffer.data());
    return true;
}

void pal::enumerate_environment_variables(const std::function<void(const pal::char_t*, const pal::char_t*)> callback)
{
    LPWCH env_strings = ::GetEnvironmentStringsW();
    if (env_strings == nullptr)
        return;

    LPWCH current = env_strings;
    while (*current != L'\0')
    {
        LPWCH eq_ptr = ::wcschr(current, L'=');
        if (eq_ptr != nullptr && eq_ptr != current)
        {
            pal::string_t name(current, eq_ptr - current);
            callback(name.c_str(), eq_ptr + 1);
        }

        current += pal::strlen(current) + 1; // Move to next string
    }

    ::FreeEnvironmentStringsW(env_strings);
}

int pal::xtoi(const char_t* input)
{
    return ::_wtoi(input);
}

bool pal::get_own_executable_path(string_t* recv)
{
    return GetModuleFileNameWrapper(NULL, recv);
}

bool pal::get_current_module(dll_t *mod)
{
    HMODULE hmod = nullptr;
    if (!GetModuleHandleFromAddress(&get_current_module, &hmod))
        return false;

    *mod = (pal::dll_t)hmod;
    return true;
}

bool pal::get_own_module_path(string_t* recv)
{
    HMODULE hmod;
    if (!GetModuleHandleFromAddress(&get_own_module_path, &hmod))
        return false;

    return GetModuleFileNameWrapper(hmod, recv);
}

bool pal::get_method_module_path(string_t* recv, void* method)
{
    HMODULE hmod;
    if (!GetModuleHandleFromAddress(method, &hmod))
        return false;

    return GetModuleFileNameWrapper(hmod, recv);
}


bool pal::get_module_path(dll_t mod, string_t* recv)
{
    return GetModuleFileNameWrapper(mod, recv);
}

bool get_extraction_base_parent_directory(pal::string_t& directory)
{
    const size_t max_len = MAX_PATH + 1;
    pal::char_t temp_path[max_len];

    size_t len = get_temp_path(max_len, temp_path);
    if (len == 0)
    {
        return false;
    }

    assert(len < max_len);
    directory.assign(temp_path);

    return pal::fullpath(&directory);
}

bool pal::get_default_bundle_extraction_base_dir(pal::string_t& extraction_dir)
{
    if (!get_extraction_base_parent_directory(extraction_dir))
    {
        trace::error(_X("Failed to determine default extraction location. Check if 'TMP' or 'TEMP' points to existing path."));
        return false;
    }

    append_path(&extraction_dir, _X(".net"));
    // Windows Temp-Path is already user-private.

    if (fullpath(&extraction_dir))
    {
        return true;
    }

    // Create the %TEMP%\.net directory
    if (CreateDirectoryW(extraction_dir.c_str(), NULL) == 0 &&
        GetLastError() != ERROR_ALREADY_EXISTS)
    {
        trace::error(_X("Failed to create default extraction directory [%s]. %s, error code: %d"), extraction_dir.c_str(), pal::strerror(errno).c_str(), GetLastError());
        return false;
    }

    return fullpath(&extraction_dir);
}

static bool wchar_convert_helper(DWORD code_page, const char* cstr, size_t len, pal::string_t* out)
{
    out->clear();

    // No need of explicit null termination, so pass in the actual length.
    size_t size = ::MultiByteToWideChar(code_page, 0, cstr, static_cast<uint32_t>(len), nullptr, 0);
    if (size == 0)
    {
        return false;
    }
    out->resize(size, '\0');
    return ::MultiByteToWideChar(code_page, 0, cstr, static_cast<uint32_t>(len), &(*out)[0], static_cast<uint32_t>(out->size())) != 0;
}

size_t pal::pal_utf8string(const pal::string_t& str, char* out_buffer, size_t len)
{
    // Pass -1 as we want explicit null termination in the char buffer.
    size_t size = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size == 0 || size > len)
        return size;

    // Pass -1 as we want explicit null termination in the char buffer.
    return ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, out_buffer, static_cast<uint32_t>(len), nullptr, nullptr);
}

bool pal::pal_utf8string(const pal::string_t& str, std::vector<char>* out)
{
    out->clear();

    // Pass -1 as we want explicit null termination in the char buffer.
    size_t size = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size == 0)
    {
        return false;
    }
    out->resize(size, '\0');
    return ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, out->data(), static_cast<uint32_t>(out->size()), nullptr, nullptr) != 0;
}

bool pal::pal_clrstring(const pal::string_t& str, std::vector<char>* out)
{
    return pal_utf8string(str, out);
}

bool pal::clr_palstring(const char* cstr, pal::string_t* out)
{
    return wchar_convert_helper(CP_UTF8, cstr, ::strlen(cstr), out);
}

typedef std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)> SmartHandle;

// Like fullpath, but resolves file symlinks (note: not necessarily directory symlinks).
bool pal::realpath(pal::string_t* path, bool skip_error_logging)
{
    if (path->empty())
    {
        return false;
    }

    // Use CreateFileW + GetFinalPathNameByHandleW to resolve symlinks
    // https://learn.microsoft.com/windows/win32/fileio/symbolic-link-effects-on-file-systems-functions#createfile-and-createfiletransacted
    SmartHandle file(
        ::CreateFileW(
            path->c_str(),
            0, // Querying only
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr, // default security
            OPEN_EXISTING, // existing file
            FILE_ATTRIBUTE_NORMAL, // normal file
            nullptr), // No attribute template
        &::CloseHandle);

    pal::char_t buf[MAX_PATH];
    size_t size;

    if (file.get() == INVALID_HANDLE_VALUE)
    {
        // If we get "access denied" that may mean the path represents a directory.
        // Even if not, we can fall back to GetFullPathNameW, which doesn't require a HANDLE

        auto error = ::GetLastError();
        file.release();
        if (ERROR_ACCESS_DENIED != error)
        {
            if (!skip_error_logging)
            {
                trace::error(_X("Error resolving full path [%s]. Error code: %d"), path->c_str(), error);
            }
            return false;
        }
    }
    else
    {
        size = ::GetFinalPathNameByHandleW(file.get(), buf, MAX_PATH, FILE_NAME_NORMALIZED);
        // If size is 0, this call failed. Fall back to GetFullPathNameW, below
        if (size != 0)
        {
            pal::string_t str;
            if (size < MAX_PATH)
            {
                str.assign(buf);
            }
            else
            {
                str.resize(size, 0);
                size = ::GetFinalPathNameByHandleW(file.get(), (LPWSTR)str.data(), static_cast<uint32_t>(size), FILE_NAME_NORMALIZED);
                assert(size <= str.size());

                if (size == 0)
                {
                    if (!skip_error_logging)
                    {
                        trace::error(_X("Error resolving full path [%s]. Error code: %d"), path->c_str(), ::GetLastError());
                    }
                    return false;
                }
            }

            // Remove the UNC extended prefix (\\?\UNC\) or extended prefix (\\?\) unless it is necessary or was already there
            if (LongFile::IsUNCExtended(str) && !LongFile::IsUNCExtended(*path) && str.length() < MAX_PATH)
            {
                str.replace(0, LongFile::UNCExtendedPathPrefix.size(), LongFile::UNCPathPrefix);
            }
            else if (LongFile::IsExtended(str) && !LongFile::IsExtended(*path) &&
                !LongFile::ShouldNormalize(str.substr(LongFile::ExtendedPrefix.size())))
            {
                str.erase(0, LongFile::ExtendedPrefix.size());
            }

            *path = str;
            return true;
        }
    }

    // If the above fails, fall back to fullpath
    return pal::fullpath(path, skip_error_logging);
}

bool pal::fullpath(string_t* path, bool skip_error_logging)
{
    if (path->empty())
    {
        return false;
    }

    if (LongFile::IsNormalized(*path))
    {
        WIN32_FILE_ATTRIBUTE_DATA data;
        if (GetFileAttributesExW(path->c_str(), GetFileExInfoStandard, &data) != 0)
        {
            return true;
        }
    }

    char_t buf[MAX_PATH];
    size_t size = ::GetFullPathNameW(path->c_str(), MAX_PATH, buf, nullptr);
    if (size == 0)
    {
        if (!skip_error_logging)
        {
            trace::error(_X("Error resolving full path [%s]"), path->c_str());
        }
        return false;
    }

    string_t str;
    if (size < MAX_PATH)
    {
        str.assign(buf);
    }
    else
    {
        str.resize(size + LongFile::UNCExtendedPathPrefix.length(), 0);

        size = ::GetFullPathNameW(path->c_str(), static_cast<uint32_t>(size), (LPWSTR)str.data(), nullptr);
        assert(size <= str.size());

        if (size == 0)
        {
            if (!skip_error_logging)
            {
                trace::error(_X("Error resolving full path [%s]"), path->c_str());
            }
            return false;
        }

        const string_t* prefix = &LongFile::ExtendedPrefix;
        //Check if the resolved path is a UNC. By default we assume relative path to resolve to disk
        if (str.compare(0, LongFile::UNCPathPrefix.length(), LongFile::UNCPathPrefix) == 0)
        {
            prefix = &LongFile::UNCExtendedPathPrefix;
            str.erase(0, LongFile::UNCPathPrefix.length());
            size = size - LongFile::UNCPathPrefix.length();
        }

        str.insert(0, *prefix);
        str.resize(size + prefix->length());
        str.shrink_to_fit();
    }

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExW(str.c_str(), GetFileExInfoStandard, &data) != 0)
    {
        *path = str;
        return true;
    }

    return false;
}

bool pal::file_exists(const string_t& path)
{
    string_t tmp(path);
    return pal::fullpath(&tmp, true);
}

bool pal::is_directory(const pal::string_t& path)
{
    DWORD attributes = ::GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static void readdir(const pal::string_t& path, const pal::string_t& pattern, bool onlydirectories, std::vector<pal::string_t>* list)
{
    assert(list != nullptr);

    std::vector<pal::string_t>& files = *list;
    pal::string_t normalized_path(path);

    if (LongFile::ShouldNormalize(normalized_path))
    {
        if (!pal::fullpath(&normalized_path))
        {
            return;
        }
    }

    pal::string_t search_string(normalized_path);
    append_path(&search_string, pattern.c_str());

    WIN32_FIND_DATAW data = { 0 };

    auto handle = ::FindFirstFileExW(search_string.c_str(), FindExInfoStandard, &data, FindExSearchNameMatch, NULL, 0);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return;
    }
    do
    {
        if (!onlydirectories || (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            pal::string_t filepath(data.cFileName);
            if (filepath != _X(".") && filepath != _X(".."))
            {
                files.push_back(filepath);
            }
        }
    } while (::FindNextFileW(handle, &data));
    ::FindClose(handle);
}

void pal::readdir(const string_t& path, const string_t& pattern, std::vector<pal::string_t>* list)
{
    ::readdir(path, pattern, false, list);
}

void pal::readdir(const string_t& path, std::vector<pal::string_t>* list)
{
    ::readdir(path, _X("*"), false, list);
}

void pal::readdir_onlydirectories(const pal::string_t& path, const string_t& pattern, std::vector<pal::string_t>* list)
{
    ::readdir(path, pattern, true, list);
}

void pal::readdir_onlydirectories(const pal::string_t& path, std::vector<pal::string_t>* list)
{
    ::readdir(path, _X("*"), true, list);
}

bool pal::is_running_in_wow64()
{
    BOOL fWow64Process = FALSE;
    if (!IsWow64Process(GetCurrentProcess(), &fWow64Process))
    {
        return false;
    }
    return (fWow64Process != FALSE);
}

typedef BOOL (WINAPI* is_wow64_process2)(
    HANDLE hProcess,
    USHORT *pProcessMachine,
    USHORT *pNativeMachine
);

bool pal::is_emulating_x64()
{
#if defined(TARGET_AMD64)
    auto kernel32 = LoadLibraryExW(L"kernel32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (kernel32 == nullptr)
    {
        // Loading kernel32.dll failed, log the error and continue.
        trace::info(_X("Could not load 'kernel32.dll': %u"), GetLastError());
        return false;
    }

    is_wow64_process2 is_wow64_process2_func = (is_wow64_process2)::GetProcAddress(kernel32, "IsWow64Process2");
    if (is_wow64_process2_func == nullptr)
    {
        // Could not find IsWow64Process2.
        return false;
    }

    USHORT process_machine;
    USHORT native_machine;
    if (!is_wow64_process2_func(GetCurrentProcess(), &process_machine, &native_machine))
    {
        // IsWow64Process2 failed. Log the error and continue.
        trace::info(_X("Call to IsWow64Process2 failed: %u"), GetLastError());
        return false;
    }

    // If we are running targeting x64 on a non-x64 machine, we are emulating
    return native_machine != IMAGE_FILE_MACHINE_AMD64;
#else
    return false;
#endif
}

bool pal::are_paths_equal_with_normalized_casing(const string_t& path1, const string_t& path2)
{
    // On Windows, paths are case-insensitive
    return (strcasecmp(path1.c_str(), path2.c_str()) == 0);
}

pal::mutex_t::mutex_t()
    : _impl{ }
{
    ::InitializeCriticalSection(&_impl);
}

pal::mutex_t::~mutex_t()
{
    ::DeleteCriticalSection(&_impl);
}

void pal::mutex_t::lock()
{
    ::EnterCriticalSection(&_impl);
}

void pal::mutex_t::unlock()
{
    ::LeaveCriticalSection(&_impl);
}
