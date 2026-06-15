#include "cppfetch.hpp"
#include <sys/utsname.h>
#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <print>
#include <string>
#include "system.hpp"

using namespace std::chrono;

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

auto is_wide_cjk(char32_t cp) -> bool {
  return (cp >= 0x1100 && cp <= 0x115F) || (cp >= 0x2E80 && cp <= 0x303E) ||
         (cp >= 0x3040 && cp <= 0x309F) || (cp >= 0x30A0 && cp <= 0x30FF) ||
         (cp >= 0x3400 && cp <= 0x4DBF) || (cp >= 0x4E00 && cp <= 0x9FFF) ||
         (cp >= 0xAC00 && cp <= 0xD7AF) || (cp >= 0xF900 && cp <= 0xFAFF) ||
         (cp >= 0xFE10 && cp <= 0xFE1F) || (cp >= 0xFE30 && cp <= 0xFE4F) ||
         (cp >= 0xFF01 && cp <= 0xFF60) || (cp >= 0xFFE0 && cp <= 0xFFE6) ||
         (cp >= 0x1B000 && cp <= 0x1B0FF) || (cp >= 0x1B100 && cp <= 0x1B12F) ||
         (cp >= 0x20000 && cp <= 0x2A6DF) || (cp >= 0x2A700 && cp <= 0x2B73F) ||
         (cp >= 0x2B740 && cp <= 0x2B81F) || (cp >= 0x2F800 && cp <= 0x2FA1F);
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
    auto consumed =
        ptr - (reinterpret_cast<const unsigned char*>(s.data()) + i);
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

  line(0, "", std::format("{}@{}", getUsername(), getHostname()));
  line(1, "", "---------");
  line(2, "OS:", getOS());
  line(3, "Kernel:", getKernel());
  line(4, "Uptime:", getUptime());
  line(5, "Packages:", getPackages());
  line(6, "CPU:", getCPU());
  line(7, "GPU:", getGPU()[0]);
  line(8, "DE:", getDE());
  line(9, "Terminal:", getTerminal());
  line(10, "Shell:", getShell());
  line(11, "RAM:", getRAM());
  line(12, "Swap:", getSwap());
  line(13, "Root:", getRootStorage());

  return found ? 0 : 1;
}
