//
//  main.cpp
//  read_image_itk
//
//  Created by Ted NIVAN on 4/28/16.
//  Copyright © 2016 Ted NIVAN. All rights reserved.
//  Motion Artefact Index Algorithm
//

#include "Ted_Motion_Library.hpp"

/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
/* MAIN FUNCTION */
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    string name;
    FloatImageType::Pointer image;
    Tinfo imageInfo;
    int *max = NULL; int slices;
    int threshold;
    
    if(argc < 3)
    {
        displayHelp();
    }
    else
    {
	stringstream ss(argv[1]);
	ss >> threshold;
        //threshold = stoi(argv[1]);
        
        for(int i=2; i<argc; i++)
        {
            loadImage(string(argv[i]), &imageInfo, &image);
            
            slices = imageInfo.zSize;
            max = new int[slices];
            
            maxSlices(max, &imageInfo, image);
            
            analyzeSlices(max, threshold, &imageInfo, image, argv[i]);
            
            delete [] max; max = NULL;
        }
        
    }
    return EXIT_SUCCESS;
}




