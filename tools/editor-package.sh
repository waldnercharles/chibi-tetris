#!/usr/bin/env bash
set -euo pipefail

engine=${1:?Engine revision required}
destination=${2:?Destination required}
repository=${3:?Repository required}
filter=$(cd "$(dirname "$0")" && pwd)/editor-manifest.jq
mkdir -p "$destination"

if [[ -n ${EDITOR:-} ]]; then
    cp -R "$EDITOR/." "$destination/"
else
    studio_url=${STUDIO_URL:-https://waldnercharles.github.io/chibi-studio/}
    if ! curl --fail --silent --show-error --max-time 30 "${studio_url%/}/studio.json" > "$destination/studio.json" ||
        ! jq -e --arg engine "$engine" -f "$filter" "$destination/studio.json" >/dev/null; then
        # Feature branches may need an editor built against their own engine.
        gh release download "web-editor-$engine" --repo "$repository" \
            --pattern studio.json --dir "$destination" --clobber
        archive=$(jq -er '.archive' "$destination/studio.json")
        [[ $archive =~ ^[A-Za-z0-9_.-]+\.tar\.gz$ ]]
        gh release download "web-editor-$engine" --repo "$repository" \
            --pattern "$archive" --dir "$destination"
    else
        archive=$(jq -er '.archive' "$destination/studio.json")
        [[ $archive =~ ^[A-Za-z0-9_.-]+\.tar\.gz$ ]]
        curl --fail --silent --show-error --max-time 60 \
            "${studio_url%/}/$archive" > "$destination/$archive"
    fi
    jq -e --arg engine "$engine" -f "$filter" "$destination/studio.json" >/dev/null
    expected=$(jq -er '.sha256' "$destination/studio.json")
    actual=$(shasum -a 256 "$destination/$archive" | cut -d ' ' -f 1)
    [[ $actual == "$expected" ]] || { printf '%s\n' 'Editor archive checksum mismatch' >&2; exit 1; }
    tar -xzf "$destination/$archive" -C "$destination"
    # Only the runtime and manifest belong in the site; the archive stays in its release.
    unlink "$destination/$archive"
fi
jq -e --arg engine "$engine" -f "$filter" "$destination/studio.json" >/dev/null || {
    printf '%s\n' 'The editor must support the game engine revision' >&2
    exit 1
}
