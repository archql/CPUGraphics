#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "vec3.h"

#include <QFile>


bool loadOBJ(
    QFile objFile,
    QVector < Math::Vec3 > &out_vertices,
    QVector < QPair<size_t, size_t> > &out_indices, // TODO test
    QVector < Math::Vec3 > &out_normals
    )
{
    if (!objFile.open(QFile::ReadOnly | QFile::Text))
    {
        return false;
    }
    while (!objFile.atEnd())
    {
        QString line = objFile.readLine().trimmed();
        auto lineParts = line.split(' ');
        auto first = lineParts.at(0);
        if (!first.compare("v", Qt::CaseInsensitive)) {
            // its a vertex
            out_vertices.append({
                lineParts.at(1).toDouble(),
                lineParts.at(2).toDouble(),
                lineParts.at(3).toDouble()
            });
        }
        else if (!first.compare("vn", Qt::CaseInsensitive))
        {
            // its a normal
            out_normals.append({
                lineParts.at(1).toDouble(),
                lineParts.at(2).toDouble(),
                lineParts.at(3).toDouble()
            });
        }
        else if (!first.compare("f", Qt::CaseInsensitive))
        {
            // read faces TODO test
            auto polydata = lineParts.at(1).split('/');
            size_t first_index = polydata.at(0).toUInt() - 1; // start from zero
            size_t last_index = first_index;
            for (size_t i = 2; i < lineParts.length(); i++) {
                auto polydata = lineParts.at(i).split('/');
                size_t index = polydata.at(0).toUInt() - 1;
                out_indices.append({last_index, index});
                last_index = index;
            }
        }
        // Ignore others for now
    }
    return true;
}

#endif // OBJLOADER_H
