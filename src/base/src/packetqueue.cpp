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

#include "scy/packetqueue.h"


using std::endl;


namespace scy {


//
// Synchronization Packet Queue
//


SyncPacketQueue::SyncPacketQueue(uv::Loop* loop, int maxSize) : 
    SyncQueue<IPacket>(loop, maxSize), 
    PacketProcessor(this->emitter)
{    
    TraceLS(this) << "Create" << endl;
}


SyncPacketQueue::SyncPacketQueue(int maxSize) : 
    SyncQueue<IPacket>(uv::defaultLoop(), maxSize), 
    PacketProcessor(this->emitter)
{    
    TraceLS(this) << "Create" << endl;
}
    

SyncPacketQueue::~SyncPacketQueue()
{
    TraceLS(this) << "Destroy" << endl;
}


void SyncPacketQueue::process(IPacket& packet)
{
    if (cancelled()) {
        WarnLS(this) << "Process late packet" << endl;
        assert(0);
        return;
    }
    
    push(packet.clone());
}


void SyncPacketQueue::dispatch(IPacket& packet)
{    
    // Emit should never be called after closure.
    // Any late packets should have been dealt with  
    // and dropped by the run() function.
    if (cancelled()) {
        WarnLS(this) << "Dispatch late packet" << endl;
        assert(0);
        return;
    }
    
    PacketStreamAdapter::emit(packet);
}


void SyncPacketQueue::onStreamStateChange(const PacketStreamState& state)
{
    TraceLS(this) << "Stream state: " << state << endl;
    
    switch (state.id()) {
    //case PacketStreamState::None:
    //case PacketStreamState::Active:
    //case PacketStreamState::Resetting:
    //case PacketStreamState::Stopping:
    //case PacketStreamState::Stopped:
    case PacketStreamState::Closed:
    case PacketStreamState::Error:
        SyncQueue<IPacket>::cancel();
        break;
    }
}


//
// Asynchronous Packet Queue
//


AsyncPacketQueue::AsyncPacketQueue(int maxSize) : 
    AsyncQueue<IPacket>(maxSize), 
    PacketProcessor(this->emitter)
{    
    TraceLS(this) << "Create" << endl;
}
    

AsyncPacketQueue::~AsyncPacketQueue()
{
    TraceLS(this) << "Destroy" << endl;
}


void AsyncPacketQueue::process(IPacket& packet)
{
    if (cancelled()) {
        WarnLS(this) << "Process late packet" << endl;
        assert(0);
        return;
    }
    
    push(packet.clone());
}


void AsyncPacketQueue::dispatch(IPacket& packet)
{
    if (cancelled()) {
        WarnLS(this) << "Dispatch late packet" << endl;
        assert(0);
        return;
    }

    PacketStreamAdapter::emit(packet);
}


void AsyncPacketQueue::onStreamStateChange(const PacketStreamState& state)
{
    TraceLS(this) << "Stream state: " << state << endl;
    
    switch (state.id()) {
    case PacketStreamState::Active:
        break;
        
    case PacketStreamState::Stopped:
        break;

    case PacketStreamState::Error:
    case PacketStreamState::Closed:
        // Flush queued items, some protocols can't afford dropped packets
        flush();    
        assert(empty());
        cancel();
        _thread.join();
        break;

    //case PacketStreamState::Resetting:
    //case PacketStreamState::None:
    //case PacketStreamState::Stopping:
    }
}


} // namespace scy