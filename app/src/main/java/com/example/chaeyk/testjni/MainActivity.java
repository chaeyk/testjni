package com.example.chaeyk.testjni;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btnCrash = (Button) findViewById(R.id.btnCrash);
        btnCrash.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                makeCrash();
            }
        });
    }

    public void nativeCrashed()
    {
        // jellybean에서 여기가 실행은 되는데 출력은 안된다.
        Log.d("testjni", "execute java code");

        // jellybean에서는 이거 수행하다가 에러가 난다.
        // W/ActivityManager: Exception thrown during pause
        //                    android.os.DeadObjectException
        new RuntimeException("crashed here").printStackTrace();
    }

    public native void makeCrash();

    static {
        System.loadLibrary("hello-jni");
    }
}
