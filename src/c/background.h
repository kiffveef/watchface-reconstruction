#pragma once

#include <pebble.h>

// 静的要素 (内円・リム・内円の縁)。初回だけ描画してフレームバッファを GBitmap に焼き、以降はそれを貼るだけにする
void background_render(GContext *ctx, GRect bounds);
void background_deinit(void);
