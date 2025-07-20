

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <tempo_utils/log_message.h>

#include "test_http_server.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


TestHttpServer::TestHttpServer(
    std::string_view address,
    unsigned short port,
    const std::filesystem::path &contentRoot,
    int concurrencyHint)
{
    m_address = boost::asio::ip::make_address(address);
    m_port = port;
    m_contentRoot = contentRoot;
    m_concurrencyHint = concurrencyHint;
}

boost::asio::ip::address
TestHttpServer::getAddress() const
{
    return m_address;
}

unsigned short
TestHttpServer::getPort() const
{
    return m_port;
}

std::filesystem::path
TestHttpServer::getContentRoot() const
{
    return m_contentRoot;
}

int
TestHttpServer::getConcurrencyHint() const
{
    return m_concurrencyHint;
}

tempo_utils::Result<bool>
TestHttpServer::error(
    tcp::socket &socket,
    const http::request<http::string_body> &req,
    http::status status,
    std::string_view message) const
{
    http::response<http::string_body> rsp{status, req.version()};
    rsp.keep_alive(req.keep_alive());
    rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    if (!message.empty()) {
        rsp.set(http::field::content_type, "text/plain");
        rsp.body() = message;
    }
    rsp.prepare_payload();

    http::message_generator msg(std::move(rsp));

    // Send the response
    beast::error_code ec;
    beast::write(socket, std::move(msg), ec);
    return req.keep_alive();
}

tempo_utils::Result<bool>
TestHttpServer::handleGET(tcp::socket &socket, const http::request<http::string_body> &req) const
{
    auto path = m_contentRoot / std::string(req.target());
    auto keep_alive = req.keep_alive();

    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;

    body.open(path.c_str(), beast::file_mode::scan, ec);
    if (ec == beast::errc::no_such_file_or_directory)
        return error(socket, req, http::status::not_found, "file not found");
    if(ec)
        return error(socket, req, http::status::internal_server_error, ec.message());

    auto const size = body.size();

    // Respond to GET request
    http::response<http::file_body> rsp{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    rsp.set(http::field::content_type, "application/octet-stream");
    rsp.content_length(size);
    rsp.keep_alive(keep_alive);

    http::message_generator msg(std::move(rsp));

    // Send the response
    beast::write(socket, std::move(msg), ec);
    return keep_alive;
}

tempo_utils::Status
handle_request(
    tcp::socket &socket,
    beast::flat_buffer &buffer,
    std::shared_ptr<const TestHttpServer> server,
    bool &keep_alive)
{
    http::request<http::string_body> req;
    beast::error_code ec;

    // close the connection unless directed otherwise
    keep_alive = false;

    // Read a request
    http::read(socket, buffer, req, ec);
    if(ec == http::error::end_of_stream)
        return {};
    if (ec)
        return tempo_utils::GenericStatus::forCondition(
            tempo_utils::GenericCondition::kInternalViolation,
            "http read failed: {}", ec.message());

    // dispatch based on request method
    switch (req.method()) {
        case http::verb::get:
            TU_ASSIGN_OR_RAISE (keep_alive, server->handleGET(socket, req));
            break;
        default:
            break;
    }

    return {};
}

// Handles an HTTP server connection
void
handle_session(tcp::socket &socket, std::shared_ptr<const TestHttpServer> server)
{
    beast::flat_buffer buffer;

    bool keep_alive = false;
    do {
        auto status = handle_request(socket, buffer, server, keep_alive);
        if (status.notOk()) {
            TU_LOG_ERROR << "failed to handle request: " << status;
            keep_alive = false;
        }
    } while (keep_alive);

    // Send a TCP shutdown
    socket.shutdown(tcp::socket::shutdown_send);
}

void
run_acceptor(
    boost::asio::io_context &ioc,
    boost::asio::ip::tcp::endpoint endpoint,
    std::shared_ptr<const TestHttpServer> server)
{
    // The acceptor receives incoming connections
    tcp::acceptor acceptor{ioc, endpoint};

    while (!ioc.stopped())
    {
        // This will receive the new connection
        tcp::socket socket{ioc};

        // Block until we get a connection
        acceptor.accept(socket);

        // Launch the session, transferring ownership of the socket
        std::thread t{std::bind(&handle_session, std::move(socket), server)};
        t.detach();
    }
}

Listener::Listener(std::shared_ptr<const TestHttpServer> server)
    : m_ioc(server->getConcurrencyHint())
{
    m_ioc.run();
    m_endpoint = boost::asio::ip::tcp::endpoint(server->getAddress(), server->getPort());
    m_server = std::move(server);
    m_thread = std::thread{std::bind(&run_acceptor, std::ref(m_ioc), m_endpoint, m_server)};
}

tempo_utils::Status
Listener::stop()
{
    m_ioc.stop();
    m_thread.join();
    return {};
}

tempo_utils::Status
TestHttpServer::start()
{
    if (m_listener)
        return tempo_utils::GenericStatus::forCondition(
            tempo_utils::GenericCondition::kInternalViolation, "server is already started");

    m_listener = std::make_shared<Listener>(shared_from_this());
    return {};
}

tempo_utils::Status TestHttpServer::stop()
{
    if (!m_listener)
        return {};

    auto listener = std::move(m_listener);
    return listener->stop();
}

std::shared_ptr<TestHttpServer>
TestHttpServer::create(
    std::string_view address,
    unsigned short port,
    const std::filesystem::path &contentRoot,
    int concurrencyHint)
{
    auto *ptr = new TestHttpServer(address, port, contentRoot, concurrencyHint);
    return std::shared_ptr<TestHttpServer>(ptr);
}
