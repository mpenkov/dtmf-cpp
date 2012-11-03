/** Author:       Plyashkevich Viatcheslav <plyashkevich@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * All rights reserved.
 */

#include "DtmfGenerator.hpp"

// Multiplicaton of two fixed-point numbers
static inline INT32 MPY48SR(INT16 o16, INT32 o32)
{
    // http://stackoverflow.com/questions/12864216/why-perform-multiplication-in-this-way
    UINT32   Temp0;
    INT32    Temp1;
    // A1. get the lower 16 bits of the 32-bit param
    // A2. multiply them with the 16-bit param
    // A3. add 16384
    // A4. bitshift to the right by 15 (TODO: why 15?)
    Temp0 = (((UINT16)o32 * o16) + 0x4000) >> 15;
    // B1. Get the higher 16 bits of the 32-bit param
    // B2. Multiply them with the 16-bit param
    Temp1 = (INT16)(o32 >> 16) * o16;
    // 1. Shift B to the left (TODO: why do this?)
    // 2. Combine with A and return
    return (Temp1 << 1) + Temp0;
}

// Generate a dual-tone multiple frequency signal and write it to a buffer.
//
// Coeff0   A coefficient for the first frequency
// Coeff1   A coefficient for the second frequency
// y        The output buffer
// COUNT    Size of the output buffer
// y1_0     ?
// y1_1
// y2_0
// y2_1
static void frequency_oscillator(INT16 Coeff0, INT16 Coeff1,
                                 INT16 y[], UINT32 COUNT,
                                 INT32 *y1_0, INT32 *y1_1,
                                 INT32 *y2_0, INT32 *y2_1)
{
    // the register keyword isn't really useful and achieves little.
    // http://www.drdobbs.com/keywords-that-arent-or-comments-by-anoth/184403859
    register INT32 Temp1_0, Temp1_1, Temp2_0, Temp2_1, Temp0, Temp1, Subject;
    UINT16 ii;

    // Write the parameters to the registers.
    // As far as I can tell, using commas instead of the semicolon does not
    // change the program semantically.
    // http://en.wikipedia.org/wiki/Comma_operator
    Temp1_0 = *y1_0,
    Temp1_1 = *y1_1,
    Temp2_0 = *y2_0,
    Temp2_1 = *y2_1,
    // TODO: what is the purpose of Subject?
    Subject = Coeff0 * Coeff1;
    for(ii = 0; ii < COUNT; ++ii)
    {
        Temp0 = MPY48SR(Coeff0, Temp1_0 << 1) - Temp2_0,
        Temp1 = MPY48SR(Coeff1, Temp1_1 << 1) - Temp2_1;
        Temp2_0 = Temp1_0,
        Temp2_1 = Temp1_1;
        Temp1_0 = Temp0,
        Temp1_1 = Temp1,
        Temp0 += Temp1;
        // "X >>= Y" means: "X = X >> Y", i.e. shift X right by Y bits.
        // http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B
        if(Subject)
            Temp0 >>= 1;
        y[ii] = (INT16)Temp0;
    }

    *y1_0 = Temp1_0,
     *y1_1 = Temp1_1,
      *y2_0 = Temp2_0,
       *y2_1 = Temp2_1;
}

// These frequencies match what is described on:
// http://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling
const INT16 DtmfGenerator::tempCoeff[8] = {
    //Low frequencies (row)
    27980, // 697Hz
    26956, // 770Hz
    25701, // 852Hz
    24218, // 941Hz
    //High frequencies (column)
    19073, // 1209Hz
    16325, // 1335Hz
    13085, // 1477Hz
    9315   // 1633Hz
}; 

DtmfGenerator::DtmfGenerator(INT32 FrameSize, INT32 DurationPush, INT32 DurationPause)
{
    // N.B. bit-shifting to the right corresponds to a multiplication by 8.
    // Determine the number of buffers each tone and silence should occupy.
    countDurationPushButton = (DurationPush << 3)/FrameSize + 1;
    countDurationPause = (DurationPause << 3)/FrameSize + 1;
    sizeOfFrame = FrameSize;
    readyFlag = 1;
    countLengthDialButtonsArray = 0;
}

// The destructor does nothing.
DtmfGenerator::~DtmfGenerator()
{
}

void DtmfGenerator::dtmfGenerating(INT16 y[])
{
    if(readyFlag)   return;

    // Iterate over all the tones we've been instructed to generate
    while(countLengthDialButtonsArray > 0)
    {
        // If we're starting a new tone, then determine the 
        // coefficients for it.  Otherwise, we're mid-tone, so we can
        // just use whatever is already set.
        if(countDurationPushButton == tempCountDurationPushButton)
        {
            // N.B. y2_1 and y2_2 always seem to be 31000
            switch(pushDialButtons[count])
            {
            case '1':
                tempCoeff1 = tempCoeff[0];
                tempCoeff2 = tempCoeff[4];
                y1_1 = tempCoeff[0];
                y2_1 = 31000;
                y1_2 = tempCoeff[4];
                y2_2 = 31000;
                break;
            case '2':
                tempCoeff1 = tempCoeff[0];
                tempCoeff2 = tempCoeff[5];
                y1_1 = tempCoeff[0];
                y2_1 = 31000;
                y1_2 = tempCoeff[5];
                y2_2 = 31000;
                break;
            case '3':
                tempCoeff1 = tempCoeff[0];
                tempCoeff2 = tempCoeff[6];
                y1_1 = tempCoeff[0];
                y2_1 = 31000;
                y1_2 = tempCoeff[6];
                y2_2 = 31000;
                break;
            case 'A':
                tempCoeff1 = tempCoeff[0];
                tempCoeff2 = tempCoeff[7];
                y1_1 = tempCoeff[0];
                y2_1 = 31000;
                y1_2 = tempCoeff[7];
                y2_2 = 31000;
                break;
            case '4':
                tempCoeff1 = tempCoeff[1];
                tempCoeff2 = tempCoeff[4];
                y1_1 = tempCoeff[1];
                y2_1 = 31000;
                y1_2 = tempCoeff[4];
                y2_2 = 31000;
                break;
            case '5':
                tempCoeff1 = tempCoeff[1];
                tempCoeff2 = tempCoeff[5];
                y1_1 = tempCoeff[1];
                y2_1 = 31000;
                y1_2 = tempCoeff[5];
                y2_2 = 31000;
                break;
            case '6':
                tempCoeff1 = tempCoeff[1];
                tempCoeff2 = tempCoeff[6];
                y1_1 = tempCoeff[1];
                y2_1 = 31000;
                y1_2 = tempCoeff[6];
                y2_2 = 31000;
                break;
            case 'B':
                tempCoeff1 = tempCoeff[1];
                tempCoeff2 = tempCoeff[7];
                y1_1 = tempCoeff[1];
                y2_1 = 31000;
                y1_2 = tempCoeff[7];
                y2_2 = 31000;
                break;
            case '7':
                tempCoeff1 = tempCoeff[2];
                tempCoeff2 = tempCoeff[4];
                y1_1 = tempCoeff[2];
                y2_1 = 31000;
                y1_2 = tempCoeff[4];
                y2_2 = 31000;
                break;
            case '8':
                tempCoeff1 = tempCoeff[2];
                tempCoeff2 = tempCoeff[5];
                y1_1 = tempCoeff[2];
                y2_1 = 31000;
                y1_2 = tempCoeff[5];
                y2_2 = 31000;
                break;
            case '9':
                tempCoeff1 = tempCoeff[2];
                tempCoeff2 = tempCoeff[6];
                y1_1 = tempCoeff[2];
                y2_1 = 31000;
                y1_2 = tempCoeff[6];
                y2_2 = 31000;
                break;
            case 'C':
                tempCoeff1 = tempCoeff[2];
                tempCoeff2 = tempCoeff[7];
                y1_1 = tempCoeff[2];
                y2_1 = 31000;
                y1_2 = tempCoeff[7];
                y2_2 = 31000;
                break;
            case '*':
                tempCoeff1 = tempCoeff[3];
                tempCoeff2 = tempCoeff[4];
                y1_1 = tempCoeff[3];
                y2_1 = 31000;
                y1_2 = tempCoeff[4];
                y2_2 = 31000;
                break;
            case '0':
                tempCoeff1 = tempCoeff[3];
                tempCoeff2 = tempCoeff[5];
                y1_1 = tempCoeff[3];
                y2_1 = 31000;
                y1_2 = tempCoeff[5];
                y2_2 = 31000;
                break;
            case '#':
                tempCoeff1 = tempCoeff[3];
                tempCoeff2 = tempCoeff[6];
                y1_1 = tempCoeff[3];
                y2_1 = 31000;
                y1_2 = tempCoeff[6];
                y2_2 = 31000;
                break;
            case 'D':
                tempCoeff1 = tempCoeff[3];
                tempCoeff2 = tempCoeff[7];
                y1_1 = tempCoeff[3];
                y2_1 = 31000;
                y1_2 = tempCoeff[7];
                y2_2 = 31000;
                break;
            default:
                tempCoeff1 = tempCoeff2 = 0;
                y1_1 = 0;
                y2_1 = 0;
                y1_2 = 0;
                y2_2 = 0;
            }
        }
        // We've determined the coefficients for the current tone.
        // Now determine whether we're in the middle of a tone or 
        // a pause.  In either case, we fill up the output buffer
        // and return.
        while(tempCountDurationPushButton>0)
        {
            // Handle the dial tone.
            --tempCountDurationPushButton;

            frequency_oscillator(tempCoeff1, tempCoeff2,
                                 y, sizeOfFrame,
                                 &y1_1, &y1_2,
                                 &y2_1, &y2_2
                                );
            return;
        }

        while(tempCountDurationPause>0)
        {
            // Handle silence.  Simply zeros the buffer.
            --tempCountDurationPause;
            for(INT32 ii=0; ii<sizeOfFrame; ii++)
            {
                y[ii] = 0;
            }
            return;
        }

        // If we've made it this far, it means that the current 
        // tone/silence has been completely generated.  Therefore,
        // prepare ourselves to generate the next tone and silence,
        // whichever comes next.
        tempCountDurationPushButton = countDurationPushButton;
        tempCountDurationPause = countDurationPause;

        // increment counters.
        ++count;
        --countLengthDialButtonsArray;
    }
    // We've run out of tones to generate, so indicate that we're not ready
    // to output any more.
    readyFlag = 1;
    return;
}

INT32 DtmfGenerator::transmitNewDialButtonsArray(char dialButtonsArray[], UINT32 lengthDialButtonsArray)
{
    // If we're still busy processing the previous tones, exit straight away.
    if(getReadyFlag() == 0) return 0;
    // We've been given an empty array to process.  Reset ourselves and exit.
    if(lengthDialButtonsArray == 0)
    {
        countLengthDialButtonsArray = 0;
        count = 0;
        readyFlag = 1;
        return 1;
    }
    countLengthDialButtonsArray = lengthDialButtonsArray;
    // clip the input to a size our pushDialButtons fixed-size array can 
    // accomodate, and populate that array.
    if(lengthDialButtonsArray > 20) countLengthDialButtonsArray = 20;
    for(INT32 ii=0; ii<countLengthDialButtonsArray; ii++)
    {
        pushDialButtons[ii] = dialButtonsArray[ii];
    }

    // prepare ourselves to generate the next tone and silence,
    // whichever comes next.
    tempCountDurationPushButton = countDurationPushButton;
    tempCountDurationPause = countDurationPause;

    count = 0;
    readyFlag = 0;
    return 1;
}
