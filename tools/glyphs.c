/* Original Stillwater stroke lettering, drawn for this game without font or
 * third-party geometry. The generated meshes are dedicated to CC0 1.0.
 *
 * Run from the project root after installing the pinned SDK:
 * cc -std=c11 -O2 -Wall -Wextra -Werror \
 *   -I.chibi/sdk/include/chibi/mesh_runtime \
 *   -I.chibi/sdk/include/chibi/support -I.chibi/sdk/include/chibi/math \
 *   tools/glyphs.c -L.chibi/sdk/lib -lchibi_mesh_runtime -lchibi_types \
 *   -lchibi_support -Wl,-dead_strip -lm -o build/glyphs
 * ./build/glyphs
 * ./build/glyphs --block
 * ./build/glyphs --preview build/glyphs.svg
 * rm build/glyphs
 * On Linux, replace -Wl,-dead_strip with -Wl,--gc-sections.
 * Pass the project directory as the last argument when running elsewhere.
 *
 * Each label is one prepared indexed mesh. A stroke is a closed box with
 * separate face normals. XY bounds are centered, height is exactly one unit,
 * depth is 0.08, and the readable front faces +Z. Printed widths include the
 * strokes, with no hidden padding. Digits use the same glyph metrics; callers
 * can space digit entities at 0.8 times their height for tabular numbers.
 */
#include "runtime/geometry/mesh_prepared.h"
#include "support/array.h"

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
#define STROKE_WIDTH .12f
#define DEPTH .08f

typedef struct
{
    float x, y;
} Point;

typedef struct
{
    char letter;
    float advance;
    const char *paths;
} Glyph;

/* Most letters use a 6 by 10 grid; M and W are wider. A space lifts the pen;
 * each subsequent coordinate pair extends its stroke. Lowercase 'a' means
 * ten on the vertical axis. */
static const Glyph glyphs[] = {
    { 'A', .76f, "003a60 1454" },
    { 'B', .76f, "000a4a68674505 4563624000" },
    { 'C', .76f, "684a2a0802204062" },
    { 'D', .76f, "000a3a67633000" },
    { 'E', .72f, "6a0a0060 0545" },
    { 'F', .72f, "000a6a 0545" },
    { 'G', .76f, "684a2a08022040626535" },
    { 'H', .76f, "000a 606a 0565" },
    { 'I', .54f, "0a4a 2a20 0040" },
    { 'J', .76f, "3a6a62402002" },
    { 'K', .76f, "000a 6a0460" },
    { 'L', .70f, "0a0060" },
    { 'M', .96f, "000a468a80" },
    { 'N', .78f, "000a606a" },
    { 'O', .78f, "204062684a2a080220" },
    { 'P', .76f, "000a4a68674505" },
    { 'Q', .90f, "204062684a2a080220 4370" },
    { 'R', .78f, "000a4a68674505 3560" },
    { 'S', .74f, "684a2a080725456362402002" },
    { 'T', .74f, "0a6a 3a30" },
    { 'U', .78f, "0a022040626a" },
    { 'V', .78f, "0a306a" },
    { 'W', 1.00f, "0a2045608a" },
    { 'X', .76f, "006a 0a60" },
    { 'Y', .76f, "0a356a 3530" },
    { 'Z', .76f, "0a6a0060" },
    { '0', .78f, "204062684a2a080220" },
    { '1', .78f, "183a30 1060" },
    { '2', .78f, "082a4a68670060" },
    { '3', .78f, "0a4a68674535 456362402002" },
    { '4', .78f, "5a0464 5a50" },
    { '5', .78f, "6a0a05456362402002" },
    { '6', .78f, "684a2a0802204062634505" },
    { '7', .78f, "0a6a10" },
    { '8', .78f, "2507082a4a68674525 250302204062634525" },
    { '9', .78f, "452507082a4a6862402002 4565" },
};

static const struct
{
    const char *name, *text;
} labels[] = {
    { "stillwater", "STILLWATER" },
    { "score", "SCORE" },
    { "lines", "LINES" },
    { "hold", "HOLD" },
    { "next", "NEXT" },
    { "journey", "JOURNEY" },
    { "endless", "ENDLESS" },
    { "sprint", "SPRINT" },
    { "duet", "DUET" },
    { "paused", "PAUSED" },
    { "complete", "COMPLETE" },
    { "game_over", "GAME OVER" },
    { "press_enter", "PRESS ENTER" },
    { "remaining", "REMAINING" },
    { "elapsed", "TIME" },
    { "best", "BEST" },
    { "no_best", "NO BEST" },
    { "chapter_controls", "7 8 CHAPTER" },
    { "controller_lost", "CONTROLLER LOST" },
    { "saving_failed", "SAVING FAILED" },
    { "chapter", "CHAPTER" },
    { "rescue", "RESCUE" },
    { "music", "M MUSIC" },
    { "listening", "M LISTENING" },
    { "audio_unavailable", "M RETRY AUDIO" },
    { "audio_waiting", "CHOOSE AUDIO" },
    { "play_controls", "ARROWS MOVE   X Z ROTATE   SPACE DROP   C HOLD   P PAUSE" },
    { "settings_controls",
      "1 2 3 4 MODE   V EFFECTS   F MOTION   G GHOST   9 0 VOLUME   M AUDIO" },
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

static void extrude(render_mesh_asset *mesh, const Point corners[4])
{
    CF_V3 front[4], back[4];
    for (int i = 0; i < 4; i++) {
        front[i] = (CF_V3){ corners[i].x, corners[i].y, DEPTH * .5f };
        back[i] = (CF_V3){ corners[i].x, corners[i].y, -DEPTH * .5f };
    }
    quad(mesh, front[0], front[1], front[2], front[3]);
    quad(mesh, back[3], back[2], back[1], back[0]);
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        quad(mesh, back[i], back[next], front[next], front[i]);
    }
}

static void stroke(render_mesh_asset *mesh, Point a, Point b)
{
    float dx = b.x - a.x, dy = b.y - a.y;
    float length = hypotf(dx, dy);
    assert(length > 0);
    float ux = dx / length * STROKE_WIDTH * .5f;
    float uy = dy / length * STROKE_WIDTH * .5f;
    Point corners[] = {
        { a.x - ux + uy, a.y - uy - ux },
        { b.x + ux + uy, b.y + uy - ux },
        { b.x + ux - uy, b.y + uy + ux },
        { a.x - ux - uy, a.y - uy + ux },
    };
    extrude(mesh, corners);
}

static float coordinate(char c)
{
    assert((c >= '0' && c <= '9') || c == 'a');
    return (float)(c == 'a' ? 10 : c - '0') * .1f;
}

static float letter(render_mesh_asset *mesh, char c, float x)
{
    if (c == ' ') {
        return .48f;
    }
    const Glyph *glyph = NULL;
    for (int i = 0; i < COUNT(glyphs); i++) {
        if (glyphs[i].letter == c) {
            glyph = &glyphs[i];
            break;
        }
    }
    assert(glyph);
    const char *path = glyph->paths;
    while (*path) {
        Point a = { x + coordinate(path[0]), coordinate(path[1]) };
        path += 2;
        while (*path && *path != ' ') {
            Point b = { x + coordinate(path[0]), coordinate(path[1]) };
            stroke(mesh, a, b);
            a = b;
            path += 2;
        }
        if (*path) {
            path++;
        }
    }
    return glyph->advance;
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
    for (int i = 0; i < acount(mesh->indices); i += 3) {
        const render_mesh_surface_vertex *a = &mesh->vertices[mesh->indices[i]];
        CF_V3 b = mesh->vertices[mesh->indices[i + 1]].position;
        CF_V3 c = mesh->vertices[mesh->indices[i + 2]].position;
        CF_V3 cross = cf_cross(cf_sub_v3(b, a->position), cf_sub_v3(c, a->position));
        assert(cf_dot_v3(cross, a->normal) > .000001f);
    }
}

static render_mesh_asset make_mesh(const char *text)
{
    render_mesh_asset mesh = { 0 };
    float x = 0;
    for (const char *c = text; *c; c++) {
        x += letter(&mesh, *c, x);
    }
    center(&mesh);
    verify(&mesh);
    assert(acount(mesh.vertices) % 24 == 0);
    assert(acount(mesh.indices) == acount(mesh.vertices) / 24 * 36);
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

static void dot(render_mesh_asset *mesh, float y)
{
    const float half = STROKE_WIDTH * .5f;
    const Point corners[] = {
        { -half, y - half },
        { half, y - half },
        { half, y + half },
        { -half, y + half },
    };
    extrude(mesh, corners);
}

/* Punctuation keeps the digits' cap frame of [-.5, .5]. Only its occupied
 * bounds are stored; centering or normalizing these meshes would misalign
 * the decimal point or turn the colon into full-height blocks. */
static void punctuation(const char *directory, const char *prototypes)
{
    render_mesh_asset mesh = { 0 };
    dot(&mesh, -.2f);
    dot(&mesh, .2f);
    bounds(&mesh);
    verify(&mesh);
    assert(fabsf(mesh.min.y + .26f) < .0001f);
    assert(fabsf(mesh.max.y - .26f) < .0001f);
    publish(&mesh, directory, prototypes, "time_colon");
    dot(&mesh, -.44f);
    bounds(&mesh);
    verify(&mesh);
    assert(fabsf(mesh.min.y + .5f) < .0001f);
    assert(fabsf(mesh.max.y + .38f) < .0001f);
    publish(&mesh, directory, prototypes, "time_decimal");
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
        "STILLWATER",
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

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--preview")) {
        preview(argv[2]);
        return 0;
    }
    if (argc >= 2 && !strcmp(argv[1], "--block")) {
        assert(argc <= 3);
        if (argc == 3) {
            assert(!chdir(argv[2]));
        }
        directory("content/meshes");
        block();
        return 0;
    }
    assert(argc <= 2);
    if (argc == 2) {
        assert(!chdir(argv[1]));
    }
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
    return 0;
}
