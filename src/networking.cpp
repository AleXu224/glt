#include "networking.hpp"

#include "httplib.h"
#include "skyr/url.hpp"

#include <cctype>
#include <format>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>


using namespace squi;

namespace {
	std::string toLowerCase(std::string_view str) {
		std::string ret;
		ret.reserve(str.size());
		for (const auto c: str) {
			ret.push_back(static_cast<char>(std::tolower(c)));
		}
		return ret;
	}

	void appendHeaders(httplib::Headers &target, const std::unordered_map<std::string, std::string> &headers) {
		for (const auto &[key, value]: headers) {
			target.emplace(key, value);
		}
	}
}// namespace


Networking::Response Networking::get(const std::string &url, const std::unordered_map<std::string, std::string> &headers) {
	auto parsedUrl = skyr::url{url};

	const auto schemeHostPort = std::format("{}//{}", parsedUrl.protocol(), parsedUrl.host());
	httplib::Client client{schemeHostPort};

	if (!client.is_valid()) {
		return Response{
			.body = "",
			.statusCode = 0,
			.headers = {},
			.success = false,
			.error = std::format("Invalid URL: {}", url),
		};
	}

	client.set_follow_location(true);
	client.set_connection_timeout(30);
	client.set_read_timeout(30);
	client.set_write_timeout(30);

	httplib::Headers requestHeaders{
		{"User-Agent", "glt-net"},
		{"Accept", "*/*"},
		{"Cache-Control", "no-cache"},
	};
	appendHeaders(requestHeaders, defaultHeaders);
	appendHeaders(requestHeaders, headers);

	const auto path = std::format("{}{}", parsedUrl.pathname(), parsedUrl.search());
	auto response = client.Get(path, requestHeaders);

	if (!response) {
		return Response{
			.body = "",
			.statusCode = 0,
			.headers = {},
			.success = false,
			.error = httplib::to_string(response.error()),
		};
	}

	std::unordered_map<std::string, std::string> responseHeaders;
	responseHeaders.reserve(response->headers.size());
	for (const auto &[key, value]: response->headers) {
		responseHeaders.emplace(toLowerCase(key), value);
	}

	return Response{
		.body = std::move(response->body),
		.statusCode = static_cast<uint32_t>(response->status),
		.headers = std::move(responseHeaders),
		.success = true,
		.error = "",
	};
}
