// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <MP.h>
#include <Adafruit_NeoPixel_Spresense.h>
#include <NeoPixelPerformer.h>
#include <music_led_performer.h>

const uint16_t NUM_PIXELS = 12;
const uint16_t PIN = 0;
typedef Adafruit_NeoPixel_Spresense NEOPIXEL_CLASS;
NEOPIXEL_CLASS neopixel(NUM_PIXELS, PIN);
CNeoPixelPerformer<NEOPIXEL_CLASS> performer(&neopixel);

const uint32_t g_color[8] = {
                    Adafruit_NeoPixel::Color(16, 0, 0), Adafruit_NeoPixel::Color(0, 16, 0),
                    Adafruit_NeoPixel::Color(0, 0, 16), Adafruit_NeoPixel::Color(16, 16, 0),
                    Adafruit_NeoPixel::Color(0, 16, 16), Adafruit_NeoPixel::Color(16, 0, 16),
                    Adafruit_NeoPixel::Color(0, 0, 0), Adafruit_NeoPixel::Color(16, 16, 16)};

///////////////////////////////////////
void setup()
{
#ifdef SUBCORE
  MP.begin();
  MP.RecvTimeout(MP_RECV_POLLING);
#endif

	Serial.begin(115200);
	delay(1000);
	Serial.println("serial console start!");

	neopixel.begin();
}

///////////////////////////////////////
void loop()
{
  static bool on = false;
  uint32_t rgb0 = g_color[0];
  uint32_t rgb1 = g_color[1];
  ulong speed = 500;

#ifdef SUBCORE
  //Mainからコマンド受信
  int8_t msgid =0; 
  uint32_t msg;
  if(MP.Recv(&msgid, &msg)==MSGIDs_CMD){
    on = (MSG_CMD(msg)==MSG_CMD_RUN);
  }
#endif

  if(on){
    //Blink
    performer.blink(speed, rgb0, rgb1);
  }
  else{
    performer.static_color(500, Adafruit_NeoPixel::Color(0, 0, 0));
  }
}
