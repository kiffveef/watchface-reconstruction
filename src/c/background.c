#include "background.h"
#include "layout.h"

static GBitmap *s_cache;

// --- Elements ---

// 12時 = 0°、時計回りの角度 (度)。atan2(dx, -dy) がそのままこの系になる
static int32_t prv_angle_deg(int16_t dx, int16_t dy) {
  int32_t angle = atan2_lookup(dx, -dy);
  if (angle < 0) {
    angle += TRIG_MAX_ANGLE;
  }
  return TRIGANGLE_TO_DEG(angle);
}

static bool prv_in_range(int32_t deg, int32_t start, int32_t end) {
  return deg >= start && deg < end;
}

// 64色に #2A2A2A が無いため市松で作る。範囲が角度で決まるのでピクセル単位で塗る
static void prv_draw_rim(GContext *ctx, GRect bounds) {
  const GPoint c = DISC_CENTER;
  const int32_t r_in2 = DISC_INNER_RADIUS * DISC_INNER_RADIUS;
  const int32_t r_out2 = DISC_RADIUS * DISC_RADIUS;

  for (int16_t y = 0; y < bounds.size.h; y++) {
    for (int16_t x = 0; x < bounds.size.w; x++) {
      int16_t dx = x - c.x;
      int16_t dy = y - c.y;
      int32_t d2 = dx * dx + dy * dy;
      if (d2 < r_in2 || d2 >= r_out2) {
        continue;
      }
      int32_t deg = prv_angle_deg(dx, dy);
      bool solid = prv_in_range(deg, RIM_SOLID_START_DEG, RIM_SOLID_END_DEG);
      bool checker = prv_in_range(deg, RIM_CHECKER_A_START_DEG, RIM_SOLID_START_DEG)
          || prv_in_range(deg, RIM_SOLID_END_DEG, RIM_CHECKER_B_END_DEG);
      if (solid || (checker && ((x ^ y) & 1) == 0)) {
        graphics_context_set_stroke_color(ctx, COLOR_GRAY);
        graphics_draw_pixel(ctx, GPoint(x, y));
      }
    }
  }
}

static void prv_draw_edge(GContext *ctx) {
  const GPoint c = DISC_CENTER;
  const int16_t r = DISC_EDGE_OUTER_RADIUS;
  graphics_context_set_fill_color(ctx, COLOR_LIGHT);
  graphics_fill_radial(ctx, GRect(c.x - r, c.y - r, 2 * r, 2 * r), GOvalScaleModeFitCircle,
                       DISC_EDGE_THICKNESS, DEG_TO_TRIGANGLE(DISC_EDGE_START_DEG),
                       DEG_TO_TRIGANGLE(DISC_EDGE_END_DEG));
}

static void prv_draw(GContext *ctx, GRect bounds) {
  // 内円とリムの暗部は地の黒のままでよい
  graphics_context_set_fill_color(ctx, COLOR_BACKGROUND);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  prv_draw_rim(ctx, bounds);
  prv_draw_edge(ctx);
}

// --- Cache ---

static void prv_store_cache(GContext *ctx, GRect bounds) {
  s_cache = gbitmap_create_blank(bounds.size, GBitmapFormat8Bit);
  if (!s_cache) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "background cache alloc failed; drawing every frame");
    return;
  }
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  if (!fb) {
    gbitmap_destroy(s_cache);
    s_cache = NULL;
    return;
  }
  for (int16_t y = 0; y < bounds.size.h; y++) {
    GBitmapDataRowInfo src = gbitmap_get_data_row_info(fb, y);
    GBitmapDataRowInfo dst = gbitmap_get_data_row_info(s_cache, y);
    memcpy(&dst.data[src.min_x], &src.data[src.min_x], src.max_x - src.min_x + 1);
  }
  graphics_release_frame_buffer(ctx, fb);
}

void background_render(GContext *ctx, GRect bounds) {
  if (s_cache) {
    graphics_draw_bitmap_in_rect(ctx, s_cache, bounds);
    return;
  }
  prv_draw(ctx, bounds);
  prv_store_cache(ctx, bounds);
}

void background_deinit(void) {
  gbitmap_destroy(s_cache);
  s_cache = NULL;
}
