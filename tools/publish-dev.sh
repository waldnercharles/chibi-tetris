#!/usr/bin/env bash
set -euo pipefail

tools=$(cd "$(dirname "$0")" && pwd)
source_root=$(cd "${1:-.}" && pwd)
project_path=${2:-.}
site_path=${3:-.}
for path in "$project_path" "$site_path"; do
    [[ $path =~ ^[A-Za-z0-9_./-]+$ && $path != /* && /$path/ != *'/../'* ]] || {
        printf '%s\n' 'Project and site paths must stay inside their repositories' >&2
        exit 1
    }
done
repository=$(cd "$source_root" && gh repo view --json nameWithOwner --jq .nameWithOwner)
mkdir -p "$source_root/build"
temporary=$(mktemp -d "$source_root/build/dev-pages.XXXXXX")
trap 'rm -rf "$temporary"' EXIT

git clone --quiet --depth 1 --single-branch --branch gh-pages \
    "https://github.com/$repository.git" "$temporary/site"
site="$temporary/site/$site_path"
revision=$(jq -er '.source_revision' "$site/deployment.json")
engine=$(jq -er '.engine_revision' "$site/deployment.json")
[[ $revision =~ ^[a-f0-9]{40}$ && $engine =~ ^[a-f0-9]{40}$ ]]
git clone --quiet --shared --no-checkout "$source_root" "$temporary/source"
if ! git -C "$temporary/source" cat-file -e "$revision^{commit}"; then
    git -C "$temporary/source" fetch "https://github.com/$repository.git" "$revision"
fi
git -C "$temporary/source" checkout --quiet --detach "$revision"
git -C "$temporary/source" remote set-url origin "https://github.com/$repository.git"
project="$temporary/source/$project_path"
[[ $(jq -er '.engine.revision' "$project/toolchain.json") == "$engine" ]]
bash "$tools/editor-package.sh" "$engine" "$temporary/editor" "$repository"
bash "$tools/assemble-dev.sh" "$project" "$site" "$temporary/editor"
(
    cd "$temporary/site"
    find . -type f ! -path './.git/*' ! -name SHA256SUMS -print | LC_ALL=C sort | \
        while IFS= read -r file; do shasum -a 256 "$file"; done > SHA256SUMS
    git add --all
    if git diff --cached --quiet; then
        printf 'Editor already matches published source %s\n' "$revision"
        exit 0
    fi
    git commit -m "Publish browser editor for $revision"
    git -c credential.helper= -c 'credential.helper=!gh auth git-credential' push origin HEAD:gh-pages
    # GITHUB_TOKEN pushes do not start a Pages build themselves.
    if [[ ${GITHUB_ACTIONS:-} == true ]]; then
        gh api --method POST "repos/$repository/pages/builds" >/dev/null
    fi
)
