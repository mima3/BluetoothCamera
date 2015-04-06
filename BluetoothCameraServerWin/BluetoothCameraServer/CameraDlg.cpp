// CCameraDlg.cpp : 実装ファイル
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

// CCameraDlg ダイアログ

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


// CCameraDlg メッセージ ハンドラー
afx_msg LRESULT CCameraDlg::OnBluetoothReceived(WPARAM wParam, LPARAM lParam) {
	CameraImageData* pData = (CameraImageData*)wParam;
	HBITMAP hBmp = pData->hBitmap;

	CDC* pDC = m_pictCamera.GetDC();

	//デバイスコンテキストの大きさ取得
	CRect rect;
	m_pictCamera.GetClientRect(&rect);

	//背景白(初期ペン・初期ブラシ)
	pDC->Rectangle(&rect);

	//ビットマップ用変数
	CBitmap bmp;
	bmp.Attach(hBmp);

	//互換デバイスコンテキスを作成
	CDC bmpDC;
	bmpDC.CreateCompatibleDC(pDC);

	//互換デバイスコンテキストにビットマップ画像を設定
	CBitmap* oldbmp = bmpDC.SelectObject(&bmp);
	BITMAP bm;
	bmp.GetObject(sizeof(BITMAP), &bm);


	//互換デバイスコンテキストの内容をpDCに転送
	pDC->BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &bmpDC, 0, 0, SRCCOPY);

	//初期化
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
