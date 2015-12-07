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


#ifndef SCY_StreamManager_H
#define SCY_StreamManager_H


#include "scy/collection.h"
#include "scy/packetstream.h"


namespace scy { 
    

typedef LiveCollection<
    std::string, PacketStream, 
        std::default_delete<PacketStream>
        //DeferredDeleter<PacketStream>
        //DestroyMethodDeleter<PacketStream>
> StreamManagerBase;


class StreamManager: public StreamManagerBase, public basic::Polymorphic
{
public:
    typedef StreamManagerBase Manager;
    typedef Manager::Map      Map;

public:
    StreamManager(bool freeClosedStreams = true);
    virtual ~StreamManager();

    virtual bool addStream(PacketStream* stream, bool whiny = true);    
    virtual bool closeStream(const std::string& name, bool whiny = true);    
    virtual void closeAll();    
    virtual PacketStream* getStream(const std::string& name, bool whiny = true);
    virtual PacketStream* getDafaultStream();
        // Returns the first stream in the list, or NULL.

    virtual Map streams() const;

    virtual void print(std::ostream& os) const;

protected:    
    virtual void onAdd(PacketStream* task);
        // Called after a stream is added.

    virtual void onRemove(PacketStream* task);
        // Called after a stream is removed.

    virtual void onStreamStateChange(void* sender, PacketStreamState& state, const PacketStreamState&);    

    virtual const char* className() const { return "Stream Manager"; };
    
protected:    
    bool _freeClosedStreams;
};


} // namespace scy


#endif // SCY_StreamManager_H