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


#ifndef SCY_UserManager_H
#define SCY_UserManager_H

#include "scy/collection.h"
#include <map>


namespace scy {


struct IUser 
{
    virtual std::string username() const = 0;
    virtual std::string password() const = 0;
};


class BasicUser: public IUser 
{
public:
    BasicUser(const std::string& username, 
              const std::string& password = "") :
        _username(username), 
        _password(password) {}
    
    std::string username() const { return _username; }
    std::string password() const { return _password; }

protected:
    std::string _username;
    std::string _password;
};


typedef std::map<std::string, IUser*> IUserMap;


class UserManager: public LiveCollection<std::string, IUser>
    /// This class contains a list of users that have access
    /// on the system.
    ///
    /// NOTE: This class is depreciated.
{
public:
    typedef LiveCollection<std::string, IUser>    Manager;
    typedef Manager::Map                        Map;

public:
    UserManager() {};
    virtual ~UserManager() {};

    virtual bool add(IUser* user) {
        return Manager::add(user->username(), user);
    };
};


} // namespace scy


#endif // SCY_UserManager_H