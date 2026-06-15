#include "cppfetch.hpp"
#include <cerrno>
#include <chrono>
#include <expected>
#include <print>
#include <sys/utsname.h>
#include <unistd.h>

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

void print_uptime(const uptime_info &u) {
  if (u.d == days{0} && u.h == hours{0} && u.m == minutes{0}) {
    std::println("Up {} seconds", u.s.count());
  } else if (u.d == days{0} && u.h == hours{0}) {
    std::println("Up {} minutes", u.m.count());
  } else if (u.d == days{0}) {
    std::println("Up {} hours, {} minutes", u.h.count(), u.m.count());
  } else {
    std::println("Up {} days, {} hours, {} minutes", u.d.count(), u.h.count(),
                 u.m.count());
  }
}

auto main(int argc, char **argv) -> int {
  auto entry = &arts[0];
  bool found = true;

  if (argc > 1) [[unlikely]] {
    found = false;
    std::string_view name = argv[1];
    for (auto it = arts.begin(); it != arts.end(); ++it) {
      const auto &[n, _] = *it;
      if (n == name) [[unlikely]] {
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

  auto [l0, l1, l2, l3] = entry->art;
  std::println("{} {}@{}", l0, username, hostname);
  std::println("{} --", l1);
  std::println("{} {} {}", l2, uname_buf.sysname, uname_buf.release);

  auto uptime_result = get_uptime();
  std::print("{} ", l3);
  if (uptime_result) [[likely]] {
    print_uptime(*uptime_result);
  }

  return found ? 0 : 1;
}
