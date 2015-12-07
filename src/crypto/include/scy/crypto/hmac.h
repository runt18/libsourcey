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


#ifndef SCY_Crypto_HMAC_H
#define SCY_Crypto_HMAC_H


#include "scy/crypto/crypto.h"
#include <string>


namespace scy {
namespace crypto {
    
    
std::string computeHMAC(const std::string& input, const std::string& key);
    /// HMAC is a MAC (message authentication code), i.e. a keyed hash function 
    /// used for message authentication, which is based on a hash function (SHA1).
    ///
    /// Input is the data to be signed, and key is the private password.
    

} } // namespace scy::crypto


#endif // SCY_Crypto_HMAC_H