package com.example.assignment1gr26;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {
    TextView Measurement, AlarmStatus;
    boolean bol = true, bol1 = false;
    Button start, Activate;
    EditText url;

    String text;
    public static final String Url_Export = "coco";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        start = findViewById(R.id.button);
        url = findViewById(R.id.URL);

        start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                OpenActivity2();
            }
        });



        //Measurement = findViewById(R.id.Measurement);
        AlarmStatus = findViewById(R.id.Alarm_Status);
        Activate = findViewById(R.id.Alarm_Button);


        //Get RealTime Database Reference
        FirebaseDatabase database = FirebaseDatabase.getInstance();
        DatabaseReference IoT_base_ref  = database.getReference();

        /*//Button On Click
        Activate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (Activate.getText().equals("ENABLE_ALARM_SOUND")){
                    Activate.setText("Enable Buzzer");
                    //IoT_base_ref.child("enable_alert_sound").setValue(bol1);
                }//When Disable Buzzer On
                else if (Activate.getText().equals("Enable Buzzer")){
                    Activate.setText("Disable Buzzer");
                    //IoT_base_ref.child("enable_alert_sound").setValue(bol);
                }//When Enable Buzzer On
            }
        });*/

        //Button On Click
        Activate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                IoT_base_ref.child("start_server").setValue(bol);
            }
        });

        // Reading Data from Database
        IoT_base_ref.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                //Get all values from Database
                if (snapshot.getValue() != null){
                    for (DataSnapshot Values : snapshot.getChildren()){
                        /*if (Values.getKey().equals("measurement")){
                            Measurement.setText(""+Values.getValue());
                        }//Measurement Value*/

                        if (Values.getKey().equals("alarm_status")){
                            if (Values.getValue().equals(bol1)){
                                AlarmStatus.setText("Nothing to Report");
                            }
                            else if (Values.getValue().equals(bol)){
                                AlarmStatus.setText("ALERT!ALERT!");
                            }
                        }//Alarm Status Value

                        /*if (Values.getKey().equals("enable_alert_sound")){
                            if (Values.getValue().equals(bol)){
                                Activate.setText("Disable Buzzer");
                            }
                            else if (Values.getValue().equals(bol1)){
                                Activate.setText("Enable Buzzer");
                            }
                        }//Button*/

                    }//for cycle
                }//snapshot check
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });


    }

    public void OpenActivity2() {
        text = url.getText().toString();
        Intent intent = new Intent(this, MainActivity2.class);
        intent.putExtra(Url_Export, text);
        startActivity(intent);
    }
}