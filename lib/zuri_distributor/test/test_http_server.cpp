

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

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

#include "test_http_server.h"

TestHttpServer::TestHttpServer(
    std::string_view address,
    unsigned short port,
    const std::filesystem::path &contentRoot)
{
    m_address = net::ip::make_address(address);
    m_port = port;
    m_contentRoot = std::make_shared<std::filesystem::path>(contentRoot);
}

template <class Body, class Allocator>
http::message_generator
handle_request(beast::string_view doc_root, http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // Make sure we can handle the method
    if( req.method() != http::verb::get &&
        req.method() != http::verb::head)
        return bad_request("Unknown HTTP-method");

    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return bad_request("Illegal request-target");

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if(req.target().back() == '/')
        path.append("index.html");

    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if(ec == beast::errc::no_such_file_or_directory)
        return not_found(req.target());

    // Handle an unknown error
    if(ec)
        return server_error(ec.message());

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if(req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    return res;
}

tempo_utils::Result<bool>
TestHttpServer::handleGET(tcp::socket &socket, const http::request<http::string_body> &req) const
{
    auto path = *m_contentRoot / std::string(req.target());
    auto keep_alive = req.keep_alive();

    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if(ec == beast::errc::no_such_file_or_directory)
        return not_found(req.target());

    // Handle an unknown error
    if(ec)
        return server_error(ec.message());

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

    // Send the response
    beast::error_code ec;
    beast::write(socket, std::move(rsp), ec);
    TU_LOG_FATAL_IF(ec) << "failed to write response: " << ec.message();

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
TestHttpServer::run()
{
    // The io_context is required for all I/O
    net::io_context ioc{1};

    // The acceptor receives incoming connections
    tcp::acceptor acceptor{ioc, {m_address, m_port}};
    for (;;)
    {
        // This will receive the new connection
        tcp::socket socket{ioc};

        // Block until we get a connection
        acceptor.accept(socket);

        // Launch the session, transferring ownership of the socket
        std::thread t{std::bind(&handle_session, std::move(socket), shared_from_this())};
        t.detach();
    }
}

std::shared_ptr<TestHttpServer>
TestHttpServer::create(
    std::string_view address,
    unsigned short port,
    const std::filesystem::path &contentRoot)
{
    return std::shared_ptr<TestHttpServer>(new TestHttpServer(address, port, contentRoot));
}
