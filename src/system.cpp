#include "system.hpp"
#include <unistd.h>
#include <string>

std::string getUsername() {
  const char* user = getenv("USER");

  if (user) {
    return std::string(user);
  } else {
    return "unknown";
  }
}

std::string getHostname() {
  char hostname[256];

  gethostname(hostname, sizeof(hostname));

  return std::string(hostname);
}
