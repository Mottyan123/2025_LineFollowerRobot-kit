/*2025年度　新入生プログラミング体験*/
#include "Scratch.hpp" 
void MainTask(void *pvParameters) {
  while (1) {
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*はじめ(ここから下の行にコードを書くよ！！！)*/
    getColor(r, g, b);

    if (ctl_flag == B_ON) {
      right_servo.write(0);
      left_servo.write(left_servo_set);

      vTaskDelay(pdMS_TO_TICKS(2000));

      right_servo.write(right_servo_set);
      left_servo.write(0);

      vTaskDelay(pdMS_TO_TICKS(2000));
    }

    /*おわり*/
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    vTaskDelay(pdMS_TO_TICKS(1)); //delay(1)
  }
}
















































































/*以下はバックグラウンド処理*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisplayTask(void *pvParameters) {
  while (1) {

    vTaskDelay(pdMS_TO_TICKS(1)); //delay(1)
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MelodyTask(void *pvParameters) {
  while (1) {

    vTaskDelay(pdMS_TO_TICKS(1)); //delay(1)
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ControlTask(void *pvParameters) {
  while (1) {
    int sw = digitalRead(17);

    if (sw != ctl_flag){
      ctl_flag = sw;
    }

    vTaskDelay(pdMS_TO_TICKS(1)); //delay(1)
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void setup() {
  Serial.begin(115200); //シリアル通信開始(115200bps)

  display.init(); //ディスプレイの初期化
  display.setFont(ArialMT_Plain_16); //フォント設定
  display.drawString(0,0,"program reset...");
  display.display(); //ディスプレイ反映

  if (!APDS.begin()) { //カラーセンサーの初期化
    Serial.println("カラーセンサーが初期化できませんでした.");
  }

  right_servo.attach(right_servo_pin, 500, 2400); //サーボモーターの初期化
  left_servo.attach(left_servo_pin, 500, 2400);  //サーボモーターの初期化
  right_servo.write(right_servo_set); //サーボモーターの初期停止値を設定
  left_servo.write(left_servo_set); //サーボモーターの初期停止値を設定

  pinMode(LED_PIN, OUTPUT); //LED用ピン
  digitalWrite(LED_PIN, LOW); //LEDを消灯

  pinMode(17, INPUT_PULLUP); //スイッチ用ピン
  ctl_flag = B_OFF; //ボタン判定フラグ初期化

  pinMode(25, OUTPUT); //圧電スピーカー用ピン
  ledcSetup(2, 12000, 8); //サンプリング周波数、解像度を設定
  ledcAttachPin(25, 2); //PWMチャンネル2に設定

  delay(1000); //初期化待機時間

  begin_melody(); //起動音
  display.clear(); //ディスプレイ削除
  display.display();
  display.drawString(0,0,"ESP32");
  display.drawString(0, 20, "Line Follower");
  display.drawString(80, 40, "Robot");
  display.display();

  xTaskCreate(MainTask, "Main", 4096, NULL, 3, &Main_Task);
  xTaskCreate(DisplayTask, "Display", 4096, NULL, 3, &Display_Task);
  xTaskCreate(ControlTask, "Control", 4096, NULL, 3, &Control_Task);
  xTaskCreate(MelodyTask, "Melody", 4096, NULL, 3, &Melody_Task);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  //freeRTOSの並列処理
}