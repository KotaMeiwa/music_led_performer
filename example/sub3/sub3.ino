// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <MP.h>
#include "music_led_performer.h"

//Configuration
#define READ_PIN 0

//抵抗と光センサを直列つなぎ。光センサ側を - へ。
//抵抗 33K ohm
//光センサ(GL5528) 明るい 10-20K ohm, 暗い 1M ohm
const int g_val_spec_min = int(1024.f * 2.f/5.3f);  //明るい  20K   / (20K + 33K) = 20/53
const int g_val_spec_max = int(1024.f * 50.f/51.f); //暗い    1000K / (1000K + 33K) = 1000/1033

const uint8_t g_leds[4] = {LED0, LED1, LED2, LED3};
const uint8_t g_led_num = sizeof(g_leds)/sizeof(g_leds[0]);

//LED on Spresense点灯パターン
// LED o x x x  明るい (スマホのFlash点灯)
// LED o o x x  普通 (室内灯on)
// LED o o o x  少し暗い (室内灯on & 光センサーを手で覆う)
// LED o o o o  暗い (室内灯off)
const int g_val_lightest = 26;  //明るい (スマホのFlash点灯)
const int g_val_light = 230;    //普通 (室内灯on)
const int g_val_dark = 450;     //少し暗い (室内灯on & 光センサーを手で覆う)
const int g_val_darkest = 610;  //暗い (室内灯off)
const int g_led_threshold[g_led_num] = {0,  //各閾値に幅を持たせる。
                    g_val_lightest + (g_val_light-g_val_lightest)*0.2f,
                    g_val_light + (g_val_dark-g_val_light)*0.8f,
                    g_val_dark + (g_val_darkest-g_val_dark)*0.2f};

const int g_val_threshold = g_led_threshold[2];  //少し暗い (室内灯on & 光センサーを手で覆う)
//const int g_val_threshold = int(float(g_val_darkest) * .9f);  //9割の暗さに達したら作動
///////////////////////////////////////
void setup() {
#ifdef SUBCORE
  MP.begin();
  MP.RecvTimeout(MP_RECV_POLLING);
#endif

  Serial.begin(115200);  

  //Initialization for LEDs on Spresense
  for(int i=0; i<g_led_num; i++){
    pinMode(g_leds[i], OUTPUT);
  }
}

///////////////////////////////////////
void loop()
{
  int val  = analogRead(READ_PIN);

  for(int i=0; i<g_led_num; i++){
    if(g_led_threshold[i]<=val)
      digitalWrite(g_leds[i], HIGH);
    else
      digitalWrite(g_leds[i], LOW);
  }

#ifdef SUBCORE
  static MSG_CMD last_msg_cmd = MSG_CMD_UNK;

  MSG_CMD msg_cmd = g_val_threshold<=val? MSG_CMD_RUN: MSG_CMD_STILL;
  msg_cmd!=last_msg_cmd? MP.Send(MSGIDs_CMD, last_msg_cmd=msg_cmd): 0;
#endif  //SUBCORE

#ifndef SUBCORE
  Serial.printf("LightValue=%d, 0| oxxx |%d| ooxx |%d| ooox |%d| oooo |1024\n", val, g_led_threshold[1], g_led_threshold[2], g_led_threshold[3]);
#endif  //SUBCORE

  delay(800);
}
