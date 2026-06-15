#!/bin/bash

set -eu

if [[ -d ./.not_git ]]; then
  mv ./.not_git ./.git
fi
git add packaging/
git commit -m "Release AUR Update" || true
git push
echo "Updated Repository"
