# PCE

## ビルド
- [コンパイラ(HUC)](http://www.zeograd.com/huc_download.php)
    - 解凍して huc-3.21-win を Cドライブ直下に配置
    - C:\huc-3.21-win\bin\ を環境変数 Path に通しておく
    - 環境変数 PCE_INCLUDE=C:\huc-3.21-win\include\pce;.\ を作成しておく
    - ビルド
        ~~~
        $huc main.c
        ~~~

## 実行
- [エミュレータ(Ootake)](https://www.ouma.jp/ootake/)
    - インストール (ここではCドライブ直下に配置)
    - C:\Program Files (x86)\Ootake を環境変数 Path に通しておく
    - 起動
        ~~~
        $Ootake.exe XXX.pce
        ~~~
    - 初期設定のキー配置
        - 方向 = Arrow Key
        - II, I = Space, N
        - セレクト, スタート = Tab, Enter

## VS Code からのビルド、実行
 - ファイル - フォルダを開く で対象のフォルダを開く
 - ターミナル - タスクの構成 - テンプレートから tasks.json を生成 - Others - tasks.json を編集する
    ~~~
    {
        "version": "2.0.0",
        "tasks": [
            {
                "label": "Compile PCE",
                "type": "shell",
                "command": "huc main.c"
            },
            {
                "label": "Execute PCE",
                "type": "shell",
                "command": "Ootake main.pce"
            },
            {
                "label": "Compile and execute PCE",
                "type": "shell",
                "command": "echo done",
                "dependsOrder": "sequence",
                "dependsOn": [
                    "Compile PCE",
                    "Execute PCE"
                ]
            }
        ]
    }
    ~~~
 - ターミナル - タスクの実行 - 上記で作成したタスク名("Compile PCE"等)を選択して実行

<!--
## コンバートツール
### [OpenCV](https://sourceforge.net/projects/opencvlibrary/)
 - インストール (ここではCドライブ直下に配置)
 - C:\opencv\build\x64\vc15\bin を環境変数 Path に通しておく
 - Visual Studio 設定
    - 追加のインクルードディレクトリ  C:\opencv\build\include
    - 追加のライブラリディレクトリ  C:\opencv\build\x64\vc15\lib
-->

<!--
## メモ
### a
 - HuC6280 
    - 6502互換
    - 64K バンク切替えで 2M
    - 6チャンネル PSG
 - Huc6270
    - VDC (Video Display Controller)
    - BG. スプライト
    - 64K VRAM (16ビットデータバス)
 - Huc6260
    - VCE (Video Color Encoder)

### b
 - 表示モード(初期設定の場合)
    - 表示範囲 256 x 224 (32 x 28セル)
    - BG 512 x 256 (64 x 32セル)
 - パレット
    - 16 色 == (背景or透明)色 + 15色
    - BG 16 パレット
    - スプライト 16 パレット
 - カラー
    - 16 ビットの下位 9 ビット使用、各色 3 ビット
    - 0bGGGRRRBBB
    - 8 x 8 x 8 == 512 色

### VRAM
 - 0x0000 - 0x07ff BAT (Background Attribute Table)
 - 0x0800 - 0x0fff 未使用
 - 0x1000 - 0x4fff load_background
 - 0x5000 - 0x7eff 未使用
 - 0x7f00 - 0x7fff SATB (Sprite Attribute Table)

### スプライト
 - サイズ
    - 16x16, 16x32, 16x64, 32x16, 32x32, 32x64
-->