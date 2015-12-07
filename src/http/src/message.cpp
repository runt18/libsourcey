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

#include "scy/http/message.h"


namespace scy {
namespace http {


const std::string Message::HTTP_1_0                   = "HTTP/1.0";
const std::string Message::HTTP_1_1                   = "HTTP/1.1";
const std::string Message::IDENTITY_TRANSFER_ENCODING = "identity";
const std::string Message::CHUNKED_TRANSFER_ENCODING  = "chunked";
const int         Message::UNKNOWN_CONTENT_LENGTH     = -1;
const std::string Message::UNKNOWN_CONTENT_TYPE;
const std::string Message::CONTENT_LENGTH             = "Content-Length";
const std::string Message::CONTENT_TYPE               = "Content-Type";
const std::string Message::TRANSFER_ENCODING          = "Transfer-Encoding";
const std::string Message::CONNECTION                 = "Connection";
const std::string Message::CONNECTION_KEEP_ALIVE      = "Keep-Alive";
const std::string Message::CONNECTION_CLOSE           = "Close";
const std::string Message::EMPTY;


Message::Message() :
    _version(HTTP_1_1)
{
}


Message::Message(const std::string& version) :
    _version(version)
{
}


Message::~Message()
{
}


void Message::setVersion(const std::string& version)
{
    _version = version;
}


void Message::setContentLength(UInt64 length)
{
    if (int(length) != UNKNOWN_CONTENT_LENGTH)
        set(CONTENT_LENGTH, util::itostr<UInt64>(length));
    else
        erase(CONTENT_LENGTH);
}

    
UInt64 Message::getContentLength() const
{
    const std::string& contentLength = get(CONTENT_LENGTH, EMPTY);
    if (!contentLength.empty())
    {
        return util::strtoi<UInt64>(contentLength);
    }
    else return UInt64(UNKNOWN_CONTENT_LENGTH);
}


void Message::setTransferEncoding(const std::string& transferEncoding)
{
    if (util::icompare(transferEncoding, IDENTITY_TRANSFER_ENCODING) == 0)
        erase(TRANSFER_ENCODING);
    else
        set(TRANSFER_ENCODING, transferEncoding);
}


const std::string& Message::getTransferEncoding() const
{
    return get(TRANSFER_ENCODING, IDENTITY_TRANSFER_ENCODING);
}


void Message::setChunkedTransferEncoding(bool flag)
{
    if (flag)
        setTransferEncoding(CHUNKED_TRANSFER_ENCODING);
    else
        setTransferEncoding(IDENTITY_TRANSFER_ENCODING);
}

    
bool Message::isChunkedTransferEncoding() const
{
    return util::icompare(getTransferEncoding(), CHUNKED_TRANSFER_ENCODING) == 0;
}

    
void Message::setContentType(const std::string& contentType)
{
    if (contentType.empty())
        erase(CONTENT_TYPE);
    else
        set(CONTENT_TYPE, contentType);
}

    
const std::string& Message::getContentType() const
{
    return get(CONTENT_TYPE, UNKNOWN_CONTENT_TYPE);
}


void Message::setKeepAlive(bool keepAlive)
{
    if (keepAlive)
        set(CONNECTION, CONNECTION_KEEP_ALIVE);
    else
        set(CONNECTION, CONNECTION_CLOSE);
}


bool Message::getKeepAlive() const
{
    const std::string& connection = get(CONNECTION, EMPTY);
    if (!connection.empty())
        return util::icompare(connection, CONNECTION_CLOSE) != 0;
    else
        return getVersion() == HTTP_1_1;
}


const std::string& Message::getVersion() const
{
    return _version;
}


bool Message::hasContentLength() const
{
    return has(CONTENT_LENGTH);
}


void Message::write(std::ostream& ostr) const
{
    NVCollection::ConstIterator it = begin();
    while (it != end()) {
        ostr << it->first << ": " << it->second << "\r\n";
        ++it;
    }
}


} } // namespace scy::http


//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
