#!/bin/bash

set -eu

folder=$aur_folder/
# Only run if it has a PKGBUILD file
if [[ -f "${folder}PKGBUILD" ]]; then
  # Only run if it has a .SRCINFO file
  if [[ -f "${folder}.SRCINFO" ]]; then
    pushd $folder
    if [[ -d "./.git" ]]; then
      git add .
      git commit -m "Github Automated: ${tag_name}"
      git push --set-upstream aur master
      rm -fr .git
      echo "$(basename "$folder") package was updated!"
    fi
    popd
  else
    echo "$(basename "$folder") package lacks a .SRCINFO file"
    exit 1
  fi
else
  echo "$(basename "$folder") package lacks a PKGBUILD file"
  exit 1
fi
