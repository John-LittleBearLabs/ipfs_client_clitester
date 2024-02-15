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
    auto handle_arg = [&](std::string const& arg){
      if (set_log_level(arg)) {
        std::clog << "Log level set to " << arg << ".\n";
      } else {
        auto req = ipfs::IpfsRequest::fromUrl(arg, handle_response);
        ctxt->partition({})->build_response(req);
      }
    };
    std::for_each(std::next(argv), std::next(argv, argc), handle_arg);
    io.run();
    std::clog << "\nFinished.\n";
}

