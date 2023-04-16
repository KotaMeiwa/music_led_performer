// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <MP.h>
#include "music_led_performer.h"

//Configuration
#define USE_LCD
#define USE_QUIRC

uint16_t g_rgb565[IMG_H * IMG_V] = {0}; //MP通信よりMainから画像を受けるBuffer

///////////////////////////////////////
void setup() {
#ifdef SUBCORE
  MP.begin();
  MP.RecvTimeout(MP_RECV_POLLING);
#endif

  Serial.begin(115200);  

#if defined(USE_LCD)
  if(!setup_lcd())
    exit(1);
#endif  //USE_LCD

#if defined(USE_QUIRC)
  if(!setup_quirc())
    exit(1);
#endif  //USE_QUIRC
}

///////////////////////////////////////
void loop(){
#ifdef SUBCORE
  int8_t msgid =0; 
  uint16_t* msg;
  //Camera画像を受け取り
  if(MP.Recv(&msgid, &msg)!=MSGIDs_IMG){
    return ;
  }

  memcpy(g_rgb565, msg, sizeof(g_rgb565));
#endif

#if defined(USE_LCD)
  //Camera画像表示
  loop_lcd(g_rgb565);
#endif  //USE_LCD

#if defined(USE_QUIRC)
  //QRコード解析
  MSG_CMD cmd = MSG_CMD_UNK;
  loop_quirc(g_rgb565, &cmd);

  #ifdef SUBCORE
  //QRコード 定義コマンド認識したらMainに送信
  cmd!=MSG_CMD_UNK? MP.Send(MSGIDs_CMD, cmd): 0;
  #endif  //SUBCORE

#endif  //USE_QUIRC
}
