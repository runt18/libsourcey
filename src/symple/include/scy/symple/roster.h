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


#ifndef SCY_Symple_Router_H
#define SCY_Symple_Router_H


#include "scy/collection.h"
#include "scy/symple/peer.h"
#include "scy/symple/address.h"


namespace scy {
namespace smpl {


class Roster: public LiveCollection<std::string, Peer>
    /// The Roster provides a registry for active network 
    /// peers indexed by session ID.
{
public:
    typedef LiveCollection<std::string, Peer>    Manager;
    typedef Manager::Map                        PeerMap;    
    
public:
    Roster();
    virtual ~Roster();
    
    Peer* getByHost(const std::string& host);
        // Returns the first peer which matches the given host address.
    
    virtual PeerMap peers() const;
    
    virtual void print(std::ostream& os) const;

    //virtual const char* className() const { return "smpl::Roster"; }
};
    

} } // namespace scy::smpl


#endif //  SCY_Symple_Router_H