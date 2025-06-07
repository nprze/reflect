package reflect.mobile.reflect;

import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Choreographer;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

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

        getWindow().setDecorFitsSystemWindows(false);
        final WindowInsetsController insetsController = getWindow().getInsetsController();
        if (insetsController != null) {
            insetsController.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
            insetsController.setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            );
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int actionMasked = event.getActionMasked(); // Properly handle multi-touch actions
        int pointerIndex = event.getActionIndex();  // The index of the pointer that caused the event
        int pointerId = event.getPointerId(pointerIndex);
        int action;

        switch (actionMasked) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                action = 0; // DOWN
                synchronized (eventQueue) {
                    eventQueue.add(new InputEvent(action, event.getX(pointerIndex), event.getY(pointerIndex), pointerId));
                }
                break;

            case MotionEvent.ACTION_MOVE:
                action = 2; // MOVE
                synchronized (eventQueue) {
                    for (int i = 0; i < event.getPointerCount(); i++) {
                        int id = event.getPointerId(i);
                        float x = event.getX(i);
                        float y = event.getY(i);
                        eventQueue.add(new InputEvent(action, x, y, id));
                    }
                }
                break;

            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                action = 1; // UP
                synchronized (eventQueue) {
                    eventQueue.add(new InputEvent(action, event.getX(pointerIndex), event.getY(pointerIndex), pointerId));
                }
                break;

            case MotionEvent.ACTION_CANCEL:
                return true;

            default:
                return true;
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
        int pointerID;
        InputEvent(int action, float x, float y, int pointerid) {
            this.action = action;
            this.x = x;
            this.y = y;
            this.pointerID = pointerid;
        }
    }
}
