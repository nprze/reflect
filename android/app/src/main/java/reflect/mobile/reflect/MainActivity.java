package reflect.mobile.reflect;
import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {
    public native void createVulkanApp(Surface surface);
    private native void sendEventsToNative(List<InputEvent> events);
    private native void renderNative();
    public native void readAndCopyFile(String dirPath);

    static {
        System.loadLibrary("reflect");
    }

    private final List<InputEvent> eventQueue = new ArrayList<>();


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SurfaceView surfaceView = findViewById(R.id.surface_view);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Surface surface = holder.getSurface();
                createVulkanApp(surface);
                startRendering();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {}

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {}
        });
        FileHelper.copyAssetsToInternalStorage(this);
        readAndCopyFile(getFilesDir().getAbsolutePath());
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        synchronized (eventQueue) {
            eventQueue.add(new InputEvent(event.getAction(), event.getX(), event.getY(), event.getEventTime()));
        }
        return true;
    }

    private void startRendering() {
        new Thread(() -> {
            while (true) {
                List<InputEvent> eventsToSend;
                synchronized (eventQueue) {
                    eventsToSend = new ArrayList<>(eventQueue);
                    eventQueue.clear();
                }
                if (!eventsToSend.isEmpty())
                sendEventsToNative(eventsToSend);
                renderNative();
                try {
                    Thread.sleep(16);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    static class InputEvent {
        int action;
        float x, y;
        long timestamp;

        InputEvent(int action, float x, float y, long timestamp) {
            this.action = action;
            this.x = x;
            this.y = y;
            this.timestamp = timestamp;
        }
    }
}
