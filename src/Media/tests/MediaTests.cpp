#include "Sourcey/Signal.h"
#include "Sourcey/PacketStream.h"
#include "Sourcey/Logger.h"
#include "Sourcey/Timeout.h"
#include "Sourcey/Net/UDPSocket.h"
#include "Sourcey/Net/TCPSocket.h" //Client
#include "Sourcey/Media/FLVMetadataInjector.h"
#include "Sourcey/Media/FormatRegistry.h"
#include "Sourcey/Media/MediaFactory.h"
#include "Sourcey/Media/AudioCapture.h"
#include "Sourcey/Media/AVEncoder.h"


#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#include <assert.h>
#include <stdio.h>


/*
// Detect Memory Leaks
#ifdef _DEBUG
#include "MemLeakDetect/MemLeakDetect.h"
#include "MemLeakDetect/MemLeakDetect.cpp"
CMemLeakDetect memLeakDetect;
#endif
*/


using namespace scy;
using namespace scy::av;
using namespace Poco;
using namespace std;
using namespace cv;


namespace scy {
namespace av {

	
//VideoCapture* videoCapture;
//AudioCapture* audioCapture;
//ofstream outputFile;


class Tests
{		
public:

	Tests() 
	{   
		traceL() << "Running tests..." << endl;	

		//videoCapture = NULL;
		//audioCapture = NULL;
		//
		// Set up our devices
		MediaFactory::initialize();
		MediaFactory::instance()->loadVideo();

		try
		{			
			//MediaFactory::instance()->devices().print(cout);
			//audioCapture = new AudioCapture(device.id, 2, 44100);
			
			//videoCapture = new VideoCapture(0, true);
			//audioCapture = new AudioCapture(0, 1, 16000);	
			//audioCapture = new AudioCapture(0, 1, 11025);			

			//for (int i = 0; i < 100; i++) { //0000000000
			//	runAudioCaptureTest();
			//}
			//runAudioCaptureThreadTest();
			//runCaptureEncoderTest();
			//runVideoCaptureThreadTest();
			//runAudioCaptureTest();
			//runOpenCVMJPEGTest();
			//runVideoRecorderTest();
			//runAudioRecorderTest();
			//runCaptureEncoderTest();
			//runAVEncoderTest();
			runStreamEncoderTest();
			//runMediaSocketTest();
			//runFormatFactoryTest();
			//runMotionDetectorTest();
			//runBuildXMLString();
		
			//runStreamEncoderTest();
			//runOpenCVCaptureTest();
			//runDirectShowCaptureTest();
		}
		catch (Poco::Exception& exc)
		{
			Log("error") << "Error: " << exc.displayText() << endl;
		}		

		MediaFactory::instance()->unloadVideo();
		MediaFactory::uninitialize();

		//if (videoCapture)
		//	delete audioCapture;
		//if (videoCapture)
		//	delete videoCapture;		

		traceL() << "Tests Exiting..." << endl;	
	};
	
		
	// ---------------------------------------------------------------------
	// Packet Stream Encoder Test
	//
	class StreamThread: public Poco::Runnable, public PacketStream
	{
	public:
		StreamThread() : 
			_stop(false), _frameNumber(0)
		{		
			_thread.start(*this);
		}		
		
		virtual ~StreamThread()
		{		
			traceL() << "[StreamThread] Destroying" << endl;
			stop();
			traceL() << "[StreamThread] Destroying: OK" << endl;
		}

		void stop()
		{
			_stop = true;
			_thread.join();
		}

		void run()
		{
			try
			{
				av::RecorderOptions options;
				
				//options.oformat = Format("MJPEG High", "mjpeg", VideoCodec("mjpeg", "mjpeg", 640, 480, 10));		
				//options.oformat.video.pixelFmt = "yuvj420p";
				//options.oformat.video.quality = 100;

				options.oformat = Format("MP4", "mp4", 
					VideoCodec("MPEG4", "mpeg4", 400, 300, 25), 
					//VideoCodec("H264", "libx264", 320, 240, 25),
					//AudioCodec("AAC", "aac")
					//AudioCodec("AAC", "aac", 2, 44100)
					//AudioCodec("MP3", "libmp3lame", 1, 8000, 64000),
					//AudioCodec("MP3", "libmp3lame", 2, 44100, 64000)
					AudioCodec("AC3", "ac3_fixed", 2, 44100, 64000)
				);

				//options.oformat = Format("MP4", "mpeg4",
					//VideoCodec("MPEG4", "mpeg4", 640, 480, 25), 
					//VideoCodec("H264", "libx264", 320, 240, 25),
					//AudioCodec("AC3", "ac3_fixed", 2, 44100));
			
				//options.oformat  = Format("MP3", "mp3", 
				//	VideoCodec(), 
				//	AudioCodec("MP3", "libmp3lame", 1, 8000, 64000)); //)

				//options.oformat = Format("MP4", Format::MP4, VideoCodec(Codec::MPEG4, "MPEG4", 1024, 768, 25));
				//options.oformat = Format("MJPEG", "mjpeg", VideoCodec(Codec::MJPEG, "MJPEG", 1024, 768, 25));
				//options.oformat = Format("FLV", "flv", VideoCodec(Codec::H264, "H264", 400, 300, 25));	
				//options.oformat = Format("FLV", "flv", 
				//	VideoCodec(Codec::FLV, "FLV", 400, 300, 15)//,
				//	//AudioCodec(Codec::NellyMoser, "NellyMoser", 1, 11025)
				//	);
				options.ofile = "av_capture_test.mp4";
				
				// init capture
				VideoCapture* videoCapture = NULL;
				AudioCapture* audioCapture = NULL;
				if (options.oformat.video.enabled) {
					videoCapture = MediaFactory::instance()->getVideoCapture(0);
					AllocateOpenCVInputFormat(videoCapture, options.iformat);	
					attach(videoCapture, false);
				}
				if (options.oformat.audio.enabled) {
					Device device;
					if (MediaFactory::instance()->devices().getDefaultAudioInputDevice(device)) {
						audioCapture = MediaFactory::instance()->createAudioCapture(device.id,
							options.oformat.audio.channels, 
							options.oformat.audio.sampleRate);
						attach(audioCapture, false); //true
					}
				}				
				
				// init proccessors
				//options.iformat.video.pixfmt = (scy::av::PixelFormat::ID); 
				//MotionDetector* detector = new MotionDetector();
				//detector->setVideoCapture(videoCapture);
				//attach(detector, 1, true);		
				//MultipartPacketizer* multi = new MultipartPacketizer("image/jpeg", false, true);
				//attach(multi, 1, true);	
		
				// init encoder				
				AVEncoder* encoder = new AVEncoder(options);
				encoder->initialize();
				attach(encoder, 5, true);

				// start the stream
				PacketStream::start();
				PacketStream::attach(packetDelegate(this, &StreamThread::onVideoEncoded));	

				while (!_stop)
				{
					Thread::sleep(50);
				}				

				PacketStream::detach(packetDelegate(this, &StreamThread::onVideoEncoded));					
				PacketStream::stop();
				
				traceL() << "[StreamThread] Ending.................." << endl;
			}
			catch (Poco::Exception& exc)
			{
				traceL() << "StreamThread: " << exc.displayText() << endl;
			}
			
			traceL() << "[StreamThread] Ended.................." << endl;
			//delete this;
		}

		void onVideoEncoded(void* sender, RawPacket& packet)
		{
			traceL() << "###################### StreamThread: On Packet: " << packet.size() << endl;
			_frameNumber++;
			//if (_frameNumber > 10)
			//	stop();
		}
		
		Poco::Thread	_thread;
		int				_frameNumber;
		bool			_stop;
	};


	void runStreamEncoderTest()
	{
		traceL() << "Stream Encoder Test: Starting" << endl;	
				
		// start and destroy multiple receivers
		list<StreamThread*> threads;
		for (unsigned i = 0; i < 1; i++) { //0000000000
			//Log("debug") << "StreamThread: " << _name << endl;
			threads.push_back(new StreamThread);
			//StreamThread test;
			//UDPSocket sock;
		}
		util::pause();
		traceL() << "Stream Encoder Test: Cleanup" << endl;	
		util::ClearList(threads);
		traceL() << "Stream Encoder Test: Ended" << endl;	

	}
	

	// ---------------------------------------------------------------------
	// OpenCV Capture Test
	//	
	void runOpenCVCaptureTest()
	{
		traceL() << "Stream Encoder Test: Starting" << endl;	

		cv::VideoCapture cap(0); // open the default camera
		traceL() << "Stream Encoder Test: 1" << endl;	
		if (!cap.isOpened())  // check if we succeeded
			assert(false);
		
		traceL() << "Stream Encoder Test: 2" << endl;	

		cv::Mat edges;
		cv::namedWindow("edges",1);
		for(;;)
		{
			cv::Mat frame;
			cap >> frame; // get a new frame from camera
			cv::cvtColor(frame, edges, CV_BGR2GRAY);
			cv::GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
			cv::Canny(edges, edges, 0, 30, 3);
			cv::imshow("edges", edges);
			if(cv::waitKey(30) >= 0) break;
		}

		// the camera will be deinitialized automatically in VideoCapture destructor
		traceL() << "Stream Encoder Test: Complete" << endl;	
	}
	

	// ---------------------------------------------------------------------
	// Video Capture Thread Test
	//	
	class VideoCaptureThread: public Poco::Runnable
	{
	public:
		VideoCaptureThread(int deviceID, const string& name = "Capture Thread") : 
			_stop(false),
			_deviceID(deviceID),
			_frameNumber(0)
		{	
			_thread.setName(name);
			_thread.start(*this);
		}		

		VideoCaptureThread(const string& filename, const string& name = "Capture Thread") : 
			_stop(false),
			_filename(filename),
			_deviceID(0),
			_frameNumber(0)
		{	
			_thread.setName(name);
			_thread.start(*this);
		}	
		
		~VideoCaptureThread()
		{
			_stop = true;
			_thread.join();
		}

		void run()
		{
			try
			{				
				VideoCapture* capture = !_filename.empty() ?
					MediaFactory::instance()->createVideoCapture(_filename) : //, true
					MediaFactory::instance()->getVideoCapture(_deviceID);
				capture->attach(packetDelegate(this, &VideoCaptureThread::onVideo));	
				capture->start();

				while (!_stop)
				{
					cv::waitKey(50);
				}
				
				capture->detach(packetDelegate(this, &VideoCaptureThread::onVideo));	

				traceL() << "[VideoCaptureThread] Ending.................." << endl;
			}
			catch (Poco::Exception& exc)
			{
				Log("error") << "[VideoCaptureThread] Error: " << exc.displayText() << endl;
			}
			
			traceL() << "[VideoCaptureThread] Ended.................." << endl;
			//delete this;
		}

		void onVideo(void* sender, MatPacket& packet)
		{
			traceL() << "VideoCaptureThread: On Packet: " << packet.size() << endl;
			//traceL() << "VideoCaptureThread: On Packet 1: " << packet.mat->total() << endl;
			//traceL() << "VideoCaptureThread: On Packet 1: " << packet.mat->cols << endl;
			//assert(packet.mat->cols);
			//assert(packet.mat->rows);
			cv::imshow(_thread.name(), *packet.mat);
			_frameNumber++;
		}
		
		Poco::Thread	_thread;
		std::string		_filename;
		int				_deviceID;
		int				_frameNumber;
		bool			_stop;
	};


	void runVideoCaptureThreadTest()
	{
		traceL() << "Running video capture test..." << endl;	
				
		// start and destroy multiple receivers
		list<VideoCaptureThread*> threads;
		for (int i = 0; i < 3; i++) { //0000000000			
			threads.push_back(new VideoCaptureThread(0, Poco::format("Capture Thread %d", i)));
			//threads.push_back(new VideoCaptureThread("D:\\dev\\lib\\ffmpeg-1.0.1-gpl\\bin\\output.avi", Poco::format("Capture Thread %d", i)));
		}

		util::pause();

		util::ClearList(threads);
	}
	
	
	// ---------------------------------------------------------------------
	//
	// Audio Capture Thread Test
	//
	// ---------------------------------------------------------------------	
	class AudioCaptureThread: public Poco::Runnable
	{
	public:
		AudioCaptureThread(const string& name = "Capture Thread")
		{	
			_thread.setName(name);
			_thread.start(*this);
		}		
		
		~AudioCaptureThread()
		{
			_wakeUp.set();
			_thread.join();
		}

		void run()
		{
			try
			{
				//capture = new AudioCapture(0, 2, 44100);
				//capture = new AudioCapture(0, 1, 16000);	
				//capture = new AudioCapture(0, 1, 11025);	
				AudioCapture* capture = new AudioCapture(0, 1, 16000);
				capture->attach(audioDelegate(this, &AudioCaptureThread::onAudio));	
				
				_wakeUp.wait();
				
				capture->detach(audioDelegate(this, &AudioCaptureThread::onAudio));	
				delete capture;
				
				traceL() << "[AudioCaptureThread] Ending.................." << endl;
			}
			catch (Poco::Exception& exc)
			{
				Log("error") << "[AudioCaptureThread] Error: " << exc.displayText() << endl;
			}
			
			traceL() << "[AudioCaptureThread] Ended.................." << endl;
			//delete this;
		}

		void onAudio(void* sender, AudioPacket& packet)
		{
			traceL() << "[AudioCaptureThread] On Packet: " << packet.size() << endl;
			//cv::imshow(_thread.name(), *packet.mat);
		}
		
		Poco::Thread	_thread;
		Poco::Event		_wakeUp;
		int				_frameNumber;
	};


	void runAudioCaptureThreadTest()
	{
		traceL() << "Running Audio Capture Thread test..." << endl;	
				
		// start and destroy multiple receivers
		list<AudioCaptureThread*> threads;
		for (int i = 0; i < 10; i++) { //0000000000
			threads.push_back(new AudioCaptureThread(Poco::format("Audio Capture Thread %d", i)));
		}

		util::pause();

		util::ClearList(threads);
	}

	
	// ---------------------------------------------------------------------
	//
	// Audio Capture Test
	//
	// ---------------------------------------------------------------------	
	void onCaptureTestAudioCapture(void*, AudioPacket& packet)
	{
		traceL() << "onCaptureTestAudioCapture: " << packet.size() << endl;
		//cv::imshow("Target", *packet.mat);
	}	

	void runAudioCaptureTest()
	{
		traceL() << "Running Audio Capture test..." << endl;	
		
		AudioCapture* capture = new AudioCapture(0, 1, 16000);
		capture->attach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		capture->detach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		capture->attach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		capture->detach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		capture->attach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		//Util::pause();
		capture->detach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		delete capture;

		/*
		AudioCapture* capture = new AudioCapture(0, 1, 16000);
		capture->attach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));	
		//audioCapture->start();
		
		//Util::pause();

		capture->detach(packetDelegate(this, &Tests::onCaptureTestAudioCapture));
		delete capture;
		//audioCapture->stop();
		*/
	}
	
	
	/*
	// ---------------------------------------------------------------------
	//
	// Video Media Socket Test
	//
	// ---------------------------------------------------------------------	

	class MediaConnection: public TCPServerConnection
	{
	public:
		MediaConnection(const StreamSocket& s) : 
		  TCPServerConnection(s), stop(false)//, lastTimestamp(0), timestampGap(0), waitForKeyFrame(true)
		{		
		}
		
		void run()
		{
			try
			{
				av::RecorderOptions options;
				//options.ofile = "enctest.mp4";
				//options.stopAt = time(0) + 3;
				AllocateOpenCVInputFormat(videoCapture, options.iformat);	
				//options.oformat = Format("MJPEG", "mjpeg", VideoCodec(Codec::MJPEG, "MJPEG", 1024, 768, 25));
				//options.oformat = Format("FLV", "flv", VideoCodec(Codec::H264, "H264", 400, 300, 25));	
				options.oformat = Format("FLV", "flv", VideoCodec(Codec::FLV, "FLV", 640, 480, 100));	
				//options.oformat = Format("FLV", "flv", VideoCodec(Codec::FLV, "FLV", 320, 240, 15));	
				//options.oformat = Format("FLV", "flv", VideoCodec(Codec::H264, "H264", 400, 300, 25));		
					

				//options.iformat.video.pixfmt = (scy::av::PixelFormat::ID)PIX_FMT_GRAY8; //PIX_FMT_BGR8; //PIX_FMT_BGR32 // PIX_FMT_BGR32
				//MotionDetector* detector = new MotionDetector();
				//detector->setVideoCapture(videoCapture);
				//stream.attach(detector, true);		
				//stream.attach(new SurveillanceMJPEGPacketizer(*detector), 20, true);	

				stream.attach(videoCapture, false);

				stream.attach(packetDelegate(this, &MediaConnection::onVideoEncoded));
		
				// init encoder				
				AVEncoder* encoder = new AVEncoder();
				encoder->setParams(options);
				encoder->initialize();
				stream.attach(encoder, 5, true);				
				
				//HTTPMultipartPacketizer* packetizer = new HTTPMultipartPacketizer("image/jpeg");
				//stream.attach(packetizer);

				//FLVMetadataInjector* injector = new FLVMetadataInjector(options.oformat);
				//stream.attach(injector);

				// start the stream
				stream.start();

				while (!stop)
				{
					Thread::sleep(50);
				}
				
				//stream.detach(packetDelegate(this, &MediaConnection::onVideoEncoded));
				//stream.stop();

				//outputFile.close();
				cerr << "MediaConnection: ENDING!!!!!!!!!!!!!" << endl;
			}
			catch (Poco::Exception& exc)
			{
				cerr << "MediaConnection: " << exc.displayText() << endl;
			}
		}

		void onVideoEncoded(void* sender, RawPacket& packet)
		{
			StreamSocket& ss = socket();

			fpsCounter.tick();
			traceL() << "On Video Packet Encoded: " << fpsCounter.fps << endl;

			//if (fpsCounter.frames < 10)
			//	return;
			//if (fpsCounter.frames == 10) {
			//	stream.reset();
			//	return;
			//}

			try
			{		
				ss.sendBytes(packet.data, packet.size);
			}
			catch (Poco::Exception& exc)
			{
				cerr << "MediaConnection: " << exc.displayText() << endl;
				stop = true;
			}
		}
		
		bool stop;
		PacketStream stream;
		FPSCounter fpsCounter;
	};

	void runMediaSocketTest()
	{		
		traceL() << "Running Media Socket Test" << endl;
		
		ServerSocket svs(666);
		TCPServer srv(new TCPServerConnectionFactoryImpl<MediaConnection>(), svs);
		srv.start();
		util::pause();
	}

	
	// ---------------------------------------------------------------------
	//
	// Video CaptureEncoder Test
	//
	// ---------------------------------------------------------------------
	//UDPSocket outputSocket;

	void runCaptureEncoderTest()
	{		
		traceL() << "Running Capture Encoder Test" << endl;	

		av::RecorderOptions options;
		options.ofile = "enctest.mp4";
		//options.stopAt = time(0) + 3;
		AllocateOpenCVInputFormat(videoCapture, options.iformat);	
		//options.oformat = Format("MJPEG", "mjpeg", VideoCodec(Codec::MJPEG, "MJPEG", 400, 300));
		options.oformat = Format("FLV", "flv", VideoCodec(Codec::H264, "H264", 400, 300, 25));	
		//options.oformat = Format("FLV", "flv", VideoCodec(Codec::FLV, "FLV", 320, 240, 15));	
				
		//CaptureEncoder<VideoEncoder> enc(videoCapture, options);	
		//encoder = new AVEncoder(stream.options());
		CaptureRecorder enc(options, videoCapture, audioCapture);		
		//enc.initialize();	
		
		enc.attach(packetDelegate(this, &Tests::onCaptureEncoderTestVideoEncoded));
		enc.start();
		util::pause();
		enc.stop();

		traceL() << "Running Capture Encoder Test: END" << endl;	
	}
	
	FPSCounter fpsCounter;
	void onCaptureEncoderTestVideoEncoded(void* sender, MediaPacket& packet)
	{
		fpsCounter.tick();
		traceL() << "On Video Packet Encoded: " << fpsCounter.fps << endl;
	}

	// ---------------------------------------------------------------------
	//
	// Video CaptureRecorder Test
	//
	// ---------------------------------------------------------------------
	void runVideoRecorderTest()
	{
		av::RecorderOptions options;
		options.ofile = "av_capture_test.flv";
		
		options.oformat = Format("FLV", "flv", 
			VideoCodec(Codec::FLV, "FLV", 320, 240, 15),
			//AudioCodec(Codec::NellyMoser, "NellyMoser", 1, 11025),
			AudioCodec(Codec::Speex, "Speex", 1, 16000)//,
			//AudioCodec(Codec::Speex, "Speex", 2, 44100)
			);	
		//options.oformat = Format("MP4", Format::MP4,
		options.oformat = Format("FLV", "flv",
			//VideoCodec(Codec::MPEG4, "MPEG4", 640, 480, 25), 
			//VideoCodec(Codec::H264, "H264", 640, 480, 25), 
			VideoCodec(Codec::FLV, "FLV", 640, 480, 25), 
			//AudioCodec(Codec::NellyMoser, "NellyMoser", 1, 11025)
			//AudioCodec(Codec::Speex, "Speex", 2, 44100)
			//AudioCodec(Codec::MP3, "MP3", 2, 44100)
			//AudioCodec(Codec::AAC, "AAC", 2, 44100)
			AudioCodec(Codec::AAC, "AAC", 1, 11025)
		);

		options.oformat = Format("M4A", Format::M4A,
			//AudioCodec(Codec::NellyMoser, "NellyMoser", 1, 44100)
			AudioCodec(Codec::AAC, "AAC", 2, 44100)
			//AudioCodec(Codec::AC3, "AC3", 2, 44100)
		);



		//options.stopAt = time(0) + 5; // Max 24 hours		
		AllocateOpenCVInputFormat(videoCapture, options.iformat);	

		CaptureRecorder enc(options, NULL, audioCapture); //videoCapture
			
		audioCapture->start();	
		enc.start();
		util::pause();
		enc.stop();
	}

	// ---------------------------------------------------------------------
	//
	// Audio CaptureRecorder Test
	//
	// ---------------------------------------------------------------------
	void runAudioRecorderTest()
	{
		av::RecorderOptions options;
		options.ofile = "audio_test.mp3";
		options.stopAt = time(0) + 5;
		options.oformat = Format("MP3", "mp3",
			AudioCodec(Codec::MP3, "MP3", 2, 44100)
		);

		CaptureRecorder enc(options, NULL, audioCapture);
		
		audioCapture->start();	
		enc.start();
		util::pause();
		enc.stop();
	}
			
	*/

};


} // namespace av
} // namespace scy


int main(int argc, char** argv)
{
	Logger::instance().add(new ConsoleChannel("debug", TraceLevel));
	
	// Set up our devices
	//MediaFactory::initialize();
	//MediaFactory::instance()->loadVideo();
	MediaFactory::instance()->loadVideo();	
	{
		Tests run;	
		util::pause();
	}		
	MediaFactory::instance()->unloadVideo();
	//MediaFactory::instance()->unloadAudio();
	//MediaFactory::uninitialize();
		
	Logger::uninitialize();
	return 0;
}
/*getDefaultInputDevice
int record( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  if ( status )
    std::cout << "Stream overflow detected!" << std::endl;

  // Do something with the data in the "inputBuffer" buffer.

  return 0;
}

int main()
{
  RtAudio adc;
  if ( adc.getDeviceCount() < 1 ) {
    std::cout << "\nNo audio devices found!\n";
    exit( 0 );
  }

  RtAudio::StreamParameters parameters;
  parameters.deviceId = 5; //adc.getDefaultInputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  unsigned int sampleRate = 44100;
  unsigned int bufferFrames = 256; // 256 sample frames
  
  // Determine the number of devices available
  //unsigned int devices = audio.getDeviceCount();

  // Scan through devices for various capabilities
  RtAudio::DeviceInfo info = adc.getDeviceInfo( parameters.deviceId );

    if ( info.probed == true ) {
      // Print, for example, the maximum number of output channels for each device
      std::cout << "device = " << info.name << "\n";
    }

  util::pause();

  try {
    adc.openStream( NULL, &parameters, RTAUDIO_SINT16,
                    sampleRate, &bufferFrames, &record );
    adc.startStream();
  }
  catch ( RtError& e ) {
    e.printMessage();
    //exit( 0 );
  }
  
  char input;
  std::cout << "\nRecording ... press <enter> to quit.\n";
  std::cin.get( input );

  try {
    // Stop the stream
    adc.stopStream();
  }
  catch (RtError& e) {
    e.printMessage();
  }

  if ( adc.isStreamOpen() ) adc.closeStream();

  return 0;
}
*/



/*
#include <stdio.h>
#include <iostream>
#include <dshow.h>

int
main()
{
 // for playing
 IGraphBuilder *pGraphBuilder;
 ICaptureGraphBuilder2 *pCaptureGraphBuilder2;
 IMediaControl *pMediaControl;
 IBaseFilter *pDeviceFilter = NULL;

 // to select a video input device
 ICreateDevEnum *pCreateDevEnum = NULL;
 IEnumMoniker *pEnumMoniker = NULL;
 IMoniker *pMoniker = NULL;
 ULONG nFetched = 0;

 // initialize COM
 CoInitialize(NULL);

 //
 // selecting a device
 //

 // Create CreateDevEnum to list device
 CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, 
   IID_ICreateDevEnum, (PVOID *)&pCreateDevEnum);
    
 // Create EnumMoniker to list VideoInputDevice 
 pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
   &pEnumMoniker, 0);
 if (pEnumMoniker == NULL) {
   // this will be shown if there is no capture device
   printf("no device\n");
   return 0;
 }

 // reset EnumMoniker
 pEnumMoniker->Reset();

 // get each Moniker
 while (pEnumMoniker->Next(1, &pMoniker, &nFetched) == S_OK) {
   IPropertyBag *pPropertyBag;
   TCHAR devname[256];
   //LPSTR devname;

   // bind to IPropertyBag
   pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
     (void **)&pPropertyBag);

   VARIANT var;

   // get FriendlyName
   var.vt = VT_BSTR;
   pPropertyBag->Read(L"FriendlyName", &var, 0);
   WideCharToMultiByte(CP_ACP, 0,
     var.bstrVal, -1, (LPSTR)devname, sizeof(devname), 0, 0);
   VariantClear(&var);

   printf("%s\r\n", devname);
   printf("  select this device ? [y] or [n]\r\n");
   int ch = getchar();

   // you can start playing by 'y' + return key
   // if you press the other key, it will not be played.
   if (ch == 'y') {
     // Bind Monkier to Filter
     pMoniker->BindToObject(0, 0, IID_IBaseFilter,
         (void**)&pDeviceFilter );
   }

   // release
   pMoniker->Release();
   pPropertyBag->Release();

   if (pDeviceFilter != NULL) {
     // go out of loop if getchar() returns 'y'
     break;
   }
 }

 if (pDeviceFilter != NULL) {
   //
   // PLAY
   //

   // create FilterGraph
   CoCreateInstance(CLSID_FilterGraph,
     NULL,
     CLSCTX_INPROC,
     IID_IGraphBuilder,
     (LPVOID *)&pGraphBuilder);

   // create CaptureGraphBuilder2
   CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
     IID_ICaptureGraphBuilder2, 
     (LPVOID *)&pCaptureGraphBuilder2);
   
   // set FilterGraph
   pCaptureGraphBuilder2->SetFiltergraph(pGraphBuilder);
   
   // get MediaControl interface
   pGraphBuilder->QueryInterface(IID_IMediaControl,
     (LPVOID *)&pMediaControl);
   
   // add device filter to FilterGraph
   pGraphBuilder->AddFilter(pDeviceFilter, L"Device Filter");
   
   // create Graph
   pCaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW,
     NULL, pDeviceFilter, NULL, NULL);

   // start playing
   pMediaControl->Run();

   // to block execution
   // without this messagebox, the graph will be stopped immediately
   MessageBox(NULL,
     "Block Execution",
     "Block",
     MB_OK);

   // release
   pMediaControl->Release();
   pCaptureGraphBuilder2->Release();
   pGraphBuilder->Release();
 }

 // release
 pEnumMoniker->Release();
 pCreateDevEnum->Release();

 // finalize COM
 CoUninitialize();

 return 0;
}
	 */






/*
#include "vendors/RtAudio/RtAudio.h"

int record( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  if ( status )
    traceL() << "Stream overflow detected!" << endl;
  
    traceL() << "inputBuffer: " << inputBuffer << endl;
  // Do something with the data in the "inputBuffer" buffer.

  return 0;
}

int main()
{
  RtAudio adc;
  if ( adc.getDeviceCount() < 1 ) {
    traceL() << "\nNo audio devices found!\n";
    //exit( 0 );
  }
  
    traceL() << "\nAudio device: " << adc.getDefaultInputDevice();
  RtAudio::StreamParameters parameters;

  parameters.deviceId = adc.getDefaultInputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  unsigned int sampleRate = 44100;
  unsigned int bufferFrames = 256; // 256 sample frames

  try {
    adc.openStream( NULL, &parameters, RTAUDIO_SINT16,
                    sampleRate, &bufferFrames, &record );
    adc.startStream();
  }
  catch ( RtError& e ) {
	traceL() << e.getMessage() << endl;
    e.printMessage();
    //exit( 0 );
  }
  
  char input;
  traceL() << "\nRecording ... press <enter> to quit.\n";
  cin.get( input );

  try {
    // Stop the stream
    adc.stopStream();
  }
  catch (RtError& e) {
    e.printMessage();
  }

  if ( adc.isStreamOpen() ) adc.closeStream();
  
	util::pause();
  return 0;
}
*/