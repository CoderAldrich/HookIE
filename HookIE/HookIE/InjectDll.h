#pragma once
#include <list>

class CInjectDll
{
public:
	CInjectDll(void);
	~CInjectDll(void);

	// ��ȡ���̾�����ڵ�·��
	static CString GetModulePath(HMODULE hModule = NULL);

	// ��ȡָ��EXE·���Ľ��̾��
	static void GetProcessHandle(CString strExePath,std::list<HANDLE> &handleList);
	
	// ָ��DLLע�뵽ָ��EXE����
	static void InjectDllToExe(CString strDllPath,CString strExePath);

	// ָ��DLLע�뵽ָ�����̾��
	static bool InjectDllToProc(CString strDllPath, HANDLE targetProc);

	// ָ��DLL��ָ��EXE����ж��
	static void UninstallDllToExe(CString strDllPath,CString strExePath);

	// ָ��DLL��ָ�����̾��ж��
	static bool  UninstallDllToProc(CString strDllPath, HANDLE targetProc);

};

