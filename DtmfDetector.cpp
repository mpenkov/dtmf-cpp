/** Author:       Plyashkevich Viatcheslav <plyashkevich@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * All rights reserved.
 */

#include <cassert>
#include "DtmfDetector.hpp"

#if DEBUG
#include <cstdio>
#endif

// This is the same function as in DtmfGenerator.cpp
static inline INT32 MPY48SR(INT16 o16, INT32 o32)
{
    UINT32   Temp0;
    INT32    Temp1;
    Temp0 = (((UINT16)o32 * o16) + 0x4000) >> 15;
    Temp1 = (INT16)(o32 >> 16) * o16;
    return (Temp1 << 1) + Temp0;
}

// The Goertzel algorithm.
// For a good description and walkthrough, see:
// https://sites.google.com/site/hobbydebraj/home/goertzel-algorithm-dtmf-detection
//
// Koeff0           Coefficient for the first frequency.
// Koeff1           Coefficient for the second frequency.
// arraySamples     Input samples to process.  Must be COUNT elements long.
// Magnitude0       Detected magnitude of the first frequency.
// Magnitude1       Detected magnitude of the second frequency.
// COUNT            The number of elements in arraySamples.  Always equal to
//                  SAMPLES in practice.
static void goertzel_filter(INT16 Koeff0, INT16 Koeff1, const INT16 arraySamples[], INT32 *Magnitude0, INT32 *Magnitude1, UINT32 COUNT)
{
    INT32 Temp0, Temp1;
    UINT16 ii;
    // Vk1_0    prev (first frequency)
    // Vk2_0    prev_prev (first frequency)
    //
    // Vk1_1    prev (second frequency)
    // Vk2_0    prev_prev (second frequency)
    INT32 Vk1_0 = 0, Vk2_0 = 0, Vk1_1 = 0, Vk2_1 = 0;

    // Iterate over all the input samples
    // For each sample, process the two frequencies we're interested in:
    // output = Input + 2*coeff*prev - prev_prev
    // N.B. bit-shifting to the left achieves the multiplication by 2.
    for(ii = 0; ii < COUNT; ++ii)
    {
        Temp0 = MPY48SR(Koeff0, Vk1_0 << 1) - Vk2_0 + arraySamples[ii],
        Temp1 = MPY48SR(Koeff1, Vk1_1 << 1) - Vk2_1 + arraySamples[ii];
        Vk2_0 = Vk1_0,
        Vk2_1 = Vk1_1;
        Vk1_0 = Temp0,
        Vk1_1 = Temp1;
    }

    // Magnitude: prev_prev**prev_prev + prev*prev - coeff*prev*prev_prev

    // TODO: what does shifting by 10 bits to the right achieve?  Probably to
    // make room for the magnitude calculations.
    Vk1_0 >>= 10,
              Vk1_1 >>= 10,
                        Vk2_0 >>= 10,
                                  Vk2_1 >>= 10;
    Temp0 = MPY48SR(Koeff0, Vk1_0 << 1),
    Temp1 = MPY48SR(Koeff1, Vk1_1 << 1);
    Temp0 = (INT16)Temp0 * (INT16)Vk2_0,
    Temp1 = (INT16)Temp1 * (INT16)Vk2_1;
    Temp0 = (INT16)Vk1_0 * (INT16)Vk1_0 + (INT16)Vk2_0 * (INT16)Vk2_0 - Temp0;
    Temp1 = (INT16)Vk1_1 * (INT16)Vk1_1 + (INT16)Vk2_1 * (INT16)Vk2_1 - Temp1;
    *Magnitude0 = Temp0,
     *Magnitude1 = Temp1;
    return;
}


// This is a GSM function, for concrete processors she may be replaced
// for same processor's optimized function (norm_l)
// This is a GSM function, for concrete processors she may be replaced
// for same processor's optimized function (norm_l)
//
// This function is used for normalization. TODO: how exactly does it work?
static inline INT16 norm_l(INT32 L_var1)
{
    INT16 var_out;

    if (L_var1 == 0)
    {
        var_out = 0;
    }
    else
    {
        if (L_var1 == (INT32)0xffffffff)
        {
            var_out = 31;
        }
        else
        {
            if (L_var1 < 0)
            {
                L_var1 = ~L_var1;
            }

            for(var_out = 0; L_var1 < (INT32)0x40000000; var_out++)
            {
                L_var1 <<= 1;
            }
        }
    }

    return(var_out);
}

const UINT32 DtmfDetectorInterface::NUMBER_OF_BUTTONS;
const unsigned DtmfDetector::COEFF_NUMBER;
// These frequencies are slightly different to what is in the generator.
// More importantly, they are also different to what is described at:
// http://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling
//
// Some frequencies seem to match musical notes:
// http://www.geocities.jp/pyua51113/artist_data/ki_setsumei.html 
// http://en.wikipedia.org/wiki/Piano_key_frequencies
//
// It seems this is done to simplify harmonic detection.  
//
const INT16 DtmfDetector::CONSTANTS[COEFF_NUMBER] = {
    27860,  // 0: 706Hz, harmonics include: 78Hz, 235Hz, 3592Hz 
    26745,  // 1: 784Hz, apparently a high G, harmonics: 78Hz
    25529,  // 2: 863Hz, harmonics: 78Hz 
    24216,  // 3: 941Hz, harmonics: 78Hz, 235Hz, 314Hz
    19747,  // 4: 1176Hz, harmonics: 78Hz, 235Hz, 392Hz, 3529Hz
    16384,  // 5: 1333Hz, harmonics: 78Hz 
    12773,  // 6: 1490Hz, harmonics: 78Hz, 2980Hz
    8967,   // 7: 1547Hz, harmonics: 314Hz, 392Hz
    // The next coefficients correspond to frequencies of harmonics of the 
    // near-DTMF frequencies above, as well as of other frequencies listed
    // below.
    21319,  // 1098Hz
    29769,  // 549Hz
    // 549Hz is:
    // - half of 1098Hz (see above)
    // - 1/3 of 1633Hz, a real DTMF frequency (see DtmfGenerator)
    32706,  // 78Hz, a very low D# on a piano
    // 78Hz is a very convenient frequency, since its (approximately): 
    // - 1/3 of 235Hz (not a DTMF frequency, but we do detect it, see below)
    // - 1/4 of 314Hz (not a DTMF frequency, but we do detect it, see below)
    // - 1/5 of 392Hz (not a DTMF frequency, but we do detect it, see below)
    // - 1/7 of 549Hz
    // - 1/9 of 706Hz
    // - 1/10 of 784Hz
    // - 1/11 of 863Hz
    // - 1/12 of 941Hz
    // - 1/14 of 1098Hz (not a DTMF frequency, but we do detect it, see above)
    // - 1/15 of 1176Hz
    // - 1/17 of 1333Hz
    // - 1/19 of 1490Hz
    32210,  // 235Hz
    // 235Hz is:
    // - 1/3 of 706Hz
    // - 1/4 of 941Hz
    // - 1/5 of 1176Hz
    // - 1/15 of 3529Hz (not a DTMF frequency, but we do detect it, see below)
    31778,  // 314Hz
    // 314Hz is:
    // - 1/3 of 941Hz
    // - 1/5 of 1547Hz
    // - 1/8 of 2510Hz (not a DTMF frequency, but we do detect it, see below)
    31226,  // 392Hz, apparently a middle-2 G
    // 392Hz is:
    // - 1/2 of 794Hz
    // - 1/3 of 1176Hz
    // - 1/4 of 1547Hz
    // - 1/9 of 3529Hz
    -1009,  // 2039Hz TODO: why is this frequency useful?
    -12772, // 2510Hz, which is 8*314Hz
    -22811, // 2980Hz, which is 2*1490Hz
    -30555  // 3529Hz, 3*1176Hz, 5*706Hz
};
INT32 DtmfDetector::powerThreshold = 328;
INT32 DtmfDetector::dialTonesToOhersTones = 16;
INT32 DtmfDetector::dialTonesToOhersDialTones = 6;
const INT32 DtmfDetector::SAMPLES = 102;
//--------------------------------------------------------------------
DtmfDetector::DtmfDetector(INT32 frameSize_): frameSize(frameSize_)
{
    // 
    // This array is padded to keep the last batch, which is smaller
    // than SAMPLES, from the previous call to dtmfDetecting.
    //
    pArraySamples = new INT16 [frameSize + SAMPLES];
    internalArray = new INT16 [SAMPLES];
    frameCount = 0;
    prevDialButton = ' ';
    permissionFlag = 0;
}
//---------------------------------------------------------------------
DtmfDetector::~DtmfDetector()
{
    delete [] pArraySamples;
    delete [] internalArray;
}

void DtmfDetector::dtmfDetecting(INT16 input_array[])
{
    // ii                   Variable for iteration
    // temp_dial_button     A tone detected in part of the input_array
    UINT32 ii;
    char temp_dial_button;

    // Copy the input array into the middle of pArraySamples.
    // I think the first frameCount samples contain the last batch from the
    // previous call to this function.
    for(ii=0; ii < frameSize; ii++)
    {
        pArraySamples[ii + frameCount] = input_array[ii];
    }

    frameCount += frameSize;
    // Read index into pArraySamples that corresponds to the current batch.
    UINT32 temp_index = 0;
    // If don't have enough samples to process an entire batch, then don't
    // do anything.
    if(frameCount >= SAMPLES)
    {
        // Process samples while we still have enough for an entire
        // batch.
        while(frameCount >= SAMPLES)
        {
            // Determine the tone present in the current batch
            temp_dial_button = DTMF_detection(&pArraySamples[temp_index]);

            // Determine if we should register it as a new tone, or
            // ignore it as a continuation of a previously 
            // registered tone.  
            //
            // This seems buggy.  Consider a sequence of three
            // tones, with each tone corresponding to the dominant
            // tone in a batch of SAMPLES samples:
            //
            // SILENCE TONE_A TONE_B will get registered as TONE_B
            //
            // TONE_A will be ignored.
            if(permissionFlag)
            {
                if(temp_dial_button != ' ')
                {
                    dialButtons[indexForDialButtons++] = temp_dial_button;
                    // NUL-terminate the string.
                    dialButtons[indexForDialButtons] = 0;
                    // If we've gone out of bounds, wrap around.
                    if(indexForDialButtons >= 64)
                        indexForDialButtons = 0;
                }
                permissionFlag = 0;
            }

            // If we've gone from silence to a tone, set the flag.
            // The tone will be registered in the next iteration.
            if((temp_dial_button != ' ') && (prevDialButton == ' '))
            {
                permissionFlag = 1;
            }

            // Store the current tone.  In light of the above
            // behaviour, all that really matters is whether it was
            // a tone or silence.  Finally, move on to the next
            // batch.
            prevDialButton = temp_dial_button;

            temp_index += SAMPLES;
            frameCount -= SAMPLES;
        }

        //
        // We have frameCount samples left to process, but it's not
        // enough for an entire batch.  Shift these left-over
        // samples to the beginning of our array and deal with them
        // next time this function is called.
        //
        for(ii=0; ii < frameCount; ii++)
        {
            pArraySamples[ii] = pArraySamples[ii + temp_index];
        }
    }

}
//-----------------------------------------------------------------
// Detect a tone in a single batch of samples (SAMPLES elements).
char DtmfDetector::DTMF_detection(INT16 short_array_samples[])
{
    INT32 Dial=32, Sum;
    char return_value=' ';
    unsigned ii;
    Sum = 0;

    // Dial         TODO: what is this?
    // Sum          Sum of the absolute values of samples in the batch.
    // return_value The tone detected in this batch (can be silence).
    // ii           Iteration variable

    // Quick check for silence.
    for(ii = 0; ii < SAMPLES; ii ++)
    {
        if(short_array_samples[ii] >= 0)
            Sum += short_array_samples[ii];
        else
            Sum -= short_array_samples[ii];
    }
    Sum /= SAMPLES;
    if(Sum < powerThreshold)
        return ' ';

    //Normalization
    // Iterate over each sample.  
    // First, adjusting Dial to an appropriate value for the batch.
    for(ii = 0; ii < SAMPLES; ii++)
    {
        T[0] = static_cast<INT32>(short_array_samples[ii]);
        if(T[0] != 0)
        {
            if(Dial > norm_l(T[0]))
            {
                Dial = norm_l(T[0]);
            }
        }
    }

    Dial -= 16;

    // Next, utilize Dial for scaling and populate internalArray.
    for(ii = 0; ii < SAMPLES; ii++)
    {
        T[0] = short_array_samples[ii];
        internalArray[ii] = static_cast<INT16>(T[0] << Dial);
    }


    //Frequency detection
    goertzel_filter(CONSTANTS[0], CONSTANTS[1], internalArray, &T[0], &T[1], SAMPLES);
    goertzel_filter(CONSTANTS[2], CONSTANTS[3], internalArray, &T[2], &T[3], SAMPLES);
    goertzel_filter(CONSTANTS[4], CONSTANTS[5], internalArray, &T[4], &T[5], SAMPLES);
    goertzel_filter(CONSTANTS[6], CONSTANTS[7], internalArray, &T[6], &T[7], SAMPLES);
    goertzel_filter(CONSTANTS[8], CONSTANTS[9], internalArray, &T[8], &T[9], SAMPLES);
    goertzel_filter(CONSTANTS[10], CONSTANTS[11], internalArray, &T[10], &T[11], SAMPLES);
    goertzel_filter(CONSTANTS[12], CONSTANTS[13], internalArray, &T[12], &T[13], SAMPLES);
    goertzel_filter(CONSTANTS[14], CONSTANTS[15], internalArray, &T[14], &T[15], SAMPLES);
    goertzel_filter(CONSTANTS[16], CONSTANTS[17], internalArray, &T[16], &T[17], SAMPLES);

#if DEBUG
    for (ii = 0; ii < COEFF_NUMBER; ++ii)
        printf("%d ", T[ii]);
    printf("\n");
#endif

    INT32 Row = 0;
    INT32 Temp = 0;
    // Row      Index of the maximum row frequency in T
    // Temp     The frequency at the maximum row/column (gets reused 
    //          below).
    //Find max row(low frequences) tones
    for(ii = 0; ii < 4; ii++)
    {
        if(Temp < T[ii])
        {
            Row = ii;
            Temp = T[ii];
        }
    }

    // Column   Index of the maximum column frequency in T
    INT32 Column = 4;
    Temp = 0;
    //Find max column(high frequences) tones
    for(ii = 4; ii < 8; ii++)
    {
        if(Temp < T[ii])
        {
            Column = ii;
            Temp = T[ii];
        }
    }

    Sum=0;
    //Find average value dial tones without max row and max column
    for(ii = 0; ii < 10; ii++)
    {
        Sum += T[ii];
    }
    Sum -= T[Row];
    Sum -= T[Column];
    // N.B. Divide by 8
    Sum >>= 3;

    // N.B. looks like avoiding a divide by zero.
    if(!Sum)
        Sum = 1;

    //If relations max row and max column to average value
    //are less then threshold then return
    // This means the tones are too quiet compared to the other, non-max
    // DTMF frequencies.
    if(T[Row]/Sum < dialTonesToOhersDialTones)
        return ' ';
    if(T[Column]/Sum < dialTonesToOhersDialTones)
        return ' ';

    // Next, check if the volume of the row and column frequencies
    // is similar.  If they are different, then they aren't part of
    // the same tone.
    //
    // In the literature, this is known as "twist".
    //If relations max colum to max row is large then 4 then return
    if(T[Row] < (T[Column] >> 2)) return ' ';
    //If relations max colum to max row is large then 4 then return
    // The reason why the twist calculations aren't symmetric is that the
    // allowed ratios for normal and reverse twist are different.
    if(T[Column] < ((T[Row] >> 1) - (T[Row] >> 3))) return ' ';

    // N.B. looks like avoiding a divide by zero.
    for(ii = 0; ii < COEFF_NUMBER; ii++)
        if(T[ii] == 0)
            T[ii] = 1;

    //If relations max row and max column to all other tones are less then
    //threshold then return
    // Check for the presence of strong harmonics.
    for(ii = 10; ii < COEFF_NUMBER; ii ++)
    {
        if(T[Row]/T[ii] < dialTonesToOhersTones)
            return ' ';
        if(T[Column]/T[ii] < dialTonesToOhersTones)
            return ' ';
    }


    //If relations max row and max column tones to other dial tones are
    //less then threshold then return
    for(ii = 0; ii < 10; ii ++)
    {
        // TODO:
        // The next two nested if's can be collapsed into a single
        // if-statement.  Basically, he's checking that the current
        // tone is NOT the maximum tone.
        //
        // A simpler check would have been (ii != Column && ii != Row)
        //
        if(T[ii] != T[Column])
        {
            if(T[ii] != T[Row])
            {
                if(T[Row]/T[ii] < dialTonesToOhersDialTones)
                    return ' ';
                if(Column != 4)
                {
                    // Column == 4 corresponds to 1176Hz.
                    // TODO: what is so special about this frequency?
                    if(T[Column]/T[ii] < dialTonesToOhersDialTones)
                        return ' ';
                }
                else
                {
                    if(T[Column]/T[ii] < (dialTonesToOhersDialTones/3))
                        return ' ';
                }
            }
        }
    }

    //We are choosed a push button
    // Determine the tone based on the row and column frequencies.
    switch (Row)
    {
    case 0:
        switch (Column)
        {
        case 4:
            return_value='1';
            break;
        case 5:
            return_value='2';
            break;
        case 6:
            return_value='3';
            break;
        case 7:
            return_value='A';
            break;
        };
        break;
    case 1:
        switch (Column)
        {
        case 4:
            return_value='4';
            break;
        case 5:
            return_value='5';
            break;
        case 6:
            return_value='6';
            break;
        case 7:
            return_value='B';
            break;
        };
        break;
    case 2:
        switch (Column)
        {
        case 4:
            return_value='7';
            break;
        case 5:
            return_value='8';
            break;
        case 6:
            return_value='9';
            break;
        case 7:
            return_value='C';
            break;
        };
        break;
    case 3:
        switch (Column)
        {
        case 4:
            return_value='*';
            break;
        case 5:
            return_value='0';
            break;
        case 6:
            return_value='#';
            break;
        case 7:
            return_value='D';
            break;
        }
    }

    return return_value;
}
