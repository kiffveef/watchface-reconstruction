#include "text.h"

// 1文字の幅を測るための仮の枠。最大のグリフ (時刻 44px) が収まれば十分
#define GLYPH_MEASURE_BOX GRect(0, 0, 100, 100)
#define VALUE_TMP_LEN 16

static int16_t prv_glyph_width(char c, GFont font) {
  char glyph[2] = { c, '\0' };
  return graphics_text_layout_get_content_size(glyph, font, GLYPH_MEASURE_BOX,
      GTextOverflowModeWordWrap, GTextAlignmentLeft).w;
}

GRect text_draw(GContext *ctx, const char *text, const TextStyle *style, GPoint anchor,
                GTextAlignment align, GColor color) {
  size_t count = strlen(text);
  int32_t total_x10 = 0;
  for (size_t i = 0; i < count; i++) {
    total_x10 += prv_glyph_width(text[i], style->font) * 10;
  }
  if (count > 1) {
    total_x10 += style->tracking_x10 * (int32_t)(count - 1);
  }

  int32_t x_x10 = anchor.x * 10;
  if (align == GTextAlignmentCenter) {
    x_x10 -= total_x10 / 2;
  } else if (align == GTextAlignmentRight) {
    x_x10 -= total_x10;
  }
  int16_t left = x_x10 / 10;
  int16_t top = anchor.y - style->ascent;

  graphics_context_set_text_color(ctx, color);
  for (size_t i = 0; i < count; i++) {
    char glyph[2] = { text[i], '\0' };
    int16_t w = prv_glyph_width(text[i], style->font);
    // 枠を詰めるとグリフの張り出しが切れるので、幅・高さとも余裕を持たせる
    GRect box = GRect(x_x10 / 10, top, w + style->ascent, style->ascent * 2);
    graphics_draw_text(ctx, glyph, style->font, box, GTextOverflowModeWordWrap,
                       GTextAlignmentLeft, NULL);
    x_x10 += w * 10 + style->tracking_x10;
  }
  return GRect(left, top, total_x10 / 10, style->ascent);
}

bool text_format_value(char *buf, size_t len, int32_t value) {
  if (value < 0) {
    snprintf(buf, len, "--");
    return false;
  }
  // 末尾から桁を詰め、3桁ごとにカンマを挟む
  char tmp[VALUE_TMP_LEN];
  int pos = sizeof(tmp) - 1;
  tmp[pos] = '\0';
  int digits = 0;
  do {
    if (digits > 0 && digits % 3 == 0) {
      tmp[--pos] = ',';
    }
    tmp[--pos] = '0' + value % 10;
    value /= 10;
    digits++;
  } while (value > 0);
  snprintf(buf, len, "%s", &tmp[pos]);
  return true;
}

void text_to_upper(char *buf) {
  for (char *p = buf; *p; p++) {
    if (*p >= 'a' && *p <= 'z') {
      *p = *p - 'a' + 'A';
    }
  }
}
