#ifndef IREQUESTMATCHER_HPP
#define IREQUESTMATCHER_HPP
#include "../communication/RequestHTTP.hpp"
#include "../config/Config.hpp"
#include "../utils/LightString.hpp"
#include "../utils/http.hpp"
#include <map>

struct RequestMatchingResult {

    typedef std::map<HTTP::t_status, HTTP::byte_string> status_dict_type;

    // CGIかどうか
    bool is_cgi;

    // リダイレクトかどうか
    bool is_redirect;
    std::pair<int, HTTP::byte_string> redirect;

    // リクエストターゲットにマッチしたローカルファイルのパス
    // CGIでいう`SCRIPT_NAME`に相当
    HTTP::byte_string path_local;

    // リクエストターゲットのうちファイルパスにマッチするパートより後の部分
    // CGIでいう`PATH_INFO`に相当
    HTTP::byte_string path_after;

    // リクエスト先がCGIである場合、かつエグゼキュータが特定できた場合、エグゼキュータのパスが入る。
    HTTP::byte_string path_cgi_executor;

    // あるHTTPステータスの時はこのファイルの中身を返す, という時の
    // ステータスとファイルパスの辞書.
    status_dict_type status_page_dict;

    long client_max_body_size;
};
class IRequestMatcher {
public:
    ~IRequestMatcher() {}

    virtual RequestMatchingResult request_match(const std::vector<config::Config> &configs, const RequestHTTP &request)
        = 0;
};

#endif
