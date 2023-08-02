package com.example.chonkystation

//import org.libsdl.app.SDL
//import org.libsdl.app.SDLActivity;
import android.content.pm.ActivityInfo
import android.net.Uri
import android.os.Bundle
import android.util.DisplayMetrics
import android.util.Log
import android.view.View
import androidx.activity.result.contract.ActivityResultContracts.GetContent
import com.google.androidgamesdk.GameActivity
import java.io.IOException


class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("ChonkyStation")
        }

        /** Reads the image represented by `fd` in native layer.
         *
         *
         * For apparently no reason!
         *
         * @return Some information about the file.
         */
        external fun readFile(fd: Int): String?
        external fun startEmulator()
        external fun setDPI(xdpi: Float, ydpi: Float)
    }

    inline fun <reified T> T.logi(message: String) = Log.i(T::class.java.simpleName, message)

    var started = false;
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if(!started) {
            started = true
        } else {
            return
        }
        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        var getURI = registerForActivityResult(GetContent()) { uri: Uri? ->
            // Handle the returned Uri
            logi("A file was picked (:rpog:) (:rfloosh:)\n")
            logi(uri.toString())
            if(uri != null) {
                val context = applicationContext
                val contentResolver = context.contentResolver
                try {
                    contentResolver.openAssetFileDescriptor(uri, "r")
                        .use { assetFileDescriptor ->
                            val parcelFileDescriptor =
                                assetFileDescriptor!!.parcelFileDescriptor
                            val fd = parcelFileDescriptor.fd

                            var res = readFile(fd);
                            logi(res!!);
                            val dm = DisplayMetrics()
                            windowManager.defaultDisplay.getMetrics(dm)
                            setDPI(dm.xdpi, dm.ydpi)
                            startEmulator();
                            parcelFileDescriptor.close()
                        }
                } catch (ioException: IOException) {
                    // TODO: Handle failure scenario.
                }
            }
        }
        getURI.launch("*/*")
    }
    override fun onStartNative(p0: Long) {
    }
    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUi()
        }
    }

    private fun hideSystemUi() {
        val decorView = window.decorView
        decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN)
    }
}