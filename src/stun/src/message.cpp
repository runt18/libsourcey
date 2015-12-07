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


#include "scy/stun/message.h"
#include "scy/logger.h"


using namespace std;


namespace scy {
namespace stun {


Message::Message() : 
    _class(Request), 
    _method(Undefined), 
    _size(0), 
    _transactionID(util::randomString(kTransactionIdLength)) 
{
    assert(_transactionID.size() == kTransactionIdLength);
}


Message::Message(ClassType clss, MethodType meth) : 
    _class(clss), 
    _method(meth),
    _size(0), 
    _transactionID(util::randomString(kTransactionIdLength)) 
{
}


Message::Message(const Message& that) : 
    _class(that._class), 
    _method(that._method), 
    _size(that._size), 
    _transactionID(that._transactionID) 
{
    assert(_method);
    assert(_transactionID.size() == kTransactionIdLength);

    // Copy attributes from source object
    for (unsigned i = 0; i < that.attrs().size(); i++)
        _attrs.push_back(that.attrs()[i]->clone());
}


Message& Message::operator = (const Message& that) 
{
    if (&that != this) {
        _method = that._method;
        _class = that._class;
        _size = that._size;
        _transactionID = that._transactionID;
        assert(_method);
        assert(_transactionID.size() == kTransactionIdLength);    

        // Clear current attributes
        for (unsigned i = 0; i < _attrs.size(); i++)
            delete _attrs[i];
        _attrs.clear();

        // Copy attributes from source object
        for (unsigned i = 0; i < that.attrs().size(); i++)
            _attrs.push_back(that.attrs()[i]->clone());
    }

    return *this;
}


Message::~Message() 
{
    for (unsigned i = 0; i < _attrs.size(); i++)
        delete _attrs[i];
}

    
IPacket* Message::clone() const
{
    return new Message(*this);
}


void Message::add(Attribute* attr) 
{
    _attrs.push_back(attr);    
    size_t attrLength = attr->size();
    if (attrLength % 4 != 0)
        attrLength += (4 - (attrLength % 4));
    _size += attrLength + kAttributeHeaderSize;
    //_size += attr->size() + kAttributeHeaderSize;    
}


Attribute* Message::get(Attribute::Type type, int index) const 
{
    for (unsigned i = 0; i < _attrs.size(); i++) {
        if (_attrs[i]->type() == type) {            
            if (index == 0)
                return _attrs[i];
            else index--;
        }
    }
    return nullptr;
}


std::size_t Message::read(const ConstBuffer& buf) //BitReader& reader
{    
    TraceL << "Parse STUN packet: " << buf.size() << endl;
    
    try {
        BitReader reader(buf);

        // Message type
        UInt16 type;
        reader.getU16(type);
        if (type & 0x8000) {
            // RTP and RTCP set MSB of first byte, since first two bits are version, 
            // and version is always 2 (10). If set, this is not a STUN packet.
            WarnL << "Not STUN packet" << endl;
            return 0;
        }

        //UInt16 method = (type & 0x000F) | ((type & 0x00E0)>>1) | 
        //    ((type & 0x0E00)>>2) | ((type & 0x3000)>>2);
        
        UInt16 classType = type & 0x0110;
        UInt16 methodType = type & 0x000F;

        if (!isValidMethod(methodType)) {
            WarnL << "STUN message unknown method: " << methodType << endl;
            return 0;
        }
        
        _class = classType; // static_cast<UInt16>(type & 0x0110);
        _method = methodType; // static_cast<UInt16>(type & 0x000F);
                
        // Message length
        reader.getU16(_size);
        if (_size > buf.size()) {
            WarnL << "STUN message larger than buffer: " << _size << " > " << buf.size() << endl;
            return 0;
        }

        // TODO: Check valid method
        // TODO: Parse message class (Message::State)

        // Magic cookie
        reader.skip(kMagicCookieLength);
        //std::string magicCookie;
        //reader.get(magicCookie, kMagicCookieLength);
        
        // Transaction ID
        std::string transactionID;
        reader.get(transactionID, kTransactionIdLength);
        assert(transactionID.size() == kTransactionIdLength);
        _transactionID = transactionID;
    
        // Attributes
        _attrs.clear();    
        //int errors = 0;
        int rest = _size;
        UInt16 attrType, attrLength, padLength;        
        assert(int(reader.available()) >= rest);
        while (rest > 0) {
            reader.getU16(attrType);
            reader.getU16(attrLength);
            padLength =  attrLength % 4 == 0 ? 0 : 4 - (attrLength % 4);

            auto attr = Attribute::create(attrType, attrLength);
            if (attr) {        
                attr->read(reader); // parse or throw
                _attrs.push_back(attr);

                // TraceL << "Parse attribute: " << Attribute::typeString(attrType) << ": " << attrLength << endl; //  << ": " << rest
            }    
            else
                WarnL << "Failed to parse attribute: " << Attribute::typeString(attrType) << ": " << attrLength << endl;
                
            rest -= (attrLength + kAttributeHeaderSize + padLength);
        }

        TraceL << "Parse success: " << reader.position() << ": " << buf.size() << endl;
        assert(rest == 0);
        assert(reader.position() == _size + kMessageHeaderSize);
        return reader.position();
    }
    catch (std::exception& exc) {
        DebugL << "Parse error: " << exc.what() << endl;
    }
    
    return 0;
}


void Message::write(Buffer& buf) const 
{
    //assert(_method);
    //assert(_size);

    BitWriter writer(buf);
    writer.putU16((UInt16)(_class | _method));
    writer.putU16(_size);
    writer.putU32(kMagicCookie);
    writer.put(_transactionID);

    // Note: MessageIntegrity must be at the end

    for (unsigned i = 0; i < _attrs.size(); i++) {
        writer.putU16(_attrs[i]->type());
        writer.putU16(_attrs[i]->size()); 
        _attrs[i]->write(writer);
    }
}


std::string Message::classString() const 
{
    switch (_class) {
    case Request:                    return "Request";
    case Indication:                return "Indication";
    case SuccessResponse:            return "SuccessResponse";
    case ErrorResponse:                return "ErrorResponse";    
    default:                        return "UnknownState";
    }
}


std::string Message::errorString(UInt16 errorCode) const
{
    switch (errorCode) {
    case BadRequest:                return "BAD REQUEST";
    case NotAuthorized:                return "UNAUTHORIZED";
    case UnknownAttribute:            return "UNKNOWN ATTRIBUTE";
    case StaleCredentials:            return "STALE CREDENTIALS";
    case IntegrityCheckFailure:        return "INTEGRITY CHECK FAILURE";
    case MissingUsername:            return "MISSING USERNAME";
    case UseTLS:                    return "USE TLS";        
    case RoleConflict:                return "Role Conflict"; // (487) rfc5245
    case ServerError:                return "SERVER ERROR";        
    case GlobalFailure:                return "GLOBAL FAILURE";    
    case ConnectionAlreadyExists:    return "Connection Already Exists";        
    case ConnectionTimeoutOrFailure:    return "Connection Timeout or Failure";            
    default:                        return "UnknownError";
    }
}


std::string Message::methodString() const 
{
    switch (_method) {
    case Binding:                    return "BINDING";
    case Allocate:                    return "ALLOCATE";
    case Refresh:                    return "REFRESH";
    case SendIndication:            return "SEND-INDICATION";
    case DataIndication:            return "DATA-INDICATION";
    case CreatePermission:            return "CREATE-PERMISSION";
    case ChannelBind:                return "CHANNEL-BIND";        
    case Connect:                    return "CONNECT";        
    case ConnectionBind:            return "CONNECTION-BIND";        
    case ConnectionAttempt:            return "CONNECTION-ATTEMPT";            
    default:                        return "UnknownMethod";
    }
}


std::string Message::toString() const 
{
    std::ostringstream os;
    os << "STUN[" << methodString() << ":" << transactionID();
    for (unsigned i = 0; i < _attrs.size(); i++)
        os << ":" << _attrs[i]->typeString();
    os << "]";
    return os.str();
}


void Message::print(std::ostream& os) const
{
    os << "STUN[" << methodString() << ":" << transactionID();
    for (unsigned i = 0; i < _attrs.size(); i++)
        os << ":" << _attrs[i]->typeString();
    os << "]";
}


void Message::setTransactionID(const std::string& id) 
{
    assert(id.size() == kTransactionIdLength);
    _transactionID = id;
}


Message::ClassType Message::classType() const 
{ 
    return static_cast<ClassType>(_class); 
}

    
Message::MethodType Message::methodType() const 
{ 
    return static_cast<MethodType>(_method);
}


void Message::setClass(ClassType type)
{ 
    _class = type;
}

void Message::setMethod(MethodType type) 
{ 
    _method = type; 
}


} } // namespace scy:stun
