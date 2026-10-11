#include "background.h"
#include "layout.h"

#define V_POINT_COUNT 3
#define V_POLYGON_POINT_COUNT 6

typedef struct {
  float x;
  float y;
} Vec2;

static GBitmap *s_cache;
static bool s_cache_valid;

// --- Geometry ---

// libm を使わずに済ませるため Newton 法で求める (正規化にしか使わないので精度は十分)
static float prv_sqrtf(float v) {
  if (v <= 0.0f) {
    return 0.0f;
  }
  float r = v > 1.0f ? v : 1.0f;
  for (int i = 0; i < 12; i++) {
    r = 0.5f * (r + v / r);
  }
  return r;
}

static Vec2 prv_normalize(Vec2 v) {
  float len = prv_sqrtf(v.x * v.x + v.y * v.y);
  return (Vec2) { v.x / len, v.y / len };
}

static GPoint prv_round(float x, float y) {
  return GPoint((int16_t)(x + (x < 0 ? -0.5f : 0.5f)), (int16_t)(y + (y < 0 ? -0.5f : 0.5f)));
}

// 線の描画は継ぎ目が丸くなるため、マイター結合の折れ線を外形ポリゴンとして塗る
static void prv_fill_miter_polyline(GContext *ctx, const GPoint pts[V_POINT_COUNT], float width,
                                    GColor color) {
  float h = width / 2.0f;
  Vec2 d1 = prv_normalize((Vec2) { pts[1].x - pts[0].x, pts[1].y - pts[0].y });
  Vec2 d2 = prv_normalize((Vec2) { pts[2].x - pts[1].x, pts[2].y - pts[1].y });
  Vec2 n1 = { -d1.y, d1.x };
  Vec2 n2 = { -d2.y, d2.x };
  Vec2 m = prv_normalize((Vec2) { n1.x + n2.x, n1.y + n2.y });
  float miter = h / (m.x * n1.x + m.y * n1.y);

  GPoint polygon[V_POLYGON_POINT_COUNT] = {
    prv_round(pts[0].x + n1.x * h, pts[0].y + n1.y * h),
    prv_round(pts[1].x + m.x * miter, pts[1].y + m.y * miter),
    prv_round(pts[2].x + n2.x * h, pts[2].y + n2.y * h),
    prv_round(pts[2].x - n2.x * h, pts[2].y - n2.y * h),
    prv_round(pts[1].x - m.x * miter, pts[1].y - m.y * miter),
    prv_round(pts[0].x - n1.x * h, pts[0].y - n1.y * h),
  };
  GPathInfo info = { .num_points = V_POLYGON_POINT_COUNT, .points = polygon };
  GPath *path = gpath_create(&info);
  graphics_context_set_fill_color(ctx, color);
  gpath_draw_filled(ctx, path);
  gpath_destroy(path);
}

// --- Elements ---

static bool prv_dot_excluded(GPoint p) {
  static const GRect excludes[] = DOT_EXCLUDE_RECTS;
  for (size_t i = 0; i < ARRAY_LENGTH(excludes); i++) {
    if (grect_contains_point(&excludes[i], &p)) {
      return true;
    }
  }
  return false;
}

static GColor prv_dot_color(int gx, int gy, GRect bounds) {
  static const uint8_t bayer[4][4] = DOT_BAYER;
  const GColor stops[] = DOT_STOPS;
  float t = DOT_T_BASE
      + (float)(gx * DOT_SPACING) / bounds.size.w * DOT_T_X
      + (float)(gy * DOT_SPACING) / bounds.size.h * DOT_T_Y;
  t = t < 0.0f ? 0.0f : t;
  t = t > 1.0f ? 1.0f : t;

  float v = t * 2.0f;
  int i = (int)v;
  float frac = v - i;
  if (i > 1) {
    i = 1;
    frac = 1.0f;
  }
  float threshold = (bayer[gy & 3][gx & 3] + 0.5f) / 16.0f;
  return stops[frac > threshold ? i + 1 : i];
}

static void prv_draw_dots(GContext *ctx, GRect bounds) {
  for (int gy = 0; gy < DOT_ROWS; gy++) {
    for (int gx = 0; gx < DOT_COLS; gx++) {
      GPoint p = GPoint(gx * DOT_SPACING, gy * DOT_SPACING);
      if (prv_dot_excluded(p)) {
        continue;
      }
      GColor color = prv_dot_color(gx, gy, bounds);
      if (gcolor_equal(color, GColorBlack)) {
        continue;
      }
      graphics_context_set_fill_color(ctx, color);
      graphics_fill_rect(ctx, GRect(p.x, p.y, DOT_SIZE, DOT_SIZE), 0, GCornerNone);
    }
  }
}

static void prv_draw_v_shapes(GContext *ctx, bool bt_connected) {
  static const GPoint shadow[V_POINT_COUNT] = V_SHADOW_POINTS;
  static const GPoint lime[V_POINT_COUNT] = V_LIME_POINTS;
  static const GPoint inner[V_POINT_COUNT] = V_INNER_POINTS;

  prv_fill_miter_polyline(ctx, shadow, V_SHADOW_WIDTH, COLOR_GRAY);
  prv_fill_miter_polyline(ctx, lime, V_LIME_WIDTH, bt_connected ? COLOR_LIME : COLOR_GRAY);

  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, COLOR_GREEN);
  graphics_draw_line(ctx, inner[0], inner[1]);
  graphics_draw_line(ctx, inner[1], inner[2]);
}

static void prv_draw_axis(GContext *ctx, bool battery_low) {
  static const int16_t tick_ys[] = AXIS_TICK_YS;

  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, COLOR_WHITE);
  for (int16_t y = AXIS_Y_START; y < AXIS_Y_END; y += AXIS_DASH_ON + AXIS_DASH_OFF) {
    int16_t end = y + AXIS_DASH_ON - 1;
    end = end < AXIS_Y_END ? end : AXIS_Y_END;
    graphics_draw_line(ctx, GPoint(AXIS_X, y), GPoint(AXIS_X, end));
  }

  graphics_context_set_stroke_color(ctx, battery_low ? COLOR_ALERT : COLOR_WHITE);
  for (size_t i = 0; i < ARRAY_LENGTH(tick_ys); i++) {
    graphics_draw_line(ctx, GPoint(AXIS_X - AXIS_TICK_HALF_WIDTH, tick_ys[i]),
                       GPoint(AXIS_X + AXIS_TICK_HALF_WIDTH, tick_ys[i]));
  }
}

static void prv_draw_rules(GContext *ctx) {
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, COLOR_GRAY);
  graphics_draw_line(ctx, RULE_LEFT_START, RULE_LEFT_END);
  graphics_draw_line(ctx, RULE_RIGHT_START, RULE_RIGHT_END);

  graphics_context_set_fill_color(ctx, COLOR_LIME);
  graphics_fill_rect(ctx, TIME_UNDERLINE_RECT, 0, GCornerNone);
}

static void prv_draw_checker(GContext *ctx) {
  GPoint origin = CHECKER_ORIGIN;
  for (int row = 0; row < CHECKER_ROWS; row++) {
    for (int col = 0; col < CHECKER_COLS; col++) {
      bool white = (row + col) % 2 == 0;
      graphics_context_set_fill_color(ctx, white ? COLOR_WHITE : COLOR_BACKGROUND);
      graphics_fill_rect(ctx, GRect(origin.x + col * CHECKER_CELL, origin.y + row * CHECKER_CELL,
                                    CHECKER_CELL, CHECKER_CELL), 0, GCornerNone);
    }
  }
}

static void prv_draw(GContext *ctx, GRect bounds, const BackgroundState *state) {
  graphics_context_set_fill_color(ctx, COLOR_BACKGROUND);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  prv_draw_dots(ctx, bounds);
  prv_draw_v_shapes(ctx, state->bt_connected);
  prv_draw_axis(ctx, state->battery_low);
  prv_draw_rules(ctx);
  prv_draw_checker(ctx);
}

// --- Cache ---

static void prv_store_cache(GContext *ctx, GRect bounds) {
  if (!s_cache) {
    s_cache = gbitmap_create_blank(bounds.size, GBitmapFormat8Bit);
  }
  if (!s_cache) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "background cache alloc failed; drawing every frame");
    return;
  }
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  if (!fb) {
    return;
  }
  for (int16_t y = 0; y < bounds.size.h; y++) {
    GBitmapDataRowInfo src = gbitmap_get_data_row_info(fb, y);
    GBitmapDataRowInfo dst = gbitmap_get_data_row_info(s_cache, y);
    memcpy(&dst.data[src.min_x], &src.data[src.min_x], src.max_x - src.min_x + 1);
  }
  graphics_release_frame_buffer(ctx, fb);
  s_cache_valid = true;
}

void background_render(GContext *ctx, GRect bounds, const BackgroundState *state) {
  if (s_cache_valid) {
    graphics_draw_bitmap_in_rect(ctx, s_cache, bounds);
    return;
  }
  prv_draw(ctx, bounds, state);
  prv_store_cache(ctx, bounds);
}

void background_invalidate(void) {
  s_cache_valid = false;
}

void background_deinit(void) {
  gbitmap_destroy(s_cache);
  s_cache = NULL;
  s_cache_valid = false;
}
