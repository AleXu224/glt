#pragma once

#include "cstdint"
#include "string"
#include <unordered_map>


namespace squi {
	class Networking {
		struct ResponseBody {
			std::string body;
			uint32_t statusCode;
			std::unordered_map<std::string, std::string> headers;
		};

		static ResponseBody parseResponse(std::string_view response);

	public:
		struct Response {
			std::string body;
			uint32_t statusCode = 0;
			std::unordered_map<std::string, std::string> headers{};
			bool success;
			std::string error;
		};

		static Response get(const std::string &url);
	};
}// namespace squi