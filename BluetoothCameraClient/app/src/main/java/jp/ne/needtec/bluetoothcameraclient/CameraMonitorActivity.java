package jp.ne.needtec.bluetoothcameraclient;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ImageView;

import java.nio.ByteBuffer;


public class CameraMonitorActivity extends Activity {
    private BluetoothServer bluetoothServer;
    private BluetoothDevice remoteDevice;
    private ReceivedData receivingData;
    private ImageView monitor;

    enum ReceivedDataStatus {
        None,	      // 未受信
        Receiving	  // 現在データを受信中
    };
    private class ReceivedData {
        public String deviceAddress;
        public ReceivedDataStatus status = ReceivedDataStatus.None;
        public int bufferSize = 0;
        public int currentPos = 0;
        public byte[] buffer;
        public int width = 0;
        public int height = 0;
        ReceivedData(String deviceAddress) {
            this.deviceAddress = deviceAddress;
            this.currentPos = 0;
        }
    };

    BluetoothServer.BluetoothServerCallback serverCallback = new BluetoothServer.BluetoothServerCallback() {
        Handler handlerReceivedImage = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                monitor.setImageBitmap((Bitmap)msg.obj);
            }
        };

        @Override
        public void onConnected(BluetoothSocket socket) {
            remoteDevice = socket.getRemoteDevice();
            receivingData = new ReceivedData(remoteDevice.getAddress());
        }

        @Override
        public void onClose() {
            receivingData = null;
        }

        @Override
        public void onError(String errorMsg) {
        }

        @Override
        public void onReceive(byte[] buffer, int bytes) {
            // 新規受信
            ByteBuffer buf = ByteBuffer.wrap(buffer);
            int offset = 0;
            if (receivingData.status == ReceivedDataStatus.None) {
                if (bytes < 4) {
                    return;
                }
                int code = buf.getInt(offset);
                offset += 4;
                switch (code) {
                    case BluetoothCameraNetInterface.DATA_CODE_PICTURE: {
                        receivingData.currentPos = 0;
                        receivingData.bufferSize = (int) buf.getLong(offset);
                        offset += 8;
                        receivingData.width = buf.getInt(offset);
                        offset += 4;
                        receivingData.height = buf.getInt(offset);
                        offset += 4;
                        receivingData.buffer = new byte[receivingData.bufferSize];
                        receivingData.status = ReceivedDataStatus.Receiving;
                        break;
                    }
                }
            }
            if (receivingData.status == ReceivedDataStatus.Receiving) {
                // 受信中
                int cpSize = bytes - offset;
                if (cpSize > receivingData.bufferSize - receivingData.currentPos) {
                    cpSize = receivingData.bufferSize - receivingData.currentPos;
                }
                System.arraycopy(buffer, offset, receivingData.buffer, receivingData.currentPos, bytes - offset);
                receivingData.currentPos += cpSize;
                if (receivingData.currentPos >= receivingData.bufferSize) {
                    // 全て受信
                    int[] rgb = new int[(receivingData.width * receivingData.height)];
                    Bitmap bmp = Bitmap.createBitmap(receivingData.width, receivingData.height, Bitmap.Config.ARGB_8888);
                    decodeYUV420SP(rgb, receivingData.buffer, receivingData.width, receivingData.height);
                    bmp.setPixels(rgb, 0, receivingData.width, 0, 0, receivingData.width, receivingData.height);
                    receivingData.currentPos = 0;
                    receivingData.bufferSize = 0;
                    receivingData.buffer = null;
                    receivingData.status = ReceivedDataStatus.None;
                    Message msg = new Message();
                    msg.what = 0;
                    msg.obj = bmp;
                    handlerReceivedImage.sendMessage(msg);

                }
            }

        }
    };

    private static final void decodeYUV420SP(int[] rgb, byte[] yuv420sp, int width, int height) {
        final int frameSize = width * height;
        for (int j = 0, yp = 0; j < height; j++) {
            int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
            for (int i = 0; i < width; i++, yp++) {
                int y = (0xff & ((int) yuv420sp[yp])) - 16;
                if (y < 0) y = 0;
                if ((i & 1) == 0) {
                    v = (0xff & yuv420sp[uvp++]) - 128;
                    u = (0xff & yuv420sp[uvp++]) - 128;
                }
                int y1192 = 1192 * y;
                int r = (y1192 + 1634 * v);
                int g = (y1192 - 833 * v - 400 * u);
                int b = (y1192 + 2066 * u);
                if (r < 0) r = 0;
                else if (r > 262143) r = 262143;
                if (g < 0) g = 0;
                else if (g > 262143) g = 262143;
                if (b < 0) b = 0;
                else if (b > 262143) b = 262143;
                rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        this.bluetoothServer = new BluetoothServer();
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera_monitor);
        this.bluetoothServer.accept(serverCallback);
        this.monitor = (ImageView)this.findViewById(R.id.id_image_monitor);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_camera_monitor, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
