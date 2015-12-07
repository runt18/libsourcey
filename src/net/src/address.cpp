//
// LibSourcey
// Copyright (C) 2005, Sourcey <http://sourcey.com>
//
// LibSourcey is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// LibSourcey is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//


#include "scy/net/address.h"
#include "scy/logger.h"
#include "scy/memory.h"
#include "scy/types.h"

#include "uv.h"


using std::endl;


namespace scy {
namespace net {


//
// AddressBase
//


class AddressBase: public SharedObject
{
public:
    virtual std::string host() const = 0;    
    virtual UInt16 port() const = 0;
    virtual Address::Family family() const = 0;
    virtual socklen_t length() const = 0;
    virtual const struct sockaddr* addr() const = 0;
    virtual int af() const = 0;

protected:
    AddressBase()
    {
    }

    virtual ~AddressBase()
    {
    }

private:
    AddressBase(const AddressBase&);
    AddressBase& operator = (const AddressBase&);
};


//
// IPv4AddressBase
//


class IPv4AddressBase: public AddressBase
{
public:
    IPv4AddressBase()
    {
        _addr.sin_family = AF_INET;
        memset(&_addr, 0, sizeof(_addr));
    }

    IPv4AddressBase(const struct sockaddr_in* addr)
    {
        _addr.sin_family = AF_INET;
        memcpy(&_addr, addr, sizeof(_addr));
    }

    IPv4AddressBase(const void* addr, UInt16 port)
    {
        memset(&_addr, 0, sizeof(_addr));
        _addr.sin_family = AF_INET;
        memcpy(&_addr.sin_addr, addr, sizeof(_addr.sin_addr));
        _addr.sin_port = port;
    }

    std::string host() const
    {
        char dest[16];
        if (uv_ip4_name(const_cast<sockaddr_in*>(&_addr), dest, 16) != 0)
            throw std::runtime_error("Cannot parse IPv4 hostname");
        return dest;
    }

    UInt16 port() const
    {
        return _addr.sin_port;
    }

    Address::Family family() const
    {
        return Address::IPv4;
    }

    socklen_t length() const
    {
        return sizeof(_addr);
    }

    const struct sockaddr* addr() const
    {
        return reinterpret_cast<const struct sockaddr*>(&_addr);
    }

    int af() const
    {
        return _addr.sin_family;
    }

private:
    struct sockaddr_in _addr;
};


//
// IPv6AddressBase
//

#if defined(LibSourcey_HAVE_IPv6)


class IPv6AddressBase: public AddressBase
{
public:
    IPv6AddressBase(const struct sockaddr_in6* addr)
    {
        _addr.sin6_family = AF_INET6;
        memcpy(&_addr, addr, sizeof(_addr));
    }

    IPv6AddressBase(const void* addr, UInt16 port)
    {
        _addr.sin6_family = AF_INET6;
        memset(&_addr, 0, sizeof(_addr));
        memcpy(&_addr.sin6_addr, addr, sizeof(_addr.sin6_addr));
        _addr.sin6_port = port;
    }

    IPv6AddressBase(const void* addr, UInt16 port, UInt32 scope)
    {
        _addr.sin6_family = AF_INET6;
        memset(&_addr, 0, sizeof(_addr));
        memcpy(&_addr.sin6_addr, addr, sizeof(_addr.sin6_addr));
        _addr.sin6_port = port;
        _addr.sin6_scope_id = scope;
    }
    
    std::string host() const
    {
        char dest[46];
        if (uv_ip6_name(const_cast<sockaddr_in6*>(&_addr), dest, 46) != 0)
            throw std::runtime_error("Cannot parse IPv6 hostname");
        return dest;
    }

    UInt16 port() const
    {
        return _addr.sin6_port;
    }
    
    Address::Family family() const
    {
        return Address::IPv6;
    }

    socklen_t length() const
    {
        return sizeof(_addr);
    }

    const struct sockaddr* addr() const
    {
        return reinterpret_cast<const struct sockaddr*>(&_addr);
    }

    int af() const
    {
        return _addr.sin6_family;
    }

private:
    struct sockaddr_in6 _addr;
};


#endif // LibSourcey_HAVE_IPv6


//
// Address
//


Address::Address()
{
    _base = new IPv4AddressBase;
}


Address::Address(const std::string& addr, UInt16 port)
{
    init(addr, port);
}


Address::Address(const std::string& addr, const std::string& port)
{
    init(addr, resolveService(port));
}


Address::Address(const std::string& hostAndPort)
{
    assert(!hostAndPort.empty());

    std::string host;
    std::string port;
    std::string::const_iterator it  = hostAndPort.begin();
    std::string::const_iterator end = hostAndPort.end();
    if (*it == '[') {
        ++it;
        while (it != end && *it != ']') host += *it++;
        if (it == end) throw std::runtime_error("Invalid address: Malformed IPv6 address");
        ++it;
    }
    else {
        while (it != end && *it != ':') host += *it++;
    }
    if (it != end && *it == ':') {
        ++it;
        while (it != end) port += *it++;
    }
    else throw std::runtime_error("Invalid address: Missing port number");
    init(host, resolveService(port));
}


Address::Address(const struct sockaddr* addr, socklen_t length)
{
    if (length == sizeof(struct sockaddr_in))
        _base = new IPv4AddressBase(reinterpret_cast<const struct sockaddr_in*>(addr));
#if defined(LibSourcey_HAVE_IPv6)
    else if (length == sizeof(struct sockaddr_in6))
        _base = new IPv6AddressBase(reinterpret_cast<const struct sockaddr_in6*>(addr));
#endif
    else throw std::runtime_error("Invalid address length passed to Address()");
}


Address::Address(const Address& addr)
{
    _base = addr._base;
    _base->duplicate();
}


Address::~Address()
{
    _base->release();
}


Address& Address::operator = (const Address& addr)
{
    if (&addr != this) {
        _base->release();
        _base = addr._base;
        _base->duplicate();
    }
    return *this;
}


void Address::init(const std::string& host, UInt16 port)
{
    //TraceLS(this) << "Parse: " << host << ":" << port << endl;

    char ia[sizeof(struct in6_addr)];
    if (uv_inet_pton(AF_INET, host.c_str(), &ia) == 0)
        _base = new IPv4AddressBase(&ia, htons(port));
    else if (uv_inet_pton(AF_INET6, host.c_str(), &ia) == 0)
        _base = new IPv6AddressBase(&ia, htons(port));
    else
        throw std::runtime_error("Invalid IP address format: " + host);
}


std::string Address::host() const
{
    return _base->host();
}


UInt16 Address::port() const
{
    return ntohs(_base->port());
}


Address::Family Address::family() const
{
    return _base->family();
}


socklen_t Address::length() const
{
    return _base->length();
}


const struct sockaddr* Address::addr() const
{
    return _base->addr();
}


int Address::af() const
{
    return _base->af();
}


bool Address::valid() const 
{
    return host() != "0.0.0.0" && port() != 0;
}


std::string Address::toString() const
{
    std::string result;
    if (family() == Address::IPv6)
        result.append("[");
    result.append(host());
    if (family() == Address::IPv6)
        result.append("]");
    result.append(":");
    result.append(util::itostr<UInt16>(port()));
    return result;
}


bool Address::operator < (const Address& addr) const
{
    if (family() < addr.family()) return true;
    return (port() < addr.port());
}


bool Address::operator == (const Address& addr) const
{
    return host() == addr.host() && port() == addr.port();
}


bool Address::operator != (const Address& addr) const
{
    return host() != addr.host() || port() != addr.port();
}


/*
void Address::swap(Address& a1, Address& a2)
{
    a1.swap(a2);
}
*/


void Address::swap(Address& addr)
{
    std::swap(_base, addr._base);
}


//
// Static helpers
//


bool Address::validateIP(const std::string& addr)
{
    char ia[sizeof(struct in6_addr)];
    if (uv_inet_pton(AF_INET, addr.c_str(), &ia) == 0)
        return true;
    else if (uv_inet_pton(AF_INET6, addr.c_str(), &ia) == 0)
        return true;
    return false;
}


UInt16 Address::resolveService(const std::string& service)
{
    UInt16 port = util::strtoi<UInt16>(service);
    if (port && port > 0) //, port) && port <= 0xFFFF
        return (UInt16) port;

    struct servent* se = getservbyname(service.c_str(), nullptr);
    if (se)
        return ntohs(se->s_port);
    else
        throw std::runtime_error("Service not found: " + service);
}


} } // namespace scy::net



// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.