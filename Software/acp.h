#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core.hpp"
#include <set>
#include <chrono>
#include <iostream>
#include <memory.h>

#include "PCIE.h"

#ifndef ACP_H
#define ACP_H

using namespace cv;
using namespace std;
using namespace std::chrono;

class ACP {

public:
    Mat mean;
    Mat eigenvectors;
    Mat eigenvalues;

    ACP();
    ACP(InputArray data, InputArray _mean, int flags, int maxComponents);
    ACP(InputArray data, InputArray _mean, int flags, double retainedVariance);
    void project(InputArray _data, OutputArray result) const;
    Mat project(InputArray data) const;
    void backProject(InputArray _data, OutputArray result) const;
    Mat backProject(InputArray data) const;
};

#endif // ACP_H
