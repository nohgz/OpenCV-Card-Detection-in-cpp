#include "cards.h"
#include "constants.h"
#include "rankSuitIsolator.h"


// 8-14-2024 Initial Commit
// TODO: FIX THE WHOLE FUCKING PROCESSS ITS SO ANNOYING
void startRankSuitIsolator(std::string imagePath)
{
    // Initialize a USB Camera
    cv::VideoCapture cam(0);

    // Initialize counter variable to switch from isolating rank to isolating suit
    int i = 1;

    // Initialize lists of suits and ranks for iterating
    std::string types[] = {"Ace", "Two", "Three", "Four", "Five", "Six", "Seven",
         "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Spades", "Diamonds", "Clubs", "Hearts"};
    std::string filename;
    cv::Mat imageIn, imageOut, imageWithText, cardPicture, cardPicture_preproc, warp;
    int key;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Point> cardContour, approx;
    std::vector<cv::Point2f> pts;
    cv::Rect bounding_rectangle;
    queryCard qCard;

    for (std::string type : types)
    {
        filename = type + ".jpg";
        std::cout << "Please press \"p\" to take a picture of " << filename << std::endl;

        while(true) {
            cam.read(imageIn);

            // Clone the input picture for drawing such that it won't get contaminated by text
            imageWithText = imageIn.clone();
            cv::putText(imageWithText, "Please press \"p\" to take a picture of " + type, cv::Point(50, 100), FONT, 0.5, cv::Scalar(255,155,255), 1, cv::LINE_AA);
            cv::imshow("Card", imageWithText);

            key = cv::waitKey(1);
            // if the key is 'p', this number is the decimal ASCII
            // representation of the keystroke
            if (key == 112)
            {
                cardPicture = imageIn;
                break;
            }
        }

        cardPicture_preproc = preprocessImage(cardPicture);

        // Find contours and sort them by size
        cv::findContours(cardPicture_preproc, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
        std::sort(contours.begin(), contours.end(),
          [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
              return cv::contourArea(a) > cv::contourArea(b);
          });

        // Assume the largest contour is the card. If there are no contours, error and quit.
        if (contours.empty()) { std::cerr << "No contours found!" << std::endl; return; }
        cardContour = contours[0];

        qCard = preprocessCard(cardContour, imageIn);

        if (i <= 13) // Isolate rank
        {
            imageOut = qCard.rank_image;
        }
        else if (i >= 13) // Isolate Suit
        {
            imageOut = qCard.suit_image;
        }

        std::cout << "Press \"c\" to continue." << std::endl;
        while (true)
        {
            cv::imshow("Final Rank/Suit Image", imageOut);
            key = cv::waitKey(1);
            if (key == 99) // if the key is 'c' (99 = c on ascii table)
            {
                cv::imwrite(imagePath + filename, imageOut);
                break;
            }
        }

        i++;
    }
}