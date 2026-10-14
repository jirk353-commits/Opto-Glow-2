// Opto Glow by ROHIT JI - one-click VST3 installer for Windows.
// The plugin binary is embedded as a resource and copied into the standard
// VST3 folder: C:\Program Files\Common Files\VST3\Opto Glow.vst3\Contents\x86_64-win\

#ifndef UNICODE
 #define UNICODE
#endif
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#include <string>

#define IDR_PLUGIN 101

static const wchar_t* kTitle = L"Opto Glow by ROHIT JI - Installer";

static std::wstring vst3Root()
{
    PWSTR common = nullptr;
    std::wstring root = L"C:\\Program Files\\Common Files";
    if (SUCCEEDED (SHGetKnownFolderPath (FOLDERID_ProgramFilesCommon, 0, nullptr, &common)) && common != nullptr)
        root = common;
    if (common != nullptr)
        CoTaskMemFree (common);
    return root + L"\\VST3";
}

static bool fileExists (const std::wstring& p)
{
    const DWORD a = GetFileAttributesW (p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && ! (a & FILE_ATTRIBUTE_DIRECTORY);
}

static void error (const std::wstring& msg)
{
    MessageBoxW (nullptr, msg.c_str(), kTitle, MB_OK | MB_ICONERROR);
}

static bool removeBundle (const std::wstring& bundle)
{
    const std::wstring bin = bundle + L"\\Contents\\x86_64-win\\Opto Glow.vst3";
    if (fileExists (bin) && ! DeleteFileW (bin.c_str()))
        return false;
    RemoveDirectoryW ((bundle + L"\\Contents\\x86_64-win").c_str());
    RemoveDirectoryW ((bundle + L"\\Contents\\Resources").c_str());
    RemoveDirectoryW ((bundle + L"\\Contents").c_str());
    RemoveDirectoryW (bundle.c_str());
    return true;
}

int WINAPI wWinMain (HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED);

    const std::wstring root   = vst3Root();
    const std::wstring bundle = root + L"\\Opto Glow.vst3";
    const std::wstring binDir = bundle + L"\\Contents\\x86_64-win";
    const std::wstring target = binDir + L"\\Opto Glow.vst3";
    const bool installed = fileExists (target);

    if (installed)
    {
        const int r = MessageBoxW (nullptr,
            L"Opto Glow is already installed on this PC.\n\n"
            L"YES  =  Update / Reinstall\n"
            L"NO   =  Uninstall (remove the plugin)\n"
            L"CANCEL  =  Do nothing",
            kTitle, MB_YESNOCANCEL | MB_ICONQUESTION);

        if (r == IDCANCEL)
            return 0;

        if (r == IDNO)
        {
            if (removeBundle (bundle))
                MessageBoxW (nullptr, L"Opto Glow has been removed.", kTitle, MB_OK | MB_ICONINFORMATION);
            else
                error (L"Could not remove the plugin.\n\nPlease close FL Studio / Cubase and try again.");
            return 0;
        }
    }
    else
    {
        const std::wstring msg =
            L"Opto Glow  -  Vintage Opto Compressor\nby ROHIT JI\n\n"
            L"This will install the VST3 plugin to:\n" + root + L"\n\n"
            L"Install now?";
        if (MessageBoxW (nullptr, msg.c_str(), kTitle, MB_OKCANCEL | MB_ICONINFORMATION) != IDOK)
            return 0;
    }

    // load the embedded plugin
    HRSRC res = FindResourceW (nullptr, MAKEINTRESOURCEW (IDR_PLUGIN), RT_RCDATA);
    HGLOBAL h = res ? LoadResource (nullptr, res) : nullptr;
    const void* data = h ? LockResource (h) : nullptr;
    const DWORD size = res ? SizeofResource (nullptr, res) : 0;
    if (data == nullptr || size == 0)
    {
        error (L"Installer is damaged (plugin data missing). Please download it again.");
        return 1;
    }

    const int dirResult = SHCreateDirectoryExW (nullptr, binDir.c_str(), nullptr);
    if (dirResult != ERROR_SUCCESS && dirResult != ERROR_ALREADY_EXISTS && dirResult != ERROR_FILE_EXISTS)
    {
        error (L"Could not create the VST3 folder:\n" + binDir +
               L"\n\nPlease run the installer again and click YES when Windows asks for permission.");
        return 1;
    }

    HANDLE f = CreateFileW (target.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (f == INVALID_HANDLE_VALUE)
    {
        const DWORD e = GetLastError();
        if (e == ERROR_SHARING_VIOLATION || e == ERROR_LOCK_VIOLATION)
            error (L"The plugin is currently in use.\n\nPlease close FL Studio / Cubase and run the installer again.");
        else
            error (L"Could not write the plugin file (error " + std::to_wstring (e) + L").\n\n"
                   L"Please run the installer again and allow administrator permission.");
        return 1;
    }

    DWORD written = 0;
    const BOOL ok = WriteFile (f, data, size, &written, nullptr);
    CloseHandle (f);

    if (! ok || written != size)
    {
        DeleteFileW (target.c_str());
        error (L"Writing the plugin failed. Is the disk full?");
        return 1;
    }

    MessageBoxW (nullptr,
        L"Opto Glow is installed!  \x2714\n\n"
        L"FL STUDIO:\n"
        L"   Options > Manage plugins > Find installed plugins\n"
        L"   Then: Mixer > insert slot > Opto Glow\n\n"
        L"CUBASE:\n"
        L"   Restart Cubase (or Studio > VST Plug-in Manager > Rescan)\n"
        L"   Then: Insert > Dynamics > Opto Glow\n\n"
        L"Tip: double-click any knob to reset it.",
        kTitle, MB_OK | MB_ICONINFORMATION);

    CoUninitialize();
    return 0;
}
