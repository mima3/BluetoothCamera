package jp.ne.needtec.bluetoothcameraclient;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Timer;


public class CameraPreviewActivity extends Activity {

    private Preview mPreview;
    private Camera mCamera; //hardware
    private BluetoothClient bluetoothClient;
    private boolean connected = false;
    Timer mTimer;
    private boolean acceptServer = true;
    CameraPreviewActivity self;

    BluetoothClient.BluetoothClientCallback clientCallback = new BluetoothClient.BluetoothClientCallback() {
        Handler handlerClose = new Handler() {
            @Override
            public void handleMessage(Message msg) {
            AlertDialog.Builder alertDialog = new AlertDialog.Builder(self);
            alertDialog.setMessage((String)msg.obj);
            alertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
                public void onCancel(DialogInterface dialog) {
                    self.finish();
                }
            });
            alertDialog.show();
            }
        };

        @Override
        public void onConnected(BluetoothClient client) {
            connected = true;

        }

        @Override
        public void onClose() {
            connected = false;
            bluetoothClient = null;
            Message msg = new Message();
            msg.what = 0;
            msg.obj = "切断されました。";
            handlerClose.sendMessage(msg);
        }

        @Override
        public void onReceive(byte[] buffer, int bytes) {
            ByteBuffer buf = ByteBuffer.wrap(buffer);
            if (bytes < 4) {
                return;
            }
            int code = buf.getInt(0);
            switch (code) {
                case BluetoothCameraNetInterface.DATA_CODE_SERVER_STATUS:
                    if (bytes < 8) {
                        return;
                    }
                    int status = buf.getInt(4);
                    if (status == 0) {
                        acceptServer = false;
                    } else {
                        acceptServer = true;
                    }
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        self = this;
        int clientId = getIntent().getIntExtra("CLIENT_ID", 0);
        BluetoothClientManager mgr = BluetoothClientManager.getInstance();
        bluetoothClient = mgr.getClient(clientId);
        bluetoothClient.connect(clientCallback);

        //SurfaceView
        mPreview = new Preview(this);
        setContentView(mPreview);

        /*
        mPreview.setOnClickListener(onSurfaceClickListener);
        mTimer = new Timer(true);
        mTimer.schedule( new TimerTask(){
            @Override
            public void run() {
                if (connected) {
                    mCamera.setOneShotPreviewCallback(previewCallback);
                }
            }
        }, 100, 2500);
        */
    }

    @Override
    protected void onResume() {
        super.onResume();
        // Open the default i.e. the first rear facing camera.
        mCamera = Camera.open();
        mPreview.setCamera(mCamera);
        mCamera.setPreviewCallback(previewCallback);
    }
    @Override
    protected void onPause() {
        super.onPause();
        // Because the Camera object is a shared resource, it's very
        // important to release it when the activity is paused.
        if (mCamera != null) {
            mCamera.setPreviewCallback(null);
            mPreview.setCamera(null);
            mCamera.release();
            mCamera = null;
        }
    }
    //Surfaceをクリックした時
    private View.OnClickListener onSurfaceClickListener = new View.OnClickListener(){
        @Override
        public void onClick(View view){
            if(mCamera != null){
                //AutoFocusを実行
                //myCamera.autoFocus(autoFocusCallback);
                //mCamera.setOneShotPreviewCallback(previewCallback);
            }
        }

    };

    //切り取った時（ここで撮影、各種画像処理を行う）
    private Camera.PreviewCallback previewCallback = new Camera.PreviewCallback(){

        //OnShotPreview時のbyte[]が渡ってくる
        @Override
        public void onPreviewFrame(byte[] bytes, Camera camera) {
            if (!acceptServer) {
                return;
            }
            camera.setPreviewCallback(null);
            int w = camera.getParameters().getPreviewSize().width;
            int h = camera.getParameters().getPreviewSize().height;
            ByteBuffer buf = ByteBuffer.allocate(8 + 4 * 3);
            buf.putInt(BluetoothCameraNetInterface.DATA_CODE_PICTURE);
            buf.putLong(bytes.length);
            buf.putInt(w);   // プレビューの幅
            buf.putInt(h);  // プレビューの高さ
            bluetoothClient.writeByte(buf.array());
            bluetoothClient.writeByte(bytes);
            camera.setPreviewCallback(this);
        }
    };
}

/**
 * A simple wrapper around a Camera and a SurfaceView that renders a centered preview of the Camera
 * to the surface. We need to center the SurfaceView because not all devices have cameras that
 * support preview sizes at the same aspect ratio as the device's display.
 */
class Preview extends ViewGroup implements SurfaceHolder.Callback {
    private final String TAG = "Preview";
    SurfaceView mSurfaceView;
    SurfaceHolder mHolder;
    Camera.Size mPreviewSize;
    Camera mCamera;
    Preview(Context context) {
        super(context);
        mSurfaceView = new SurfaceView(context);
        addView(mSurfaceView);
        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed.
        mHolder = mSurfaceView.getHolder();
        mHolder.addCallback(this);
        mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
    }
    public void setCamera(Camera camera) {
        mCamera = camera;
        if (mCamera != null) {
            requestLayout();
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        // We purposely disregard child measurements because act as a
        // wrapper to a SurfaceView that centers the camera preview instead
        // of stretching it.
        final int width = resolveSize(getSuggestedMinimumWidth(), widthMeasureSpec);
        final int height = resolveSize(getSuggestedMinimumHeight(), heightMeasureSpec);
        setMeasuredDimension(width, height);
    }
    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        if (changed && getChildCount() > 0) {
            final View child = getChildAt(0);
            final int width = r - l;
            final int height = b - t;
            int previewWidth = width;
            int previewHeight = height;
            if (mPreviewSize != null) {
                previewWidth = mPreviewSize.width;
                previewHeight = mPreviewSize.height;
            }
            // Center the child SurfaceView within the parent.
            if (width * previewHeight > height * previewWidth) {
                final int scaledChildWidth = previewWidth * height / previewHeight;
                child.layout((width - scaledChildWidth) / 2, 0,
                        (width + scaledChildWidth) / 2, height);
            } else {
                final int scaledChildHeight = previewHeight * width / previewWidth;
                child.layout(0, (height - scaledChildHeight) / 2,
                        width, (height + scaledChildHeight) / 2);
            }
        }
    }
    public void surfaceCreated(SurfaceHolder holder) {
        // The Surface has been created, acquire the camera and tell it where
        // to draw.
        try {
            if (mCamera != null) {
                mCamera.setPreviewDisplay(holder);
            }
        } catch (IOException exception) {
            Log.e(TAG, "IOException caused by setPreviewDisplay()", exception);
        }
    }
    public void surfaceDestroyed(SurfaceHolder holder) {
        // Surface will be destroyed when we return, so stop the preview.
        if (mCamera != null) {
            mCamera.stopPreview();
        }
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        // Now that the size is known, set up the camera parameters and begin
        // the preview.
        Camera.Parameters parameters = mCamera.getParameters();
        //parameters.setPreviewSize(mPreviewSize.width, mPreviewSize.height);
        requestLayout();
        mCamera.setParameters(parameters);
        mCamera.startPreview();
    }
}