#pragma once
#ifndef VIDEOSTREAM_H
#define VIDEOSTREAM_H

// 8-14-2024 Initial Commit
#include <opencv2/opencv.hpp>

class VideoStream
{
public:
    VideoStream(cv::Size resolution, int framerate, bool isUSB, int src);
    void start();
    void update();
    cv::Mat read();
    void stop();

private:
    int m_framerate, m_src;
    bool m_isUSB, m_stopped;
    cv::Size m_resolution;
    cv::VideoCapture m_stream;
    cv::Mat m_frame;
};

#endif