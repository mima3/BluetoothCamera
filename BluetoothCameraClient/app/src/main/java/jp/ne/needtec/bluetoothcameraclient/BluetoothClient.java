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
 * Bluetoothのクライアント側の処理
 * Created by m.ita on 2015/03/27.
 */
public class BluetoothClient {
    public interface BluetoothClientCallback {
        public void onConnected(BluetoothClient client);
        public void onClose();
        public void onReceive(byte[] buffer, int bytes);
        public void onError(String errorMsg);
    }

    private ReadWriteThread readWriteThread = null;
    private ConnectThread connThread = null;
    private BluetoothDevice connectDevice;
    private BluetoothClientCallback callback;

    /**
     * コンストラクタ
     * @param device 接続対象のペアリング中のBluetoothDevice
     */
    BluetoothClient(BluetoothDevice device) {
       this.connectDevice = device;
    }

    /**
     * 接続を行う
     * @param cb コールバッククラス
     */
    public void connect(BluetoothClientCallback cb) {
        this.connThread = new ConnectThread(this.connectDevice);
        this.connThread.start();
        this.callback = cb;
    }

    /**
     * 接続の切断
     */
    public void close() {
        if (this.connThread != null) {
            this.connThread.close();
        }
        if (this.readWriteThread != null) {
            this.readWriteThread.close();
        }
    }

    /**
     * バイトを送信する
     * @param bytes 送信するバイト
     * @return true送信が成功
     */
    public boolean writeByte(byte[] bytes) {
        if (this.readWriteThread == null) {
            return false;
        }
        this.readWriteThread.write(bytes);
        return true;
    }

    /**
     * 文字列を送信する
     * @param msg 送信する文字列
     * @return true送信が成功
     */
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
     * @param socket 接続対象のSOCKET
     */
    private void manageConnectedSocket(BluetoothSocket socket) {
        Log.i("BluetoothClient", "Connection");
        this.readWriteThread = new ReadWriteThread(socket);
        this.readWriteThread.start();
        this.callback.onConnected(this);
    }

    /**
     * 接続用のスレッドクラス
     */
    private class ConnectThread extends Thread {
        private final BluetoothSocket mSocket;
        private final UUID serviceUUID = UUID.fromString(BluetoothCameraNetInterface.SERVICE_GUID);

        public ConnectThread(BluetoothDevice device) {
            BluetoothSocket tmp = null;
            try {
                tmp = device.createRfcommSocketToServiceRecord(serviceUUID);
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }
            this.mSocket = tmp;
        }
        public void run() {
            try {
                this.mSocket.connect();
            } catch (IOException connectException) {
                try {
                    Log.e("BluetoothClient", connectException.getMessage());
                    callback.onError(connectException.getMessage());
                    mSocket.close();
                } catch (IOException e) {
                    Log.e("BluetoothClient", e.getMessage());
                    callback.onError(e.getMessage());
                }
                return;
            }
            manageConnectedSocket(this.mSocket);
        }

        /**
         * キャンセル時に呼ばれる
         */
        public void close() {
            try {
                mSocket.close();
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }
        }
    }
    /**
     * 接続確立時のデータ送受信用のThread
     */
    private class ReadWriteThread extends Thread {
        private final BluetoothSocket mSocket;
        private final InputStream mInStream;
        private final OutputStream mOutStream;

        public ReadWriteThread(BluetoothSocket socket) {
            Log.i("BluetoothClient", "ConnectedThread");
            mSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }

            // データ受信用
            mInStream = tmpIn;
            // データ送信用
            mOutStream = tmpOut;
        }

        public void run() {
            Log.i("BluetoothClient", "ConnectionThread#run()");
            byte[] buffer = new byte[1024];
            int bytes;

            // Whileループで入力が入ってくるのを常時待機
            while (true) {
                try {
                    // InputStreamから値を取得
                    bytes = mInStream.read(buffer);
                    // 取得したデータをStringの変換
                    //String readMsg = new String(buffer, 0, bytes, "UTF-8");
                    // Logに表示
                    //Log.d("BluetoothClient", "GET: " + readMsg);
                    callback.onReceive(buffer, bytes);

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
                    mOutStream.write(bytes, offset, buffSize);
                }
                if (offset < bytes.length) {
                    mOutStream.write(bytes, offset, bytes.length - offset);
                }
                mOutStream.flush();
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }
        }

        /**
         * キャンセル時に呼ばれる
         */
        public void close() {
            try {
                mSocket.close();
            } catch (IOException e) {
                Log.e("BluetoothClient", e.getMessage());
            }
        }
    }
}
