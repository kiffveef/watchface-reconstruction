#include <pebble.h>
#include "background.h"
#include "layout.h"
#include "text.h"

#define TIME_BUF_LEN 8
#define DATE_BUF_LEN 16
// 3桁区切りの歩数 ("99,999") と "--" が収まる長さ
#define VALUE_BUF_LEN 12

static Window *s_window;
static Layer *s_background_layer;
static Layer *s_dynamic_layer;

static GFont s_font_time;
static GFont s_font_date;
static GFont s_font_value;
static GFont s_font_label;

static BackgroundState s_background_state;
static char s_time_buf[TIME_BUF_LEN];
static char s_date_buf[DATE_BUF_LEN];
// 負の値は取得不可 (`--` 表示)
static int32_t s_steps = -1;
static int32_t s_bpm = -1;

// --- Drawing ---

static void prv_background_update_proc(Layer *layer, GContext *ctx) {
  background_render(ctx, layer_get_bounds(layer), &s_background_state);
}

// 値が長いとラベルの定位置に重なるので、そのときだけ値から DATA_LABEL_GAP 離す
static void prv_draw_data(GContext *ctx, int32_t value, GPoint value_anchor, GPoint label_anchor,
                          const char *label, GTextAlignment align) {
  const TextStyle value_style = { s_font_value, VALUE_ASCENT, 0 };
  const TextStyle label_style = { s_font_label, LABEL_ASCENT, LABEL_TRACKING_X10 };

  char buf[VALUE_BUF_LEN];
  bool available = text_format_value(buf, sizeof(buf), value);
  GRect drawn = text_draw(ctx, buf, &value_style, value_anchor, align,
                          available ? COLOR_WHITE : COLOR_GRAY);

  GPoint anchor = label_anchor;
  if (align == GTextAlignmentLeft) {
    int16_t min_x = drawn.origin.x + drawn.size.w + DATA_LABEL_GAP;
    anchor.x = anchor.x > min_x ? anchor.x : min_x;
  } else {
    int16_t max_x = drawn.origin.x - DATA_LABEL_GAP;
    anchor.x = anchor.x < max_x ? anchor.x : max_x;
  }
  text_draw(ctx, label, &label_style, anchor, align, COLOR_GRAY);
}

static void prv_dynamic_update_proc(Layer *layer, GContext *ctx) {
  const TextStyle time_style = { s_font_time, TIME_ASCENT, TIME_TRACKING_X10 };
  const TextStyle date_style = { s_font_date, DATE_ASCENT, DATE_TRACKING_X10 };

  text_draw(ctx, s_time_buf, &time_style, TIME_ANCHOR, GTextAlignmentCenter, COLOR_WHITE);
  text_draw(ctx, s_date_buf, &date_style, DATE_ANCHOR, GTextAlignmentCenter, COLOR_DATE);
  prv_draw_data(ctx, s_steps, STEPS_ANCHOR, STEPS_LABEL_ANCHOR, STEPS_LABEL, GTextAlignmentLeft);
  prv_draw_data(ctx, s_bpm, BPM_ANCHOR, BPM_LABEL_ANCHOR, BPM_LABEL, GTextAlignmentRight);
}

// --- State updates ---

static void prv_update_time(const struct tm *t) {
  strftime(s_time_buf, sizeof(s_time_buf), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
}

static void prv_update_date(const struct tm *t) {
  strftime(s_date_buf, sizeof(s_date_buf), DATE_FORMAT, t);
  text_to_upper(s_date_buf);
}

static bool prv_metric_accessible(HealthMetric metric, time_t start, time_t end) {
  return health_service_metric_accessible(metric, start, end)
      & HealthServiceAccessibilityMaskAvailable;
}

static void prv_update_health(void) {
  time_t now = time(NULL);
  s_steps = prv_metric_accessible(HealthMetricStepCount, time_start_of_today(), now)
      ? health_service_sum_today(HealthMetricStepCount) : -1;
  s_bpm = prv_metric_accessible(HealthMetricHeartRateBPM, now, now)
      ? health_service_peek_current_value(HealthMetricHeartRateBPM) : -1;
  // 心拍 0 はセンサー未計測なので「値なし」として扱う
  if (s_bpm == 0) {
    s_bpm = -1;
  }
}

static bool prv_is_battery_low(BatteryChargeState charge) {
  return charge.charge_percent <= BATTERY_LOW_PERCENT;
}

// --- Handlers ---

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  prv_update_time(tick_time);
  if (units_changed & DAY_UNIT) {
    prv_update_date(tick_time);
  }
  layer_mark_dirty(s_dynamic_layer);
}

static void prv_health_handler(HealthEventType event, void *context) {
  if (event != HealthEventMovementUpdate && event != HealthEventHeartRateUpdate) {
    return;
  }
  prv_update_health();
  layer_mark_dirty(s_dynamic_layer);
}

static void prv_connection_handler(bool connected) {
  if (s_background_state.bt_connected && !connected) {
    vibes_short_pulse();
  }
  s_background_state.bt_connected = connected;
  background_invalidate();
  layer_mark_dirty(s_background_layer);
}

static void prv_battery_handler(BatteryChargeState charge) {
  bool low = prv_is_battery_low(charge);
  if (low == s_background_state.battery_low) {
    return;
  }
  s_background_state.battery_low = low;
  background_invalidate();
  layer_mark_dirty(s_background_layer);
}

// --- Window ---

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_44));
  s_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_10));
  s_font_value = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VALUE_15));
  s_font_label = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LABEL_9));

  s_background_layer = layer_create(bounds);
  layer_set_update_proc(s_background_layer, prv_background_update_proc);
  layer_add_child(window_layer, s_background_layer);

  s_dynamic_layer = layer_create(bounds);
  layer_set_update_proc(s_dynamic_layer, prv_dynamic_update_proc);
  layer_add_child(window_layer, s_dynamic_layer);
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_dynamic_layer);
  layer_destroy(s_background_layer);
  background_deinit();

  fonts_unload_custom_font(s_font_label);
  fonts_unload_custom_font(s_font_value);
  fonts_unload_custom_font(s_font_date);
  fonts_unload_custom_font(s_font_time);
}

static void prv_init(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  prv_update_time(t);
  prv_update_date(t);
  s_background_state.bt_connected = connection_service_peek_pebble_app_connection();
  s_background_state.battery_low = prv_is_battery_low(battery_state_service_peek());
  if (health_service_events_subscribe(prv_health_handler, NULL)) {
    prv_update_health();
  }

  s_window = window_create();
  window_set_background_color(s_window, COLOR_BACKGROUND);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, false);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = prv_connection_handler,
  });
  battery_state_service_subscribe(prv_battery_handler);
}

static void prv_deinit(void) {
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  tick_timer_service_unsubscribe();
  health_service_events_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
