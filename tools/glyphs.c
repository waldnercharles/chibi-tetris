/* Geist outlines become filled, shallow extruded meshes in the 3D scene.
 * Font copyright and SIL Open Font License: fonts/Geist-OFL.txt.
 *
 * Run from the project root with the pinned SDK installed:
 * cc -std=c11 -O2 -Wall -Wextra -Werror \
 *   -I.chibi/sdk/include/chibi/mesh_runtime \
 *   -I.chibi/sdk/include/chibi/support -I.chibi/sdk/include/chibi/math \
 *   -isystem .chibi/sdk/include/chibi/desktop_vendor/cute/2/stb \
 *   -Itools/vendor/libtess2/Include \
 *   tools/glyphs.c tools/vendor/libtess2/Source/{bucketalloc,dict,geom,mesh,priorityq,sweep,tess}.c \
 *   -L.chibi/sdk/lib -lchibi_mesh_runtime -lchibi_types \
 *   -lchibi_support -Wl,-dead_strip -lm -o build/glyphs
 * ./build/glyphs
 * ./build/glyphs --block
 * ./build/glyphs --preview build/glyphs.svg
 * rm build/glyphs
 * On Linux, replace -Wl,-dead_strip with -Wl,--gc-sections.
 *
 * Each label is one indexed mesh, centered in XY with a one-unit cap frame.
 * Readable front faces +Z. Extrusion is 0.08 units. Digits are spaced at
 * 0.8 times their height by the authored HUD. No font raster is rendered.
 */
#include "runtime/geometry/mesh_prepared.h"
#include "support/array.h"
#include "support/xalloc.h"
#include "vendor/libtess2/Include/tesselator.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define DEPTH .08f
#define CURVE_TOLERANCE .0025f
#define FILL_SAMPLE_AXIS 24
#define FILL_SIDE_EPSILON .000001f
#define FACE_AREA_EPSILON 1e-10f

static stbtt_fontinfo regular, medium, numeric;

typedef struct
{
    float x, y;
} Point;

typedef struct
{
    Point a, b;
} Edge;

static const struct
{
    const char *name, *text;
} labels[] = {
    { "stillwater", "Stillwater" },
    { "score", "SCORE" },
    { "lines", "LINES" },
    { "hold", "HOLD" },
    { "next", "NEXT" },
    { "journey", "JOURNEY" },
    { "endless", "ENDLESS" },
    { "sprint", "SPRINT" },
    { "duet", "DUET" },
    { "paused", "Paused" },
    { "complete", "Complete" },
    { "game_over", "Game over" },
    { "press_enter", "Enter to continue" },
    { "remaining", "REMAINING" },
    { "elapsed", "TIME" },
    { "best", "BEST" },
    { "no_best", "No best yet" },
    { "chapter_controls", "7 / 8  Chapter" },
    { "controller_lost", "Controller disconnected" },
    { "saving_failed", "Could not save" },
    { "chapter", "CHAPTER" },
    { "rescue", "RESCUE" },
    { "music", "M  Connect music" },
    { "audio_share_hint", "Choose a music tab and share its audio" },
    { "audio_no_source", "Audio sharing unavailable" },
    { "audio_cancelled", "Audio sharing cancelled" },
    { "audio_ended", "Audio sharing ended" },
    { "setting_effects_off", "Effects off" },
    { "setting_effects_low", "Effects low" },
    { "setting_effects_full", "Effects full" },
    { "setting_motion_on", "Reduced motion on" },
    { "setting_motion_off", "Reduced motion off" },
    { "setting_ghost_on", "Ghost on" },
    { "setting_ghost_off", "Ghost off" },
    { "setting_volume", "Volume" },
    { "setting_das", "Move delay" },
    { "setting_arr", "Move repeat" },
    { "setting_ms", "ms" },
    { "setting_percent", "%" },
    { "play_controls", "Arrows  Move     X / Z  Rotate     Space  Drop     C  Hold" },
    { "settings_controls",
      "1 - 4  Mode     V  Effects     F  Motion     G  Ghost     9 / 0  Volume" },
    { "connect_music", "Connect your music" },
    { "music_youtube", "1  YouTube" },
    { "music_spotify", "2  Spotify" },
    { "music_tidal", "3  TIDAL" },
    { "music_listen", "M  Share music audio" },
    { "music_score", "4  Use game score" },
    { "music_intro", "Play a track, then share its audio" },
    { "music_play", "Play a track to begin" },
    { "music_retry", "M  Try again" },
    { "pause_hint", "P  Pause" },

};

static void face(render_mesh_asset *mesh, const CF_V3 *positions, int count)
{
    assert(count == 3 || count == 4);
    CF_V3 edge = cf_sub_v3(positions[1], positions[0]);
    CF_V3 normal = cf_norm_v3(cf_cross(edge, cf_sub_v3(positions[2], positions[0])));
    CF_V3 tangent = cf_norm_v3(edge);
    const CF_V2 uv[] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
    const int corners[] = { 0, 1, 2, 0, 2, 3 };
    int first = acount(mesh->vertices);
    for (int i = 0; i < count; i++) {
        render_mesh_surface_vertex v = {
            .position = positions[i],
            .normal = normal,
            .uv = uv[i],
            .tangent = { tangent.x, tangent.y, tangent.z, 1 }
        };
        apush(mesh->vertices, v);
    }
    for (int i = 0; i < (count - 2) * 3; i++) {
        apush(mesh->indices, first + corners[i]);
    }
}

static void quad(render_mesh_asset *mesh, CF_V3 a, CF_V3 b, CF_V3 c, CF_V3 d)
{
    const CF_V3 positions[] = { a, b, c, d };
    face(mesh, positions, COUNT(positions));
}

static Point midpoint(Point a, Point b)
{
    return (Point){ (a.x + b.x) * .5f, (a.y + b.y) * .5f };
}

static void edge(Edge **edges, Point a, Point b)
{
    if (a.x == b.x && a.y == b.y) {
        return;
    }
    apush(*edges, ((Edge){ a, b }));
}

static void curve(Edge **edges, Point a, Point control, Point b, int depth)
{
    Point middle = midpoint(a, b);
    float deviation = hypotf(control.x - middle.x, control.y - middle.y);
    if (deviation <= CURVE_TOLERANCE || depth == 12) {
        edge(edges, a, b);
        return;
    }
    Point left = midpoint(a, control), right = midpoint(control, b);
    middle = midpoint(left, right);
    curve(edges, a, left, middle, depth + 1);
    curve(edges, middle, right, b, depth + 1);
}

static float side_of(Point a, Point b, Point p)
{
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

/* Compare the filled triangles to the contours' nonzero winding, including
 * interior counters and overlapping outlines. Boundary samples are omitted
 * because both representations include their own floating-point rounding. */
static void verify_fill(TESStesselator *tess, const Edge *edges)
{
    Point low = edges[0].a, high = low;
    for (int i = 0; i < acount(edges); i++) {
        low.x = fminf(low.x, edges[i].a.x);
        low.y = fminf(low.y, edges[i].a.y);
        high.x = fmaxf(high.x, edges[i].a.x);
        high.y = fmaxf(high.y, edges[i].a.y);
    }
    const TESSreal *vertices = tessGetVertices(tess);
    const TESSindex *indices = tessGetElements(tess);
    for (int row = 0; row < FILL_SAMPLE_AXIS; row++) {
        for (int column = 0; column < FILL_SAMPLE_AXIS; column++) {
            Point sample = {
                low.x + (high.x - low.x) * (column + .5f) / FILL_SAMPLE_AXIS,
                low.y + (high.y - low.y) * (row + .5f) / FILL_SAMPLE_AXIS,
            };
            int winding = 0;
            bool boundary = false;
            for (int i = 0; i < acount(edges); i++) {
                Edge edge = edges[i];
                float side = side_of(edge.a, edge.b, sample);
                boundary |= fabsf(side) < FILL_SIDE_EPSILON;
                if (edge.a.y <= sample.y && edge.b.y > sample.y && side > 0) {
                    winding++;
                }
                if (edge.a.y > sample.y && edge.b.y <= sample.y && side < 0) {
                    winding--;
                }
            }
            if (boundary) {
                continue;
            }
            bool filled = false;
            for (int i = 0; i < tessGetElementCount(tess) && !filled; i++) {
                Point p[3];
                for (int j = 0; j < 3; j++) {
                    int at = indices[i * 3 + j] * 2;
                    p[j] = (Point){ vertices[at], vertices[at + 1] };
                }
                if (fabsf(side_of(p[0], p[1], p[2])) < FACE_AREA_EPSILON) {
                    continue;
                }
                filled = side_of(p[0], p[1], sample) >= -FILL_SIDE_EPSILON &&
                         side_of(p[1], p[2], sample) >= -FILL_SIDE_EPSILON &&
                         side_of(p[2], p[0], sample) >= -FILL_SIDE_EPSILON;
            }
            assert(filled == (winding != 0));
        }
    }
}

static void fill(render_mesh_asset *mesh, const Edge *edges, const int *contours)
{
    TESStesselator *tess = tessNewTess(NULL);
    assert(tess);
    Point *points = NULL;
    for (int i = 0; i < acount(contours); i++) {
        int end = i + 1 < acount(contours) ? contours[i + 1] : acount(edges);
        asetlen(points, 0);
        for (int j = contours[i]; j < end; j++) {
            apush(points, edges[j].a);
        }
        assert(acount(points) >= 3);
        tessAddContour(tess, 2, points, sizeof *points, acount(points));
    }
    const TESSreal normal[] = { 0, 0, 1 };
    assert(tessTesselate(tess, TESS_WINDING_NONZERO, TESS_POLYGONS, 3, 2, normal));
    verify_fill(tess, edges);
    const TESSreal *vertices = tessGetVertices(tess);
    const TESSindex *indices = tessGetElements(tess);
    int count = tessGetVertexCount(tess);
    int first = acount(mesh->vertices);
    for (int i = 0; i < count; i++) {
        for (int side = 0; side < 2; side++) {
            float sign = side ? -1.0f : 1.0f;
            render_mesh_surface_vertex vertex = {
                .position = { vertices[i * 2], vertices[i * 2 + 1], sign * DEPTH * .5f },
                .normal = { 0, 0, sign },
                .uv = { vertices[i * 2], vertices[i * 2 + 1] },
                .tangent = { 1, 0, 0, sign }
            };
            apush(mesh->vertices, vertex);
        }
    }
    for (int i = 0; i < tessGetElementCount(tess); i++) {
        int triangle[3];
        for (int j = 0; j < 3; j++) {
            int index = indices[i * 3 + j];
            assert(index >= 0 && index < count);
            triangle[j] = first + index * 2;
        }
        CF_V3 a = mesh->vertices[triangle[0]].position;
        CF_V3 b = mesh->vertices[triangle[1]].position;
        CF_V3 c = mesh->vertices[triangle[2]].position;
        float area = cf_cross(cf_sub_v3(b, a), cf_sub_v3(c, a)).z;
        if (fabsf(area) < FACE_AREA_EPSILON) {
            continue;
        }
        assert(area > 0);
        for (int j = 0; j < 3; j++) {
            apush(mesh->indices, triangle[j]);
        }
        for (int j = 2; j >= 0; j--) {
            apush(mesh->indices, triangle[j] + 1);
        }
    }
    for (int i = 0; i < acount(edges); i++) {
        Point a = edges[i].a, b = edges[i].b;
        /* TrueType outer contours are clockwise; holes reverse winding. */
        quad(mesh,
             (CF_V3){ b.x, b.y, -DEPTH * .5f },
             (CF_V3){ a.x, a.y, -DEPTH * .5f },
             (CF_V3){ a.x, a.y, DEPTH * .5f },
             (CF_V3){ b.x, b.y, DEPTH * .5f });
    }
    afree(points);
    tessDeleteTess(tess);
}

static float letter(render_mesh_asset *mesh, const stbtt_fontinfo *font, int c, float x)
{
    int advance, bearing, x0, y0, x1, y1;
    stbtt_GetCodepointHMetrics(font, c, &advance, &bearing);
    assert(stbtt_GetCodepointBox(font, 'H', &x0, &y0, &x1, &y1));
    float scale = 1.0f / (float)(y1 - y0);
    stbtt_vertex *vertices;
    int count = stbtt_GetCodepointShape(font, c, &vertices);
    if (c == ' ') {
        assert(!count);
        return advance * scale;
    }
    assert(count);
    Edge *edges = NULL;
    int *contours = NULL;
    Point previous = { 0 }, start = { 0 };
    for (int i = 0; i < count; i++) {
        const stbtt_vertex *v = &vertices[i];
        Point p = { x + v->x * scale, (v->y - y0) * scale - .5f };
        switch (v->type) {
            case STBTT_vmove:
                if (i) {
                    edge(&edges, previous, start);
                }
                apush(contours, acount(edges));
                start = p;
                break;
            case STBTT_vline:
                edge(&edges, previous, p);
                break;
            case STBTT_vcurve: {
                Point control = { x + v->cx * scale, (v->cy - y0) * scale - .5f };
                curve(&edges, previous, control, p, 0);
                break;
            }
            default:
                /* The pinned Geist TrueType files use quadratic outlines. */
                assert(false);
        }
        previous = p;
    }
    edge(&edges, previous, start);
    fill(mesh, edges, contours);
    afree(contours);
    afree(edges);
    stbtt_FreeShape(font, vertices);
    return advance * scale;
}

static void bounds(render_mesh_asset *mesh)
{
    assert(acount(mesh->vertices));
    mesh->min = mesh->max = mesh->vertices[0].position;
    for (int i = 1; i < acount(mesh->vertices); i++) {
        mesh->min = cf_min_v3(mesh->min, mesh->vertices[i].position);
        mesh->max = cf_max_v3(mesh->max, mesh->vertices[i].position);
    }
}

static void center(render_mesh_asset *mesh)
{
    bounds(mesh);
    float x = (mesh->min.x + mesh->max.x) * .5f;
    float y = (mesh->min.y + mesh->max.y) * .5f;
    float scale = 1 / (mesh->max.y - mesh->min.y);
    for (int i = 0; i < acount(mesh->vertices); i++) {
        CF_V3 *p = &mesh->vertices[i].position;
        p->x = (p->x - x) * scale;
        p->y = (p->y - y) * scale;
    }
    bounds(mesh);
}

static void verify(const render_mesh_asset *mesh)
{
    for (int i = 0; i < acount(mesh->vertices); i++) {
        const render_mesh_surface_vertex *v = &mesh->vertices[i];
        CF_V3 t = { v->tangent.x, v->tangent.y, v->tangent.z };
        assert(
            isfinite(v->position.x) && isfinite(v->position.y) &&
            isfinite(v->position.z)
        );
        assert(fabsf(cf_len_v3(v->normal) - 1) < .0001f);
        assert(fabsf(cf_len_v3(t) - 1) < .0001f);
        assert(fabsf(cf_dot_v3(v->normal, t)) < .0001f);
    }
    assert(acount(mesh->indices) % 3 == 0);
    for (int i = 0; i < acount(mesh->indices); i++) {
        assert(mesh->indices[i] >= 0 && mesh->indices[i] < acount(mesh->vertices));
    }
    for (int i = 0; i < acount(mesh->indices); i += 3) {
        const render_mesh_surface_vertex *a = &mesh->vertices[mesh->indices[i]];
        CF_V3 b = mesh->vertices[mesh->indices[i + 1]].position;
        CF_V3 c = mesh->vertices[mesh->indices[i + 2]].position;
        CF_V3 cross = cf_cross(cf_sub_v3(b, a->position), cf_sub_v3(c, a->position));
        assert(cf_dot_v3(cross, a->normal) > 1e-12f);
    }
}

static render_mesh_asset make_mesh(const char *text)
{
    render_mesh_asset mesh = { 0 };
    const stbtt_fontinfo *font = &medium;
    if (!strcmp(text, "Stillwater")) {
        font = &regular;
    }
    if (strlen(text) == 1 && isdigit((unsigned char)text[0])) {
        font = &numeric;
    }
    float x = 0;
    for (const char *c = text; *c; c++) {
        x += letter(&mesh, font, (unsigned char)*c, x);
        int x0, y0, x1, y1;
        assert(stbtt_GetCodepointBox(font, 'H', &x0, &y0, &x1, &y1));
        x += (float)stbtt_GetCodepointKernAdvance(font, *c, c[1]) / (float)(y1 - y0);
    }
    center(&mesh);
    verify(&mesh);
    assert(fabsf(mesh.min.x + mesh.max.x) < .0001f);
    assert(fabsf(mesh.min.y + .5f) < .0001f);
    assert(fabsf(mesh.max.y - .5f) < .0001f);
    return mesh;
}

static void write_prototype(const char *directory, const char *name)
{
    char prototype[128] = "Text";
    int length = 4;
    bool upper = true;
    for (const char *c = name; *c; c++) {
        if (*c == '_') {
            upper = true;
            continue;
        }
        assert(length < (int)sizeof prototype - 1);
        prototype[length++] = upper ? (char)toupper((unsigned char)*c) : *c;
        upper = false;
    }
    prototype[length] = 0;
    char path[512];
    int n = snprintf(path, sizeof path, "%s/%s.kdl", directory, prototype);
    assert(n > 0 && (size_t)n < sizeof path);
    FILE *file = fopen(path, "w");
    assert(file);
    /* All root-level prototypes are prepared for export. This reference keeps
     * a mesh available when the HUD selects its source dynamically. */
    assert(
        fprintf(file, "name \"%s\"\ncomponents {\n    MeshAsset source=\"content/meshes/text/%s.mesh\"\n}\n", prototype, name) >
        0
    );
    assert(!fclose(file));
}

static void write_mesh(const render_mesh_asset *mesh, const char *path)
{
    unsigned char *data;
    size_t size;
    char error[512];
    if (!render_mesh_asset_prepared_write(mesh, &data, &size, error, sizeof error)) {
        fprintf(stderr, "%s: %s\n", path, error);
        exit(1);
    }
    size_t budget = 64 * 1024 * 1024;
    render_mesh_asset decoded = { 0 };
    if (!render_mesh_asset_prepared_read_budget(data, size, &budget, &decoded, error, sizeof error)) {
        fprintf(stderr, "%s: %s\n", path, error);
        exit(1);
    }
    verify(&decoded);
    assert(acount(decoded.vertices) == acount(mesh->vertices));
    assert(acount(decoded.indices) == acount(mesh->indices));
    FILE *file = fopen(path, "wb");
    assert(file);
    assert(fwrite(data, 1, size, file) == size);
    assert(!fclose(file));
    free(data);
    render_mesh_asset_free(&decoded);
}

static void publish(render_mesh_asset *mesh, const char *directory, const char *prototypes, const char *name)
{
    char path[512];
    int n = snprintf(path, sizeof path, "%s/%s.mesh", directory, name);
    assert(n > 0 && (size_t)n < sizeof path);
    write_mesh(mesh, path);
    write_prototype(prototypes, name);
    printf(
        "%-20s %.6f x %.6f x %.6f  %d triangles\n",
        name,
        mesh->max.x - mesh->min.x,
        mesh->max.y - mesh->min.y,
        mesh->max.z - mesh->min.z,
        acount(mesh->indices) / 3
    );
    render_mesh_asset_free(mesh);
}

static void generate(const char *directory, const char *prototypes, const char *name, const char *text)
{
    render_mesh_asset mesh = make_mesh(text);
    publish(&mesh, directory, prototypes, name);
}

/* Punctuation retains the digit cap frame rather than expanding to one unit. */
static void punctuation(const char *directory, const char *prototypes)
{
    const char *names[] = { "time_colon", "time_decimal" };
    const char *characters = ":.";
    for (int i = 0; i < 2; i++) {
        render_mesh_asset mesh = { 0 };
        letter(&mesh, &numeric, characters[i], 0);
        bounds(&mesh);
        float x = (mesh.min.x + mesh.max.x) * .5f;
        for (int j = 0; j < acount(mesh.vertices); j++) {
            mesh.vertices[j].position.x -= x;
        }
        bounds(&mesh);
        verify(&mesh);
        publish(&mesh, directory, prototypes, names[i]);
    }
}

static void outward_face(render_mesh_asset *mesh, CF_V3 *positions, int count)
{
    CF_V3 normal = cf_cross(
        cf_sub_v3(positions[1], positions[0]),
        cf_sub_v3(positions[2], positions[0])
    );
    if (cf_dot_v3(normal, positions[0]) < 0) {
        for (int i = 0; i < count / 2; i++) {
            CF_V3 value = positions[i];
            positions[i] = positions[count - i - 1];
            positions[count - i - 1] = value;
        }
    }
    face(mesh, positions, count);
}

static bool same(CF_V3 a, CF_V3 b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

/* Each triangle edge must have exactly one oppositely wound neighbor, even
 * where vertices are split for flat normals. This catches a missing bevel
 * face, wrong corner, or winding that a position-only bounds check misses. */
static void closed(const render_mesh_asset *mesh)
{
    float volume = 0;
    for (int i = 0; i < acount(mesh->indices); i += 3) {
        CF_V3 a = mesh->vertices[mesh->indices[i]].position;
        CF_V3 b = mesh->vertices[mesh->indices[i + 1]].position;
        CF_V3 c = mesh->vertices[mesh->indices[i + 2]].position;
        volume += cf_dot_v3(a, cf_cross(b, c)) / 6;
        for (int edge = 0; edge < 3; edge++) {
            a = mesh->vertices[mesh->indices[i + edge]].position;
            b = mesh->vertices[mesh->indices[i + (edge + 1) % 3]].position;
            int neighbors = 0;
            for (int j = 0; j < acount(mesh->indices); j += 3) {
                for (int other = 0; other < 3; other++) {
                    CF_V3 from = mesh->vertices[mesh->indices[j + other]].position;
                    CF_V3 to = mesh->vertices[mesh->indices[j + (other + 1) % 3]]
                                   .position;
                    neighbors += same(a, to) && same(b, from);
                }
            }
            assert(neighbors == 1);
        }
    }
    assert(volume > .97f && volume < .98f);
}

static void block()
{
    const float inner = .5f - .07f;
    render_mesh_asset mesh = { 0 };
    for (int axis = 0; axis < 3; axis++) {
        int u = (axis + 1) % 3, v = (axis + 2) % 3;
        for (int sign = -1; sign <= 1; sign += 2) {
            float p[4][3] = { 0 };
            for (int i = 0; i < 4; i++) {
                p[i][axis] = .5f * sign;
                p[i][u] = (i == 1 || i == 2) ? inner : -inner;
                p[i][v] = i >= 2 ? inner : -inner;
            }
            CF_V3 positions[4];
            for (int i = 0; i < 4; i++) {
                positions[i] = (CF_V3){ p[i][0], p[i][1], p[i][2] };
            }
            outward_face(&mesh, positions, 4);
        }
        for (int a = -1; a <= 1; a += 2) {
            for (int b = -1; b <= 1; b += 2) {
                float p[4][3] = { 0 };
                for (int i = 0; i < 4; i++) {
                    p[i][axis] = i >= 2 ? inner : -inner;
                    p[i][u] = a * ((i == 0 || i == 3) ? .5f : inner);
                    p[i][v] = b * ((i == 0 || i == 3) ? inner : .5f);
                }
                CF_V3 positions[4];
                for (int i = 0; i < 4; i++) {
                    positions[i] = (CF_V3){ p[i][0], p[i][1], p[i][2] };
                }
                outward_face(&mesh, positions, 4);
            }
        }
    }
    for (int x = -1; x <= 1; x += 2) {
        for (int y = -1; y <= 1; y += 2) {
            for (int z = -1; z <= 1; z += 2) {
                CF_V3 positions[] = {
                    { x * .5f, y * inner, z * inner },
                    { x * inner, y * .5f, z * inner },
                    { x * inner, y * inner, z * .5f },
                };
                outward_face(&mesh, positions, 3);
            }
        }
    }
    bounds(&mesh);
    verify(&mesh);
    assert(acount(mesh.vertices) == 96 && acount(mesh.indices) == 132);
    assert(same(mesh.min, (CF_V3){ -.5f, -.5f, -.5f }));
    assert(same(mesh.max, (CF_V3){ .5f, .5f, .5f }));
    closed(&mesh);
    write_mesh(&mesh, "content/meshes/Block.mesh");
    render_mesh_asset_free(&mesh);
    puts("Block                1.000000 x 1.000000 x 1.000000  44 triangles, bevel 0.07");
}

static void directory(const char *path)
{
    if (mkdir(path, 0755) && errno != EEXIST) {
        perror(path);
        exit(1);
    }
}

static void preview(const char *path)
{
    static const char *const samples[] = {
        "Stillwater",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "0123456789",
        "SCORE   LINES   HOLD   NEXT   CHAPTER",
        "JOURNEY   ENDLESS   SPRINT   DUET",
        "PAUSED   COMPLETE   GAME OVER",
        "ARROWS MOVE   X Z ROTATE   SPACE DROP",
    };
    FILE *file = fopen(path, "w");
    assert(file);
    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1100\" height=\"450\" viewBox=\"0 0 1100 450\"><rect width=\"1100\" height=\"450\" fill=\"#102421\"/><g fill=\"#e9e6d7\">\n");
    for (int row = 0; row < COUNT(samples); row++) {
        render_mesh_asset mesh = make_mesh(samples[row]);
        for (int i = 0; i < acount(mesh.indices); i += 3) {
            if (mesh.vertices[mesh.indices[i]].normal.z < .5f) {
                continue;
            }
            fprintf(file, "<polygon points=\"");
            for (int j = 0; j < 3; j++) {
                CF_V3 p = mesh.vertices[mesh.indices[i + j]].position;
                fprintf(file, "%.3f,%.3f ", 30 + (p.x - mesh.min.x) * 35, 45 + row * 58 - p.y * 35);
            }
            fprintf(file, "\"/>\n");
        }
        render_mesh_asset_free(&mesh);
    }
    fprintf(file, "</g></svg>\n");
    assert(!ferror(file));
    assert(!fclose(file));
}

static void load_font(stbtt_fontinfo *font, const char *name)
{
    char path[512];
    int n = snprintf(path, sizeof path, ".chibi/engine/third_party/fonts/geist/Geist-%s.ttf", name);
    assert(n > 0 && (size_t)n < sizeof path);
    FILE *file = fopen(path, "rb");
    assert(file);
    assert(!fseek(file, 0, SEEK_END));
    long size = ftell(file);
    assert(size > 0);
    rewind(file);
    unsigned char *data = xmalloc((size_t)size);
    assert(fread(data, 1, (size_t)size, file) == (size_t)size);
    assert(!fclose(file));
    assert(stbtt_InitFont(font, data, 0));
}

static void free_fonts()
{
    free(regular.data);
    free(medium.data);
    free(numeric.data);
}

int main(int argc, char **argv)
{
    if (argc >= 2 && !strcmp(argv[1], "--block")) {
        assert(argc <= 3);
        if (argc == 3) {
            assert(!chdir(argv[2]));
        }
        directory("content/meshes");
        block();
        return 0;
    }
    if ((argc == 3 || argc == 4) && !strcmp(argv[1], "--preview")) {
        if (argc == 4) {
            assert(!chdir(argv[3]));
        }
        load_font(&regular, "Regular");
        load_font(&medium, "Medium");
        load_font(&numeric, "Numeric");
        preview(argv[2]);
        free_fonts();
        return 0;
    }
    assert(argc <= 2);
    if (argc == 2) {
        assert(!chdir(argv[1]));
    }
    load_font(&regular, "Regular");
    load_font(&medium, "Medium");
    load_font(&numeric, "Numeric");
    const char *output = "content/meshes/text";
    const char *prototypes = "content/prototypes";
    directory("content/meshes");
    directory("content/meshes/text");
    for (int i = 0; i < 10; i++) {
        char name[] = "digit_0";
        char text[] = "0";
        name[6] += i;
        text[0] += i;
        generate(output, prototypes, name, text);
    }
    for (int i = 0; i < COUNT(labels); i++) {
        generate(output, prototypes, labels[i].name, labels[i].text);
    }
    punctuation(output, prototypes);
    free_fonts();
    return 0;
}
