#define _XOPEN_SOURCE 700
#define _DARWIN_C_SOURCE
#define PICO_UNIT_IMPLEMENTATION
#include "pico_unit.h"
#include "yyjson.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Public Control values from the pinned SDK's runtime/components.h. */
enum
{
    MOUSE_LEFT = 1,
    KEY_SPACE = 4,
    KEY_ENTER = 9,
    KEY_BACKSPACE = 10,
    KEY_DOWN = 12,
    KEY_LEFT = 13,
    KEY_RIGHT = 14,
    KEY_A = 15,
    KEY_C = 17,
    KEY_E = 19,
    KEY_F = 20,
    KEY_G = 21,
    KEY_M = 27,
    KEY_P = 30,
    KEY_Q = 31,
    KEY_R = 32,
    KEY_V = 36,
    KEY_W = 37,
    KEY_X = 38,
    KEY_Z = 40,
    KEY_0 = 41,
    KEY_1 = 42,
    KEY_2 = 43,
    KEY_3 = 44,
    KEY_4 = 45,
    KEY_7 = 48,
    KEY_8 = 49,
    KEY_9 = 50
};

static char controller[PATH_MAX], project[PATH_MAX], temporary[PATH_MAX];
static volatile sig_atomic_t child_pid;
static yyjson_doc *reply;

static void timeout_handler(int signal_number)
{
    (void)signal_number;
    if (child_pid > 0) {
        kill(-child_pid, SIGKILL);
    }
    static const char message[] = "Stillwater tests exceeded their shared "
                                  "58-second deadline.\n";
    write(STDERR_FILENO, message, sizeof message - 1);
    _exit(124);
}

static void path_join(char *out, const char *base, const char *relative)
{
    int count = snprintf(out, PATH_MAX, "%s/%s", base, relative);
    assert(count > 0 && count < PATH_MAX);
}

static int run(char *const arguments[], const char *input, const char *output)
{
    pid_t pid = fork();
    assert(pid >= 0);
    if (!pid) {
        setpgid(0, 0);
        if (input) {
            int fd = open(input, O_RDONLY);
            if (fd < 0 || dup2(fd, STDIN_FILENO) < 0) {
                _exit(126);
            }
            close(fd);
        }
        if (output) {
            int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
            if (fd < 0 || dup2(fd, STDOUT_FILENO) < 0) {
                _exit(126);
            }
            close(fd);
        }
        execv(arguments[0], arguments);
        _exit(127);
    }
    child_pid = pid;
    setpgid(pid, pid);
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        assert(errno == EINTR);
    }
    child_pid = 0;
    return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}

static void copy(const char *source, const char *destination, bool clone)
{
#ifdef __APPLE__
    char *clone_arguments[] = { "/bin/cp", "-cR", (char *)source, (char *)destination, NULL };
#else
    char *clone_arguments[] = { "/bin/cp",      "--reflink=auto",    "-R",
                                (char *)source, (char *)destination, NULL };
#endif
    char *arguments[] = { "/bin/cp", "-R", (char *)source, (char *)destination, NULL };
    assert(run(clone ? clone_arguments : arguments, NULL, NULL) == 0);
}

static void prepare()
{
    char scratch[PATH_MAX];
    path_join(scratch, project, "build/resonance");
    assert(mkdir(scratch, 0755) == 0 || errno == EEXIST);
    path_join(temporary, scratch, "stillwater-tests-XXXXXX");
    assert(mkdtemp(temporary));
    char source[PATH_MAX], destination[PATH_MAX];
    path_join(source, project, "project.kdl");
    path_join(destination, temporary, "project.kdl");
    copy(source, destination, false);
    path_join(source, project, "content");
    path_join(destination, temporary, "content");
    copy(source, destination, true);
    path_join(source, project, "tests/fixtures/content/.");
    copy(source, destination, false);
}

static yyjson_mut_doc *request(const char *scene, int frames)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    assert(doc);
    yyjson_mut_val *root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);
    yyjson_mut_obj_add_int(doc, root, "contract", 1);
    yyjson_mut_obj_add_str(doc, root, "operation", "runtime_snapshot");
    yyjson_mut_obj_add_str(doc, root, "kind", "scene");
    yyjson_mut_obj_add_str(doc, root, "name", scene);
    yyjson_mut_obj_add_int(doc, root, "frames", frames);
    yyjson_mut_obj_add_int(doc, root, "entity_limit", 256);
    yyjson_mut_obj_add_arr(doc, root, "input_timeline");
    return doc;
}

static yyjson_mut_val *sample_input(yyjson_mut_doc *doc, int frame)
{
    yyjson_mut_val *timeline = yyjson_mut_obj_get(yyjson_mut_doc_get_root(doc), "input_timeline");
    yyjson_mut_val *sample = yyjson_mut_arr_add_obj(doc, timeline);
    yyjson_mut_obj_add_int(doc, sample, "frame", frame);
    return yyjson_mut_obj_add_obj(doc, sample, "input");
}

static void key(yyjson_mut_doc *doc, int frame, int code)
{
    yyjson_mut_val *input = sample_input(doc, frame);
    yyjson_mut_val *controls = yyjson_mut_obj_add_arr(doc, input, "controls");
    if (code) {
        yyjson_mut_arr_add_int(doc, controls, code);
    }
}

static void pads(yyjson_mut_doc *doc, int frame, bool second_connected, int code)
{
    int slot = 0;
    const char *button = NULL;
    switch (code) {
        case 0: break;
        case KEY_LEFT: button = "Dpad Left"; break;
        case KEY_RIGHT: button = "Dpad Right"; break;
        case KEY_DOWN: button = "Dpad Down"; break;
        case KEY_SPACE: button = "Dpad Up"; break;
        case KEY_X: button = "A"; break;
        case KEY_Z: button = "B"; break;
        case KEY_C: button = "Left Shoulder"; break;
        case KEY_A:
            slot = 1;
            button = "Dpad Left";
            break;
        case KEY_W:
            slot = 1;
            button = "Dpad Up";
            break;
        case KEY_E:
            slot = 1;
            button = "A";
            break;
        case KEY_Q:
            slot = 1;
            button = "B";
            break;
        case KEY_R:
            slot = 1;
            button = "Left Shoulder";
            break;
        default: assert(false);
    }
    yyjson_mut_val *input = sample_input(doc, frame);
    yyjson_mut_obj_add_arr(doc, input, "controls");
    yyjson_mut_val *controllers = yyjson_mut_obj_add_arr(doc, input, "controllers");
    for (int i = 0; i < 2; ++i) {
        yyjson_mut_val *pad = yyjson_mut_arr_add_obj(doc, controllers);
        yyjson_mut_obj_add_bool(doc, pad, "connected", i == 0 || second_connected);
        yyjson_mut_val *buttons = yyjson_mut_obj_add_arr(doc, pad, "buttons");
        if (button && i == slot) {
            yyjson_mut_arr_add_str(doc, buttons, button);
        }
    }
}

static void tap(yyjson_mut_doc *doc, int frame, int code)
{
    key(doc, frame, code);
    key(doc, frame + 1, 0);
}

static void pointer_world(yyjson_mut_doc *doc, int frame, double x, double y)
{
    yyjson_mut_val *input = sample_input(doc, frame);
    yyjson_mut_val *controls = yyjson_mut_obj_add_arr(doc, input, "controls");
    yyjson_mut_arr_add_int(doc, controls, MOUSE_LEFT);
    yyjson_mut_val *pointer = yyjson_mut_obj_add_obj(doc, input, "pointer");
    yyjson_mut_val *origin = yyjson_mut_obj_add_obj(doc, pointer, "origin");
    yyjson_mut_obj_add_real(doc, origin, "x", x);
    yyjson_mut_obj_add_real(doc, origin, "y", y);
    yyjson_mut_obj_add_real(doc, origin, "z", 1080);
    yyjson_mut_val *direction = yyjson_mut_obj_add_obj(doc, pointer, "direction");
    yyjson_mut_obj_add_real(doc, direction, "x", 0);
    yyjson_mut_obj_add_real(doc, direction, "y", 0);
    yyjson_mut_obj_add_real(doc, direction, "z", -1);
}

static void click_world(yyjson_mut_doc *doc, int frame, double x, double y)
{
    pointer_world(doc, frame, x, y);
    key(doc, frame + 1, 0);
}

static void device_sample(
    yyjson_mut_doc *doc,
    yyjson_mut_val *input,
    int width,
    int height,
    bool active,
    int status,
    double tilt,
    bool haptics
)
{
    yyjson_mut_val *device = yyjson_mut_obj_add_obj(doc, input, "device");
    yyjson_mut_obj_add_int(doc, device, "width", width);
    yyjson_mut_obj_add_int(doc, device, "height", height);
    yyjson_mut_obj_add_bool(doc, device, "touch", true);
    yyjson_mut_obj_add_bool(doc, device, "active", active);
    yyjson_mut_obj_add_int(doc, device, "motion_status", status);
    yyjson_mut_obj_add_bool(doc, device, "haptics", haptics);
    yyjson_mut_val *angles = yyjson_mut_obj_add_obj(doc, device, "tilt");
    yyjson_mut_obj_add_real(doc, angles, "x", tilt);
    yyjson_mut_obj_add_real(doc, angles, "y", 0);
}

static void phone_timeline(yyjson_mut_doc *doc, int fallback)
{
    yyjson_mut_val *timeline = yyjson_mut_obj_get(yyjson_mut_doc_get_root(doc), "input_timeline");
    size_t i, count;
    yyjson_mut_val *sample;
    yyjson_mut_arr_foreach(timeline, i, count, sample)
    {
        int frame = yyjson_mut_get_int(yyjson_mut_obj_get(sample, "frame"));
        yyjson_mut_val *input = yyjson_mut_obj_get(sample, "input");
        if (fallback) {
            device_sample(doc, input, 390, 844, true, fallback, 0, false);
            continue;
        }
        int status = frame < 8 || frame >= 115 ? 0 : frame < 12 ? 1 : 2;
        device_sample(
            doc,
            input,
            frame < 150 ? 390 : 844,
            frame < 150 ? 844 : 390,
            frame < 134 || frame >= 140,
            status,
            frame >= 52 && frame < 60 ? -18 : 0,
            true
        );
    }
}

static bool snapshot(yyjson_mut_doc *doc)
{
    char input[PATH_MAX], output[PATH_MAX];
    path_join(input, temporary, "request.json");
    path_join(output, temporary, "reply.json");
    bool written = yyjson_mut_write_file(input, doc, 0, NULL, NULL);
    yyjson_mut_doc_free(doc);
    REQUIRE(written);
    char *arguments[] = { controller, "--project-root", temporary, NULL };
    int status = run(arguments, input, output);
    yyjson_doc_free(reply);
    reply = yyjson_read_file(output, 0, NULL, NULL);
    REQUIRE(reply);
    yyjson_val *root = yyjson_doc_get_root(reply);
    if (status || !yyjson_get_bool(yyjson_obj_get(root, "ok"))) {
        char *text = yyjson_val_write(root, YYJSON_WRITE_PRETTY, NULL);
        fprintf(stderr, "%s\n", text);
        free(text);
        return false;
    }
    yyjson_val *entities = yyjson_obj_get(yyjson_obj_get(root, "result"), "entities");
    size_t i, count;
    yyjson_val *entity;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        yyjson_val *behaviors = yyjson_obj_get(entity, "behaviors");
        size_t j, total;
        yyjson_val *behavior;
        yyjson_arr_foreach(behaviors, j, total, behavior)
        {
            yyjson_val *failure = yyjson_obj_get(behavior, "last_failure");
            if (yyjson_is_null(failure)) {
                continue;
            }
            char *text = yyjson_val_write(behavior, YYJSON_WRITE_PRETTY, NULL);
            fprintf(stderr, "%s\n", text);
            free(text);
            return false;
        }
    }
    return true;
}

static yyjson_val *entity_field(yyjson_val *entity, const char *component_name, const char *field_name)
{
    yyjson_val *components = yyjson_obj_get(entity, "components");
    size_t i, count;
    yyjson_val *component;
    yyjson_arr_foreach(components, i, count, component)
    {
        const char *name = yyjson_get_str(yyjson_obj_get(component, "name"));
        if (!name || strcmp(name, component_name)) {
            continue;
        }
        yyjson_val *fields = yyjson_obj_get(component, "fields");
        size_t j, total;
        yyjson_val *entry;
        yyjson_arr_foreach(fields, j, total, entry)
        {
            name = yyjson_get_str(yyjson_obj_get(entry, "name"));
            if (name && !strcmp(name, field_name)) {
                return yyjson_obj_get(entry, "value");
            }
        }
    }
    return NULL;
}

static yyjson_val *snapshot_entities()
{
    yyjson_val *result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    return yyjson_obj_get(result, "entities");
}

static yyjson_val *field(const char *owner, const char *component_name, const char *field_name)
{
    yyjson_val *entities = snapshot_entities();
    size_t i, count;
    yyjson_val *entity;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        const char *name = yyjson_get_str(yyjson_obj_get(entity, "name"));
        if (name && !strcmp(name, owner)) {
            return entity_field(entity, component_name, field_name);
        }
    }
    return NULL;
}

static bool visible_mesh(const char *source)
{
    yyjson_val *entities = snapshot_entities();
    size_t i, count;
    yyjson_val *entity;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        const char *mesh = yyjson_get_str(entity_field(entity, "MeshAsset", "source"));
        if (mesh && !strcmp(mesh, source) &&
            !yyjson_get_bool(entity_field(entity, "RenderState", "hidden"))) {
            return true;
        }
    }
    return false;
}

static bool read_number_meshes(double y, int side, char actual[17])
{
    struct
    {
        double x;
        char glyph;
    } glyphs[16];
    int glyph_count = 0;
    yyjson_val *entities = snapshot_entities();
    size_t i, count;
    yyjson_val *entity;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        const char *mesh = yyjson_get_str(entity_field(entity, "MeshAsset", "source"));
        yyjson_val *position = entity_field(entity, "Transform3D", "position");
        double actual_y = yyjson_get_num(yyjson_obj_get(position, "y"));
        double x = yyjson_get_num(yyjson_obj_get(position, "x"));
        if ((side < 0 && x >= 0) || (side > 0 && x <= 0)) {
            continue;
        }
        if (!mesh || actual_y < y - 0.001 || actual_y > y + 0.001 ||
            yyjson_get_bool(entity_field(entity, "RenderState", "hidden"))) {
            continue;
        }
        char glyph = 0;
        int digit;
        if (sscanf(mesh, "content/meshes/text/digit_%d.mesh", &digit) == 1 &&
            digit >= 0 && digit <= 9) {
            glyph = (char)('0' + digit);
        } else if (!strcmp(mesh, "content/meshes/text/time_colon.mesh")) {
            glyph = ':';
        } else if (!strcmp(mesh, "content/meshes/text/time_decimal.mesh")) {
            glyph = '.';
        }
        if (!glyph) {
            continue;
        }
        REQUIRE(glyph_count < 16);
        int at = glyph_count++;
        while (at && glyphs[at - 1].x > x) {
            glyphs[at] = glyphs[at - 1];
            at--;
        }
        glyphs[at].x = x;
        glyphs[at].glyph = glyph;
    }
    for (int j = 0; j < glyph_count; j++) {
        actual[j] = glyphs[j].glyph;
    }
    actual[glyph_count] = 0;
    return true;
}

static bool number_meshes(double y, int side, const char *expected)
{
    char actual[17];
    REQUIRE(read_number_meshes(y, side, actual));
    if (strcmp(actual, expected)) {
        fprintf(stderr, "Number mesh row at %.1f, side %d: expected %s, got %s\n", y, side, expected, actual);
    }
    REQUIRE(!strcmp(actual, expected));
    return true;
}

static bool no_decorative_hud()
{
    static const char *labels[] = { "stillwater", "journey", "endless",
                                    "sprint",     "duet",    "chapter" };
    for (int i = 0; i < (int)(sizeof labels / sizeof *labels); i++) {
        char mesh[128];
        snprintf(mesh, sizeof mesh, "content/meshes/text/%s.mesh", labels[i]);
        REQUIRE(!visible_mesh(mesh));
    }
    REQUIRE(number_meshes(264, 0, ""));
    return true;
}

static bool connection_depth()
{
    int plates = 0, text = 0;
    yyjson_val *entity;
    size_t i, count;
    yyjson_arr_foreach(snapshot_entities(), i, count, entity)
    {
        if (yyjson_get_bool(entity_field(entity, "RenderState", "hidden"))) {
            continue;
        }
        const char *mesh = yyjson_get_str(entity_field(entity, "MeshAsset", "source"));
        yyjson_val *position = entity_field(entity, "Transform3D", "position");
        double z = yyjson_get_num(yyjson_obj_get(position, "z"));
        if (mesh) {
            REQUIRE(z >= 70);
            text++;
            continue;
        }
        if (!yyjson_is_num(entity_field(entity, "Geometry3D", "depth"))) {
            continue;
        }
        double depth = yyjson_get_num(entity_field(entity, "Geometry3D", "depth"));
        REQUIRE(depth > 0 && z + depth * .5 < 69);
        plates++;
    }
    REQUIRE(plates >= 3 && text >= 4);
    REQUIRE(no_decorative_hud());
    return true;
}

static bool modal_depth()
{
    yyjson_val *entities = snapshot_entities();
    yyjson_val *entity, *panel = NULL;
    size_t i, count;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        if (yyjson_is_num(entity_field(entity, "Geometry3D", "depth")) &&
            !yyjson_is_str(entity_field(entity, "MeshAsset", "source")) &&
            yyjson_get_num(
                yyjson_obj_get(entity_field(entity, "Transform3D", "position"), "z")
            ) > 60 &&
            !yyjson_get_bool(entity_field(entity, "RenderState", "hidden"))) {
            REQUIRE(!panel);
            panel = entity;
        }
    }
    REQUIRE(panel);
    yyjson_val *position = entity_field(panel, "Transform3D", "position");
    yyjson_val *scale = entity_field(panel, "Transform3D", "scale");
    double center = yyjson_get_num(yyjson_obj_get(position, "z"));
    double half_depth = yyjson_get_num(entity_field(panel, "Geometry3D", "depth")) *
                        yyjson_get_num(yyjson_obj_get(scale, "z")) * 0.5;
    REQUIRE(half_depth > 0);
    int screen = yyjson_get_int(field("World", "Session", "screen"));
    bool connection = screen == 0;
    int foreground = 0, background = 0;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        const char *mesh = yyjson_get_str(entity_field(entity, "MeshAsset", "source"));
        if (!mesh || yyjson_get_bool(entity_field(entity, "RenderState", "hidden"))) {
            continue;
        }
        position = entity_field(entity, "Transform3D", "position");
        double y = yyjson_get_num(yyjson_obj_get(position, "y"));
        bool summary = screen == 3 && (y < 266.999 || y > 267.001) &&
                       (!strcmp(mesh, "content/meshes/text/score.mesh") ||
                        !strcmp(mesh, "content/meshes/text/lines.mesh") ||
                        !strcmp(mesh, "content/meshes/text/elapsed.mesh") ||
                        !strcmp(mesh, "content/meshes/text/best.mesh") ||
                        !strcmp(mesh, "content/meshes/text/no_best.mesh") ||
                        !strncmp(mesh, "content/meshes/text/digit_", sizeof "content/meshes/text/digit_" - 1) ||
                        !strncmp(mesh, "content/meshes/text/time_", sizeof "content/meshes/text/time_" - 1));
        bool modal = connection || summary ||
                     !strcmp(mesh, "content/meshes/text/paused.mesh") ||
                     !strcmp(mesh, "content/meshes/text/controller_lost.mesh") ||
                     !strcmp(mesh, "content/meshes/text/game_over.mesh") ||
                     !strcmp(mesh, "content/meshes/text/complete.mesh") ||
                     !strcmp(mesh, "content/meshes/text/press_enter.mesh") ||
                     !strcmp(mesh, "content/meshes/text/chapter_controls.mesh") ||
                     !strcmp(mesh, "content/meshes/text/play_controls.mesh") ||
                     !strcmp(mesh, "content/meshes/text/settings_controls.mesh") ||
                     !strcmp(mesh, "content/meshes/text/music.mesh");
        position = entity_field(entity, "Transform3D", "position");
        scale = entity_field(entity, "Transform3D", "scale");
        double z = yyjson_get_num(yyjson_obj_get(position, "z"));
        /* Glyph meshes have a 0.08-unit extrusion in tools/glyphs.c. */
        double glyph_half = 0.04 * yyjson_get_num(yyjson_obj_get(scale, "z")) *
                            yyjson_get_num(entity_field(entity, "Geometry3D", "depth"));
        if (modal) {
            REQUIRE(z - glyph_half > center + half_depth);
            foreground++;
        } else {
            if (z + glyph_half >= center - half_depth) {
                fprintf(stderr, "%s reaches in front of the modal panel\n", mesh);
            }
            REQUIRE(z + glyph_half < center - half_depth);
            bool setting = y > 266.999 && y < 267.001 &&
                           (!strncmp(mesh, "content/meshes/text/setting_", sizeof "content/meshes/text/setting_" - 1) ||
                            !strncmp(mesh, "content/meshes/text/digit_", sizeof "content/meshes/text/digit_" - 1));
            bool save_failed = !strcmp(mesh, "content/meshes/text/saving_failed.mesh") &&
                               yyjson_get_bool(field("World", "Session", "save_failed"));
            REQUIRE(setting || save_failed);
            double panel_y = yyjson_get_num(
                yyjson_obj_get(entity_field(panel, "Transform3D", "position"), "y")
            );
            double panel_half_height = yyjson_get_num(
                                           entity_field(panel, "Geometry3D", "height")
                                       ) *
                                       yyjson_get_num(
                                           yyjson_obj_get(entity_field(panel, "Transform3D", "scale"), "y")
                                       ) *
                                       .5;
            REQUIRE(y - yyjson_get_num(yyjson_obj_get(scale, "y")) * .5 > panel_y + panel_half_height);
            background++;
        }
    }
    REQUIRE(foreground >= 2);
    REQUIRE(!connection || background == 0);
    REQUIRE(no_decorative_hud());
    return true;
}

static bool stats_backings(int mode)
{
    yyjson_val *entities = snapshot_entities();
    size_t i, count;
    yyjson_val *entity;
    int stats = 0, timers = 0, hidden = 0;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        if (!yyjson_is_num(entity_field(entity, "Geometry3D", "width")) ||
            yyjson_is_str(entity_field(entity, "MeshAsset", "source"))) {
            continue;
        }
        yyjson_val *position = entity_field(entity, "Transform3D", "position");
        double x = yyjson_get_num(yyjson_obj_get(position, "x"));
        double y = yyjson_get_num(yyjson_obj_get(position, "y"));
        double z = yyjson_get_num(yyjson_obj_get(position, "z"));
        if (z > 60) {
            continue;
        }
        double width = yyjson_get_num(entity_field(entity, "Geometry3D", "width"));
        double height = yyjson_get_num(entity_field(entity, "Geometry3D", "height"));
        double depth = yyjson_get_num(entity_field(entity, "Geometry3D", "depth"));
        REQUIRE(width >= 160 && width <= 180);
        REQUIRE(depth > 0 && z + depth * 0.5 < 44);
        if (yyjson_get_bool(entity_field(entity, "RenderState", "hidden"))) {
            hidden++;
            continue;
        }
        if (y < -70) {
            REQUIRE(mode == 2 && x == -205);
            REQUIRE(y + height * 0.5 > -86 && y - height * 0.5 < -208);
            REQUIRE(height <= 150);
            timers++;
        } else {
            REQUIRE(x == (mode == 3 ? -459 + stats * 508 : -205));
            REQUIRE(y + height * 0.5 > 57 && y - height * 0.5 < -52);
            REQUIRE(height <= 140);
            stats++;
        }
    }
    REQUIRE(stats + timers + hidden == 3);
    if (yyjson_get_int(field("World", "Session", "screen")) != 1) {
        REQUIRE(stats == 0 && timers == 0 && hidden == 3);
        return true;
    }
    REQUIRE(stats == (mode == 3 ? 2 : 1));
    REQUIRE(timers == (mode == 2 ? 1 : 0));
    return true;
}

static bool active_hud()
{
    REQUIRE(no_decorative_hud());
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 1);
    static const char *essential[] = { "score", "lines", "hold", "next", "pause_hint" };
    for (int i = 0; i < (int)(sizeof essential / sizeof *essential); i++) {
        char mesh[128];
        snprintf(mesh, sizeof mesh, "content/meshes/text/%s.mesh", essential[i]);
        REQUIRE(visible_mesh(mesh));
    }
    REQUIRE(!visible_mesh("content/meshes/text/play_controls.mesh"));
    REQUIRE(!visible_mesh("content/meshes/text/settings_controls.mesh"));
    REQUIRE(!visible_mesh("content/meshes/text/connect_music.mesh"));
    REQUIRE(!visible_mesh("content/meshes/text/music.mesh"));
    yyjson_val *entities = snapshot_entities();
    size_t i, count;
    yyjson_val *entity;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        const char *mesh = yyjson_get_str(entity_field(entity, "MeshAsset", "source"));
        if (!mesh || yyjson_get_bool(entity_field(entity, "RenderState", "hidden"))) {
            continue;
        }
        REQUIRE(
            strncmp(mesh, "content/meshes/text/audio_", sizeof "content/meshes/text/audio_" - 1)
        );
        REQUIRE(
            strncmp(mesh, "content/meshes/text/music_", sizeof "content/meshes/text/music_" - 1)
        );
    }
    return true;
}

static bool report(int minimum_checks)
{
    yyjson_val *done = field("Report", "TestReport", "done");
    yyjson_val *checks = field("Report", "TestReport", "checks");
    yyjson_val *failures = field("Report", "TestReport", "failures");
    REQUIRE(yyjson_is_bool(done) && yyjson_get_bool(done));
    REQUIRE(yyjson_is_num(checks) && yyjson_get_int(checks) >= minimum_checks);
    REQUIRE(yyjson_is_num(failures));
    if (yyjson_get_int(failures)) {
        const char *message = yyjson_get_str(field("Report", "TestReport", "first_failure"));
        fprintf(stderr, "%d authored checks failed. First: %s\n", yyjson_get_int(failures), message);
    }
    REQUIRE(yyjson_get_int(failures) == 0);
    return true;
}

TEST_CASE(authored_rules)
{
    yyjson_mut_doc *doc = request("RulesChecks", 2);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(811));
    return true;
}

TEST_CASE(sidebar_previews_follow_play_state)
{
    yyjson_mut_doc *doc = request("PreviewChecks", 48);
    key(doc, 0, 0);
    tap(doc, 1, KEY_ENTER);
    tap(doc, 5, KEY_C);
    tap(doc, 10, KEY_P);
    tap(doc, 16, KEY_ENTER);
    tap(doc, 24, KEY_4);
    tap(doc, 27, KEY_C);
    tap(doc, 29, KEY_R);
    tap(doc, 35, KEY_P);
    tap(doc, 41, KEY_ENTER);
    REQUIRE(snapshot(doc));
    REQUIRE(report(16));
    return true;
}

static bool board_scenario(bool controllers)
{
    yyjson_mut_doc *doc = request("BoardChecks", 98);
    static const int events[][2] = {
        { 0, 0 },  { 2, KEY_LEFT },   { 3, 0 },  { 5, KEY_RIGHT },
        { 6, 0 },  { 8, KEY_X },      { 9, 0 },  { 10, KEY_Z },
        { 11, 0 }, { 12, KEY_C },     { 13, 0 }, { 14, KEY_C },
        { 15, 0 }, { 16, KEY_SPACE }, { 17, 0 }, { 18, KEY_C },
        { 19, 0 }, { 20, KEY_W },     { 21, 0 }, { 22, KEY_E },
        { 23, 0 }, { 24, KEY_Q },     { 25, 0 }, { 26, KEY_R },
        { 27, 0 }, { 28, KEY_A },     { 40, 0 }, { 42, KEY_RIGHT },
        { 61, 0 }, { 62, KEY_DOWN },  { 90, 0 }
    };
    for (size_t i = 0; i < sizeof events / sizeof events[0]; ++i) {
        if (controllers) {
            pads(doc, events[i][0], true, events[i][1]);
            continue;
        }
        key(doc, events[i][0], events[i][1]);
    }
    REQUIRE(snapshot(doc));
    REQUIRE(report(30));
    yyjson_val *result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    REQUIRE(yyjson_get_int(yyjson_obj_get(result, "live_entities")) == 632);
    return true;
}

TEST_CASE(live_keyboard_boards)
{
    return board_scenario(false);
}

TEST_CASE(live_controller_boards)
{
    return board_scenario(true);
}

TEST_CASE(lock_reset_limit)
{
    yyjson_mut_doc *doc = request("LockChecks", 78);
    key(doc, 0, 0);
    for (int frame = 2; frame <= 72; frame += 2) {
        tap(doc, frame, KEY_X);
    }
    REQUIRE(snapshot(doc));
    REQUIRE(report(5));
    return true;
}

TEST_CASE(topout_and_rescue)
{
    yyjson_mut_doc *doc = request("RescueChecks", 59);
    key(doc, 0, 0);
    for (int frame = 2; frame <= 50; frame += 2) {
        tap(doc, frame, KEY_SPACE);
    }
    REQUIRE(snapshot(doc));
    REQUIRE(report(3));
    return true;
}

TEST_CASE(journey_settings_and_reopen)
{
    yyjson_mut_doc *doc = request("ShellChecks", 275);
    key(doc, 0, 0);
    tap(doc, 24, KEY_P);
    tap(doc, 26, KEY_8);
    tap(doc, 29, KEY_8);
    tap(doc, 32, KEY_ENTER);
    tap(doc, 34, KEY_7);
    tap(doc, 52, KEY_P);
    tap(doc, 56, KEY_0);
    tap(doc, 58, KEY_V);
    tap(doc, 60, KEY_F);
    tap(doc, 62, KEY_G);
    tap(doc, 64, KEY_ENTER);
    tap(doc, 73, KEY_P);
    tap(doc, 75, KEY_7);
    tap(doc, 77, KEY_7);
    tap(doc, 79, KEY_8);
    tap(doc, 81, KEY_8);
    tap(doc, 83, KEY_8);
    tap(doc, 86, KEY_ENTER);
    tap(doc, 103, KEY_7);
    tap(doc, 270, KEY_7);
    REQUIRE(snapshot(doc));
    REQUIRE(report(22));
    yyjson_val *result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    yyjson_val *saved = yyjson_obj_get(result, "saved_state");
    REQUIRE(yyjson_is_obj(saved));
    doc = request("ReopenChecks", 8);
    yyjson_mut_val *initial = yyjson_val_mut_copy(doc, saved);
    REQUIRE(initial);
    yyjson_mut_obj_add_val(doc, yyjson_mut_doc_get_root(doc), "saved_state", initial);
    key(doc, 0, 0);
    key(doc, 2, KEY_P);
    key(doc, 3, KEY_7);
    key(doc, 4, 0);
    tap(doc, 5, KEY_ENTER);
    REQUIRE(snapshot(doc));
    REQUIRE(report(6));
    return true;
}

TEST_CASE(sprint_duet_and_disconnect)
{
    yyjson_mut_doc *doc = request("ModeChecks", 252);
    pads(doc, 0, true, 0);
    pads(doc, 30, false, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(14));
    return true;
}

TEST_CASE(music_choice_precedes_play)
{
    yyjson_mut_doc *doc = request("Stillwater", 18);
    key(doc, 0, 0);
    tap(doc, 3, KEY_SPACE);
    tap(doc, 6, KEY_P);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 0);
    REQUIRE(yyjson_get_int(field("World", "Session", "run")) == 0);
    REQUIRE(yyjson_get_num(field("World", "Session", "elapsed")) == 0);
    REQUIRE(yyjson_get_int(field("Board1", "BoardState", "pieces")) == 0);
    REQUIRE(yyjson_get_int(field("Board2", "BoardState", "pieces")) == 0);
    return true;
}

TEST_CASE(menu_ignores_keyboard_shortcuts)
{
    yyjson_mut_doc *doc = request("MenuShortcutChecks", 45);
    yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
    yyjson_mut_obj_add_str(doc, saved, "chapter", "3");
    key(doc, 0, 0);
    static const int shortcuts[] = {
        KEY_ENTER, KEY_M, KEY_1, KEY_2, KEY_3, KEY_4, KEY_V,         KEY_F,
        KEY_G,     KEY_9, KEY_0, KEY_7, KEY_8, KEY_P, KEY_BACKSPACE, KEY_SPACE,
    };
    for (int i = 0; i < (int)(sizeof shortcuts / sizeof *shortcuts); ++i) {
        tap(doc, 3 + i * 2, shortcuts[i]);
    }
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 0);
    REQUIRE(yyjson_get_int(field("World", "Session", "run")) == 0);
    REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 0);
    REQUIRE(yyjson_get_int(field("World", "Session", "stage")) == 3);
    REQUIRE(yyjson_get_int(field("World", "Session", "audio_source")) == 0);
    REQUIRE(!yyjson_get_bool(field("World", "Session", "audio_ready")));
    REQUIRE(!yyjson_get_bool(field("World", "Session", "entering")));
    REQUIRE(yyjson_get_int(field("World", "Session", "effects")) == 2);
    REQUIRE(!yyjson_get_bool(field("World", "Session", "reduced_motion")));
    REQUIRE(yyjson_get_bool(field("World", "Session", "ghost")));
    REQUIRE(yyjson_get_num(field("World", "Session", "master")) == 0.8);
    REQUIRE(report(12));
    return true;
}

TEST_CASE(loss_menu_retains_journey_and_audio)
{
    for (int source = 0; source < 2; ++source) {
        yyjson_mut_doc *doc = request("MenuRecoveryChecks", 120);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "chapter", "3");
        yyjson_mut_obj_add_str(doc, saved, "reduced_motion", "1");
        yyjson_mut_obj_add_str(doc, saved, "test_source", source ? "1" : "0");
        key(doc, 0, 0);
        tap(doc, 60, KEY_ENTER);
        click_world(doc, 70, -440, -86);
        tap(doc, 110, KEY_P);
        tap(doc, 114, KEY_ENTER);
        REQUIRE(snapshot(doc));
        REQUIRE(report(14));
        REQUIRE(!visible_mesh("content/meshes/text/game_over.mesh"));
    }
    return true;
}

TEST_CASE(pointer_play_preserves_returning_camera)
{
    yyjson_mut_doc *doc = request("CameraRecoveryChecks", 438);
    key(doc, 0, 0);
    click_world(doc, 205, -440, -86);
    REQUIRE(snapshot(doc));
    REQUIRE(report(10));
    return true;
}

TEST_CASE(music_connection_controls)
{
    yyjson_mut_doc *doc = request("MusicOnboardingChecks", 375);
    key(doc, 0, 0);
    tap(doc, 3, KEY_SPACE);
    tap(doc, 40, KEY_SPACE);
    click_world(doc, 130, -440, -86);
    tap(doc, 360, KEY_P);
    tap(doc, 365, KEY_M);
    REQUIRE(snapshot(doc));
    REQUIRE(report(24));
    return true;
}

TEST_CASE(music_choices_use_world_space_pointer)
{
    static const struct
    {
        double x, y;
        int source;
        bool starts;
    } choices[] = {
        { -440, -86, 0, true },
        { -440, 135, 1, false },
        { 600, 24, 0, false },
    };
    for (int i = 0; i < (int)(sizeof choices / sizeof *choices); i++) {
        yyjson_mut_doc *doc = request("Stillwater", 18);
        key(doc, 0, 0);
        click_world(doc, 12, choices[i].x, choices[i].y);
        REQUIRE(snapshot(doc));
        REQUIRE(
            yyjson_get_int(field("World", "Session", "audio_source")) ==
            choices[i].source
        );
        REQUIRE(
            yyjson_get_bool(field("World", "Session", "audio_ready")) ==
            choices[i].starts
        );
        REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 0);
        REQUIRE(yyjson_get_int(field("World", "Session", "run")) == 0);
        REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 0);
        REQUIRE(
            yyjson_get_bool(field("World", "Session", "entering")) == choices[i].starts
        );
    }
    return true;
}

TEST_CASE(attract_placement_rules)
{
    yyjson_mut_doc *doc = request("AttractRulesChecks", 2);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(13));
    return true;
}

TEST_CASE(attract_play_handoff)
{
    yyjson_mut_doc *doc = request("AttractChecks", 385);
    key(doc, 0, 0);
    tap(doc, 100, KEY_3);
    click_world(doc, 140, -440, -86);
    tap(doc, 337, KEY_SPACE);
    REQUIRE(snapshot(doc));
    REQUIRE(report(31));
    return true;
}

TEST_CASE(attract_reduced_motion)
{
    yyjson_mut_doc *doc = request("Stillwater", 44);
    yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
    yyjson_mut_obj_add_str(doc, saved, "reduced_motion", "1");
    key(doc, 0, 0);
    click_world(doc, 6, -367, 11);
    click_world(doc, 12, -440, -86);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_bool(field("World", "Session", "reduced_motion")));
    REQUIRE(!yyjson_get_bool(field("World", "Session", "attract")));
    REQUIRE(!yyjson_get_bool(field("World", "Session", "entering")));
    REQUIRE(yyjson_get_int(field("World", "Session", "run")) == 1);
    REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 0);
    REQUIRE(yyjson_get_num(field("World", "Session", "hud_alpha")) == 1);
    REQUIRE(
        yyjson_get_num(yyjson_obj_get(field("Camera", "Transform3D", "position"), "x")) == 90
    );
    REQUIRE(!visible_mesh("content/meshes/text/music_score.mesh"));
    return true;
}

TEST_CASE(attract_audio_failure_recovery)
{
    yyjson_mut_doc *doc = request("AttractRecoveryChecks", 253);
    key(doc, 0, 0);
    click_world(doc, 5, -440, -86);
    click_world(doc, 20, -440, -86);
    REQUIRE(snapshot(doc));
    REQUIRE(report(10));
    return true;
}

TEST_CASE(compact_opening_pointer_controls)
{
    yyjson_mut_doc *doc = request("UiChecks", 82);
    key(doc, 0, 0);
    click_world(doc, 6, -440, 135);
    click_world(doc, 10, -367, 11);
    click_world(doc, 14, -440, -36);
    click_world(doc, 18, -378, -80);
    click_world(doc, 22, -378, -118);
    click_world(doc, 26, -378, -156);
    click_world(doc, 30, -447, -194);
    click_world(doc, 34, -316, -194);
    click_world(doc, 38, -440, -36);
    click_world(doc, 42, -378, -118);
    click_world(doc, 46, -440, -86);
    REQUIRE(snapshot(doc));
    REQUIRE(report(25));
    return true;
}

TEST_CASE(score_choice_and_mode_keys)
{
    /* A pooled cue stays dormant until its event payload is configured. */
    yyjson_mut_doc *doc = request("ReactionField", 2);
    yyjson_mut_val *kind = yyjson_mut_obj_get(yyjson_mut_doc_get_root(doc), "kind");
    REQUIRE(yyjson_mut_set_str(kind, "prototype"));
    REQUIRE(snapshot(doc));
    yyjson_val *result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    REQUIRE(yyjson_get_int(yyjson_obj_get(result, "particle_count")) == 0);
    yyjson_val *cue = yyjson_arr_get(snapshot_entities(), 0);
    REQUIRE(yyjson_get_int(entity_field(cue, "ParticleField3D", "count")) == 0);

    doc = request("Stillwater", 250);
    key(doc, 0, 0);
    click_world(doc, 6, -440, -86);
    tap(doc, 230, KEY_SPACE);
    tap(doc, 234, KEY_P);
    tap(doc, 238, KEY_ENTER);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 1);
    REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 0);
    REQUIRE(active_hud());
    REQUIRE(yyjson_get_int(field("World", "Session", "run")) == 1);
    REQUIRE(yyjson_get_bool(field("Board1", "BoardState", "ready")));
    REQUIRE(yyjson_get_int(field("Board1", "BoardState", "pieces")) == 1);
    result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    REQUIRE(yyjson_arr_size(yyjson_obj_get(result, "ui")) == 0);
    int entities = yyjson_get_int(yyjson_obj_get(result, "live_entities"));
    int particles = yyjson_get_int(yyjson_obj_get(result, "particle_count"));
    REQUIRE(particles == 0);
    REQUIRE(yyjson_get_int(field("Vortex", "ParticleField3D", "count")) == 131072);
    REQUIRE(yyjson_get_int(field("Stars", "ParticleField3D", "count")) == 16384);
    REQUIRE(yyjson_get_num(field("Vortex", "ParticleField3D", "time")) > 0);
    REQUIRE(yyjson_get_int(field("World", "ReactionQueue", "dropped")) == 0);
    REQUIRE(yyjson_get_int(field("Board1", "ReactionQueue", "dropped")) == 0);
    doc = request("Stillwater", 250);
    key(doc, 0, 0);
    click_world(doc, 6, -440, -86);
    tap(doc, 230, KEY_2);
    tap(doc, 234, KEY_3);
    tap(doc, 238, KEY_4);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 1);
    REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 3);
    REQUIRE(active_hud());
    REQUIRE(yyjson_get_int(field("World", "Session", "run")) == 4);
    REQUIRE(yyjson_get_bool(field("Board2", "BoardState", "ready")));
    result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    REQUIRE(yyjson_get_int(yyjson_obj_get(result, "live_entities")) == entities);
    static const char *quiet_settings[] = { "effects", "reduced_motion" };
    for (int i = 0; i < 2; ++i) {
        doc = request("Stillwater", 250);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, quiet_settings[i], i == 0 ? "0" : "1");
        key(doc, 0, 0);
        click_world(doc, 6, -440, -86);
        tap(doc, 230, KEY_SPACE);
        REQUIRE(snapshot(doc));
        result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
        REQUIRE(yyjson_get_int(yyjson_obj_get(result, "particle_count")) == 0);
        REQUIRE(yyjson_get_num(field("Vortex", "ParticleField3D", "time")) == 0);
        REQUIRE(
            yyjson_get_int(field("Vortex", "ParticleField3D", "count")) ==
            (i == 0 ? 0 : 131072)
        );
        REQUIRE(
            yyjson_get_int(field("Stars", "ParticleField3D", "count")) ==
            (i == 0 ? 512 : 16384)
        );
        REQUIRE(yyjson_get_int(yyjson_obj_get(result, "live_entities")) == entities);
        REQUIRE(yyjson_get_int(field("Board1", "BoardState", "pieces")) == 1);
    }
    return true;
}

TEST_CASE(final_clear_wins_over_blocked_spawn)
{
    static const char *modes[] = { "0", "2", "3" };
    for (int i = 0; i < 3; ++i) {
        yyjson_mut_doc *doc = request("CompletionChecks", 25);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "chapter", "5");
        yyjson_mut_obj_add_str(doc, saved, "test_mode", modes[i]);
        key(doc, 0, 0);
        REQUIRE(snapshot(doc));
        REQUIRE(report(3));
        REQUIRE(yyjson_get_bool(field("World", "Session", "won")));
        int mode = yyjson_get_int(field("World", "Session", "mode"));
        REQUIRE(visible_mesh("content/meshes/text/complete.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/play_controls.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/settings_controls.mesh"));
        if (mode == 2) {
            REQUIRE(visible_mesh("content/meshes/text/elapsed.mesh"));
            REQUIRE(visible_mesh("content/meshes/text/best.mesh"));
            REQUIRE(!visible_mesh("content/meshes/text/score.mesh"));
            int tenths = (int)(yyjson_get_num(field("World", "Session", "elapsed")) * 10);
            char expected[17];
            snprintf(expected, sizeof expected, "%d:%02d.%d", tenths / 600, tenths / 10 % 60, tenths % 10);
            REQUIRE(number_meshes(20, -1, expected));
            REQUIRE(number_meshes(20, 1, expected));
        } else {
            REQUIRE(visible_mesh("content/meshes/text/score.mesh"));
            REQUIRE(visible_mesh("content/meshes/text/lines.mesh"));
            REQUIRE(!visible_mesh("content/meshes/text/elapsed.mesh"));
            REQUIRE(!visible_mesh("content/meshes/text/best.mesh"));
            if (mode == 3) {
                REQUIRE(number_meshes(24, -1, "1000"));
                REQUIRE(number_meshes(24, 1, "0"));
                REQUIRE(number_meshes(-46, -1, "20"));
                REQUIRE(number_meshes(-46, 1, "0"));
            } else {
                REQUIRE(number_meshes(18, -1, "1000"));
                REQUIRE(number_meshes(18, 1, "12"));
            }
        }
        REQUIRE(stats_backings(mode));
        REQUIRE(modal_depth());
    }
    return true;
}

TEST_CASE(sprint_time_display)
{
    yyjson_mut_doc *doc = request("HudChecks", 18);
    yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
    yyjson_mut_obj_add_str(doc, saved, "sprint", "123.4");
    key(doc, 0, 0);
    tap(doc, 3, KEY_3);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 2);
    REQUIRE(active_hud());
    REQUIRE(visible_mesh("content/meshes/text/elapsed.mesh"));
    REQUIRE(visible_mesh("content/meshes/text/best.mesh"));
    REQUIRE(!visible_mesh("content/meshes/text/no_best.mesh"));
    REQUIRE(number_meshes(-195, 0, "2:03.4"));
    char timer[17];
    REQUIRE(read_number_meshes(-126, 0, timer));
    int minutes, seconds, tenths, end = 0;
    REQUIRE(sscanf(timer, "%d:%d.%d%n", &minutes, &seconds, &tenths, &end) == 3);
    REQUIRE(timer[end] == 0 && minutes == 0 && seconds == 0 && tenths >= 0 && tenths < 10);
    char expected[17];
    snprintf(expected, sizeof expected, "%d:%02d.%d", minutes, seconds, tenths);
    REQUIRE(!strcmp(timer, expected));
    double current = yyjson_get_num(field("World", "Session", "elapsed"));
    REQUIRE(current > 0 && current < 1);
    /* A tenth-second readout refreshed at 10 Hz can trail by under 0.2 s. */
    REQUIRE(current >= tenths * .1 && current - tenths * .1 < .2);
    REQUIRE(stats_backings(2));

    double elapsed = 0;
    for (int i = 0; i < 2; i++) {
        yyjson_mut_doc *doc = request("HudChecks", i == 0 ? 48 : 90);
        yyjson_mut_val *root = yyjson_mut_doc_get_root(doc);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, root, "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "sprint", "123.4");
        key(doc, 0, 0);
        tap(doc, 3, KEY_3);
        tap(doc, 30, KEY_P);
        tap(doc, 34, KEY_7);
        tap(doc, 36, KEY_8);
        REQUIRE(snapshot(doc));
        REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 2);
        REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 2);
        REQUIRE(yyjson_get_num(field("World", "Session", "sprint_best")) == 123.4);
        double current = yyjson_get_num(field("World", "Session", "elapsed"));
        REQUIRE(current > 0 && current < 1);
        if (i == 0) {
            elapsed = current;
        } else {
            REQUIRE(current == elapsed);
        }
        REQUIRE(!visible_mesh("content/meshes/text/elapsed.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/best.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/no_best.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/chapter_controls.mesh"));
        REQUIRE(number_meshes(-126, 0, ""));
        REQUIRE(number_meshes(-195, 0, ""));
        REQUIRE(stats_backings(2));
        REQUIRE(modal_depth());
    }
    doc = request("HudChecks", 18);
    key(doc, 0, 0);
    tap(doc, 3, KEY_3);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_num(field("World", "Session", "sprint_best")) == 0);
    REQUIRE(visible_mesh("content/meshes/text/no_best.mesh"));
    REQUIRE(active_hud());
    REQUIRE(stats_backings(2));
    doc = request("HudChecks", 18);
    saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
    yyjson_mut_obj_add_str(doc, saved, "chapter", "3");
    key(doc, 0, 0);
    tap(doc, 3, KEY_P);
    tap(doc, 5, KEY_7);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "stage")) == 2);
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 2);
    REQUIRE(visible_mesh("content/meshes/text/chapter_controls.mesh"));
    REQUIRE(!visible_mesh("content/meshes/text/elapsed.mesh"));
    REQUIRE(!visible_mesh("content/meshes/text/best.mesh"));
    REQUIRE(stats_backings(0));
    REQUIRE(modal_depth());
    doc = request("HudChecks", 18);
    key(doc, 0, 0);
    tap(doc, 3, KEY_4);
    tap(doc, 8, KEY_P);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 3);
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 2);
    REQUIRE(stats_backings(3));
    REQUIRE(modal_depth());
    return true;
}

TEST_CASE(audio_source_status_labels)
{
    static const struct
    {
        const char *source, *status, *signal, *quiet, *action, *label;
    } states[] = {
        { "0", "0", "0", "0", "ui_connect", NULL },
        { "1", "1", "0", "0", "ui_connect", "ui_waiting" },
        { "1", "2", "0", "0", "ui_connect", "ui_connected" },
        { "1", "2", "1", "0", "ui_connect", "ui_connected" },
        { "1", "2", "0", "1", "ui_connect", "ui_quiet" },
        { "1", "3", "0", "0", "ui_retry", "ui_unavailable" },
        { "1", "4", "0", "0", "ui_retry", "ui_cancelled" },
        { "1", "5", "0", "0", "ui_retry", "ui_ended" },
    };
    for (int i = 0; i < (int)(sizeof states / sizeof *states); i++) {
        yyjson_mut_doc *doc = request("AudioHudChecks", 18);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "audio_source", states[i].source);
        yyjson_mut_obj_add_str(doc, saved, "audio_status", states[i].status);
        yyjson_mut_obj_add_str(doc, saved, "audio_signal", states[i].signal);
        yyjson_mut_obj_add_str(doc, saved, "audio_quiet", states[i].quiet);
        yyjson_mut_obj_add_str(doc, saved, "screen", "0");
        REQUIRE(snapshot(doc));
        char mesh[128];
        snprintf(mesh, sizeof mesh, "content/meshes/text/%s.mesh", states[i].action);
        REQUIRE(visible_mesh(mesh));
        if (states[i].label) {
            snprintf(mesh, sizeof mesh, "content/meshes/text/%s.mesh", states[i].label);
            REQUIRE(visible_mesh(mesh));
        }
        REQUIRE(!visible_mesh("content/meshes/text/music_youtube.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/music_spotify.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/music_tidal.mesh"));
        REQUIRE(visible_mesh("content/meshes/text/ui_play.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/play_controls.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/score.mesh"));
        REQUIRE(stats_backings(0));
        REQUIRE(connection_depth());
    }
    for (int capture = 0; capture < 2; capture++) {
        yyjson_mut_doc *doc = request("HudChecks", 18);
        key(doc, 0, 0);
        if (capture) {
            tap(doc, 3, KEY_M);
        }
        if (capture) {
            click_world(doc, 16, 0, 135);
        } else {
            tap(doc, 16, KEY_M);
        }
        REQUIRE(snapshot(doc));
        REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 0);
        REQUIRE(!yyjson_get_bool(field("World", "Session", "audio_ready")));
        REQUIRE(yyjson_get_int(field("World", "Session", "audio_source")) == 1);
        REQUIRE(yyjson_get_int(field("World", "Session", "audio_status")) == 1);
        REQUIRE(!yyjson_get_bool(field("World", "Session", "audio_signal")));
        REQUIRE(!yyjson_get_bool(field("World", "Session", "audio_quiet")));
        REQUIRE(yyjson_get_num(field("World", "VisualState", "energy")) == 0);
        REQUIRE(visible_mesh("content/meshes/text/ui_waiting.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/pause_hint.mesh"));
    }
    return true;
}

TEST_CASE(setting_value_labels)
{
    static const struct
    {
        int key;
        const char *label;
    } settings[] = {
        { KEY_V, "content/meshes/text/setting_effects_off.mesh" },
        { KEY_F, "content/meshes/text/setting_motion_on.mesh" },
        { KEY_G, "content/meshes/text/setting_ghost_off.mesh" },
        { KEY_0, "content/meshes/text/setting_volume.mesh" },
    };
    for (int i = 0; i < (int)(sizeof settings / sizeof *settings); i++) {
        yyjson_mut_doc *doc = request("HudChecks", 18);
        key(doc, 0, 0);
        tap(doc, 16, settings[i].key);
        REQUIRE(snapshot(doc));
        REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 1);
        REQUIRE(visible_mesh(settings[i].label));
        if (settings[i].key == KEY_0) {
            REQUIRE(number_meshes(267, 0, "90"));
            REQUIRE(visible_mesh("content/meshes/text/setting_percent.mesh"));
        }
    }
    for (int maximum = 0; maximum <= 1; maximum++) {
        yyjson_mut_doc *doc = request("HudChecks", 18);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "master", maximum ? "1" : "0");
        key(doc, 0, 0);
        tap(doc, 16, maximum ? KEY_0 : KEY_9);
        REQUIRE(snapshot(doc));
        REQUIRE(yyjson_get_num(field("World", "Session", "master")) == maximum);
        REQUIRE(visible_mesh("content/meshes/text/setting_volume.mesh"));
        REQUIRE(number_meshes(267, 0, maximum ? "100" : "0"));
        REQUIRE(visible_mesh("content/meshes/text/setting_percent.mesh"));
    }
    yyjson_mut_doc *doc = request("HudChecks", 18);
    key(doc, 0, 0);
    tap(doc, 3, KEY_P);
    tap(doc, 16, KEY_0);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 2);
    REQUIRE(visible_mesh("content/meshes/text/setting_volume.mesh"));
    REQUIRE(visible_mesh("content/meshes/text/setting_percent.mesh"));
    REQUIRE(number_meshes(267, 0, "90"));
    REQUIRE(!visible_mesh("content/meshes/text/score.mesh"));
    REQUIRE(stats_backings(0));
    REQUIRE(modal_depth());

    doc = request("HudChecks", 180);
    key(doc, 0, 0);
    tap(doc, 16, KEY_F);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_bool(field("World", "Session", "reduced_motion")));
    REQUIRE(!visible_mesh("content/meshes/text/setting_motion_on.mesh"));
    return true;
}

TEST_CASE(immediate_session_labels)
{
    static const int keys[] = { KEY_1, KEY_2, KEY_3, KEY_4 };
    for (int mode = 0; mode < 4; mode++) {
        yyjson_mut_doc *doc = request("HudChecks", 18);
        key(doc, 0, 0);
        tap(doc, 16, keys[mode]);
        REQUIRE(snapshot(doc));
        REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == mode);
        REQUIRE(yyjson_get_int(field("World", "Session", "run")) == 2);
        REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 1);
        REQUIRE(active_hud());
    }
    return true;
}

/* Ordered player and session feedback must exist on the real authored board. */
TEST_CASE(ordered_player_reactions)
{
    yyjson_mut_doc *doc = request("ReactionChecks", 166);
    key(doc, 0, 0);
    yyjson_mut_val *input = sample_input(doc, 2);
    yyjson_mut_val *controls = yyjson_mut_obj_add_arr(doc, input, "controls");
    yyjson_mut_arr_add_int(doc, controls, KEY_LEFT);
    yyjson_mut_arr_add_int(doc, controls, KEY_X);
    yyjson_mut_arr_add_int(doc, controls, KEY_SPACE);
    key(doc, 3, 0);
    tap(doc, 4, KEY_C);
    tap(doc, 6, KEY_C);
    key(doc, 8, KEY_DOWN);
    input = sample_input(doc, 10);
    controls = yyjson_mut_obj_add_arr(doc, input, "controls");
    yyjson_mut_arr_add_int(doc, controls, KEY_DOWN);
    yyjson_mut_arr_add_int(doc, controls, KEY_X);
    yyjson_mut_arr_add_int(doc, controls, KEY_Z);
    key(doc, 11, KEY_DOWN);
    key(doc, 16, 0);
    key(doc, 18, KEY_RIGHT);
    key(doc, 21, 0);
    key(doc, 22, KEY_RIGHT);
    key(doc, 28, KEY_LEFT);
    key(doc, 30, KEY_RIGHT);
    key(doc, 32, 0);
    input = sample_input(doc, 33);
    controls = yyjson_mut_obj_add_arr(doc, input, "controls");
    yyjson_mut_arr_add_int(doc, controls, KEY_LEFT);
    yyjson_mut_arr_add_int(doc, controls, KEY_RIGHT);
    key(doc, 34, KEY_DOWN);
    key(doc, 134, 0);
    key(doc, 135, KEY_DOWN);
    key(doc, 160, 0);
    tap(doc, 163, KEY_LEFT);
    REQUIRE(snapshot(doc));
    REQUIRE(report(80));
    return true;
}

TEST_CASE(ordered_clear_reactions)
{
    yyjson_mut_doc *doc = request("ClearReactionChecks", 58);
    key(doc, 0, 0);
    static const int route[][2] = {
        { 2, KEY_RIGHT },  { 4, KEY_RIGHT },  { 6, KEY_RIGHT },
        { 8, KEY_RIGHT },  { 10, KEY_SPACE }, { 13, KEY_LEFT },
        { 15, KEY_LEFT },  { 17, KEY_LEFT },  { 19, KEY_LEFT },
        { 21, KEY_SPACE }, { 24, KEY_RIGHT }, { 26, KEY_SPACE },
        { 29, KEY_X },     { 31, KEY_X },     { 34, KEY_RIGHT },
        { 36, KEY_RIGHT }, { 38, KEY_SPACE }, { 41, KEY_Z },
        { 43, KEY_LEFT },  { 45, KEY_SPACE }, { 48, KEY_Z },
        { 50, KEY_LEFT },  { 52, KEY_LEFT },  { 54, KEY_SPACE }
    };
    for (size_t i = 0; i < sizeof route / sizeof *route; i++) {
        tap(doc, route[i][0], route[i][1]);
    }
    REQUIRE(snapshot(doc));
    REQUIRE(report(60));
    return true;
}

TEST_CASE(ordered_session_reactions)
{
    yyjson_mut_doc *doc = request("SessionReactionChecks", 52);
    key(doc, 0, 0);
    tap(doc, 2, KEY_P);
    yyjson_mut_val *input = sample_input(doc, 4);
    yyjson_mut_val *controls = yyjson_mut_obj_add_arr(doc, input, "controls");
    yyjson_mut_arr_add_int(doc, controls, KEY_V);
    yyjson_mut_arr_add_int(doc, controls, KEY_F);
    yyjson_mut_arr_add_int(doc, controls, KEY_G);
    yyjson_mut_arr_add_int(doc, controls, KEY_0);
    key(doc, 5, 0);
    tap(doc, 14, KEY_7);
    tap(doc, 16, KEY_ENTER);
    tap(doc, 18, KEY_P);
    tap(doc, 20, KEY_BACKSPACE);
    tap(doc, 22, KEY_2);
    tap(doc, 24, KEY_P);
    tap(doc, 26, KEY_8);
    tap(doc, 28, KEY_BACKSPACE);
    tap(doc, 30, KEY_4);
    REQUIRE(snapshot(doc));
    REQUIRE(report(80));
    return true;
}

TEST_CASE(authored_environment_consumer)
{
    yyjson_mut_doc *doc = request("ConsumerChecks", 44);
    REQUIRE(snapshot(doc));
    REQUIRE(report(64));
    REQUIRE(yyjson_get_int(field("World", "VisualState", "cue_overwrites")) == 0);
    return true;
}

TEST_CASE(controlled_soft_drop_and_piece_motion)
{
    yyjson_mut_doc *doc = request("MotionChecks", 746);
    key(doc, 0, 0);
    tap(doc, 28, KEY_LEFT);
    tap(doc, 30, KEY_RIGHT);
    tap(doc, 32, KEY_X);
    tap(doc, 34, KEY_Z);
    key(doc, 42, KEY_DOWN);
    key(doc, 72, 0);
    tap(doc, 77, KEY_SPACE);
    key(doc, 142, KEY_DOWN);
    key(doc, 147, 0);
    for (int frame = 155; frame <= 169; frame += 2) {
        tap(doc, frame, KEY_DOWN);
    }
    tap(doc, 175, KEY_SPACE);
    tap(doc, 220, KEY_SPACE);
    tap(doc, 270, KEY_SPACE);
    tap(doc, 278, KEY_SPACE);
    REQUIRE(snapshot(doc));
    REQUIRE(report(141));
    return true;
}

TEST_CASE(evolving_stage_direction)
{
    yyjson_mut_doc *doc = request("StageChecks", 126);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(66));
    return true;
}

TEST_CASE(layered_world_space_reactions)
{
    yyjson_mut_doc *doc = request("VfxChecks", 1147);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(216));
    REQUIRE(yyjson_get_int(field("World", "VisualState", "cue_dropped")) == 5);
    REQUIRE(yyjson_get_int(field("World", "VisualState", "cue_overwrites")) == 0);
    return true;
}

TEST_CASE(occupied_stack_releases_on_loss)
{
    for (int variant = 0; variant < 4; ++variant) {
        bool quiet = variant == 1 || variant == 3;
        yyjson_mut_doc *doc = request("LossChecks", quiet ? 100 : 315);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "test_quiet", quiet ? "1" : "0");
        yyjson_mut_obj_add_str(doc, saved, "reduced_motion", variant == 1 ? "1" : "0");
        yyjson_mut_obj_add_str(doc, saved, "effects", variant == 3 ? "0" : "2");
        yyjson_mut_obj_add_str(doc, saved, "test_duet", variant == 2 ? "1" : "0");
        key(doc, 0, 0);
        yyjson_mut_val *hold_input = sample_input(doc, 10);
        yyjson_mut_val *hold_controls = yyjson_mut_obj_add_arr(doc, hold_input, "controls");
        yyjson_mut_arr_add_int(doc, hold_controls, KEY_C);
        yyjson_mut_arr_add_int(doc, hold_controls, KEY_R);
        key(doc, 11, 0);
        for (int frame = 12; frame <= 60; frame += 2) {
            yyjson_mut_val *input = sample_input(doc, frame);
            yyjson_mut_val *controls = yyjson_mut_obj_add_arr(doc, input, "controls");
            yyjson_mut_arr_add_int(doc, controls, KEY_SPACE);
            yyjson_mut_arr_add_int(doc, controls, KEY_W);
            key(doc, frame + 1, 0);
        }
        if (!quiet) {
            tap(doc, 62, KEY_P);
            tap(doc, 64, KEY_ENTER);
            tap(doc, 66, KEY_BACKSPACE);
            tap(doc, 68, KEY_4);
            tap(doc, 70, KEY_M);
        }
        REQUIRE(snapshot(doc));
        REQUIRE(report(quiet ? 10 : 14));
        REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 0);
        REQUIRE(yyjson_get_bool(field("World", "Session", "attract")));
        REQUIRE(visible_mesh("content/meshes/text/ui_journey.mesh"));
        REQUIRE(visible_mesh("content/meshes/text/ui_play.mesh"));
        REQUIRE(!visible_mesh("content/meshes/text/game_over.mesh"));
    }
    yyjson_mut_doc *doc = request("LossChecks", 525);
    key(doc, 0, 0);
    tap(doc, 10, KEY_C);
    for (int frame = 12; frame <= 60; frame += 2) {
        tap(doc, frame, KEY_SPACE);
    }
    tap(doc, 290, KEY_ENTER);
    click_world(doc, 300, -440, -86);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 1);
    REQUIRE(yyjson_get_int(field("World", "Session", "run")) == 2);
    REQUIRE(!yyjson_get_bool(field("Board1", "BoardState", "topout")));
    REQUIRE(yyjson_get_int(field("Board1", "BoardState", "pieces")) == 0);
    REQUIRE(yyjson_get_int(field("Board1", "BoardState", "score")) == 0);
    REQUIRE(yyjson_get_int(field("Board1", "BoardState", "ghost_row")) >= 18);
    REQUIRE(!visible_mesh("content/meshes/text/game_over.mesh"));
    return true;
}

TEST_CASE(occupied_boards_depart_before_new_runs)
{
    for (int variant = 0; variant < 7; ++variant) {
        bool quiet = variant == 4 || variant == 5;
        yyjson_mut_doc *doc = request("TransitionChecks", quiet ? 73 : 253);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        char value[8];
        snprintf(value, sizeof value, "%d", variant);
        yyjson_mut_obj_add_strcpy(doc, saved, "test_transition", value);
        yyjson_mut_obj_add_str(doc, saved, "reduced_motion", variant == 4 ? "1" : "0");
        yyjson_mut_obj_add_str(doc, saved, "effects", variant == 5 ? "0" : "2");
        key(doc, 0, 0);
        for (int frame = 10; frame <= 14; frame += 4) {
            yyjson_mut_val *input = sample_input(doc, frame);
            yyjson_mut_val *controls = yyjson_mut_obj_add_arr(doc, input, "controls");
            yyjson_mut_arr_add_int(doc, controls, KEY_SPACE);
            yyjson_mut_arr_add_int(doc, controls, KEY_W);
            key(doc, frame + 1, 0);
        }
        if (variant == 0 || variant == 4) {
            tap(doc, 18, KEY_P);
        }
        tap(doc, 28, KEY_SPACE);
        if (variant < 4) {
            tap(doc, 200, KEY_SPACE);
        }
        REQUIRE(snapshot(doc));
        REQUIRE(report(variant == 6 ? 19 : 15));
    }
    return true;
}

TEST_CASE(journey_passage_preserves_the_board)
{
    for (int variant = 0; variant < 4; ++variant) {
        bool quiet = variant == 1 || variant == 2;
        yyjson_mut_doc *doc = request("JourneyChecks", quiet ? 58 : 213);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "reduced_motion", variant == 1 ? "1" : "0");
        yyjson_mut_obj_add_str(doc, saved, "effects", variant == 2 ? "0" : "2");
        yyjson_mut_obj_add_str(doc, saved, "test_audio_failure", variant == 3 ? "1" : "0");
        key(doc, 0, 0);
        tap(doc, 10, KEY_SPACE);
        tap(doc, 14, KEY_SPACE);
        if (!quiet) {
            tap(doc, 28, KEY_SPACE);
        }
        tap(doc, quiet ? 45 : 200, KEY_SPACE);
        REQUIRE(snapshot(doc));
        REQUIRE(report(variant == 3 ? 27 : 17));
    }
    return true;
}

TEST_CASE(authored_music_response)
{
    yyjson_mut_doc *doc = request("MusicResponseChecks", 1);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(56));
    return true;
}

TEST_CASE(musical_spatial_forces)
{
    yyjson_mut_doc *doc = request("MusicFieldChecks", 212);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(44));
    return true;
}

TEST_CASE(musical_character_and_sections)
{
    yyjson_mut_doc *doc = request("MusicCharacterChecks", 2);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(33));
    return true;
}

TEST_CASE(phone_control_rules)
{
    yyjson_mut_doc *doc = request("PhoneRulesChecks", 2);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(49));
    return true;
}

TEST_CASE(phone_device_and_touch_play)
{
    yyjson_mut_doc *doc = request("PhoneChecks", 228);
    yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
    yyjson_mut_obj_add_str(doc, saved, "reduced_motion", "1");
    key(doc, 0, 0);
    click_world(doc, 6, -95, -332);
    key(doc, 8, 0);
    click_world(doc, 10, 0, -404);
    key(doc, 12, 0);
    click_world(doc, 16, 0, -86);
    key(doc, 52, 0);
    key(doc, 60, 0);
    click_world(doc, 70, 120, -292);
    click_world(doc, 74, 120, -358);
    click_world(doc, 78, -120, -358);
    click_world(doc, 82, 0, -292);
    click_world(doc, 86, -120, -424);
    click_world(doc, 90, -120, -424);
    click_world(doc, 94, 0, -358);
    click_world(doc, 100, 0, -424);
    click_world(doc, 104, 0, -358);
    click_world(doc, 110, -95, -85);
    click_world(doc, 114, 95, -85);
    click_world(doc, 118, -95, 40);
    pointer_world(doc, 122, -120, -292);
    pointer_world(doc, 124, 0, -358);
    key(doc, 128, 0);
    key(doc, 134, 0);
    key(doc, 140, 0);
    click_world(doc, 142, -95, 40);
    key(doc, 150, 0);
    click_world(doc, 158, 400, -60);
    click_world(doc, 162, 95, 40);
    phone_timeline(doc, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(29));
    return true;
}

TEST_CASE(phone_expanded_options)
{
    yyjson_mut_doc *doc = request("PhoneMenuChecks", 102);
    key(doc, 0, 0);
    click_world(doc, 6, 0, -26);
    click_world(doc, 22, 62, -64);
    click_world(doc, 28, 62, -124);
    click_world(doc, 34, 62, -184);
    click_world(doc, 40, -15, -242);
    click_world(doc, 46, 126, -242);
    click_world(doc, 52, 0, -26);
    click_world(doc, 58, 62, -146);
    click_world(doc, 64, 0, -26);
    click_world(doc, 68, 0, -318);
    phone_timeline(doc, 3);
    REQUIRE(snapshot(doc));
    REQUIRE(report(20));
    return true;
}

TEST_CASE(phone_sprint_timers)
{
    yyjson_mut_doc *doc = request("HudChecks", 18);
    yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
    yyjson_mut_obj_add_str(doc, saved, "sprint", "123.4");
    key(doc, 0, 0);
    tap(doc, 3, KEY_3);
    phone_timeline(doc, 3);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_bool(field("World", "PhoneState", "portrait")));
    REQUIRE(yyjson_get_int(field("World", "Session", "mode")) == 2);
    REQUIRE(visible_mesh("content/meshes/text/elapsed.mesh"));
    REQUIRE(visible_mesh("content/meshes/text/best.mesh"));
    REQUIRE(!visible_mesh("content/meshes/text/no_best.mesh"));
    REQUIRE(number_meshes(375, 1, "2:03.4"));
    char timer[17];
    REQUIRE(read_number_meshes(375, -1, timer));
    int minutes, seconds, tenths, end = 0;
    REQUIRE(sscanf(timer, "%d:%d.%d%n", &minutes, &seconds, &tenths, &end) == 3);
    REQUIRE(timer[end] == 0 && minutes == 0 && seconds == 0 && tenths >= 0 && tenths < 10);
    double current = yyjson_get_num(field("World", "Session", "elapsed"));
    REQUIRE(current >= tenths * .1 && current - tenths * .1 < .2);
    doc = request("CompletionChecks", 25);
    key(doc, 0, 0);
    phone_timeline(doc, 3);
    REQUIRE(snapshot(doc));
    REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 3);
    REQUIRE(visible_mesh("content/meshes/text/elapsed.mesh"));
    REQUIRE(visible_mesh("content/meshes/text/best.mesh"));
    int finished = (int)(yyjson_get_num(field("World", "Session", "elapsed")) * 10);
    char expected[17];
    snprintf(expected, sizeof expected, "%d:%02d.%d", finished / 600, finished / 10 % 60, finished % 10);
    REQUIRE(number_meshes(100, -1, expected));
    REQUIRE(number_meshes(100, 1, expected));
    return true;
}

TEST_CASE(phone_permission_fallback)
{
    for (int status = 3; status <= 4; status++) {
        yyjson_mut_doc *doc = request("PhoneFallback", 5);
        key(doc, 0, 0);
        phone_timeline(doc, status);
        REQUIRE(snapshot(doc));
        REQUIRE(yyjson_get_bool(field("World", "PhoneState", "enabled")));
        REQUIRE(yyjson_get_int(field("World", "PhoneState", "motion_status")) == status);
        REQUIRE(report(2));
        doc = request("PhoneFallback", 50);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "reduced_motion", "1");
        key(doc, 0, 0);
        click_world(doc, 6, 0, -86);
        click_world(doc, 40, 0, -358);
        click_world(doc, 46, 0, -424);
        phone_timeline(doc, status);
        REQUIRE(snapshot(doc));
        REQUIRE(yyjson_get_int(field("Board2", "BoardState", "pieces")) == 0);
        REQUIRE(yyjson_get_int(field("Board1", "BoardState", "pieces")) == 1);
        REQUIRE(yyjson_get_int(field("World", "Session", "screen")) == 2);
        REQUIRE(!yyjson_get_bool(field("World", "PhoneState", "haptics")));
    }
    return true;
}

int main(int argc, char **argv)
{
    const char *selection = getenv("STILLWATER_REACTIONS") ? "--reactions" : "--all";
    const char *controller_path = getenv("STILLWATER_CTL");
    const char *project_path = getenv("STILLWATER_PROJECT");
    if (argc == 4) {
        selection = argv[1];
        controller_path = argv[2];
        project_path = argv[3];
    }
    bool all = !strcmp(selection, "--all");
    bool reactions = !strcmp(selection, "--reactions");
    bool consumer = !strcmp(selection, "--consumer");
    bool hud = !strcmp(selection, "--hud");
    bool onboarding = !strcmp(selection, "--onboarding");
    bool attract = !strcmp(selection, "--attract");
    bool ui = !strcmp(selection, "--ui");
    bool menu = !strcmp(selection, "--menu");
    bool menu_camera = !strcmp(selection, "--menu-camera");
    bool rules = !strcmp(selection, "--rules");
    bool previews = !strcmp(selection, "--previews");
    bool motion = !strcmp(selection, "--motion");
    bool stage = !strcmp(selection, "--stage");
    bool vfx = !strcmp(selection, "--vfx");
    bool loss = !strcmp(selection, "--loss");
    bool music_response = !strcmp(selection, "--music-response");
    bool music_field = !strcmp(selection, "--music-field");
    bool music_character = !strcmp(selection, "--music-character");
    bool phone = !strcmp(selection, "--phone");
    if ((argc != 1 && argc != 4) || !controller_path || !project_path ||
        !(all || reactions || consumer || hud || onboarding || attract || ui ||
          menu || menu_camera || rules || previews || motion || stage || vfx ||
          loss || music_response || music_field || music_character || phone)) {
        fprintf(
            stderr,
            "Usage: %s "
            "--all|--rules|--reactions|--consumer|--hud|--onboarding|--attract|"
            "--ui|--menu|--menu-camera|--"
            "previews|--motion|--stage|--vfx|--loss|--music-response|--music-"
            "field|--music-character|--phone "
            "/path/to/chibi_ctl /path/to/game\n",
            argv[0]
        );
        return 2;
    }
    assert(realpath(controller_path, controller) && realpath(project_path, project));
    signal(SIGALRM, timeout_handler);
    alarm(58);
    prepare();
    if (all || phone) {
        RUN_TEST_CASE(phone_control_rules);
        RUN_TEST_CASE(phone_device_and_touch_play);
        RUN_TEST_CASE(phone_permission_fallback);
        RUN_TEST_CASE(phone_expanded_options);
        RUN_TEST_CASE(phone_sprint_timers);
    }
    if (all || consumer || reactions) {
        RUN_TEST_CASE(authored_environment_consumer);
    }
    if (all || reactions) {
        RUN_TEST_CASE(ordered_player_reactions);
        RUN_TEST_CASE(ordered_session_reactions);
        RUN_TEST_CASE(ordered_clear_reactions);
        RUN_TEST_CASE(topout_and_rescue);
    }
    if (all || ui || menu) {
        RUN_TEST_CASE(compact_opening_pointer_controls);
        RUN_TEST_CASE(menu_ignores_keyboard_shortcuts);
        if (ui) {
            RUN_TEST_CASE(audio_source_status_labels);
        }
    }
    if (all || attract) {
        RUN_TEST_CASE(attract_placement_rules);
        RUN_TEST_CASE(attract_play_handoff);
        RUN_TEST_CASE(attract_reduced_motion);
        RUN_TEST_CASE(attract_audio_failure_recovery);
    }
    if (all || onboarding) {
        RUN_TEST_CASE(music_choice_precedes_play);
        RUN_TEST_CASE(music_connection_controls);
        RUN_TEST_CASE(music_choices_use_world_space_pointer);
    }
    if (all || rules) {
        RUN_TEST_CASE(authored_rules);
    }
    if (all) {
        RUN_TEST_CASE(live_keyboard_boards);
        RUN_TEST_CASE(live_controller_boards);
        RUN_TEST_CASE(lock_reset_limit);
        RUN_TEST_CASE(score_choice_and_mode_keys);
    }
    if (all || loss || menu) {
        RUN_TEST_CASE(loss_menu_retains_journey_and_audio);
    }
    if (all || menu || menu_camera) {
        RUN_TEST_CASE(pointer_play_preserves_returning_camera);
    }
    if (all || loss) {
        RUN_TEST_CASE(journey_settings_and_reopen);
        RUN_TEST_CASE(sprint_duet_and_disconnect);
    }
    if (all || hud || previews) {
        RUN_TEST_CASE(sidebar_previews_follow_play_state);
    }
    if (all || hud || loss) {
        RUN_TEST_CASE(final_clear_wins_over_blocked_spawn);
    }
    if (all || hud) {
        RUN_TEST_CASE(sprint_time_display);
        RUN_TEST_CASE(audio_source_status_labels);
        RUN_TEST_CASE(setting_value_labels);
        RUN_TEST_CASE(immediate_session_labels);
    }
    if (all || motion) {
        RUN_TEST_CASE(controlled_soft_drop_and_piece_motion);
    }
    if (all || stage) {
        RUN_TEST_CASE(evolving_stage_direction);
    }
    if (all || vfx) {
        RUN_TEST_CASE(layered_world_space_reactions);
    }
    if (all || loss) {
        RUN_TEST_CASE(occupied_stack_releases_on_loss);
        RUN_TEST_CASE(occupied_boards_depart_before_new_runs);
        RUN_TEST_CASE(journey_passage_preserves_the_board);
    }
    if (all || music_response) {
        RUN_TEST_CASE(authored_music_response);
    }
    if (all || music_field) {
        RUN_TEST_CASE(musical_spatial_forces);
    }
    if (all || music_character) {
        RUN_TEST_CASE(musical_character_and_sections);
    }
    pu_print_stats();
    yyjson_doc_free(reply);
    if (pu_test_failed()) {
        fprintf(stderr, "Failure artifacts retained in %s\n", temporary);
        return 1;
    }
    char *remove_arguments[] = { "/bin/rm", "-rf", temporary, NULL };
    assert(run(remove_arguments, NULL, NULL) == 0);
    alarm(0);
    return 0;
}
