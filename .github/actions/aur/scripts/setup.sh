#!/bin/bash

set -eu

pacman -Syyu --noconfirm --needed base-devel git openssh
echo "Installed Runtime Packages"
mkdir -p /root/.ssh/
echo "${aur_key}" > /root/.ssh/aur
chmod 0600 /root/.ssh/aur
cat > /root/.ssh/config << 'EOF'
Host aur.archlinux.org
  IdentityFile /root/.ssh/aur
  StrictHostKeyChecking no
  UserKnownHostsFile /dev/null
EOF
echo "Configured SSH Credentials"
