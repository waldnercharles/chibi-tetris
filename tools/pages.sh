#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

repository=https://github.com/waldnercharles/chibi-tetris.git
url=https://waldnercharles.github.io/chibi-tetris/
revision=$(git rev-parse HEAD)
engine=$(jq -er '.engine.revision' toolchain.json)

fail() { printf '%s\n' "$*" >&2; exit 1; }
hash() { shasum -a 256 "$1" | cut -d ' ' -f 1; }

[[ -z $(git status --porcelain) ]] || fail 'Commit source changes before publishing or verifying a release.'
mkdir -p build
temporary=$(mktemp -d "$PWD/build/pages.XXXXXX")
trap 'rm -rf "$temporary"' EXIT

case ${1:-} in
publish)
    sdk=${2:?SDK directory required}
    player=${3:?Web player directory required}
    branch=$(git symbolic-ref --short HEAD)
    remote=$(git ls-remote "$repository" "refs/heads/$branch" | cut -f 1)
    [[ $revision == "$remote" ]] || fail "Push source branch $branch before publishing."
    gh api "repos/waldnercharles/chibi/commits/$engine" --jq .sha > "$temporary/engine"
    [[ $(cat "$temporary/engine") == "$engine" ]] || fail 'Publish the pinned engine commit first.'
    gh api repos/waldnercharles/chibi-tetris/pages > "$temporary/pages.json"
    jq -e '.build_type == "legacy" and .source == {branch: "gh-pages", path: "/"}' \
        "$temporary/pages.json" >/dev/null || fail 'Pages must serve gh-pages from its root.'
    snapshot=$(jq -er '.engine.source_snapshot_sha256' toolchain.json)
    jq -e --arg revision "$engine" --arg snapshot "$snapshot" \
        '.source.revision == $revision and .source.snapshot_sha256 == $snapshot and .source.dirty == false' \
        "$sdk/share/chibi/package.json" >/dev/null || fail 'The SDK must match the clean pinned engine.'
    jq -e --arg engine "$engine" '.tested_web_player.source_revision == $engine and
        (.tested_web_player.files | type == "object" and has("chibi-player.js") and
        has("chibi-player.wasm") and has("index.html") and has("compiler/chibi-compile.js") and
        has("compiler/chibi-compile.wasm"))' toolchain.json >/dev/null || \
        fail 'The web player must be pinned to the same engine revision.'
    jq -er '.tested_web_player.files | to_entries[] | [.key, .value] | @tsv' \
        toolchain.json > "$temporary/player-files"
    while IFS=$'\t' read -r file expected; do
        [[ $(hash "$player/$file") == "$expected" ]] || fail "Web player differs from toolchain.json: $file"
    done < "$temporary/player-files"

    make web SDK="$sdk" WEB_PLAYER="$player"
    [[ $(git rev-parse HEAD) == "$revision" && -z $(git status --porcelain) ]] || fail 'Source changed during export.'
    git clone --quiet --depth 1 --single-branch --branch gh-pages "$repository" "$temporary/site"
    # The companion editor workflow refreshes /dev for this source revision.
    # Keep the current editor available until that publication completes.
    rsync -a --delete --exclude=.git --exclude=/dev/ dist/web/ "$temporary/site/"
    cp LICENSES.md "$temporary/site/"
    touch "$temporary/site/.nojekyll"
    bash tools/editor-package.sh "$engine" "$temporary/editor" waldnercharles/chibi-tetris
    bash tools/assemble-dev.sh "$PWD" "$temporary/site" "$temporary/editor"
    jq -n --arg revision "$revision" --arg engine "$engine" \
        '{format: 1, source_revision: $revision, engine_revision: $engine}' > "$temporary/site/deployment.json"
    (
        cd "$temporary/site"
        find . -type f ! -path './.git/*' ! -name SHA256SUMS -print | LC_ALL=C sort | \
            while IFS= read -r file; do shasum -a 256 "$file"; done > SHA256SUMS
        git add --all
        if ! git diff --cached --quiet; then
            git commit -m "Publish Stillwater from $revision"
            git -c credential.helper= -c 'credential.helper=!gh auth git-credential' push origin HEAD:gh-pages
        fi
        printf 'Pages commit: %s\n' "$(git rev-parse HEAD)"
    )
    printf 'Submitted %s to %s\nWait for Pages deployment, then run make verify-published and check browser play.\n' "$revision" "$url"
    ;;
verify)
    curl --fail --silent --show-error --max-time 15 "${url}deployment.json?revision=$revision" > "$temporary/deployment.json"
    jq -e --arg revision "$revision" --arg engine "$engine" \
        '.source_revision == $revision and .engine_revision == $engine' "$temporary/deployment.json" >/dev/null || \
        fail 'The public release does not match this source and engine revision yet.'
    git clone --quiet --depth 1 --single-branch --branch gh-pages "$repository" "$temporary/site"
    cmp "$temporary/deployment.json" "$temporary/site/deployment.json"
    curl --fail --silent --show-error --max-time 15 "${url}SHA256SUMS?revision=$revision" > "$temporary/SHA256SUMS"
    cmp "$temporary/SHA256SUMS" "$temporary/site/SHA256SUMS"
    for file in index.html files.json chibi-player.js chibi-player.wasm data/game.chibip \
        dev/index.html dev/project.json dev/project.zip dev/studio.json; do
        curl --fail --silent --show-error --max-time 15 "${url}${file}?revision=$revision" > "$temporary/asset"
        [[ $(hash "$temporary/asset") == "$(hash "$temporary/site/$file")" ]] || fail "Public asset differs: $file"
    done
    printf 'Verified public revision %s and its game/player assets: %s\n' "$revision" "$url"
    ;;
*) fail 'Usage: pages.sh publish SDK WEB_PLAYER | verify' ;;
esac
