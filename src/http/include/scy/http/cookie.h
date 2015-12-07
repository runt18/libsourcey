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


#ifndef SCY_HTTP_Cookie_H
#define SCY_HTTP_Cookie_H


#include "scy/collection.h"
#include "scy/net/types.h"


namespace scy {
namespace http {


class Cookie
    /// This class represents a HTTP Cookie.
    ///
    /// A cookie is a small amount of information sent by a Web 
    /// server to a Web browser, saved by the browser, and later sent back 
    /// to the server. A cookie's value can uniquely identify a client, so 
    /// cookies are commonly used for session management.
    ///
    /// A cookie has a name, a single value, and optional attributes such 
    /// as a comment, path and domain qualifiers, a maximum age, and a 
    /// version number.
    ///
    /// This class supports both the Version 0 (by Netscape) and Version 1 
    /// (by RFC 2109) cookie specifications. By default, cookies are created 
    /// using Version 0 to ensure the best interoperability.
{
public:
    Cookie();
        /// Creates an empty Cookie.
        
    explicit Cookie(const std::string& name);
        /// Creates a cookie with the given name.    
        /// The cookie never expires.
        
    explicit Cookie(const NVCollection& nvc);
        /// Creates a cookie from the given NVCollection.
        
    Cookie(const std::string& name, const std::string& value);
        /// Creates a cookie with the given name and value.
        /// The cookie never expires.
        //
        /// Note: If value contains whitespace or non-alphanumeric
        /// characters, the value should be escaped by calling escape()
        /// before passing it to the constructor.
        
    Cookie(const Cookie& cookie);
        /// Creates the Cookie by copying another one.

    ~Cookie();
        /// Destroys the Cookie.
        
    Cookie& operator = (const Cookie& cookie);
        /// Assigns a cookie.
        
    void setVersion(int version);
        /// Sets the version of the cookie.
        //
        /// Version must be either 0 (denoting a Netscape cookie)
        /// or 1 (denoting a RFC 2109 cookie).
        
    int getVersion() const;
        /// Returns the version of the cookie, which is
        /// either 0 or 1.    
        
    void setName(const std::string& name);
        /// Sets the name of the cookie.
        
    const std::string& getName() const;
        /// Returns the name of the cookie.
        
    void setValue(const std::string& value);
        /// Sets the value of the cookie.
        //
        /// According to the cookie specification, the
        /// size of the value should not exceed 4 Kbytes.
        //
        /// Note: If value contains whitespace or non-alphanumeric
        /// characters, the value should be escaped by calling escape()
        /// prior to passing it to setName().
        
    const std::string& getValue() const;
        /// Returns the value of the cookie.
        
    void setComment(const std::string& comment);
        /// Sets the comment for the cookie.
        //
        /// Comments are only supported for version 1 cookies.

    const std::string& getComment() const;
        /// Returns the comment for the cookie.

    void setDomain(const std::string& domain);
        /// Sets the domain for the cookie.
        
    const std::string& getDomain() const;
        /// Returns the domain for the cookie.

    void setPath(const std::string& path);
        /// Sets the path for the cookie.
        
    const std::string& getPath() const;
        /// Returns the path for the cookie.

    void setSecure(bool secure);
        /// Sets the value of the secure flag for
        /// the cookie.
        
    bool getSecure() const;
        /// Returns the value of the secure flag
        /// for the cookie.

    void setMaxAge(int maxAge);
        /// Sets the maximum age in seconds for
        /// the cookie.
        //
        /// A value of -1 causes the cookie to
        /// never expire on the client.
        //
        /// A value of 0 deletes the cookie on
        /// the client.

    int getMaxAge() const;
        /// Returns the maximum age in seconds for
        /// the cookie.
        
    void setHttpOnly(bool flag = true);
        /// Sets the HttpOnly flag for the cookie.
        
    bool getHttpOnly() const;
        /// Returns true if the cookie's HttpOnly flag is set.
        
    std::string toString() const;
        /// Returns a std::string representation of the cookie,
        /// suitable for use in a Set-Cookie header.
        
    static std::string escape(const std::string& str);
        /// Escapes the given std::string by replacing all 
        /// non-alphanumeric characters with escape
        /// sequences in the form %xx, where xx is the
        /// hexadecimal character code.
        //
        /// The following characters will be replaced
        /// with escape sequences:
        ///   - percent sign %
        ///   - less-than and greater-than < and >
        ///   - curly brackets { and }
        ///   - square brackets [ and ]
        ///   - parenthesis ( and )
        ///   - solidus /
        ///   - vertical line |
        ///   - reverse solidus (backslash /)
        ///   - quotation mark "
        ///   - apostrophe '
        ///   - circumflex accent ^
        ///   - grave accent `
        ///   - comma and semicolon , and ;
        ///   - whitespace and control characters
        
    static std::string unescape(const std::string& str);
        /// Unescapes the given std::string by replacing all
        /// escape sequences in the form %xx with the
        /// respective characters.

private:
    int         _version;
    std::string _name;
    std::string _value;
    std::string _comment;
    std::string _domain;
    std::string _path;
    bool        _secure;
    int         _maxAge;
    bool        _httpOnly;
};


inline int Cookie::getVersion() const
{
    return _version;
}


inline const std::string& Cookie::getName() const
{
    return _name;
}


inline const std::string& Cookie::getValue() const
{
    return _value;
}


inline const std::string& Cookie::getComment() const
{
    return _comment;
}


inline const std::string& Cookie::getDomain() const
{
    return _domain;
}


inline const std::string& Cookie::getPath() const
{
    return _path;
}


inline bool Cookie::getSecure() const
{
    return _secure;
}


inline int Cookie::getMaxAge() const
{
    return _maxAge;
}


inline bool Cookie::getHttpOnly() const
{
    return _httpOnly;
}


} } // namespace scy::http


#endif // SCY_HTTP_Cookie_H


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
