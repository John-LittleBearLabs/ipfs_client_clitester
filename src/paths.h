#pragma once

#include <string>

namespace ipfs {
class IpfsRequest;
class Response;
}

std::string output_path(ipfs::IpfsRequest const&);
std::string output_path(ipfs::Response const&);
