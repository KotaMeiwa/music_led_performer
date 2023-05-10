// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <MP.h>
#include "music_led_performer.h"

//Configuration
#define READ_PIN 0

//抵抗と光センサを直列つなぎ。光センサ側を＋へ。
//抵抗 1K ohm
//光センサ 明るい 0.5K ohm, 暗い 50K ohm
const int g_val_min = 1024 * 1/3;   //明るい  0.5K / (0.5K + 1K) = 1/3
const int g_val_max = 1024 * 50/51; //暗い  50K / (50K + 1K) = 50/51
const int g_val_step = (g_val_max - g_val_min)/4;
const uint8_t g_leds[4] = {LED0, LED1, LED2, LED3};
const uint8_t g_led_num = sizeof(g_leds)/sizeof(g_leds[0]);
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
  int val  = analogRead(READ_PIN) - g_val_min;

  //LED点灯数 最明るい1つ  最暗い4つ
  int level = val / g_val_step +1;

  for(int i=0; i<g_led_num; i++){
    digitalWrite(g_leds[i], i<level? HIGH: LOW);
  }

#ifdef SUBCORE
  static MSG_CMD last_msg_cmd = MSG_CMD_UNK;

  //テンポラリ実装
  //Level 3以下＝明るい ならRUN。
  MSG_CMD msg_cmd = level<=3? MSG_CMD_RUN: MSG_CMD_STILL;
  msg_cmd!=last_msg_cmd? MP.Send(MSGIDs_CMD, last_msg_cmd=msg_cmd): 0;
#endif  //SUBCORE

  Serial.printf("min=%d, max=%d, step=%d, val=%d, level=%d \n", g_val_min, g_val_max, g_val_step, val, level);
  delay(500);
}
