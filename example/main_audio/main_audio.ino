// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

///////////////////////////////////////
// main part of music_led_performer
// music_led_performer consists of 3 sketch,
// one (main) is controller of audio + camera,
// other (sub1) is controller of LCD display + QR code,
// other (sub2) is controller of LED strap.
// other (sub3) is controller of Photocell
// Both sub1 and sub2 can't be used at the same time.
///////////////////////////////////////

#include <MP.h>
#include <Camera.h>
#include <SP_AudioPlayer.h>
#include "music_led_performer.h"

///////////////////////////////////
//Configuration
#define USE_AUDIO
//#define USE_CAMERA
#if defined(USE_CAMERA)
  #define USE_LCD
  #define USE_QUIRC
#endif
//#define USE_SUB_LCD_QUIRC
#define USE_SUB_LED_STRAP
#define USE_SUB_PHOTOCELL

///////////////////////////////////
//Both can't be active at the same time.   Photocell is higher prioritized.
#if defined(USE_SUB_LCD_QUIRC) && defined(USE_SUB_PHOTOCELL)
  #undef USE_SUB_LCD_QUIRC
#endif

///////////////////////////////////
//For LCD & QRCODE
#if defined(USE_LCD) || defined(USE_QUIRC)
  #include "music_led_performer_lcd_qrcode.h"
#endif

///////////////////////////////////
//For Audio
SP_AudioPlayer* theSpAudio = NULL;
const char g_fileName[] = {"Sound.mp3"};    //audio file
const bool g_audio_initial_state = false;   //if true, audio starts soon after system is ready.

// Declaration of function
static bool control_audio(MSG_CMD cmd);
static bool control_led_strap(MSG_CMD cmd);
static void CamCB(CamImage img);
static bool setup_audio();
static bool setup_camera();
static bool setup_multi_core();
static void loop_audio();
//void setup();
//void loop();

///////////////////////////////////////
static bool control_audio(MSG_CMD cmd)
{
  bool ret = false;
  
  switch(cmd){
    case MSG_CMD_RUN:
      if(theSpAudio->isPaused()){
        theSpAudio->resume();
        ret = true;
      }
      else if(theSpAudio->isStopped()){
        theSpAudio->mp3play(g_fileName, false);
        ret = true;
      }
      break;
    
    case MSG_CMD_STILL:
      if(theSpAudio->isPlaying()){
        theSpAudio->pause();
        ret = true;
      }
      break;

    case MSG_CMD_STOP:
      if(!theSpAudio->isStopped()){
        theSpAudio->stop();
        ret = true;
      }
      break;

    default:
      Serial.printf("control_audio received bad command %d \n", cmd);
      break;
  }
  
  return ret;
}

///////////////////////////////////////
static bool control_led_strap(MSG_CMD cmd)
{
  MP.Send(MSGIDs_CMD, cmd, SUB_LED_STRAP);

  return true;
}

///////////////////////////////////////
static void CamCB(CamImage img)
{
  static bool cb_on = false;

//  Serial.println("CamCB called");

//  if(!(cb_on = !cb_on))
//    return ;

  if(!img.isAvailable())
    return;

#if defined(USE_SUB_LCD_QUIRC)
  //SubにCamera画像送信
  MP.Send(MSGIDs_IMG, img.getImgBuff(), SUB_LCD_QUIRC);
#endif

#if defined(USE_LCD)
  loop_lcd((uint16_t*)img.getImgBuff());
#endif  //USE_LCD

#if defined(USE_QUIRC)
  MSG_CMD cmd = MSG_CMD_UNK;
  loop_quirc((uint16_t*)img.getImgBuff(), &cmd);
#endif  //USE_QUIRC

}

///////////////////////////////////////
static bool setup_audio()
{
  theSpAudio = new SP_AudioPlayer;
  theSpAudio->begin();
  theSpAudio->volume(-200);

  if(g_audio_initial_state){
    control_audio(MSG_CMD_RUN);
  }

  Serial.println("SP_Audio Ready!");

  return true;
}

///////////////////////////////////////
static bool setup_camera()
{
  CamErr err;
  if(err=theCamera.begin(1, CAM_VIDEO_FPS_5, 
    IMG_H, IMG_V, CAM_IMAGE_PIX_FMT_RGB565), err!=CAM_ERR_SUCCESS){
    Serial.printf("Camera bigin error!, code=%d\n", err);
    return false;
  }
  
  if(err=theCamera.startStreaming(true, CamCB), err!=CAM_ERR_SUCCESS){
    Serial.printf("Camera startStreaming error, code=%d\n", err);
    return false;
  }

  Serial.println("Camera Ready!");

  return true;
}

///////////////////////////////////////
static bool setup_multi_core()
{
  int ret;
#if defined(USE_SUB_LCD_QUIRC)
  ret = MP.begin(SUB_LCD_QUIRC);
  Serial.println(ret < 0? "subcore1 not started": "subcore1 started");
#endif

#if defined(USE_SUB_PHOTOCELL)
  ret = MP.begin(SUB_PHOTOCELL);
  Serial.println(ret < 0? "subcore3 not started": "subcore3 started");
#endif

#if defined(USE_SUB_LED_STRAP)
  ret = MP.begin(SUB_LED_STRAP);
  Serial.println(ret < 0? "subcore2 not started": "subcore2 started");
#endif

#if defined(USE_SUB_LCD_QUIRC) || defined(USE_SUB_LED_STRAP) || defined(USE_SUB_PHOTOCELL)
  MP.RecvTimeout(MP_RECV_POLLING);
#endif

  return true;
}

///////////////////////////////////////
/**
 * @brief Setup audio player to play mp3 file
 *
 * Set clock mode to normal <br>
 * Set output device to speaker <br>
 * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect" <br>
 * System directory "/mnt/sd0/BIN" will be searched for MP3 decoder (MP3DEC file)
 * Open "Sound.mp3" file <br>
 * Set master volume to -16.0 dB
 */
void setup()
{
  Serial.begin(115200);
  
#if defined(USE_LCD)
  if(!setup_lcd())
    exit(1);
#endif  //USE_LCD

#if defined(USE_QUIRC)
  if(!setup_quirc())
    exit(1);
#endif  //USE_QUIRC

#if defined(USE_CAMERA)
  if(!setup_camera())
    exit(1);
#endif  //USE_CAMERA

#if defined(USE_AUDIO)
  if(!setup_audio())
    exit(1);
#endif  //USE_AUDIO

  if(!setup_multi_core())
    exit(1);
}

///////////////////////////////////////
void loop()
{
//  puts("loop!!");

  static MSG_CMD last_received_cmd = MSG_CMD_UNK;

#if defined(USE_AUDIO)
  #if defined(USE_SUB_LCD_QUIRC) || defined(USE_SUB_PHOTOCELL)
  int8_t msgid =0; 
  uint32_t msg;
    #if defined(USE_SUB_LCD_QUIRC)
    int subid = SUB_LCD_QUIRC;
    #elif defined(USE_SUB_PHOTOCELL)
    int subid = SUB_PHOTOCELL;
    #endif

  //QRCODE/Photocellによる指示。Audioのみ制御
  if(MP.Recv(&msgid, &msg, subid)==MSGIDs_CMD){
    //Commandによる制御
    Serial.printf("cmd = %d, audio status=%d \n", msg, theSpAudio->readState());
    
    //Audio制御
    last_received_cmd = (control_audio(MSG_CMD(msg))? msg: last_received_cmd);
  }
  #endif  //USE_SUB_LCD_QUIRC || USE_SUB_PHOTOCELL

  //File-endによるStop状態 -> 先頭から再生
  if(theSpAudio->isStopped() && last_received_cmd!=MSG_CMD_STOP && last_received_cmd!=MSG_CMD_UNK){
    control_audio(MSG_CMD_RUN);
  }

  #if defined(USE_SUB_LED_STRAP)
  //LED StrapはAudioに連動
  static MSG_CMD last_sent_cmd = MSG_CMD_UNK;
  if((last_sent_cmd!=MSG_CMD_STOP) && (theSpAudio->isStopped() || theSpAudio->isPaused())){
    //LED Strap制御
    Serial.println("strap stop");
    control_led_strap(last_sent_cmd = MSG_CMD_STOP);
  }
  else if(last_sent_cmd!=MSG_CMD_RUN && theSpAudio->isPlaying()){
    Serial.println("strap run");
    control_led_strap(last_sent_cmd = MSG_CMD_RUN);
  }
  #endif  //USE_SUB_LED_STRAP
#else //USE_AUDIO
  delay(100);

#endif //USE_AUDIO
}
