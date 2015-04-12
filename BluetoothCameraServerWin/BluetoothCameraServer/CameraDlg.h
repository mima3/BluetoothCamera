#pragma once
#include "afxwin.h"
class CAviFile;

/**
 * @class CCameraDlg
 * @brief �J�����v���r���[�p�̃_�C�A���O
 */
class CCameraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCameraDlg)

public:
	CCameraDlg(CWnd* pParent, UINT64 addr);   // �W���R���X�g���N�^�[
	virtual ~CCameraDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_CAMERA_DIALOG };

protected:
	CStatic m_pictCamera;
	UINT64 m_deviceAddr;
	CStatic m_lblDeviceId;
	bool m_recording;
	CButton m_buttonRec;
	CAviFile* m_aviFile;
	bool m_IgnoreReceive;
	HBITMAP m_lastReceivedBitmap;


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnBluetoothReceived(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual void PostNcDestroy();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedLightOn();
	afx_msg void OnBnClickedLightOff();
	afx_msg void OnBnClickedButtonRec();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	CStatic m_lblCreatedTime;
};
