#pragma once

#include "Amalthea.h"

struct AmaltheaBuffer
{
    EuropaBuffer* buffer;
    uint32 offset;
    EuropaImageFormat format;
};

struct AmaltheaImage
{
    EuropaImage::Ref image;
    EuropaImageView::Ref view;
};

