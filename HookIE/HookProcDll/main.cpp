#include "stdafx.h"
#include "MsgHook.h"
#include <Windows.h>

HMODULE g_hModule ;
HHOOK g_hook = NULL;

LRESULT CALLBACK HookProc(
	int code, // hook code
	WPARAM wParam, // removal option
	LPARAM lParam // message
	)
{
	if(g_hook)
	{
		return CallNextHookEx(g_hook,code,wParam,lParam);
	}
	return 1;
}

void SetMsgHookOn()
{
	OutputDebugString(_T("===== SetMsgHookOn enter===== \n"));
	g_hook = SetWindowsHookEx(WH_GETMESSAGE,HookProc,g_hModule,0);
	if(g_hook == NULL )
	{
		DWORD errCode = GetLastError();
		CString errMsg = CUtility::GetErrorMsg(errCode);
		CString temp;
		temp.Format(_T("SetWindowsHookEx false!!!errCode:%d,errMsg:%s\n"),errCode,errMsg);
		OutputDebugString(temp);
	}
}



void SetMsgHookOff()
{
	OutputDebugString(_T("===== SetMsgHookOff enter =====\n"));
	CString temp;
	temp.Format(_T("Cur Exe:%s\n"),CUtility::GetCurExeName());
	OutputDebugString(temp);
	BOOL ret = FALSE;
	if(g_hook)
	{
		ret = UnhookWindowsHookEx(g_hook);
	}
	if(ret == FALSE)
	{
		DWORD errCode = GetLastError();
		CString errMsg = CUtility::GetErrorMsg(errCode);
		CString temp;
		temp.Format(_T("UnhookWindowsHookEx false!!!errCode:%d,errMsg:%s\n"),errCode,errMsg);
		OutputDebugString(temp);
	}
}

// �µ�ȫ�ֹ��ӱ�ϵͳǿ��ע�뵽�����к�֪ͨ���Ŀ�����
void NotifyYourApp(BOOL bTatch)
{
	HWND targetWnd = FindWindow(_T("#32770"),MAIN_APP_TITLE);
	//HWND targetWnd = FindWindow(_T("Notepad"),NULL);
	if(targetWnd == NULL)
	{
		OutputDebugString(_T("target exe not found"));
		// Ŀ�����û����������û�ô���ȡ��ȫ�ֹ���
		SetMsgHookOff();
		return;
	}

	CString exeName = CUtility::GetCurExeName();
	if(exeName.CompareNoCase(_T("iexplore.exe")) == 0)
	{
		if(bTatch)
		{
			OutputDebugString(_T("HookProcDll.dll Attach iexplore.exe"));
		}
		else
		{
			OutputDebugString(_T("HookProcDll.dll Detach iexplore.exe"));
		}
		
		// ��ǰע��Ľ���ΪIE�����,֪ͨĿ�����,������ID����Ŀ�����
		DWORD processId = GetProcessId(GetCurrentProcess());
		BOOL ret = PostMessage(targetWnd,WM_IE_OPEN,bTatch,(LPARAM)processId);
		if(ret == FALSE)
		{
			DWORD errCode = GetLastError();
			CString errMsg = CUtility::GetErrorMsg(errCode);
			CString temp;
			temp.Format(_T("PostMessage failed!!!errCode:%d,errMsg:%s"),errCode,errMsg);
			OutputDebugString(temp);
		}
	}
}

// DLL ���
BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	g_hModule = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			NotifyYourApp(TRUE);
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			// ʲô��� ����DLL�ᱻж�أ���Windowsȫ�ֹ��ӱ�ж�غ� ϵͳ��ж�ظ�DLL�����������˳�Ҳ��ж�ظ�DLL
			NotifyYourApp(FALSE);
		}
		break;
	}
	return TRUE;
}