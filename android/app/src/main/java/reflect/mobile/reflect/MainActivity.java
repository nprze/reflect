/*package reflect.mobile.reflect;
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
    private native void resizedSurface(int width, int height);
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
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                resizedSurface(width, height);
            }

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

    @Override
    protected void onPause() {
        super.onPause();
        resizedSurface(0,0);
    }

    private void startRendering() {
        new Thread(() -> {
            while (true) {
                List<InputEvent> eventsToSend;
                synchronized (eventQueue) {
                    eventsToSend = new ArrayList<>(eventQueue);
                    eventQueue.clear();
                }
                if (!eventsToSend.isEmpty()) sendEventsToNative(eventsToSend);
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
    */

    package reflect.mobile.reflect;

import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Choreographer;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity implements Choreographer.FrameCallback {
    Surface surface = null;
    static boolean called = false;
    public native void createVulkanApp(Surface surface);
    private native void sendEventsToNative(List<InputEvent> events);
    private native void renderNative();
    private native void resizedSurface(int width, int height, Surface surface);
    public native void readAndCopyFile(String dirPath);

    static {
        System.loadLibrary("reflect");
    }

    private final List<InputEvent> eventQueue = new ArrayList<>();
    private boolean isRendering = false;
    private boolean appCreated = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SurfaceView surfaceView = findViewById(R.id.surface_view);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                surface = holder.getSurface();
                createVulkanApp(surface);
                appCreated = true;
                startRendering();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                surface = holder.getSurface();
                if(!appCreated) return;
                if (!called){
                    called = true;
                    return;
                }
                resizedSurface(width, height, holder.getSurface());
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                stopRendering();
            }
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

    @Override
    protected void onPause() {
        super.onPause();
        if (!appCreated) return;
        resizedSurface(0, 0, surface);
        stopRendering();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(!appCreated)return;
        startRendering();
    }

    private void startRendering() {
        if (!isRendering || !appCreated) {
            isRendering = true;
            Choreographer.getInstance().postFrameCallback(this);
        }
    }

    private void stopRendering() {
        isRendering = false;
        Choreographer.getInstance().removeFrameCallback(this);
    }
    @Override
    public void doFrame(long frameTimeNanos) {
        if (!isRendering || !appCreated) return;

        List<InputEvent> eventsToSend;
        synchronized (eventQueue) {
            eventsToSend = new ArrayList<>(eventQueue);
            eventQueue.clear();
        }
        if (!eventsToSend.isEmpty()) sendEventsToNative(eventsToSend);

        renderNative();

        Choreographer.getInstance().postFrameCallback(this);
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
