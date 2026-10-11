# Asset provenance

This repository contains Stillwater's source and authored game content. No commercial Tetris Effect assets, themes, music or branding are included.

The procedural environments, block geometry and mesh text are original project assets. `tools/glyphs.c` generates the text meshes from original stroke forms using Chibi's mesh writer. The former generated 2D paintings have been removed. `docs/visuals.md` records the current presentation.

The music and sound effects in `content/sounds` and their original score are dedicated to the public domain under CC0 1.0. They were composed and synthesized specifically for Stillwater with `tools/compose.c`, without third-party samples. `docs/audio.md` records the composition, reproduction and audio measurements. The generator remains project source.

The Chibi distribution carries its engine and dependency license notices. Its bundled Inter font is supplied through that SDK and retains its original license. Exported releases must retain the SDK notices.

The rotation offsets implement publicly documented geometric rules. `docs/rules.md` records rule references and intentional choices. Stillwater is an independent game and is not affiliated with The Tetris Company or Enhance.
