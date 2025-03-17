package reflect.mobile.reflect;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;
import java.io.*;
import java.util.Arrays;
import java.util.List;

public class FileHelper {
    private static final String TAG = "FileHelper";
    private static final List<String> EXCLUDED_DIRS = Arrays.asList("geoid_map", "images", "webkit");

    public static void copyAssetsToInternalStorage(Context context) {
        AssetManager assetManager = context.getAssets();
        try {
            copyAssetFolder(assetManager, "", context.getFilesDir().getAbsolutePath());
        } catch (IOException e) {
            Log.e(TAG, "Failed to copy assets", e);
        }
    }

    private static void copyAssetFolder(AssetManager assetManager, String assetPath, String destPath) throws IOException {
        String[] assets = assetManager.list(assetPath);
        if (assets == null || assets.length == 0) {
            if (!shouldExclude(assetPath)) {
                copyAssetFile(assetManager, assetPath, destPath);
            }
        } else {
            if (shouldExclude(assetPath)) {
                return;
            }

            File dir = new File(destPath, assetPath);
            if (!dir.exists() && !dir.mkdirs()) {
                Log.e(TAG, "Failed to create directory: " + dir.getAbsolutePath());
                return;
            }

            for (String asset : assets) {
                copyAssetFolder(assetManager, assetPath.isEmpty() ? asset : assetPath + "/" + asset, destPath);
            }
        }
    }

    private static void copyAssetFile(AssetManager assetManager, String assetPath, String destPath) {
        File file = new File(destPath, assetPath);
        /*if (file.exists()) {
            return;
        }*/
        try (InputStream is = assetManager.open(assetPath);
             OutputStream os = new FileOutputStream(file)) {
            byte[] buffer = new byte[1024];
            int length;
            while ((length = is.read(buffer)) > 0) {
                os.write(buffer, 0, length);
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to copy " + assetPath, e);
        }
    }

    private static boolean shouldExclude(String assetPath) {
        for (String excluded : EXCLUDED_DIRS) {
            if (assetPath.equals(excluded) || assetPath.startsWith(excluded + "/")) {
                return true;
            }
        }
        return false;
    }
}
