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


#include "scy/turn/server/server.h"
#include "scy/logger.h"
#include "scy/buffer.h"
#include <algorithm>


using std::endl;
using std::min;
using namespace scy::net;


namespace scy {
namespace turn {


Server::Server(ServerObserver& observer, const ServerOptions& options) :
    _observer(observer),
    _options(options),
    _udpSocket(nullptr),
    _tcpSocket(nullptr)
{
    TraceL << "Create" << endl;
}


Server::~Server() 
{
    TraceL << "Destroy" << endl;
    //assert(_udpSocket.isNull() || _udpSocket./*base().*/refCount() == 1);
    //assert(_tcpSocket.isNull() || _tcpSocket./*base().*/refCount() == 1);
    stop();    
    TraceL << "Destroy: OK" << endl;
}


void Server::start()
{
    TraceL << "Starting" << endl;    

    if (_options.enableUDP) {
        //_udpSocket.assign(new UDPSocket, false);
        _udpSocket.Recv += sdelegate(this, &Server::onSocketRecv, 1);
        _udpSocket.bind(_options.listenAddr);        
        //_udpSocket./*base().*/setBroadcast(true);
        TraceL << "UDP listening on " << _options.listenAddr << endl;    
    }
    
    if (_options.enableTCP) {
        //_tcpSocket.assign(new TCPSocket, false);
        _tcpSocket.bind(_options.listenAddr);
        _tcpSocket.listen();
        _tcpSocket.AcceptConnection += sdelegate(this, &Server::onTCPAcceptConnection);
        TraceL << "TCP listening on " << _options.listenAddr << endl;    
    }

    _timer.Timeout += sdelegate(this, &Server::onTimer);
    _timer.start(_options.timerInterval, _options.timerInterval);
}


void Server::stop()
{
    TraceL << "Stopping" << endl;    

    _timer.stop();
    
    // Delete allocations
    ServerAllocationMap allocations = this->allocations();
    for (auto it = allocations.begin(); it != allocations.end(); ++it)
        delete it->second;

    // Should have been cleared via callback
    assert(_allocations.empty());
    
    // Free all TCP control sockets.
    // Sockets should have a base reference  
    // count of 1 to ensure they are destroyed.
    _tcpSockets.clear();

    // Close server sockets
    if (_udpSocket.active()) {
        //assert(_udpSocket./*base().*/refCount() == 1);
        _udpSocket.close();
    }
    if (_tcpSocket.active()) {        
        //assert(_tcpSocket./*base().*/refCount() == 1);
        _tcpSocket.close();
    }
}


void Server::onTimer(void*)
{
    ServerAllocationMap allocations = this->allocations();
    for (auto it = allocations.begin(); it != allocations.end(); ++it) {
        //TraceL << "Checking allocation: " << *it->second << endl;    // print the allocation debug info
        if (!it->second->onTimer()) {
            // Entry removed via ServerAllocation destructor
            delete it->second;
        }
    }
}


void Server::onTCPAcceptConnection(void*, const net::TCPSocket::Ptr& sock)
{
    TraceL << "TCP connection accepted: " << sock->peerAddress() << endl;    
    
    //assert(sock./*base().*/refCount() == 1);
    _tcpSockets.push_back(sock);
    net::TCPSocket::Ptr& socket = _tcpSockets.back();
    //assert(socket./*base().*/refCount() == 2);
    socket->Recv += sdelegate(this, &Server::onSocketRecv);
    socket->Close += sdelegate(this, &Server::onTCPSocketClosed);

    // No need to increase control socket buffer size
    // setServerSocketBufSize<net::TCPSocket>(socket, SERVER_SOCK_BUF_SIZE); // TODO: make option
}


net::TCPSocket::Ptr Server::getTCPSocket(const net::Address& peerAddr)
{
    for (auto& sock : _tcpSockets) {
        TraceL << "sock->peerAddress(): " << sock->peerAddress() << ": " << peerAddr << endl;    
        if (sock->peerAddress() == peerAddr) {
            return sock;
        }
    }
    assert(0 && "unknown socket");
    return net::TCPSocket::Ptr();
}


void Server::onSocketRecv(void* sender, const MutableBuffer& buffer, const net::Address& peerAddress)
{
    TraceL << "Data received: " << buffer.size() << endl;    
     //auto info = reinterpret_cast<net::PacketInfo*>(packet.info);
    //assert(info);
    //if (!info)
    //    return;    const net::TCPSocket::Ptr& socket
    stun::Message message;
    auto socket = reinterpret_cast<net::Socket*>(sender);        
    char* buf = bufferCast<char*>(buffer);
    std::size_t len = buffer.size();
    std::size_t nread = 0;
    while (len > 0 && (nread = message.read(constBuffer(buf, len))) > 0) {
        if (message.classType() == stun::Message::Request || 
            message.classType() == stun::Message::Indication) {                
            Request request(message, socket->transport(), socket->address(), peerAddress); //getTCPSocket(socket->address()), 
            //if (!request.socket) {
            //    assert(0 && "invalid socket");
            //    continue;
            //}

            // TODO: Only authenticate stun::Message::Request types
            handleRequest(request, _observer.authenticateRequest(this, request));
        }
        else {
            assert(0 && "unknown request type");
        }

        buf += nread;
        len -= nread;
    }
    if (len == buffer.size())
        WarnL << "Non STUN packet received" << std::endl;

#if 0
    stun::Message message;
    if (message.read(constBuffer(packet.data(), packet.size()))) {
        assert(message.state() == stun::Message::Request);    

        Request request(*info->socket, message, info->socket->address(), info->peerAddress);
        AuthenticationState state = _observer.authenticateRequest(this, request);
        handleRequest(request, state);
    }
    else
#endif
}


void Server::onTCPSocketClosed(void* sender)
{
    TraceL << "TCP socket closed" << endl;    
    releaseTCPSocket(reinterpret_cast<net::Socket*>(sender));
}


void Server::releaseTCPSocket(net::Socket* socket)
{    
    TraceLS(this) << "Removing TCP socket: " << socket << std::endl;
    for (auto it = _tcpSockets.begin(); it != _tcpSockets.end(); ++it) { //::Ptr
        if (it->get() == socket) {
            socket->Recv -= sdelegate(this, &Server::onSocketRecv);
            socket->Close -= sdelegate(this, &Server::onTCPSocketClosed);

            // All we need to do is erase the socket in order to 
            // deincrement the ref counter and destroy the socket.
            //socket->close();
            _tcpSockets.erase(it);
            return;
        }
    }
    assert(0 && "unknown socket");
}


void Server::handleRequest(Request& request, AuthenticationState state)
{    
    TraceL << "Received STUN request:\n" 
        << "\tFrom: " << request.remoteAddress << "\n"
        << "\tData: " << request.toString()
        << endl;

    switch (state) {
        case Authenticating: 
            // await async response
            break;

        case Authorized: 
            handleAuthorizedRequest(request);
            break;

        case QuotaReached:
            respondError(request, 486, "Allocation Quota Reached");
            break;

        case NotAuthorized: 
            respondError(request, 401, "NotAuthorized");
            break;
    }
}


void Server::handleAuthorizedRequest(Request& request) //, AuthenticationState state
{        
    TraceL << "Handle authorized request: " << request.toString() << endl;    

    // All requests after the initial Allocate must use the same username as
    // that used to create the allocation, to prevent attackers from
    // hijacking the client's allocation.  Specifically, if the server
    // requires the use of the long-term credential mechanism, and if a non-
    // Allocate request passes authentication under this mechanism, and if
    // the 5-tuple identifies an existing allocation, but the request does
    // not use the same username as used to create the allocation, then the
    // request MUST be rejected with a 441 (Wrong Credentials) error.
    // 
    // When a TURN message arrives at the server from the client, the server
    // uses the 5-tuple in the message to identify the associated
    // allocation.  For all TURN messages (including ChannelData) EXCEPT an
    // Allocate request, if the 5-tuple does not identify an existing
    // allocation, then the message MUST either be rejected with a 437
    // Allocation Mismatch error (if it is a request) or silently ignored
    // (if it is an indication or a ChannelData message).  A client
    // receiving a 437 error response to a request other than Allocate MUST
    // assume the allocation no longer exists.

    switch (request.methodType()) {
        case stun::Message::Binding: 
            handleBindingRequest(request);
            break;

        case stun::Message::Allocate: 
            handleAllocateRequest(request);
            break;

        case stun::Message::ConnectionBind: 
            handleConnectionBindRequest(request);
            break;

        default: {
            FiveTuple tuple(request.remoteAddress, request.localAddress, request.transport); //socket->
            auto allocation = getAllocation(tuple); //reinterpret_cast<ServerAllocation*>();
            if (!allocation)  {
                respondError(request, 437, "Allocation Mismatch");
                return;
            }            

            TraceL << "Obtained allocation: " << tuple << endl;                    
            if (!allocation->handleRequest(request))
                respondError(request, 600, "Operation Not Supported");
        }
    }
}


void Server::handleConnectionBindRequest(Request& request)
{
    auto connAttr = request.get<stun::ConnectionID>();
    if (!connAttr) {        
        TraceL << "ConnectionBind request has no ConnectionID" << endl;
        respondError(request, 400, "Bad Request");
        return;
    }

    auto alloc = getTCPAllocation(connAttr->value());
    if (!alloc) {
        TraceL << "ConnectionBind request has no allocation for: " << connAttr->value() << endl;
        respondError(request, 400, "Bad Request");
        return;
    }

    alloc->handleConnectionBindRequest(request);
}


void Server::handleBindingRequest(Request& request) 
{
    TraceL << "Handle Binding request" << endl;

    assert(request.methodType() == stun::Message::Binding);
    assert(request.classType() == stun::Message::Request);

    stun::Message response(stun::Message::SuccessResponse, stun::Message::Binding);
    //response.setClass(stun::Message::Request);
    //response.setMethod(stun::Message::Binding);
    response.setTransactionID(request.transactionID());

    // XOR-MAPPED-ADDRESS
    auto addrAttr = new stun::XorMappedAddress;
    addrAttr->setAddress(request.remoteAddress);
    //addrAttr->setFamily(1);
    //addrAttr->setPort(request.remoteAddress.port());
    //addrAttr->setIP(request.remoteAddress.host());
    response.add(addrAttr);
  
    //request.socket->sendPacket(response, request.remoteAddress);
    respond(request, response);
}


void Server::handleAllocateRequest(Request& request) 
{
    TraceL << "Handle Allocate request" << endl;

    assert(request.methodType() == stun::Message::Allocate);
    assert(request.classType() == stun::Message::Request);

    // When the server receives an Allocate request, it performs the
    // following checks:
    // 
    // 1.  The server MUST require that the request be authenticated.  This
    //     authentication MUST be done using the long-term credential
    //     mechanism of [RFC5389] unless the client and server agree to use
    //     another mechanism through some procedure outside the scope of
    //     this document.
    // 
    auto usernameAttr = request.get<stun::Username>();
    if (!usernameAttr) {
        TraceL << "NotAuthorized STUN Request" << endl;
        respondError(request, 401, "NotAuthorized");
        return;
    }

    std::string username(usernameAttr->asString());    

    // 2.  The server checks if the 5-tuple is currently in use by an
    //     existing allocation.  If yes, the server rejects the request with
    //     a 437 (Allocation Mismatch) error.

    // 3.  The server checks if the request contains a REQUESTED-TRANSPORT
    //     attribute.  If the REQUESTED-TRANSPORT attribute is not included
    //     or is malformed, the server rejects the request with a 400 (Bad
    //     Request) error.  Otherwise, if the attribute is included but
    //     specifies a protocol other that UDP, the server rejects the
    //     request with a 442 (Unsupported Transport Protocol) error.
    // 
    auto transportAttr = request.get<stun::RequestedTransport>();
    if (!transportAttr) {
        ErrorL << "No Requested Transport" << endl;
        respondError(request, 400, "Bad Request");
        return;
    }
        
    int protocol = transportAttr->value() >> 24;
    if (protocol != 6 &&
        protocol != 17) {
        ErrorL << "Requested Transport is neither TCP or UDP: " << protocol << endl;
        respondError(request, 422, "Unsupported Transport Protocol");
        return;
    }

    FiveTuple tuple(request.remoteAddress, request.localAddress, protocol == 17 ? net::UDP : net::TCP);
    if (getAllocation(tuple))  {
        ErrorL << "Allocation already exists for 5tuple: " << tuple << endl;
        respondError(request, 437, "Allocation Mismatch");
        return;
    } 

    // 4.  The request may contain a DONT-FRAGMENT attribute.  If it does,
    //     but the server does not support sending UDP datagrams with the DF
    //     bit set to 1 (see Section 12), then the server treats the DONT-
    //     FRAGMENT attribute in the Allocate request as an unknown
    //     comprehension-required attribute.

    // 5.  The server checks if the request contains a RESERVATION-TOKEN
    //     attribute.  If yes, and the request also contains an EVEN-PORT
    //     attribute, then the server rejects the request with a 400 (Bad
    //     Request) error.  Otherwise, it checks to see if the token is
    //     valid (i.e., the token is in range and has not expired and the
    //     corresponding relayed transport address is still available).  If
    //     the token is not valid for some reason, the server rejects the
    //     request with a 508 (Insufficient Capacity) error.

    // 6.  The server checks if the request contains an EVEN-PORT attribute.
    //     If yes, then the server checks that it can satisfy the request
    //     (i.e., can allocate a relayed transport address as described
    //     below).  If the server cannot satisfy the request, then the
    //     server rejects the request with a 508 (Insufficient Capacity)
    //     error.

    // 7.  At any point, the server MAY choose to reject the request with a
    //     486 (ServerAllocation Quota Reached) error if it feels the client is
    //     trying to exceed some locally defined allocation quota.  The
    //     server is free to define this allocation quota any way it wishes,
    //     but SHOULD define it based on the username used to authenticate
    //     the request, and not on the client's transport address.

    // 8.  Also at any point, the server MAY choose to reject the request
    //     with a 300 (Try Alternate) error if it wishes to redirect the
    //     client to a different server.  The use of this error code and
    //     attribute follow the specification in [RFC5389].

    // Compute the appropriate LIFETIME for this allocation.
    UInt32 lifetime = min(options().allocationMaxLifetime / 1000, options().allocationDefaultLifetime / 1000);
    auto lifetimeAttr = request.get<stun::Lifetime>();
    if (lifetimeAttr)
        lifetime = min(lifetime, lifetimeAttr->value());

    ServerAllocation* allocation = nullptr;

    // Protocol specific allocation handling. 6 = TCP, 17 = UDP.
    if (protocol == 17) {        // UDP

        // If all the checks pass, the server creates the allocation.  The
        // 5-tuple is set to the 5-tuple from the Allocate request, while the
        // list of permissions and the list of channels are initially empty.

        // The server chooses a relayed transport address for the allocation as
        // follows:

        // o  If the request contains a RESERVATION-TOKEN, the server uses the
        //    previously reserved transport address corresponding to the
        //    included token (if it is still available).  Note that the
        //    reservation is a server-wide reservation and is not specific to a
        //    particular allocation, since the Allocate request containing the
        //    RESERVATION-TOKEN uses a different 5-tuple than the Allocate
        //    request that made the reservation.  The 5-tuple for the Allocate
        //    request containing the RESERVATION-TOKEN attribute can be any
        //    allowed 5-tuple; it can use a different client IP address and
        //    port, a different transport protocol, and even different server IP
        //    address and port (provided, of course, that the server IP address
        //    and port are ones on which the server is listening for TURN
        //    requests).

        // o  If the request contains an EVEN-PORT attribute with the R bit set
        //    to 0, then the server allocates a relayed transport address with
        //    an even port number.

        // o  If the request contains an EVEN-PORT attribute with the R bit set
        //    to 1, then the server looks for a pair of port numbers N and N+1
        //    on the same IP address, where N is even.  Port N is used in the
        //    current allocation, while the relayed transport address with port
        //    N+1 is assigned a token and reserved for a future allocation.  The
        //    server MUST hold this reservation for at least 30 seconds, and MAY
        //    choose to hold longer (e.g., until the allocation with port N
        //    expires).  The server then includes the token in a RESERVATION-
        //    TOKEN attribute in the success response.

        // o  Otherwise, the server allocates any available relayed transport
        //    address.        

        // In all cases, the server SHOULD only allocate ports from the range
        // 49152 - 65535 (the Dynamic and/or Private Port range [Port-Numbers]),
        // unless the TURN server application knows, through some means not
        // specified here, that other applications running on the same host as
        // the TURN server application will not be impacted by allocating ports
        // outside this range.  This condition can often be satisfied by running
        // the TURN server application on a dedicated machine and/or by
        // arranging that any other applications on the machine allocate ports
        // before the TURN server application starts.  In any case, the TURN
        // server SHOULD NOT allocate ports in the range 0 - 1023 (the Well-
        // Known Port range) to discourage clients from using TURN to run
        // standard services.

        //    NOTE: The IETF is currently investigating the topic of randomized
        //    port assignments to avoid certain types of attacks (see
        //    [TSVWG-PORT]).  It is strongly recommended that a TURN implementor
        //    keep abreast of this topic and, if appropriate, implement a
        //    randomized port assignment algorithm.  This is especially
        //    applicable to servers that choose to pre-allocate a number of
        //    ports from the underlying OS and then later assign them to
        //    allocations; for example, a server may choose this technique to
        //    implement the EVEN-PORT attribute.

        // The server determines the initial value of the time-to-expiry field
        // as follows.  If the request contains a LIFETIME attribute, then the
        // server computes the minimum of the client's proposed lifetime and the
        // server's maximum allowed lifetime.  If this computed value is greater
        // than the default lifetime, then the server uses the computed lifetime
        // as the initial value of the time-to-expiry field.  Otherwise, the
        // server uses the default lifetime.  It is RECOMMENDED that the server
        // use a maximum allowed lifetime value of no more than 3600 seconds (1
        // hour).  Servers that implement allocation quotas or charge clients for
        // allocations in some way may wish to use a smaller maximum allowed
        // lifetime (perhaps as small as the default lifetime) to more quickly
        // remove orphaned allocations (that is, allocations where the
        // corresponding client has crashed or isTerminated or the client
        // IConnection has been lost for some reason).  Also, note that the time-
        // to-expiry is recomputed with each successful Refresh request, and
        // thus the value computed here applies only until the first refresh.

        // Find or create the allocation matching the 5-TUPLE. If the allocation
        // already exists then send an error.
        allocation = new UDPAllocation(*this, tuple, username, lifetime);
    } 
    
    else if (protocol == 6) {    // TCP

        // 5.1. Receiving a TCP Allocate Request
        // 
        // 
        // The process is similar to that defined in [RFC5766], Section 6.2,
        // with the following exceptions:
        // 
        // 1.  If the REQUESTED-TRANSPORT attribute is included and specifies a
        //     protocol other than UDP or TCP, the server MUST reject the
        //     request with a 442 (Unsupported Transport Protocol) error.  If
        //     the value is UDP, and if UDP transport is allowed by local
        //     policy, the server MUST continue with the procedures of [RFC5766]
        //     instead of this document.  If the value is UDP, and if UDP
        //     transport is forbidden by local policy, the server MUST reject
        //     the request with a 403 (Forbidden) error.
        // 
        // 2.  If the client connection transport is not TCP or TLS, the server
        //     MUST reject the request with a 400 (Bad Request) error.
        // 
        // 3.  If the request contains the DONT-FRAGMENT, EVEN-PORT, or
        //     RESERVATION-TOKEN attribute, the server MUST reject the request
        //     with a 400 (Bad Request) error.
        // 
        // 4.  A TCP relayed transport address MUST be allocated instead of a
        //     UDP one.
        // 
        // 5.  The RESERVATION-TOKEN attribute MUST NOT be present in the
        //     success response.
        // 
        // If all checks pass, the server MUST start accepting incoming TCP
        // connections on the relayed transport address.  Refer to Section 5.3
        // for details.

        //net::TCPSocket& socket = static_cast<net::TCPSocket&>(request.socket);  
        //static_cast<net::TCPSocket&>(request.socket)
        //assert(request.socket->/*base().*/refCount() == 1);
        allocation = new TCPAllocation(*this, getTCPSocket(request.remoteAddress), tuple, username, lifetime); //request.socket
        //assert(request.socket->/*base().*/refCount() == 2);
    } 

    // Once the allocation is created, the server replies with a success
    // response.  The success response contains:

    stun::Message response(stun::Message::SuccessResponse, stun::Message::Allocate);
    response.setTransactionID(request.transactionID());

    // o  An XOR-RELAYED-ADDRESS attribute containing the relayed transport
    //    address.
    assert(!options().externalIP.empty());
    
    // Try to use the externalIP value for the XorRelayedAddress 
    // attribute to overcome proxy and NAT issues.
    std::string relayHost(options().externalIP);
    if (relayHost.empty()) {
        relayHost.assign(allocation->relayedAddress().host());
        assert(0 && "external IP not set");
    }

    auto relayAddrAttr = new stun::XorRelayedAddress;
    relayAddrAttr->setAddress(net::Address(relayHost, allocation->relayedAddress().port()));
    response.add(relayAddrAttr);

    // o  A LIFETIME attribute containing the current value of the time-to-
    //    expiry timer.
    auto resLifetimeAttr = new stun::Lifetime;
    resLifetimeAttr->setValue(lifetime); // / 1000
    response.add(resLifetimeAttr);

    // o  A RESERVATION-TOKEN attribute (if a second relayed transport
    //    address was reserved).

    // o  An XOR-MAPPED-ADDRESS attribute containing the client's IP address
    //    and port (from the 5-tuple).

    //    NOTE: The XOR-MAPPED-ADDRESS attribute is included in the response
    //    as a convenience to the client.  TURN itself does not make use of
    //    this value, but clients running ICE can often need this value and
    //    can thus avoid having to do an extra Binding transaction with some
    //    STUN server to learn it. 
    auto mappedAddressAttr = new stun::XorMappedAddress;
    mappedAddressAttr->setAddress(request.remoteAddress);
    //mappedAddressAttr->setFamily(1);
    //mappedAddressAttr->setIP(request.remoteAddress.host());
    //mappedAddressAttr->setPort(request.remoteAddress.port());
    response.add(mappedAddressAttr);
        
    TraceL << "Allocate response: " 
        << "XorRelayedAddress=" << relayAddrAttr->address() 
        << ", XorMappedAddress=" << mappedAddressAttr->address() 
        << ", MessageIntegrity=" << request.hash << endl;
    
    // Sign the response message
    //auto integrityAttr = new stun::MessageIntegrity;
    //integrityAttr->setKey(request.hash);
    //response.add(integrityAttr);
    
    // The response (either success or error) is sent back to the client on
    // the 5-tuple.
    //request.socket->send(response, request.remoteAddress);
    respond(request, response);

    TraceL << "Handle Allocate request: OK" << endl;

    //    NOTE: When the Allocate request is sent over UDP, section 7.3.1 of
    //    [RFC5389] requires that the server handle the possible
    //    retransmissions of the request so that retransmissions do not
    //    cause multiple allocations to be created.  Implementations may
    //    achieve this using the so-called "stateless stack approach" as
    //    follows.  To detect retransmissions when the original request was
    //    successful in creating an allocation, the server can store the
    //    transaction id that created the request with the allocation data
    //    and compare it with incoming Allocate requests on the same
    //    5-tuple.  Once such a request is detected, the server can stop
    //    parsing the request and immediately generate a success response.
    //    When building this response, the value of the LIFETIME attribute
    //    can be taken from the time-to-expiry field in the allocate state
    //    data, even though this value may differ slightly from the LIFETIME
    //    value originally returned.  In addition, the server may need to
    //    store an indication of any reservation token returned in the
    //    original response, so that this may be returned in any
    //    retransmitted responses.

    //    For the case where the original request was unsuccessful in
    //    creating an allocation, the server may choose to do nothing
    //    special.  Note, however, that there is a rare case where the
    //    server rejects the original request but accepts the retransmitted
    //    request (because conditions have changed in the brief intervening
    //    time period).  If the client receives the first failure response,
    //    it will ignore the second (success) response and believe that an
    //    allocation was not created.  An allocation created in this matter
    //    will eventually timeout, since the client will not refresh it.
    //    Furthermore, if the client later retries with the same 5-tuple but
    //    different transaction id, it will receive a 437 (ServerAllocation
    //    Mismatch), which will cause it to retry with a different 5-tuple.
    //    The server may use a smaller maximum lifetime value to minimize
    //    the lifetime of allocations "orphaned" in this manner.
}


void Server::respond(Request& request, stun::Message& response)
{    
    // Sign the response message
    if (!request.hash.empty()) {
        auto integrityAttr = new stun::MessageIntegrity;
        integrityAttr->setKey(request.hash);
        response.add(integrityAttr);
    }
    
    InfoL << "Sending message: " << response << ": " << request.remoteAddress << endl;
    
    // The response (either success or error) is sent back to the
    // client on the 5-tuple.
    switch (request.transport) {
        case net::UDP:
            _udpSocket.sendPacket(response, request.remoteAddress);
            break;
        case net::TCP:
        case net::SSLTCP:
            auto socket = getTCPSocket(request.remoteAddress);
            if (!socket) {
                return;
            }
            socket->sendPacket(response);
            break;
    }
}
                   
void Server::respondError(Request& request, int errorCode, const char* errorDesc) 
{
    TraceL << "Send STUN error: " << errorCode << ": " << errorDesc << endl;
    
    //Mutex::ScopedLock lock(_mutex);

    stun::Message errorMsg(stun::Message::ErrorResponse, request.methodType());
    errorMsg.setTransactionID(request.transactionID());

    // SOFTWARE
    auto softwareAttr = new stun::Software;
    softwareAttr->copyBytes(_options.software.c_str(), _options.software.size());
    errorMsg.add(softwareAttr);

    // REALM
    auto realmAttr = new stun::Realm;
    realmAttr->copyBytes(_options.realm.c_str(), _options.realm.size());
    errorMsg.add(realmAttr);

    // NONCE
    auto nonceAttr = new stun::Nonce;
    std::string noonce = util::randomString(32);
    nonceAttr->copyBytes(noonce.c_str(), noonce.size());
    errorMsg.add(nonceAttr);

    // ERROR-CODE
    auto errorCodeAttr = new stun::ErrorCode();
    errorCodeAttr->setErrorCode(errorCode);
    errorCodeAttr->setReason(errorDesc);
    errorMsg.add(errorCodeAttr);
    assert(errorCode == errorCodeAttr->errorCode());
    
    //request.socket->sendPacket(errorMsg, request.remoteAddress);
    respond(request, errorMsg);
}

    
net::UDPSocket& Server::udpSocket()
{ 
    //Mutex::ScopedLock lock(_mutex);
    return _udpSocket; 
}


ServerObserver& Server::observer() 
{ 
    //Mutex::ScopedLock lock(_mutex);
    return _observer; 
}


ServerOptions& Server::options() 
{ 
    //Mutex::ScopedLock lock(_mutex);
    return _options; 
}


ServerAllocationMap Server::allocations() const
{
    //Mutex::ScopedLock lock(_mutex);
    return _allocations;
}


Timer& Server::timer()
{
    return _timer;
}


void Server::addAllocation(ServerAllocation* alloc) 
{
    {
        //Mutex::ScopedLock lock(_mutex);
        
        assert(_allocations.find(alloc->tuple()) == _allocations.end());
        _allocations[alloc->tuple()] = alloc;

        InfoL << "Allocation added: " 
            << alloc->tuple().toString() << ": " 
            << _allocations.size() << " total" << endl;
    }

    _observer.onServerAllocationCreated(this, alloc);
}


void Server::removeAllocation(ServerAllocation* alloc) 
{
    {
        //Mutex::ScopedLock lock(_mutex);    

        auto it = _allocations.find(alloc->tuple());
        if (it != _allocations.end()) {
            _allocations.erase(it);

            InfoL << "Allocation removed: " 
                << alloc->tuple().toString() << ": " 
                << _allocations.size() << " remaining" << endl;
        }
        else assert(0);
    }

    _observer.onServerAllocationRemoved(this, alloc);
}


ServerAllocation* Server::getAllocation(const FiveTuple& tuple) 
{
    //Mutex::ScopedLock lock(_mutex);

    auto it = _allocations.find(tuple);
    if (it != _allocations.end())
        return it->second;
    return nullptr;
}


TCPAllocation* Server::getTCPAllocation(const UInt32& connectionID) 
{
    //Mutex::ScopedLock lock(_mutex);    

    for (auto it = _allocations.begin(); it != _allocations.end(); ++it) {
        auto alloc = dynamic_cast<TCPAllocation*>(it->second);
        if (alloc && alloc->pairs().exists(connectionID))
            return alloc;
    }

    // TODO: Handle via allocation so we can remove lookup overhead.
    // The TCP allocation may have been deleted before the 
    // ConnectionBind request comes in.
    //assert(0 && "allocation mismatch");
    return nullptr;
}


} } //  namespace scy::turn