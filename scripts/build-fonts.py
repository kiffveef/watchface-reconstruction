# /// script
# requires-python = ">=3.10"
# dependencies = ["fonttools"]
# ///
"""可変フォントから resources/fonts/instances.json の指定どおりに静的 TTF を書き出す。

Pebble のフォント変換は可変フォントの既定インスタンス (Outfit は Thin) しか使わないため、
ウェイトや字幅を固定した TTF を生成してリソースとして登録する。
OpenType の字形切り替え (ss01 など) も効かないので、glyph_substitutions で cmap を直接差し替える。

使い方: uv run scripts/build-fonts.py
"""

import json
from pathlib import Path

from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont

FONTS_DIR = Path(__file__).resolve().parent.parent / "resources" / "fonts"
CONFIG = FONTS_DIR / "instances.json"


def substitute_glyphs(font: TTFont, substitutions: dict[str, str], source_name: str) -> None:
    glyphs = set(font.getGlyphOrder())
    for char, glyph in substitutions.items():
        if glyph not in glyphs:
            raise SystemExit(f"glyph '{glyph}' for '{char}' not found in source '{source_name}'")
        for table in font["cmap"].tables:
            if ord(char) in table.cmap:
                table.cmap[ord(char)] = glyph


def main() -> None:
    config = json.loads(CONFIG.read_text())
    sources = config["sources"]
    for filename, spec in config["instances"].items():
        source_name = spec["source"]
        if source_name not in sources:
            raise SystemExit(f"{filename}: unknown source '{source_name}' in {CONFIG.name}")
        source = sources[source_name]
        font = instantiateVariableFont(TTFont(FONTS_DIR / source["file"]), spec["axes"])
        substitute_glyphs(font, source.get("glyph_substitutions", {}), source_name)
        # 保存日時を書き込むと同じ設定でも生成のたびに差分が出るため、元フォントの日時を保つ
        font.recalcTimestamp = False
        out = FONTS_DIR / filename
        font.save(out)
        print(f"{out.relative_to(FONTS_DIR.parent.parent)}: {source_name} {spec['axes']}")


if __name__ == "__main__":
    main()
