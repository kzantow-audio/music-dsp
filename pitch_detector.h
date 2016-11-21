/*
 * pitch_detector.h
 */

#pragma once

#include <map>
#include <vector>

#include "lmtypes.h"

#define FREQ_INVALID            ((freq_hz_t) -1)
#define SEMITONES_A0_TO_A4      -48 // TODO: define proper values and notes
#define SEMITONES_A4_TO_C8      200 // TODO: define proper values and notes
#define SEMITONES_TOTAL         (SEMITONES_A4_TO_C8 - SEMITONES_A0_TO_A4)

#ifndef PITCH_DETECTOR_TEST_FRIENDS
#define PITCH_DETECTOR_TEST_FRIENDS
#endif

class PitchDetector {

PITCH_DETECTOR_TEST_FRIENDS;

private:
    freq_hz_t                   *__mPitches;
    std::map<int8_t, note_t>    __mNotesFromA4;

    void __initPitches();

    /**
     * @ return true if freq matches one of the pitch frequencies
     */
    bool __isPitch(freq_hz_t freq);

    /**
     * Find frequency with the highest amplitude
     *
     * @param   x           frequency domain data
     * @param   sampleRate  sample rate of x
     * @return  frequency value
     */
    freq_hz_t __getTonic(std::vector<complex_t> x, uint32_t sampleRate);

public:
    /**
     * Constructor
     */
    PitchDetector();

    /**
     * Destructor
     */
    ~PitchDetector();

    /**
     * Correspond frequency to the pitch
     *
     * In other words round up provided frequency value to one of the pitch
     * frequencies
     *
     * @param   freq        arbitrary frequency
     * @param   sampleRate  sample rate of x
     * @return  pitch frequency corresponding to freq
     */
    freq_hz_t getPitch(std::vector<complex_t> x, uint32_t sampleRate);

    /**
     * Find note corresponding to the given pitch
     *
     * @param   pitch frequency in Hz
     * @return  note corresponding to the given pitch
     */
    note_t pitchToNote(freq_hz_t freq);

    /**
     * Find octave corresponding to the given pitch
     *
     * @param   pitch frequency in Hz
     * @return  octave number corresponding to the given pitch
     */
    note_t pitchToOctave(freq_hz_t freq);
};
