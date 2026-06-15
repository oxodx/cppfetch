#include "cppfetch.hpp"
#include <sys/utsname.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <fstream>
#include <memory>
#include <print>
#include <string>

using namespace std::chrono;

struct uptime_info {
  days d{};
  hours h{};
  minutes m{};
  seconds s{};
};

auto get_uptime() -> std::expected<uptime_info, std::errc> {
  struct timespec ts{};
  if (clock_gettime(CLOCK_BOOTTIME, &ts) != 0) {
    return std::unexpected(std::errc{errno});
  }
  auto dur = seconds{ts.tv_sec};
  return uptime_info{
      .d = duration_cast<days>(dur),
      .h = duration_cast<hours>(dur % days{1}),
      .m = duration_cast<minutes>(dur % hours{1}),
      .s = dur % minutes{1},
  };
}

auto format_uptime(const uptime_info& u) -> std::string {
  if (u.d == days{0} && u.h == hours{0} && u.m == minutes{0}) {
    return std::format("{}s", u.s.count());
  }
  if (u.d == days{0} && u.h == hours{0}) {
    return std::format("{}m", u.m.count());
  }
  if (u.d == days{0}) {
    return std::format("{}h {}m", u.h.count(), u.m.count());
  }
  return std::format("{}d {}h {}m", u.d.count(), u.h.count(), u.m.count());
}

auto exec(const char* cmd) -> std::string {
  std::array<char, 256> buf{};
  std::string result;
  auto pipe = std::unique_ptr<FILE, int (*)(FILE*)>(popen(cmd, "r"), pclose);
  if (!pipe) return {};
  while (fgets(buf.data(), buf.size(), pipe.get())) {
    result += buf.data();
  }
  while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
    result.pop_back();
  }
  return result;
}

auto read_file(const char* path) -> std::string {
  std::ifstream f(path);
  if (!f) return {};
  return std::string(std::istreambuf_iterator<char>(f), {});
}

auto get_field_delim(const std::string& content, const std::string& key,
                     char delim) -> std::string {
  auto pos = content.find(key);
  if (pos == std::string::npos) return {};
  pos = content.find(delim, pos);
  if (pos == std::string::npos) return {};
  pos++;
  auto result = std::string{};
  if (pos < content.size() && (content[pos] == '"' || content[pos] == '\'')) {
    auto quote = content[pos++];
    while (pos < content.size() && content[pos] != quote) {
      result += content[pos++];
    }
  } else {
    while (pos < content.size() && content[pos] != '\n') {
      result += content[pos++];
    }
  }
  while (!result.empty() && (result.back() == ' ' || result.back() == '\t')) {
    result.pop_back();
  }
  return result;
}

auto get_field_eq(const std::string& content, const std::string& key)
    -> std::string {
  return get_field_delim(content, key, '=');
}

auto get_field_colon(const std::string& content, const std::string& key)
    -> std::string {
  return get_field_delim(content, key, ':');
}

auto get_os() -> std::string {
  auto content = read_file("/etc/os-release");
  if (content.empty()) return "Linux";
  auto name = get_field_eq(content, "PRETTY_NAME");
  if (!name.empty()) return name;
  name = get_field_eq(content, "NAME");
  if (!name.empty()) return name;
  return "Linux";
}

auto get_packages() -> std::string {
  auto pacman = exec("pacman -Q 2>/dev/null | wc -l");
  auto result = std::string{};
  if (!pacman.empty() && pacman != "0") {
    result += pacman + " (pacman)";
  }
  auto flatpak = exec("flatpak list 2>/dev/null | wc -l");
  if (!flatpak.empty() && flatpak != "0") {
    if (!result.empty()) result += ", ";
    result += flatpak + " (flatpak)";
  }
  if (result.empty()) return "unknown";
  return result;
}

auto get_cpu() -> std::string {
  auto content = read_file("/proc/cpuinfo");
  auto model = get_field_colon(content, "model name");
  if (model.empty()) return "unknown";

  auto pos = model.find("(R)");
  if (pos != std::string::npos) model.erase(pos, 3);
  pos = model.find("(TM)");
  if (pos != std::string::npos) model.erase(pos, 4);
  pos = model.find("CPU");
  if (pos != std::string::npos) model.erase(pos, 3);

  while (!model.empty() && model.back() == ' ') model.pop_back();
  while (!model.empty() && model[0] == ' ') model.erase(0, 1);

  return model;
}

auto get_gpu() -> std::string {
  auto out = exec(
      "lspci 2>/dev/null | grep -iE 'vga|3d|display' | head -1 | "
      "sed 's/.*: //'");
  if (out.empty()) return "unknown";
  return out;
}

auto get_de_wm() -> std::string {
  auto de = std::getenv("XDG_CURRENT_DESKTOP");
  if (de) {
    auto mode = std::getenv("XDG_SESSION_TYPE");
    if (mode) return std::string(de) + " (" + mode + ")";
    return de;
  }
  auto session = std::getenv("DESKTOP_SESSION");
  if (session) return session;
  auto wl = std::getenv("WAYLAND_DISPLAY");
  if (wl) return "Wayland";
  return "unknown";
}

auto get_terminal() -> std::string {
  auto term_prog = std::getenv("TERM_PROGRAM");
  if (term_prog) return term_prog;
  auto term_env = std::getenv("TERM");
  if (term_env) {
    auto sv = std::string_view(term_env);
    if (sv == "xterm-256color" || sv == "xterm") {
      auto desktop = std::getenv("XDG_SESSION_TYPE");
      if (desktop && std::string_view(desktop) == "wayland") return "foot";
    }
    return std::string(term_env);
  }
  return "unknown";
}

auto get_shell() -> std::string {
  auto shell = std::getenv("SHELL");
  if (shell) {
    auto sv = std::string_view(shell);
    auto pos = sv.rfind('/');
    if (pos != std::string_view::npos) return std::string(sv.substr(pos + 1));
    return std::string(sv);
  }
  return "unknown";
}

auto get_mem_label(const std::string& key) -> std::string {
  auto content = read_file("/proc/meminfo");
  auto total_str = get_field_colon(content, key);
  if (total_str.empty()) return "unknown";

  auto total_kb = std::stoll(total_str);
  auto total_gib = total_kb / (1024.0 * 1024.0);

  if (key == "MemTotal") {
    auto available = get_field_colon(content, "MemAvailable");
    auto avail_kb = available.empty() ? 0 : std::stoll(available);
    auto used_kb = total_kb - avail_kb;
    auto used_gib = used_kb / (1024.0 * 1024.0);
    return std::format("{:.1f} GiB / {:.1f} GiB", used_gib, total_gib);
  }

  auto free_str = get_field_colon(content, "SwapFree");
  auto free_kb = free_str.empty() ? 0 : std::stoll(free_str);
  auto used_kb = total_kb - free_kb;
  auto used_gib = used_kb / (1024.0 * 1024.0);
  return std::format("{:.1f} GiB / {:.1f} GiB", used_gib, total_gib);
}

auto get_root() -> std::string {
  auto out =
      exec("df -h / 2>/dev/null | awk 'NR==2{print $3\"/\"$2\" (\"$5\")\"}'");
  if (out.empty()) return "unknown";
  return out;
}

auto is_wide_cjk(char32_t cp) -> bool {
  return (cp >= 0x1100 && cp <= 0x115F) ||
         (cp >= 0x2E80 && cp <= 0x303E) ||
         (cp >= 0x3040 && cp <= 0x309F) ||
         (cp >= 0x30A0 && cp <= 0x30FF) ||
         (cp >= 0x3400 && cp <= 0x4DBF) ||
         (cp >= 0x4E00 && cp <= 0x9FFF) ||
         (cp >= 0xAC00 && cp <= 0xD7AF) ||
         (cp >= 0xF900 && cp <= 0xFAFF) ||
         (cp >= 0xFE10 && cp <= 0xFE1F) ||
         (cp >= 0xFE30 && cp <= 0xFE4F) ||
         (cp >= 0xFF01 && cp <= 0xFF60) ||
         (cp >= 0xFFE0 && cp <= 0xFFE6) ||
         (cp >= 0x1B000 && cp <= 0x1B0FF) ||
         (cp >= 0x1B100 && cp <= 0x1B12F) ||
         (cp >= 0x20000 && cp <= 0x2A6DF) ||
         (cp >= 0x2A700 && cp <= 0x2B73F) ||
         (cp >= 0x2B740 && cp <= 0x2B81F) ||
         (cp >= 0x2F800 && cp <= 0x2FA1F);
}

auto utf8_decode(const unsigned char*& s) -> char32_t {
  if (s[0] < 0x80) {
    auto cp = s[0];
    s++;
    return cp;
  }
  if ((s[0] & 0xE0) == 0xC0) {
    auto cp = static_cast<char32_t>(s[0] & 0x1F) << 6;
    cp |= s[1] & 0x3F;
    s += 2;
    return cp;
  }
  if ((s[0] & 0xF0) == 0xE0) {
    auto cp = static_cast<char32_t>(s[0] & 0x0F) << 12;
    cp |= static_cast<char32_t>(s[1] & 0x3F) << 6;
    cp |= s[2] & 0x3F;
    s += 3;
    return cp;
  }
  auto cp = static_cast<char32_t>(s[0] & 0x07) << 18;
  cp |= static_cast<char32_t>(s[1] & 0x3F) << 12;
  cp |= static_cast<char32_t>(s[2] & 0x3F) << 6;
  cp |= s[3] & 0x3F;
  s += 4;
  return cp;
}

auto visible_width(std::string_view s) -> size_t {
  size_t w = 0;
  for (size_t i = 0; i < s.size();) {
    if (s[i] == '\033') {
      while (i < s.size() && s[i] != 'm') i++;
      if (i < s.size()) i++;
      continue;
    }
    auto ptr = reinterpret_cast<const unsigned char*>(s.data()) + i;
    auto cp = utf8_decode(ptr);
    auto consumed = ptr - (reinterpret_cast<const unsigned char*>(s.data()) + i);
    i += consumed;
    w += is_wide_cjk(cp) ? 2 : 1;
  }
  return w;
}

auto main(int argc, char** argv) -> int {
  std::string_view art_name = "";
  bool show_art = true;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      std::println("CppFetch - A system fetch tool made in cpp\n");
      std::println("Argument list:\n");
      std::println("-na, --no-art | Display without the art.");
      std::println("-v, --version | Display package version.");
      return 0;
    } else if (arg == "--version" || arg == "-v") {
      std::println("CppFetch {}", VERSION);
      return 0;
    } else if (arg == "--no-art" || arg == "-na") {
      show_art = false;
    } else {
      art_name = arg;
    }
  }

  auto entry = &arts[0];
  bool found = true;

  if (argc > 1) [[unlikely]] {
    found = false;
    for (auto it = arts.begin(); it != arts.end(); ++it) {
      const auto& [n, _] = *it;
      if (n == art_name) [[unlikely]] {
        entry = &*it;
        found = true;
        break;
      }
    }
  }

  char username[128]{};
  char hostname[128]{};
  struct utsname uname_buf{};

  getlogin_r(username, sizeof(username));
  gethostname(hostname, sizeof(hostname));
  uname(&uname_buf);

  size_t pad = 0;
  if (show_art) {
    for (auto& line : entry->art) {
      pad = std::max(pad, visible_width(line));
    }
  }

  auto line = [&](size_t i, std::string_view label, const std::string& value) {
    auto prefix = std::string{};
    if (show_art && i < entry->art.size()) {
      prefix = entry->art[i];
      auto w = visible_width(prefix);
      if (w < pad) prefix.append(pad - w, ' ');
    } else {
      prefix.assign(pad, ' ');
    }
    if (label.empty()) {
      std::println("{}{}", prefix, value);
    } else {
      std::println("{}{:<10}{}", prefix, label, value);
    }
  };

  auto uptime_result = get_uptime();
  auto uptime_str =
      uptime_result ? format_uptime(*uptime_result) : std::string("unknown");

  line(0, "", std::format("{}@{}", username, hostname));
  line(1, "", "---------");
  line(2, "OS:", get_os());
  line(3,
       "Kernel:", std::format("{} {}", uname_buf.sysname, uname_buf.release));
  line(4, "Uptime:", uptime_str);
  line(5, "Packages:", get_packages());
  line(6, "CPU:", get_cpu());
  line(7, "GPU:", get_gpu());
  line(8, "DE/WM:", get_de_wm());
  line(9, "Terminal:", get_terminal());
  line(10, "Shell:", get_shell());
  line(11, "RAM:", get_mem_label("MemTotal"));
  line(12, "Swap:", get_mem_label("SwapTotal"));
  line(13, "Root:", get_root());

  return found ? 0 : 1;
}
