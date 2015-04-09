package jp.ne.needtec.bluetoothcameraclient;

import android.bluetooth.BluetoothDevice;

import java.util.HashMap;

/**
 * BluetoothClientの管理クラス
 * このクラスを経由してActivity間でBluetoothクラスの受け渡しを行う
 * Created by m.ita on 2015/03/31.
 */
public class BluetoothClientManager {
    private static BluetoothClientManager instance;
    private static int nextId = 1;
    private static HashMap<Integer, BluetoothClient> clients = new HashMap<>();
    private BluetoothClientManager() {
        instance = null;
    }
    public static BluetoothClientManager getInstance() {
        if (instance == null) {
            instance = new BluetoothClientManager();
        }
        return instance;
    }
    public static int createClient(BluetoothDevice device) {
        BluetoothClient c = new BluetoothClient(device);
        int id = nextId++;
        clients.put(id, c);
        return id;
     }

    public static BluetoothClient getClient(int id) {
        return clients.get(id);
    }
}
