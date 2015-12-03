#include "scy/application.h"
#include "scy/packetstream.h"
#include "scy/media/iencoder.h"
#include "scy/net/network.h"
#include "scy/http/server.h"
#include "scy/http/packetizers.h"


using namespace std;
using namespace scy;


/*
// Detect Memory Leaks
#ifdef _DEBUG
#include "MemLeakDetect/MemLeakDetect.h"
#include "MemLeakDetect/MemLeakDetect.cpp"
CMemLeakDetect memLeakDetect;
#endif
*/


#define USE_AVDEVICE_CAPTURE 1

#if USE_AVDEVICE_CAPTURE
#include "scy/media/avinputreader.h"
#include "scy/media/avpacketencoder.h"
#define VIDEO_FILE_SOURCE "~/test.mp4"
av::AVInputReader* gVideoCapture;
#else
#include "scy/media/videocapture.h"
av::VideoCapture* gVideoCapture;
#endif


namespace scy {


class MPEGResponder: public http::ServerResponder
{
	av::FPSCounter fpsCounter;
	PacketStream* stream;

public:
	MPEGResponder(http::ServerConnection& conn) :
		http::ServerResponder(conn)
	{
		DebugL << "Creating" << endl;

		// We will be sending our own headers
		conn.shouldSendHeader(false);

		// Attach the video capture
		stream->attachSource(gVideoCapture, false);

		// Setup the encoder options
		av::EncoderOptions options;
		options.oformat = av::Format("MJPEG", "mjpeg", av::VideoCodec(
			"MJPEG", "mjpeg", 400, 300, 25, 48000, 128000, "yuvj420p"));
		gVideoCapture->getEncoderFormat(options.iformat);

		// Create and attach the encoder
		av::AVPacketEncoder* encoder = new av::AVPacketEncoder(options);
		encoder->initialize();
		stream->attach(encoder, 5, true);

		// Create and attach the HTTP multipart packetizer
		auto packetizer = new http::MultipartAdapter("image/jpeg", false);
		stream->attach(packetizer, 10, true);
		//assert(0 && "fixme");

		// Start the stream
		stream->emitter += packetDelegate(this, &MPEGResponder::onVideoEncoded);
		stream->start();
	}

	~MPEGResponder()
	{
		DebugL << "Destroying" << endl;
		//stream->destroy();
		delete stream;
	}

	void onPayload(const Buffer& body)
	{
		DebugL << "On recv payload: " << body.size() << endl;

		// do something with data from peer
	}

	void onClose()
	{
		DebugL << "On close" << endl;

		stream->emitter += packetDelegate(this, &MPEGResponder::onVideoEncoded);
		stream->stop();
	}

	void onVideoEncoded(void* sender, RawPacket& packet)
	{
		TraceL << "Sending packet: "
			<< packet.size() << ": " << fpsCounter.fps << endl;

		try {
			connection().send(packet.data(), packet.size());
			fpsCounter.tick();
		}
		catch (std::exception/*Exception*/& exc) {
			ErrorL << "Error: " << std::string(exc.what())/*message()*/ << endl;
			connection().close();
		}
	}
};


// -------------------------------------------------------------------
//
class StreamingResponderFactory: public http::ServerResponderFactory
{
public:
	http::ServerResponder* createResponder(http::ServerConnection& conn)
	{
		return new MPEGResponder(conn);
	}
};


} // namespace scy


static void onShutdownSignal(void* opaque)
{
	reinterpret_cast<http::Server*>(opaque)->shutdown();
}


int main(int argc, char** argv)
{
	Logger::instance().add(new ConsoleChannel("debug", LTrace));

#if USE_AVDEVICE_CAPTURE
	gVideoCapture = new av::AVInputReader();
	//gAVVideoCapture->openDevice(0);
	gVideoCapture->openFile(VIDEO_FILE_SOURCE);
	gVideoCapture->start();
#else
	// VideoCapture instances must be
	// instantiated in the main thread.
	gVideoCapture = new av::VideoCapture(0);
#endif

	{
		Application app;
		http::Server server(328, new StreamingResponderFactory);
		server.start();
		app.waitForShutdown(onShutdownSignal, &server);
	}

	delete gVideoCapture;
	Logger::destroy();
	return 0;
}
