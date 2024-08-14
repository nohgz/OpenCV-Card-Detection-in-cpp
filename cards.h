#pragma once

#ifndef CARDS_H
#define CARDS_H

#include <opencv2/opencv.hpp>
#include "constants.h"

// 8-14-2024 Initial Commit

/* -- Structures to hold query card and train card information -- */
struct queryCard
{
    std::vector<cv::Point> contour; // Contour of card
    double width, height; // Width and height of card
    double rank_diff, suit_diff; // Difference between rank or suit image and best matched train image
    std::vector<cv::Point2f> corner_points; // Corner points of card
    cv::Point2f center_point; // Center point of card
    cv::Mat warp; // 200x300, flattened, grayed, blurred image
    cv::Mat rank_image, suit_image; // Thresholded, sized images of card's rank and suit
    std::string best_rank_match, best_suit_match; // Best matched rank and suit
};

struct trainRank
{
    cv::Mat rank_image; // Thresholded, sized card image loaded from drive
    std::string rank_name; // Name of card
};

struct trainSuit
{
    cv::Mat suit_image; // Thresholded, sized card image loaded from drive
    std::string suit_name; // Name of card
};

/* -- Function Prototypes (implemented in .cpp file) -- */
std::vector<trainRank> loadRanks(std::string filepath);
std::vector<trainSuit> loadSuits(std::string filepath);

cv::Mat preprocessImage(cv::Mat image);
std::tuple<std::vector<int>, std::vector<std::vector<cv::Point>>> findCards(cv::Mat thresh_image);
queryCard preprocessCard(std::vector<cv::Point> contour, cv::Mat image);
std::tuple<std::string, std::string, double, double> matchCard(queryCard qCard, std::vector<trainRank> train_ranks, std::vector<trainSuit> train_suits);
cv::Mat drawResults(cv::Mat image, queryCard q_card);
cv::Mat flattener(cv::Mat image, std::vector<cv::Point2f> points, int width, int height);

#endif