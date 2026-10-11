CC ?= cc
ENGINE ?= ../chibi
CTL ?= $(ENGINE)/build/bin/chibi_ctl
TEST_BINARY := build/stillwater-tests
TEST_FLAGS := -std=c11 -O2 -Wall -Wextra -Werror
TEST_INCLUDES := -I$(ENGINE)/third_party/pico_headers -I$(ENGINE)/third_party/yyjson
ifeq ($(shell uname -s),Darwin)
TEST_LINK := -Wl,-dead_strip
else
TEST_LINK := -Wl,--gc-sections
endif

.PHONY: test test-build test-publishing
test-build: $(TEST_BINARY)

$(TEST_BINARY): tests/main.c tests/publishing.h $(ENGINE)/third_party/pico_headers/pico_unit.h $(ENGINE)/third_party/yyjson/yyjson.c
	mkdir -p build
	$(CC) $(TEST_FLAGS) $(TEST_INCLUDES) tests/main.c $(ENGINE)/third_party/yyjson/yyjson.c $(TEST_LINK) -o $@

test: test-build
	STILLWATER_CTL="$(CTL)" STILLWATER_PROJECT="$(CURDIR)" python3 "$(ENGINE)/tools/test_policy.py" short "$(CURDIR)/$(TEST_BINARY)"

test-publishing: test-build
	python3 -c 'import runpy, sys; sys.exit(runpy.run_path(sys.argv[1])["run_commands"]([[sys.argv[2], "--publishing", sys.argv[3]]]))' \
		"$(ENGINE)/tools/test_policy.py" "$(CURDIR)/$(TEST_BINARY)" "$(CURDIR)"
