package com.example.assignment1gr26;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity2 extends AppCompatActivity {

    WebView cam;
    Button refresh, shut_up;
    boolean bol = true, bol1 = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);

        refresh = findViewById(R.id.button2);
        shut_up = findViewById(R.id.button3);

        Intent intent = getIntent();
        String text = intent.getStringExtra(MainActivity.Url_Export);

        cam = findViewById(R.id.cam_server);
        WebSettings webSettings = cam.getSettings();
        webSettings.setAllowContentAccess(true);
        webSettings.setJavaScriptEnabled(true);

        String newUA= "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.0.4) Gecko/20100101 Firefox/4.0";
        webSettings.setUserAgentString(newUA);

        cam.setWebViewClient(new WebViewClient());
        cam.loadUrl(text);

        refresh.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                cam.loadUrl(text);;
            }
        });

        //Get RealTime Database Reference
        FirebaseDatabase database = FirebaseDatabase.getInstance();
        DatabaseReference IoT_base_ref  = database.getReference();

        shut_up.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (shut_up.getText().equals("Shut Down")){
                    IoT_base_ref.child("shut_down").setValue(bol);
                }//change to true
                else if (shut_up.getText().equals("Reverse Call")){
                    IoT_base_ref.child("shut_down").setValue(bol1);
                }//change to false
            }
        });

        // Reading Data from Database
        IoT_base_ref.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                //Get all values from Database
                if (snapshot.getValue() != null){
                    for (DataSnapshot Values : snapshot.getChildren()){
                        if (Values.getKey().equals("shut_down")){
                            if (Values.getValue().equals(bol)){
                                shut_up.setText("Reverse Call");
                            }
                            else if (Values.getValue().equals(bol1)){
                                shut_up.setText("Shut Down");
                            }
                        }//Shut_Down Value
                    }//for cycle
                }//snapshot check
            }
            @Override
            public void onCancelled(@NonNull DatabaseError error) {
            }
        });
    }
}