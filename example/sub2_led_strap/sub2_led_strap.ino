// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <MP.h>
#include <Adafruit_NeoPixel_Spresense.h>
#include <music_led_performer.h>

const uint16_t NUM_PIXELS = 8;
const uint16_t PIN = 0;

Adafruit_NeoPixel_Spresense* neopixel = NULL;;

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

  neopixel = new Adafruit_NeoPixel_Spresense(NUM_PIXELS, PIN);
	neopixel->begin();
}

///////////////////////////////////////
void loop()
{
  static bool on = true;

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
    neopixel->fill(Adafruit_NeoPixel::Color(8, 8, 0)); neopixel->show(), delay(500);
    neopixel->fill(Adafruit_NeoPixel::Color(8, 0, 0)); neopixel->show(), delay(500);
  }
  else{
    neopixel->fill(Adafruit_NeoPixel::Color(0, 0, 0)); neopixel->show(), delay(500);
  }
}
