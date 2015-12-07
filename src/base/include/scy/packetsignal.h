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


#ifndef SCY_PacketSignal_H
#define SCY_PacketSignal_H


#include "scy/types.h"
#include "scy/packet.h"
#include "scy/polymorphicsignal.h"


namespace scy {
    
    
typedef DelegateBase<IPacket&> PacketDelegateBase;
typedef SignalBase<PacketDelegateBase, IPacket&> PacketSignal;
typedef std::vector<PacketSignal*> PacketSignalVec;

DefinePolymorphicDelegate(packetDelegate, IPacket, PacketDelegateBase)


} // namespace scy


#endif // SCY_PacketSignal_H