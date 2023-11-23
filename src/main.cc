#include <ipfs_client/all_inclusive.h>
#include <ipfs_client/ipfs_request.h>
#include <ipfs_client/logger.h>

#include <boost/asio/io_context.hpp>

#include <iterator>
#include <iostream>
#include <fstream>
#include <thread>

namespace as = boost::asio;
using namespace std::chrono_literals;

namespace {
    std::vector<std::shared_ptr<ipfs::IpfsRequest>> requests;
    void handle_response(ipfs::IpfsRequest const&, ipfs::Response const&);
}

int main(int const argc, char const* const argv[]) {
    as::io_context io;
    SetLevel(ipfs::log::Level::TRACE);
    auto ctxt = ipfs::start_default(io);
    std::time_t running = std::time(nullptr);
    auto yield = [&](){
            while (io.run()) {
                std::clog.put('.');
                std::this_thread::sleep_for(1ms);
                running = std::time(nullptr);
            }
        };
    auto send = [&](std::string const& url){
        auto req = ipfs::IpfsRequest::fromUrl(url, handle_response);
        requests.push_back(req);
        ctxt->build_response(req);
        yield();
    };
    std::for_each(std::next(argv), std::next(argv, argc), send);
    if (argc < 2) {
        std::string url;
        while (std::getline(std::cin, url)) {
            send(url);
            yield();
        }
    }
    while (running + 99 > std::time(nullptr)) {
        yield();
        std::this_thread::sleep_for(1s);
        yield();
    }
}

namespace {
    void handle_response(ipfs::IpfsRequest const& req, ipfs::Response const& res) {
        std::clog << req.path().to_string() << " finished status " << res.status_ << std::endl;
        std::string file_name{req.path().to_string()};
        for (auto& c : file_name) {
            if (!std::isalnum(c)) {
                c = '_';
            }
        }
        std::ofstream f{file_name};
        f.write(res.body_.c_str(), res.body_.size());
        auto i = std::find_if(requests.begin(), requests.end(), [&req](auto&p){return p.get()==&req;});
        if (requests.end() != i) {
            std::clog << "popping done request.\n";
            requests.erase(i);
        }
    }
}
