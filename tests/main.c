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

/* Public GameControl values from the pinned SDK's runtime/components.h. */
enum
{
    KEY_SPACE = 4,
    KEY_DOWN = 12,
    KEY_LEFT = 13,
    KEY_RIGHT = 14,
    KEY_A = 15,
    KEY_C = 17,
    KEY_E = 19,
    KEY_Q = 31,
    KEY_R = 32,
    KEY_W = 37,
    KEY_X = 38,
    KEY_Z = 40
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
    strcpy(temporary, "/tmp/stillwater-tests-XXXXXX");
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
    yyjson_mut_obj_add_int(doc, root, "entity_limit", 4);
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

static yyjson_val *field(const char *owner, const char *component_name, const char *field_name)
{
    yyjson_val *result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    yyjson_val *entities = yyjson_obj_get(result, "entities");
    size_t i, count;
    yyjson_val *entity;
    yyjson_arr_foreach(entities, i, count, entity)
    {
        const char *name = yyjson_get_str(yyjson_obj_get(entity, "name"));
        if (!name || strcmp(name, owner)) {
            continue;
        }
        yyjson_val *components = yyjson_obj_get(entity, "components");
        size_t j, total;
        yyjson_val *component;
        yyjson_arr_foreach(components, j, total, component)
        {
            name = yyjson_get_str(yyjson_obj_get(component, "name"));
            if (!name || strcmp(name, component_name)) {
                continue;
            }
            yyjson_val *fields = yyjson_obj_get(component, "fields");
            size_t k, size;
            yyjson_val *entry;
            yyjson_arr_foreach(fields, k, size, entry)
            {
                name = yyjson_get_str(yyjson_obj_get(entry, "name"));
                if (name && !strcmp(name, field_name)) {
                    return yyjson_obj_get(entry, "value");
                }
            }
        }
    }
    return NULL;
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
    REQUIRE(report(600));
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
    REQUIRE(report(23));
    yyjson_val *result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    REQUIRE(yyjson_get_int(yyjson_obj_get(result, "live_entities")) == 468);
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
    yyjson_mut_doc *doc = request("ShellChecks", 74);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(14));
    yyjson_val *result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    yyjson_val *saved = yyjson_obj_get(result, "saved_state");
    REQUIRE(yyjson_is_obj(saved));
    doc = request("ReopenChecks", 8);
    yyjson_mut_val *initial = yyjson_val_mut_copy(doc, saved);
    REQUIRE(initial);
    yyjson_mut_obj_add_val(doc, yyjson_mut_doc_get_root(doc), "saved_state", initial);
    key(doc, 0, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(6));
    return true;
}

TEST_CASE(sprint_duet_and_disconnect)
{
    yyjson_mut_doc *doc = request("ModeChecks", 82);
    pads(doc, 0, true, 0);
    pads(doc, 30, false, 0);
    REQUIRE(snapshot(doc));
    REQUIRE(report(14));
    return true;
}

static const char *ui_text(const char *selector)
{
    yyjson_val *result = yyjson_obj_get(yyjson_doc_get_root(reply), "result");
    yyjson_val *ui = yyjson_obj_get(result, "ui");
    size_t i, count;
    yyjson_val *element;
    yyjson_arr_foreach(ui, i, count, element)
    {
        const char *name = yyjson_get_str(yyjson_obj_get(element, "element"));
        if (name && !strcmp(name, selector)) {
            return yyjson_get_str(yyjson_obj_get(element, "text"));
        }
    }
    return NULL;
}

TEST_CASE(final_clear_wins_over_blocked_spawn)
{
    static const char *modes[] = { "0", "2", "3" };
    static const char *titles[] = { "The water is still",
                                    "Forty lines. One flow.",
                                    "Together, home" };
    for (int i = 0; i < 3; ++i) {
        yyjson_mut_doc *doc = request("CompletionChecks", 25);
        yyjson_mut_val *saved = yyjson_mut_obj_add_obj(doc, yyjson_mut_doc_get_root(doc), "saved_state");
        yyjson_mut_obj_add_str(doc, saved, "chapter", "5");
        yyjson_mut_obj_add_str(doc, saved, "test_mode", modes[i]);
        key(doc, 0, 0);
        REQUIRE(snapshot(doc));
        REQUIRE(report(3));
        const char *title = ui_text("#result-title");
        REQUIRE(title && !strcmp(title, titles[i]));
    }
    return true;
}

int main(int argc, char **argv)
{
    const char *controller_path = getenv("STILLWATER_CTL");
    const char *project_path = getenv("STILLWATER_PROJECT");
    if (argc == 4 && !strcmp(argv[1], "--all")) {
        controller_path = argv[2];
        project_path = argv[3];
    } else if (argc != 1 || !controller_path || !project_path) {
        fprintf(stderr, "Usage: %s --all /path/to/chibi_ctl /path/to/game\n", argv[0]);
        return 2;
    }
    assert(realpath(controller_path, controller) && realpath(project_path, project));
    signal(SIGALRM, timeout_handler);
    alarm(58);
    prepare();
    RUN_TEST_CASE(authored_rules);
    RUN_TEST_CASE(live_keyboard_boards);
    RUN_TEST_CASE(live_controller_boards);
    RUN_TEST_CASE(lock_reset_limit);
    RUN_TEST_CASE(topout_and_rescue);
    RUN_TEST_CASE(journey_settings_and_reopen);
    RUN_TEST_CASE(sprint_duet_and_disconnect);
    RUN_TEST_CASE(final_clear_wins_over_blocked_spawn);
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
