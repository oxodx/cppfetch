# cppfetch

A system fetch tool written in C++. Displays system information with ASCII art.

## Features

- OS, kernel, uptime, shell, terminal, DE, packages, CPU, GPU, RAM, swap,
  disk, display resolution, local IP
- ASCII art per distro (Arch, Gentoo, FreeBSD, cat)
- Package manager detection (pacman, dpkg, emerge, rpm, flatpak)
- Display detection for Wayland (Hyprland, Sway, Niri, KDE, GNOME), X11,
  DRM, and framebuffer
- GPU name lookup from `pci.ids`

## Dependencies

- C++26 compiler (clang++ recommended)
- libX11, libXrandr (for X11 display detection)
- nlohmann/json (vendored in `vendor/nlohmann/`)

## Build & Install

### From source

```sh
make          # build (produces bin/cppfetch)
sudo make install   # install to /usr/local/bin
sudo make uninstall # remove from /usr/local/bin
```

Override the install path with `BIN=/some/path`.

The version string is derived from `git describe --tags`. Override with
`VERSION=x.y.z`.

Compiler flags and linker flags can be overridden via `CXXFLAGS` and `LDFLAGS`.

### Arch Linux (AUR)

```
yay -S cppfetch
```

Or with any other AUR helper.

## Usage

```
cppfetch [options] [art_name]
```

| Argument      | Description                      |
| ------------- | -------------------------------- |
| `-h`, `--help`  | Show help                        |
| `-v`, `--version` | Show version                   |
| `-na`, `--no-art` | Hide ASCII art                |
| `art_name`    | Art to display (arch, gentoo, freebsd, cat) |

## Colors

Use the environment variable `CPPFETCH_COLOR` to override the accent color.
Valid values: `black`, `red`, `green`, `yellow`, `blue`, `purple`, `cyan`,
`white`, `pink`.

## License

MIT
