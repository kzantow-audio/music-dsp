/**
 * @file        window_functions.h
 * @addtogroup  libmusic
 * @{
 */

#pragma once

#include <vector>

#include "lmtypes.h"

#ifndef WINDOW_FUNCIONS_TEST_FRIENDS
#define WINDOW_FUNCIONS_TEST_FRIENDS
#endif


class WindowFunctions {

WINDOW_FUNCIONS_TEST_FRIENDS;

public:
    static void applyHamming(amplitude_t *, uint32_t);
    static void applyHann(amplitude_t *, uint32_t);
    static void applyBlackman(amplitude_t *, uint32_t);
    static void applyDefault(amplitude_t *, uint32_t);
    static std::vector<amplitude_t> getHamming(uint32_t len, uint32_t offset);
    static const char * toString(uint32_t);
};

/** @} */
