/** Author:       Plyashkevich Viatcheslav <plyashkevich@yandex.ru> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * All rights reserved. 
 */


#include <stdio.h>
#include "DtmfDetector.hpp"
#include "DtmfGenerator.hpp"

const unsigned FRAME_SIZE = 160;

char dialButtons[16];
short int samples[100000];

DtmfDetector dtmfDetector(FRAME_SIZE);
DtmfGenerator dtmfGenerator(FRAME_SIZE, 40, 20);

int main()
{ 
	dialButtons[0] = '1';
	dialButtons[1] = '2';
	dialButtons[2] = '3';
	dialButtons[3] = 'A';
	dialButtons[4] = '4';
	dialButtons[5] = '5';
	dialButtons[6] = '6';
	dialButtons[7] = 'B';
	dialButtons[8] = '7';
	dialButtons[9] = '8';
	dialButtons[10] = '9';
	dialButtons[11] = 'C';
	dialButtons[12] = '*';
	dialButtons[13] = '0';
	dialButtons[14] = '#';
	dialButtons[15] = 'D';
   

	while(true)
	{
		static int framenumber = 0;
		++framenumber;	
		dtmfGenerator.dtmfGeneratorReset();
		dtmfDetector.zerosIndexDialButton();
		dtmfGenerator.transmitNewDialButtonsArray(dialButtons, 16);
		while(!dtmfGenerator.getReadyFlag())
		{
			dtmfGenerator.dtmfGenerating(samples); // Generating of a 16 bit's little-endian's pcm samples array
			dtmfDetector.dtmfDetecting(samples); // Detecting from 16 bit's little-endian's pcm samples array
		}

		if(dtmfDetector.getIndexDialButtons() != 16)
		{
			printf("Error of a number of detecting buttons \n");
			continue;
		}
		
		for(int ii = 0; ii < dtmfDetector.getIndexDialButtons(); ++ii)
		{
			if(dtmfDetector.getDialButtonsArray()[ii] != dialButtons[ii])
			{
				printf("Error of a detecting button \n");
				continue;
			}
		}		
		printf("Success in frame: %d \n", framenumber);
	}
 
  return 0;
}
