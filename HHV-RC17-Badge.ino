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
  Serial.begin(115200);
  
  #if defined(IR_SEND_PIN)
      IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
  #  if defined(IR_SEND_PIN_STRING)
      Serial.println(F("Send IR signals at pin " IR_SEND_PIN_STRING));
  #  else
      Serial.println(F("Send IR signals at pin " STR(IR_SEND_PIN)));
  #  endif
  #else
      IrSender.begin(3, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Specify send pin and enable feedback LED at default feedback LED pin
      Serial.println(F("Send IR signals at pin 3"));
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

    //send TV codes
    IrSender.sendPanasonic(0x8, 0x12, 2);
    IrSender.sendPanasonic(0x8, 0x3D, 2);
    IrSender.sendSamsung(0x707, 0x2, 2);
    IrSender.sendSamsung(0xD0D, 0xB, 2);
    IrSender.sendSamsung(0x1818, 0xB, 2);
    IrSender.sendSamsung(0x808, 0xB, 2);
    IrSender.sendSamsung(0xE0E, 0x13, 2);
    IrSender.sendSamsung(0x808, 0x1D, 2);
    IrSender.sendSamsung(0x1717, 0x2, 2);
    IrSender.sendSamsung(0xE0E, 0xC, 2);
    IrSender.sendSamsung(0xE0E, 0x14, 2);
    IrSender.sendDenon(0x1, 0x16, 2);
    IrSender.sendSharp(0x1, 0xE9, 2);
    IrSender.sendNEC(0xBD00, 0x1, 2);
    IrSender.sendSony(0x1, 0x15, 2, 12);
    IrSender.sendPanasonic(0x8, 0x12, 2);
    IrSender.sendPanasonic(0x8, 0x3D, 2);
    IrSender.sendNEC(0x38, 0x1C, 2);
    IrSender.sendNEC(0x50, 0x17, 2);
    IrSender.sendNEC(0x40, 0x12, 2);
    IrSender.sendNEC(0x4, 0x8, 2);
    IrSender.sendNEC(0x8, 0x12, 2);
    IrSender.sendNEC(0xBF00, 0xD, 2);
    IrSender.sendNEC(0xFE, 0x4, 2);
    IrSender.sendNEC(0x20, 0xB, 2);
    IrSender.sendNEC(0xA, 0x12, 2);
    IrSender.sendNEC(0x4040, 0xA, 2);
    IrSender.sendNEC(0xDF00, 0x1C, 2);
    IrSender.sendNEC(0xCB04, 0x4A, 2);
    IrSender.sendNEC(0x7F00, 0x15, 2);
    IrSender.sendNEC(0x1, 0x0, 2);
    IrSender.sendNEC(0x2E01, 0x0, 2);
    IrSender.sendNEC(0x80, 0x82, 2);
    IrSender.sendNEC(0x8, 0xC, 2);
    IrSender.sendNEC(0x40, 0x17, 2);
    IrSender.sendNEC(0x80, 0x17, 2);
    IrSender.sendNEC(0xA0, 0x1C, 2);
    IrSender.sendNEC(0x8, 0x5, 2);
    IrSender.sendNEC(0x3F80, 0x44, 2);
    IrSender.sendRC5(0x0, 0xC, 2);
    IrSender.sendRC6(0x0, 0xC, 2);
    IrSender.sendRC5(0x1, 0xC, 2);
    IrSender.sendPulseDistanceWidth(38, 9000, 4550, 500, 1800, 500, 600, 0x3B44BDC611C, 42, PROTOCOL_IS_LSB_FIRST, 100, 2);
    IrSender.sendPulseDistanceWidth(38, 4050, 4050, 500, 2000, 500, 1000, 0xAB054F, 24, PROTOCOL_IS_LSB_FIRST, 100, 2);
    IrSender.sendPulseDistanceWidth(38, 9000, 4550, 500, 1800, 500, 600, 0x3B44BDC611C, 42, PROTOCOL_IS_LSB_FIRST, 100, 2);
    IrSender.sendPulseDistanceWidth(38, 4050, 4050, 500, 2000, 500, 1000, 0xAB054F, 24, PROTOCOL_IS_LSB_FIRST, 100, 2);
    IrSender.sendPulseDistanceWidth(38, 9000, 4500, 550, 1700, 550, 550, 0x38C73DCC119, 42, PROTOCOL_IS_LSB_FIRST, 100, 2);
    //IrSender.sendJVC(0x3, 0x17, 3);
    //IrSender.sendJVC(0x3, 0x21, 2);
  }
