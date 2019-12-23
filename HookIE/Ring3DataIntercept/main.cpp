#include "stdafx.h"
#include "windows.h"
#include "winnt.h"

PVOID pNtDeviceIoControl  = NULL ; 



//InternetOpenUrlFunc dwInternetOpenUrlAddr = 0;
DWORD* pNtDeviceIoControlAddrOrgin = 0;


//

#define AFD_RECV 0x12017

#define AFD_SEND 0x1201f

typedef struct AFD_WSABUF{
	UINT  len ;
	PCHAR  buf ;
}AFD_WSABUF , *PAFD_WSABUF;

typedef struct AFD_INFO {
	PAFD_WSABUF  BufferArray ; 
	ULONG  BufferCount ; 
	ULONG  AfdFlags ;
	ULONG  TdiFlags ;
} AFD_INFO,  *PAFD_INFO;
typedef LONG NTSTATUS;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

const CHAR GetXX[] = "GET ";
const CHAR PostXX[] = "POST ";
const CHAR HttpXX[] = "HTTP";
//////////////////////////////////////////////////////////////////////////
//
// LookupSendPacket
// ���Send��
// Ŀǰʵ���˹���HTTP����GET AND POST��
//
//////////////////////////////////////////////////////////////////////////

BOOL LookupSendPacket(PVOID Buffer , ULONG Len)
{
	if (Len < 5)
	{
		return FALSE ; 
	}

	//��������쳣����
	if (memcmp(Buffer , GetXX , 4) == 0 
		||
		memcmp(Buffer , PostXX , 5) == 0 )
	{
		return TRUE ; 
	}
	return FALSE ; 
}        
//////////////////////////////////////////////////////////////////////////
//
// LookupRecvPacket
//
// ���Recv��
// ���������ʵ��Recv�����ֵ书��
// Ŀǰʵ���˹���HTTP�������ݰ��Ĺ���
//
//
///////////////////////////////////////////////////////////////////////////
BOOL LookupRecvPacket(PVOID Buffer , ULONG Len)
{
	if (Len < 4)
	{
		return FALSE ; 
	}

	if (memcmp(Buffer , HttpXX , 4) == 0 )
	{
		return TRUE ; 
	}

	return FALSE ; 
}
//hook����

//////////////////////////////////////////////////////////////////////////
//
// NtDeviceIoControlFile��HOOK���� 
// ws2_32.dll��send , recv���ջ���õ�mswsock.dll�ڵ����ݷ��ͺ���
// mswsock.dll�����NtDeviceIoControlFile��TDI Client��������Send Recvָ��
// ���������������أ����Թ������е�TCP �շ�����UDP֮����ɣ�����Ҫ����ָ�
//
//////////////////////////////////////////////////////////////////////////

NTSTATUS __stdcall NewNtDeviceIoControlFile(
	HANDLE FileHandle,
	HANDLE Event OPTIONAL,
	PVOID ApcRoutine OPTIONAL,
	PVOID ApcContext OPTIONAL,
	PVOID IoStatusBlock,
	ULONG IoControlCode,
	PVOID InputBuffer OPTIONAL,
	ULONG InputBufferLength,
	PVOID OutputBuffer OPTIONAL,
	ULONG OutputBufferLength
	)
{
	//OutputDebugString(L"NewNtDeviceIoControlFile into \n");
	//�ȵ���ԭʼ����

	LONG stat ; 
	__asm
	{
		    push        OutputBufferLength
			push        OutputBuffer
			push        InputBufferLength
			push        InputBuffer 
			push        IoControlCode
			push        IoStatusBlock 
			push        ApcContext
			push        ApcRoutine
			push        Event
			push        FileHandle
			call        pNtDeviceIoControl
			mov                stat ,eax
	}

	//���ԭʼ����ʧ���ˣ�����RECV�����ݣ�

	if (!NT_SUCCESS(stat))
	{
		return stat ; 
	}
	//OutputDebugString(L"NewNtDeviceIoControlFile into aaaaaa\n");
	//����Ƿ�ΪTCP�շ�ָ��

	if (IoControlCode != AFD_SEND && IoControlCode != AFD_RECV)
	{
		return stat ; 
	}
	//OutputDebugString(L"NewNtDeviceIoControlFile into bbbbbb\n");
	//����AFD INFO�ṹ�����SEND��RECV��BUFFER��Ϣ
	//����������������BUFFER���������Ҫ��TRY EXCEPT
	//

	__try
	{
		//��InputBuffer�õ�Buffer��Len

		PAFD_INFO AfdInfo = (PAFD_INFO)InputBuffer ; 
		PVOID Buffer = AfdInfo->BufferArray->buf ; 
		ULONG Len = AfdInfo->BufferArray->len;
		//char temp[11] = {0};
		//memcpy_s(temp,10,Buffer,10);
		//char temp2[100] = {0};
		//sprintf(temp2,"%x.%x.%x.%x.%x",temp[0],temp[1],temp[2],temp[3],temp[4]);
		//OutputDebugStringA(temp2);
		if (IoControlCode == AFD_SEND)
		{
			//OutputDebugString(L"AFD_SEND-----!\n");
			
			if (LookupSendPacket(Buffer , Len))
			{
				//���������
				//�������������Ϣ��������DbgView�鿴�������UI��������SendMessage��ʽ~
				OutputDebugString(L"SendPacket!\n");                
				OutputDebugStringA((char*)Buffer);
			}
		}
		else
		{
			//OutputDebugString(L"AFD_RECV-----!\n");
			if (LookupRecvPacket(Buffer , Len))
			{
				OutputDebugString(L"RecvPacket!\n");
				OutputDebugStringA((char*)Buffer);
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		OutputDebugString(L"EXCEPTION_EXECUTE_HANDLER -----!\n");
		return stat ; 
	}

	return stat ; 



}

//////////////////////////////////////////////////////////////////////////
//
//  Hook mswsock.dll�������Ntdll!NtDeviceIoControlFile
//  ���������TDI Cilent�����������˷��
//  �ȶ������Σ�RING3����ײ�İ�����~
//
//////////////////////////////////////////////////////////////////////////
void SuperHookDeviceIoControl()
{
	//�õ�ws2_32.dll��ģ���ַ
	HMODULE hMod = LoadLibrary(L"mswsock.dll");
	if (hMod == 0 )
	{
		OutputDebugString(L"LoadLibrary false !!! \n");
		return ;
	}

	//�õ�DOSͷ

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod ; 

	//���DOSͷ��Ч
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		OutputDebugString(L"PIMAGE_DOS_HEADER invalid !!! \n");
		return ; 
	}

	//�õ�NTͷ

	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)hMod + pDosHeader->e_lfanew);

	//���NTͷ��Ч
	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		OutputDebugString(L"PIMAGE_NT_HEADERS invalid !!! \n");
		return ; 
	}

	//������������Ŀ¼�Ƿ����
	if (pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0 ||
		pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0 )
	{
		OutputDebugString(L"IMAGE_DIRECTORY_ENTRY_IMPORT invalid !!! \n");
		return ; 
	}
	//�õ����������ָ��

	PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((ULONG)hMod + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	PIMAGE_THUNK_DATA ThunkData ; 

	//���ÿ��������
	while(ImportDescriptor->FirstThunk)
	{
		//�����������Ƿ�Ϊntdll.dll

		char* dllname = (char*)((ULONG)hMod + ImportDescriptor->Name);
		CString cstrName = dllname;
		OutputDebugString(cstrName);
		//������ǣ���������һ������

		if (stricmp(dllname , "ntdll.dll") !=0)
		{
			ImportDescriptor ++ ; 
			continue;
		}

		OutputDebugString(L"ntdll.dll find \n");
		ThunkData = (PIMAGE_THUNK_DATA)((ULONG)hMod + ImportDescriptor->OriginalFirstThunk);

		int no = 1;
		while(ThunkData->u1.Function)
		{
			//��麯���Ƿ�ΪNtDeviceIoControlFile

			char* functionname = (char*)((ULONG)hMod + ThunkData->u1.AddressOfData + 2);
			if (stricmp(functionname , "NtDeviceIoControlFile") == 0 )
			{
				OutputDebugString(L"NtDeviceIoControlFile find \n");
				//
				//����ǣ���ô��¼ԭʼ������ַ
				//HOOK���ǵĺ�����ַ
				//
				ULONG myaddr = (ULONG)NewNtDeviceIoControlFile;
				ULONG btw ; 
				PDWORD lpAddr = (DWORD *)((ULONG)hMod + (DWORD)ImportDescriptor->FirstThunk) +(no-1);
				pNtDeviceIoControlAddrOrgin = lpAddr; // ��ź�����ַ�ĵ�ַ
				pNtDeviceIoControl = (PVOID)(*(ULONG*)lpAddr) ; // ԭ�������ĵ�ַ

				DWORD oldProtect = 0;
				BOOL ret  = FALSE;
				// �޸�ҳ����Ϊ�ɶ���д
				ret = VirtualProtect(lpAddr,sizeof(ULONG),PAGE_READWRITE,&oldProtect);
				if(ret == FALSE)
				{
					DWORD errorCode = GetLastError();
					CString errorMsg = CUtility::GetErrorMsg(errorCode);
					CString msg ;
					msg.Format(_T("VirtualProtect false !!! errorCode:%d,errorMsg:%s \n"),errorCode,errorMsg);
					OutputDebugString(msg);
					return;
				}
				ret = WriteProcessMemory(GetCurrentProcess() , lpAddr , &myaddr , sizeof(ULONG), &btw );
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
					OutputDebugString(L"WriteProcessMemory true \n");
				}
				// ҳ�������ԸĻ�ȥ
				VirtualProtect(lpAddr,sizeof(ULONG),oldProtect,NULL);
				return ; 

			}

			no++;
			ThunkData ++;
		}
		ImportDescriptor ++;
	}
	return ; 
}

//////////////////////////////////////////////////////////////////////////
//
// CheckProcess ����Ƿ�����Ҫ�ҹ��Ľ���
//
//
//////////////////////////////////////////////////////////////////////////

BOOL CheckProcess()
{
	//�ڴ˼�����Ľ��̹���
	return TRUE ;
}

void UnkHookDeviceIoControl()
{
	OutputDebugString(_T("UnkHookDeviceIoControl into"));
	if(pNtDeviceIoControl)
	{
		DWORD oldProtect = 0;
		BOOL ret  = FALSE;
		// �޸�ҳ����Ϊ�ɶ���д
		ret = VirtualProtect(pNtDeviceIoControlAddrOrgin,sizeof(ULONG),PAGE_READWRITE,&oldProtect);
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
		ret = WriteProcessMemory(GetCurrentProcess(),(LPVOID)pNtDeviceIoControlAddrOrgin,&pNtDeviceIoControl,sizeof(DWORD),NULL); 
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
		VirtualProtect(pNtDeviceIoControlAddrOrgin,sizeof(ULONG),oldProtect,NULL);
	}
}
BOOL APIENTRY DllMain( HANDLE hModule, 
	DWORD  ul_reason_for_call, 
	LPVOID lpReserved
	)
{
	//������DLLʱ������API HOOK
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			//����Ƿ���Ҫ���˵Ľ���
			if (CheckProcess() == FALSE)
			{        
				//������ǣ�����FALSE,������ӽ�����ж��
				return FALSE ; 
			}

			//HOOK API
			SuperHookDeviceIoControl();
		}
		break;
	case DLL_PROCESS_DETACH:
		UnkHookDeviceIoControl();
		break;
	}
	return TRUE;
}	