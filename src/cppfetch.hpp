#pragma once
#include <array>
#include <string_view>

struct art_entry {
  std::string_view name;
  std::array<std::string_view, 4> art;
};

constexpr std::array arts = {
    art_entry{"cat",
              std::array{
                  std::string_view{" ╱|、    "},
                  std::string_view{"(˚ˎ。7   "},
                  std::string_view{"| 、˜〵  "},
                  std::string_view{"じしˍ,)ノ"},
              }},
    art_entry{"freebsd",
              std::array{
                  std::string_view{"\033[31m_    _\033[0m"},
                  std::string_view{"\033[31m\\‾‾‾‾/\033[0m"},
                  std::string_view{"\033[31m|^vv^|\033[0m"},
                  std::string_view{"\033[31m\\____/\033[0m"},
              }},
    art_entry{"gentoo",
              std::array{
                  std::string_view{"\033[35m /‾\\ \033[0m"},
                  std::string_view{"\033[35m( \033[37mo \033[35m\\\033[0m"},
                  std::string_view{"\033[35m/   /\033[0m"},
                  std::string_view{"\033[35m\\__/ \033[0m"},
              }},
    art_entry{"arch",
              std::array{
                  std::string_view{"\033[34m   /\\   \033[0m"},
                  std::string_view{"\033[34m  /\\ \\  \033[0m"},
                  std::string_view{"\033[34m /    \\ \033[0m"},
                  std::string_view{"\033[34m/__/\\__\\\033[0m"},
              }},
};
