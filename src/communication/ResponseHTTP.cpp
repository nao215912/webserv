#include "ResponseHTTP.hpp"
#include <unistd.h>

ResponseHTTP::ResponseHTTP(HTTP::t_version version,
                           HTTP::t_status status,
                           const header_list_type *headers,
                           IResponseDataConsumer *data_consumer)
    : version_(version)
    , status_(status)
    , is_error_(false)
    , lifetime(Lifetime::make_response())
    , sent_size(0)
    , data_consumer_(data_consumer)
    , should_close_(false) {
    if (headers != NULL) {
        for (header_list_type::const_iterator it = headers->begin(); it != headers->end(); ++it) {
            feed_header(it->first, it->second);
        }
    }
    lifetime.activate();
}

ResponseHTTP::ResponseHTTP(HTTP::t_version version, http_error error, bool should_close)
    : version_(version)
    , status_(error.get_status())
    , is_error_(true)
    , lifetime(Lifetime::make_response())
    , sent_size(0)
    , data_consumer_(NULL)
    , should_close_(should_close) {
    lifetime.activate();
    local_datalist.inject("", 0, true);
    local_datalist.determine_sending_mode();
}

ResponseHTTP::~ResponseHTTP() {}

void ResponseHTTP::set_version(HTTP::t_version version) {
    version_ = version;
}

void ResponseHTTP::set_status(HTTP::t_status status) {
    status_ = status;
}

void ResponseHTTP::feed_header(const HTTP::header_key_type &key, const HTTP::header_val_type &val) {
    if (header_dict.find(key) != header_dict.end() && !is_error_) {
        throw std::runtime_error("Invalid Response: Duplicate header");
    }
    header_dict[key] = val;
    header_list.push_back(HTTP::header_kvpair_type(key, val));
}

HTTP::byte_string ResponseHTTP::serialize_former_part() {
    // 状態行
    message_text = HTTP::version_str(version_) + ParserHelper::SP + ParserHelper::utos(status_, 10) + ParserHelper::SP
                   + HTTP::reason(status_) + ParserHelper::CRLF;
    // ヘッダ
    for (std::vector<HTTP::header_kvpair_type>::iterator it = header_list.begin(); it != header_list.end(); ++it) {
        message_text
            += it->first + ParserHelper::HEADER_KV_SPLITTER + ParserHelper::SP + it->second + ParserHelper::CRLF;
    }
    // 空行
    message_text += ParserHelper::CRLF;
    return message_text;
}

void ResponseHTTP::start() {
    if (should_close_) {
        feed_header(HeaderHTTP::connection, HTTP::strfy("close"));
    }
    consumer()->start(serialize_former_part());
}

const ResponseHTTP::byte_string &ResponseHTTP::get_message_text() const {
    return message_text;
}

const char *ResponseHTTP::get_unsent_head() {
    consumer()->serialize_if_needed();
    return consumer()->serialized_head();
}

void ResponseHTTP::mark_sent(ssize_t sent) {
    if (sent < 0) {
        return;
    }
    consumer()->mark_sent(sent);
    if (is_complete()) {
        lifetime.deactivate();
    }
}

size_t ResponseHTTP::get_unsent_size() const {
    return consumer()->rest_serialized();
}

bool ResponseHTTP::is_complete() const {
    // VOUT(status_);
    // VOUT(getpid());
    // VOUT(this);
    // VOUT(consumer());
    return consumer() != NULL && consumer()->is_sending_over();
}

void ResponseHTTP::swap(ResponseHTTP &lhs, ResponseHTTP &rhs) {
    std::swap(lhs.version_, rhs.version_);
    std::swap(lhs.status_, rhs.status_);
    std::swap(lhs.is_error_, rhs.is_error_);
    std::swap(lhs.sent_size, rhs.sent_size);
    std::swap(lhs.header_list, rhs.header_list);
    std::swap(lhs.header_dict, rhs.header_dict);
    std::swap(lhs.body, rhs.body);
    std::swap(lhs.message_text, rhs.message_text);
    std::swap(lhs.local_datalist, rhs.local_datalist);
    std::swap(lhs.data_consumer_, rhs.data_consumer_);
}

bool ResponseHTTP::is_error() const {
    return is_error_;
}

bool ResponseHTTP::is_timeout(t_time_epoch_ms now) const {
    return lifetime.is_timeout(now);
}

bool ResponseHTTP::should_close() const {
    return should_close_;
}

IResponseDataConsumer *ResponseHTTP::consumer() {
    return (data_consumer_ != NULL) ? data_consumer_ : &local_datalist;
}

const IResponseDataConsumer *ResponseHTTP::consumer() const {
    return (data_consumer_ != NULL) ? data_consumer_ : &local_datalist;
}
