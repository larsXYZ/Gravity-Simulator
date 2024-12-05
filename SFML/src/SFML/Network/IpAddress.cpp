////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Http.hpp>
#include <SFML/Network/SocketImpl.hpp>
#include <cstring>
#include <utility>


namespace sf
{
////////////////////////////////////////////////////////////
const IpAddress IpAddress::None;
const IpAddress IpAddress::Any(0, 0, 0, 0);
const IpAddress IpAddress::LocalHost(127, 0, 0, 1);
const IpAddress IpAddress::Broadcast(255, 255, 255, 255);


////////////////////////////////////////////////////////////
IpAddress::IpAddress() :
m_address(0),
m_valid  (false)
{
}


////////////////////////////////////////////////////////////
IpAddress::IpAddress(const std::string& address) :
m_address(0),
m_valid  (false)
{
    resolve(address);
}


////////////////////////////////////////////////////////////
IpAddress::IpAddress(const char* address) :
m_address(0),
m_valid  (false)
{
    resolve(address);
}


////////////////////////////////////////////////////////////
IpAddress::IpAddress(Uint8 byte0, Uint8 byte1, Uint8 byte2, Uint8 byte3) :
m_address(htonl(static_cast<Uint32>((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3))),
m_valid  (true)
{
}


////////////////////////////////////////////////////////////
IpAddress::IpAddress(Uint32 address) :
m_address(htonl(address)),
m_valid  (true)
{
}


////////////////////////////////////////////////////////////
std::string IpAddress::toString() const
{
    in_addr address;
    address.s_addr = m_address;

    return inet_ntoa(address);
}


////////////////////////////////////////////////////////////
Uint32 IpAddress::toInteger() const
{
    return ntohl(m_address);
}


////////////////////////////////////////////////////////////
IpAddress IpAddress::getLocalAddress()
{
    // The method here is to connect a UDP socket to anyone (here to localhost),
    // and get the local socket address with the getsockname function.
    // UDP connection will not send anything to the network, so this function won't cause any overhead.

    IpAddress localAddress;

    // Create the socket
    SocketHandle sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == priv::SocketImpl::invalidSocket())
        return localAddress;

    // Connect the socket to localhost on any port
    sockaddr_in address = priv::SocketImpl::createAddress(ntohl(INADDR_LOOPBACK), 9);
    if (connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1)
    {
        priv::SocketImpl::close(sock);
        return localAddress;
    }

    // Get the local address of the socket connection
    priv::SocketImpl::AddrLength size = sizeof(address);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&address), &size) == -1)
    {
        priv::SocketImpl::close(sock);
        return localAddress;
    }

    // Close the socket
    priv::SocketImpl::close(sock);

    // Finally build the IP address
    localAddress = IpAddress(ntohl(address.sin_addr.s_addr));

    return localAddress;
}


////////////////////////////////////////////////////////////
IpAddress IpAddress::getPublicAddress(Time timeout)
{
    // The trick here is more complicated, because the only way
    // to get our public IP address is to get it from a distant computer.
    // Here we get the web page from http://www.sfml-dev.org/ip-provider.php
    // and parse the result to extract our IP address
    // (not very hard: the web page contains only our IP address).

    Http server("www.sfml-dev.org");
    Http::Request request("/ip-provider.php", Http::Request::Get);
    Http::Response page = server.sendRequest(request, timeout);
    if (page.getStatus() == Http::Response::Ok)
        return IpAddress(page.getBody());

    // Something failed: return an invalid address
    return IpAddress();
}


////////////////////////////////////////////////////////////
void IpAddress::resolve(const std::string& address)
{
    m_address = 0;
    m_valid = false;

    if (address == "255.255.255.255")
    {
        // The broadcast address needs to be handled explicitly,
        // because it is also the value returned by inet_addr on error
        m_address = INADDR_BROADCAST;
        m_valid = true;
    }
    else if (address == "0.0.0.0")
    {
        m_address = INADDR_ANY;
        m_valid = true;
    }
    else
    {
        // Try to convert the address as a byte representation ("xxx.xxx.xxx.xxx")
        Uint32 ip = inet_addr(address.c_str());
        if (ip != INADDR_NONE)
        {
            m_address = ip;
            m_valid = true;
        }
        else
        {
            // Not a valid address, try to convert it as a host name
            addrinfo hints;
            std::memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            addrinfo* result = NULL;
            if (getaddrinfo(address.c_str(), NULL, &hints, &result) == 0)
            {
                if (result)
                {
                    sockaddr_in sin;
                    std::memcpy(&sin, result->ai_addr, sizeof(*result->ai_addr));
                    ip = sin.sin_addr.s_addr;
                    freeaddrinfo(result);
                    m_address = ip;
                    m_valid = true;
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////
bool operator ==(const IpAddress& left, const IpAddress& right)
{
    return !(left < right) && !(right < left);
}


////////////////////////////////////////////////////////////
bool operator !=(const IpAddress& left, const IpAddress& right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
bool operator <(const IpAddress& left, const IpAddress& right)
{
    return std::make_pair(left.m_valid, left.m_address) < std::make_pair(right.m_valid, right.m_address);
}


////////////////////////////////////////////////////////////
bool operator >(const IpAddress& left, const IpAddress& right)
{
    return right < left;
}


////////////////////////////////////////////////////////////
bool operator <=(const IpAddress& left, const IpAddress& right)
{
    return !(right < left);
}


////////////////////////////////////////////////////////////
bool operator >=(const IpAddress& left, const IpAddress& right)
{
    return !(left < right);
}


////////////////////////////////////////////////////////////
std::istream& operator >>(std::istream& stream, IpAddress& address)
{
    std::string str;
    stream >> str;
    address = IpAddress(str);

    return stream;
}


////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& stream, const IpAddress& address)
{
    return stream << address.toString();
}

} // namespace sf
