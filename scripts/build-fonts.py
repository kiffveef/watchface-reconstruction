# /// script
# requires-python = ">=3.10"
# dependencies = ["fonttools"]
# ///
"""可変フォントから resources/fonts/instances.json の指定どおりに静的 TTF を書き出す。

Pebble のフォント変換は可変フォントの既定インスタンス (Outfit は Thin) しか使わないため、
ウェイトを固定した TTF を生成してリソースとして登録する。
OpenType の字形切り替え (ss01 など) も効かないので、glyph_substitutions で cmap を直接差し替える。

使い方: uv run scripts/build-fonts.py
"""

import json
from pathlib import Path

from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont

FONTS_DIR = Path(__file__).resolve().parent.parent / "resources" / "fonts"
CONFIG = FONTS_DIR / "instances.json"


def substitute_glyphs(font: TTFont, substitutions: dict[str, str]) -> None:
    glyphs = set(font.getGlyphOrder())
    for char, glyph in substitutions.items():
        if glyph not in glyphs:
            raise SystemExit(f"glyph '{glyph}' for '{char}' not found in {CONFIG.name} source font")
        for table in font["cmap"].tables:
            if ord(char) in table.cmap:
                table.cmap[ord(char)] = glyph


def main() -> None:
    config = json.loads(CONFIG.read_text())
    source = FONTS_DIR / config["source"]
    substitutions = config.get("glyph_substitutions", {})
    for filename, axes in config["instances"].items():
        font = instantiateVariableFont(TTFont(source), axes)
        substitute_glyphs(font, substitutions)
        out = FONTS_DIR / filename
        font.save(out)
        print(f"{out.relative_to(FONTS_DIR.parent.parent)}: {axes}")


if __name__ == "__main__":
    main()
