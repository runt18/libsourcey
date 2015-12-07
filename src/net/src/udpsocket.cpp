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


#include "scy/net/udpsocket.h"
#include "scy/net/types.h"
#include "scy/logger.h"


using namespace std;


namespace scy {
namespace net {

    
#if 0
UDPSocket::UDPSocket() : 
    net::Socket(new UDPSocket, false)
{
}


UDPSocket::UDPSocket(UDPSocket* base, bool shared) : 
    net::Socket(base, shared) 
{
}


UDPSocket::UDPSocket(const Socket& socket) : 
    net::Socket(socket)
{
    if (!dynamic_cast<UDPSocket*>(_base))
        throw std::runtime_error("Cannot assign incompatible socket");
}
    

UDPSocket& UDPSocket::base() const
{
    return static_cast<UDPSocket&>(*_base);
}
#endif


//
// UDP Base
//


UDPSocket::UDPSocket(uv::Loop* loop) :
    uv::Handle(loop), 
    _buffer(65536)
{
    TraceLS(this) << "Create" << endl;
    init();
}


UDPSocket::~UDPSocket()
{
    TraceLS(this) << "Destroy" << endl;
}


void UDPSocket::init() 
{
    if (ptr()) return;
    
    TraceLS(this) << "Init" << endl;
    uv_udp_t* udp = new uv_udp_t;
    udp->data = this; //instance();
    _closed = false;
    _ptr = reinterpret_cast<uv_handle_t*>(udp);
    int r = uv_udp_init(loop(), udp);
    if (r)
        setUVError("Cannot initialize UDP socket", r);
}


void UDPSocket::connect(const Address& peerAddress) 
{
    _peer = peerAddress;

    // Send the Connected signal to mimic TCP behaviour  
    // since socket implementations are interchangable.
    //emitConnect();
    onSocketConnect();
}


void UDPSocket::close()
{
    TraceLS(this) << "Closing" << endl;    
    recvStop();
    uv::Handle::close();
}


void UDPSocket::bind(const Address& address, unsigned flags) 
{    
    TraceLS(this) << "Binding on " << address << endl;

    int r;
    switch (address.af()) {
    case AF_INET:
        r = uv_udp_bind(ptr<uv_udp_t>(), address.addr(), flags);
        break;
    //case AF_INET6:
    //    r = uv_udp_bind6(ptr<uv_udp_t>(), address.addr(), flags);
    //    break;
    default:
        throw std::runtime_error("Unexpected address family");
    }

    // Throw and exception of error
    if (r)
        setAndThrowError("Cannot bind UDP socket", r); 
    
    // Open the receiver channel
    recvStart();
}


int UDPSocket::send(const char* data, std::size_t len, int flags) 
{    
    assert(_peer.valid());
    return send(data, len, _peer, flags);
}


namespace internal {
    struct SendRequest 
    {
        uv_udp_send_t req;
        uv_buf_t buf;
    };
}


int UDPSocket::send(const char* data, std::size_t len, const Address& peerAddress, int /* flags */) 
{    
    TraceLS(this) << "Send: " << len << ": " << peerAddress << endl;
    assert(Thread::currentID() == tid());
    //assert(len <= net::MAX_UDP_PACKET_SIZE);

    if (_peer.valid() && _peer != peerAddress) {
        ErrorLS(this) << "Peer not authorized: " << peerAddress << endl;
        return -1;
    }

    if (!peerAddress.valid()) {
        ErrorLS(this) << "Peer not valid: " << peerAddress << endl;
        return -1;
    }
    
    int r;    
    auto sr = new internal::SendRequest;
    sr->buf = uv_buf_init((char*)data, len); // TODO: memcpy data?
    r = uv_udp_send(&sr->req, ptr<uv_udp_t>(), &sr->buf, 1, peerAddress.addr(), UDPSocket::afterSend);

#if 0
    switch (peerAddress.af()) {
    case AF_INET:
        r = uv_udp_send(&sr->req, ptr<uv_udp_t>(), &sr->buf, 1, peerAddress.addr(), UDPSocket::afterSend);
        break;
    case AF_INET6:
        r = uv_udp_send6(&sr->req, ptr<uv_udp_t>(), &sr->buf, 1,
            *reinterpret_cast<const sockaddr_in6*>(peerAddress.addr()), UDPSocket::afterSend);
        break;
    default:
        throw std::runtime_error("Unexpected address family");
    }
#endif
    if (r) {
        ErrorLS(this) << "Send failed: " << uv_err_name(r) << endl;
        setUVError("Invalid UDP socket", r); 
    }
    
    // R is -1 on error, otherwise return len
    return r ? r : len;
}

    
bool UDPSocket::setBroadcast(bool flag)
{
    if (!ptr()) return false;
    return uv_udp_set_broadcast(ptr<uv_udp_t>(), flag ? 1 : 0) == 0;
}


bool UDPSocket::setMulticastLoop(bool flag)
{
    if (!ptr()) return false;
    return uv_udp_set_broadcast(ptr<uv_udp_t>(), flag ? 1 : 0) == 0;
}


bool UDPSocket::setMulticastTTL(int ttl)
{
    assert(ttl > 0 && ttl < 255);
    if (!ptr()) return false;
    return uv_udp_set_broadcast(ptr<uv_udp_t>(), ttl) == 0;
}


bool UDPSocket::recvStart() 
{
    // UV_EALREADY means that the socket is already bound but that's okay
    // TODO: No need for boolean value as this method can throw exceptions
    // since it is called internally by bind().
    int r = uv_udp_recv_start(ptr<uv_udp_t>(), UDPSocket::allocRecvBuffer, onRecv);
    if (r && r != UV_EALREADY) {
        setAndThrowError("Cannot start recv on invalid UDP socket", r);
        return false;
    }  
    return true;
}


bool UDPSocket::recvStop() 
{
    // This method must not throw since it is called internally via libuv callbacks.
    if (!ptr()) return false;
    return uv_udp_recv_stop(ptr<uv_udp_t>()) == 0;
}


void UDPSocket::onRecv(const MutableBuffer& buf, const net::Address& address)
{
    TraceLS(this) << "Recv: " << buf.size() << endl;    
    //emitRecv(buf, address);
    onSocketRecv(buf, address);
}


void UDPSocket::setError(const scy::Error& err)
{
    uv::Handle::setError(err);
}

        
const scy::Error& UDPSocket::error() const
{
    return uv::Handle::error();
}


net::Address UDPSocket::address() const
{    
    if (!active())
        return net::Address();
        //throw std::runtime_error("Invalid UDP socket: No address");
    
    struct sockaddr address;
    int addrlen = sizeof(address);
    int r = uv_udp_getsockname(ptr<uv_udp_t>(), &address, &addrlen);
    if (r)
        return net::Address();
        //throwLastError("Invalid UDP socket: No address");

    return Address(&address, addrlen);
}


net::Address UDPSocket::peerAddress() const
{
    if (!_peer.valid())
        return net::Address();
        //throw std::runtime_error("Invalid UDP socket: No peer address");
    return _peer;
}


net::TransportType UDPSocket::transport() const 
{ 
    return net::UDP; 
}
    

bool UDPSocket::closed() const
{
    return uv::Handle::closed();
}


//
// Callbacks

void UDPSocket::onRecv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned /* flags */) 
{    
    auto socket = static_cast<UDPSocket*>(handle->data);
    TraceL << "On recv: " << nread << endl;
            
    if (nread < 0) {
        //assert(0 && "unexpected error");    
        TraceL << "Recv error: " << uv_err_name(nread)<< endl;
        socket->setUVError("UDP error", nread);
        return;
    }
    
    if (nread == 0) {
        assert(addr == NULL);
        // Returning unused buffer, this is not an error
        // 11/12/13: This happens on linux but not windows
        //socket->setUVError("End of file", UV_EOF);
        return;
    }
    
    socket->onRecv(mutableBuffer(buf->base, nread), net::Address(addr, sizeof(*addr)));
}


void UDPSocket::afterSend(uv_udp_send_t* req, int status) 
{
    auto sr = reinterpret_cast<internal::SendRequest*>(req);
    auto socket = reinterpret_cast<UDPSocket*>(sr->req.handle->data);    
    if (status) {        
        ErrorL << "Send error: " << uv_err_name(status) << endl;
        socket->setUVError("UDP send error", status);
    }
    delete sr;
}


void UDPSocket::allocRecvBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    auto self = static_cast<UDPSocket*>(handle->data);    
    //TraceL << "Allocating Buffer: " << suggested_size << endl;    
    
    // Reserve the recommended buffer size
    // XXX: libuv wants us to allocate 65536 bytes for UDP .. hmmm
    //if (suggested_size > self->_buffer.available())
    //    self->_buffer.reserve(suggested_size); 
    //assert(self->_buffer.capacity() >= suggested_size);
    assert(self->_buffer.size() >= suggested_size);

    // Reset the buffer position on each read
    //self->_buffer.position(0);
    buf->base = self->_buffer.data();
    buf->len = self->_buffer.size();

    //return uv_buf_init(self->_buffer.data(), suggested_size);
}


void UDPSocket::onError(const scy::Error& error) 
{        
    ErrorLS(this) << "Error: " << error.message << endl;    
    //emitError(error);
    onSocketError(error);
    close(); // close on error
}


void UDPSocket::onClose() 
{        
    ErrorLS(this) << "On close" << endl;    
    //emitClose();
    onSocketClose();
}


uv::Loop* UDPSocket::loop() const
{
    return uv::Handle::loop();
}


} } // namespace scy::net