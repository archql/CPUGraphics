#ifndef TEXINFO_H
#define TEXINFO_H

#include "vec3.h"

#include <QImage>

struct TexInfo {
    QImage tDiffuse;
    QImage tNormal;
    QImage tBump;
    Math::Vec3 tColor;
};

#endif // TEXINFO_H
