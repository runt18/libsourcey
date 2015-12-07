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


#ifndef SCY_TURN_ServerAllocation_H
#define SCY_TURN_ServerAllocation_H


#include "scy/turn/iallocation.h"
#include "scy/turn/fivetuple.h"


namespace scy {
namespace turn {


class Server;


class ServerAllocation: public IAllocation
{
public:
    ServerAllocation(Server& server, 
                     const FiveTuple& tuple, 
                     const std::string& username, 
                     Int64 lifetime);
    
    virtual bool handleRequest(Request& request);    
    virtual void handleRefreshRequest(Request& request);    
    virtual void handleCreatePermission(Request& request);
        
    //virtual bool IAllocation::deleted() const;

    virtual bool onTimer();
        // Asynchronous timer callback for updating the allocation
        // permissions and state etc.
        // If this call returns false the allocation will be deleted.
    
    virtual Int64 timeRemaining() const; 
    virtual Int64 maxTimeRemaining() const;
    virtual Server& server(); 
    
    virtual void print(std::ostream& os) const;

protected:
    virtual ~ServerAllocation();
        // IMPORTANT: The destructor should never be called directly 
        // as the allocation is deleted via the timer callback.
        // See onTimer()

    friend class Server;
    
    UInt32 _maxLifetime;
    Server&    _server;

private:    
    ServerAllocation(const ServerAllocation&); // = delete;
    ServerAllocation(ServerAllocation&&); // = delete;
    ServerAllocation& operator=(const ServerAllocation&); // = delete;
    ServerAllocation& operator=(ServerAllocation&&); // = delete;

};


} } // namespace scy::turn


#endif // SCY_TURN_ServerAllocation_H
