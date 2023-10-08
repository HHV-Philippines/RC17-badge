/*
Hardware Hacking Village Badge
Rootcon 17 2023

We will update this as often as we can but we will love it if you contribute.

Excuse the messy code. We have to start somewhere right?
*/

#include <Arduino.h>

#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.

#define DISABLE_CODE_FOR_RECEIVER // Disables restarting receiver after each send. 

#include <IRremote.hpp>

int Led_Red = D2; //set digital pin for red
int Led_Green = D3; //set digital pin for green
int Led_Blue = D0; //set digital pin for blue

int button = D1; //D7(gpio13)
int buttonState=0;

int randR;
int randG;
int randB;
 
byte lastButtonState = LOW;
byte ledState = LOW;

void setup() {
  pinMode(Led_Red, OUTPUT); 
  pinMode(Led_Green, OUTPUT); 
  pinMode(Led_Blue, OUTPUT); 
  randomSeed(analogRead(0));

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  //initialize button
  pinMode(button, INPUT);

  //setup serial monitor
  //Serial.begin(115200);
  
  #if defined(IR_SEND_PIN)
      IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
  #  if defined(IR_SEND_PIN_STRING)
      Serial.println(F("Send IR signals at pin " IR_SEND_PIN_STRING));
  #  else
      Serial.println(F("Send IR signals at pin " STR(IR_SEND_PIN)));
  #  endif
  #else
      IrSender.begin(3, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Specify send pin and enable feedback LED at default feedback LED pin
      //Serial.println(F("Send IR signals at pin 3"));
  #endif 
}

void loop() {
   digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
   
   randR = random(255);
   randG = random(255);
   randB = random(255);
   
   //code for breathing light
   for(int val = 250; val> 50; val--) {

      if (digitalRead(button) == HIGH) {

        SendIRSignal();
        
        //exit for loop
        break;
        
      } else {
        digitalWrite(LED_BUILTIN, LOW);
      }

      
      analogWrite (Led_Red, 0);
      analogWrite (Led_Blue, 255-val);
      analogWrite (Led_Green, 0);
      delay (15);
   }


   //code for breathing light 
   for(int val = 50; val <250; val++) {

      if (digitalRead(button) == HIGH) {

        SendIRSignal();
                
        //exit for loop
        break;
        
      } else {
        digitalWrite(LED_BUILTIN, LOW);
      }    

      analogWrite (Led_Red, 0);
      analogWrite (Led_Blue, 255-val);
      analogWrite (Led_Green, 0);
      delay (15);
   }
   

}

void SendIRSignal()
  {
    digitalWrite(LED_BUILTIN, HIGH);
    analogWrite (Led_Red, 255);
    analogWrite (Led_Blue, 0);
    analogWrite (Led_Green, 0);

    //commands copied from flipperzero
    IrSender.sendSony(0x1, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendSony(0x1, 0x15, 2, 12);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendSony(0x1, 0x15, 2, 12);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x40, 0xB, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendPanasonic(0x8, 0x3D, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x4, 0x40, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7F00, 0x15, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xDF00, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x1, 0x3, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x3, 0x1D, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x19, 0x18, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xA0BA, 0x4C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xEDAD, 0xB5, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x1, 0x40, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xB7A0, 0xE9, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x10, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x10, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x10, 0x15, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x8, 0x5, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0xDF00, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0xE0E, 0xC, 2);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x707, 0x2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x50, 0x17, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x4931, 0x63, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xAA, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x38, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x7A83, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x53, 0x17, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x1818, 0xC0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendOnkyo(0x1818, 0xC0, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x38, 0x10, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xAA, 0xC5, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x4, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x18, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x71, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x6F80, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x48, 0x0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x7B80, 0x13, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x7E80, 0x18, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x50, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x7580, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x5780, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0xB0B, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xAA, 0x1B, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x4685, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x505, 0x2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x808, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x0, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x707, 0xE6, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x71, 0x4A, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x60, 0x3, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x60, 0x0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x42, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendNEC(0x42, 0x1, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0xAD50, 0x0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0xAD50, 0x2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x50, 0x3F, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x606, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x8, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x808, 0xB, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x5583, 0xC2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x0, 0x51, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0xBD00, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x0, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x1616, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x1, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x6880, 0x49, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendOnkyo(0x286, 0x49, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7F00, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7201, 0x1E, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x586, 0xF, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7D02, 0x46, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xE084, 0x20, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x6E, 0x2, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7C85, 0x80, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xC7EA, 0x17, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x707, 0xE0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x707, 0x98, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendSamsung(0x707, 0x98, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x6D, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x6D, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x6D, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x1, 0x10, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x8, 0xD7, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendRC5(0x1, 0xC, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x4664, 0x5D, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x20, 0x52, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xBF00, 0xD, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendRC5(0x3, 0xC, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xBF00, 0x0, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x1717, 0x14, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendRC5(0x0, 0xC, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xA0, 0x5F, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x38, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println(" IrSender.sendNEC(0x38, 0x12, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2E, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2E, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2E, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2F, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2F, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSony(0x1, 0x2F, 2, 12);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xC7EA, 0x97, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x4040, 0xA, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x0, 0x1A, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x28, 0xB, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x80, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xBD00, 0x1, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendDenon(0x1, 0x16, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendDenon(0x1, 0x16, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSharp(0x1, 0xE9, 2);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x40, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendSamsung(0x3E3E, 0xC, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xF404, 0x8, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x40, 0x12, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendNEC(0x40, 0x12, 1);");
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x7F00, 0x1E, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0xA0, 0x1C, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    IrSender.sendNEC(0x80, 0x82, 1);
    //delay(50);                // waits 50 milliseconds to let the buffer clear
    //Serial.println("IrSender.sendNEC(0x80, 0x82, 1);");
    IrSender.sendPulseDistanceWidth(38, 1150, 500, 700, 1450, 700, 500, 0x356, 10, PROTOCOL_IS_LSB_FIRST, 150, 6);
    IrSender.sendPulseDistanceWidth(38, 4000, 4050, 450, 2050, 450, 1050, 0xAB054F, 24, PROTOCOL_IS_LSB_FIRST, 150, 1);
    //Serial.println("DONE");

  }
