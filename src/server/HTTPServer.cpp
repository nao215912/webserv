#include "HTTPServer.hpp"
#include "../config/Parser.hpp"
#include "../router/RequestMatcher.hpp"
#include "../utils/File.hpp"

RequestMatchingResult MockMatcher::request_match(const std::vector<config::Config> &configs,
                                                 const IRequestMatchingParam &param) {
    (void)configs;
    const RequestTarget &given_target = param.get_request_target();
    RequestMatchingResult result(&given_target);
    const HTTP::light_string path   = given_target.dpath();
    HTTP::light_string::size_type i = path.rfind(".rb");
    if (i != HTTP::light_string::npos && i + strlen(".rb") == path.size()) {
        result.result_type = RequestMatchingResult::RT_CGI;
    } else if (path.size() > 0 && path[path.size() - 1] == '/') {
        result.result_type = RequestMatchingResult::RT_AUTO_INDEX;
    } else {
        result.result_type = RequestMatchingResult::RT_FILE;
    }
    result.path_local           = HTTP::strfy(".") + given_target.dpath();
    result.path_cgi_executor    = HTTP::strfy("/usr/bin/ruby");
    result.status_code          = HTTP::STATUS_MOVED_PERMANENTLY;
    result.redirect_location    = HTTP::strfy("/mmmmm");
    result.client_max_body_size = 1024; // 仮
    return result;
}

HTTPServer::HTTPServer(IObserver *observer) : socket_observer_(observer) {}

HTTPServer::~HTTPServer() {
    delete socket_observer_;
}

void HTTPServer::init(const std::string &config_path) {
    file::ErrorType err;
    if ((err = file::check(config_path)) != file::NONE) {
        throw std::runtime_error(file::error_message(err));
    }

    config::Parser parser;
    configs_ = parser.parse(file::read(config_path));

    for (config::config_dict::const_iterator it = configs_.begin(); it != configs_.end(); ++it) {
        const config::host_port_pair &hp     = it->first;
        const config::config_vector &configs = it->second;
        this->listen(SD_IP4, ST_TCP, hp.second, configs, cacher_);
    }
}

void HTTPServer::listen(t_socket_domain sdomain,
                        t_socket_type stype,
                        t_port port,
                        const config::config_vector &configs,
                        FileCacher &cacher) {
    Channel *ch            = new Channel(this, sdomain, stype, port, configs, cacher);
    channels[ch->get_id()] = ch;
    socket_observer_->reserve_hold(ch);
    socket_observer_->reserve_set(ch, IObserver::OT_READ);
}

void HTTPServer::run() {
    socket_observer_->loop();
}

RequestMatchingResult HTTPServer::route(const IRequestMatchingParam &matching_param,
                                        const config::config_vector &configs) {

    // Connectionに紐づくserver群に対し, リクエストを渡してマッチングを要請.
    // その結果を使ってここでオリジネータを生成する.
    RequestMatcher rm;
    return rm.request_match(configs, matching_param);
}
