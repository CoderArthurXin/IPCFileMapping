
// IPCFileMappingDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "IPCFileMapping.h"
#include "IPCFileMappingDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIPCFileMappingDlg dialog



CIPCFileMappingDlg::CIPCFileMappingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IPCFILEMAPPING_DIALOG, pParent)
	, m_bAsWriter(FALSE)
	, m_strData(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIPCFileMappingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_WRITER, m_bAsWriter);
	DDX_Text(pDX, IDC_EDIT_DATA, m_strData);
}

BEGIN_MESSAGE_MAP(CIPCFileMappingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_WRITE, &CIPCFileMappingDlg::OnBnClickedBtnWrite)
	ON_BN_CLICKED(IDC_CHECK_WRITER, &CIPCFileMappingDlg::OnBnClickedCheckWriter)
END_MESSAGE_MAP()


// CIPCFileMappingDlg message handlers

BOOL CIPCFileMappingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	OnBnClickedCheckWriter();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIPCFileMappingDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIPCFileMappingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIPCFileMappingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

std::string ConvertWideToUTF8(const std::wstring& wideString)
{
	if (wideString.empty())
		return "";

	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideString.data(), static_cast<int>(wideString.size()), nullptr, 0, 0, 0);
	std::string utf8String(requiredSize, '\0');

	if (WideCharToMultiByte(CP_UTF8, 0, wideString.data(), static_cast<int>(wideString.size()), &utf8String[0], requiredSize, 0, 0) == 0)
	{
		return "";
	}

	return utf8String;
}

std::wstring ConvertUTF8ToWide(const std::string& utf8String)
{
	if (utf8String.empty())
		return L"";

	int requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), static_cast<int>(utf8String.size()), nullptr, 0);
	std::wstring wideString(requiredSize, L'\0');

	if (MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), static_cast<int>(utf8String.size()), &wideString[0], requiredSize) == 0)
	{
		return L"";
	}

	return wideString;
}

void CIPCFileMappingDlg::ReadData(std::string data)
{
	auto strRead = ConvertUTF8ToWide(data);
	
	CString strText;
	GetDlgItem(IDC_EDIT_DATA)->GetWindowText(strText);

	strText = strRead.c_str() + CString("\r\n") + strText;

	GetDlgItem(IDC_EDIT_DATA)->SetWindowText(strText);
}

void CIPCFileMappingDlg::OnBnClickedBtnWrite()
{
	UpdateData(TRUE);

	if (m_strData.IsEmpty()) {
		MessageBox(_T("Please input data you want to write."));
		return;
	}

	m_spIPC->Write(ConvertWideToUTF8(m_strData.GetBuffer()).c_str());
}


void CIPCFileMappingDlg::OnBnClickedCheckWriter()
{
	UpdateData(TRUE);

	auto writeBtn = GetDlgItem(IDC_BTN_WRITE);
	if (writeBtn) {
		writeBtn->EnableWindow(m_bAsWriter);
	}

	auto readBtn = GetDlgItem(IDC_BTN_READ);
	if (readBtn) {
		readBtn->EnableWindow(!m_bAsWriter);
	}

	if (m_spIPC) {
		m_spIPC.reset();
	}

	m_spIPC = std::make_shared<RzIPCFileMapping>(m_bAsWriter);

	if (!m_bAsWriter) {
		auto fnRead = std::bind(&CIPCFileMappingDlg::ReadData, this, std::placeholders::_1);
		m_spIPC->OnRead(fnRead);
	}
}


