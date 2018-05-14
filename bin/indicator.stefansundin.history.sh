#!/bin/sh -e

basecommit=e0b39ef434fdb0a225f2e9dd71bdd677c73e3231
repo=https://github.com/stefansundin/truecrypt.deb.git
remote=stefansundin
branch=indicator.stefansundin
patch=patches/truecrypt-7.1a-indicator.patch
patch_source=refs/remotes/$remote/master

## Clean up (allow redoing)
#git reset --hard HEAD
#git clean -dxf
#git checkout refs/remotes/origin/master
#git branch -D "$branch"
#git remote remove "$remote"

git remote add "$remote" "$repo"
git fetch "$remote"
git checkout -b "$branch" "$basecommit"
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
