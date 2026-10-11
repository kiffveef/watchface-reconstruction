#pragma once

#include <pebble.h>

// --- Colors (コード値。実機の見え方は Sunlight 補正で異なる) ---
#define COLOR_BACKGROUND GColorBlack
#define COLOR_LIME GColorSpringBud
#define COLOR_GREEN GColorMayGreen
#define COLOR_WHITE GColorWhite
#define COLOR_GRAY GColorDarkGray
#define COLOR_DATE GColorLightGray
// 元画像 TERMINAL の赤帯 (#AD2A4C) に最も近い色。オレンジは緑と補色で浮くため避けた
#define COLOR_ALERT GColorJazzberryJam

#define BATTERY_LOW_PERCENT 20

// --- V shapes (折れ線 3点) ---
#define V_SHADOW_POINTS { {15, 31}, {109, 193}, {203, 31} }
#define V_SHADOW_WIDTH 3.8f
#define V_LIME_POINTS { {6, 24}, {100, 186}, {194, 24} }
#define V_LIME_WIDTH 7.0f
#define V_INNER_POINTS { {26, 18}, {100, 146}, {174, 18} }

// --- Axis ---
#define AXIS_X 100
#define AXIS_Y_START 150
#define AXIS_Y_END 204
#define AXIS_DASH_ON 14
#define AXIS_DASH_OFF 5
#define AXIS_TICK_HALF_WIDTH 3
#define AXIS_TICK_YS { 160, 177, 194 }

// --- Short rules ---
#define RULE_LEFT_START GPoint(6, 200)
#define RULE_LEFT_END GPoint(40, 200)
#define RULE_RIGHT_START GPoint(160, 200)
#define RULE_RIGHT_END GPoint(194, 200)

#define TIME_UNDERLINE_RECT GRect(78, 92, 44, 2)

// --- Checker ---
#define CHECKER_ORIGIN GPoint(4, 196)
#define CHECKER_CELL 5
#define CHECKER_COLS 3
#define CHECKER_ROWS 2

// --- Background dots (ベイヤーディザ) ---
#define DOT_SPACING 10
#define DOT_SIZE 4
#define DOT_COLS 21
#define DOT_ROWS 24
// t = BASE + x/W*X + y/H*Y (左上が緑、右下へ黒)
#define DOT_T_BASE 0.25f
#define DOT_T_X 0.45f
#define DOT_T_Y 0.55f
#define DOT_STOPS { GColorMayGreen, GColorDarkGray, GColorBlack }
#define DOT_BAYER { {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5} }
// 文字が埋もれないよう点を打たない領域 (時刻・日付・下段データ)
#define DOT_EXCLUDE_RECTS { GRect(70, 52, 60, 48), GRect(118, 112, 78, 14), GRect(0, 204, 200, 24) }

// --- Text (anchor の y はベースライン) ---
// ASCENT はベースラインから描画矩形の上端までの距離。グリフ上の余白込みでエミュレータで合わせた値
#define TIME_ANCHOR GPoint(100, 82)
#define TIME_ASCENT 44
#define TIME_TRACKING_X10 (-20)

#define DATE_ANCHOR GPoint(100, 110)
#define DATE_ASCENT 10
#define DATE_TRACKING_X10 15
#define DATE_FORMAT "%a %d %b"

#define VALUE_ASCENT 15
#define LABEL_ASCENT 9
#define LABEL_TRACKING_X10 10

#define STEPS_ANCHOR GPoint(6, 222)
#define STEPS_LABEL "STEPS"
#define BPM_ANCHOR GPoint(194, 222)
#define BPM_LABEL "BPM"
// 値とラベルの間隔。桁数によらず左右で同じ並びに見えるよう、ラベルは値の端から置く
#define DATA_LABEL_GAP 4
