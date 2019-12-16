#include "StdAfx.h"
#include "InjectDll.h"
#include <TlHelp32.h>

#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif

#include "tlhelp32.h"
#include <Psapi.h>  
#pragma comment (lib,"Psapi.lib")  

CInjectDll::CInjectDll(void)
{
}


CInjectDll::~CInjectDll(void)
{
}

CString CInjectDll::GetModulePath(HMODULE hModule)
{
	TCHAR buf[MAX_PATH] = {'\0'};
	CString strDir, strTemp;

	::GetModuleFileName( hModule, buf, MAX_PATH);
	strTemp = buf;
	strDir = strTemp.Left( strTemp.ReverseFind('\\') + 1 );
	return strDir;
}

void CInjectDll::GetProcessHandle(CString strExePath,std::list<HANDLE>& handleList)
{
	CString exeName ;
	int index= strExePath.ReverseFind('\\');
	exeName = strExePath.Right(strExePath.GetLength()-index);

	HANDLE snapHandele = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
	if( INVALID_HANDLE_VALUE == snapHandele)
	{
		return;
	}
	PROCESSENTRY32 entry = {0};
	entry.dwSize = sizeof(entry);// ���ȱ��븳ֵ
	BOOL bRet = Process32First(snapHandele,&entry);
	CString  exeTempName;
	while (bRet) 
	{
		exeTempName = (entry.szExeFile);
		if( exeTempName.CompareNoCase(exeName) ==0 )
		{
			HANDLE procHandle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,entry.th32ProcessID);  
			TCHAR exePath[MAX_PATH] = {0};
			if(procHandle)
			{
				if( GetModuleFileNameEx(procHandle,NULL,exePath,MAX_PATH) )
				{
					// ȫ·����ȡ��
					if(CString(exePath).CompareNoCase(strExePath) == 0)
					{
						// ���̾���ҵ�
						handleList.push_back(procHandle);
					}
					else
					{
						CloseHandle(procHandle);
					}
				}
				else
				{
					CloseHandle(procHandle);
				}
				
			}
		}
		bRet = Process32Next(snapHandele,&entry);
	}
	CloseHandle(snapHandele);
	return;
}

void CInjectDll::InjectDllToExe(CString strDllPath,CString strExePath)
{
	std::list<HANDLE> handleList;
	GetProcessHandle(strExePath,handleList);
	HANDLE targetProc = NULL;

	// ��ȡ����ÿ��EXE���̾��������DLLע��
	for(std::list<HANDLE>::iterator it = handleList.begin(); it != handleList.end(); it++)
	{
		targetProc = *it;
		bool ret = InjectDllToProc(strDllPath, targetProc);
		if(ret == false)
		{
			AfxGetApp()->GetMainWnd()->MessageBox(_T("InjectDllToProc failed"));
		}
		CloseHandle(targetProc);
	}
	return;
}

bool CInjectDll::InjectDllToProc(CString strDllPath, HANDLE targetProc)
{
	if(targetProc == NULL)
	{
		return false;
	}
	/*
	ע��DLL��˼·���裺
	1. ��Ŀ�����������һ���ڴ�ռ�(ʹ��VirtualAllocEx����) ���DLL��·�����������ִ��LoadLibraryA
	2. ��DLL·��д�뵽Ŀ�����(ʹ��WriteProcessMemory����)
	3. ��ȡLoadLibraryA������ַ(ʹ��GetProcAddress)��������Ϊ�̵߳Ļص�����
	4. ��Ŀ����� �����̲߳�ִ��(ʹ��CreateRemoteThread)
	*/

	int dllLen = strDllPath.GetLength();
	// 1.Ŀ���������ռ�
	LPVOID pDLLPath = VirtualAllocEx(targetProc,NULL,dllLen,MEM_COMMIT,PAGE_READWRITE );
	if( pDLLPath == NULL )
	{
		return false;
	}
	SIZE_T wLen = 0;
	// 2.��DLL·��д��Ŀ������ڴ�ռ�
	int ret = WriteProcessMemory(targetProc,pDLLPath,strDllPath,dllLen,&wLen);
	if( ret == 0 )
	{
		return false;
	}
	// 3.��ȡLoadLibraryA������ַ
	FARPROC myLoadLibrary = GetProcAddress(GetModuleHandleA("kernel32.dll"),"LoadLibraryA");
	if( myLoadLibrary == NULL )
	{
		return false;
	}
	// 4.��Ŀ�����ִ��LoadLibrary ע��ָ�����߳�
	HANDLE tHandle = CreateRemoteThread(targetProc,NULL,NULL,
		(LPTHREAD_START_ROUTINE)myLoadLibrary,pDLLPath,NULL,NULL);
	if(tHandle == NULL)
	{
		return false;
	}
	WaitForSingleObject(tHandle,INFINITE);
	CloseHandle(tHandle);
	CloseHandle(targetProc);
	return true;
}

void CInjectDll::UninstallDllToExe(CString strDllPath,CString strExePath)
{
	std::list<HANDLE> handleList;
	GetProcessHandle(strExePath,handleList);
	HANDLE targetProc = NULL;

	// ��ȡ����ÿ��EXE���̾��������DLLж��
	for(std::list<HANDLE>::iterator it = handleList.begin(); it != handleList.end(); it++)
	{
		targetProc = *it;
		bool ret = UninstallDllToProc(strDllPath, targetProc);
		if(ret == false)
		{
			AfxGetApp()->GetMainWnd()->MessageBox(_T("UninstallDllToProc failed"));
		}
		CloseHandle(targetProc);
	}
	return;
}

bool CInjectDll::UninstallDllToProc(CString strDllPath, HANDLE targetProc)
{
    /*
    ж�ز����ע��DLL����ʵ�ʲ��.
    ע��DLL�� ��Ŀ�������ִ��LoadLibraryA
    ж��DLL�� ��Ŀ�������ִ��FreeLibrary��������ͬ����ж�ز���Ҫ��Ŀ�����������ռ䣬
    ��ΪFreeLibrary����ΪHMODULE ʵ���Ͼ���һ��ָ��ֵ���������Ѿ����ؾ��Ѿ����ڡ�
    */
	
    if( targetProc == NULL )
    {
        return false;
    }
	DWORD processID = GetProcessId(targetProc);

    // 1. ��ȡж��dll��ģ����
    HANDLE snapHandele = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE ,processID);
    if( INVALID_HANDLE_VALUE == snapHandele)
    {
        return false;
    }
    MODULEENTRY32 entry = {0};
    entry.dwSize = sizeof(entry);// ���ȱ��븳ֵ
    BOOL ret = Module32First(snapHandele,&entry);
    HMODULE dllHandle = NULL;
	CString tempDllPath;
    while (ret) {
        tempDllPath = entry.szModule;
        if(tempDllPath.CompareNoCase((strDllPath)))
        {
            dllHandle = entry.hModule;
            break;
        }
        ret = Module32Next(snapHandele,&entry);
    }

    CloseHandle(snapHandele);
    if( dllHandle == NULL )
    {
        return false;
    }

    // 2.��ȡFreeLibrary������ַ
    FARPROC myLoadLibrary = GetProcAddress(GetModuleHandleA("kernel32.dll"),"FreeLibrary");
    if( myLoadLibrary == NULL )
    {
        return false;
    }
    // 3.��Ŀ�����ִ��FreeLibrary ж��ָ�����߳�
    HANDLE tHandle = CreateRemoteThread(targetProc,NULL,NULL,
                       (LPTHREAD_START_ROUTINE)myLoadLibrary,dllHandle,NULL,NULL);
    if(tHandle == NULL)
    {
        return false;
    }
    WaitForSingleObject(tHandle,INFINITE);
    CloseHandle(tHandle);
    CloseHandle(targetProc);
	return true;
}

