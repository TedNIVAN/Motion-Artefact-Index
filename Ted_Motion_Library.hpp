//
//  main.cpp
//  read_image_itk
//
//  Created by Ted NIVAN on 4/28/16.
//  Copyright © 2016 Ted NIVAN. All rights reserved.
//  Motion Artefact Index Algorithm
//

#ifndef Ted_Motion_Library_hpp
#define Ted_Motion_Library_hpp

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageSliceIteratorWithIndex.h"
#include "itkImageDuplicator.h"
#include "itkStatisticsImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#define MARGINBOUNDARIES 5
#define MARGINSLICES 0

using namespace std;
using namespace itk;

//Image definition
const unsigned int ImageSpatialDimension = 3;
typedef float FloatPixelType;
typedef Image<FloatPixelType, ImageSpatialDimension> FloatImageType;

//File I/O
typedef ImageFileReader<FloatImageType> ReaderType;
typedef ImageFileWriter<FloatImageType> WriterType;

//Image iterator
typedef ImageSliceIteratorWithIndex<FloatImageType> SliceIteratorType;

//General filters
typedef ImageDuplicator<FloatImageType> DuplicatorType;
typedef StatisticsImageFilter<FloatImageType> StatisticsImageFilterType;
typedef RegionOfInterestImageFilter< FloatImageType, FloatImageType> FilterType;
typedef BinaryThresholdImageFilter <FloatImageType, FloatImageType> BinaryThresholdImageFilterType;

/*--------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------*/

typedef struct info{
    float min, max, mean, std;
    int xSize, ySize, zSize;
    string iD;
} Tinfo;

typedef struct bound{
    int xmin, xmax, ymin, ymax;
} Tbound;

/*--------------------------------------------------------*/
/* BASIC FUNCTIONS */
/*--------------------------------------------------------*/

/* Help. */
void displayHelp();

/* Get the statistical information. */
void getInfo(Tinfo *imageInfo, string input, FloatImageType::Pointer image);

/* Display the image's features. */
void showInfo(Tinfo imageInfo);

/* Load the input image into the
 pipeline and create a sheet feature. */
void loadImage(string input, Tinfo *imageInfo, FloatImageType::Pointer *image);

/* Save the output image into the working folder. */
void saveImage(string output, Tinfo *imageInfo, FloatImageType::Pointer image);

/* Dupplicate an image. */
void duplicateImage(FloatImageType::Pointer image, FloatImageType::Pointer *duplicated);

//  EXAMPLE:
//      -ImageType::Pointer clonedImage;
//      -duplicateImage(image, &clonedImage);

/* Mean ROI. */
void meanROI(float *ROI, int numberOfElementsROI, float *mean);

/* Max Slices. */
void maxSlices(int *max, Tinfo *imageInfo, FloatImageType::Pointer image);

/* Get MAI. */
void getMAI(float sumOfElementsVolumeROIa, float sumOfElementsVolumeROIb, int numberOfElementsVolumeROIa, int numberOfElementsVolumeROIb, float *MAI);

/* Write MAI into a json file. */
void writeJson(ofstream *myFile, string name, float MAI);

/*--------------------------------------------------------*/
/* TRANSFORMATION FUNCTIONS */
/*--------------------------------------------------------*/

/* Threshold the image. */
void thresholdImage(Tinfo imageInfo, FloatImageType::Pointer image, FloatImageType::Pointer *thresholded, float lowerThreshold, float upperThreshold);

/* Get Boundaries. */
void getBoundaries(Tbound *boundaries, Tinfo imageInfo, FloatImageType::Pointer maskImage);

/* Get ROIs. */
void getROIs(float *ROIa, float *ROIb, int numberOfElementsROIa, int numberOfElementsROIb, Tbound boundaries, int margin, Tinfo imageInfo, FloatImageType::Pointer image);

/* Merge ROIs. */
int mergeROIs(int R1, int R2);

/* Analyze Slices. */
void analyzeSlices(int *max, int threshold, Tinfo *imageInfo, FloatImageType::Pointer image);

/* Sum of elements of a vector. */
float sumVector(int size, float *myVector);

/*--------------------------------------------------------*/
/* TEMPLATED FUNCTIONS */
/*--------------------------------------------------------*/

template <class T>
void showVector(string name, int size, T myVector)
{
    cout << endl;
    for(int index=0; index<size; index++)
    {
        cout << "index: "<< index << " || " + name + ": " << myVector[index] << endl;
    }
    cout << endl;
}

template <class T1, class T2>
void setVector(int size, T1 *myVector, T2 value)
{
    for(int index=0; index<size; index++)
    {
        myVector[index] = (T1) value;
    }
}

#endif /* Ted_Motion_Library_hpp */
