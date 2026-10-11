#pragma once

#include <pebble.h>

// 静的要素の見た目を変える状態。変わったら background_invalidate() で焼き直す
typedef struct {
  bool bt_connected;
  bool battery_low;
} BackgroundState;

// 初回 (と invalidate 後) は描画してフレームバッファを GBitmap に焼き、以降はそれを貼るだけにする
void background_render(GContext *ctx, GRect bounds, const BackgroundState *state);
void background_invalidate(void);
void background_deinit(void);
