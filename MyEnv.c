#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shlwapi.lib")

void BroadcastEnvironmentChange() {
    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
}

void UpdateSystemEnv(const wchar_t* name, const wchar_t* value) {
    HKEY hKey;
    wchar_t oldVal[4096];
    DWORD size = sizeof(oldVal);
    LONG ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, KEY_READ | KEY_WRITE, &hKey);
    if (ret != ERROR_SUCCESS) return;

    ret = RegQueryValueExW(hKey, name, NULL, NULL, (LPBYTE)oldVal, &size);
    if (ret != ERROR_SUCCESS) oldVal[0] = L'\0';

    if (wcslen(value) > 0 && (!wcsstr(oldVal, value))) {
        wchar_t newVal[8192];
        if (_wcsicmp(name, L"Path") == 0 && wcslen(oldVal) > 0) {
            swprintf(newVal, 8192, L"%s;%s", oldVal, value);
        } else {
            swprintf(newVal, 8192, L"%s", value);
        }
        RegSetValueExW(hKey, name, 0, REG_EXPAND_SZ, (const BYTE*)newVal, (DWORD)((wcslen(newVal) + 1) * sizeof(wchar_t)));
    }

    RegCloseKey(hKey);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);

    wchar_t value[MAX_PATH];

    // Java 环境
    swprintf(value, MAX_PATH, L"%sjdk-11", exePath);
    UpdateSystemEnv(L"JAVA_HOME", value);

    UpdateSystemEnv(L"Path", L"%JAVA_HOME%\\bin");
    UpdateSystemEnv(L"Path", L"%JAVA_HOME%\\jre\\bin");

    UpdateSystemEnv(L"CLASSPATH", L".;%JAVA_HOME%\\lib\\dt.jar;%JAVA_HOME%\\lib\\tools.jar");

    // Python 环境
    swprintf(value, MAX_PATH, L"%sPython", exePath);
    UpdateSystemEnv(L"PYTHONHOME", value);
    UpdateSystemEnv(L"Path", value);
    swprintf(value, MAX_PATH, L"%sPython\\Scripts", exePath);
    UpdateSystemEnv(L"Path", value);

    // QASO 环境
    swprintf(value, MAX_PATH, L"%sfilepath.txt", exePath);
    UpdateSystemEnv(L"Qaso_External_File_Read_Path", value);

    UpdateSystemEnv(L"Qaso_System_Environment", L"windows");
    UpdateSystemEnv(L"Qaso_Python_Version", L"3.7");

    swprintf(value, MAX_PATH, L"%sqaso-python", exePath);
    UpdateSystemEnv(L"Qaso_Python_Project_Path", value);

    UpdateSystemEnv(L"Qaso_SqlServer_Password", L"111222Ab_");

    swprintf(value, MAX_PATH, L"%sPython\\Lib\\site-packages\\jep\\jep.dll", exePath);
    UpdateSystemEnv(L"Qaso_Jep_Library_Path", value);

    swprintf(value, MAX_PATH, L"%sapache-tomcat-8.5.8\\bin", exePath);
    UpdateSystemEnv(L"Qaso_Tomcat_Startup_Path", value);

    // 调用 modify_server_xml.exe
    wchar_t modifyExe[MAX_PATH];
    swprintf(modifyExe, MAX_PATH, L"%s\\modify_server_xml.exe", exePath);

    if (PathFileExistsW(modifyExe)) {
        ShellExecuteW(NULL, L"open", modifyExe, NULL, NULL, SW_HIDE);
    } else {
        MessageBoxW(NULL, L"\u8b66\u544a: modify_server_xml.exe \u672a\u627e\u5230, \u8df3\u8fc1 server.xml \u4fee\u6539!", L"\u9519\u8bef", MB_OK | MB_ICONERROR);
    }

    // 刷新环境变量
    BroadcastEnvironmentChange();

    return 0;
}
