#include <ipfs_client/test_context.h>
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

namespace {
    std::vector<std::shared_ptr<ipfs::IpfsRequest>> requests;
    void handle_response(ipfs::IpfsRequest const&, ipfs::Response const&);
    bool set_log_level(std::string const& level_name);
    std::shared_ptr<ipfs::Orchestrator> orc;
    auto running_until = std::chrono::system_clock::now() + 9s;
    std::size_t last_count = 0UL;
    void clean_finished();
    std::string output_path(ipfs::IpfsRequest const&);
    std::string output_path(ipfs::Response const&);
}

int main(int const argc, char const* const argv[]) {
    as::io_context io;
    SetLevel(ipfs::log::Level::WARN);
    orc = ipfs::start_default(io);
    running_until += 9s * argc;
    auto yield = [&](){
            auto handlers_run = io.run_for(60s);
            if (handlers_run) {
              std::clog.put('.');
              clean_finished();
              running_until += 1ms * handlers_run;
            } else {
                std::this_thread::sleep_for(1ms);
                io.reset();
            }
        };
    auto handle_arg = [&](std::string const& arg){
      if (set_log_level(arg)) {
        std::clog << "Log level set to " << arg << ".\n";
      } else {
        auto req = ipfs::IpfsRequest::fromUrl(arg, handle_response);
        requests.push_back(req);
        orc->build_response(req);
      }
    };
    yield();
    std::for_each(std::next(argv), std::next(argv, argc), handle_arg);
    auto still_going = [&](){
      clean_finished();
      if (requests.empty()) {
        return false;
      }
      auto stored = orc->Stored();
      if (stored > last_count) {
        std::clog << stored << " blocks > " << last_count << " blocks.\n";
        running_until += (stored - last_count) * 9s;
        last_count = stored;
        return true;
      }
      auto now = std::chrono::system_clock::now();
      if (running_until > now) {
        return true;
      } else {
        std::clog << "Timed out.\n";
        for (auto& r : requests) {
          std::clog << "  Failed: " << r->path() << '\n';
        }
        return false;
      }
    };
    while (still_going()) {
        for (auto& r : requests) {
          std::clog << "Will request " << r->path() << " hoping to write it to " << output_path(*r) << '\n';
          orc->build_response(r);
        }
        yield();
    }
    std::clog << "Done.\n";
}

namespace {
    std::map<std::string,std::set<int>> stati;
    void handle_response(ipfs::IpfsRequest const& req, ipfs::Response const& res) {
        running_until += 1s;
        clean_finished();
        if (!stati[req.path().to_string()].emplace(res.status_).second) {
          return;
        }
        running_until += 1s;
        std::clog << req.path().to_string() << " got status " << res.status_;
        if (res.status_ == 404) {
            std::clog << " body:" << res.body_;
        }
        if (res.status_ / 100 == 3) {
            std::clog << " location=" << res.location_ ;
            auto i = std::find_if(requests.begin(), requests.end(), [&req](auto&p){return p.get()==&req;});
            auto parent = *i;
            if (requests.end() != i) {
              std::clog << " dropping original request...";
              requests.erase(i);
            }
            auto handler = [parent](auto&req,auto&res){
              std::clog << "Handling a " << res.status_ << " response to "
                        << req.path().to_string()
                        << " which is also a response to "
                        << parent->path().to_string() << '\n';
              handle_response(*parent,res);
              handle_response(req,res);
            };
            auto reqp = ipfs::IpfsRequest::fromUrl(res.location_, handler);
            std::clog << " and now requesting " << reqp->path().to_string() << '\n';
            requests.push_back(reqp);
            orc->build_response(reqp);
            return;
        }
        running_until += 9s * requests.size();
        std::clog << std::endl;

        auto i = std::find_if(requests.begin(), requests.end(), [&req](auto&p){return p.get()==&req;});
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
          if (requests.end() == i) {
            std::clog << "Superflous response.\n";
          } else {
            std::clog << "popping done request.\n";
            requests.erase(i);
          }
        } else if (i != requests.end()) {
          std::clog << "Restarting request " << req.path().to_string() << '\n';
          std::this_thread::sleep_for(1s);
          orc->build_response(*i);
        }
    }
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
    void clean_finished() {
      for (auto it = requests.begin(); it != requests.end(); ++it) {
        using namespace std::filesystem;
        auto f = output_path(**it);
        if (is_regular_file(f)) {
          std::clog << "Check output in " << f << '\n';
          requests.erase(it);
          running_until += 9s;
          return;
        }
      }
    }
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
}
