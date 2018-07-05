# ESP32_camrobo

## Overview

[タミヤのカムプログラムロボット工作セット](http://www.tamiya.com/japan/products/70227/index.html) をマイコンロボット化するためのボート **ESP32Camrobo** と、それを使用した作例 **CrappyPark** についてのプロジェクトです。


## Get Started

### Arduinoでプログラミングするための準備

Arduinoでプログラムをボードに書き込むために以下の環境設定を行う。  


#### ESP32 ボードライブラリ

ESP32のBLE機能を使用するので、ESP32のボードライブラリを追加する。  

[arduino-esp32](https://github.com/espressif/arduino-esp32)  
**Installation Instructions** を参照してインストールする。  

参考) [Arduino IDEでESP32 BLEライブラリを導入](https://qiita.com/tomorrow56/items/afa06e206eec9fafcc7a)


#### ライブラリ

Arduino の **ライブラリマネージャ** から以下のライブラリをインストールする。  

- [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)


### カムプログラムロボットの準備

1. 以下の商品を購入します。
    - [カムプログラムロボット工作セット](http://www.tamiya.com/japan/products/70227/index.html)
    - ユニバーサルプレートとプッシュリベット(以下の組み合わせのうちどちらか)
        - [ユニバーサルプレート](http://www.tamiya.com/japan/products/70098/index.html) & [3mmプッシュリベットセット](http://www.tamiya.com/japan/products/70155/index.html)
        - [ユニバーサルプレート （2枚セット）](http://www.tamiya.com/japan/products/70157/index.html)
    - [電池ボックス　単３×４本　リード線・背中合わせ BH-343-3A](http://akizukidenshi.com/catalog/g/gP-02678/)
2. 3Dプリンタでパーツを出力します。
    - 電池ボックスホルダー
    - クラッピーマウンターA
    - クラッピーマウンターB
    - スマートフォンマウンターベース
    - iPhoneXマウンター
3. カムプログラムロボットを一旦組み立てます。
4. カムプログラムロボットにユニバーサルプレートを固定します。


## Step.1 リモコンロボット化

カムプログラムロボットにESP32Camroboを組み込み、スマホから操作可能なリモコンロボットに改造します。

1. ESP32Camrobo基盤のR19を半田付けでブリッジしてショートさせます。
2. ユニバーサルプレートの中央にESP32Camrobo基盤を固定します。
    - アンテナ側が前方に来るようにします。
    - プッシュリベットを使用して固定します。
3. ESP32Camrobo基盤に配線します。
    - カムプログラムロボットのモーターを `MotorLeft`, `MotorRight` ターミナルに接続します。
    - 電池ボックスを `+5V`, `GND` ピンに接続します。
        - 適宜、QIコネクタ等を使用してください。
        - 間に、カムプログラムロボットに付属のスイッチを挟むと操作がしやすくなります。
4. 電池ボックスホルダーをカムプログラムロボットに固定し、電池ボックスを格納します。
5. ESP32Camrobo基盤にArduinoでプログラムを書き込みます。
6. リモコンのアプリを開き、接続と操作を試します。
    - https://crappypark.uzukiaoba.net/
    - [Web Bluetooth](https://webbluetoothcg.github.io/web-bluetooth/)が使用可能な端末で利用できます。
        - Chromeがインストールされた Android ５.0以上のAndroid端末
        - Chromeがインストールされた Macbook, Macbook Pro
        - Chromeがインストールされ、なおかつBLEをサポートする Windows PC


## Step.2 クラッピーを搭載

TBD


## Step.3 AIロボット化

TBD

