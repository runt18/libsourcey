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


#include "scy/symple/presence.h"
#include "scy/util.h"
#include "assert.h"


using std::endl;


namespace scy {
namespace smpl {


Presence::Presence() 
{
    (*this)["type"] = "presence";
}


Presence::Presence(const Presence& root) :
    Message(root)
{
    if (!isMember("type"))
        (*this)["type"] = "presence";
}


Presence::Presence(const json::Value& root) :
    Message(root)
{
    if (!isMember("type"))
        (*this)["type"] = "presence";
}


Presence::~Presence() 
{
}


bool Presence::isProbe() 
{
    return (*this)["probe"].asBool();
}


void Presence::setProbe(bool flag)
{    
    (*this)["probe"] = flag;
}


} // namespace symple 
} // namespace scy