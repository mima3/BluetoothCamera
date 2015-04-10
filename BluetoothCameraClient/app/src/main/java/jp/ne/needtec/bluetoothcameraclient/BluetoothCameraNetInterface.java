package jp.ne.needtec.bluetoothcameraclient;

/**
 * C++側と共通の定義
 * Created by mitagaki on 2015/04/07.
 */
public class BluetoothCameraNetInterface {
    public static final int DATA_VERSION = 0x00000001;
    public static final int DATA_CODE_PICTURE = 0xffff0001;
    public static final int DATA_CODE_SERVER_STATUS = 0xffff0002;
    public static final int DATA_CODE_SET_LIGHT_STATUS = 0xffff0003;
    public static final String SERVICE_GUID = "2C242765-AEE4-41EB-A7E3-B28D39B75D33";
    public static final int IMAGE_FORMAT_YUV420 = 0x00000001;
}
