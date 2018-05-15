#!/bin/sh -e

basecommit="refs/tags/upstream/1.0f-2"
repo=https://anonscm.debian.org/git/debian-edu/pkg-team/veracrypt.git
remote=unit193-indicator
branch=indicator.unit193-veracrypt
patch=debian/patches/003-indicator-support.diff
patch_source=refs/remotes/$remote/master

prepatch=debian/patches/002-build-flags.diff
prepatch_source=d35595e562fd029ba1415ced843cae046e37e113

## Clean up (allow redoing)
#git reset --hard HEAD
#git clean -dxf
#git checkout refs/remotes/origin/master
#git branch -D "$branch"
#git remote remove "$remote"

git remote add "$remote" "$repo"
git fetch "$remote"
git checkout -b "$branch" "$basecommit"

git show "$prepatch_source:$prepatch" | git apply -
git add src/
author="$(git show -s --format="%an <%ae>" "$prepatch_source")"
date="$(git show -s --format="%ad" "$prepatch_source")"
message="$(git show -s --format="%B" "$prepatch_source")"

message="$message

(applied $patch at $commit in $repo)"

git commit -q --no-gpg-sign -m "$message" --allow-empty --author="$author" --date="$date"
basecommit="$(git rev-parse HEAD)"

commits="$(git log --reverse --format=format:%H "$patch_source" -- "$patch")"

for commit in $commits; do
	git checkout "$basecommit" .
	git show "$commit:$patch" | ( cd src/ && git apply -; )
	git add src/

	author="$(git show -s --format="%an <%ae>" "$commit")"
	date="$(git show -s --format="%ad" "$commit")"
	message="$(git show -s --format="%B" "$commit")"

	message="$message

(applied $patch at $commit in $repo)"

	git commit -q --no-gpg-sign -m "$message" --allow-empty --author="$author" --date="$date"
done
