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


#include "scy/turn/server/udpallocation.h"
#include "scy/turn/server/server.h"
#include "scy/net/udpsocket.h"
#include "scy/buffer.h"
#include "scy/logger.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>


using namespace std;


namespace scy {
namespace turn {


UDPAllocation::UDPAllocation(Server& server,
                             const FiveTuple& tuple, 
                             const std::string& username, 
                             const UInt32& lifetime) : 
    ServerAllocation(server, tuple, username, lifetime)//,
    //_relaySocket(new net::UDPSocket) //server.reactor(), server.runner()
{
    // Handle data from the relay socket directly from the allocation.
    // This will remove the need for allocation lookups when receiving
    // data from peers.
    _relaySocket.bind(net::Address(server.options().listenAddr.host(), 0));        
    _relaySocket.Recv += sdelegate(this, &UDPAllocation::onPeerDataReceived);

    TraceL << " Initializing on address: " << _relaySocket.address() << endl;
}


UDPAllocation::~UDPAllocation() 
{
    TraceL << "Destroy" << endl;    
    _relaySocket.Recv -= sdelegate(this, &UDPAllocation::onPeerDataReceived);
    _relaySocket.close();
}


bool UDPAllocation::handleRequest(Request& request) 
{    
    TraceL << "Handle Request" << endl;    

    if (!ServerAllocation::handleRequest(request)) {
        if (request.methodType() == stun::Message::SendIndication)
            handleSendIndication(request);
        else
            return false;
    }
    
    return true; 
}


void UDPAllocation::handleSendIndication(Request& request) 
{    
    TraceL << "Handle Send Indication" << endl;

    // The message is first checked for validity.  The Send indication MUST
    // contain both an XOR-PEER-ADDRESS attribute and a DATA attribute.  If
    // one of these attributes is missing or invalid, then the message is
    // discarded.  Note that the DATA attribute is allowed to contain zero
    // bytes of data.

    auto peerAttr = request.get<stun::XorPeerAddress>();
    if (!peerAttr || (peerAttr && peerAttr->family() != 1)) {
        ErrorL << "Send Indication error: No Peer Address" << endl;
        // silently discard...
        return;
    }

    auto dataAttr = request.get<stun::Data>();
    if (!dataAttr) {
        ErrorL << "Send Indication error: No Data attribute" << endl;
        // silently discard...
        return;
    }

    // The Send indication may also contain the DONT-FRAGMENT attribute.  If
    // the server is unable to set the DF bit on outgoing UDP datagrams when
    // this attribute is present, then the server acts as if the DONT-
    // FRAGMENT attribute is an unknown comprehension-required attribute
    // (and thus the Send indication is discarded).

    // The server also checks that there is a permission installed for the
    // IP address contained in the XOR-PEER-ADDRESS attribute.  If no such
    // permission exists, the message is discarded.  Note that a Send
    // indication never causes the server to refresh the permission.

    // The server MAY impose restrictions on the IP address and port values
    // allowed in the XOR-PEER-ADDRESS attribute -- if a value is not
    // allowed, the server silently discards the Send indication.
    
    net::Address peerAddress = peerAttr->address();
    if (!hasPermission(peerAddress.host())) {
        ErrorL << "Send Indication error: No permission for: " << peerAddress.host() << endl;
        // silently discard...
        return;
    }

    // If everything is OK, then the server forms a UDP datagram as follows:

    // o  the source transport address is the relayed transport address of
    //    the allocation, where the allocation is determined by the 5-tuple
    //    on which the Send indication arrived;

    // o  the destination transport address is taken from the XOR-PEER-
    //    ADDRESS attribute;

    // o  the data following the UDP header is the contents of the value
    //    field of the DATA attribute.

    // The handling of the DONT-FRAGMENT attribute (if present), is
    // described in Section 12.

    // The resulting UDP datagram is then sent to the peer.
    
    TraceL << "Relaying Send Indication: " 
        << "\r\tFrom: " << request.remoteAddress.toString()
        << "\r\tTo: " << peerAddress
        << endl;    

    if (send(dataAttr->bytes(), dataAttr->size(), peerAddress) == -1) {
        _server.respondError(request, 486, "Allocation Quota Reached");
        delete this;
    }
}


void UDPAllocation::onPeerDataReceived(void*, const MutableBuffer& buffer, const net::Address& peerAddress)
{    
    //auto source = reinterpret_cast<net::PacketInfo*>(packet.info);
    TraceL << "Received UDP Datagram from " << peerAddress << endl;    
    
    if (!hasPermission(peerAddress.host())) {
        TraceL << "No Permission: " << peerAddress.host() << endl;    
        return;
    }

    updateUsage(buffer.size());
    
    // Check that we have not exceeded out lifetime and bandwidth quota.
    if (IAllocation::deleted())
        return;
    
    stun::Message message(stun::Message::Indication, stun::Message::DataIndication);
        
    // Try to use the externalIP value for the XorPeerAddress 
    // attribute to overcome proxy and NAT issues.
    std::string peerHost(server().options().externalIP);
    if (peerHost.empty()) {
        peerHost.assign(peerAddress.host());
        assert(0 && "external IP not set");
    }
    
    auto peerAttr = new stun::XorPeerAddress;
    peerAttr->setAddress(net::Address(peerHost, peerAddress.port()));
    message.add(peerAttr);

    auto dataAttr = new stun::Data;
    dataAttr->copyBytes(bufferCast<const char*>(buffer), buffer.size());
    message.add(dataAttr);
    
    //Mutex::ScopedLock lock(_mutex);

    TraceL << "Send data indication:" 
        << "\n\tFrom: " << peerAddress
        << "\n\tTo: " << _tuple.remote()
        //<< "\n\tData: " << std::string(packet.data(), packet.size())
        << endl;
        
    server().udpSocket().sendPacket(message, _tuple.remote());
    
    //net::Address tempAddress("58.7.41.244", _tuple.remote().port());
    //server().udpSocket().send(message, tempAddress);
}


int UDPAllocation::send(const char* data, std::size_t size, const net::Address& peerAddress)
{
    updateUsage(size);
    
    // Check that we have not exceeded our lifetime and bandwidth quota.
    if (IAllocation::deleted()) {
        WarnL << "Send indication dropped: Allocation quota reached" << endl;
        return -1;
    }

    return _relaySocket./*base().*/send(data, size, peerAddress);
}


net::Address UDPAllocation::relayedAddress() const 
{ 
    //Mutex::ScopedLock lock(_mutex);
    return _relaySocket.address();
}


} } //  namespace scy::turn