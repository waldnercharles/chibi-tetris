# Stillwater

This is an authored Chibi game. Keep all game rules, scene composition, visual choices and tuning in KDL and readable `.chibi`. Render the game and its text in 3D, and launch directly into play. Use the installed SDK, never a standalone browser game stack.

Read the pinned engine's authoring and test policies. Preserve the hard 60-second deadline for test commands. Permanent checks use C/pico_unit through a single dispatcher; do not create Python or Node test suites. Format edited authored assets with `chibi_ctl --format`, validate the project and verify runtime outcomes. Building is not playability evidence.

Use original or appropriately licensed assets and keep provenance in `docs/audio.md` and `LICENSES.md`. GitHub Pages serves the browser export from the root of `gh-pages`; see the README for publishing steps. Review the whole diff and run applicable focused checks before committing.

All additional agents must use `gpt-6-astra` at `xhigh` reasoning.
