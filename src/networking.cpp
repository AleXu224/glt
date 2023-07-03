#include "networking.hpp"
#include <asio/error_code.hpp>
#include <asio/ssl/verify_mode.hpp>
#include <charconv>
#include <iostream>
#include <skyr/v1/url.hpp>
#include <sstream>
#include <string>
#include "asio/ssl.hpp"
#include "skyr/url.hpp"

using namespace squi;

asio::io_context Networking::ioContext;
std::vector<std::thread> Networking::threads;

Networking::ResponseBody Networking::parseResponse(std::string_view response) {
    ResponseBody body;
    const auto bodyStart = response.find("\r\n\r\n");
    const auto header = response.substr(0, bodyStart);
    const auto bodyStr = response.substr(bodyStart + 4);

    auto pos = header.begin();
    do {
        const auto lineEnd = std::find(pos, header.end(), '\r');
        const auto line = header.substr(pos - header.begin(), lineEnd - pos);
        
        if (line.starts_with("HTTP/1.1")) {
            const auto statusCode = line.substr(9, 3);
            body.statusCode = std::stoi(std::string{statusCode});
        } else {
            const auto colon = line.find(':');
            const auto key = line.substr(0, colon);
            const auto value = line.substr(colon + 2);
            body.headers.emplace(key, value);
        }

        if (lineEnd == header.end()) break;
        pos = lineEnd + 2;
    } while (pos != header.end());

    body.body = std::string{bodyStr};

    return body;
}

Networking::Response Networking::get(std::string_view url) {
    const auto parsedUrl = skyr::url{url};
    bool isHttps = parsedUrl.protocol().starts_with("https");
    asio::ip::tcp::resolver resolver(ioContext);
    asio::ip::tcp::socket socket(ioContext);
    asio::ssl::context context(asio::ssl::context::method::tls_client);
    asio::ssl::stream<asio::ip::tcp::socket&> stream(socket, context);
    asio::error_code ec;
    const auto protocol = parsedUrl.protocol().substr();
    auto endpoint = resolver.resolve(parsedUrl.host(), protocol.substr(0, protocol.size() - 1), ec);
    if (ec) {
        return Response{
            .body = "",
            .success = false,
            .error = std::format("Failed to resolve {}: {}", parsedUrl.host(), ec.message()),
        };
    }

    if (isHttps) {
        context.set_verify_mode(asio::ssl::verify_peer);
        context.set_default_verify_paths();
    }
    stream.lowest_layer().connect(endpoint->endpoint(), ec);
    if (ec) {
        return Response{
            .body = "",
            .success = false,
            .error = std::format("Failed to connect to {}: {}", parsedUrl.host(), ec.message()),
        };
    }

    SSL_set_tlsext_host_name(stream.native_handle(), parsedUrl.host().data());

    stream.handshake(asio::ssl::stream_base::client, ec);
    if (ec) {
        return Response{
            .body = "",
            .success = false,
            .error = std::format("Failed to handshake with {}: {}", parsedUrl.host(), ec.message()),
        };
    }

	std::stringstream ss;
    ss << parsedUrl.pathname();
    if (!parsedUrl.search_parameters().empty()) ss << "?" << parsedUrl.search_parameters().to_string();

    stream.write_some(asio::buffer(std::format("GET {} HTTP/1.1\r\nHost: {}\r\nAccept: Accept: */*\r\nConnection: close\r\n\r\n", ss.str(), parsedUrl.host())), ec);
	if (ec) {
        return Response{
            .body = "",
            .success = false,
            .error = std::format("Failed to send request to {}: {}", parsedUrl.host(), ec.message()),
        };
    }

    std::string response;
    std::array<char, 10 * 1024> buffer{};
    while (true) {
        size_t len = stream.read_some(asio::buffer(buffer), ec);
        if (!ec) {
            // printf("Got %zu bytes\n", len);
            response.append(buffer.data(), len);
        } else {
            break;
        }
    }

    auto body = parseResponse(response);

    return Response{
        .body = body.body,
        .statusCode = body.statusCode,
        .headers = body.headers,
        .success = true,
        .error = "",
    };
}