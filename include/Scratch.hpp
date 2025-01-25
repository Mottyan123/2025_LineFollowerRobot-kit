#ifndef SCRATCH_HPP
#define SCRATCH_HPP

#include <Arduino.h>
#include <Wire.h> //I2C通信
#include "SSD1306.h" //OLED液晶
#include "Arduino_APDS9960.h" //カラーセンサー
#include "ESP32Servo.h" //サーボモーター
#include <freertos/FreeRTOS.h> //並列処理
#include <freertos/task.h> //並列処理

#define B_ON LOW
#define B_OFF HIGH
#define LED_PIN 2

/*OLED液晶*/
SSD1306 display(0x3c,21,22); //ディスプレイのI2Cアドレスを指定

/*カラーセンサー*/
int r, g, b; //赤、緑、青

/*サーボモーター*/
Servo right_servo, left_servo;
const int right_servo_pin = 18;
const int left_servo_pin = 19;
const int right_servo_set = 92;
const int left_servo_set = 92;

/*制御変数*/
int ctl_flag;

/*並列処理*/
void DisplayTask(void *pvParameters);
void MainTask(void *pvParameters);
void ControlTask(void *pvParameters);
void MelodyTask(void *pvParameters);
TaskHandle_t Display_Task = NULL;
TaskHandle_t Main_Task = NULL;
TaskHandle_t Control_Task = NULL;
TaskHandle_t Melody_Task = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void getColor(int r, int g, int b) {
    if (APDS.colorAvailable()) {
        APDS.readColor(r, g, b);
        Serial.print("r = ");
        Serial.print(r);
        Serial.print(", g = ");
        Serial.print(g);
        Serial.print(", b = ");
        Serial.println(b);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void begin_melody() {
    ledcWriteTone(2, 262);
    delay(170);
    ledcWriteTone(2, 294);
    delay(170);
    ledcWriteTone(2, 330);
    delay(170);
    ledcWriteTone(2, 349);
    delay(85);
    ledcWriteTone(2, 392);
    delay(85);
    ledcWriteTone(2, 440);
    delay(85);
    ledcWriteTone(2, 494);
    delay(85);
    ledcWriteTone(2, 523);
    delay(85);
    ledcWriteTone(2, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif