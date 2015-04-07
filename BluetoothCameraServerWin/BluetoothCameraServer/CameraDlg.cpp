// CCameraDlg.cpp : 実装ファイル
#include "stdafx.h"
#include "BluetoothCameraServer.h"
#include "CameraDlg.h"
#include "afxdialogex.h"
#include "BluetoothAppDefine.h"
#include "BluetoothCameraReceiver.h"
#include "BluetoothAppDefine.h"
#include "resource.h"

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
	DDX_Control(pDX, IDC_DEVICE_ID, m_lblDeviceId);
}


BEGIN_MESSAGE_MAP(CCameraDlg, CDialogEx)
	ON_MESSAGE(WM_BLUETOOTH_RECEIVED, &CCameraDlg::OnBluetoothReceived)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LIGHT_ON, &CCameraDlg::OnBnClickedLightOn)
	ON_BN_CLICKED(IDC_LIGHT_OFF, &CCameraDlg::OnBnClickedLightOff)
END_MESSAGE_MAP()


/**
 * Bluetoothからの画像取得
 * @param [in] wParam HBITMAP
 * @param [in] lParam NULL
 */
afx_msg LRESULT CCameraDlg::OnBluetoothReceived(WPARAM wParam, LPARAM /*lParam*/) {
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

	this->m_pictCamera.SetWindowPos(
		NULL,
		0,
		0,
		bm.bmWidth,
		bm.bmHeight,
		SWP_NOMOVE | SWP_NOZORDER
	);
	this->m_pictCamera.GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	this->SetWindowPos(
		NULL,
		0,
		0,
		bm.bmWidth + rect.left * 2 + 20,
		bm.bmHeight + rect.top + 50,
		SWP_NOMOVE | SWP_NOZORDER
	);

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
	/*
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
	}*/
}



void CCameraDlg::OnDestroy()
{
	this->GetParent()->SendMessage(WM_CLOSE_CAMERA_WINDOW, (WPARAM)&this->m_deviceAddr, NULL);
	CDialogEx::OnDestroy();
}


BOOL CCameraDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	WCHAR deviceId[256];
	wsprintf(deviceId, L"%I64x", this->m_deviceAddr);
	CString id = deviceId;
	this->m_lblDeviceId.SetWindowTextW(id.MakeUpper());

	return TRUE;  // return TRUE unless you set the focus to a control
}

/**
 * 証明ONボタン
 */
void CCameraDlg::OnBnClickedLightOn()
{
	this->GetParent()->SendMessage(WM_CAMERA_LIGHT_STATUS, (WPARAM)&this->m_deviceAddr, 1);
}


/**
* 証明OFFボタン
*/
void CCameraDlg::OnBnClickedLightOff()
{
	this->GetParent()->SendMessage(WM_CAMERA_LIGHT_STATUS, (WPARAM)&this->m_deviceAddr, 0);
}
