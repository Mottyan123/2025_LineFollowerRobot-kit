#ifndef SCRATCH_HPP
#define SCRATCH_HPP

//#define DEBUG //この行をコメントアウトでdebugモードをOFF

#include <Arduino.h>
#include <Wire.h> //I2C通信
#include "SSD1306.h" //OLED液晶
#include "Arduino_APDS9960.h" //カラーセンサー
#include "ESP32Servo.h" //サーボモーター
#include <freertos/FreeRTOS.h> //並列処理
#include <freertos/task.h> //並列処理

/*ボタン*/
const int START_BUTTON = 17;
const int START_ON = 0;
const int START_OFF = 1;
const int STOP_BUTTON = 16;
const int STOP_ON = 0;
const int STOP_OFF = 1;

/*圧電ブザー*/
const int BUZZER_PIN = 25;

/*LED*/
const int LED_PIN = 2;

/*OLED液晶*/
SSD1306 display(0x3c,21,22); //ディスプレイのI2Cアドレスを指定

/*カラーセンサー*/
int r, g, b, ref; //赤、緑、青、反射光

/*サーボモーター*/
Servo right_servo, left_servo;
const int right_servo_pin = 18;
const int left_servo_pin = 19;
const int right_servo_set = 92; //初期停止値
const int left_servo_set = 92; //初期停止値
int speed = 50; //初期回転速度(％)

/*制御変数*/
int sta_flag = START_OFF; //ボタン判定フラグ初期化
int sto_flag = STOP_OFF; //ボタン判定フラグ初期化
int black;
int white;
int stearing;
double p_gain;
double target;

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

void getColor() { //カラーセンサー色情報取得
    if (APDS.colorAvailable()) {
        APDS.readColor(r, g, b, ref);

        #ifdef DEBUG
        Serial.print("r = ");
        Serial.print(r);
        Serial.print(", g = ");
        Serial.print(g);
        Serial.print(", b = ");
        Serial.print(b);
        Serial.print(", ref = ");
        Serial.println(ref);
        #endif
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void display_reflect() { //カラーセンサー情報表示
    display.clear();
    display.display();
    display.drawString(0,0,"ESP32");
    display.drawString(0, 20, "Line Follower");
    display.drawString(0, 40, (String)ref);
    display.drawString(80, 40, "Robot");
    display.display();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void check_startbutton() { //ボタンチェック
    int sw = digitalRead(START_BUTTON); 

    if (sw != sta_flag){
        sta_flag = sw;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void wait_startbutton(){ //ボタンを押すまで待機
    while (!sta_flag == START_ON){
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void check_stopbutton() { //ボタンチェック
    int sw = digitalRead(STOP_BUTTON); 

    if (sw != sto_flag){
        sto_flag = sw;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void wait_stopbutton(){ //ボタンを押すまで待機
    while (!sto_flag == STOP_ON){
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void reboot_stopbutton(){ //停止ボタンでリセット
    if (sto_flag == STOP_ON){
        ESP.restart();
    }
} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void motor_speed(int _speed){ //回転速度設定
    speed = _speed;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void motor_front(){ //前進
    int re_speed = map(speed, 0, 100, 0, 90);

    while (1){
        right_servo.write(90-re_speed);
        left_servo.write(90+re_speed);

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void motor_back(){ //後進
    int re_speed = map(speed, 0, 100, 0, 90);

    while (1){
        right_servo.write(90+re_speed);
        left_servo.write(90-re_speed);

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void motor_stop(){ //停止
    right_servo.write(right_servo_set);
    left_servo.write(left_servo_set);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void motor_frontR(double r){ //前進(回転数制御)、制御精度は50%速度のみ保証
    int re_speed = map(speed, 0, 100, 0, 90);

    right_servo.write(90-re_speed);
    left_servo.write(90+re_speed);
    vTaskDelay(pdMS_TO_TICKS(r*910*50/speed));
    motor_stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void motor_backR(double r){ //後進(回転数制御)、制御精度は50%速度のみ保証
    int re_speed = map(speed, 0, 100, 0, 90);

    right_servo.write(90+re_speed);
    left_servo.write(90-re_speed);
    vTaskDelay(pdMS_TO_TICKS(r*910*50/speed));
    motor_stop();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void motor_stear(int stearing){ //ステアリング角の向きに前進
    int re_speed = map(speed, 0, 100, 0, 90);

    while (1){
        right_servo.write(constrain(90-re_speed+stearing, 0, 180));
        left_servo.write(constrain(90+re_speed+stearing, 0, 180));

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void motor_stearR(int stearing, double r){ //ステアリング角の向きに前進(回転数制御)、制御精度は50%速度のみ保証
    int re_speed = map(speed, 0, 100, 0, 90);

    right_servo.write(constrain(90-re_speed+stearing, 0, 180));
    left_servo.write(constrain(90+re_speed+stearing, 0, 180));
    vTaskDelay(pdMS_TO_TICKS(r*910*50/speed));
    motor_stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void set_black(int _black){ //黒の値を設定
    black = _black;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void set_white(int _white){ //白の値を設定
    white = _white;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void set_pgain(double _p_gain){ //Pゲインを設定
    p_gain = _p_gain;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void auto_linefollowing(){ //パラメーター設定後、自動ライントレース
    int re_speed = map(speed, 0, 100, 0, 90);
    double target = (double)(black+white)/2;

    while (1){
        int stearing = (int)((target-ref)*p_gain);

        right_servo.write(constrain(90-re_speed+stearing, 0, 180));
        left_servo.write(constrain(90+re_speed+stearing, 0, 180));

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void start_linefollowing(int stearing){ //パラメータ設定後、要制御式ライントレース
    int re_speed = map(speed, 0, 100, 0, 90);

    right_servo.write(constrain(90-re_speed+stearing, 0, 180));
    left_servo.write(constrain(90+re_speed+stearing, 0, 180));

    vTaskDelay(pdMS_TO_TICKS(1));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void begin_melody() { //起動音
    ledcWriteTone(2, 740);
    delay(375);
    ledcWriteTone(2, 587);
    delay(375);
    ledcWriteTone(2, 440);
    delay(375);
    ledcWriteTone(2, 587);
    delay(375);
    ledcWriteTone(2, 659);
    delay(375);
    ledcWriteTone(2, 880);
    delay(375);
    ledcWriteTone(2, 0);
    delay(375);
    ledcWriteTone(2, 330);
    delay(375);
    ledcWriteTone(2, 659);
    delay(375);
    ledcWriteTone(2, 740);
    delay(375);
    ledcWriteTone(2, 659);
    delay(375);
    ledcWriteTone(2, 440);
    delay(375);
    ledcWriteTone(2, 587);
    delay(750);
    ledcWriteTone(2, 0);
    delay(750);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void run_melody() { //BGM
    /*intro1*/
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 294);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 330);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 349);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 370);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 392);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 440);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 494);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 587);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 784);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 784);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 784);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 784);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    /*intro2*/
    ledcWriteTone(2, 880);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 784);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 698);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 659);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 587);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 880);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 784);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 698);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 659);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 587);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 784);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 698);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 698);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 659);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 587);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 494);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 440);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 494);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    for(int i = 0; i < 2; i++){
        /*Amero*/
        ledcWriteTone(2, 330);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 262);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 330);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 524);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 494);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(1500));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 494);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(1500));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 294);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 330);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 262);
        vTaskDelay(pdMS_TO_TICKS(1500));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        /*Bmero*/
        ledcWriteTone(2, 415);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 415);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 415);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 415);
        vTaskDelay(pdMS_TO_TICKS(1166));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(666));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 349);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 349);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 349);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 294);
        vTaskDelay(pdMS_TO_TICKS(322));
        ledcWriteTone(2, 330);
        vTaskDelay(pdMS_TO_TICKS(1666));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 262);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 262);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 494);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(1166));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 494);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 587);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 494);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(1500));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 494);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 330);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 494);
        vTaskDelay(pdMS_TO_TICKS(1166));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(332));
        ledcWriteTone(2, 392);
        vTaskDelay(pdMS_TO_TICKS(166));
        ledcWriteTone(2, 440);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 494);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        ledcWriteTone(2, 523);
        vTaskDelay(pdMS_TO_TICKS(1500));
        ledcWriteTone(2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(332));
    ledcWriteTone(2, 494);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(332));
    ledcWriteTone(2, 392);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 330);
    vTaskDelay(pdMS_TO_TICKS(332));
    ledcWriteTone(2, 330);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(332));
    ledcWriteTone(2, 494);
    vTaskDelay(pdMS_TO_TICKS(1166));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 392);
    vTaskDelay(pdMS_TO_TICKS(332));
    ledcWriteTone(2, 392);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 440);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 494);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ledcWriteTone(2, 932);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 831);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 740);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 659);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 587);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 554);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 523);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 466);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 415);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 370);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 330);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 294);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 277);
    vTaskDelay(pdMS_TO_TICKS(250));
    ledcWriteTone(2, 262);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 1047);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 1047);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 1047);
    vTaskDelay(pdMS_TO_TICKS(166));
    ledcWriteTone(2, 1047);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 262);
    vTaskDelay(pdMS_TO_TICKS(500));
    ledcWriteTone(2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif