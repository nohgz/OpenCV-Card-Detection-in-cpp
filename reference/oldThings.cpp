#include <opencv2/opencv.hpp>
#include <iostream>

/* IMAGES */
/*
int main(int, char**)
{
    //std::string imagePath = "resources/images/thing.png";
    std::string imagePath = "resources/images/beep.png";
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    cv::imshow("Image", image);
    cv::waitKey(0);
    return 0;
}
*/

/* VIDEOS */
/*
int main (int, char**)
{
    std::string videoPath = "resources/videos/thingsplosion.mov";
    cv::VideoCapture cap(videoPath);
    cv::Mat imageIn;

    while (true)
    {
        cap.read(imageIn);

        cv::imshow("video", imageIn);

        cv::waitKey(30);
    }
}
*/

/* BLURS AND FUNCTIONS */
/*
int main(int, char**)
{
    std::string imagePath = "resources/images/cards.png";
    cv::Mat imageIn = cv::imread(imagePath);
    cv::Mat imageBlurred, imageGrayscale, imageCanny, imageDilated, imageEroded;

    cv::cvtColor(imageIn, imageGrayscale, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(imageIn, imageBlurred, cv::Size(3,3), 3, 0);
    cv::Canny(imageBlurred, imageCanny, 50, 150);

    // creates kernel for dilation, increase size for more dilation
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2,2));
    cv::dilate(imageCanny, imageDilated, kernel);
    cv::erode(imageDilated, imageEroded, kernel);

    cv::imshow("Source", imageIn);
    cv::imshow("Image Blurred",   imageBlurred);
    cv::imshow("Image Grayscale", imageGrayscale);
    cv::imshow("Image Canny",     imageCanny);
    cv::imshow("Image Dilation",  imageDilated);
    cv::imshow("Image Eroded",    imageEroded);

    cv::waitKey(0);
}
*/

/* Resizing and Cropping */
/*
int main(int, char**)
{
    std::string imagePath = "resources/images/cards.png";
    cv::Mat imageIn = cv::imread(imagePath);
    cv::Mat imageResized, imageCropped;

    cv::resize(imageIn, imageResized, cv::Size(), 0.5, 0.5);

    cv::Rect roi(100, 100, 300, 250);
    imageCropped = imageIn(roi);

    cv::imshow("Source", imageIn);
    cv::imshow("Resized", imageResized);
    cv::imshow("Cropped", imageCropped);

    cv::waitKey(0);
}
*/

/* Drawing Shapes and Stuff */
/*
int main(int, char**)
{
    cv::Mat imageIn(512, 512, CV_8UC3, cv::Scalar(255, 255, 255));

    cv::circle(imageIn, cv::Point(256, 256), 128, cv::Scalar(0, 69, 255), 10);

    cv::imshow("Image", imageIn);

    cv::waitKey(0);
}
*/

/* birds eye view of cards */
/*
int main(int, char**)
{
    float width = 250.0f, height = 350.0f;
    cv::Mat matrix, imageWarp;

    std::string imagePath = "resources/images/cards.png";
    cv::Mat imageIn = cv::imread(imagePath);

    cv::Point2f source[4] = {{702, 270}, {896, 213}, {763, 509}, {940, 459}};
    cv::Point2f destination[4] = {{0.0f, 0.0f}, {width, 0.0f}, {0.0f, height}, {width, height}};

    matrix = cv::getPerspectiveTransform(source, destination);
    cv::warpPerspective(imageIn, imageWarp, matrix, cv::Size2f(width, height));

    cv::imshow("Source", imageIn);
    cv::imshow("Image Warped", imageWarp);
    cv::waitKey(0);
}
*/



/* Color Detection */
/*
int main(int, char**)
{

    std::string imagePath = "resources/images/beep.png";
    cv::Mat imageHSV, imageMask;
    int hmin = 0, smin = 110, vmin = 153;
    int hmax = 19, smax = 240, vmax = 255;

    cv::Mat imageIn = cv::imread(imagePath);
    cv::cvtColor(imageIn, imageHSV, cv::COLOR_BGR2HSV);


    cv::namedWindow("Trackbars", (640,200));
    cv::createTrackbar("Hue Min", "Trackbars", &hmin, 255);
    cv::createTrackbar("Hue Max", "Trackbars", &hmax, 255);
    cv::createTrackbar("Sat Min", "Trackbars", &smin, 255);
    cv::createTrackbar("Sat Max", "Trackbars", &smax, 255);
    cv::createTrackbar("Val Min", "Trackbars", &vmin, 255);
    cv::createTrackbar("Val Max", "Trackbars", &vmax, 255);

    while (true)
    {
        cv::Scalar lower(hmin, smin, vmin);
        cv::Scalar upper(hmax, smax, vmax);

        cv::inRange(imageHSV, lower, upper, imageMask);

        cv::imshow("Source", imageIn);
        cv::imshow("Image HSV", imageHSV);
        cv::imshow("Image Mask", imageMask);

        cv::waitKey(1);

    }
    return 0;
}
*/

/* Shape Detection */
void getContours(cv::Mat imageDilated, cv::Mat image)
{
    double area, perimeter;
    float aspectRatio;
    int numCorners;
    std::string objectType;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(imageDilated, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    // cv::drawContours(image, contours, -1, cv::Scalar(255, 0, 255), 2);

    std::vector<std::vector<cv::Point>> contourPoly(contours.size());
    std::vector<cv::Rect> boundRectangle(contours.size());

    for (int i = 0; i < contours.size(); i++)
    {
        area = cv::contourArea(contours[i]);
        std::cout << area << std::endl;

        if (area > 1000)
        {
            perimeter = cv::arcLength(contours[i], true);

            cv::approxPolyDP(contours[i], contourPoly[i], 0.02 * perimeter, true);
            // cv::drawContours(image, contourPoly, i, cv::Scalar(255, 0, 255), 2);

            boundRectangle[i] = cv::boundingRect(contourPoly[i]);
            cv::rectangle(image, boundRectangle[i], cv::Scalar(255, 255, 0), 2);
            aspectRatio = (float) boundRectangle[i].width / (float) boundRectangle[i].height;

            numCorners = (int) contourPoly[i].size();
            if (numCorners == 3) {
                objectType = "Triangle";
            } else if (numCorners == 4) {
                // defaults to rectangle, changes to square if the
                // aspect ratio is square like
                objectType = "Rectangle";
                if (aspectRatio > 0.95 && aspectRatio < 1.05 )
                    objectType = "Square";
            } else if (numCorners > 4 && numCorners < 9) {
                objectType = "Circle";
            } else if (numCorners > 9) {
                objectType = "Club";
            } else {
                objectType = "Undefined";
            }

            cv::putText(image, objectType, {boundRectangle[i].x, boundRectangle[i].y - 5}, cv::FONT_HERSHEY_DUPLEX, 0.75, cv::Scalar(0, 69, 255), 2);

        }

    }


}

int main(int, char**)
{
    std::string imagePath = "resources/images/shapes.png";
    cv::Mat imageIn = cv::imread(imagePath);
    cv::Mat imageBlurred, imageGrayscale, imageCanny, imageDilated, imageEroded;


    cv::cvtColor(imageIn, imageGrayscale, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(imageGrayscale, imageBlurred, cv::Size(3,3), 3, 0);
    cv::Canny(imageBlurred, imageCanny, 50, 150);

    // creates kernel for dilation, increase size for more dilation
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2,2));
    cv::dilate(imageCanny, imageDilated, kernel);

    getContours(imageDilated, imageIn);

    cv::imshow("Source", imageIn);
    // cv::imshow("Image Blurred",   imageBlurred);
    // cv::imshow("Image Grayscale", imageGrayscale);
    // cv::imshow("Image Canny",     imageCanny);
    // cv::imshow("Image Dilation",  imageDilated);

    cv::waitKey(0);
    return 0;
}