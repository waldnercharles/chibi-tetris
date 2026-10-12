# Asset provenance

This repository contains Stillwater's source and authored game content. No commercial Tetris Effect assets, themes, music or branding are included.

The procedural environments and block geometry are original project assets. `tools/glyphs.c` converts the pinned engine's Geist Regular, Medium and Numeric font outlines to filled 3D meshes with Chibi's mesh writer. Geist is copyright 2024 The Geist Project Authors and licensed under the SIL Open Font License 1.1, included in `fonts/Geist-OFL.txt`. The former generated 2D paintings have been removed. `docs/visuals.md` records the current presentation.

The music and sound effects in `content/sounds` and their original score are dedicated to the public domain under CC0 1.0. They were composed and synthesized specifically for Stillwater with `tools/compose.c`, without third-party samples. `docs/audio.md` records the composition, reproduction and audio measurements. The generator remains project source.

The Chibi distribution carries its engine and dependency license notices. Its bundled Inter font is supplied through that SDK and retains its original license. Exported releases must retain the SDK notices.

The pinned engine includes Michael Krzyzaniak's Beat-and-Tempo-Tracking under the MIT License, with the upstream revision and local fixes recorded in its `third_party/btt/README.chibi.md`. The SDK retains `share/chibi/licenses/btt/LICENSE`; browser exports carry the notice under `data/engine/licenses/btt/`. BTT includes its own FFT; KissFFT is not bundled. No LYGIA source is included.

The rotation offsets implement publicly documented geometric rules. `docs/rules.md` records rule references and intentional choices. Stillwater is an independent game and is not affiliated with The Tetris Company or Enhance.

The build-time glyph generator uses libtess2 at upstream commit `8dbd6483e920311a58c9af10a10beb278efebc36`, under the SGI Free Software License B 2.0. Its unmodified source and license are in `tools/vendor/libtess2`. It is not linked into the game.
