
// BluetoothCameraServerDlg.h : �w�b�_�[ �t�@�C��
//

#pragma once
#include "afxwin.h"
#include "BluetoothServer.h"
#include "BluetoothCameraReceiver.h"

// CBluetoothCameraServerDlg �_�C�A���O
class CBluetoothCameraServerDlg : public CDialogEx
{
// �R���X�g���N�V����
public:
	CBluetoothCameraServerDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^�[
	~CBluetoothCameraServerDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_BLUETOOTHCAMERASERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g

private:
	CBluetoothServer m_bluetoothServer;
	CBluetoothCameraReceiver* m_bluetoothReceiver;
	map<UINT64, CWnd*> m_mapCameraWindows;


	// ����
protected:
	HICON m_hIcon;
	CListBox m_lstBluetoothLog;
	void WriteLog(WCHAR* format, ...);

	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnBluetoothReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBluetoothConnected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBluetoothClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBluetoothError(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseCameraWindow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCameraLightStatus(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedConnect();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
