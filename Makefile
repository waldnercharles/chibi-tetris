ENGINE ?= .chibi/engine
SDK ?= .chibi/sdk
ENGINE_BUILD ?= $(abspath $(ENGINE))/build/stillwater
CTL ?= $(ENGINE_BUILD)/bin/chibi_ctl
WEB_PLAYER ?= $(abspath $(ENGINE))/dist/web-stillwater
JOBS ?= 4
RELEASE_FLAGS := -DCMAKE_BUILD_TYPE=Release -DCHIBI_ENABLE_SANITIZERS=OFF -DCHIBI_ENABLE_CCACHE=ON

.PHONY: sdk authoring web-player play native web serve format validate test test-build

sdk:
	$(MAKE) -C "$(ENGINE)" package-sdk SDK_BUILD_DIR=build/sdk-stillwater SDK_PREFIX="$(abspath $(SDK))" SDK_CMAKE_FLAGS='$(RELEASE_FLAGS)' JOBS=$(JOBS)

authoring:
	$(MAKE) -C "$(ENGINE)" build BUILD_DIR="$(ENGINE_BUILD)" LOCAL_CMAKE_FLAGS='$(RELEASE_FLAGS)' JOBS=$(JOBS)

web-player:
	$(MAKE) -C "$(ENGINE)" package-web-player WEB_PLAYER_BUILD_DIR=build/web-stillwater WEB_PLAYER_OUTPUT="$(WEB_PLAYER)" JOBS=$(JOBS)

play:
	"$(SDK)/bin/chibi" "$(CURDIR)"

native:
	"$(SDK)/bin/chibi" export "$(CURDIR)" --output "$(CURDIR)/dist/Stillwater" --archive

web:
	"$(SDK)/bin/chibi" export "$(CURDIR)" --output "$(CURDIR)/dist/web" --web "$(WEB_PLAYER)" --archive

serve:
	python3 -m http.server 18769 --bind 127.0.0.1 --directory dist/web

format:
	"$(CTL)" --project-root "$(CURDIR)" --format

validate:
	printf '%s\n' '{"contract":1,"operation":"validate_project"}' | "$(CTL)" --project-root "$(CURDIR)"

test test-build:
	$(MAKE) -f tests/Makefile $@ ENGINE="$(abspath $(ENGINE))" CTL="$(abspath $(CTL))"
