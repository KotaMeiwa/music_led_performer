// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once
#ifndef MUSIC_CAMERA_LED_LCD_QRC_H
#define MUSIC_CAMERA_LED_LCD_QRC_H

#include "Adafruit_ILI9341.h"
#include "quirc.h"

//for LCD
#define TFT_DC 9
#define TFT_CS -1
#define TFT_RST 8
#define IMG_H 160
#define IMG_V 120
#define LCD_ORIGIN_X ((320-IMG_H)/2)
#define LCD_ORIGIN_Y ((240-IMG_V)/2)
Adafruit_ILI9341* theTft = NULL;

//for quirc
#define IMG_QUIRC_H IMG_H
#define IMG_QUIRC_V IMG_V
struct quirc* qr = NULL;

//for MP
#define SUB_LCD_QUIRC 1
#define SUB_LED_STRAP 2
#define SUB_PHOTOCELL 3

enum MSGIDs{
  MSGIDs_IMG = 0,
  MSGIDs_CMD
};
enum MSG_CMD{
  MSG_CMD_RUN = 0,
  MSG_CMD_STILL,
  MSG_CMD_STOP,
  MSG_CMD_NUM = 3,
  MSG_CMD_UNK = 0xff
};

String g_cmd[MSG_CMD_NUM] = {"http://Run", "http://Still", "http://Stop"};

///////////////////////////////////////
MSG_CMD chk_quirc_data(struct quirc_data* data)
{
  MSG_CMD ret = MSG_CMD_UNK;
  for(int i=0; i<MSG_CMD_NUM; i++){
    if(g_cmd[i]==data->payload){
      ret = MSG_CMD(i);
      break;
    }
  }
  return ret;
}

///////////////////////////////////////
bool setup_lcd()
{
  theTft = new Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);
  theTft->begin();
  theTft->setRotation(3);
  
  Serial.println("LCD Ready!");
  return true;
}

///////////////////////////////////////
bool setup_quirc()
{
  if(qr = quirc_new(), !qr){
    Serial.println("can't create quirc object");
    return false;
  }  
  if(quirc_resize(qr, IMG_QUIRC_H, IMG_QUIRC_V) < 0) {
    Serial.println("Failed to allocate video memory");
    return false;
  }
  return true;
}

///////////////////////////////////////
void loop_lcd(uint16_t img565[])
{
  theTft->drawRGBBitmap(LCD_ORIGIN_X, LCD_ORIGIN_Y,
    img565,
    IMG_H, IMG_V);
}

///////////////////////////////////////
void loop_quirc(uint16_t img565[], MSG_CMD* cmd, struct quirc_code** qrcode=NULL)
{
  *cmd = MSG_CMD_UNK;

  int w, h;
  uint8_t* image = quirc_begin(qr, &w, &h);  
  if(w != IMG_QUIRC_H || h != IMG_QUIRC_V) {
    Serial.println("configration error");
    while(1); // fatal error to enter the infinite loop (stop process)
  }

  //Green scale
  for(int n = 0; n < w * h; ++n) {
    image[n] = (img565[n] & 0x7E0) >> 3; // extract g image
//    image[n] = (img565[n] & 0x7E0) >> 5; // extract g image
  }
  //GreenでもGray ScaleでもQR読み込み精度はあんまり変わらない
/*
  //Gray scale
  for(int n = 0; n < w * h; ++n) {    
    int R = (img565[n] & 0xF800) >> 8;
    int G = (img565[n] & 0x07E0) >> 3;
    int B = (img565[n] & 0x001F) << 3;
    image[n] = uint8_t(0.212671f*float(R) + 0.715160f*float(G) + 0.072169f*float(B));
  }
*/

  quirc_end(qr);
  
  static struct quirc_code code;
  static struct quirc_data data;

  //decodeするのは1つだけ
  int num_codes = quirc_count(qr);
  if(0 < num_codes){
    Serial.println("num codes: " + String(num_codes));

    quirc_decode_error_t err;
    quirc_extract(qr, 0, &code);
    err = quirc_decode(&code, &data);
    if(err){
      printf("DECODE FAILED: %s\n", quirc_strerror(err));
      //*cmd = MSG_CMD_RUN;
    }
    else{
      printf("Data: %s\n", data.payload);
      *cmd = chk_quirc_data(&data);
      qrcode? (*qrcode = &code): NULL;
    }
  }
}

#endif  //MUSIC_CAMERA_LED_LCD_QRC_H

