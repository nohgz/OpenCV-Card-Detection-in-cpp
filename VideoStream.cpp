#include "VideoStream.h"


// 8-14-2024 Initial Commit
VideoStream::VideoStream(cv::Size resolution, int framerate, bool isUSB, int src)
    : m_resolution(resolution), m_framerate(framerate), m_isUSB(isUSB), m_src(src), m_stopped(false)
{
    // Initialize the camera and image stream
    this->m_stream = cv::VideoCapture(m_src);

    if (!this->m_stream.isOpened())
    {
        std::cerr << "ERROR: Could not open camera!" << std::endl;
        this->m_stopped = true;
    }

    this->m_stream.set(3, resolution.width);
    this->m_stream.set(4, resolution.height);

    // Read the first frame from the stream
    this->m_stream.read(this->m_frame);
}

void VideoStream::start()
{
    std::thread(&VideoStream::update, this).detach();
}

void VideoStream::update()
{
    // Loop infinitely until the thread is stopped
    while(!this->m_stopped)
    {
       this->m_stream.read(this->m_frame);
    }
    // When thread is stopped, close camera resources and stop
    this->m_stream.release();
}

cv::Mat VideoStream::read()
{
    // return the most recent frame
    return m_frame;
}

void VideoStream::stop()
{
    // indicate that the camera and thread should be stopped
    this->m_stopped = true;
}