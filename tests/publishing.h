static char publishing_repository[PATH_MAX], publishing_editor[PATH_MAX];
static char publishing_site[PATH_MAX], publishing_helper[PATH_MAX];
static const char publishing_pin[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

static void publishing_write(const char *base, const char *relative, const char *text)
{
    char path[PATH_MAX];
    path_join(path, base, relative);
    FILE *file = fopen(path, "wb");
    assert(file);
    size_t size = strlen(text);
    assert(fwrite(text, 1, size, file) == size);
    assert(!fclose(file));
}

static char *publishing_read(const char *base, const char *relative)
{
    char path[PATH_MAX];
    path_join(path, base, relative);
    FILE *file = fopen(path, "rb");
    assert(file && !fseek(file, 0, SEEK_END));
    long length = ftell(file);
    assert(length >= 0 && !fseek(file, 0, SEEK_SET));
    char *text = malloc((size_t)length + 1);
    assert(text && fread(text, 1, (size_t)length, file) == (size_t)length);
    text[length] = '\0';
    assert(!fclose(file));
    return text;
}

static int publishing_count(const char *text, const char *needle)
{
    int count = 0;
    while ((text = strstr(text, needle))) {
        count++;
        text += strlen(needle);
    }
    return count;
}

static void publishing_manifest(const char *engine, bool prototype_includes)
{
    char manifest[512];
    snprintf(
        manifest,
        sizeof manifest,
        "{\"format\":1,\"engine\":\"%s\",\"studio\":"
        "\"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\","
        "\"archive\":\"editor.tar.gz\",\"sha256\":"
        "\"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\"%"
        "s}"
        "\n",
        engine,
        prototype_includes ? ",\"capabilities\":{\"prototype_includes\":true}" : ""
    );
    publishing_write(publishing_editor, "studio.json", manifest);
}

static void publishing_commit()
{
    char *add[] = { "/usr/bin/git", "-C", publishing_repository,
                    "add",          ".",  NULL };
    assert(run(add, NULL, NULL) == 0);
    char *commit[] = { "/usr/bin/git",
                       "-C",
                       publishing_repository,
                       "-c",
                       "user.name=Fixture",
                       "-c",
                       "user.email=fixture@example.invalid",
                       "-c",
                       "commit.gpgsign=false",
                       "-c",
                       "core.hooksPath=/dev/null",
                       "commit",
                       "--quiet",
                       "-m",
                       "Fixture",
                       NULL };
    assert(run(commit, NULL, NULL) == 0);
}

static int publishing_assemble(const char *root)
{
    char log[PATH_MAX];
    path_join(log, temporary, "assemble.log");
    char *arguments[] = { "/bin/bash",     publishing_helper, (char *)root,
                          publishing_site, publishing_editor, NULL };
    return run(arguments, NULL, log);
}

static bool publishing_archive(const char *root, const char *id)
{
    REQUIRE(publishing_assemble(root) == 0);
    char archive[PATH_MAX], listing[PATH_MAX];
    path_join(archive, publishing_site, "dev/project.zip");
    path_join(listing, temporary, "archive.txt");
    char *arguments[] = { "/usr/bin/unzip", "-Z1", archive, NULL };
    REQUIRE(run(arguments, NULL, listing) == 0);
    char *names = publishing_read(temporary, "archive.txt");
    bool correct = !strcmp(names, "content/\ncontent/Scene.kdl\nproject.kdl\n");
    free(names);
    REQUIRE(correct);
    char *text = publishing_read(publishing_site, "dev/project.json");
    yyjson_doc *doc = yyjson_read(text, strlen(text), 0);
    free(text);
    REQUIRE(doc);
    yyjson_val *manifest = yyjson_doc_get_root(doc);
    const char *actual = yyjson_get_str(yyjson_obj_get(manifest, "id"));
    bool identity = actual && !strcmp(actual, id);
    yyjson_doc_free(doc);
    REQUIRE(identity);
    return true;
}

TEST_CASE(published_editor_root_and_repeat)
{
    REQUIRE(publishing_archive(publishing_repository, "example-games"));
    char *initial = publishing_read(publishing_site, "index.html");
    REQUIRE(publishing_assemble(publishing_repository) == 0);
    char *editor = publishing_read(publishing_site, "dev/index.html");
    bool startup = publishing_count(editor, "chibiPublishedProject") == 1 &&
                   strstr(editor, "chibiPublishedProject") <
                       strstr(editor, "src=chibi-default.js") &&
                   strstr(editor, "'project.json'");
    free(editor);
    REQUIRE(startup);
    char *threaded = publishing_read(publishing_site, "dev/threads/index.html");
    bool thread_startup = publishing_count(threaded, "chibiPublishedProject") == 1 &&
                          strstr(threaded, "'../project.json'");
    free(threaded);
    REQUIRE(thread_startup);
    char *player = publishing_read(publishing_site, "index.html");
    bool redirect = publishing_count(player, "<!-- chibi-dev:start -->") == 1 &&
                    strstr(player, "<body>Player</body>");
    bool unchanged_html = !strcmp(initial, player);
    free(initial);
    free(player);
    REQUIRE(redirect);
    REQUIRE(unchanged_html);
    char *asset = publishing_read(publishing_site, "game.wasm");
    bool unchanged = !strcmp(asset, "player bytes\n");
    free(asset);
    REQUIRE(unchanged);
    return true;
}

TEST_CASE(published_editor_nested_project)
{
    char nested[PATH_MAX];
    path_join(nested, publishing_repository, "games/Block Game");
    REQUIRE(publishing_archive(nested, "example-games-games-block-game"));
    char archive[PATH_MAX], output[PATH_MAX];
    path_join(archive, publishing_site, "dev/project.zip");
    path_join(output, temporary, "nested.kdl");
    char *arguments[] = { "/usr/bin/unzip", "-p", archive, "project.kdl", NULL };
    REQUIRE(run(arguments, NULL, output) == 0);
    char *manifest = publishing_read(temporary, "nested.kdl");
    bool nested_source = !strcmp(manifest, "name \"Nested\"\n");
    free(manifest);
    REQUIRE(nested_source);
    char checksum[PATH_MAX];
    path_join(checksum, temporary, "checksum.txt");
    char *hash[] = { "/usr/bin/shasum", "-a", "256", archive, NULL };
    REQUIRE(run(hash, NULL, checksum) == 0);
    char *before = publishing_read(temporary, "checksum.txt");
    sleep(2);
    REQUIRE(publishing_assemble(nested) == 0);
    REQUIRE(run(hash, NULL, checksum) == 0);
    char *after = publishing_read(temporary, "checksum.txt");
    bool stable = !strcmp(before, after);
    free(before);
    free(after);
    REQUIRE(stable);
    return true;
}

TEST_CASE(published_editor_rejects_mismatch_and_dirty_source)
{
    char *before = publishing_read(publishing_site, "dev/project.json");
    publishing_manifest("dddddddddddddddddddddddddddddddddddddddd", false);
    REQUIRE(publishing_assemble(publishing_repository) != 0);
    publishing_manifest(publishing_pin, false);
    publishing_write(publishing_repository, "content/Scene.kdl", "uncommitted\n");
    REQUIRE(publishing_assemble(publishing_repository) != 0);
    publishing_write(publishing_repository, "content/Scene.kdl", "name \"Scene\"\n");
    publishing_write(publishing_editor, "chibi-studio.html", "<html>broken bootstrap</html>\n");
    REQUIRE(publishing_assemble(publishing_repository) != 0);
    char *after = publishing_read(publishing_site, "dev/project.json");
    bool preserved = !strcmp(before, after);
    free(before);
    free(after);
    REQUIRE(preserved);
    return true;
}

TEST_CASE(published_editor_requires_prototype_includes)
{
    char nested[PATH_MAX], prototypes[PATH_MAX];
    path_join(nested, publishing_repository, "games/Block Game");
    path_join(prototypes, nested, "content/prototypes/layers");
    char *directories[] = { "/bin/mkdir", "-p", prototypes, NULL };
    REQUIRE(run(directories, NULL, NULL) == 0);
    const char *source = "name \"World\"\n(array)includes \"Base\"\n";
    publishing_write(prototypes, "World.kdl", source);
    publishing_write(prototypes, "Base.kdl", "name \"Base\"\n");
    /* Untracked prototypes are outside the committed publication. */
    REQUIRE(publishing_assemble(nested) == 0);
    publishing_commit();
    /* A sibling project cannot require capabilities for this root. */
    REQUIRE(publishing_assemble(publishing_repository) == 0);
    char *before = publishing_read(publishing_site, "dev/project.json");
    REQUIRE(publishing_assemble(nested) != 0);
    char *after = publishing_read(publishing_site, "dev/project.json");
    bool preserved = !strcmp(before, after);
    free(before);
    free(after);
    REQUIRE(preserved);
    publishing_manifest(publishing_pin, true);
    REQUIRE(publishing_assemble(nested) == 0);
    char archive[PATH_MAX], output[PATH_MAX];
    path_join(archive, publishing_site, "dev/project.zip");
    path_join(output, temporary, "included.kdl");
    char *extract[] = { "/usr/bin/unzip",
                        "-p",
                        archive,
                        "content/prototypes/layers/World.kdl",
                        NULL };
    REQUIRE(run(extract, NULL, output) == 0);
    char *archived = publishing_read(temporary, "included.kdl");
    bool unchanged = !strcmp(archived, source);
    free(archived);
    REQUIRE(unchanged);
    publishing_manifest(publishing_pin, false);
    return true;
}

static int publishing_main(const char *root)
{
    assert(realpath(root, project));
    signal(SIGALRM, timeout_handler);
    alarm(58);
    char scratch[PATH_MAX];
    path_join(scratch, project, "build/resonance");
    assert(mkdir(scratch, 0755) == 0 || errno == EEXIST);
    path_join(temporary, scratch, "publishing-tests-XXXXXX");
    assert(mkdtemp(temporary));
    path_join(publishing_helper, project, "tools/assemble-dev.sh");
    path_join(publishing_repository, temporary, "repository with spaces");
    path_join(publishing_editor, temporary, "editor");
    path_join(publishing_site, temporary, "site");
    char content[PATH_MAX], nested[PATH_MAX], ignored[PATH_MAX], threads[PATH_MAX];
    path_join(content, publishing_repository, "content");
    path_join(nested, publishing_repository, "games/Block Game/content");
    path_join(ignored, publishing_repository, "tests");
    path_join(threads, publishing_editor, "threads");
    char *directories[] = { "/bin/mkdir",    "-p",    content, nested, ignored,
                            publishing_site, threads, NULL };
    assert(run(directories, NULL, NULL) == 0);
    const char *pin = "{\"engine\":{\"revision\":"
                      "\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\"}}\n";
    publishing_write(publishing_repository, "toolchain.json", pin);
    publishing_write(publishing_repository, "project.kdl", "name \"Root\"\n");
    publishing_write(publishing_repository, "content/Scene.kdl", "name \"Scene\"\n");
    publishing_write(publishing_repository, "tests/private.txt", "excluded\n");
    publishing_write(publishing_repository, "games/Block Game/project.kdl", "name \"Nested\"\n");
    publishing_write(publishing_repository, "games/Block Game/toolchain.json", pin);
    publishing_write(publishing_repository, "games/Block Game/content/Scene.kdl", "name \"Nested Scene\"\n");
    char *initialize[] = { "/usr/bin/git", "init", "--quiet", publishing_repository, NULL };
    assert(run(initialize, NULL, NULL) == 0);
    char *remote[] = { "/usr/bin/git",
                       "-C",
                       publishing_repository,
                       "remote",
                       "add",
                       "origin",
                       "https://github.com/example/example-games.git",
                       NULL };
    assert(run(remote, NULL, NULL) == 0);
    publishing_commit();
    publishing_manifest(publishing_pin, false);
    publishing_write(publishing_editor, "chibi-studio.html", "<html><head></head><body><script>var Module={};</script><script src=chibi-default.js></script></body></html>\n");
    publishing_write(publishing_editor, "threads/chibi-studio.html", "<html><script>var Module = {};</script><script src=\"chibi-isolation.js\"></script></html>\n");
    const char *files[] = { "chibi-studio.js",
                            "chibi-studio.wasm",
                            "chibi-studio.data",
                            "chibi-default.js" };
    for (size_t i = 0; i < sizeof files / sizeof *files; i++) {
        publishing_write(publishing_editor, files[i], "editor fixture\n");
    }
    publishing_write(publishing_site, "index.html", "<html><head></head><body>Player</body></html>\n");
    publishing_write(publishing_site, "game.wasm", "player bytes\n");
    RUN_TEST_CASE(published_editor_root_and_repeat);
    RUN_TEST_CASE(published_editor_nested_project);
    RUN_TEST_CASE(published_editor_requires_prototype_includes);
    RUN_TEST_CASE(published_editor_rejects_mismatch_and_dirty_source);
    pu_print_stats();
    if (pu_test_failed()) {
        fprintf(stderr, "Failure artifacts retained in %s\n", temporary);
        return 1;
    }
    char *remove[] = { "/bin/rm", "-rf", temporary, NULL };
    assert(run(remove, NULL, NULL) == 0);
    alarm(0);
    return 0;
}
