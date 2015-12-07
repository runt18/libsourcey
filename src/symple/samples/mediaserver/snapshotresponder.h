#include "mediaserver.h"


namespace scy {


class SnapshotRequestHandler: public http::ServerResponder
{
public:
    SnapshotRequestHandler(http::ServerConnection& connection, const StreamingOptions& options) :
        http::ServerResponder(connection), options(options)
    {
    }

    virtual ~SnapshotRequestHandler()
    {
    }

    void onRequest(http::Request& request, http::Response& response)
    {
        DebugLS(this) << "Running" << std::endl;

        cv::Mat frame;
        options.videoCapture->getFrame(frame,
            options.oformat.video.width,
            options.oformat.video.height);

        std::vector<unsigned char> buffer;
        std::vector<int> param = std::vector<int>(2);
        param[0] = CV_IMWRITE_JPEG_QUALITY;
        param[1] = 95; // default(95) 0-100
        cv::imencode(".jpg", frame, buffer, param);

        DebugLS(this) << "Taking Snapshot Image: "
            << "\n\tWidth: " << frame.cols
            << "\n\tHeight: " << frame.rows
            << "\n\tCapture Width: " << options.videoCapture->width()
            << "\n\tCapture Height: " << options.videoCapture->height()
            << "\n\tType: " << frame.type()
            << "\n\tInput Size: " << frame.total()
            << "\n\tOutput Size: " << buffer.size()
            << std::endl;

        //unsigned char* data = new unsigned char[buffer.size()];
        //std::copy(buffer.begin(), buffer.end(), data);
        //connection().sendData((const char*)data, buffer.size());
        //delete data;

        connection().response().set("Access-Control-Allow-Origin", "*");
        //connection().sendData((const char*)&buffer[0], buffer.size());
        connection().socket()->send((const char*)&buffer[0], buffer.size());
        connection().close();
    }

    StreamingOptions options;
};


} // namespace scy
