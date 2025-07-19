#ifndef ZURI_DISTRIBUTOR_TEST_HTTP_SERVER_H
#define ZURI_DISTRIBUTOR_TEST_HTTP_SERVER_H

#include <filesystem>

#include <tempo_utils/result.h>

class TestHttpServer : public std::enable_shared_from_this<TestHttpServer> {
public:
    static std::shared_ptr<TestHttpServer> create(
        std::string_view address,
        unsigned short port,
        const std::filesystem::path &contentRoot);

    void run();

    tempo_utils::Result<bool> handleGET(tcp::socket &socket, const http::request<http::string_body> &req) const;

private:
    boost::asio::ip::address m_address;
    unsigned short m_port;
    std::shared_ptr<std::filesystem::path> m_contentRoot;

    TestHttpServer(
        std::string_view address,
        unsigned short port,
        const std::filesystem::path &contentRoot);
};

#endif // ZURI_DISTRIBUTOR_TEST_HTTP_SERVER_H
