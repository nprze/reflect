package reflect.mobile.reflect;
import android.app.Activity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Choreographer;
import android.widget.Toast;

public class MainActivity extends Activity {

    // Load the native library
    static {
        System.loadLibrary("reflect");
    }

    // Declare the native method to create Vulkan surface
    public native void createVulkanSurface(Surface surface);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Find the SurfaceView (which holds a Surface object)
        SurfaceView surfaceView = findViewById(R.id.surface_view);

        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Surface surface = holder.getSurface();
                if (surface != null) {
                    // Use Choreographer to wait for one frame before creating Vulkan surface
                    Choreographer.getInstance().postFrameCallback(new Choreographer.FrameCallback() {
                        @Override
                        public void doFrame(long frameTimeNanos) {
                            createVulkanSurface(surface);
                        }
                    });
                } else {
                    Toast.makeText(MainActivity.this, "Surface not available", Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                // Handle surface changes if necessary
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                // Handle surface destruction if necessary
            }
        });
    }
}
