#include "constants.h"
#include "opencv2/opencv.hpp"

// 8-14-2024 Initial Commit
/* -- Define Constants -- */
// Adaptive Threshold Levels
const int BKG_THRESH = 60;
const int CARD_THRESH = 30;

// Width and height of card corner, where rank and suit are
const int CORNER_WIDTH = 32;
const int CORNER_HEIGHT = 84;

// Dimensions of rank train images
const int RANK_WIDTH = 70;
const int RANK_HEIGHT = 125;

// Dimensions of suit train images
const int SUIT_WIDTH = 70;
const int SUIT_HEIGHT = 100;
const int RANK_DIFF_MAX = 2000;
const int SUIT_DIFF_MAX = 700;
const int CARD_MAX_AREA = 120000;
const int CARD_MIN_AREA = 25000;

// Font for the whole thing
const int FONT = cv::FONT_HERSHEY_SIMPLEX;

// Image dimensions of camera
const int IM_WIDTH = 1280;
const int IM_HEIGHT = 720;

// Frame rate for VideoStream object
const int FRAME_RATE = 20;

// A list for all ranks that a card can be
const std::string CARD_RANKS[13] = {"Ace", "Two", "Three", "Four", "Five", "Six", "Seven",
         "Eight", "Nine", "Ten", "Jack", "Queen", "King"};

// A list for all suits that a card can be
const std::string CARD_SUITS[4] = {"Hearts", "Spades", "Diamonds", "Clubs"};