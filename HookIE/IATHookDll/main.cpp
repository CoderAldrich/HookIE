#include "stdafx.h"
#include <Windows.h>
#include <Wininet.h>


//���庯��ָ�룬��InternetOpenUrlWһ��
typedef HINTERNET (* WINAPI InternetOpenUrlFunc)(
	_In_ HINTERNET hInternet,
	_In_ LPCTSTR   lpszUrl,
	_In_ LPCTSTR   lpszHeaders,
	_In_ DWORD     dwHeadersLength,
	_In_ DWORD     dwFlags,
	_In_ DWORD_PTR dwContext
	);


InternetOpenUrlFunc dwInternetOpenUrlAddr = 0;
DWORD* pInternetOpenUrlAddrOrgin = 0;

//HINTERNET InternetOpenUrl(
//	_In_ HINTERNET hInternet,
//	_In_ LPCTSTR   lpszUrl,
//	_In_ LPCTSTR   lpszHeaders,
//	_In_ DWORD     dwHeadersLength,
//	_In_ DWORD     dwFlags,
//	_In_ DWORD_PTR dwContext
//	);



HANDLE WINAPI myInternetOpenUrl(
		_In_ HINTERNET hInternet,
		_In_ LPCTSTR   lpszUrl,
		_In_ LPCTSTR   lpszHeaders,
		_In_ DWORD     dwHeadersLength,
		_In_ DWORD     dwFlags,
		_In_ DWORD_PTR dwContext
	)
{
	CString str;
	CString cstrUrl = lpszUrl;
	OutputDebugString(_T("myInternetOpenUrl into"));
	// �ɴ��ļ��ж�ȡ���ص�domain
	if (cstrUrl.Find(_T("baidu.com")) >= 0)
	{
		if (MessageBoxW(NULL,L"���ʰٶ�?",L"NOTICE",MB_YESNO)==IDYES)
		{
			return   dwInternetOpenUrlAddr(
				hInternet,
				lpszUrl,
				lpszHeaders,
				dwHeadersLength,
				dwFlags,
				dwContext);
		} 
		else
		{
			return NULL;
		}
	} 
	else
	{
		return dwInternetOpenUrlAddr(
			hInternet,
			lpszUrl,
			lpszHeaders,
			dwHeadersLength,
			dwFlags,
			dwContext);
	}
}

VOID HookIEIAT()
{
	OutputDebugString(_T("HookIEIAT into"));
	
	//---- ����һ��Ҫע��,��ȡ�Ļ�ַ һ��������hook�� ��̬�� ���ڵ�PE����Ļ�ַ  --------- //
	// WININET.dll ������ֱ����iexplore.exe�У�������ieframe.dll��
	// ���̻�ַ
	//HMODULE hModule = GetModuleHandleA(NULL); // ��ǰEXE���
	// dll��ַ
	HMODULE hModule = LoadLibrary(_T("ieframe.dll")); //




	OutputDebugString(_T("HookIEIAT into  1111"));
	// ��λPE�ṹ
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule; // DOSͷ
	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)hModule+pDosHeader->e_lfanew); // PEͷ��ʼλ��

	// ����ӳ���ַ�͵����RVA
	DWORD dwImageBase = pNTHeader->OptionalHeader.ImageBase;
	DWORD dwImpRva = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

	// ������VA
	PIMAGE_IMPORT_DESCRIPTOR pImgDes = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)dwImageBase+dwImpRva);
	PIMAGE_IMPORT_DESCRIPTOR pTempImgDes = pImgDes;

	// ������InternetOpenUrlW�ĵ�ַ
	HMODULE handle=LoadLibraryA("WININET.dll");
	DWORD dwFuncAddr = (DWORD)GetProcAddress(handle,"InternetOpenUrlW");
	OutputDebugString(_T("HookIEIAT into  2222"));
	BOOL bFound = FALSE;
	// ������hook������ģ����
	while (pTempImgDes->Name) // �ṹ��ȫ0Ϊ������־
	{
		DWORD dwNameAddr = dwImageBase + pTempImgDes->Name;
		char szName[MAXBYTE]={0};
		strcpy(szName,(char*)dwNameAddr);
		
		CString cstrName = szName;
		OutputDebugString(cstrName);
		if (cstrName.CompareNoCase(_T("WININET.dll")) == 0 )
		{
			OutputDebugString(_T("WININET.dll find"));
			bFound = TRUE;
			break;
		}
		pTempImgDes ++; // ������һ�������һ�������ṹ��һ��DLL��Ӧ
	}
	OutputDebugString(_T("HookIEIAT into  3333"));
	// �ж��Ƿ��ҵ���hook�������ڵĺ�����
	if (bFound==TRUE)
	{
		bFound= FALSE;
		char szAddr[10] = {0};
		// ���������ģ���IAT�����ַ��
		PIMAGE_THUNK_DATA pThunk=(PIMAGE_THUNK_DATA)(pTempImgDes->FirstThunk + dwImageBase); 
		while ( pThunk->u1.Function ) // �ṹ��ȫ0Ϊ������־
		{
			// �������ĺ�����ַ
			DWORD* pAddr=(DWORD*)(&pThunk->u1.Function); 
			// �Ƚ��Ƿ�����hook�����ĵ�ַ��ͬ
			if (*pAddr == dwFuncAddr)         
			{
				bFound = TRUE;         
				pInternetOpenUrlAddrOrgin = pAddr; // ���� �洢������ַ�� ��ַ�������´λ�ԭ
				dwInternetOpenUrlAddr = (InternetOpenUrlFunc)*pAddr; // ����InternetOpenUrlW������ַ    
				DWORD dwMyHookAddr = (DWORD) myInternetOpenUrl;

				DWORD oldProtect = 0;
				BOOL ret  = FALSE;
				// �޸�ҳ����Ϊ�ɶ���д
				ret = VirtualProtect(pAddr,sizeof(ULONG),PAGE_READWRITE,&oldProtect);
				if(ret == FALSE)
				{
					DWORD errorCode = GetLastError();
					CString errorMsg = CUtility::GetErrorMsg(errorCode);
					CString msg ;
					msg.Format(_T("VirtualProtect false !!! errorCode:%d,errorMsg:%s \n"),errorCode,errorMsg);
					OutputDebugString(msg);
					return;
				}
				// �޸Ĵ˴���ַΪhook������ַ
				ret = WriteProcessMemory(GetCurrentProcess(),(LPVOID)pAddr,&dwMyHookAddr,sizeof(DWORD),NULL); 
				if(ret == FALSE)
				{
					DWORD errorCode = GetLastError();
					CString errorMsg = CUtility::GetErrorMsg(errorCode);
					CString msg ;
					msg.Format(_T("WriteProcessMemory false !!! errorCode:%d,errorMsg:%s \n"),errorCode,errorMsg);
					OutputDebugString(msg);
					return;
				}
				else
				{
					OutputDebugString(_T("WriteProcessMemory true"));
				}
				// ҳ�������ԸĻ�ȥ
				VirtualProtect(pAddr,sizeof(ULONG),oldProtect,NULL);
				return;
			}
			pThunk ++; // ������һ�������ַ��ṹ�壬һ��������һ�������ַ��ṹ���Ӧ
		}
	}
	else
	{
		OutputDebugString(_T("HookIEIAT into  55555"));
	}

	return;
}

VOID UnHookIEIAT()
{
	OutputDebugString(_T("UnHookIEIAT into"));
	if(dwInternetOpenUrlAddr)
	{
		DWORD oldProtect = 0;
		BOOL ret  = FALSE;
		// �޸�ҳ����Ϊ�ɶ���д
		ret = VirtualProtect(pInternetOpenUrlAddrOrgin,sizeof(ULONG),PAGE_READWRITE,&oldProtect);
		if(ret == FALSE)
		{
			DWORD errorCode = GetLastError();
			CString errorMsg = CUtility::GetErrorMsg(errorCode);
			CString msg ;
			msg.Format(_T("VirtualProtect false !!! errorCode:%d,errorMsg:%s \n"),errorCode,errorMsg);
			OutputDebugString(msg);
			return;
		}
		// �����hook�ˣ���hook�ط���ԭΪԭ����ַ
		ret = WriteProcessMemory(GetCurrentProcess(),(LPVOID)pInternetOpenUrlAddrOrgin,&dwInternetOpenUrlAddr,sizeof(DWORD),NULL); 
		if(ret == FALSE)
		{
			DWORD errorCode = GetLastError();
			CString errorMsg = CUtility::GetErrorMsg(errorCode);
			CString msg ;
			msg.Format(_T("WriteProcessMemory false !!! errorCode:%d,errorMsg:%s \n"),errorCode,errorMsg);
			OutputDebugString(msg);
			return;
		}
		else
		{
			OutputDebugString(_T("WriteProcessMemory true"));
		}
	
	}
}

// DLL ���
BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			HookIEIAT();
		}
		break;
	case DLL_PROCESS_DETACH:
		UnHookIEIAT();
		break;
	}
	return TRUE;
}

