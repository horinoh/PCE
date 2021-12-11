# PCE

## ビルド
- [コンパイラ(HUC)](http://www.zeograd.com/huc_download.php)
    - 解凍して huc-3.21-win を Cドライブ直下に配置
    - C:\huc-3.21-win\bin\ を環境変数 Path に通しておく
    - 環境変数 PCE_INCLUDE=C:\huc-3.21-win\include\pce を作成しておく
    - ビルド
        ~~~
        $huc main.c
        ~~~
### VS Code
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
                    "Compile",
                    "Execute"
                ]
            }
        ]
    }
    ~~~
 - ターミナル - タスクの実行 - 上記で作成したタスク名("Compile PCE"等)を選択して実行

## 実行
- [エミュレータ(Ootake)](https://www.ouma.jp/ootake/)
    - インストール 
    - C:\Program Files (x86)\Ootake を環境変数 Path に通しておく
    - 起動
        ~~~
        $Ootake.exe XXX.pce
        ~~~