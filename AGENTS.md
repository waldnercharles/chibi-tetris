# Stillwater

This is an authored Chibi game. Keep all game rules, scene composition, visual choices and tuning in KDL and readable `.chibi`. Render the game and its text in 3D, and launch directly into play. Use the installed SDK, never a standalone browser game stack.

Read the pinned engine's authoring and test policies. Preserve the hard 60-second deadline for test commands. Permanent checks use C/pico_unit through a single dispatcher; do not create Python or Node test suites. Format edited authored assets with `chibi_ctl --format`, validate the project and verify runtime outcomes. Building is not playability evidence.

Use original or appropriately licensed assets and keep provenance in `docs/audio.md` and `LICENSES.md`. Review the whole diff and run applicable focused checks before committing.

Publish every completed game iteration to GitHub Pages at `https://waldnercharles.github.io/chibi-tetris/`. Follow the README's publishing steps, verify the live revision and browser play, and include the public link in the handoff. For scene or runtime performance changes, measure warmed browser frame times at the normal display resolution; a successful screenshot does not establish playable performance. A local export alone is incomplete.

All additional agents must use `gpt-6-astra` at `xhigh` reasoning.
