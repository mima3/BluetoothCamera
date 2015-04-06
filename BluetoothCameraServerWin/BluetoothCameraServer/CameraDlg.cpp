// CCameraDlg.cpp : �����t�@�C��
#include "stdafx.h"
#include "BluetoothCameraServer.h"
#include "CameraDlg.h"
#include "afxdialogex.h"
#include "BluetoothAppDefine.h"
#include "BluetoothCameraReceiver.h"
#include "BluetoothAppDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCameraDlg �_�C�A���O

IMPLEMENT_DYNAMIC(CCameraDlg, CDialogEx)

CCameraDlg::CCameraDlg(CWnd* pParent, UINT64 addr)
	: CDialogEx(CCameraDlg::IDD, pParent),
	  m_deviceAddr(addr)
{
	Create(CCameraDlg::IDD, pParent);
	ShowWindow(SW_SHOW);
}

CCameraDlg::~CCameraDlg()
{
}

void CCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CAMERA, m_pictCamera);
}


BEGIN_MESSAGE_MAP(CCameraDlg, CDialogEx)
	ON_MESSAGE(WM_BLUETOOTH_RECEIVED, &CCameraDlg::OnBluetoothReceived)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CCameraDlg ���b�Z�[�W �n���h���[
afx_msg LRESULT CCameraDlg::OnBluetoothReceived(WPARAM wParam, LPARAM lParam) {
	CameraImageData* pData = (CameraImageData*)wParam;
	HBITMAP hBmp = pData->hBitmap;

	CDC* pDC = m_pictCamera.GetDC();

	//�f�o�C�X�R���e�L�X�g�̑傫���擾
	CRect rect;
	m_pictCamera.GetClientRect(&rect);

	//�w�i��(�����y���E�����u���V)
	pDC->Rectangle(&rect);

	//�r�b�g�}�b�v�p�ϐ�
	CBitmap bmp;
	bmp.Attach(hBmp);

	//�݊��f�o�C�X�R���e�L�X���쐬
	CDC bmpDC;
	bmpDC.CreateCompatibleDC(pDC);

	//�݊��f�o�C�X�R���e�L�X�g�Ƀr�b�g�}�b�v�摜��ݒ�
	CBitmap* oldbmp = bmpDC.SelectObject(&bmp);
	BITMAP bm;
	bmp.GetObject(sizeof(BITMAP), &bm);


	//�݊��f�o�C�X�R���e�L�X�g�̓��e��pDC�ɓ]��
	pDC->BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &bmpDC, 0, 0, SRCCOPY);

	//������
	bmpDC.SelectObject(oldbmp);
	bmp.DeleteObject();

	return 0;
}

void CCameraDlg::PostNcDestroy()
{
	delete this;
}


void CCameraDlg::OnCancel()
{
	DestroyWindow();
}


void CCameraDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rect;
	if (this->m_pictCamera.GetSafeHwnd()) {
		this->m_pictCamera.GetWindowRect(&rect);
		this->ScreenToClient(&rect);
		this->m_pictCamera.SetWindowPos(
			NULL,
			0,
			0,
			cx - rect.left * 2,
			cy - rect.top,
			SWP_NOMOVE | SWP_NOZORDER
			);
	}
}


void CCameraDlg::OnClose()
{
	CDialogEx::OnClose();
}


void CCameraDlg::OnDestroy()
{
	this->GetParent()->SendMessage(WM_CLOSE_CAMERA_WINDOW, (WPARAM)&this->m_deviceAddr, NULL);
	CDialogEx::OnDestroy();
}
