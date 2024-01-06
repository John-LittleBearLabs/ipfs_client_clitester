#include "set_log_level.h"

#include <ipfs_client/logger.h>

bool set_log_level(std::string const& level_name) {
  using L = ipfs::log::Level;
  for (auto i = -9; i <= 9; ++i) {
    auto lev = static_cast<L>(i);
    auto desc = LevelDescriptor(lev);
    if (desc == level_name) {
      SetLevel(lev);
      return true;
    }
  }
  return false;
}
