#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#include "asio/io_context.hpp"
#include "vector"
#include <unordered_map>


namespace squi {
	class Networking {
		static asio::io_context ioContext;
		static std::vector<std::thread> threads;

		struct ResponseBody {
			std::string body;
			uint32_t statusCode;
			std::unordered_map<std::string, std::string> headers;
		};

		static ResponseBody parseResponse(std::string_view response);

	public:
		struct Response {
			std::string body;
			uint32_t statusCode;
			std::unordered_map<std::string, std::string> headers;
			bool success;
			std::string error;
		};

		static Response get(std::string_view url);
	};
}// namespace squi