
// BluetoothCameraServer.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CBluetoothCameraServerApp:
// このクラスの実装については、BluetoothCameraServer.cpp を参照してください。
//

class CBluetoothCameraServerApp : public CWinApp
{
public:
	CBluetoothCameraServerApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CBluetoothCameraServerApp theApp;