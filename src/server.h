#include <arpa/inet.h>
#include <cstring>
#include <format>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <streambuf>
#include <string>
#include <unistd.h>

#include "errors.h"

#ifndef SERVER_H

#define SERVER_H

namespace Database::Utils {

class SocketStreamBuf : public std::streambuf {
public:
    explicit SocketStreamBuf(int socket_fd)
        : sockfd(socket_fd)
    {
    }

protected:
    int overflow(int c) override
    {
        if (c == EOF)
            return 0;
        char ch = static_cast<char>(c);
        if (send(sockfd, &ch, 1, 0) < 0) {
            return EOF;
        }
        return c;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        ssize_t sent = send(sockfd, s, n, 0);
        if (sent < 0)
            return 0;
        return sent;
    }

private:
    int sockfd;
};

class SocketOStream : public std::iostream {
public:
    explicit SocketOStream(int socket_fd)
        : std::iostream(nullptr)
        , sockfd(socket_fd)
        , buf(socket_fd)
    {
        rdbuf(&buf);
    }

    // Call this when you're done
    void Close()
    {
        if (sockfd >= 0) {
            ::close(sockfd);
            sockfd = -1;
        }
    }

    // Destructor ensures cleanup if not already closed
    ~SocketOStream()
    {
        Close();
    }

private:
    int sockfd;
    SocketStreamBuf buf;
};

class Server {
public:
    // Utility function to connect and return a SocketOStream
    static auto ConnectTcpStream(const std::string& address, int port) -> SocketOStream
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            throw Errors::Error(Errors::ErrorType::RuntimeError, "Failed to create socket", 0, 0, Errors::ERROR_NETWORK_FAILURE);
        sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr) <= 0) {
            close(sockfd);

            throw Errors::Error(Errors::ErrorType::RuntimeError, std::format("Invalid address: {}", address), 0, 0, Errors::ERROR_NETWORK_FAILURE);
        }

        if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sockfd);

            throw Errors::Error(Errors::ErrorType::RuntimeError, std::format("Connection failed to {}:{}", address, std::to_string(port)), 0, 0, Errors::ERROR_NETWORK_FAILURE);
        }

        return SocketOStream(sockfd);
    }

    static auto ReadStream(SocketOStream& stream) -> const std::string
    {
        stream.seekp(stream.end);

        int length = stream.tellp();

        char* buffer = new char[length];

        stream.read(buffer, length);

        std::string res = std::string(buffer);

        delete[] buffer;

        return res;
    }

    static auto PrintStream(SocketOStream& stream, const std::string& output) -> void
    {
        stream << output << std::endl;
    }
};
}

#endif
