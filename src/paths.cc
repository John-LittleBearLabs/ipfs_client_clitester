#include "paths.h"

#include <ipfs_client/ipfs_request.h>
#include <ipfs_client/response.h>

std::string output_path(ipfs::IpfsRequest const& r) {
  std::string rv{r.path().to_string()};
  for (auto &c : rv) {
    if (!std::isalnum(c) && c != '.') {
      c = '_';
    }
  }
  return rv;
}
std::string output_path(ipfs::Response const& r) {
  auto rv = r.location_;
  for (auto &c : rv) {
    if (!std::isalnum(c) && c != '.') {
      c = '_';
    }
  }
  return rv;
}
