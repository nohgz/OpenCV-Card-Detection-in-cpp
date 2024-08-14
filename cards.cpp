#include "cards.h"
#include <opencv2/opencv.hpp>
#include "constants.h"

// 8-14-2024 Initial Commit

// File name for use in loadRanks and loadSuits
std::string file;

/* -- Function Implementations -- */
std::vector<trainRank> v_trainRanks;
std::vector<trainRank> loadRanks(std::string filepath)
{
    /* Loads rank images from directory specified by filepath. Stores
    them in a list of Train_ranks objects */
    for (std::string Rank : CARD_RANKS)
    {
        trainRank rank_struct;
        rank_struct.rank_name = Rank;
        file = filepath + Rank + ".jpg";
        rank_struct.rank_image = cv::imread(file, cv::IMREAD_GRAYSCALE);
        v_trainRanks.push_back(rank_struct);
    }

    return v_trainRanks;
};

std::vector<trainSuit> v_trainSuits;
std::vector<trainSuit> loadSuits(std::string filepath)
{
    /* Loads suit images from directory specified by filepath. Stores
    them in a list of TrainSuit objects */
    for (std::string Suit : CARD_SUITS)
    {
        trainSuit suit_struct;
        suit_struct.suit_name = Suit;
        file = filepath + Suit + ".jpg";
        suit_struct.suit_image = cv::imread(file, cv::IMREAD_GRAYSCALE);
        v_trainSuits.push_back(suit_struct);
    }

    return v_trainSuits;
};


cv::Mat preprocessImage(cv::Mat image)
{
    cv::Mat imageGrayscale, imageBlurred, threshold;
    cv::Size imageSize;
    double bkg_level, thresh_level;
    
    cv::cvtColor(image, imageGrayscale, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(imageGrayscale, imageBlurred, cv::Size(3,3), 3, 0);

    // The best threshold level depends on the ambient lighting conditions.
    // For bright lighting, a high threshold must be used to isolate the cards
    // from the background. For dim lighting, a low threshold must be used.
    // To make the card detector independent of lighting conditions, the
    // following adaptive threshold method is used.

    // A background pixel in the center top of the image is sampled to determine
    // its intensity. The adaptive threshold is set at 50 (THRESH_ADDER) higher
    // than that. This allows the threshold to adapt to the lighting conditions.
    imageSize = imageGrayscale.size();
    bkg_level = imageGrayscale.at<uchar>(imageSize.height / 100, imageSize.width / 2);
    thresh_level = bkg_level + BKG_THRESH;

    cv::threshold(imageBlurred, threshold, thresh_level, 255, cv::THRESH_BINARY);

    return threshold;
};


std::tuple<std::vector<int>, std::vector<std::vector<cv::Point>>> findCards(cv::Mat thresh_image)
{
    /* Finds all card-sized contours in a thresholded camera image.
    Returns the number of cards, and a list of card contours sorted
    from largest to smallest. */

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    // Find contours
    cv::findContours(thresh_image, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // If there are no contours, return
    if (contours.empty()) return std::make_tuple(std::vector<int>(), std::vector<std::vector<cv::Point>>());

    // Otherwise, initialize the empty sorted contour and hierarchy lists
    std::vector<cv::Vec4i> hier_sort;
    std::vector<std::vector<cv::Point>> contour_sort;
    std::vector<int> if_contour_is_card(contours.size());
    std::vector<int> index_sort(contours.size());

    //  Fill empty lists with sorted contour and sorted hierarchy. Now,
    //  the indices of the contour list still correspond with those of
    //  the hierarchy list. The hierarchy array can be used to check if
    //  the contours have parents or not.
    for (int i = 0; i < contours.size(); i++) { index_sort[i] = i; };

    // stupid brainfuck thing that sorts by contour area
    std::sort(index_sort.begin(), index_sort.end(),
                [contours](int i, int j){
                    return cv::contourArea(contours[i]) > cv::contourArea(contours[j]);
                });

    // Fill empty lists with sorted contour and sorted hierarchy. Now,
    // the indices of the contour list still correspond with those of
    // the hierarchy list. The hierarchy array can be used to check if
    // the contours have parents or not.
    for (int i : index_sort)
    {
        contour_sort.push_back(contours[i]);
        hier_sort.push_back(hierarchy[i]);
    }

    // define variables for sorting order
    double contour_size, perimeter;
    std::vector<cv::Point> approx(contours.size());


    for (int i = 0; i < contour_sort.size(); i++)
    {
        contour_size = cv::contourArea(contour_sort[i]);
        perimeter = cv::arcLength(contour_sort[i], true);

        cv::approxPolyDP(contour_sort[i], approx, 0.01 * perimeter, true);

        if ((contour_size < CARD_MAX_AREA) && (contour_size > CARD_MIN_AREA)
            && (hier_sort[i][3] == -1 && approx.size() == 4))
        {
            if_contour_is_card[i] = 1;
        }
    }

    return std::make_tuple(if_contour_is_card, contour_sort);

};


queryCard preprocessCard(std::vector<cv::Point> contour, cv::Mat image)
{
    /* Uses contour to find information about the query card. Isolates rank
    and suit images from the card. */

    queryCard qCard;
    int width, height;
    float cent_x, cent_y;
    double perimeter, white_level, threshold_level;
    cv::Rect bounding_rectangle, roi;
    std::vector<std::vector<cv::Point>> rank_contours, suit_contours;
    std::vector<cv::Point> approx;
    std::vector<cv::Point2f> pts;
    cv::Mat qCorner, qCorner_zoom, query_threshold, qRank, qSuit, qRank_gray, qSuit_gray, qRank_roi, qSuit_roi, qRank_sized, qSuit_sized;

    // write the contour to the qCard object
    qCard.contour = contour;

    // find the perimeter of the card and use it to approximate the corner points
    perimeter = cv::arcLength(contour, true);
    cv::approxPolyDP(contour, approx, 0.01 * perimeter, true);
    for (const auto& point : approx)
    {
        pts.push_back(cv::Point2f(static_cast<float>(point.x), static_cast<float>(point.y)));
    }
    qCard.corner_points = pts;

    // find the width and height of the card's bounding rectangle
    bounding_rectangle = cv::boundingRect(contour);
    width = bounding_rectangle.width;
    height = bounding_rectangle.height;

    cent_x = 0, cent_y = 0;

    // Find the center point of the card by taking x and y average of the four corners
    for (const auto& point : pts)
    {
        cent_x += point.x;
        cent_y += point.y;
    }
    cent_x /= pts.size();
    cent_y /= pts.size();

    qCard.center_point = cv::Point2f(cent_x, cent_y);
    // warp the card into 200x300 flattened image using perspective transform
    qCard.warp = flattener(image, pts, width, height);

    // grab the corner of the warped card image and do a 4x zoom
    roi = cv::Rect(0, 0, CORNER_WIDTH, CORNER_HEIGHT);
    qCorner = qCard.warp(roi);
    cv::resize(qCorner, qCorner_zoom, cv::Size(0,0), 4.0f, 4.0f);

    // sample known white pixel intensity to determine good threshold level
    white_level = qCorner_zoom.at<uchar>(15, (int)((CORNER_WIDTH*4) / 2));
    threshold_level = white_level - CARD_THRESH;
    if (threshold_level <= 0 ) threshold_level = 1;
    cv::threshold(qCorner_zoom, query_threshold, threshold_level, 255, cv::THRESH_BINARY_INV);


    // split in to top and bottom half (top shows rank, bottom shows suit)
    roi = cv::Rect(0, 30, 128, 175); //(x y w h), python equiv -> [20:185, 0:128]
    qRank = query_threshold(roi);

    // grayscale the threshold because it cries if you dont
    cv::cvtColor(qRank, qRank_gray, cv::COLOR_BGR2GRAY);

    // Find rank contour and bounding rectangle, isolate and find largest contour
    cv::findContours(qRank_gray, rank_contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    std::sort(rank_contours.begin(), rank_contours.end(),
          [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
              return cv::contourArea(a) > cv::contourArea(b);
          });


    // Find bounding rectangle for largest contour, use it to resize query rank
    // image to match dimensions of the train rank image
    if (rank_contours.size() != 0)
    {
        bounding_rectangle = cv::boundingRect(rank_contours[0]);
        qRank_roi = qRank_gray(bounding_rectangle);
        cv::resize(qRank_roi, qRank_sized, cv::Size(RANK_WIDTH, RANK_HEIGHT), 0, 0);
        qCard.rank_image = qRank_sized;
    }

    roi = cv::Rect(0, 196, 128, 140);
    qSuit = query_threshold(roi);

    // grayscale the threshold because it cries if you dont
    cv::cvtColor(qSuit, qSuit_gray, cv::COLOR_BGR2GRAY);

    // Find suit contour and bounding rectangle, isolate and find largest contour
    cv::findContours(qSuit_gray, suit_contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    std::sort(suit_contours.begin(), suit_contours.end(),
          [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
              return cv::contourArea(a) > cv::contourArea(b);
          });

    // Find bounding rectangle for largest contour, use it to resize query rank
    // image to match dimensions of the train rank image
    if (suit_contours.size() != 0)
    {
        bounding_rectangle = cv::boundingRect(suit_contours[0]);
        qSuit_roi = qSuit_gray(bounding_rectangle);
        cv::resize(qSuit_roi, qSuit_sized, cv::Size(SUIT_WIDTH, SUIT_HEIGHT), 0, 0);
        qCard.suit_image = qSuit_sized;
    }

    return qCard;
};


std::tuple<std::string, std::string, double, double> matchCard(queryCard qCard, std::vector<trainRank> train_ranks, std::vector<trainSuit> train_suits)
{
    /* Finds best rank and suit matches for the query card. Differences
    the query card rank and suit images with the train rank and suit images.
    The best match is the rank or suit image that has the least difference. */

    cv::Size suitImageSize = qCard.rank_image.size(), rankImageSize = qCard.rank_image.size();
    cv::Size trainSuitSize, trainRankSize;
    cv::Mat diff_image;
    int rank_diff;
    double suit_diff;
    double best_rank_match_diff = 10000;
    double best_suit_match_diff = 10000;
    std::string best_rank_match_name = "Unknown";
    std::string best_suit_match_name = "Unknown";
    std::string best_rank_name, best_suit_name;
    int i = 0;


    /* If no contours were found in query card in preprocess_card function,
    the image size is zero, so skip the differencing process
    (the card will be left as "Unknown"). */
    if (!suitImageSize.empty() && !rankImageSize.empty())
    {
        // Difference the query card rank image from each of the train rank images
        // and store the result with the least difference
        for (trainRank tRank : train_ranks)
        {
            cv::absdiff(qCard.rank_image, tRank.rank_image, diff_image);
            rank_diff = (int)cv::sum(diff_image)[0] / 255;

            if (rank_diff < best_rank_match_diff)
            {
                best_rank_match_diff = rank_diff;
                best_rank_name = tRank.rank_name;
            }
        }

        // Same process with suits
        for (trainSuit tSuit : train_suits)
        {
            cv::absdiff(qCard.suit_image, tSuit.suit_image, diff_image);
            suit_diff = cv::sum(diff_image)[0] / 255;

            if (suit_diff < best_suit_match_diff)
            {
                best_suit_match_diff = suit_diff;
                best_suit_name = tSuit.suit_name;
            }
        }
    }

    /* Combine best rank match and best suit match to get query card's identity.
    If the best matches have too high of a difference value, card identity is
    still unknown. */
    if (best_rank_match_diff < RANK_DIFF_MAX) best_rank_match_name = best_rank_name;
    if (best_suit_match_diff < SUIT_DIFF_MAX) best_suit_match_name = best_suit_name;

    // Return the identity of the card and quality of the suit and rank match
    return std::make_tuple(best_rank_match_name, best_suit_match_name, best_rank_match_diff, best_suit_match_diff);
}

cv::Mat drawResults(cv::Mat image, queryCard q_card)
{
    /* Draw the card name, center point, and contour on the camera image. */

    int x = (int) q_card.center_point.x;
    int y = (int) q_card.center_point.y;

    cv::circle(image, cv::Point(x, y), 5, cv::Scalar(255, 0, 0), -1);

    std::string rank_name = q_card.best_rank_match;
    std::string suit_name = q_card.best_suit_match;

    // Draw card's name twice, so letters have a black outline
    cv::putText(image, (rank_name + " of"), cv::Point(x-60, y-10), FONT, 1, cv::Scalar(0,0,0), 3, cv::LINE_AA);
    cv::putText(image, (rank_name + " of"), cv::Point(x-60, y-10), FONT, 1, cv::Scalar(50,200,200), 2, cv::LINE_AA);

    cv::putText(image, suit_name, cv::Point(x-60,y+25), FONT, 1, cv::Scalar(0,0,0), 3, cv::LINE_AA);
    cv::putText(image, suit_name, cv::Point(x-60,y+25), FONT, 1, cv::Scalar(50,200,200), 2, cv::LINE_AA);

    // can draw difference value for troubleshooting purposes
    // commented out during normal operation
    std::string r_diff = std::to_string(q_card.rank_diff);
    std::string s_diff = std::to_string(q_card.suit_diff);
    cv::putText(image, r_diff, cv::Point(x+20, y+30), FONT, 0.5, cv::Scalar(0,0,255), 1, cv::LINE_AA);
    cv::putText(image, s_diff, cv::Point(x+20, y+50), FONT, 0.5, cv::Scalar(0,0,255), 1, cv::LINE_AA);

    return image;
};

cv::Mat flattener(cv::Mat image, std::vector<cv::Point2f> points, int width, int height)
{
    /* Flattens an image of a card into a top-down 200x300 perspective.
    Returns the flattened, re-sized, grayed image. Adapted from:
    www.pyimagesearch.com/2014/08/25/4-point-opencv-getperspective-transform-example/ */
    cv::Point2f temp_rect[4] = {{0,0},{0,0},{0,0},{0,0}};
    cv::Point2f top_left, top_right, bottom_left, bottom_right;
    cv::Mat matrix, warp;

    // sum & diff across all points in the array
    std::vector<float> sum, diff;
    for (const auto point : points)
    {
        sum.push_back(point.x + point.y);
        diff.push_back(point.y - point.x); // potential fault, difference order
    }

    // get the points in the array and map them to values
    top_left = points[std::distance(sum.begin(), std::min_element(sum.begin(), sum.end()))];
    bottom_right = points[std::distance(sum.begin(), std::max_element(sum.begin(), sum.end()))];
    top_right = points[std::distance(diff.begin(), std::min_element(diff.begin(), diff.end()))];
    bottom_left = points[std::distance(diff.begin(), std::max_element(diff.begin(), diff.end()))];

    // need to create an array listing points in order of
    // [top left, top right, bottom right, bottom left]
    // before doing the perspective transform

    if (width <= 0.8 * height) // vertical orientation
    {
        temp_rect[0] = top_left;
        temp_rect[1] = top_right;
        temp_rect[2] = bottom_right;
        temp_rect[3] = bottom_left;
        std::cout << "vertical orient" << std::endl;
    }

    if (width >= 1.2 * height) // horizontal orientation
    {
        temp_rect[0] = bottom_left;
        temp_rect[1] = top_left;
        temp_rect[2] = top_right;
        temp_rect[3] = bottom_right;
    }

    // If the card is 'diamond' oriented, a different algorithm
    // has to be used to identify which point is top left, top right
    // bottom left, and bottom right

    if (width > 0.8 * height && width < 1.2 * height)
    {
        // if furthest left point is higher than the furthest right point,
        // card is tilted to the left
        if (points[1].y <= points[3].y)
        {
            temp_rect[0] = points[1]; // top left
            temp_rect[1] = points[0]; // top right
            temp_rect[2] = points[3]; // bottom right
            temp_rect[3] = points[2]; // bottom left
        }
        // if the furthest left point is lower than the furthest right point
        // card is tilted to the right
        else if (points[1].y > points[3].y)
        {
            temp_rect[0] = points[0]; // top left
            temp_rect[1] = points[3]; // top right
            temp_rect[2] = points[2]; // bottom right
            temp_rect[3] = points[1]; // bottom left
        }
    }

    float maxWidth = 200, maxHeight = 300;

    // Create destination array, calculate perspective transform matrix,
    // and warp the card's image
    cv::Point2f destination[4] = {{0, 0}, {maxWidth - 1, 0},
                            {maxWidth - 1, maxHeight - 1}, {0, maxHeight - 1}};
    matrix = cv::getPerspectiveTransform(temp_rect, destination);
    cv::warpPerspective(image, warp, matrix, cv::Size2f(maxWidth, maxHeight));
    // cv::cvtColor(warp, warp, cv::COLOR_BGR2GRAY);

    return warp;
};