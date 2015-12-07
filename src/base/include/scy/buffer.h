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


#ifndef SCY_Buffer_H
#define SCY_Buffer_H


#include "scy/types.h"
#include "scy/memory.h"
#include "scy/byteorder.h"

#include <string>
#include <vector>
#include <algorithm>


namespace scy {


typedef std::vector<char> Buffer;


//
// Mutable Buffer
//


class MutableBuffer
    /// The MutableBuffer class provides a safe representation of a
    /// buffer that can be modified. It does not own the underlying
    /// data, and so is cheap to copy or assign.
{
public:
    MutableBuffer() : 
        _data(0), _size(0)
        // Construct an empty buffer.
    {
    }

    MutableBuffer(void* data, std::size_t size) : 
        _data(data), _size(size)
        // Construct a buffer to represent the given memory range.
    {
    }
        
    void* data() const { return _data; }
    std::size_t size() const { return _size; }

private:
    void* _data;
    std::size_t _size;
};


// Warning: The following functions permit violations of type safety, 
// so uses of it in application code should be carefully considered.


template<typename T> inline MutableBuffer mutableBuffer(T data, std::size_t size)
{
    return MutableBuffer(reinterpret_cast<void*>(data), size);
}

inline MutableBuffer mutableBuffer(std::string& str) 
{
    return MutableBuffer(reinterpret_cast<void*>(&str[0]), str.size()); // std::string is contiguous as of C++11
}


inline MutableBuffer mutableBuffer(const std::string& str)
{
    return MutableBuffer(reinterpret_cast<void*>(const_cast<char*>(&str[0])), str.size()); // careful!
}


template<typename T> inline MutableBuffer mutableBuffer(const std::vector<T>& vec)
{
    return MutableBuffer(reinterpret_cast<void*>(const_cast<T>(&vec[0])), vec.size()); // careful!
}


inline MutableBuffer mutableBuffer(Buffer& buf)
{
    return MutableBuffer(reinterpret_cast<void*>(buf.data()), buf.size());
}


inline MutableBuffer mutableBuffer(const Buffer& buf)
{
    return MutableBuffer(reinterpret_cast<void*>(const_cast<char*>(buf.data())), buf.size());
}


//
// Const Buffer
//


class ConstBuffer
    /// The ConstBuffer class provides a safe representation of a 
    /// buffer that cannot be modified. It does not own the underlying
    /// data, and so is cheap to copy or assign.
{
public:
    ConstBuffer() : 
        _data(0), _size(0)
        // Construct an empty buffer.
    {
    }

    ConstBuffer(const void* data, std::size_t size) : 
        _data(data), _size(size)
        // Construct a buffer to represent the given memory range.
    {
    }

    ConstBuffer(const MutableBuffer& b) : 
        _data(b.data()), _size(b.size())
        // Construct a non-modifiable buffer from a modifiable one.
    {
    }
            
    const void* data() const { return _data; }
    std::size_t size() const { return _size; }

private:
    //friend const void* bufferCastHelper(const ConstBuffer& b);
    //friend std::size_t bufferSizeHelper(const ConstBuffer& b);

    const void* _data;
    std::size_t _size;
};


/*
inline const void* bufferCastHelper(const ConstBuffer& b)
{
    return b._data;
}


inline std::size_t bufferSizeHelper(const ConstBuffer& b)
{
    return b._size;
}
*/


template<typename T> inline ConstBuffer constBuffer(T data, std::size_t size)
{
    return ConstBuffer(reinterpret_cast<const void*>(data), size);
}

inline ConstBuffer constBuffer(const std::string& str)
{
    return ConstBuffer(reinterpret_cast<const void*>(&str[0]), str.size()); // careful!
}

template<typename T> inline ConstBuffer constBuffer(const std::vector<T>& vec)
{
    return ConstBuffer(reinterpret_cast<const void*>(&vec[0]), vec.size()); // careful!
}

inline ConstBuffer constBuffer(const MutableBuffer& buf)
{
    return ConstBuffer(buf.data(), buf.size());
}

template<typename T> inline ConstBuffer constBuffer(Buffer& buf)
{
    return ConstBuffer(reinterpret_cast<void*>(buf.data()), buf.size());
}

template<typename T> inline ConstBuffer constBuffer(const Buffer& buf)
{
    return ConstBuffer(reinterpret_cast<void*>(const_cast<char *>(buf.data())), buf.size());
}


//
// Buffer Cast
//


template <typename PointerToPodType>
inline PointerToPodType bufferCast(const MutableBuffer& b)
    /// Cast a non-modifiable buffer to a specified pointer to POD type.
{
    return static_cast<PointerToPodType>(b.data());
}

template <typename PointerToPodType>
inline PointerToPodType bufferCast(const ConstBuffer& b)
    /// Cast a non-modifiable buffer to a specified pointer to POD type.
{
    return static_cast<PointerToPodType>(b.data());
}


//
// Bit Reader
//


class BitReader 
    /// A BitReader for reading binary streams.
{
public:
    BitReader(const char* bytes, std::size_t size, ByteOrder order = ByteOrder::Network);
    BitReader(const Buffer& buf, ByteOrder order = ByteOrder::Network);
    BitReader(const ConstBuffer& pod, ByteOrder order = ByteOrder::Network);
    ~BitReader();

    void get(char* val, std::size_t len);
    void get(std::string& val, std::size_t len);
    void getU8(UInt8& val);
    void getU16(UInt16& val);
    void getU24(UInt32& val);
    void getU32(UInt32& val);
    void getU64(UInt64& val);
        // Reads a value from the BitReader. 
        // Returns false if there isn't enough data left for the specified type.
        // Throws a std::out_of_range exception if reading past the limit.

    const char peek();
    const UInt8 peekU8();
    const UInt16 peekU16();
    const UInt32 peekU24();
    const UInt32 peekU32();
    const UInt64 peekU64();
        // Peeks data from the BitReader. 
        // -1 is returned if reading past boundary.

    int skipToChar(char c);
    int skipWhitespace();
    int skipToNextLine();
    int skipNextWord();
    int readNextWord(std::string& val);
    int readNextNumber(unsigned int& val);
    int readLine(std::string& val);
    int readToNext(std::string& val, char c);
        // String parsing methods.

    void seek(std::size_t val);
        // Set position pointer to absolute position.
        // Throws a std::out_of_range exception if the value exceeds the limit.

    void skip(std::size_t size);
        // Set position pointer to relative position.
        // Throws a std::out_of_range exception if the value exceeds the limit.

    std::size_t limit() const;
        // Returns the read limit.

    std::size_t position() const { return _position; }
        // Returns the current read position.

    std::size_t available() const;
        // Returns the number of elements between the current position and the limit.

    const char* begin() const { return _bytes; }
    const char* current() const { return _bytes + _position; }

    ByteOrder order() const { return _order; }

    std::string toString();

    friend std::ostream& operator << (std::ostream& stream, const BitReader& buf) 
    {
        return stream.write(buf.current(), buf.position());
    }

private:
    void init(const char* bytes, std::size_t size, ByteOrder order); // nocopy

    std::size_t _position;
    std::size_t _limit;
    const char* _bytes;
    ByteOrder _order;
};


//
// Bit Writer
//


class BitWriter 
    /// A BitWriter for reading/writing binary streams.
    ///
    /// Note that when using the constructor with the Buffer reference
    /// as an argument, the writer will dynamically expand the given buffer
    /// when writing passed the buffer capacity.
    /// All other cases will throw a std::out_of_range error when writing
    /// past the buffer capacity.
{
public:    
    BitWriter(char* bytes, std::size_t size, ByteOrder order = ByteOrder::Network);
    BitWriter(Buffer& buf, ByteOrder order = ByteOrder::Network);
    BitWriter(MutableBuffer& pod, ByteOrder order = ByteOrder::Network);
    ~BitWriter();

    void put(const char* val, std::size_t len);
    void put(const std::string& val);
    void putU8(UInt8 val);
    void putU16(UInt16 val);
    void putU24(UInt32 val);
    void putU32(UInt32 val);
    void putU64(UInt64 val);
        // Append bytes to the buffer.
        // Throws a std::out_of_range exception if reading past the limit.

    bool update(const char* val, std::size_t len, std::size_t pos);
    bool update(const std::string& val, std::size_t pos);
    bool updateU8(UInt8 val, std::size_t pos);
    bool updateU16(UInt16 val, std::size_t pos);
    bool updateU24(UInt32 val, std::size_t pos);
    bool updateU32(UInt32 val, std::size_t pos);
    bool updateU64(UInt64 val, std::size_t pos);
        // Update a byte range.
        // Throws a std::out_of_range exception if reading past the limit.

    void seek(std::size_t val);
        // Set position pointer to absolute position.
        // Throws a std::out_of_range exception if the value exceeds the limit.

    void skip(std::size_t size);
        // Set position pointer to relative position.
        // Throws a std::out_of_range exception if the value exceeds the limit.

    std::size_t limit() const;
        // Returns the write limit.

    std::size_t position() const { return _position; }
        // Returns the current write position.

    std::size_t available() const;
        // Returns the number of elements between the current write position and the limit.

    char* begin() { return _bytes; }
    char* current() { return _bytes + _position; }

    const char* begin() const { return _bytes; }
    const char* current() const { return _bytes + _position; }

    ByteOrder order() const { return _order; }

    std::string toString();
        // Returns written bytes as a string.

    friend std::ostream& operator << (std::ostream& stream, const BitWriter& wr) 
    {
        return stream.write(wr.begin(), wr.position());
    }

private:
    void init(char* bytes, std::size_t size, ByteOrder order); // nocopy

    std::size_t _position;
    std::size_t _limit;
    ByteOrder _order;
    Buffer* _buffer;
    char* _bytes;
};


} // namespace scy


#endif  // SCY_Buffer_H