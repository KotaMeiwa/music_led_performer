# music_led_performer

## HW準備
- Spresense, 拡張ボードを接続する。
- 拡張ボードとLED Strapを接続する。
- ブレッドボードに光センサーと抵抗を接続する。
- 拡張ボードとブレッドボードを接続する。
![接続](./QR-code/Spresense-breadboard-led_strap.png)

## 必要なライブラリ
下記ライブラリをZIPでDownloadしてArduino IDEにセットアップする。
- [nepils](https://github.com/KotaMeiwa/nepils/archive/refs/heads/develop.zip)
- [Adafruit_NeoPixel](https://github.com/KotaMeiwa/Adafruit_NeoPixel/archive/refs/heads/master.zip)
- [SP_Audio](https://github.com/KotaMeiwa/SP_Audio/archive/refs/heads/master.zip)

## 本ライブラリのセットアップ
同様にZIPでDownloadしてArduino IDEにセットアップする。
- [music_led_performer](https://github.com/KotaMeiwa/music_led_performer/archive/refs/heads/development.zip)

セットアップしたライブラリは下記フォルダ以下に配置される。（Windowsデフォルト）

**C:/Users/< login user name >/Documents/Arduino/libraries**

## SDカードのセットアップ
SDカードに下記ファイルを配置してください。
- < SD Card Root >/BIN/MP3DEC

    [こちらからダウンロードできます。](https://github.com/sonydevworld/spresense-arduino-compatible/raw/master/Arduino15/packages/SPRESENSE/hardware/spresense/1.0.0/libraries/Audio/examples/dsp_installer/mp3_dec_installer/MP3DEC)

- < SD Card Root >/Sound.mp3

    推奨 サンプルリングレート 128Kbps, Stereo Channel, 16bit
    
## ビルド&デプロイ
3つのスケッチで構成されている。

- Camera + Audio制御

    music_led_performer/example/main_audio
  
- LED Strap制御
    
    music_led_performer/example/sub2_led_strap

- 光センサー制御
    
    music_led_performer/example/sub3_photocell

sub3_photocell, sub2_led_strap, main_audio  の順でビルド&デプロイしてください。

main_audioのメモリサイズは768 KB(default)に設定すること。

sub2_led_strap, sub3_photocellはArduino IDEが適切にメモリサイズ設定してくれます。

## その他 (QRコード、LCD、カメラを使用する場合)
sub3_photocellの代わりにsub1_qrcode (QRCODE読み取り、LCD表示) を使用します。この場合は下記ライブラリも必要になります。
- [QR_decode_for_Arduino](https://github.com/KotaMeiwa/QR_decode_for_Arduino/archive/refs/heads/main.zip)

下記3ライブラリはArduino IDE内"ライブラリマネージャー"から検索してインストールできます。要ネットワーク環境
- Adafruit_BusIO

- Adafruit_GFX_Library

- Adafruit_ILI9341

### 実行中の制御
下記3つの文字列をQRCODEにエンコードして、カメラの前にかざしてください。
- http://Run    Audio/LED Strap開始。現在位置から。
- http://Still  Audio/LED Strap停止。現在位置保持。
- http://Stop   Audio/LED Strap停止。現在位置先頭に戻る。

QRCODE画像は **music_led_performer/QR-code/QR-code2.png**   をお使いください。
