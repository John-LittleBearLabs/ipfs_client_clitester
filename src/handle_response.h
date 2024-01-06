#pragma once

namespace ipfs {
class IpfsRequest;
class Response;
}

void handle_response(ipfs::IpfsRequest const&, ipfs::Response const&);
