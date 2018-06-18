#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "ClappyPark_v03.h"

#include <Wire.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

#include <Adafruit_NeoPixel.h>
#define NEO_NUM 12

SSD1306  display(0x3c, I2CSDA, I2CSCL);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEO_NUM, NEO_PIN, NEO_GRB + NEO_KHZ800);

//Use LCDC for Servo Control
// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0
// use 10 bit precission for LEDC timer
#define LEDC_TIMER_BIT  10
// use 50 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     50

// int min = 26;  // (26/1024)*20ms ≒ 0.5 ms  (-90°)
int min = 75;  // (26/1024)*20ms ≒ 0.5 ms  (-90°)
int max = 123; // (123/1024)*20ms ≒ 2.4 ms (+90°)
int n = min;
boolean srv_flg = false;

void setup_tb6612() {
  pinMode( TB_A1, OUTPUT );
  pinMode( TB_A2, OUTPUT );
  pinMode( TB_Ap, OUTPUT );
  pinMode( TB_B1, OUTPUT );
  pinMode( TB_B2, OUTPUT );
  pinMode( TB_Bp, OUTPUT );
   
  digitalWrite( TB_A1, LOW );
  digitalWrite( TB_A2, LOW );
  digitalWrite( TB_Ap, LOW );
  digitalWrite( TB_B1, LOW );
  digitalWrite( TB_B2, LOW );
  digitalWrite( TB_Bp, LOW );
}

void cmd_motorA_forward() {
  Serial.println("motorA forward");
  digitalWrite( TB_Ap, HIGH );
  digitalWrite( TB_A1, HIGH );
  digitalWrite( TB_A2, LOW );
}
void cmd_motorA_back() {
  Serial.println("motorA back");
  digitalWrite( TB_Ap, HIGH );
  digitalWrite( TB_A1, LOW );
  digitalWrite( TB_A2, HIGH );
}
void cmd_motorA_stop() {
  Serial.println("motorA stop");
  digitalWrite( TB_Ap, LOW );
}

void cmd_motorB_forward() {
  Serial.println("motorB forward");
  digitalWrite( TB_Bp, HIGH );
  digitalWrite( TB_B1, HIGH );
  digitalWrite( TB_B2, LOW );
}
void cmd_motorB_back() {
  Serial.println("motorB back");
  digitalWrite( TB_Bp, HIGH );
  digitalWrite( TB_B1, LOW );
  digitalWrite( TB_B2, HIGH );
}
void cmd_motorB_stop() {
  Serial.println("motorB stop");
  digitalWrite( TB_Bp, LOW );
}

void cmd_forward() {
  Serial.println("all forward");
  
  digitalWrite( TB_Ap, HIGH );
  digitalWrite( TB_A1, HIGH );
  digitalWrite( TB_A2, LOW );
  
  digitalWrite( TB_Bp, HIGH );
  digitalWrite( TB_B1, HIGH );
  digitalWrite( TB_B2, LOW );
}

void cmd_back() {
  Serial.println("all back");
  
  digitalWrite( TB_Ap, HIGH );
  digitalWrite( TB_A1, LOW );
  digitalWrite( TB_A2, HIGH );
  
  digitalWrite( TB_Bp, HIGH );
  digitalWrite( TB_B1, LOW );
  digitalWrite( TB_B2, HIGH );
}

void cmd_stop() {
  Serial.println("all stop");
  
  digitalWrite( TB_Ap, LOW );

  digitalWrite( TB_Bp, LOW );
}

void cmd_spin_turn() {
  Serial.println("spin turn");
  
  digitalWrite( TB_Ap, HIGH );
  digitalWrite( TB_A1, HIGH );
  digitalWrite( TB_A2, LOW );
  
  digitalWrite( TB_Bp, HIGH );
  digitalWrite( TB_B1, LOW );
  digitalWrite( TB_B2, HIGH );
}

void cmd_turn_left() {
  Serial.println("turn left");
  
  digitalWrite( TB_Ap, LOW );
  digitalWrite( TB_A1, HIGH );
  digitalWrite( TB_A2, LOW );
  
  digitalWrite( TB_Bp, HIGH );
  digitalWrite( TB_B1, HIGH );
  digitalWrite( TB_B2, LOW );
}

void cmd_turn_right() {
  Serial.println("turn right");
  
  digitalWrite( TB_Ap, HIGH );
  digitalWrite( TB_A1, HIGH );
  digitalWrite( TB_A2, LOW );
  
  digitalWrite( TB_Bp, LOW );
  digitalWrite( TB_B1, HIGH );
  digitalWrite( TB_B2, LOW );
}

void cmd_srv_on() {
  if(srv_flg == false){
    srv_flg = true;
    for(n = min; n <= max; n++){
      ledcWrite(0, n);
      delay(10);
    }
  }
}

void cmd_srv_off() {
  if(srv_flg == true){
    srv_flg = false;
    for(n = max; n >= min; n--){
      ledcWrite(0, n);
      delay(10);
    }
  }
}

//------------------------------------------
// BLE
//------------------------------------------

#define DEVICE_NAME  "CLAPPY_PARK"
#define SERVICE_UUID         "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC2_UUID  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/**************
value kind            need time param?  description
0x01  CMD_FORWARD     YES 指定した時間前進
0x02  CMD_BACK        YES 指定した時間後進
0xFF  CMD_STOP        NO  停止
0x11  CMD_SPIN_TURN   NO  超信地旋回で180度回転
0x12  CMD_TURN_LEFT   YES 指定した時間左回転
0x13  CMD_TURN_RIGHT  YES 指定した時間右回転
0x21  CMD_SRV_ON      NO  クラッピーを起こす
0x22  CMD_SRV_OFF     NO  クラッピーを倒す　
*/

#define CMD_FORWARD    0x01
#define CMD_BACK       0x02
#define CMD_STOP       0xFF
#define CMD_SPIN_TURN  0x11
#define CMD_TURN_LEFT  0x12
#define CMD_TURN_RIGHT 0x13

#define CMD_SRV_ON     0x21
#define CMD_SRV_OFF    0x22

#define TURN_TIME 0x0E74

uint8_t CMD_CURRENT = 0x00;

uint16_t EXEC_TIME = 0;

unsigned long start_time;

//Characteristic
BLECharacteristic *pCharacteristic;
BLECharacteristic *pCharacteristic2;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChara) {
      std::string value = pChara->getValue();
//      int value = pChara->getValue(); //cannot convert 'std::__cxx11::string {aka std::__cxx11::basic_string<char>}' to 'int' in initialization

      if (value.length() > 0) {
        if ((int)value[0] == CMD_FORWARD){
          if(CMD_CURRENT != CMD_FORWARD){
            CMD_CURRENT = CMD_FORWARD;
            cmd_forward();
            start_time = millis();
            EXEC_TIME = ((int)value[2] << 8) + (int)value[1];
            for(int i=0;i<NEO_NUM;i++){
              pixels.setPixelColor(i, pixels.Color(150,0,0)); // Moderately bright green color.
              delay(10);
              pixels.show(); // This sends the updated pixel color to the hardware.
            }
          }
        } else if ((int)value[0] == CMD_BACK){
          if(CMD_CURRENT != CMD_BACK){
            CMD_CURRENT = CMD_BACK;
            cmd_back();
            start_time = millis();
            EXEC_TIME = ((int)value[2] << 8) + (int)value[1];
            for(int i=0;i<NEO_NUM;i++){
              pixels.setPixelColor(i, pixels.Color(0,0,150)); // Moderately bright green color.
              delay(10);
              pixels.show(); // This sends the updated pixel color to the hardware.
            }
          }
        } else if ((int)value[0] == CMD_STOP){
          if(CMD_CURRENT != CMD_STOP){
            CMD_CURRENT = CMD_STOP;
            cmd_stop();
            EXEC_TIME = 0;
            for(int i=0;i<NEO_NUM;i++){
              pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
              delay(10);
              pixels.show(); // This sends the updated pixel color to the hardware.
            }
          }
        } else if ((int)value[0] == CMD_SPIN_TURN){
          if(CMD_CURRENT != CMD_SPIN_TURN){
            CMD_CURRENT = CMD_SPIN_TURN;
            cmd_spin_turn();
            start_time = millis();
//            EXEC_TIME = ((int)value[2] << 8) + (int)value[1];
              EXEC_TIME = TURN_TIME;
            for(int i=0;i<NEO_NUM;i++){
              pixels.setPixelColor(i, pixels.Color(150,0,0)); // Moderately bright green color.
              delay(10);
              pixels.show(); // This sends the updated pixel color to the hardware.
            }
          }
        } else if ((int)value[0] == CMD_TURN_LEFT){
          if(CMD_CURRENT != CMD_TURN_LEFT){
            CMD_CURRENT = CMD_TURN_LEFT;
            cmd_turn_left();
            start_time = millis();
            EXEC_TIME = ((int)value[2] << 8) + (int)value[1];
            for(int i=0;i<NEO_NUM;i++){
              pixels.setPixelColor(i, pixels.Color(150,0,0)); // Moderately bright green color.
              delay(10);
              pixels.show(); // This sends the updated pixel color to the hardware.
            }
          }
        } else if ((int)value[0] == CMD_TURN_RIGHT){
          if(CMD_CURRENT != CMD_TURN_RIGHT){
            CMD_CURRENT = CMD_TURN_RIGHT;
            cmd_turn_right();
            start_time = millis();
            EXEC_TIME = ((int)value[2] << 8) + (int)value[1];
            for(int i=0;i<NEO_NUM;i++){
              pixels.setPixelColor(i, pixels.Color(150,0,0)); // Moderately bright green color.
              delay(10);
              pixels.show(); // This sends the updated pixel color to the hardware.
            }
          }
        } else if ((int)value[0] == CMD_SRV_ON){
          cmd_srv_on();
          for(int i=0;i<NEO_NUM;i++){
            pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
            delay(10);
            pixels.show(); // This sends the updated pixel color to the hardware.
          }
        } else if ((int)value[0] == CMD_SRV_OFF){
          cmd_srv_off();
          for(int i=0;i<NEO_NUM;i++){
            pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
            delay(10);
            pixels.show(); // This sends the updated pixel color to the hardware.
          }

        } else {
          Serial.println("*********");
          Serial.print("New value: ");
          for (int i=0; i<value.length(); i++){
            Serial.print((int)value[i]);
          }
          Serial.println();
          Serial.println("*********");
        }
          Serial.println(EXEC_TIME);

      }
    }
};

void setup_ble() {
    // BLEデバイス名の定義
    BLEDevice::init(DEVICE_NAME);
    // BLEサーバーの定義
    BLEServer *pServer = BLEDevice::createServer();
    // BLEサービスの定義
    BLEService *pService = pServer->createService(SERVICE_UUID);
    // Characteristicの定義
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
                        );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("off");
    
    pCharacteristic2 = pService->createCharacteristic(
                        CHARACTERISTIC2_UUID,
                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                        );
    pCharacteristic2->addDescriptor(new BLE2902());
    pCharacteristic2->setValue("dmcd");

    // Start
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
    Serial.println("Waiting a client connection to notify...");
}

// Timer Interrupt setting
hw_timer_t * timer = NULL;

volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);

  /*** ここにタイマー割り込みで実行するコードを記載 ***/
  if( EXEC_TIME > 0){
    if(millis() > start_time + EXEC_TIME){
       CMD_CURRENT = CMD_STOP;
       cmd_stop();
      for(int i=0;i<NEO_NUM;i++){
        pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
        delay(10);
        pixels.show(); // This sends the updated pixel color to the hardware.
      }
       EXEC_TIME = 0;
    }
  }

  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

//------------------------------------------
// Main logics
//------------------------------------------

void setup() { 
    Serial.begin(115200);

    setup_tb6612();
    cmd_stop();
    
    setup_ble();

    // Setup timer and attach timer to a led pin
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
    ledcAttachPin(Servo_PIN1, LEDC_CHANNEL_0);

    ledcWrite(0, n);

    // Create semaphore to inform us   when the timer has fired
    timerSemaphore = xSemaphoreCreateBinary();
    // Use 1st timer of 4 (counted from zero).
    // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more info).
    timer = timerBegin(0, 80, true);
    // Attach onTimer function to our timer.
    timerAttachInterrupt(timer, &onTimer, true);
    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(timer, 1000, true);
    // Start an alarm
    timerAlarmEnable(timer);

    pixels.begin(); // This initializes the NeoPixel library.
    for(int i=0;i<NEO_NUM;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
      delay(1);
      pixels.show(); // This sends the updated pixel color to the hardware.
    }

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  
  display.setFont(ArialMT_Plain_16);
  display.drawString(20, 20, "Clappy Park");

  display.setFont(ArialMT_Plain_10);
  display.drawString(60, 50, "Setup done");
  display.display();

  Serial.println( "Setup done");
  Serial.println( "*** Welcome to Clappy Park! ***");
  Serial.println( "CMD_FORWARD    0x01 + time"); 
  Serial.println( "CMD_BACK       0x02 + time");
  Serial.println( "CMD_STOP       0xFF");
  Serial.println( "CMD_SPIN_TURN  0x11");
  Serial.println( "CMD_TURN_LEFT  0x12 + time");
  Serial.println( "CMD_TURN_RIGHT 0x13 + time");
  Serial.println( "CMD_SRV_ON     0x21");
  Serial.println( "CMD_SRV_OFF    0x22");
}

void loop() {
//  Serial.println(n);
//  ledcWrite(0, n);
//  n+=5;
//  if (n > max) n = min;
//  delay(500);
}
