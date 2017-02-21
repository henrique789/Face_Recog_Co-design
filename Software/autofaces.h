#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core.hpp"
#include "acp.h"
#include <set>
#include <chrono>
#include <iostream>
#include <memory.h>

#include "PCIE.h"

#ifndef AUTOFACES_H
#define AUTOFACES_H

using namespace cv;
using namespace std;
using namespace std::chrono;

class Autofaces : public FaceRecognizer
{
private:
    int _num_components;
    double _threshold;
    vector<Mat> _projections;
    Mat _labels;
    Mat _eigenvectors;
    Mat _eigenvalues;
    Mat _mean;

public:
    //Autofaces();

    Mat getMean();
    Mat getEigenVectors();
    Mat getEigenValues();

    Mat getProjections();

    // Initializes an empty Eigenfaces model.
    Autofaces(int num_components = 0, double threshold = DBL_MAX) :
        _num_components(num_components),
        _threshold(threshold) {}

    // Initializes and computes an Eigenfaces model with images in src and
    // corresponding labels in labels. num_components will be kept for
    // classification.
    Autofaces(InputArrayOfArrays src, InputArray labels, int num_components = 0, double threshold = DBL_MAX) :
        _num_components(num_components),
        _threshold(threshold)
        {
        	//train(hPCIe, src, labels);
        	train(src, labels);
        }

    // Computes an Eigenfaces model with images in src and corresponding labels
    // in labels.
    void train(PCIE_HANDLE hPCIe, InputArrayOfArrays src, InputArray labels);
    void train(InputArrayOfArrays src, InputArray labels);

    void FPGAsubspaceProject(PCIE_HANDLE hPCIe, Mat src, Mat mean, Mat egvec);

    // Predicts the label of a query image in src.
    int predict(InputArray src) const;

    // Predicts the label and confidence for a given sample.
    void predict(InputArray _src, int &label, double &dist) const;

    // See FaceRecognizer::load.
    void load(const FileStorage& fs);

    // See FaceRecognizer::save.
    void save(FileStorage& fs) const;

    BOOL ReadProjsDMA(PCIE_HANDLE hPCIe, Mat src);
    BOOL ReadProjsDMA2(PCIE_HANDLE hPCIe, Mat src);
};

#endif // AUTOFACES_H
