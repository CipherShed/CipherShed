#!/bin/sh -e

basecommit=e0b39ef434fdb0a225f2e9dd71bdd677c73e3231
repo=https://github.com/stefansundin/truecrypt.deb.git
remote=stefansundin
branch1=indicator.stefansundin.step1
branch2=indicator.stefansundin.step2
patch=patches/truecrypt-7.1a-indicator.patch
patch_source=refs/remotes/$remote/master

prepatch=patches/truecrypt-7.1a-build-fixes.patch
prepatch_source=0569ec52dbec589af1cb5e9609b4fa9a1afa7994

## Clean up (allow redoing)
#git reset --hard HEAD
#git clean -dxf
#git checkout refs/remotes/origin/master
#git branch -D "$branch1"
#git branch -D "$branch2"
#git remote remove "$remote"

git remote add "$remote" "$repo"
git fetch "$remote"
commits1="$(git log --reverse --format=format:%H "$prepatch_source"^1 -- patches/truecrypt-7.1a-indicator.patch)"
commits2="$(git log --reverse --format=format:%H "$prepatch_source"^'!' refs/remotes/stefansundin/master -- patches/truecrypt-7.1a-indicator.patch)"

# Revisions before patches/truecrypt-7.1a-build-fixes.patch dependency
git checkout -b "$branch1" "$basecommit"
for commit in $commits1; do
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

# Address patches/truecrypt-7.1a-build-fixes.patch dependency
git checkout -b "$branch2"
git checkout "$basecommit" .
git show "$prepatch_source:$prepatch" | ( cd src/ && git apply -; )
git add src/

author="$(git show -s --format="%an <%ae>" "$prepatch_source")"
date="$(git show -s --format="%ad" "$prepatch_source")"
message="$(git show -s --format="%B" "$prepatch_source")"

message="$message

(applied $prepatch at $prepatch_source in $repo)"

git commit -q --no-gpg-sign -m "$message" --allow-empty --author="$author" --date="$date"

# Revisions after patches/truecrypt-7.1a-build-fixes.patch dependency
for commit in $commits2; do
	git checkout "$basecommit" .
	git show "$prepatch_source:$prepatch" | ( cd src/ && git apply -; )
	git show "$commit:$patch" | ( cd src/ && git apply -; )
	git add src/

	author="$(git show -s --format="%an <%ae>" "$commit")"
	date="$(git show -s --format="%ad" "$commit")"
	message="$(git show -s --format="%B" "$commit")"

	message="$message

(applied $patch at $commit in $repo)"

	git commit -q --no-gpg-sign -m "$message" --allow-empty --author="$author" --date="$date"
done
