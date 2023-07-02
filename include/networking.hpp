#pragma once

#include <unordered_map>
#define ASIO_STANDALONE
#include "asio.hpp"
#include "vector"

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
}