#pragma once
class CBluetoothCameraSender
{
public:
	static void SetLightStatus(SOCKET socket, UINT32 sts);
	static void SetServerStatus(SOCKET socket, UINT32 status);
};

