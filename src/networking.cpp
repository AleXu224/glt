#include "networking.hpp"
// #include "asio/ssl.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/ssl/context.hpp"
#include "asio/ssl/stream.hpp"
#include "print"
#include "skyr/url.hpp"
#include "thread"
#include <algorithm>
#include <asio/error_code.hpp>
#include <asio/ssl/verify_mode.hpp>
#include <charconv>
#include <sstream>
#include <string>


using namespace squi;

namespace {
	asio::io_context ioContext;
	std::vector<std::thread> threads;

	std::string toLowerCase(std::string_view str) {
		std::string ret;
		ret.reserve(str.size());
		for (const auto c: str) {
			ret.push_back(static_cast<char>(std::tolower(c)));
		}
		return ret;
	}
}// namespace


Networking::ResponseBody Networking::parseResponse(std::string_view response) {
	ResponseBody ret;
	// const auto bodyStart = response.find("\r\n\r\n");
	// const auto header = response.substr(0, bodyStart);
	// const auto bodyStr = response.substr(bodyStart + 4);

	auto cursor = response.begin();
	do {
		const auto lineEnd = std::find(cursor, response.end(), '\r');
		const auto line = response.substr(cursor - response.begin(), lineEnd - cursor);

		if (line.starts_with("HTTP/1.1")) {
			const auto statusCode = line.substr(9, 3);
			ret.statusCode = std::stoi(std::string{statusCode});
		} else {
			const auto colon = line.find(':');
			const auto key = toLowerCase(line.substr(0, colon));
			// An empty key means we've reached the body
			if (key.empty()) {
				cursor = lineEnd + 2;
				break;
			}
			auto value = line.substr(colon + 1);
			// Trim whitespace from header value
			auto firstNonSpace = value.find_first_not_of(" \t");
			if (firstNonSpace == std::string_view::npos) {
				value = "";
			} else {
				value = value.substr(firstNonSpace);
				value = std::string_view(value.begin(), std::next(value.begin(), value.find_last_not_of(" \t") + 1));
			}
			ret.headers.emplace(key, value);
		}

		cursor = lineEnd + 2;
	} while (cursor != response.end());

	const auto body = response.substr(cursor - response.begin());
	;

	if (ret.headers.contains("content-length")) {
		const auto contentLength = ret.headers.at("content-length");
		const auto length = std::stoi(std::string{contentLength});
		ret.body = body.substr(0, length);
	} else if (ret.headers.contains("transfer-encoding")) {
		const auto encoding = ret.headers.at("transfer-encoding");
		if (encoding == "chunked") {
			std::stringstream ss;
			auto chunkStart = body.begin();
			while (true) {
				const auto chunkSizeEnd = std::find(chunkStart, body.end(), '\r');
				const auto chunkSizeStr = body.substr(chunkStart - body.begin(), chunkSizeEnd - chunkStart);
				size_t chunkSize = 0;
				std::from_chars(chunkSizeStr.data(), chunkSizeStr.data() + chunkSizeStr.size(), chunkSize, 16);
				if (chunkSize == 0) break;
				const auto chunkEnd = chunkSizeEnd + 2 + static_cast<std::ptrdiff_t>(chunkSize);
				ss << body.substr(chunkSizeEnd - body.begin() + 2, chunkSize);
				chunkStart = chunkEnd + 2;
			}
			ret.body = ss.str();
		} else {
			std::println("Unknown transfer encoding: {}", encoding);
		}
	}

	return ret;
}

Networking::Response Networking::get(const std::string &url) {
	std::string currentUrl = url;
	constexpr int maxRedirects = 10;

	for (int redirectCount = 0; redirectCount <= maxRedirects; redirectCount++) {
		auto parsedUrl = skyr::url{currentUrl};
		const bool isHttps = parsedUrl.protocol().starts_with("https");
		asio::ip::tcp::resolver resolver(ioContext);
		asio::ip::tcp::socket socket(ioContext);
		asio::ssl::context context(asio::ssl::context::method::tls_client);
		asio::ssl::stream<asio::ip::tcp::socket &> stream(socket, context);
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
		ec = stream.lowest_layer().connect(endpoint->endpoint(), ec);
		if (ec) {
			return Response{
				.body = "",
				.success = false,
				.error = std::format("Failed to connect to {}: {}", parsedUrl.host(), ec.message()),
			};
		}

		SSL_set_tlsext_host_name(stream.native_handle(), parsedUrl.host().data());

		ec = stream.handshake(asio::ssl::stream_base::client, ec);
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

		stream.write_some(asio::buffer(std::format("GET {} HTTP/1.1\r\nHost: {}\r\nUser-Agent: glt-net\r\nAccept: */*\r\nConnection: close\r\nCache-Control: no-cache\r\n\r\n", ss.str(), parsedUrl.host())), ec);
		if (ec) {
			return Response{
				.body = "",
				.success = false,
				.error = std::format("Failed to send request to {}: {}", parsedUrl.host(), ec.message()),
			};
		}

		std::string response;
		std::array<char, 10ull * 1024> buffer{};
		while (true) {
			size_t len = stream.read_some(asio::buffer(buffer), ec);
			if (!ec) {
				response.append(buffer.data(), len);
			} else {
				break;
			}
		}

		auto body = parseResponse(response);

		if (body.statusCode == 301 || body.statusCode == 302 || body.statusCode == 303 || body.statusCode == 307 || body.statusCode == 308) {
			if (body.headers.contains("location")) {
				std::string location = body.headers.at("location");
				if (location.starts_with('/')) {
					location = parsedUrl.protocol().substr() + "://" + parsedUrl.host() + location;
				}
				currentUrl = location;
				continue;
			}
		}

		return Response{
			.body = body.body,
			.statusCode = body.statusCode,
			.headers = body.headers,
			.success = true,
			.error = "",
		};
	}

	return Response{
		.body = "",
		.success = false,
		.error = "Too many redirects",
	};
}