#pragma once
#include "afxwin.h"


// CameraDlg ダイアログ

class CCameraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCameraDlg)

public:
	CCameraDlg(CWnd* pParent, UINT64 addr);   // 標準コンストラクター
	virtual ~CCameraDlg();

// ダイアログ データ
	enum { IDD = IDD_CAMERA_DIALOG };

protected:
	CStatic m_pictCamera;
	UINT64 m_deviceAddr;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnBluetoothReceived(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual void PostNcDestroy();
	virtual void OnCancel();
	afx_msg void OnClose();
public:
	afx_msg void OnDestroy();
};
