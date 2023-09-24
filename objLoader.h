#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "vec3.h"

#include <QFile>


bool loadOBJ(
    QFile objFile,
    QVector < Math::Vec3 > &out_vertices,
    QVector < QVector<int> > &out_indices, // TODO test
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
                lineParts.at(1).toFloat(),
                lineParts.at(2).toFloat(),
                lineParts.at(3).toFloat()
            });
        }
        else if (!first.compare("vn", Qt::CaseInsensitive))
        {
            // its a normal
            out_normals.append({
                lineParts.at(1).toFloat(),
                lineParts.at(2).toFloat(),
                lineParts.at(3).toFloat()
            });
        }
        else if (!first.compare("f", Qt::CaseInsensitive))
        {
            // read faces TODO test
            QVector<int> indexes;
            for (size_t i = 1; i < lineParts.length(); i++) {
                auto polydata = lineParts.at(i).split('/');
                int index = polydata.at(0).toInt() - 1;
                indexes.append(index);
            }
            if (indexes.length() > 3) {
                out_indices.append({indexes[0], indexes[1], indexes[2]});
                out_indices.append({indexes[2], indexes[3], indexes[0]});
            } else {
                out_indices.append(indexes);
            }


//            auto polydata = lineParts.at(1).split('/');
//            int first_index = polydata.at(0).toUInt() - 1; // start from zero
//            int last_index = first_index;
//            for (size_t i = 2; i < lineParts.length(); i++) {
//                auto polydata = lineParts.at(i).split('/');
//                int index = polydata.at(0).toUInt() - 1;
//                out_indices.append({last_index, index});
//                last_index = index;
//            }

        }
        // Ignore others for now
    }
    return true;
}

#endif // OBJLOADER_H
