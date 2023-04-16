// SPDX-FileCopyrightText: 2023 Kota Meiwa
// SPDX-License-Identifier: LGPL-2.1-or-later

///////////////////////////////////////
// main part of music_led_performer
// music_led_performer consists of 3 sketch,
// one (main) is controller of audio + camera,
// other (sub1) is controller of LCD display + QR code,
// other (sub2) is controller of LED strap.
///////////////////////////////////////

#include <MP.h>
#include <SDHCI.h>
#include <Audio.h>
#include <Camera.h>
#include "music_led_performer.h"

//Configuration
#define USE_AUDIO
#define USE_CAMERA
#if defined(USE_CAMERA)
//  #define USE_LCD
//  #define USE_QUIRC
#endif
#define USE_SUB_LCD_QUIRC
#define USE_SUB_LED_STRAP

//For Audio
#if defined(USE_AUDIO)
  SDClass theSD;
  AudioClass *theAudio = NULL;
  File myFile;
  bool ErrEnd = false;
#endif

// Declaration of function
static void audio_attention_cb(const ErrorAttentionParam *atprm);
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
/**
 * @brief Audio attention callback
 *
 * When audio internal error occurs, this function will be called back.
 */
static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

///////////////////////////////////////
static bool control_audio(MSG_CMD cmd)
{
  bool ret = true;
  
  switch(cmd){
    case MSG_CMD_RUN:
      theAudio->writeFrames(AudioClass::Player0, myFile);
      theAudio->startPlayer(AudioClass::Player0);
    case MSG_CMD_STILL:
      theAudio->stopPlayer(AudioClass::Player0);
      break;

    case MSG_CMD_STOP:
      theAudio->stopPlayer(AudioClass::Player0);
      myFile.seek(0);
      break;

    default:
      Serial.printf("control_audio received bad command %d \n", cmd);
      ret = false;
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

  Serial.println("CamCB called");

  if(!(cb_on = !cb_on))
    return ;

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
  /* Initialize SD */
  while (!theSD.begin()){
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
  }

  // start audio system
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);

  /* Set output device to speaker with first argument.
   * If you want to change the output device to I2S,
   * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
   * Set speaker driver mode to LineOut with second argument.
   * If you want to change the speaker driver mode to other,
   * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
   * as an argument.
   */
  err_t err = theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
  if(err != AUDIOLIB_ECODE_OK){
      printf("setPlayerMode() 1 error %d\n", err);
      exit(1);
  }

//  err = theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, 96*1024, 4);
  if(err != AUDIOLIB_ECODE_OK){
      printf("setPlayerMode() 2 error %d\n", err);
      exit(1);
  }

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if(err != AUDIOLIB_ECODE_OK){
      printf("Player0 initialize error\n");
      exit(1);
  }

  /* Open file placed on SD card */
  myFile = theSD.open("Sound.mp3");

  /* Verify file open */
  if(!myFile){
      printf("File open error\n");
      exit(1);
  }
  printf("Open! 0x%08lx\n", (uint32_t)myFile);

  //Send first frames to be decoded
  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND)){
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
  }

  puts("Play!");

  /* Main volume set to -16.0 dB */
  theAudio->setVolume(-160);
  theAudio->startPlayer(AudioClass::Player0);  

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

#if defined(USE_SUB_LED_STRAP)
  ret = MP.begin(SUB_LED_STRAP);
  Serial.println(ret < 0? "subcore2 not started": "subcore2 started");
#endif

#if defined(USE_SUB_LCD_QUIRC) || defined(USE_SUB_LED_STRAP) 
  MP.RecvTimeout(MP_RECV_POLLING);
#endif

  return true;
}

///////////////////////////////////////
static void loop_audio()
{
  /* Send new frames to decode in a loop until file ends */
  int err = theAudio->writeFrames(AudioClass::Player0, myFile);

  /*  Tell when player file ends */
  if (err == AUDIOLIB_ECODE_FILEEND){
      printf("Main player File End!\n");
  }

  /* Show error code from player and stop */
  if (err){
      printf("Main player error code: %d\n", err);
      goto stop_player;
  }

  if (ErrEnd){
      printf("Error End\n");
      goto stop_player;
  }

  /* This sleep is adjusted by the time to read the audio stream file.
   * Please adjust in according with the processing contents
   * being processed at the same time by Application.
   *
   * The usleep() function suspends execution of the calling thread for usec
   * microseconds. But the timer resolution depends on the OS system tick time
   * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
   * it will sleep for a longer time than the time requested here.
   */

  usleep(40000);

  /* Don't go further and continue play */
  return;

stop_player:
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  exit(1);
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

#if defined(USE_SUB_LCD_QUIRC)
  int8_t msgid =0; 
  uint32_t msg;
  if(MP.Recv(&msgid, &msg, SUB_LCD_QUIRC)==MSGIDs_CMD){
    //Commandによる制御
    Serial.printf("cmd = %d \n", msg);
    
    //Audio制御
    control_audio(MSG_CMD(msg));

#if defined(USE_SUB_LED_STRAP)
    //LED Strap制御
    control_led_strap(MSG_CMD(msg));
#endif  //USE_SUB_LED_STRAP

  }
#endif  //USE_SUB_LCD_QUIRC

  //Send new frames to decode in a loop until file ends

#if defined(USE_AUDIO)
  loop_audio();

#else
  delay(100);

#endif //USE_AUDIO
}
