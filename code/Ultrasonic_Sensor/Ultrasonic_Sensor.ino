/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "pitches.h"

#define LED_PIN 13
#define trig 11
#define echo 12

int alarm_count = 0;
int proximity = LOW, tx_proximity = 0;
int buzzer_enable = false;

long duration;
int distance, distance_init;

int diff = 3; //cm

uint8_t rcv_buffer[12];

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0x9E, 0xE2, 0x04, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x71, 0xC4, 0x45, 0x81, 0xFB, 0xB9, 0xBD, 0x95, 0x0C, 0x8D, 0xD5, 0x94, 0x30, 0x8C, 0xF5 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[4];
static osjob_t sendjob, checkjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL_ALARM = 20;
const unsigned TX_INTERVAL_NORMAL = 20;
const unsigned CS_INTERVAL = 2;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void do_send(osjob_t* j);
int calculateDistance();

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
          if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }

            if (LMIC.dataLen != 0 || LMIC.dataBeg != 0) {
              u1_t bPort = 0;
              Serial.println(LMIC.dataLen);
              Serial.println(LMIC.dataBeg);
              if (LMIC.txrxFlags & TXRX_PORT){
                bPort = LMIC.frame[LMIC.dataBeg - 1];
              }
              for (int i = 0 ; i < LMIC.dataLen ; i++){
                rcv_buffer[i] = LMIC.frame[LMIC.dataBeg + i];
              }
              Serial.println(rcv_buffer[0]);
//               Serial.write(rcv_buffer, LMIC.dataLen);
//               Serial.flush();
               //Serial.println("END");
            }

            // Schedule next transmission
            int interval = (proximity == LOW)?TX_INTERVAL_ALARM:TX_INTERVAL_NORMAL;
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(interval), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void check_sensor(osjob_t* j){
    distance = calculateDistance();
    Serial.println(distance);
    Serial.println("heloo");
    
    int prev_proximity = proximity; 

    Serial.println(rcv_buffer[0]);

    if (distance > (distance_init + diff) || distance < (distance_init - diff)){
      proximity = HIGH; 
    }
    else{
      proximity = LOW;
    }

    if (distance > (distance_init + diff) || distance < (distance_init - diff)){
       digitalWrite(LED_PIN, HIGH);

       Serial.println("Motion detected!");

       tx_proximity = 1;
       if ( prev_proximity == LOW )
       {
          alarm_count++;
          do_send(&sendjob);
       }
    }
    else
    {
       digitalWrite(LED_PIN, LOW); 
       Serial.println("No motion...");
       
       //do_send(&sendjob);
    }

    os_setTimedCallback(&checkjob, os_getTime()+sec2osticks(CS_INTERVAL), check_sensor);        
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        mydata[0] = tx_proximity;
        mydata[1] = alarm_count >> 8;
        mydata[2] = alarm_count & 0x00FF;
        mydata[3] = buzzer_enable;
        
        tx_proximity = 0;
        
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

int calculateDistance(){ 
  
  digitalWrite(trig, LOW); 
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trig, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance= duration*0.034/2;
  return distance; //in cm
}

void setup() {
    pinMode(trig, OUTPUT); // Sets the trigPin as an Output
    pinMode(echo, INPUT); // Sets the echoPin as an Input
    while(!Serial);
    Serial.begin(9600);
    Serial.println(F("Starting"));

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN,LOW);

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);

    rcv_buffer[0] = 0;
    check_sensor(&checkjob);

    distance_init = calculateDistance();
}

void loop() {
    os_runloop_once();
}
