#include <iostream>
#include <opencv2/opencv.hpp>

#include "cards.h"
#include "rankSuitIsolator.h"
#include "constants.h"
#include "VideoStream.h"

// 8-14-2024 Initial Commit
int main(int, char**)
{
    std::vector<queryCard> cards;
    std::vector<int> contour_is_card;
    std::vector<std::vector<cv::Point>> contours_sorted, temp_contours;
    cv::Mat imageIn, outImage, pre_proc;
    double t1, t2, time1, freq;
    int k = 0, frame_rate_calc = 1, key, cameraID;
    freq = (int)cv::getTickFrequency();


    std::cout << "What camera would you like to use? -> ";
    std::cin >> cameraID;
    std::cout << "\n You chose camera #" << cameraID << std::endl;;

    // Initialize the camera object and video feed from the camera
    VideoStream videoStream{cv::Size(IM_WIDTH, IM_HEIGHT), FRAME_RATE, true, cameraID};
    videoStream.start();
    // Sleep for a second to let the camera warm up
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Load the train rank and suit images
    std::string trainImagePath = "resources/isolatorOutput/";
    std::vector<trainRank> train_ranks = loadRanks(trainImagePath);
    std::vector<trainSuit> train_suits = loadSuits(trainImagePath);

    std::cout << "Rank and suit train images loaded!" << std::endl;

    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Source", cv::WINDOW_AUTOSIZE);

    cv::Size rankImageSize, suitImageSize;

    while (true)
    {
        // Start timer for calculating frame rate
        t1 = cv::getTickCount();

        // Grab frame from video stream
        imageIn = videoStream.read();
        cv::imshow("Source", imageIn);

        //imageIn = cv::imread("resources/images/cards.png");

        // Preprocess the image (gray, blur and threshold)
        pre_proc = preprocessImage(imageIn);

        // Find and sort the contours of all cards in the image (query cards)
        std::tie(contour_is_card, contours_sorted) = findCards(pre_proc);

        // If there are no contours, continue to next iteration
        if (contours_sorted.empty())
        {
            continue;
        }

        // Clear the cards array to assign new card objects
        cards.clear();
        k = 0;

        // For each contour detected
        for (size_t i = 0; i < contours_sorted.size(); i++)
        {
            if (contour_is_card[i] == 1)
            {
                cards.push_back(preprocessCard(contours_sorted[i], imageIn));

                rankImageSize = cards[k].rank_image.size();
                suitImageSize = cards[k].suit_image.size();
                std::cout << "card #" << i << "rank image width" << rankImageSize.width << std::endl;
                std::cout << "card #" << i << "rank image height" << rankImageSize.height << std::endl;
                std::cout << "card #" << i << "suit image width" << suitImageSize.width << std::endl;
                std::cout << "card #" << i << "suit image height" << suitImageSize.height << std::endl;



                //std::tie(cards[k].best_rank_match, cards[k].best_suit_match, cards[k].rank_diff, cards[k].suit_diff) = matchCard(cards[k], train_ranks, train_suits);
                //imageIn = drawResults(imageIn, cards[k]);
                k++;
            }
        }

        // After processing the cards
        if (!cards.empty())
        {
            // Clear the temp_contours vector before adding new contours
            temp_contours.clear();

            // Collect the contours from the detected cards
            for (size_t i = 0; i < cards.size(); i++)
            {
                temp_contours.push_back(cards[i].contour);
            }

            // Draw the contours on the image
            cv::drawContours(imageIn, temp_contours, -1, cv::Scalar(255, 0, 0), 2);
        }

        // Draw framerate in the corner of the image
        cv::putText(imageIn, "FPS: " + std::to_string(frame_rate_calc), cv::Point(10, 26), FONT, 0.7, cv::Scalar(255, 0, 255), 2, cv::LINE_AA);

        // Display the image with identified cards
        cv::imshow("Display Image", imageIn);

        // Calculate framerate
        t2 = cv::getTickCount();
        time1 = (t2 - t1) / freq;
        frame_rate_calc = time1 > 0 ? 1 / time1 : 0;

        // Poll the keyboard. If 'q' is pressed, exit the main loop
        key = cv::waitKey(1);
        if (key == 'q') { break; }
    }

    // Close all windows and stop the video stream
    cv::destroyAllWindows();
    videoStream.stop();
    return 0;
}
