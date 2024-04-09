
// IPCFileMappingDlg.h : header file
//

#pragma once
#include "RzIPCFileMapping.h"

using namespace RzFileMapping;

// CIPCFileMappingDlg dialog
class CIPCFileMappingDlg : public CDialogEx
{
// Construction
public:
	CIPCFileMappingDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IPCFILEMAPPING_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bAsWriter;
	CString m_strData;

	std::shared_ptr<RzIPCFileMapping> m_spIPC{ nullptr };

	void ReadData(std::string data);
public:
	afx_msg void OnBnClickedBtnWrite();
	afx_msg void OnBnClickedCheckWriter();
};
