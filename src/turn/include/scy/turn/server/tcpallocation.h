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


#ifndef SCY_TURN_SERVER_TCPAllocation_H
#define SCY_TURN_SERVER_TCPAllocation_H


#include "scy/turn/server/serverallocation.h"
#include "scy/collection.h"
#include "scy/net/tcpsocket.h"
#include "scy/turn/server/tcpconnectionpair.h"


namespace scy {
namespace turn {

    
class Server;

typedef PointerCollection<UInt32, TCPConnectionPair> TCPConnectionPairMap;


class TCPAllocation: public ServerAllocation
{
public:
    TCPAllocation(Server& server, 
                  const net::Socket::Ptr& control, 
                  const FiveTuple& tuple, 
                  const std::string& username, 
                  const UInt32& lifetime);
    virtual ~TCPAllocation();
    
    bool handleRequest(Request& request);    
    void handleConnectRequest(Request& request);
    void handleConnectionBindRequest(Request& request);

    void sendPeerConnectResponse(TCPConnectionPair* pair, bool success);
        // Send a Connect request response to control.

    int sendToControl(stun::Message& message);

    net::TCPSocket& control();
    net::Address relayedAddress() const;
    TCPConnectionPairMap& pairs();
    
    bool onTimer();

    void onPeerAccept(void* sender, const net::TCPSocket::Ptr& sock);
        // Accepts incoming peer sockets for ConnectionBind requests.

    void onControlClosed(void* sender);
        // Callback for handling controll connection destruction.
        // The allocation will be deleted.

protected:
    net::TCPSocket::Ptr _control;
    net::TCPSocket::Ptr _acceptor;
    TCPConnectionPairMap _pairs;
};


} } //  namespace scy::turn


#endif // SCY_TURN_SERVER_TCPAllocation_H