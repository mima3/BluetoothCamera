package jp.ne.needtec.bluetoothcameraclient;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;


public class MainActivity extends Activity {
    BluetoothAdapter myBluetooth = BluetoothAdapter.getDefaultAdapter();
    Object[] pairedDeviceList;
    ListView listDevice;
    ArrayAdapter<String> listDeviceAdapter;
    List<String> deviceNameList = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        this.listDevice = (ListView)findViewById(R.id.id_listView_found_device);
        this.searchDevice();
        this.listDevice.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                BluetoothDevice device = (BluetoothDevice)pairedDeviceList[position];
                BluetoothClientManager mgr = BluetoothClientManager.getInstance();
                // インテントの生成
                Intent intent = new Intent(MainActivity.this, CameraPreviewActivity.class);
                int clientId = BluetoothClientManager.createClient(device);
                intent.putExtra("CLIENT_ID", clientId);
                startActivity(intent);
            }
        });

    }

    /**
     * ペアリング中のデバイスの検索をおこない、リストビューに追加を行う
     */
    protected void searchDevice() {
        this.listDeviceAdapter = new ArrayAdapter<>(
                this,
                android.R.layout.simple_list_item_1,
                this.deviceNameList
        );
        this.listDevice.setAdapter(this.listDeviceAdapter);

        Set<BluetoothDevice> pairedDevices = myBluetooth.getBondedDevices();
        pairedDeviceList = pairedDevices.toArray();
        for (int i = 0; i < pairedDeviceList.length; ++i) {
            BluetoothDevice device = (BluetoothDevice)pairedDeviceList[i];
            this.deviceNameList.add(device.getName() + " " + device.getAddress());
        }
    }

    /**
     * モニターの受信を開始
     * @param v View
     */
    public void onStartMonitorReceiving(View v) {
        Intent intent = new Intent(MainActivity.this, CameraMonitorActivity.class);
        startActivity(intent);
    }
}
