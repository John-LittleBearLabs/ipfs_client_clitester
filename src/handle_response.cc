#include "handle_response.h"
#include "paths.h"

#include <ipfs_client/ipfs_request.h>
#include <ipfs_client/response.h>

#include <fstream>
#include <iostream>

void handle_response(ipfs::IpfsRequest const& req, ipfs::Response const& res) {
  std::clog << req.path().to_string() << " got status " << res.status_;
  if (res.status_ == 404) {
    std::clog << " body:" << res.body_;
  }
  std::clog << std::endl;
  if (res.status_ == 200) {
    auto write = [&](auto file_name) {
      std::ofstream f{file_name};
      f.write(res.body_.c_str(), res.body_.size());
      std::clog << "Wrote result to " << file_name << '\n';
    };
    auto req_path = output_path(req);
    write(req_path);
    auto loc_path = output_path(res);
    if (loc_path.size() && loc_path != req_path) {
      write(loc_path);
    }
  }
}
