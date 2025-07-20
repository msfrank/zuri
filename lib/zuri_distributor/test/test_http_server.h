#ifndef ZURI_DISTRIBUTOR_TEST_HTTP_SERVER_H
#define ZURI_DISTRIBUTOR_TEST_HTTP_SERVER_H

#include <filesystem>

#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <tempo_utils/result.h>

class Listener;

class TestHttpServer : public std::enable_shared_from_this<TestHttpServer> {
public:
    static std::shared_ptr<TestHttpServer> create(
        std::string_view address,
        unsigned short port,
        const std::filesystem::path &contentRoot,
        int concurrencyHint);

    boost::asio::ip::address getAddress() const;
    unsigned short getPort() const;
    std::filesystem::path getContentRoot() const;
    int getConcurrencyHint() const;

    tempo_utils::Status start();
    tempo_utils::Status stop();

    tempo_utils::Result<bool> error(
        boost::asio::ip::tcp::socket &socket,
        const boost::beast::http::request<boost::beast::http::string_body> &req,
        boost::beast::http::status status,
        std::string_view message = {}) const;
    tempo_utils::Result<bool> handleGET(
        boost::asio::ip::tcp::socket &socket,
        const boost::beast::http::request<boost::beast::http::string_body> &req) const;

private:
    boost::asio::ip::address m_address;
    unsigned short m_port;
    std::filesystem::path m_contentRoot;
    int m_concurrencyHint;

    std::shared_ptr<Listener> m_listener;

    TestHttpServer(
        std::string_view address,
        unsigned short port,
        const std::filesystem::path &contentRoot,
        int concurrencyHint);
};

class Listener {
public:
    explicit Listener(std::shared_ptr<const TestHttpServer> server);

    tempo_utils::Status stop();

private:
    std::shared_ptr<const TestHttpServer> m_server;
    boost::asio::io_context m_ioc;
    boost::asio::ip::tcp::endpoint m_endpoint;
    std::thread m_thread;
};

#endif // ZURI_DISTRIBUTOR_TEST_HTTP_SERVER_H
