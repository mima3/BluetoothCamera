// CCameraDlg.cpp : 実装ファイル
#include "stdafx.h"
#include "BluetoothCameraServer.h"
#include "CameraDlg.h"
#include "afxdialogex.h"
#include "BluetoothAppDefine.h"
#include "BluetoothCameraReceiver.h"
#include "BluetoothAppDefine.h"
#include "resource.h"
#include "AviFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCameraDlg ダイアログ

IMPLEMENT_DYNAMIC(CCameraDlg, CDialogEx)

#define RECORD_TIMER_ID 1


CCameraDlg::CCameraDlg(CWnd* pParent, UINT64 addr)
	: CDialogEx(CCameraDlg::IDD, pParent),
	  m_deviceAddr(addr),
	  m_recording(false),
	  m_aviFile(NULL),
	  m_lastReceivedBitmap(NULL),
	  m_IgnoreReceive(false)
{
	Create(CCameraDlg::IDD, pParent);
	ShowWindow(SW_SHOW);
}

CCameraDlg::~CCameraDlg()
{
	if (m_aviFile) {
		delete m_aviFile;
		m_aviFile = NULL;
	}
}

void CCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CAMERA, m_pictCamera);
	DDX_Control(pDX, IDC_DEVICE_ID, m_lblDeviceId);
	DDX_Control(pDX, IDC_BUTTON_REC, m_buttonRec);
}


BEGIN_MESSAGE_MAP(CCameraDlg, CDialogEx)
	ON_MESSAGE(WM_BLUETOOTH_RECEIVED, &CCameraDlg::OnBluetoothReceived)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LIGHT_ON, &CCameraDlg::OnBnClickedLightOn)
	ON_BN_CLICKED(IDC_LIGHT_OFF, &CCameraDlg::OnBnClickedLightOff)
	ON_BN_CLICKED(IDC_BUTTON_REC, &CCameraDlg::OnBnClickedButtonRec)
	ON_WM_TIMER()
END_MESSAGE_MAP()

HBITMAP __stdcall CopyBitmap(HWND hWnd, HBITMAP hBmSrc)
{
	BITMAP bm;
	::GetObject(hBmSrc, sizeof(BITMAP), &bm);

	HDC hDC = ::GetDC(hWnd);
	HDC hDcDst = ::CreateCompatibleDC(hDC);
	HBITMAP hBmDst = ::CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);
	if (hBmDst) {
		HGDIOBJ hlast1, hlast2;
		hlast1 = ::SelectObject(hDcDst, hBmDst);

		HDC hDcSrc = ::CreateCompatibleDC(hDC);
		hlast2 = ::SelectObject(hDcSrc, hBmSrc);

		::BitBlt(hDcDst, 0, 0, bm.bmWidth, bm.bmHeight, hDcSrc, 0, 0, SRCCOPY);

		::SelectObject(hDcSrc, hlast2);
		::DeleteObject(hDcSrc);
		//::DeleteObject(hBmSrc); // 引数だから削除しない
		::SelectObject(hDcDst, hlast1);
	}
	::DeleteObject(hDcDst);
	::ReleaseDC(hWnd, hDC);

	return hBmDst;
}
/**
 * Bluetoothからの画像取得
 * @param [in] wParam HBITMAP
 * @param [in] lParam NULL
 */
afx_msg LRESULT CCameraDlg::OnBluetoothReceived(WPARAM wParam, LPARAM /*lParam*/) {
	if (m_IgnoreReceive) {
		return 0;
	}
	CameraImageData* pData = (CameraImageData*)wParam;
	HBITMAP hBmp = pData->hBitmap;
	if (this->m_aviFile) {
		m_lastReceivedBitmap = CopyBitmap(NULL, hBmp);
	}

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


/**
 * レコードの切り替え
 */
void CCameraDlg::OnBnClickedButtonRec()
{
	if (!m_recording) {

		// レコード開始
		m_IgnoreReceive = true;
		LPCTSTR cszFilter = L"avi(*.avi)|*.avi|\nAll(*.*)|*.*|\0";
		CFileDialog dlg(FALSE, L"AVI", L"", OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT, cszFilter, this);
		if (dlg.DoModal() != IDOK)
		{
			m_IgnoreReceive = false;
			return;
		}
		m_IgnoreReceive = false;
		//m_aviFile = new CAviFile(dlg.GetPathName(), mmioFOURCC('M', 'S', 'V', 'C'), 10);
		m_aviFile = new CAviFile(dlg.GetPathName(), NULL, 10);
		SetTimer(RECORD_TIMER_ID, 100, NULL);
;		m_recording = true;
		m_buttonRec.SetWindowTextW(L"■");
	}
	else {
		// レコード停止
		if (m_aviFile) {
			delete m_aviFile;
			m_aviFile = NULL;
		}
		KillTimer(RECORD_TIMER_ID);
		m_recording = false;
		m_buttonRec.SetWindowTextW(L"●");
	}
}


void CCameraDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == RECORD_TIMER_ID) 
	{
		if (this->m_aviFile) {
			if (this->m_lastReceivedBitmap) {
				this->m_aviFile->AppendNewFrame(this->m_lastReceivedBitmap);
			}
		}

	}
	else 
	{
		CDialogEx::OnTimer(nIDEvent);
	}
}
