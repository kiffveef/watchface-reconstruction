#include "disc.h"
#include "layout.h"

#define BAND_COUNT 5
#define PERCENT 100
#define DEG_X10_TO_TRIGANGLE(d) ((d) * TRIG_MAX_ANGLE / 3600)

// 実行時に乱数を使うと再描画のたびに形が変わってちらつくため、形はすべて固定テーブルで決める
static const uint8_t W_MIN[60] = {
  26, 76, 29, 56, 46, 50, 60, 35, 44, 55, 78, 70, 36, 80, 70, 50, 76, 49, 61, 43,
  67, 56, 62, 49, 33, 44, 53, 26, 35, 49, 79, 65, 41, 49, 37, 57, 79, 76, 51, 67,
  58, 76, 50, 38, 64, 69, 41, 54, 34, 32, 42, 32, 70, 56, 48, 73, 52, 38, 43, 35,
};
static const uint8_t W_HOUR[12] = { 71, 61, 58, 72, 88, 45, 47, 84, 86, 81, 86, 79 };
// {開始%, 終了%, 濃淡 0=白 1=LightGray 2=DarkGray}
static const uint8_t BAND[12][BAND_COUNT][3] = {
  { {0, 19, 0}, {22, 30, 1}, {32, 39, 1}, {70, 86, 2}, {92, 100, 2} },
  { {0, 14, 0}, {16, 27, 1}, {29, 34, 1}, {74, 85, 2}, {91, 100, 2} },
  { {0, 15, 0}, {18, 28, 1}, {31, 37, 1}, {72, 85, 2}, {92, 100, 2} },
  { {0, 17, 0}, {18, 25, 1}, {27, 33, 1}, {70, 86, 2}, {91, 100, 2} },
  { {0, 15, 0}, {17, 26, 1}, {28, 36, 1}, {70, 87, 2}, {92, 100, 2} },
  { {0, 20, 0}, {22, 31, 1}, {33, 38, 1}, {75, 88, 2}, {93, 100, 2} },
  { {0, 18, 0}, {20, 30, 1}, {32, 40, 1}, {70, 83, 2}, {89, 100, 2} },
  { {0, 14, 0}, {16, 24, 1}, {26, 34, 1}, {72, 85, 2}, {90, 100, 2} },
  { {0, 17, 0}, {19, 28, 1}, {30, 37, 1}, {70, 85, 2}, {90, 100, 2} },
  { {0, 15, 0}, {17, 28, 1}, {30, 36, 1}, {73, 87, 2}, {92, 100, 2} },
  { {0, 18, 0}, {20, 29, 1}, {31, 38, 1}, {76, 87, 2}, {92, 100, 2} },
  { {0, 13, 0}, {15, 23, 1}, {25, 31, 1}, {74, 86, 2}, {91, 100, 2} },
};
// {位置%, 色 0=Cyan 1=Magenta 2=Yellow}
static const uint8_t IRIS[12][2] = {
  {19, 0}, {14, 0}, {16, 1}, {68, 1}, {68, 2}, {91, 0},
  {87, 1}, {24, 2}, {28, 2}, {15, 2}, {18, 0}, {72, 0},
};

// --- Geometry ---

// 半径 r0..r1、角度 start..end (TRIG 単位) の扇を塗る。360° をまたぐときは2回に分ける
static void prv_fill_wedge(GContext *ctx, int16_t r0, int16_t r1, int32_t start, int32_t end,
                           GColor color) {
  if (end <= start) {
    return;
  }
  int32_t span = end - start;
  start %= TRIG_MAX_ANGLE;
  end = start + span;

  const GPoint c = DISC_CENTER;
  GRect box = GRect(c.x - r1, c.y - r1, 2 * r1, 2 * r1);
  uint16_t inset = r1 - r0;
  graphics_context_set_fill_color(ctx, color);
  if (end > TRIG_MAX_ANGLE) {
    graphics_fill_radial(ctx, box, GOvalScaleModeFitCircle, inset, start, TRIG_MAX_ANGLE);
    graphics_fill_radial(ctx, box, GOvalScaleModeFitCircle, inset, 0, end - TRIG_MAX_ANGLE);
  } else {
    graphics_fill_radial(ctx, box, GOvalScaleModeFitCircle, inset, start, end);
  }
}

static int32_t prv_percent_of(int32_t angle, int32_t percent) {
  return angle * percent / PERCENT;
}

static GColor prv_iris_color(uint8_t index) {
  const GColor colors[] = { GColorCyan, GColorMagenta, GColorYellow };
  return colors[index];
}

// --- Public ---

void disc_draw_minute_fan(GContext *ctx, const struct tm *t, bool bt_connected) {
  int32_t a0 = DEG_TO_TRIGANGLE(t->tm_min * 6);
  int32_t w = DEG_TO_TRIGANGLE(W_MIN[t->tm_min]);
  int32_t split = a0 + prv_percent_of(w, FAN_LIME_PERCENT);

  prv_fill_wedge(ctx, 0, DISC_RADIUS, a0, split, bt_connected ? COLOR_LIME : COLOR_GRAY);
  prv_fill_wedge(ctx, 0, DISC_RADIUS, split, a0 + w, bt_connected ? COLOR_GREEN : COLOR_GRAY);

  // 後縁の分光
  for (int i = 0; i < 3; i++) {
    int32_t s = a0 + w + DEG_X10_TO_TRIGANGLE(i * FAN_IRIS_STEP_X10);
    int32_t e = s + DEG_X10_TO_TRIGANGLE(FAN_IRIS_WIDTH_X10);
    prv_fill_wedge(ctx, FAN_IRIS_INNER_RADIUS, FAN_IRIS_OUTER_RADIUS, s, e, prv_iris_color(i));
  }
}

void disc_draw_hour_mirror(GContext *ctx, const struct tm *t) {
  const GColor shades[] = { COLOR_WHITE, COLOR_LIGHT, COLOR_GRAY };
  int h12 = t->tm_hour % 12;
  int32_t a0 = DEG_TO_TRIGANGLE(h12 * 30) + DEG_TO_TRIGANGLE(t->tm_min) / 2;
  int32_t w = DEG_TO_TRIGANGLE(W_HOUR[h12]);

  for (int i = 0; i < BAND_COUNT; i++) {
    const uint8_t *band = BAND[h12][i];
    prv_fill_wedge(ctx, MIRROR_INNER_RADIUS, MIRROR_OUTER_RADIUS,
                   a0 + prv_percent_of(w, band[0]), a0 + prv_percent_of(w, band[1]),
                   shades[band[2]]);
  }

  int32_t iris = IRIS[h12][0];
  prv_fill_wedge(ctx, MIRROR_INNER_RADIUS, MIRROR_OUTER_RADIUS,
                 a0 + prv_percent_of(w, iris),
                 a0 + prv_percent_of(w, iris + MIRROR_IRIS_WIDTH_PERCENT),
                 prv_iris_color(IRIS[h12][1]));
}

void disc_draw_cap(GContext *ctx, bool battery_low) {
  graphics_context_set_fill_color(ctx, COLOR_BACKGROUND);
  graphics_fill_circle(ctx, DISC_CENTER, DISC_CAP_RADIUS);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, battery_low ? COLOR_ALERT : COLOR_GRAY);
  graphics_draw_circle(ctx, DISC_CENTER, DISC_CAP_RADIUS);
}
