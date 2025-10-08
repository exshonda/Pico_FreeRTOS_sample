# Pico2 向けサンプルプログラム

## フォルダ
- sample1_single
  - SDK提供サンプルをシングルコア専用としたもの
- sample1_smp
  - SDK提供サンプルをマルチコアコア専用としたもの
- smp_schedule
  - SMPスケジューリングを客員するサンプル
- dis_multiple_priorities
  - configRUN_MULTIPLE_PRIORITIES を 0 とした際の挙動の確認

## 開発環境

### ハードウェア環境
- Raspberry Pi Pico 2
- Raspberry Pi Pico 2 用デバッグプルーブ
  - [オフィシャル品](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html)やPicoやPico2をもう1個購入してブレッドボードで接続等どの方法で良い．
    - [こちら](https://www.switch-science.com/products/10122?srsltid=AfmBOoqKtpkzlNBW2UZ3yTW38qtnHDGO_nH1YGhpg46dNVw3n3jM4J7s)でも確認．ブレッドボードなしで接続できる．
  - デバッグ用ファームウェアは[ここ](https://github.com/raspberrypi/debugprobe/releases)からuf2をダウンロード． 
    - デバッグプルーブとしてPICOを使うかPICO2を使うかでバイナリが異なるので注意
    
- 接続
  - UARTはUART0を使い，PIN1がTX，PIN2がRXとなる．
  
### ソフトウェア環境
- VSCode
  - Raspberry Pi Pico 拡張をインストール
- このレポジトリをクローン
  - クローン後，サブモジュール updateを実施．
  
### ビルド
- サンプルのフォルダをVSCodeで開く
  - 1回目はツールとSDKのインストールが行われるので時間を要する
  - VSCodeの右下のポップアップでインストール状況が表示されるので，表示がなくなるまで待つ
- 左のペインから Raspberry Pi Pico 拡張 を選択
- Configure CMake を選択
- Compile Project を選択

### デバッグ
- 左のペインから Raspberry Pi Pico 拡張 を選択
- Debug Projectを選択
- 実行ボタンを押して実行


## 情報
- [FreeRTOSのSMP拡張仕様](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/13-Symmetric-multiprocessing-introduction)

## サンプル詳細

### smp_schedule
- SMPスケジューリングとaffinityの確認用
  - 確認事項
    - 中優先度のタスクと同じコアでコア固定の高優先度のタスクを起動した場合の振る舞い
    - vTaskPreemptionDisableの振る舞いの確認
  - タスク
    - btask_m : MID  : コア固定なし
      - ビジーループ実行毎に実行するコアを確認
    - btask_l : LOW  : コア固定なし
    - otask_h : HIGH : コア0固定
      - 起動後にsuspend
      - resumeされると実行コアを出力して数秒動作した後にsuspend
  - コマンド
    - 'w' : btask_mからotask_hを起動
    - 'd' : vTaskPreemptionDisable をbask_l/btask_mで呼び出し
    - 'e' : vTaskPreemptionDisable をbask_l/btask_mで呼び出し
    - 's' : vTaskSuspendAll をbtask_mで呼び出し
    - 'r' : xTaskResumeAll をbtask_mで呼び出し
    - 'c' : 500msのタイマー割込みハンドラをコア1で起動
    - 'C' : 500msのタイマー割込みハンドラのキャンセル

### dis_multiple_priorities
- configRUN_MULTIPLE_PRIORITIES の確認用
  - コンフィギュレーション
    - configRUN_MULTIPLE_PRIORITIES を 0 としている．
  - タスク  
    - btask1_m : MID  : コア固定なし
      - ビジーループ実行毎に実行するコアを確認
    - btask2_m : MID  : コア固定なし
    - otask_h : HIGH : コア0固定
      - 起動後にsuspend
      - resumeされると実行コアを出力して数秒動作した後にsuspend
  - コマンド
    - 'w' : btask1_mからotask_hを起動


## 基本

- コアの識別
  - 0オリジン

- getchar()
  - 内部でブロック（ビジーループ）．
  - タイムアウト付きを使う．

- vTaskSuspendAll() の振る舞い
  - 他のコアのタスク実行も止める．
    - 他のコアの優先度が高い場合も停止させる．
    - 割込みは禁止しない
    
- configRUN_MULTIPLE_PRIORITIES
  - 0 にする場合は，vTaskPreemptionDisable は無効にする必要がある