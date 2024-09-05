#include "handle_response.h"
#include "set_log_level.h"

#include <ipfs_client/opinionated_context.h>
#include <ipfs_client/ipfs_request.h>
#include <ipfs_client/logger.h>

#include <boost/asio/io_context.hpp>
#include <boost/beast/http/field.hpp>

#include <filesystem>
#include <iterator>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <thread>

namespace as = boost::asio;
using namespace std::chrono_literals;

int main(int const argc, char const* const argv[]) {
    as::io_context io;
    SetLevel(ipfs::log::Level::Warn);
    auto ctxt = ipfs::start_default(io);
    std::string semantic = "http";
    auto handle_arg = [&](std::string const& arg){
      if (set_log_level(arg)) {
        std::clog << "Log level set to " << arg << ".\n";
      } else if (arg.starts_with("semantic=")) {
        semantic.assign(arg, 9);
        std::clog << "Desired semantic set to " << semantic << ".\n";
      } else {
        auto req = ipfs::IpfsRequest::fromUrl(arg, handle_response);
        req->semantic(semantic);
        ctxt->partition({})->build_response(req);
      }
    };
    std::for_each(std::next(argv), std::next(argv, argc), handle_arg);
    io.run();
    std::clog << "\nFinished.\n";
}

