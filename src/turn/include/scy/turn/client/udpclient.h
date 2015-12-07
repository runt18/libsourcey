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


#ifndef SCY_TURN_UDPClient_H
#define SCY_TURN_UDPClient_H


#include "scy/turn/client/client.h"
#include "scy/turn/util.h"
#include "scy/turn/iallocation.h"
#include "scy/turn/types.h"
#include "scy/stun/transaction.h"
#include "scy/stateful.h"
#include "scy/net/udpsocket.h"

#include <deque>


namespace scy {
namespace turn {


class UDPClient: public Client
{    
public:
    UDPClient(ClientObserver& observer, const Options& options = Options());
    virtual ~UDPClient();
};


} } //  namespace scy::turn


#endif // SCY_TURN_Client_H