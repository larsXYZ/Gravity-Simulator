#include "udp_server.h"
#include "space.h"
#include <sstream>
#include <iostream>

UdpCommandServer::UdpCommandServer(unsigned short port) : port(port) {}

UdpCommandServer::~UdpCommandServer()
{
    stop();
}

bool UdpCommandServer::start()
{
    recvSocket.setBlocking(false);
    if (recvSocket.bind(port) != sf::Socket::Done)
    {
        std::cerr << "UDP server: failed to bind port " << port << std::endl;
        return false;
    }

    running = true;
    listenerThread = std::thread(&UdpCommandServer::listenerLoop, this);
    std::cout << "UDP server listening on port " << port << std::endl;
    return true;
}

void UdpCommandServer::stop()
{
    running = false;
    recvSocket.unbind();
    if (listenerThread.joinable())
        listenerThread.join();
}

void UdpCommandServer::listenerLoop()
{
    recvSocket.setBlocking(true);
    char buffer[4096];
    std::size_t received;
    sf::IpAddress sender;
    unsigned short senderPort;

    while (running)
    {
        // Use non-blocking with a short timeout approach:
        // setBlocking(true) would block forever on shutdown, so use non-blocking + sleep
        recvSocket.setBlocking(false);
        auto status = recvSocket.receive(buffer, sizeof(buffer) - 1, received, sender, senderPort);

        if (status == sf::Socket::Done)
        {
            buffer[received] = '\0';
            std::string data(buffer, received);
            // Trim trailing newline/whitespace
            while (!data.empty() && (data.back() == '\n' || data.back() == '\r' || data.back() == ' '))
                data.pop_back();

            if (!data.empty())
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                queue.push_back({std::move(data), sender, senderPort});
            }
        }
        else if (status == sf::Socket::NotReady)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        else
        {
            // Error or disconnected — exit loop
            break;
        }
    }
}

void UdpCommandServer::sendReply(const sf::IpAddress& addr, unsigned short port, const std::string& msg)
{
    sendSocket.send(msg.c_str(), msg.size(), addr, port);
}

void UdpCommandServer::processCommands(Space& space, sf::View& view, sf::RenderWindow& window)
{
    std::vector<UdpCommand> local;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        local.swap(queue);
    }

    for (auto& cmd : local)
    {
        std::string response = handleCommand(cmd.data, space, view, window);
        if (!response.empty())
            sendReply(cmd.sender, cmd.senderPort, response);
    }
}

static std::string bodyTypeToString(BodyType type)
{
    switch (type)
    {
        case ROCKY:       return "ROCKY";
        case TERRESTRIAL: return "TERRESTRIAL";
        case GASGIANT:    return "GASGIANT";
        case BROWNDWARF:  return "BROWNDWARF";
        case STAR:        return "STAR";
        case WHITEDWARF:  return "WHITEDWARF";
        case NEUTRONSTAR: return "NEUTRONSTAR";
        case BLACKHOLE:   return "BLACKHOLE";
        default:          return "UNKNOWN";
    }
}

static bool parseBodyType(const std::string& name, BodyType& out)
{
    if (name == "ROCKY")       { out = ROCKY; return true; }
    if (name == "TERRESTRIAL") { out = TERRESTRIAL; return true; }
    if (name == "GASGIANT")    { out = GASGIANT; return true; }
    if (name == "BROWNDWARF")  { out = BROWNDWARF; return true; }
    if (name == "STAR")        { out = STAR; return true; }
    if (name == "WHITEDWARF")  { out = WHITEDWARF; return true; }
    if (name == "NEUTRONSTAR") { out = NEUTRONSTAR; return true; }
    if (name == "BLACKHOLE")   { out = BLACKHOLE; return true; }
    return false;
}

static sf::Keyboard::Key parseKeyName(const std::string& name)
{
    if (name == "P") return sf::Keyboard::P;
    if (name == "R") return sf::Keyboard::R;
    if (name == "N") return sf::Keyboard::N;
    if (name == "O") return sf::Keyboard::O;
    if (name == "A") return sf::Keyboard::A;
    if (name == "D") return sf::Keyboard::D;
    if (name == "C") return sf::Keyboard::C;
    if (name == "G") return sf::Keyboard::G;
    if (name == "S") return sf::Keyboard::S;
    if (name == "Q") return sf::Keyboard::Q;
    if (name == "I") return sf::Keyboard::I;
    if (name == "F") return sf::Keyboard::F;
    if (name == "B") return sf::Keyboard::B;
    if (name == "COMMA") return sf::Keyboard::Comma;
    if (name == "PERIOD") return sf::Keyboard::Period;
    if (name == "F1") return sf::Keyboard::F1;
    return sf::Keyboard::Unknown;
}

std::string UdpCommandServer::handleCommand(const std::string& line, Space& space, sf::View& view, sf::RenderWindow& window)
{
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "PING")
    {
        return "PONG";
    }
    else if (cmd == "ADD")
    {
        double x, y, xv, yv, mass;
        if (!(iss >> x >> y >> xv >> yv >> mass))
            return "ERR invalid args";

        Planet p(mass, x, y, xv, yv);

        // Optional type override: ADD <x> <y> <xv> <yv> <mass> [TYPE]
        std::string typeName;
        if (iss >> typeName)
        {
            BodyType type;
            if (!parseBodyType(typeName, type))
                return "ERR unknown type";
            p.setType(type);
            if (type == WHITEDWARF || type == NEUTRONSTAR)
                p.setIsEvolved(true);
            p.updateRadiAndType();
        }

        int id = space.addPlanet(std::move(p));
        return "OK " + std::to_string(id);
    }
    else if (cmd == "REMOVE")
    {
        int id;
        if (!(iss >> id))
            return "ERR invalid args";

        Planet* p = space.findPlanetPtr(id);
        if (!p)
            return "ERR not found";

        space.removePlanet(id);
        return "OK";
    }
    else if (cmd == "PAUSE")
    {
        space.config.paused = true;
        return "OK";
    }
    else if (cmd == "UNPAUSE")
    {
        space.config.paused = false;
        return "OK";
    }
    else if (cmd == "TOGGLE_PAUSE")
    {
        space.config.paused = !space.config.paused;
        return space.config.paused ? "OK paused" : "OK running";
    }
    else if (cmd == "TIMESTEP")
    {
        float val;
        if (!(iss >> val))
            return "ERR invalid args";

        space.config.timestep_slider_value = val;
        return "OK";
    }
    else if (cmd == "RESET")
    {
        space.full_reset(view, window);
        return "OK";
    }
    else if (cmd == "KEY")
    {
        std::string keyName;
        if (!(iss >> keyName))
            return "ERR invalid args";

        sf::Keyboard::Key key = parseKeyName(keyName);
        if (key == sf::Keyboard::Unknown)
            return "ERR unknown key";

        sf::Event event;
        event.type = sf::Event::KeyReleased;
        event.key.code = key;
        event.key.alt = false;
        event.key.control = false;
        event.key.shift = false;
        event.key.system = false;
        space.hotkeys(event, view, window);
        return "OK";
    }
    else if (cmd == "GET_OBJECT")
    {
        int id;
        if (!(iss >> id))
            return "ERR invalid args";

        Planet* p = space.findPlanetPtr(id);
        if (!p)
            return "ERR not found";

        std::ostringstream out;
        out << p->getId() << " "
            << p->getx() << " " << p->gety() << " "
            << p->getxv() << " " << p->getyv() << " "
            << p->getMass() << " " << p->getRadius() << " "
            << bodyTypeToString(p->getType());
        return out.str();
    }
    else if (cmd == "LIST")
    {
        std::ostringstream out;
        const auto& planets = space.getPlanets();
        for (size_t i = 0; i < planets.size(); i++)
        {
            if (i > 0) out << " ";
            out << planets[i].getId();
        }
        return out.str();
    }
    else if (cmd == "COUNT")
    {
        return std::to_string(space.getPlanets().size());
    }
    else if (cmd == "STATE")
    {
        std::ostringstream out;
        out << (space.config.paused ? "paused" : "running") << " "
            << space.config.timestep_slider_value << " "
            << space.getPlanets().size() << " "
            << space.get_iteration();
        return out.str();
    }
    else if (cmd == "SET")
    {
        std::string key;
        if (!(iss >> key))
            return "ERR missing key";

        auto& c = space.config;
        if (key == "gravity") { int v; if (!(iss >> v)) return "ERR missing value"; c.gravity_enabled = (v != 0); return "OK"; }
        if (key == "heat") { int v; if (!(iss >> v)) return "ERR missing value"; c.heat_enabled = (v != 0); return "OK"; }
        if (key == "bloom") { int v; if (!(iss >> v)) return "ERR missing value"; c.bloom_enabled = (v != 0); return "OK"; }
        if (key == "gui") { int v; if (!(iss >> v)) return "ERR missing value"; c.show_gui = (v != 0); return "OK"; }
        if (key == "autobound") { int v; if (!(iss >> v)) return "ERR missing value"; c.autobound = (v != 0); return "OK"; }
        if (key == "fuel_burn") { double v; if (!(iss >> v)) return "ERR missing value"; c.fuel_burn_rate = v; return "OK"; }
        if (key == "render_life") { int v; if (!(iss >> v)) return "ERR missing value"; c.render_life_always = (v != 0); return "OK"; }
        if (key == "timestep") { float v; if (!(iss >> v)) return "ERR missing value"; c.timestep_slider_value = v; return "OK"; }
        if (key == "paused") { int v; if (!(iss >> v)) return "ERR missing value"; c.paused = (v != 0); return "OK"; }

        return "ERR unknown setting";
    }
    else if (cmd == "GET")
    {
        std::string key;
        if (!(iss >> key))
            return "ERR missing key";

        const auto& c = space.config;
        if (key == "gravity") return std::to_string(c.gravity_enabled ? 1 : 0);
        if (key == "heat") return std::to_string(c.heat_enabled ? 1 : 0);
        if (key == "bloom") return std::to_string(c.bloom_enabled ? 1 : 0);
        if (key == "gui") return std::to_string(c.show_gui ? 1 : 0);
        if (key == "autobound") return std::to_string(c.autobound ? 1 : 0);
        if (key == "fuel_burn") { std::ostringstream out; out << c.fuel_burn_rate; return out.str(); }
        if (key == "render_life") return std::to_string(c.render_life_always ? 1 : 0);
        if (key == "timestep") { std::ostringstream out; out << c.timestep_slider_value; return out.str(); }
        if (key == "paused") return std::to_string(c.paused ? 1 : 0);

        return "ERR unknown setting";
    }

    return "ERR unknown command";
}
