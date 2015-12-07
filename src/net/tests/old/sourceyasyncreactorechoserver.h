#include "Poco/Thread.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/ServerSocket.h"
#include "AsyncReactor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/SocketAcceptor.h"
#include "scy/logger.h"
#include "assert.h"


namespace Sourcey {
namespace Net {


int numAsyncEchoServiceHandler;
int numAsyncReadableNotification;


        /*    
class EchoServiceHandler
{
public:
    EchoServiceHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketAsyncReactor& reactor):
        _socket(socket),
        _reactor(reactor)
    {        
        numAsyncEchoServiceHandler++;
        _id = numAsyncEchoServiceHandler;

        Sourcey::Log("trace") << "[EchoServiceHandler:" << this << "] " << _id << ": EchoServiceHandler: " << socket.available() << std::endl;

        _reactor.addEventHandler(_socket, Poco::Observer<EchoServiceHandler, Poco::Net::ReadableNotification>(*this, &EchoServiceHandler::onReadable));
            
    
        // In high traffic situations the socket may become
        // writable with data in the read buffers, and the
        // readable notification get's missed. Let's flush 
        // the beffers now...
                    
        if (socket.available() > 0) {
            numAsyncReadableNotification++;
            std::stringstream ss;
            ss << _id;
            _socket.sendBytes(ss.str().data(), ss.str().length());
            Sourcey::Log("debug") << "[EchoServiceHandler:" << this << "] " << _id << ": Sent" << std::endl;

            return;
            //std::cout << ">>>>>>>>>>>>>>>>>>>> " << std::endl; 
            //_socket.sendBytes("hitme", 5);
            //onReadable(rNf);
            Poco::Net::ReadableNotification* rNf = new Poco::Net::ReadableNotification(&reactor);
            onReadable(rNf);
        }

    }
        
    ~EchoServiceHandler()
    {
        _reactor.removeEventHandler(_socket, Poco::Observer<EchoServiceHandler, Poco::Net::ReadableNotification>(*this, &EchoServiceHandler::onReadable));
    }
        
        
    void doit()
    {    
        printf("EchoServiceHandler:doit\n");
    }


    void onReadable(Poco::Net::ReadableNotification* pNf)
    {        
        Sourcey::Log("debug") << "[EchoServiceHandler:" << this << "] " << _id << ": onReadable" << std::endl;

        numAsyncReadableNotification++;
        
        //pNf->duplicate();
        //pNf->release();
        char buffer[8];
        int n = _socket.receiveBytes(buffer, sizeof(buffer));
        if (n > 0)
        {
            std::stringstream ss;
            ss << _id;
            _socket.sendBytes(ss.str().data(), ss.str().length());
            Sourcey::Log("debug") << "[EchoServiceHandler:" << this << "] " << _id << ": Sent" << std::endl;

            //std::cout << ">>>>>>>>>>>>>>>>>>>> " << std::endl;     
        }
        else
        {
            _socket.shutdownSend();
            delete this;
        }
    }
        
private:
    int   _id;
    Poco::Net::StreamSocket   _socket;
    Poco::Net::SocketAsyncReactor& _reactor;
};




class AsyncReactorSocket: public IAsyncReactorHandler
{    
    virtual void onReadable() {
        Sourcey::Log("debug") << "[AsyncReactorSocket:" << this << "] onReadable" << std::endl;
    };

    virtual void onWritable() {
        Sourcey::Log("debug") << "[AsyncReactorSocket:" << this << "] onWritable" << std::endl;
    };

    virtual void onError() {
        Sourcey::Log("debug") << "[AsyncReactorSocket:" << this << "] onError" << std::endl;
    };

    virtual void onIdle() {
        Sourcey::Log("debug") << "[AsyncReactorSocket:" << this << "] onIdle" << std::endl;
    };
};
*/


struct AsyncReactorSocket: public IAsyncReactorHandler
{
    Poco::Net::ServerSocket server;
    std::vector<Poco::Net::StreamSocket> connections;

    AsyncReactorSocket(const Poco::Net::ServerSocket& socket) : server(socket)
    {
    };

    virtual void onReadable(AsyncReactorNotification& n) {
        
        connections.push_back(server.acceptConnection()); //impl()->
        Poco::Net::StreamSocket ss = connections.back(); //server.acceptConnection();
        Sourcey::Log("debug") << "[PocoEchoServer:" << this << "] acceptConnection" << std::endl;

        /*
        try
        {
            char buffer[256];
            int size = ss.receiveBytes(buffer, sizeof(buffer));
            while (size > 0)
            {
                ss.sendBytes(buffer, size);

                // read once
                break;
                //n = ss.receiveBytes(buffer, sizeof(buffer));

            }
        }
        catch (Poco::Exception& exc)
        {
            std::cerr << "EchoServer: " << exc.displayText() << std::endl;
        }
        */
        Sourcey::Log("debug") << "[PocoEchoServer:" << this << "] acceptConnection: OK" << std::endl;
    };

    virtual void onWritable(AsyncReactorNotification& n) {
        Sourcey::Log("debug") << "[AsyncReactorSocket:" << this << "] onWritable" << std::endl;
    };

    virtual void onError(AsyncReactorNotification& n) {
        Sourcey::Log("debug") << "[AsyncReactorSocket:" << this << "] onError" << std::endl;
    };

    virtual void onIdle() {
        Sourcey::Log("debug") << "[AsyncReactorSocket:" << this << "] onIdle" << std::endl;
    };
};


class SourceyAsyncReactorEchoServer: public Poco::Runnable
{
public:    
    SourceyAsyncReactorEchoServer(short port, Sourcey::Net::AsyncReactor& reactor):
        _socket(Poco::Net::SocketAddress("127.0.0.1", port)),
        _reactor(reactor),
        _handler(_socket),
        _thread("SourceyAsyncReactorEchoServer")
    {
        numAsyncEchoServiceHandler = 0;
        numAsyncReadableNotification = 0;
        _thread.start(*this);
        _ready.wait();
    }

    virtual ~SourceyAsyncReactorEchoServer()
    {
        _reactor.stop();
        _thread.join();
    }

    Poco::UInt16 port() const
    {
        return _socket.address().port();
    }
    
    void run()
    {        
        Sourcey::Log("debug") << "[SourceyAsyncReactorEchoServer:" << this << "] Listening: " << _socket.address().toString() << std::endl;
        //_socket.bind(_socket.address());
        //_socket.listen(1000);
        //Poco::Net::SocketAcceptor<EchoServiceHandler> acceptor(_socket, _reactor);
        //Sourcey::Log("debug") << "[UVEchoServer:" << this << "] Liistening 1: " << _socket.address().toString() << std::endl;
        _ready.set();
        //Sourcey::Log("debug") << "[UVEchoServer:" << this << "] Liistening 2: " << _socket.address().toString() << std::endl;
        _reactor.attach(_socket, _handler, AsyncReactor::Readable);
        _reactor.run();
    }

protected:
    Poco::Net::ServerSocket _socket;
    Sourcey::Net::AsyncReactor& _reactor;
    AsyncReactorSocket _handler;
    Poco::Thread _thread;
    Poco::Event  _ready;
};


} } // namespace Sourcey::Net