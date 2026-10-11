# Stillwater environments

Shore uses moonlit water and distant island silhouettes. Current moves below the surface, with pale light at the upper corners. Aurora returns to open water under soft green and violet curtains. Chapter pairs share one mood: First light and Tide pools use Shore; Undertow and Open water use Current; Polar night and Homeward use Aurora.

The backgrounds keep the center dark. The board frame is opaque and sits above every environment layer. Each source plate is 1672 by 941 pixels. Chibi sprites use one world unit per texel and expose no scale field in the pinned renderer, so the 1280 by 720 camera takes a deliberate central crop. The source's overscan also covers the 2.5-unit horizontal and 1.5-unit vertical drift. The source images are preserved without retouching.

World sets the existing HRC component to ambient 1 with emission brightness, fog, bounce and surface lighting disabled. Every visual uses an explicit scene-color material with zero albedo and emission, so authored colors stay readable and block outlines do not become light sources. Effects change material scene alpha directly. The HRC grid stays at quarter resolution with one trace level and no feedback.

The Environment behavior spawns a fixed pool once: three background plates, eight ambient glints, twenty-four clear glints and two ripple marks. Line-clear serials trigger each board's twelve-glint pool independently. A new clear replaces that board's earlier response. The number of entities does not grow with play time. The glints sit outside the board rectangle and the ripple sits beneath it. Four-line clears increase brightness and size without filling the screen. Stage changes dissolve over 1.6 seconds.

Effects off displays static art. Low effects adds the clear response with six points per board. Full effects adds eight slow perimeter glints and all twelve clear points. Reduced motion freezes background drift, hides moving points, selects stage art immediately and retains only a stationary underline that fades after a clear. Pausing stops the environment clock. All pool entities are visual only and write no gameplay state.

## Provenance

The three plates were generated for Stillwater on 2026-10-10 with OpenAI's built-in image generation tool using original prompts below. No reference image, third-party texture, game screenshot, logo or character was supplied. The selected outputs were copied into this repository:

- `content/images/shore.png`
- `content/images/current.png`
- `content/images/aurora.png`

The visual direction and prompts are original to this project. The source images remain in the local Codex generated-images directory as originals. The repository copies are the game's resources and do not depend on that directory.

## Generation prompts

### shore

Use case: stylized-concept. Asset type: original cinematic 2D game background plate for a quiet falling-block puzzle named Stillwater, stage Shore. Generate one widescreen 16:9 landscape image, ideally 1536x864. Scene: a moonlit tidal sea at night, distant slender islands toward the far left and far right edges, pale blue haze at the high horizon, a few smooth wet rocks low at the extreme outer corners, glasslike dark water reflecting faint silver light. Style: polished painterly digital environment art, restrained and sophisticated, softly textured, broad calm forms with careful depth, a little analog grain, believable water. Composition: viewer at water level, horizon at 28 percent from top; central 65 percent of the image must be mostly uninterrupted deep navy negative space for game boards, readable but never pure flat black. Concentrate visible land silhouettes and pale glints at the edges, retain interesting atmospheric detail above and below boards. Color palette: midnight indigo, muted slate blue, pale seafoam silver. Lighting: quiet nocturnal luminance, no bright sun or large moon, no harsh glow. Constraints: environment only, no text, no typography, no letters, no numbers, no logos, no UI, no grids, no blocks, no frame, no characters, no recognizable existing game imagery.

### current

Use case: stylized-concept. Asset type: original cinematic 2D game background plate for a quiet falling-block puzzle named Stillwater, stage Current. Generate one widescreen 16:9 landscape image. Scene: looking through a deep calm sea just beneath the water surface, the undulating surface visible only in the top fifth; shafts of soft aqua light descend diagonally toward the far left and right edges, distant underwater cliff silhouettes frame the sides, deep water fades into a broad clear midnight-blue central void. No ocean floor across center. Style: polished painterly digital environment art, restrained and sophisticated, softly textured with careful atmospheric depth, subtle analog grain, believable underwater optics, very sparse fine suspended matter only at edges. Composition: central 65 percent and central 70 percent height must be dark quiet negative space for game boards; all strongest forms and light at the perimeter. Broad graceful diagonal currents suggested by light haze, no vortex or visible swirling objects. Palette: inky navy, petrol teal, restrained pale turquoise highlights. Lighting: deep tranquil blue hour, no bright sun, no harsh glow. Constraints: environment only, no text, no typography, no letters, no numbers, no logos, no UI, no grids, no blocks, no frame, no characters, no animals, no recognizable existing game imagery.

### aurora

Use case: stylized-concept. Asset type: original cinematic 2D game background plate for a quiet falling-block puzzle named Stillwater, stage Aurora. Generate one widescreen 16:9 landscape image. Scene: a still northern sea at deepest twilight beneath delicate ribbons of aurora. Remote steep island silhouettes hug far left and far right horizon. Soft green and lavender aurora curtains arc across only the upper quarter of the sky, with very restrained reflection along the water at the outer edges. The sky has a few small stars. Style: polished painterly digital environment art, restrained and sophisticated, softly textured with careful atmospheric depth, subtle analog grain, believable quiet water. Composition: horizon 37 percent from top; central 65 percent of the image must be low-contrast deep navy negative space for game boards, almost empty across the middle, land and glints concentrated at the far edges. Color palette: inky indigo, desaturated violet, hints of glacier mint and pearl. Lighting: nocturnal, faint ethereal luminous wisps overhead, no neon, no harsh bloom, no bright moon. Constraints: environment only, no text, no typography, no letters, no numbers, no logos, no UI, no grids, no blocks, no frame, no characters, no recognizable existing game imagery.

## Validation

The pinned controller validates `Environment.chibi` and all six environment prototypes with zero errors and warnings. Its returned canonical behavior source was saved. The source uses decimal packed colors, `match enum`, explicit fractional conversion of loop indices, and the supported `int` to `uint` conversion for alpha. Generated plate composition was inspected visually. The parent task owns the coordinated whole-game render and runtime checks; source validation does not establish rendered transitions or performance.
