/*************************************** 
Waveshare AlphaBot Car Run Test

CN: www.waveshare.net/wiki/AlphaBot
EN: www.waveshare.com/wiki/AlphaBot
****************************************/
#include <Arduino.h>
#include "AlphaBot.h"
#include "tones.h"
AlphaBot Car1 = AlphaBot();
#define BUZZER 13

int melody[] = {
  
  // Dart Vader theme (Imperial March) - Star wars 
  // Score available at https://musescore.com/user/202909/scores/1141521
  // The tenor saxophone part was used
  
  NOTE_A4,-4, NOTE_A4,-4, NOTE_A4,16, NOTE_A4,16, NOTE_A4,16, NOTE_A4,16, NOTE_F4,8, REST,8,
  NOTE_A4,-4, NOTE_A4,-4, NOTE_A4,16, NOTE_A4,16, NOTE_A4,16, NOTE_A4,16, NOTE_F4,8, REST,8,
  NOTE_A4,4, NOTE_A4,4, NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16,

  NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2,//4
  NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, NOTE_F5,-8, NOTE_C5,16,
  NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2,
  
  NOTE_A5,4, NOTE_A4,-8, NOTE_A4,16, NOTE_A5,4, NOTE_GS5,-8, NOTE_G5,16, //7 
  NOTE_DS5,16, NOTE_D5,16, NOTE_DS5,8, REST,8, NOTE_A4,8, NOTE_DS5,4, NOTE_D5,-8, NOTE_CS5,16,

  NOTE_C5,16, NOTE_B4,16, NOTE_C5,16, REST,8, NOTE_F4,8, NOTE_GS4,4, NOTE_F4,-8, NOTE_A4,-16,//9
  NOTE_C5,4, NOTE_A4,-8, NOTE_C5,16, NOTE_E5,2,

  NOTE_A5,4, NOTE_A4,-8, NOTE_A4,16, NOTE_A5,4, NOTE_GS5,-8, NOTE_G5,16, //7 
  NOTE_DS5,16, NOTE_D5,16, NOTE_DS5,8, REST,8, NOTE_A4,8, NOTE_DS5,4, NOTE_D5,-8, NOTE_CS5,16,

  NOTE_C5,16, NOTE_B4,16, NOTE_C5,16, REST,8, NOTE_F4,8, NOTE_GS4,4, NOTE_F4,-8, NOTE_A4,-16,//9
  NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2,
  
};
// change this to make the song slower or faster
int tempo = 120;

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;

void play_song(int note){
    int thisNote;
    thisNote=note;

  // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(BUZZER, melody[thisNote], noteDuration*0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);
    
    // stop the waveform generation before the next note.
    noTone(BUZZER);
  
}

int run_speed=150;
int turn_speed=50;
int back_speed=100;

void setup()
{
  Car1.SetSpeed(run_speed);       //Speed:0 - 255
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

char state;
int note=0;
char music_state='N';
void loop()
{  
  
  if (Serial.available()) {
    state=Serial.read();
    Serial.println(state);
  }

  if(state == 'F'){
    Car1.MotorRun(run_speed,run_speed);  //Car run forward
    delay(10);
    play_song(note);
    note=note+2;
    if (note>=notes*2){
      note=0;
    }
  }else if(state == 'L'){
    Car1.MotorRun(-turn_speed,turn_speed);       //Car left circle
    delay(10);
  }else if(state == 'R'){
    Car1.MotorRun(turn_speed,-turn_speed);       //Car turn right
    delay(10);
  }else if(state == 'B'){
    Car1.MotorRun(-back_speed,-back_speed);  //Car run backward
    delay(10);
  }else if(state == 'W'){
    if (music_state=='G'){
      play_song(note);
      note=note+2;
      if (note>=notes*2){
        note=0;
      }
    }
    Car1.Brake();
  }else if(state == 'M'){
    if (music_state=='N'){
      music_state='G'; 
    }else{
      music_state='N';
    }
    play_song(note);
    note=note+2;
    if (note>=notes*2){
      note=0;
    }
  }
}
