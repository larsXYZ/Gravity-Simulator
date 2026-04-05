#pragma once

#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

class Space;

struct UdpCommand {
    std::string data;
    sf::IpAddress sender;
    unsigned short senderPort;
};

class UdpCommandServer {
    unsigned short port;
    sf::UdpSocket recvSocket;
    sf::UdpSocket sendSocket;
    std::vector<UdpCommand> queue;
    std::mutex queueMutex;
    std::thread listenerThread;
    std::atomic<bool> running{false};

    void listenerLoop();
    void sendReply(const sf::IpAddress& addr, unsigned short port, const std::string& msg);
    std::string handleCommand(const std::string& line, Space& space, sf::View& view, sf::RenderWindow& window);

public:
    explicit UdpCommandServer(unsigned short port);
    ~UdpCommandServer();

    bool start();
    void stop();
    bool processCommands(Space& space, sf::View& view, sf::RenderWindow& window);
};
