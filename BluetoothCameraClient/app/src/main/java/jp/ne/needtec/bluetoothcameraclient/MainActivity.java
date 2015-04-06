package jp.ne.needtec.bluetoothcameraclient;

import android.app.Activity;
import android.app.ProgressDialog;
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
                int clientId = mgr.createClient(device);
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

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
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
