#include "stdafx.h"
#include "BluetoothCameraServer.h"
#include "BluetoothCameraServerDlg.h"
#include "BluetoothAppDefine.h"
#include "afxdialogex.h"
#include "CameraDlg.h"
#include "BluetoothCameraSender.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// ����
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


// CBluetoothCameraServerDlg �_�C�A���O


/**
 * CBluetoothCameraServerDlg�̃R���X�g���N�^
 */
CBluetoothCameraServerDlg::CBluetoothCameraServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBluetoothCameraServerDlg::IDD, pParent),
	m_bluetoothReceiver(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBluetoothCameraServerDlg::~CBluetoothCameraServerDlg() {
}

void CBluetoothCameraServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_BLUETOOTH_LOG, m_lstBluetoothLog);
}

BEGIN_MESSAGE_MAP(CBluetoothCameraServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CBluetoothCameraServerDlg::OnBnClickedConnect)
	ON_MESSAGE(WM_BLUETOOTH_RECEIVED, &CBluetoothCameraServerDlg::OnBluetoothReceived)
	ON_MESSAGE(WM_BLUETOOTH_CONNECTED, &CBluetoothCameraServerDlg::OnBluetoothConnected)
	ON_MESSAGE(WM_BLUETOOTH_ERROR, &CBluetoothCameraServerDlg::OnBluetoothError)
	ON_MESSAGE(WM_CLOSE_CAMERA_WINDOW, &CBluetoothCameraServerDlg::OnCloseCameraWindow)
	ON_MESSAGE(WM_CAMERA_LIGHT_STATUS, &CBluetoothCameraServerDlg::OnCameraLightStatus)
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CBluetoothCameraServerDlg ���b�Z�[�W �n���h���[

BOOL CBluetoothCameraServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->m_bluetoothReceiver = new CBluetoothCameraReceiver(this->m_hWnd);

	// "�o�[�W�������..." ���j���[���V�X�e�� ���j���[�ɒǉ����܂��B

	// IDM_ABOUTBOX �́A�V�X�e�� �R�}���h�͈͓̔��ɂȂ���΂Ȃ�܂���B
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

	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R���̐ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R���̐ݒ�

	// TODO: �������������ɒǉ����܂��B

	return TRUE;  // �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
}

void CBluetoothCameraServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �_�C�A���O�ɍŏ����{�^����ǉ�����ꍇ�A�A�C�R����`�悷�邽�߂�
//  ���̃R�[�h���K�v�ł��B�h�L�������g/�r���[ ���f�����g�� MFC �A�v���P�[�V�����̏ꍇ�A
//  ����́AFramework �ɂ���Ď����I�ɐݒ肳��܂��B

void CBluetoothCameraServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N���C�A���g�̎l�p�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R���̕`��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ���[�U�[���ŏ��������E�B���h�E���h���b�O���Ă���Ƃ��ɕ\������J�[�\�����擾���邽�߂ɁA
//  �V�X�e�������̊֐����Ăяo���܂��B
HCURSOR CBluetoothCameraServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



/**
 * �T�[�o�[�̐ڑ��J�n
 */
void CBluetoothCameraServerDlg::OnBnClickedConnect()
{
	GUID guid = { 0x2c242765, 0xaee4, 0x41eb, { 0xa7, 0xe3, 0xb2, 0x8d, 0x39, 0xb7, 0x5d, 0x33 } };
	this->WriteLog(L"Service: %I64x is start...", guid);
	this->m_bluetoothServer.StartService(guid, m_bluetoothReceiver);
}

/**
 * Bluetooth����̉摜��M
 */
afx_msg LRESULT CBluetoothCameraServerDlg::OnBluetoothReceived(WPARAM wParam, LPARAM lParam) {
	//this->m_mapCameraWindows[];
	CameraImageData* pData = (CameraImageData*)wParam;
	CWnd* pWnd = this->m_mapCameraWindows[pData->deviceId];
	pWnd->SendMessage(WM_BLUETOOTH_RECEIVED, (WPARAM)pData, NULL);
	delete pData;
	return 0;
}

/**
* Bluetooth����̐ڑ��J�n
*/
afx_msg LRESULT CBluetoothCameraServerDlg::OnBluetoothConnected(WPARAM wParam, LPARAM lParam) {
	UINT64* addr = (UINT64*)wParam;
	if (addr != NULL) {
		CCameraDlg* pDlg = new CCameraDlg(this, *addr);
		this->m_mapCameraWindows[*addr] = pDlg;

		this->WriteLog(L"Connected: %I64x", *addr);

		return 0;
	}
	return 0;
}

/**
 * Bluetooth�̃G���[�����C�x���g
 */
afx_msg LRESULT CBluetoothCameraServerDlg::OnBluetoothError(WPARAM wParam, LPARAM lParam) {
	WCHAR* msg = (WCHAR*)wParam;
	this->WriteLog(L"OnBluetoothError: %s" , msg);

	return 0;
}

/**
 * �J�����p�̃E�B���h�E�I���C�x���g
 */
afx_msg LRESULT CBluetoothCameraServerDlg::OnCloseCameraWindow(WPARAM wParam, LPARAM lParam) {
	UINT64 *deviceAddr = (UINT64*)wParam;
	this->m_bluetoothServer.CloseSocket(*deviceAddr);
	this->WriteLog(L"Close Windows: %I64x", *deviceAddr);
	return 0;
}

/**
* �J�����̏Ɩ��ύX
*/
afx_msg LRESULT CBluetoothCameraServerDlg::OnCameraLightStatus(WPARAM wParam, LPARAM lParam) {
	UINT64 *deviceAddr = (UINT64*)wParam;
	SOCKET socket = this->m_bluetoothServer.GetSocketByAddress(*deviceAddr);
	CBluetoothCameraSender::SetLightStatus(socket, (UINT32)lParam);
	return 0;
}


void CBluetoothCameraServerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	this->m_bluetoothServer.StopService();
	/*
	for (map<UINT64, CWnd*>::iterator itr = this->m_mapCameraWindows.begin();
		itr != this->m_mapCameraWindows.end(); ++itr) {
		delete itr->second;
	}
	*/

	this->m_mapCameraWindows.clear();
	if (m_bluetoothReceiver) {
		delete m_bluetoothReceiver;
	}
}


void CBluetoothCameraServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rect;
	if (this->m_lstBluetoothLog.GetSafeHwnd()) {
		this->m_lstBluetoothLog.GetWindowRect(&rect);
		this->ScreenToClient(&rect);
		this->m_lstBluetoothLog.SetWindowPos(
			NULL,
			0,
			0,
			cx - rect.left * 2,
			cy - rect.top,
			SWP_NOMOVE | SWP_NOZORDER
		);
	}
}

/**
 * ���O�̋L�q
 * @param [in] format ����
 */
void CBluetoothCameraServerDlg::WriteLog(WCHAR* format, ...) {
	WCHAR msg[1024];
	va_list arg;
	va_start(arg, format);
	vswprintf(msg, 1024, format, arg);
	va_end(arg);
	CTime cTime = CTime::GetCurrentTime();
	CString timestamp = cTime.Format("%Y/%m/%d %H:%M:%S");
	this->m_lstBluetoothLog.AddString(timestamp + ' ' + msg);
	if (this->m_lstBluetoothLog.GetCount() > 500) {
		this->m_lstBluetoothLog.DeleteString(0);
	}
	this->m_lstBluetoothLog.SetCurSel(this->m_lstBluetoothLog.GetCount() - 1);
}