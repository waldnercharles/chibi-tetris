# Stillwater 3D

Replace the painted 2D presentation with an authored 3D game that starts playing immediately. Keep the existing falling-block rules and add measured music response through a reusable Chibi audio capability.

## Context

`Board.chibi` owns the grid, queue, movement and scoring. `Shell.chibi` owns progression and saves. The installed SDK already supplies perspective cameras, solid geometry, emissive materials, bloom, ambient occlusion, native particles and trails. Its audio API plays files but does not expose analysis or external input.

The baseline game tests passed all eight cases and 95 assertions in 12.139 seconds. The main engine checkout contains unrelated work. Audio changes belong in the isolated `stillwater-audio-reactivity` engine worktree, starting at the game's pinned engine revision.

## Changes

- Convert the board, pieces, ghost, hold and queue to real 3D geometry. Preserve the rule grid as the authoritative game state. Publish successful player events for visual consumers.
- Replace the environment with procedural currents, orbital trails, depth particles and chapter-specific forms. Scale response with bass, midrange, treble and transients, plus drops, locks and clears. Keep the well readable and respect reduced motion and effect settings.
- Start Journey directly. Replace menus with in-game controls and world-space mesh text for scores, progress, pause and completion. Retain saves, the other modes and both players.
- Add shared C audio analysis and user-selected external audio input. The game consumes a small analysis snapshot; capture and signal processing remain in the engine. Browser capture requires the browser's source picker. Native capture uses an available loopback device. Spotify sign-in is unsuitable under its current developer policy.
- Remove unused 2D artwork and menu assets. Update controls, audio setup, provenance and validation documentation.

## Verification

Format and validate all authored assets. Extend the C dispatcher for direct launch, presentation and event contracts. Test analysis against known PCM input, silence, transients and source lifecycle. Run the game checks under their shared 60-second deadline. Inspect bounded native render previews and exercise the packaged player and browser export where available. Record observed coverage and unresolved platform limitations explicitly.

## Implemented

The game now launches directly into 3D Journey. Geometry, mesh text, procedural chapter scenery, native particles and measured audio response replace the painted presentation and menus. Native and browser builds share the authored project. The SDK integrates the current native naming contract, reusable PCM analysis/capture and a WebGPU noise-shader correction. Final execution evidence and platform limits are in [docs/validation.md](docs/validation.md).
