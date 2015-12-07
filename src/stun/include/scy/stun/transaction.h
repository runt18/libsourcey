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


#ifndef SCY_STUN_Transaction_H
#define SCY_STUN_Transaction_H


#include "scy/net/transaction.h"
#include "scy/net/socket.h"
#include "scy/stun/message.h"


namespace scy {
namespace stun {


class Transaction: public net::Transaction<Message>
{
public:
    Transaction(const net::Socket::Ptr& socket, 
                const net::Address& peerAddress, 
                long timeout = 10000, 
                int retries = 1,
                uv::Loop* loop = uv::defaultLoop());

    virtual bool checkResponse(const Message& message);
    virtual void onResponse();
    
protected:
    virtual ~Transaction();
};


} } // namespace scy:stun


#endif // SCY_STUN_Transaction_H