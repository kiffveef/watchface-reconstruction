#pragma once

#include <pebble.h>

// 分の扇 (先端が分を指す)。BT 切断中は2色とも DarkGray にする
void disc_draw_minute_fan(GContext *ctx, const struct tm *t, bool bt_connected);

// 時の鏡面 (先端が時を指す)。扇の隙間から覗かせるため、必ず扇より後に描く
void disc_draw_hour_mirror(GContext *ctx, const struct tm *t);

// 時刻・日付を載せる中心のキャップ。電池低下中は縁を警告色にする
void disc_draw_cap(GContext *ctx, bool battery_low);
