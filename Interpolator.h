#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include "Source.h"

class Interpolator
{
public:
    Interpolator();

    Source *target_;

    SourceCore from_;
    SourceCore to_;
    SourceCore current_;

    float cursor_;


};

#endif // INTERPOLATOR_H
