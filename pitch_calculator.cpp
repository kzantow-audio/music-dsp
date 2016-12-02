/*
 * pitch_calculator.cpp
 *
 * Implementation of various pitch calculation functions
 */

#include "pitch_calculator.h"

#include <stdexcept>
#include <iostream>

#include "lmtypes.h"
#include "lmhelpers.h"

#define FREQ_PRECISION          4
#define OCTAVES_CNT             9   /* from 0 to 8 */
#define SEMITONES_PER_OCTAVE    ((int32_t)12)


PitchCalculator::PitchCalculator()
{
    // TODO: apply properly -12 to math in pitchToNote() to make this map shorter
    __mNotesFromA4[-11] = note_A_sharp;
    __mNotesFromA4[-10] = note_B;
    __mNotesFromA4[-9] = note_C;
    __mNotesFromA4[-8] = note_C_sharp;
    __mNotesFromA4[-7] = note_D;
    __mNotesFromA4[-6] = note_D_sharp;
    __mNotesFromA4[-5] = note_E;
    __mNotesFromA4[-4] = note_F;
    __mNotesFromA4[-3] = note_F_sharp;
    __mNotesFromA4[-2] = note_G;
    __mNotesFromA4[-1] = note_G_sharp;
    __mNotesFromA4[0] = note_A;
    __mNotesFromA4[1] = note_A_sharp;
    __mNotesFromA4[2] = note_B;
    __mNotesFromA4[3] = note_C;
    __mNotesFromA4[4] = note_C_sharp;
    __mNotesFromA4[5] = note_D;
    __mNotesFromA4[6] = note_D_sharp;
    __mNotesFromA4[7] = note_E;
    __mNotesFromA4[8] = note_F;
    __mNotesFromA4[9] = note_F_sharp;
    __mNotesFromA4[10] = note_G;
    __mNotesFromA4[11] = note_G_sharp;

    __mSemitonesFromA4[note_C]       = -9;
    __mSemitonesFromA4[note_C_sharp] = -8;
    __mSemitonesFromA4[note_D]       = -7;
    __mSemitonesFromA4[note_D_sharp] = -6;
    __mSemitonesFromA4[note_E]       = -5;
    __mSemitonesFromA4[note_F]       = -4;
    __mSemitonesFromA4[note_F_sharp] = -3;
    __mSemitonesFromA4[note_G]       = -2;
    __mSemitonesFromA4[note_G_sharp] = -1;
    __mSemitonesFromA4[note_A]       = -0;
    __mSemitonesFromA4[note_A_sharp] = 1;
    __mSemitonesFromA4[note_B]       = 2;

    __initPitches();
}

PitchCalculator::~PitchCalculator()
{
    delete __mPitches;
}

void PitchCalculator::__initPitches()
{
    __mPitches = new freq_hz_t[SEMITONES_TOTAL];

    for (int32_t i = 0; i < SEMITONES_TOTAL; i++) {
        double n = i + SEMITONES_A0_TO_A4;
        __mPitches[i] = pow(2, (n / SEMITONES_PER_OCTAVE)) * FREQ_A4;
        __mPitches[i] = Helpers::stdRound<freq_hz_t>(__mPitches[i], FREQ_PRECISION);
        if(__mPitches[i] == FREQ_A4) {
            __mPitchIdxA4 = i;
        }
    }

    //TODO: add assert if __mPitchIdxA4 == 0
}

freq_hz_t PitchCalculator::getPitchByInterval(freq_hz_t pitch, uint16_t n)
{
    int16_t pitchIdx = __getPitchIdx(pitch);
    int16_t retIdx;

    if (pitchIdx < 0) {
        return FREQ_INVALID;
    }

    retIdx = pitchIdx + n;

    return ((retIdx >= 0) && (retIdx < SEMITONES_TOTAL) ?
            __mPitches[pitchIdx + n]  : FREQ_INVALID);
}

int16_t PitchCalculator::__getPitchIdx(freq_hz_t freq)
{
    uint16_t start = 0, end = SEMITONES_TOTAL - 1, mid;
    int16_t idx = -1;

    freq = Helpers::stdRound(freq, FREQ_PRECISION);

    while (start <= end) {
        mid = start + (end - start) / 2;

        if (__mPitches[mid] < freq) {
            start = mid + 1;
        } else if (__mPitches[mid] > freq) {
            end = mid - 1;
        } else {
            idx = mid;
            break;
        }
    }

    return idx;
}

bool PitchCalculator::__isPitch(freq_hz_t freq)
{
    return (__getPitchIdx(freq) >= 0);
}

/* TODO: reuse delta mechanism used in getPitch */
freq_hz_t PitchCalculator::__getTonic(amplitude_t *freqDomain, uint32_t len,
                                    uint32_t fftSize, uint32_t sampleRate)
{
    if (freqDomain == NULL || sampleRate == 0) {
        throw std::invalid_argument("Invalid argument");
    }

    amplitude_t max = freqDomain[0];
    amplitude_t mag;
    uint32_t i = 0, maxIndex = 0;

    /* TODO: replace with faster algorithm than the one with linear complexity */
    while (i < len) {
        mag = freqDomain[i];
        if (mag > max) {
            max = mag;
            maxIndex = i;
        }
        i++;
    }

    return (maxIndex * sampleRate / fftSize);
}

freq_hz_t PitchCalculator::getPitch(amplitude_t *freqDomain, uint32_t len,
                                  uint32_t fftSize, uint32_t sampleRate)
{
    freq_hz_t freqTonic;                // frequency with the highest amplitude
    freq_hz_t freqPitch = FREQ_INVALID; // closest pitch matching freqTonic
    freq_hz_t deltaRight, deltaLeft, deltaMid;
    uint16_t start = 0, end = SEMITONES_TOTAL - 1, mid;

    freqTonic = __getTonic(freqDomain, len, fftSize, sampleRate);

    if (__isPitch(freqTonic)) {
        freqPitch = freqTonic;
        goto ret;
    }

    while (start <= end) {
        mid = start + (end - start) / 2;
        deltaMid = abs(__mPitches[mid] - freqTonic);
        /* fix index out of range potential bug */
        deltaLeft = abs(__mPitches[mid - 1] - freqTonic);
        deltaRight = abs(__mPitches[mid + 1] - freqTonic);

        // TODO: check the case - when all values in __mPitches are 0 this
        // loop becomes infinite
        if (deltaLeft < deltaMid) {
            end = mid - 1;
        } else if (deltaRight < deltaMid) {
            start = mid + 1;
        } else if ((deltaMid < deltaLeft) && (deltaMid < deltaRight)) {
            freqPitch = __mPitches[mid];
            break;
        }
    }

 ret:
    return freqPitch;
}

note_t PitchCalculator::pitchToNote(freq_hz_t freq)
{
    if (!__isPitch(freq)) {
        throw std::invalid_argument("Invalid frequency - pitch is expected");
    }

    int32_t semitonesFromA4 = semitonesDistance(freq, FREQ_A4) % SEMITONES_PER_OCTAVE;

    return (__mNotesFromA4.find(semitonesFromA4) != __mNotesFromA4.end() ?
            __mNotesFromA4[semitonesFromA4] : note_Unknown);
}

freq_hz_t PitchCalculator::noteToPitch(note_t note, octave_t octave)
{
    if ((note < note_Min) || (note > note_Max)) {
        throw std::invalid_argument("Invalid note");
    }
    if ((octave < OCTAVE_MIN) || (octave > OCTAVE_MAX)) {
        throw std::invalid_argument("Invalid octave");
    }

    int16_t semitonesFromA4 = (__mSemitonesFromA4[note] +
                               (octave - OCTAVE_4) * SEMITONES_PER_OCTAVE);
    int16_t idx = __mPitchIdxA4 + semitonesFromA4;
    freq_hz_t ret;

    if ((idx < 0) || (idx >= SEMITONES_TOTAL)) {
        ret = FREQ_INVALID;
    } else {
        ret = __mPitches[idx];
    }

    return ret;
}

double PitchCalculator::octavesDistance(freq_hz_t f1, freq_hz_t f2)
{
    if (!IS_FREQ_VALID(f1) || !IS_FREQ_VALID(f2)) {
        throw std::invalid_argument("Invalid frequency");
    }

    return log2(f1 / f2);
}

int32_t PitchCalculator::semitonesDistance(freq_hz_t f1, freq_hz_t f2)
{
    return (int32_t)Helpers::stdRound(SEMITONES_PER_OCTAVE * octavesDistance(f1, f2), 0);
}