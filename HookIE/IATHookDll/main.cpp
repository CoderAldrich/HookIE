// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include <windows.h>
#include <Wininet.h>

//���庯��ָ�룬��CreateFileWһ��
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



//fake func replace the one above
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
			return INVALID_HANDLE_VALUE;
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
	// ���̻�ַ
	HMODULE hModule = GetModuleHandleA(NULL); // ��ǰEXE���
	
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

	BOOL bFound = FALSE;
	// ������hook������ģ����
	while (pTempImgDes->Name) // �ṹ��ȫ0Ϊ������־
	{
		DWORD dwNameAddr = dwImageBase + pTempImgDes->Name;
		char szName[MAXBYTE]={0};
		strcpy(szName,(char*)dwNameAddr);
		CString cstrName = szName;
		if (cstrName.CompareNoCase(_T("WININET.dll")) == 0 )
		{
			bFound = TRUE;
			break;
		}
		pTempImgDes ++; // ������һ�������һ�������ṹ��һ��DLL��Ӧ
	}

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
				pInternetOpenUrlAddrOrgin = pAddr;
				dwInternetOpenUrlAddr = (InternetOpenUrlFunc)*pAddr;    
				DWORD dwMyHookAddr = (DWORD) myInternetOpenUrl;
				// �޸Ĵ˴���ַΪhook������ַ
				WriteProcessMemory(GetCurrentProcess(),(LPVOID)pAddr,&dwMyHookAddr,sizeof(DWORD),NULL); 
				break;
			}
			pThunk ++; // ������һ�������ַ��ṹ�壬һ��������һ�������ַ��ṹ���Ӧ
		}
	}

	return;
}

VOID UnHookIEIAT()
{
	if(dwInternetOpenUrlAddr)
	{
		// �����hook�ˣ���hook�ط���ԭΪԭ����ַ
		WriteProcessMemory(GetCurrentProcess(),(LPVOID)pInternetOpenUrlAddrOrgin,&dwInternetOpenUrlAddr,sizeof(DWORD),NULL); 
	}
}
BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		HookIEIAT();
		break;
	case DLL_PROCESS_DETACH:
		UnHookIEIAT();
		break;
	}
	return TRUE;
}

