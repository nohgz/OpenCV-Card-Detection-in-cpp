#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <iostream>

// 8-14-2024 Initial Commit
/* --- Define Constants --- */

// Capture Image Sizes
extern const int IM_WIDTH, IM_HEIGHT;

// Adaptive Threshold Levels
extern const int BKG_THRESH, CARD_THRESH;

// Width and height of card corner, where rank and suit are
extern const int CORNER_WIDTH, CORNER_HEIGHT;

// Dimensions of rank train images
extern const int RANK_WIDTH, RANK_HEIGHT;

// Dimensions of suit train images
extern const int SUIT_WIDTH, SUIT_HEIGHT, RANK_DIFF_MAX, SUIT_DIFF_MAX, CARD_MAX_AREA, CARD_MIN_AREA;

// Font for Consistency
extern const int FONT;

// Framerate for VideoStream object
extern const int FRAME_RATE;

extern const std::string CARD_SUITS[4], CARD_RANKS[13];

#endif