/*
 * MSP3526 LovyanGFX Display Test with LVGL v9
 * for Raspberry Pi Pico 2
 *
 * uc_disp_hvga_lvgl.ino
 *
 * Copyright (c) 2026 K. Yoshi
 * This source code is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <cstdint>
#include <FS.h>
#include <SD.h>
#include <math.h>
#include <lvgl.h>
#include "MSP3526.h"

// LVGL用の外部フォントを宣言
LV_FONT_DECLARE(barlow_regular_22);
LV_FONT_DECLARE(barlow_regular_28);

// LVGL用の画像データを宣言
LV_IMAGE_DECLARE(reticle);

// LVGL v9用の描画バッファ (DMA転送用ダブルバッファ)
#define DRAW_BUF_SIZE (480 * 320 / 4 * 2)
uint32_t draw_buf1[DRAW_BUF_SIZE / 4];
uint32_t draw_buf2[DRAW_BUF_SIZE / 4];

// DMA転送中かどうかを管理するフラグ
volatile bool dma_busy = false;

// ディスプレイのインスタンス生成 (MSP3526.hで定義されたクラス)
LGFX_Pico2_MSP3526 display;

// UIオブジェクト(ウィジェット)のポインタ保持用変数
lv_obj_t *btn_save;     // SD書き込みトリガー用ボタン
lv_obj_t *lbl_status;   // 書き込み成否表示用ラベル
lv_obj_t *lbl_sd_init;  // SDカード初期化状態表示用ラベル

/* LVGL ディスプレイフラッシュ(画面更新)コールバック関数 */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  // 前回のDMA転送がまだ走っていれば、完了を待ってライトを終了(バスを解放)する
  if (dma_busy) {
    display.waitDMA();
    display.endWrite();
    dma_busy = false;
  }

  // 更新領域の幅と高さを算出
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  // LovyanGFXの関数を使用してピクセルデータをDMAで非同期一括転送
  display.startWrite();
  display.setAddrWindow(area->x1, area->y1, w, h);
  display.writePixelsDMA((uint16_t *)px_map, w * h);

  // endWrite()を呼ばずに抜ける(呼ぶと同期処理になってしまうため)
  dma_busy = true;

  // LVGL側にフラッシュ(更新)指示が完了したことを即座に通知
  // ダブルバッファ構成のためLVGLはもう一方のバッファを使って裏で次の描画計算を開始
  lv_display_flush_ready(disp);
}

/* LVGL タッチ入力読み込みコールバック関数 */
void my_touch_read(lv_indev_t *indev, lv_indev_data_t *data) {
  int32_t touchX, touchY;

  // タッチパネルが押されているか確認し、座標を取得
  if (display.getTouch(&touchX, &touchY)) {
    data->state = LV_INDEV_STATE_PRESSED;  // 「押されている」状態を設定
    data->point.x = touchX;                // X座標を格納
    data->point.y = touchY;                // Y座標を格納
  } else {
    data->state = LV_INDEV_STATE_RELEASED;  // 「離されている」状態を設定
  }
}

/* SDカードへの書き込み処理関数 */
void writeToSD() {
  // SDカードに触る前に、走っている可能性のあるディスプレイDMAを確実に終了させる
  if (dma_busy) {
    display.waitDMA();
    display.endWrite();
    dma_busy = false;
  }

  // バス競合を防ぐためのSPIトランザクションの初期化処理
  SPI1.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
  SPI1.endTransaction();

  // SDカードの初期化確認
  if (!SD.begin(SD_CS, SPI1)) return;

  // "/test.txt" ファイルを書き込みモードで開く
  File myFile = SD.open("/test.txt", "w");
  if (myFile) {
    myFile.println("Hello, Pico 2!");
    myFile.close();

    // 書き込み成功：ステータステキストの更新 (Saved: OK!)
    lv_label_set_text(lbl_status, "Saved: OK!");
    // 文字色を白(DDEEF5)に変更
    lv_obj_set_style_text_color(lbl_status, lv_color_hex(0x8EAE8D), 0);
  } else {
    // 書き込み失敗：エラーテキストの更新 (Error: Cannot Open)
    lv_label_set_text(lbl_status, "Error: Cannot Open");
    // 文字色を赤(RED: 0xCB1136)に変更
    lv_obj_set_style_text_color(lbl_status, lv_color_hex(0xCB1136), 0);
  }
}

/* ボタンのイベントコールバック関数 */
static void btn_event_cb(lv_event_t *e) {
  // トリガーされたイベントの種類(コード)を取得
  lv_event_code_t code = lv_event_get_code(e);

  // ボタンが「押された(PRESSED)」タイミングであれば、SDカード書き込みを実行
  if (code == LV_EVENT_PRESSED) {
    writeToSD();
  }
}

/* 自作のイメージボタン */
void lv_uc_imgbtn(void) {
  LV_IMAGE_DECLARE(ucbtn_left);
  LV_IMAGE_DECLARE(ucbtn_right);
  LV_IMAGE_DECLARE(ucbtn_mid);

  static lv_style_t style_def;
  lv_style_init(&style_def);
  lv_style_set_text_color(&style_def, lv_color_white());

  static lv_style_t style_pr;
  lv_style_init(&style_pr);
  lv_style_set_image_recolor_opa(&style_pr, LV_OPA_30);
  lv_style_set_image_recolor(&style_pr, lv_color_black());

  lv_obj_t *ucbtn = lv_imgbtn_create(lv_screen_active());
  lv_imgbtn_set_src(ucbtn, LV_IMGBTN_STATE_RELEASED, &ucbtn_left, &ucbtn_mid, &ucbtn_right);
  lv_obj_add_style(ucbtn, &style_def, 0);
  lv_obj_add_style(ucbtn, &style_pr, LV_STATE_PRESSED);
  lv_obj_set_width(ucbtn, 106);
  lv_obj_align(ucbtn, LV_ALIGN_BOTTOM_MID, 0, -30);

  lv_obj_t *lbl_save = lv_label_create(ucbtn);
  lv_obj_set_style_text_font(lbl_save, &barlow_regular_22, 0);
  lv_obj_set_style_text_color(lbl_save, lv_color_hex(0x9D9CCD), 0);
  lv_label_set_text(lbl_save, "SAVE");
  lv_obj_align(lbl_save, LV_ALIGN_CENTER, 0, 0);

  lv_obj_add_event_cb(ucbtn, btn_event_cb, LV_EVENT_PRESSED, NULL);
}

void setup(void) {
  // 各種コントロールピンの入出力モード設定
  pinMode(SD_CS, OUTPUT);
  pinMode(CTP_RST, OUTPUT);

  // 静電容量式タッチパネル(CTP)のリセット処理
  digitalWrite(CTP_RST, LOW);
  delay(10);
  digitalWrite(CTP_RST, HIGH);
  delay(50);

  // SDカードのCSピンを一旦High(非アクティブ)にする
  digitalWrite(SD_CS, HIGH);

  // LovyanGFXの初期化とディスプレイの基本構成設定
  display.init();
  display.setBrightness(0);   // 起動時はバックライトを消灯しておく
  display.setRotation(1);     // 画面の向きを横向き(1)に設定
  display.setColorDepth(16);  // カラーモードを16ビット(RGB565)に設定

  // LVGLグラフィックライブラリの初期化
  lv_init();

  // ディスプレイデバイスの登録(横480 × 縦320ピクセル)
  lv_display_t *disp = lv_display_create(480, 320);
  lv_display_set_flush_cb(disp, my_disp_flush);

  // 描画モードを部分的更新(PARTIAL)に指定し、ダブルバッファ(buf_1, buf_2)を割り当て
  lv_display_set_buffers(disp, draw_buf1, draw_buf2, sizeof(draw_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  // LVGL v9用カラーフォーマット(エンディアンが反転したRGB565_SWAPPED)の設定
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565_SWAPPED);

  // 入力デバイス(タッチパネル)の登録
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);  // ポインティングデバイス(マウス・タッチ等)として設定
  lv_indev_set_read_cb(indev, my_touch_read);       // 読み込み関数を登録
  lv_indev_set_mode(indev, LV_INDEV_MODE_TIMER);    // タイマー駆動モードに設定

  // --- UIの構築 ---
  // アクティブ画面(背景)の背景色を黒(0x000000)に設定
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), 0);

  // タイトルラベルの作成
  lv_obj_t *lbl_title = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(lbl_title, &barlow_regular_28, 0);
  lv_label_set_text(lbl_title, "TOUCH PANEL TEST");
  lv_obj_set_style_text_color(lbl_title, lv_color_hex(0x9D9CCD), 0);
  lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 50);

  // SDカード初期化状態を表示するラベルの作成
  lbl_sd_init = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(lbl_sd_init, &barlow_regular_28, 0);

  // 起動時のSDカード初期化テスト
  if (!SD.begin(SD_CS, SPI1)) {
    lv_label_set_text(lbl_sd_init, "SD Init Failed!");
    lv_obj_set_style_text_color(lbl_sd_init, lv_color_hex(0xCB1136), 0);  // 赤
  } else {
    lv_label_set_text(lbl_sd_init, "SD Card Ready");
    lv_obj_set_style_text_color(lbl_sd_init, lv_color_hex(0x8EAE8D), 0);  // 緑
  }
  // 画面の上部中央(TOP_MID)から下に120ピクセルの位置に配置
  lv_obj_align(lbl_sd_init, LV_ALIGN_TOP_MID, 0, 120);
  digitalWrite(SD_CS, HIGH);  // 初期化完了後にCSピンを解放

  // SD書き込み結果表示用ラベルの作成(初期状態は空文字)
  lbl_status = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(lbl_status, &barlow_regular_28, 0);
  lv_label_set_text(lbl_status, "");
  // 画面の上部中央(TOP_MID)から下に160ピクセルの位置に配置
  lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, 160);

  // SAVEボタンを配置
  lv_uc_imgbtn();

  // SVG画像を配置
  lv_obj_t *img_reticle = lv_image_create(lv_screen_active());
  lv_image_set_src(img_reticle, &reticle);
  lv_obj_align(img_reticle, LV_ALIGN_TOP_MID, 0, 240);  // ボタンに重ねて表示

  // SVG画像をレイヤ最上位へ移動
  lv_obj_move_foreground(img_reticle);  // 現時点では不要(ただし重要な関数)

  // フェードイン効果
  for (int brightness = 0; brightness <= 255; brightness++) {
    display.setBrightness(brightness);
    delay(10);
  }
}

void loop(void) {
  // 前回の実行からの経過時間を計算し、LVGLの内部タイマー(ティック)を更新
  static uint32_t last_tick = millis();
  uint32_t current_time = millis();
  lv_tick_inc(current_time - last_tick);
  last_tick = current_time;

  // 次のタスクまでの空き時間を取得
  uint32_t time_till_next = lv_timer_handler();

  // 空き時間に応じて適切に待つ(ただしタッチの反応性を落とさないよう最大5ms程度に制限)
  if (time_till_next > 5) {
    delay(5);
  } else if (time_till_next > 0) {
    delay(time_till_next);
  }
}
