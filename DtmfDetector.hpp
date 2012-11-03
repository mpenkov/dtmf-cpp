/** Author:       Plyashkevich Viatcheslav <plyashkevich@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * All rights reserved.
 */


#ifndef DTMF_DETECTOR
#define DTMF_DETECTOR

#include "types_cpp.hpp"


typedef Types<sizeof(long int), sizeof(int), sizeof(short int), sizeof(char)>::Int32     INT32;
typedef Types<sizeof(long int), sizeof(int), sizeof(short int), sizeof(char)>::Uint32    UINT32;
typedef Types<sizeof(long int), sizeof(int), sizeof(short int), sizeof(char)>::Int16     INT16;
typedef Types<sizeof(long int), sizeof(int), sizeof(short int), sizeof(char)>::Uint16    UINT16;


// DTMF detector object

class DtmfDetectorInterface
{
public:
    static const UINT32 NUMBER_OF_BUTTONS = 65;
    char dialButtons[NUMBER_OF_BUTTONS];
    char *const pDialButtons;
    mutable INT16 indexForDialButtons;
public:
    INT32 getIndexDialButtons() // The number of detected push buttons, max number = 64
    const
    {
        return indexForDialButtons;
    }
    char *getDialButtonsArray() // Address of array, where store detected push buttons
    const
    {
        return pDialButtons;
    }
    void zerosIndexDialButton() // Zeros of a detected button array
    const
    {
        indexForDialButtons = 0;
    }

    DtmfDetectorInterface():indexForDialButtons(0), pDialButtons(dialButtons)
    {
        dialButtons[0] = 0;
    }
};

class DtmfDetector : public DtmfDetectorInterface
{
protected:
    static const unsigned COEFF_NUMBER=18;
    static const INT16 CONSTANTS[COEFF_NUMBER];
    INT16 *pArraySamples;
    INT32 T[COEFF_NUMBER];
    INT16  *internalArray;
    const INT32 frameSize; //Size of a frame is measured in INT16(word)
    static const INT32 SAMPLES;
    INT32 frameCount;
    char prevDialButton;
    char permissionFlag;

    static INT32 powerThreshold;
    static INT32 dialTonesToOhersTones;
    static INT32 dialTonesToOhersDialTones;

    char DTMF_detection(INT16 short_array_samples[]);
public:

// frameSize_ - input frame size
    DtmfDetector(INT32 frameSize_);
    ~DtmfDetector();

    void dtmfDetecting(INT16 inputFrame[]); // The DTMF detection.
    // The size of a inputFrame must be equal of a frameSize_, who
    // was set in constructor.
};

#endif




