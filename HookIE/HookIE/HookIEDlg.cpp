
// HookIEDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HookIE.h"
#include "HookIEDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHookIEDlg �Ի���




CHookIEDlg::CHookIEDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHookIEDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHookIEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_HOOKIE, m_uiHookIEBtn);
	DDX_Control(pDX, IDC_BTN_UNHOOKIE, m_uiUnHookIEBtn);
}

BEGIN_MESSAGE_MAP(CHookIEDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_HOOKIE, &CHookIEDlg::OnBnClickedBtnHookie)
	ON_BN_CLICKED(IDC_BTN_UNHOOKIE, &CHookIEDlg::OnBnClickedBtnUnhookie)
	ON_BN_CLICKED(IDC_BTN_HOOKIE2, &CHookIEDlg::OnBnClickedBtnHookie2)
	ON_BN_CLICKED(IDC_BTN_UNHOOKIE2, &CHookIEDlg::OnBnClickedBtnUnhookie2)
END_MESSAGE_MAP()


// CHookIEDlg ��Ϣ�������

BOOL CHookIEDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	// �����������̸�����������Ϣ
	ChangeWindowMessageFilterEx(m_hWnd, WM_IE_OPEN, MSGFLT_ALLOW, NULL);
	//ChangeWindowMessageFilter (WM_DROPFILES, MSGFLT_ADD);
	//ChangeWindowMessageFilter (WM_COPYDATA, MSGFLT_ADD);
	//ChangeWindowMessageFilter (WM_IE_OPEN, MSGFLT_ADD);
	//ChangeWindowMessageFilterEx(AfxGetMainWnd()->m_hWnd, WM_IE_OPEN, MSGFLT_ALLOW, NULL);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CHookIEDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CHookIEDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CHookIEDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CHookIEDlg::OnBnClickedBtnHookie()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString exePath,dllPath;
	exePath = CUtility::GetIEPath();
	dllPath = CUtility::GetModulePath() + _T("InlineHookDll.dll");
	CUtility::InjectDllToExe(dllPath,exePath);
}


void CHookIEDlg::OnBnClickedBtnUnhookie()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString exePath,dllPath;
	exePath = CUtility::GetIEPath();
	dllPath = CUtility::GetModulePath() + _T("InlineHookDll.dll");
	CUtility::UninstallDllToExe(dllPath,exePath);
}

static HMODULE g_DllModule = NULL;

typedef void  (* MyFunc)(); // ���庯��ָ��

void CHookIEDlg::OnBnClickedBtnHookie2()
{
	if(g_DllModule == NULL)
	{
		CString path = CUtility::GetModulePath(NULL);
		path.Append(_T("HookProcDll.dll"));
		g_DllModule = LoadLibraryEx(path,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
	}
	if(g_DllModule == NULL)
	{
		MessageBox(_T("HookProcDll.dll not found"));
		return;
	}
	MyFunc func = (MyFunc)GetProcAddress(g_DllModule, "SetMsgHookOn");
	if(func)
	{
		func();
	}
	else
	{
		MessageBox(_T("HookProcDll.dll.SetMsgHookOn not found"));
	}
	
}


void CHookIEDlg::OnBnClickedBtnUnhookie2()
{
	if(g_DllModule == NULL)
	{
		CString path = CUtility::GetModulePath(NULL);
		path.Append(_T("HookProcDll.dll"));
		g_DllModule = LoadLibraryEx(path,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
	}
	if(g_DllModule == NULL)
	{
		MessageBox(_T("HookProcDll.dll not found"));
		return;
	}
	MyFunc func = (MyFunc)GetProcAddress(g_DllModule, "SetMsgHookOff");
	if(func)
	{
		func();
	}
	else
	{
		MessageBox(_T("HookProcDll.dll.SetMsgHookOff not found"));
	}
}


LRESULT CHookIEDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_IE_OPEN)
	{
		
		BOOL bAtaach = (BOOL)wParam;
		DWORD dwPID = (DWORD)lParam;
		CString dllPath =CUtility::GetModulePath() + _T("InlineHookDll.dll");
		//CString temp;
		//temp.Format(_T("%s,WM_IE_OPEN Attach=%d,PID=%d"),dllPath,bAtaach,dwPID);
		//MessageBox(temp);
		if(bAtaach)
		{
			// ��IE���������
			CUtility::InjectDllToProc(dllPath,dwPID);
		}
		else
		{
			CUtility::UninstallDllToProc(dllPath,dwPID);
		}
	}

	return CDialogEx::DefWindowProc(message, wParam, lParam);
}
