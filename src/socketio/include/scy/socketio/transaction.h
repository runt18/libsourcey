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


#ifndef SCY_SocketIO_Transaction_H
#define SCY_SocketIO_Transaction_H


#include "scy/packettransaction.h"
#include "scy/socketio/packet.h"


namespace scy {
namespace sockio {


class Client;


struct Transaction: public PacketTransaction<sockio::Packet>
{
    Transaction(sockio::Client& client, long timeout = 10000);
    Transaction(sockio::Client& client, const sockio::Packet& request, long timeout = 10000);
    
    virtual bool send();
    virtual bool checkResponse(const Packet& packet);

    virtual void onPotentialResponse(void*, Packet& packet);
    virtual void onResponse();

    sockio::Client& client;

protected:
    virtual ~Transaction();
};


} } // namespace scy::sockio


#endif // SCY_SocketIO_Transaction_H