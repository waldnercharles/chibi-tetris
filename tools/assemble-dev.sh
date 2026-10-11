#!/usr/bin/env bash
set -euo pipefail

fail() { printf '%s\n' "$*" >&2; exit 1; }
[[ $# == 3 ]] || fail 'Usage: assemble-dev.sh PROJECT_ROOT SITE EDITOR_DIR'
project=$(cd "$1" && pwd -P)
site=$(cd "$2" && pwd -P)
editor=$(cd "$3" && pwd -P)
[[ $site != "$editor" && $site != "$editor/"* ]] || fail 'The site must be outside the editor package.'
[[ -f $site/index.html ]] || fail 'The site must already contain its player index.html.'

repository=$(git -C "$project" rev-parse --show-toplevel)
prefix=$(git -C "$project" rev-parse --show-prefix)
revision=$(git -C "$project" rev-parse HEAD)
tree=$revision
[[ -z $prefix ]] || tree="$revision:${prefix%/}"
git -C "$repository" diff --quiet HEAD -- "${prefix}project.kdl" "${prefix}content" "${prefix}toolchain.json" ||
    fail 'Commit the project source and toolchain before assembling its editor.'
engine=$(git -C "$repository" show "$revision:${prefix}toolchain.json" | jq -er '.engine.revision | select(test("^[0-9a-f]{40}$"))')
jq -e --arg engine "$engine" '.format == 1 and .engine == $engine and
    (.studio | type == "string" and test("^[0-9a-f]{40}$")) and
    (.archive | type == "string" and test("^[A-Za-z0-9_.-]+\\.tar\\.gz$")) and
    (.sha256 | type == "string" and test("^[0-9a-f]{64}$"))' "$editor/studio.json" >/dev/null ||
    fail 'The editor manifest must match the committed engine pin.'
# Match authored include nodes in this project's committed prototypes, including
# nested folders. An engine pin alone does not establish Studio loader support.
includes_status=0
git -C "$repository" grep -q -E '(^|[;{])[[:space:]]*(\([^)]*\)[[:space:]]*)?(includes|"includes")([[:space:];{]|$)' \
    "$revision" -- ":(glob)${prefix}content/prototypes/**/*.kdl" || includes_status=$?
(( includes_status <= 1 )) || fail 'Could not inspect committed prototype includes.'
if (( includes_status == 0 )); then
    jq -e '.capabilities.prototype_includes == true' "$editor/studio.json" >/dev/null ||
        fail 'The editor must declare capabilities.prototype_includes for this project.'
fi
for file in chibi-studio.html chibi-studio.js chibi-studio.wasm chibi-studio.data chibi-default.js; do
    [[ -f $editor/$file ]] || fail "Missing editor package file: $file"
done

origin=$(git -C "$repository" config --get remote.origin.url || true)
name=${origin##*/}
[[ -n $name ]] || name=${repository##*/}
name=${name%.git}
id=$(printf '%s' "$name${prefix:+-${prefix%/}}" | LC_ALL=C tr '[:upper:]' '[:lower:]' | sed -E 's/[^a-z0-9]+/-/g; s/^-+//; s/-+$//')
[[ -n $id && ${#id} -le 80 ]] || fail 'The repository and project path must produce an ID of 1–80 characters.'

temporary=$(mktemp -d "$site/.assemble-dev.XXXXXX")
replacing=false
complete=false
cleanup() {
    if $replacing && ! $complete; then
        rm -rf "$site/dev"
        if [[ -e $temporary/previous-dev || -L $temporary/previous-dev ]]; then mv "$temporary/previous-dev" "$site/dev"; fi
        cp "$temporary/previous-index.html" "$site/index.html"
    fi
    rm -rf "$temporary"
}
trap cleanup EXIT
mkdir "$temporary/dev"
cp -R "$editor/." "$temporary/dev/"
# A subtree archive otherwise uses the current time, changing its checksum on
# every publication and conflicting with the saved browser workspace identity.
timestamp=$(git -C "$repository" show -s --format=%ct "$revision")
git -C "$repository" archive --format=zip --mtime="@$timestamp" "$tree" project.kdl content > "$temporary/dev/project.zip"
checksum=$(shasum -a 256 "$temporary/dev/project.zip" | cut -d ' ' -f 1)
jq -n --arg id "$id" --arg revision "$revision" --arg sha256 "$checksum" \
    '{format: 1, id: $id, revision: $revision, archive: "project.zip", sha256: $sha256}' > "$temporary/dev/project.json"

inject_editor() {
    perl - "$1" "$2" "$3" "$4" <<'PERL'
use strict;
use warnings;
my ($input, $output, $bootstrap, $source) = @ARGV;
open my $in, '<', $input or die "$input: $!\n";
local $/;
my $html = <$in>;
my $pattern = qr{<script\b[^>]*\bsrc\s*=\s*["']?\Q$bootstrap\E["']?(?=[\s>])[^>]*>\s*</script\s*>}i;
my @scripts = $html =~ /$pattern/g;
die "Studio HTML must have exactly one supported startup script\n" unless @scripts == 1;
my $position = index($html, $scripts[0]);
die "Studio HTML must define Module before startup\n" unless substr($html, 0, $position) =~ /\bvar\s+Module\s*=/;
die "Editor package already selects a published project\n" if $html =~ /chibiPublishedProject/;
substr($html, $position, 0) = "<script>Module['chibiPublishedProject'] = '$source';</script>\n";
open my $out, '>', $output or die "$output: $!\n";
print {$out} $html or die "$output: $!\n";
close $out or die "$output: $!\n";
PERL
}
inject_editor "$editor/chibi-studio.html" "$temporary/dev/index.html" chibi-default.js project.json
if [[ -d $editor/threads ]]; then
    inject_editor "$editor/threads/chibi-studio.html" "$temporary/dev/threads/index.html" chibi-isolation.js ../project.json
fi
perl - "$site/index.html" "$temporary/index.html" <<'PERL'
use strict;
use warnings;
my ($input, $output) = @ARGV;
open my $in, '<', $input or die "$input: $!\n";
local $/;
my $html = <$in>;
my $redirect = <<'HTML';
<!-- chibi-dev:start -->
<script>
if (new URLSearchParams(location.search).has('dev')) {
  const url = new URL('dev/', location.href);
  url.search = location.search;
  url.searchParams.delete('dev');
  url.hash = location.hash;
  location.replace(url);
}
</script>
<!-- chibi-dev:end -->
HTML
my @heads = $html =~ /<head(?:\s[^>]*)?>/gi;
die "Player HTML must have one head element\n" unless @heads == 1;
my $block = qr{<!-- chibi-dev:start -->.*?<!-- chibi-dev:end -->(?:\r?\n)?}s;
my @blocks = $html =~ /$block/g;
die "Player HTML has duplicate editor redirects\n" if @blocks > 1;
if (@blocks) {
    $html =~ s{$block}{$redirect};
} else {
    $html =~ s{(<head(?:\s[^>]*)?>)}{$1\n$redirect}i;
}
open my $out, '>', $output or die "$output: $!\n";
print {$out} $html or die "$output: $!\n";
close $out or die "$output: $!\n";
PERL

[[ $(git -C "$repository" rev-parse HEAD) == "$revision" ]] || fail 'Source revision changed during editor assembly.'
git -C "$repository" diff --quiet HEAD -- "${prefix}project.kdl" "${prefix}content" "${prefix}toolchain.json" ||
    fail 'Project source changed during editor assembly.'
cp "$site/index.html" "$temporary/previous-index.html"
if [[ -e $site/dev || -L $site/dev ]]; then mv "$site/dev" "$temporary/previous-dev"; fi
replacing=true
mv "$temporary/dev" "$site/dev"
mv "$temporary/index.html" "$site/index.html"
complete=true
printf 'Assembled editor for %s at revision %s\n' "$id" "$revision"
