#include "NetworkStreamHandler.h"
using asio::ip::tcp;

ByteVector getData(std::string IP, uint32_t port) {
    try {
        asio::io_context io;

        asio::ip::address address = asio::ip::make_address(IP);
        tcp::acceptor acceptor(io, tcp::endpoint(address, port));
        std::cout << "Server listening on port " << port << "...\n";

        tcp::socket socket(io);
        acceptor.accept(socket); // wait for client to connect
        std::cout << "Client connected from " << socket.remote_endpoint() << "\n";

        uint32_t len_net;
        asio::read(socket, asio::buffer(&len_net, sizeof(len_net)));
        uint32_t len = ntohl(len_net);

        ByteVector data(len);
        asio::read(socket, asio::buffer(data.data(), len));
        return data;
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return {};
}

void sendData(std::string IP, uint32_t port, ByteVector data) {
    if (data.size() <= 0) return;
    try {
        asio::io_context io;

        tcp::resolver resolver(io);
        tcp::resolver::results_type endpoints = resolver.resolve(IP, std::to_string(port));

        tcp::socket socket(io);
        asio::connect(socket, endpoints);

        uint32_t len = htonl(data.size());   // network byte order
        asio::write(socket, asio::buffer(&len, sizeof(len)));
        asio::write(socket, asio::buffer(data));

        std::cout << "Data sent.\n";
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
