#pragma once

#include <pebble.h>

// --- Colors (コード値。実機の見え方は Sunlight 補正で異なる) ---
#define COLOR_BACKGROUND GColorBlack
#define COLOR_LIME GColorSpringBud
#define COLOR_GREEN GColorMayGreen
#define COLOR_WHITE GColorWhite
#define COLOR_GRAY GColorDarkGray
#define COLOR_LIGHT GColorLightGray
// 元画像 TERMINAL の赤帯 (#AD2A4C) に最も近い色。オレンジは緑と補色で浮くため避けた
#define COLOR_ALERT GColorJazzberryJam

#define BATTERY_LOW_PERCENT 20

// --- Disc geometry (角度は 12時 = 0°、時計回り) ---
#define DISC_CENTER GPoint(100, 100)
#define DISC_RADIUS 106
#define DISC_INNER_RADIUS 78
#define DISC_CAP_RADIUS 51

// 内円の縁 (リム側に置く LightGray の弧)
#define DISC_EDGE_OUTER_RADIUS 80
#define DISC_EDGE_THICKNESS 2
#define DISC_EDGE_START_DEG 170
#define DISC_EDGE_END_DEG 310

// リムの明るい範囲。CHECKER は黒 + DarkGray の 2×2 市松で #2A2A2A 相当を作る
#define RIM_CHECKER_A_START_DEG 150
#define RIM_SOLID_START_DEG 214
#define RIM_SOLID_END_DEG 252
#define RIM_CHECKER_B_END_DEG 300

// --- Minute fan ---
// 開き角のうち先端からこの割合までを明るいライムにする
#define FAN_LIME_PERCENT 34
// 後縁の分光: 半径 0.74R〜R+1 に幅 1.4° の弧を 1.6° 間隔で3本 (角度は 1/10 度)
#define FAN_IRIS_INNER_RADIUS DISC_INNER_RADIUS
#define FAN_IRIS_OUTER_RADIUS (DISC_RADIUS + 1)
#define FAN_IRIS_WIDTH_X10 14
#define FAN_IRIS_STEP_X10 16

// --- Hour mirror ---
#define MIRROR_INNER_RADIUS 47
#define MIRROR_OUTER_RADIUS 76
#define MIRROR_IRIS_WIDTH_PERCENT 2

// --- Text (anchor の y はベースライン) ---
// ASCENT はベースラインから描画矩形の上端までの距離。グリフ上の余白込みでエミュレータで合わせた値
// 時刻は Saira (字幅75)。Outfit より字幅が狭いぶん、キャップ内で数字を大きくできる
// 数字の実際の塗り範囲がキャップの中心 (100, 100) に来る位置。字幅計算に含まれる余白ぶん x を 1px 右に寄せている
#define TIME_ANCHOR GPoint(101, 115)
#define TIME_ASCENT 44
#define TIME_TRACKING_X10 (-10)
// 下線の幅は時刻のフォントサイズと揃える (仕様書の 34px / 34 と同じ比率)
#define TIME_UNDERLINE_RECT GRect(78, 122, 44, 2)

// 仕様書の 8px では「6」が潰れて読めないため 11px にしている。
// 最も幅の広い日付 ("MON 04 MAY") でもキャップの縁 (半径 51) の内側に収まる上限のサイズ
#define DATE_ANCHOR GPoint(100, 136)
#define DATE_ASCENT 11
#define DATE_TRACKING_X10 10
#define DATE_FORMAT "%a %d %b"

#define VALUE_ASCENT 13
#define LABEL_ASCENT 8
#define LABEL_TRACKING_X10 10

// 歩数は「数字 STEPS」で数字の左端、心拍は「数字 BPM」でラベルの右端を固定する
#define STEPS_ANCHOR GPoint(10, 222)
#define STEPS_LABEL "STEPS"
#define BPM_ANCHOR GPoint(190, 222)
#define BPM_LABEL "BPM"
// 値とラベルの間隔。桁数によらず左右で同じ並びに見えるよう、ラベルは値の端から置く
#define DATA_LABEL_GAP 4
