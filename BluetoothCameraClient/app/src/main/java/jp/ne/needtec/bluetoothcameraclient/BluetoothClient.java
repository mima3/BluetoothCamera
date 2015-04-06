package jp.ne.needtec.bluetoothcameraclient;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.util.UUID;

/**
 * Created by m.ita on 2015/03/27.
 */

public class BluetoothClient {
    public interface BluetoothClientCallback {
        public void onConnected(BluetoothClient client);
        public void onClose();
        public void onReceive(String readMsg);
    }

    private ReadWriteThread readWriteThread = null;
    private ConnectThread connThread = null;
    private BluetoothDevice connectDevice;
    private BluetoothClientCallback callback;

    BluetoothClient(BluetoothDevice device) {
        this.connectDevice = device;
    }

    public void connect(BluetoothClientCallback cb)
    {
        this.connThread = new ConnectThread(this.connectDevice);
        this.connThread.start();
        this.callback = cb;
    }

    public void close()
    {
        if (this.connThread != null) {
            this.connThread.close();
        }
        if (this.readWriteThread != null) {
            this.readWriteThread.close();
        }
    }

    public boolean writeByte(byte[] bytes) {
        if (this.readWriteThread == null) {
            return false;
        }
        this.readWriteThread.write(bytes);
        return true;
    }


    public boolean writeMessage(String msg) {
        if (this.readWriteThread == null) {
            return false;
        }
        try {
            byte[] bytes = msg.getBytes("UTF-8");
            this.readWriteThread.write(bytes);
        } catch (UnsupportedEncodingException e) {
            return false;
        }
        return true;
    }

    /**
     * Server, Client共通 接続が確立した際に呼び出される
     */
    private void manageConnectedSocket(BluetoothSocket socket) {
        Log.i("BluetoothClient", "Connection");
        this.readWriteThread = new ReadWriteThread(socket);
        this.readWriteThread.start();
        this.callback.onConnected(this);
    }

    private class ConnectThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final UUID serviceUUID = UUID.fromString("2C242765-AEE4-41EB-A7E3-B28D39B75D33");

        public ConnectThread(BluetoothDevice device) {
            BluetoothSocket tmp = null;
            try {
                tmp = device.createRfcommSocketToServiceRecord(serviceUUID);
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }
            this.mmSocket = tmp;
        }
        public void run() {
            try {
                this.mmSocket.connect();
            } catch (IOException connectException) {
                try {
                    Log.e("BluetoothClient", connectException.getMessage());
                    mmSocket.close();
                } catch (IOException e) {
                    Log.e("BluetoothClient", e.getMessage());
                }
                return;
            }
            manageConnectedSocket(this.mmSocket);
        }

        /**
         * キャンセル時に呼ばれる
         */
        public void close() {
            try {
                mmSocket.close();
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }
        }
    }
    /**
     * 接続確立時のデータ送受信用のThread
     */
    private class ReadWriteThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ReadWriteThread(BluetoothSocket socket) {
            Log.i("BluetoothClient", "ConnectedThread");
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }

            // データ受信用
            mmInStream = tmpIn;
            // データ送信用
            mmOutStream = tmpOut;
        }

        public void run() {
            Log.i("BluetoothClient", "ConnectionThread#run()");
            byte[] buffer = new byte[1024];
            int bytes;

            // Whileループで入力が入ってくるのを常時待機
            while (true) {
                try {
                    // InputStreamから値を取得
                    bytes = mmInStream.read(buffer);
                    // 取得したデータをStringの変換
                    String readMsg = new String(buffer, 0, bytes, "UTF-8");
                    // Logに表示
                    Log.d("BluetoothClient", "GET: " + readMsg);
                    callback.onReceive(readMsg);

                } catch (IOException e) {
                    Log.e("BluetoothClient", e.getMessage());
                    callback.onClose();
                    break;
                }
            }
        }

        /**
         * 書き込み処理
         */
        public void write(byte[] bytes) {
            final int buffSize = 1024;
            try {
                int offset = 0;
                for (; offset < bytes.length - buffSize; offset = offset + buffSize) {
                    mmOutStream.write(bytes, offset, buffSize);
                }
                if (offset < bytes.length) {
                    mmOutStream.write(bytes, offset, bytes.length - offset);
                }
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }
        }

        /**
         * キャンセル時に呼ばれる
         */
        public void close() {
            try {
                mmSocket.close();
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }
        }
    }
}
