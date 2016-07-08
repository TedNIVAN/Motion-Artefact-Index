//
//  main.cpp
//  read_image_itk
//
//  Created by Ted NIVAN on 4/28/16.
//  Copyright © 2016 Ted NIVAN. All rights reserved.
//  Motion Artefact Index Algorithm
//

#ifndef Ted_Motion_Library_cpp
#define Ted_Motion_Library_cpp

#include "Ted_Motion_Library.hpp"

/*--------------------------------------------------------*/
/* BASIC FUNCTIONS */
/*--------------------------------------------------------*/

/* Help. */
void displayHelp()
{
    cout << "#######################################################" << endl;
    cout << "############## How To Use The Algorithm. ##############" << endl;
    cout << "#######################################################" << endl << endl;
    cout << " ----Command line----" << endl;
    cout << "     ./main <thresholdValue> <image1 image2 ... imageN>" << endl << endl;
    cout << " ----Threshold Value----" << endl;
    cout << "        T1-images: thresholdValue = 15" << endl;
    cout << "        T2-images: thresholdValue = 5" << endl;
    cout << "     FLAIR-images: thresholdValue = 5" << endl << endl;
    cout << " ----Images----" << endl;
    cout << "     image.nii or image.nii.gz" << endl << endl;
    cout << " ----Example for a T1-image----" << endl;
    cout << "     ./main 15 image.nii" << endl << endl;
    cout << " ----Examples for three T1-images----" << endl;
    cout << "     ./main 15 image1.nii image2.nii image3.nii" << endl;
}

/* Get the statistical information. */
void getInfo(Tinfo *imageInfo, string input, FloatImageType::Pointer image)
{
    FloatImageType::RegionType region = image->GetLargestPossibleRegion();
    FloatImageType::SizeType size = region.GetSize();
    
    imageInfo->iD= input;
    
    imageInfo->xSize = size[0];
    imageInfo->zSize = size[1];
    imageInfo->ySize = size[2];
    
    StatisticsImageFilterType::Pointer statisticsImageFilter = StatisticsImageFilterType::New();
    statisticsImageFilter->SetInput(image);
    statisticsImageFilter->Update();
    
    imageInfo->min = (float) statisticsImageFilter->GetMinimum();
    imageInfo->max = (float) statisticsImageFilter->GetMaximum();
    imageInfo->mean = (float) statisticsImageFilter->GetMean();
    imageInfo->std = (float) statisticsImageFilter->GetSigma();
}

/* Display the image's features. */
void showInfo(Tinfo imageInfo)
{
    cout << endl;
    cout << "iD: " << imageInfo.iD << endl;
}

/* Load the input image into the
 pipeline and create a sheet feature. */
void loadImage(string input, Tinfo *imageInfo, FloatImageType::Pointer *image)
{
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(input);
    reader->Update();
    
    *image = reader->GetOutput();
    
    getInfo(&(*imageInfo), input, *image);
}

/* Save the output image into the working folder. */
void saveImage(string output, Tinfo *imageInfo, FloatImageType::Pointer image)
{
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(output);
    writer->SetInput(image);
    writer->Update();
    
    getInfo(&(*imageInfo), output, image);
}

/* Dupplicate an image. */
void duplicateImage(FloatImageType::Pointer image, FloatImageType::Pointer *duplicated)
{
    DuplicatorType::Pointer duplicator;
    duplicator = DuplicatorType::New();
    duplicator->SetInputImage(image);
    duplicator->Update();
    
    *duplicated = duplicator->GetOutput();
}

/* Mean ROI. */
void meanROI(float *ROI, int numberOfElementsROI, float *mean)
{
    for(int i = 0; i < numberOfElementsROI; i++)
    {
        *mean += ROI[i];
    }
    
    *mean /= numberOfElementsROI;
}

/* Max Slices. */
void maxSlices(int *max, Tinfo *imageInfo, FloatImageType::Pointer image)
{
    int slices = imageInfo->zSize, slice;
    
    for(int slice=0; slice<slices; slice++)
    {
        max[slice] = 0;
    }
    
    FloatImageType::IndexType start;
    start[0] = 0; //x
    start[1] = 0; //z
    start[2] = 0; //y
    
    FloatImageType::IndexType end;
    end[0] = imageInfo->xSize - 1;
    end[1] = 0;
    end[2] = imageInfo->ySize - 1;
    
    FloatImageType::RegionType region;
    FloatImageType::IndexType position;
    
    FloatImageType::Pointer duplicated;
    duplicateImage(image, &duplicated);
    
    SliceIteratorType it(duplicated, duplicated->GetRequestedRegion());
    it.SetFirstDirection(2);
    it.SetSecondDirection(0);
    
    it.GoToBegin();
    
    while(!it.IsAtEnd())
    {
        position = it.GetIndex();
        slice = (int) position[1];
        
        start[1] = slice;
        end[1] = slice;
        
        region.SetIndex(start);
        region.SetUpperIndex(end);
        
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(duplicated);
        filter->SetRegionOfInterest(region);
        
        StatisticsImageFilterType::Pointer statisticsImageFilter = StatisticsImageFilterType::New();
        statisticsImageFilter->SetInput(filter->GetOutput());
        statisticsImageFilter->Update();
        
        max[slice] = (float) statisticsImageFilter->GetMaximum();
        
        it.NextSlice();
    }
}

/* Get MAI. */
void getMAI(float sumOfElementsVolumeROIa, float sumOfElementsVolumeROIb, int numberOfElementsVolumeROIa, int numberOfElementsVolumeROIb, float *MAI)
{
    float meanROIa = sumOfElementsVolumeROIa/numberOfElementsVolumeROIa;
    float meanROIb = sumOfElementsVolumeROIb/numberOfElementsVolumeROIb;
    
    *MAI = abs((meanROIb/meanROIa) - 1);
}

/* Write MAI into a json file. */
void writeJson(ofstream *myFile, string name, float MAI)
{
    myFile->open(name);
    *myFile << "{" << endl;
    *myFile << "  \"mai\": " + to_string(MAI) << endl;
    *myFile << "}" << endl;
    myFile->close();
}

/* Threshold the image. */
void thresholdImage(Tinfo imageInfo, FloatImageType::Pointer image, FloatImageType::Pointer *thresholded, float lowerThreshold, float upperThreshold)
{
    BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(image);
    thresholdFilter->SetLowerThreshold(lowerThreshold);
    thresholdFilter->SetUpperThreshold(upperThreshold);
    thresholdFilter->SetInsideValue(1);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->Update();
    
    *thresholded = thresholdFilter->GetOutput();
}

/* Get Boundaries. */
void getBoundaries(Tbound *boundaries, Tinfo imageInfo, FloatImageType::Pointer maskImage)
{
    int x, y;
    int xmin, xmax, ymin, ymax;
    
    FloatImageType::IndexType start;
    start[0] = 0; //x
    start[1] = 0; //z
    start[2] = 0; //y
    
    FloatImageType::IndexType end;
    end[0] = imageInfo.xSize - 1; //endX
    end[1] = 0;                   //endZ
    end[2] = imageInfo.ySize - 1; //endY
    
    FloatImageType::RegionType region;
    FloatImageType::IndexType position;
    
    SliceIteratorType it(maskImage, maskImage->GetRequestedRegion());
    
    //Y-BOUNDARIES
    it.SetFirstDirection(0);
    it.SetSecondDirection(2);
    
    //ymin-ymax
    ymin = end[2]; ymax = 0;
    it.GoToBegin();
    
    while (!it.IsAtEndOfSlice())
    {
        while(!it.IsAtEndOfLine())
        {
            position = it.GetIndex();

            y = (int) position[2];
            
            if (it.Get() == 0)
            {
                if (y < ymin)
                {
                    ymin = y;
                }
                else if (y > ymax)
                {
                    ymax = y;
                }
            }
            ++it;
        }
        it.NextLine();
    }
    
    //X-BOUNDARIES
    it.SetFirstDirection(2);
    it.SetSecondDirection(0);
    
    //xmin-xmax
    xmin = end[0]; xmax = 0;
    it.GoToBegin();
    
    while (!it.IsAtEndOfSlice())
    {
        while(!it.IsAtEndOfLine())
        {
            position = it.GetIndex();
            
            x = (int) position[0];
            
            if (it.Get() == 0)
            {
                if (x < xmin)
                {
                    xmin = x;
                }
                else if (x > xmax)
                {
                    xmax = x;
                }
            }
            ++it;
        }
        it.NextLine();
    }
    
    boundaries->xmin = xmin; boundaries->xmax = xmax;
    boundaries->ymin = ymin; boundaries->ymax = ymax;
}

/* Get ROIs. */
void getROIs(float *ROIa, float *ROIb, int numberOfElementsROIa, int numberOfElementsROIb, Tbound boundaries, int margin, Tinfo imageInfo, FloatImageType::Pointer image)
{
    int x, y;
    int xmin = boundaries.xmin, xmax = boundaries.xmax;
    int ymin = boundaries.ymin, ymax = boundaries.ymax;
    
    FloatImageType::IndexType start;
    start[0] = 0; //x
    start[1] = 0; //z
    start[2] = 0; //y
    
    FloatImageType::IndexType end;
    end[0] = imageInfo.xSize - 1; //endX
    end[1] = 0;                   //endZ
    end[2] = imageInfo.ySize - 1; //endY
    
    FloatImageType::RegionType region;
    FloatImageType::IndexType position;
    
    SliceIteratorType it(image, image->GetRequestedRegion());
    
    it.SetFirstDirection(0);
    it.SetSecondDirection(2);
    
    it.GoToBegin();
    
    while (!it.IsAtEndOfSlice())
    {
        while(!it.IsAtEndOfLine())
        {
            position = it.GetIndex();
            
            x = (int) position[0];
            y = (int) position[2];
            
            //ROIa1
            if ( ((xmin + margin <= x) && (x <= xmax - margin)) && ((margin <= y) && (y <= ymin - margin)) )
            {
                ROIa[numberOfElementsROIa] = it.Get();
                numberOfElementsROIa--;
            }
            //ROIb1
            else if ( ((margin <= x) && (x <= xmin - margin)) && ((ymin + margin <= y) && (y <= ymax - margin)) )
            {
                ROIb[numberOfElementsROIb] = it.Get();
                numberOfElementsROIb--;
            }
            //ROIa2
            else if ( ((xmin + margin <= x) && (x <= xmax - margin)) && ((ymax + margin <= y) && (y <= end[2] - margin)) )
            {
                ROIa[numberOfElementsROIa] = it.Get();
                numberOfElementsROIa--;
            }
            //ROIb2
            else if ( ((xmax + margin <= x) && (x <= end[0] - margin)) && ((ymin + margin <= y) && (y <= ymax - margin)) )
            {
                ROIb[numberOfElementsROIb] = it.Get();
                numberOfElementsROIb--;
            }
            ++it;
        }
        it.NextLine();
    }
}

/* Sum of elements of a vector. */
float sumVector(int size, float *myVector)
{
    float result = 0.0;
    
    for(int index=0; index<size; index++)
    {
        result += myVector[index];
    }
    
    return result;
}

/* Merge ROIs. */
int mergeROIs(int R1, int R2)
{
    int result =  0;
    
    if (R1 > 0)
    {
        result += R1;
    }
    
    if (R2 > 0)
    {
        result += R2;
    }
    
    return result;
}

/* Analyze Slices. */
void analyzeSlices(int *max, int threshold, Tinfo *imageInfo, FloatImageType::Pointer image)
{
    Tbound boundaries;
    
    int xmin, xmax, ymin, ymax;
    int sliceIndex, sliceMargin = 0;
    int numberOfElementsROIa = 0, numberOfElementsROIb = 0;
    int numberOfElementsVolumeROIa = 0, numberOfElementsVolumeROIb = 0;
    int A1, A2, B1, B2;
    
    float lowerThreshold, upperThreshold;
    float *ROIa = NULL, *ROIb = NULL; int sizeROIs;
    float sumOfElementsVolumeROIa = 0.0, sumOfElementsVolumeROIb = 0.0;
    float MAI;
    
    string fileNameMAI = "mai.json";
    
    ofstream MAIoutput;

    showInfo(*imageInfo);
    
    FloatImageType::IndexType start;
    start[0] = 0; //x
    start[1] = 0; //z
    start[2] = 0; //y
    
    FloatImageType::IndexType end;
    end[0] = imageInfo->xSize - 1; //endX
    end[1] = 0;                    //endZ
    end[2] = imageInfo->ySize - 1; //endY
    
    FloatImageType::RegionType region;
    FloatImageType::IndexType position;
    
    SliceIteratorType it(image, image->GetRequestedRegion());
    it.SetFirstDirection(2);
    it.SetSecondDirection(0);
    
    it.GoToBegin();
    
    while(!it.IsAtEnd())
    {
        position = it.GetIndex();
        sliceIndex = (int) position[1];
        
        start[1] = sliceIndex;
        end[1] = sliceIndex;
        
        region.SetIndex(start);
        region.SetUpperIndex(end);
        
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(image);
        filter->SetRegionOfInterest(region);
        
        /* Threshold Imgae. */
        lowerThreshold = 0;
        upperThreshold = max[sliceIndex]*threshold*0.01;
        
        FloatImageType::Pointer thresholded;
        thresholdImage(*imageInfo, filter->GetOutput(), &thresholded, lowerThreshold, upperThreshold);

        /* Get Boundaries. */
        getBoundaries(&boundaries, *imageInfo, thresholded);
        
        xmin = boundaries.xmin, xmax = boundaries.xmax;
        ymin = boundaries.ymin, ymax = boundaries.ymax;
        
        /* Get ROIs. */
        A1 = (xmax - xmin - 2*MARGINBOUNDARIES + 1)*(ymin - 2*MARGINBOUNDARIES + 1);
        A2 = (xmax - xmin - 2*MARGINBOUNDARIES + 1)*(end[2] - ymax - 2*MARGINBOUNDARIES + 1);
        
        B1 = (xmin - 2*MARGINBOUNDARIES + 1)*(ymax - ymin - 2*MARGINBOUNDARIES + 1);
        B2 = (end[0] - xmax - 2*MARGINBOUNDARIES + 1)*(ymax - ymin - 2*MARGINBOUNDARIES + 1);
        
        numberOfElementsROIa = mergeROIs(A1, A2);
        numberOfElementsROIb = mergeROIs(B1, B2);

        sizeROIs = imageInfo->xSize * imageInfo->ySize;
        
        ROIa = new float[sizeROIs];
        ROIb = new float[sizeROIs];
        
        setVector(sizeROIs, ROIa, 0);
        setVector(sizeROIs, ROIb, 0);
        
        if ((numberOfElementsROIa > 0) && (numberOfElementsROIb > 0))
        {
            sliceMargin++;
            if (sliceMargin > MARGINSLICES)
            {
            getROIs(ROIa, ROIb, numberOfElementsROIa, numberOfElementsROIb, boundaries, MARGINBOUNDARIES, *imageInfo, image);
            
            sumOfElementsVolumeROIa += sumVector(numberOfElementsROIa, ROIa);
            sumOfElementsVolumeROIb += sumVector(numberOfElementsROIb, ROIb);
            
            numberOfElementsVolumeROIa += numberOfElementsROIa;
            numberOfElementsVolumeROIb += numberOfElementsROIb;
            }
        }
        
        numberOfElementsROIa = 0;
        numberOfElementsROIb = 0;
        
        delete [] ROIa; ROIa = NULL;
        delete [] ROIb; ROIb = NULL;
        
        it.NextSlice();
    }

    /* Get MAI. */
    getMAI(sumOfElementsVolumeROIa, sumOfElementsVolumeROIb,numberOfElementsVolumeROIa, numberOfElementsVolumeROIb, &MAI);
    
    /* Write MAI into a json file. */
    writeJson(&MAIoutput, fileNameMAI, MAI);
    
    cout << "   \'" + fileNameMAI + "\' file generated" << endl;
    
}

#endif /* Ted_Motion_Library_cpp */

