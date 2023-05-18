// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once
#ifndef MUSIC_LED_PERFORMER_H
#define MUSIC_LED_PERFORMER_H

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

//for LCD
#define TFT_DC 9
#define TFT_CS -1
#define TFT_RST 8
#define IMG_H 160
#define IMG_V 120
#define LCD_ORIGIN_X ((320-IMG_H)/2)
#define LCD_ORIGIN_Y ((240-IMG_V)/2)

//for quirc
#define IMG_QUIRC_H IMG_H
#define IMG_QUIRC_V IMG_V

#endif  //MUSIC_LED_PERFORMER_H

