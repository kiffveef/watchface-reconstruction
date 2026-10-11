#pragma once

#include <pebble.h>

typedef struct {
  GFont font;
  // ベースラインから描画矩形の上端までの距離
  int16_t ascent;
  // 字間 (1/10 px)。フォント側で字間を変えられないので1文字ずつ置いて実現する
  int16_t tracking_x10;
} TextStyle;

// anchor.y はベースライン、anchor.x は align に応じた左端・中央・右端。
// 実際に文字が占めた横範囲を返す (隣の要素の位置決め用)
GRect text_draw(GContext *ctx, const char *text, const TextStyle *style, GPoint anchor,
                GTextAlignment align, GColor color);

// 負の値は "--"、それ以外は3桁区切りで書き込む。値があれば true
bool text_format_value(char *buf, size_t len, int32_t value);

// ASCII 小文字を大文字にする (フォントは大文字のみ収録)
void text_to_upper(char *buf);
