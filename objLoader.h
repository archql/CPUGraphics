#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "vec3.h"

#include <QFile>


bool loadOBJ(
    QFile objFile,
    QVector < Math::Vec3 > &out_vertices,
    QVector < QVector<std::tuple<int, int, int>> > &out_indices, // TODO test
    QVector < Math::Vec3 > &out_normals,
    QVector < Math::Vec3 > &out_colors,
    QVector < Math::Vec3 > &out_textures
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
            if (lineParts.length() > 4) { // color
                out_colors.append({
                    lineParts.at(4).toFloat(),
                    lineParts.at(5).toFloat(),
                    lineParts.at(6).toFloat()
                });
            }
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
        else if (!first.compare("vt", Qt::CaseInsensitive))
        {
            // its a tex
            switch (lineParts.size()) {
            case 2: out_textures.append({
                    lineParts.at(1).toFloat(),
                    0,
                    0
                }); break;
            case 3: out_textures.append({
                    lineParts.at(1).toFloat(),
                    lineParts.at(2).toFloat(),
                    0
                }); break;
            }
        }
        else if (!first.compare("f", Qt::CaseInsensitive))
        {
            // read faces TODO test
            QVector<std::tuple<int, int, int>> indexes;
            for (size_t i = 1; i < lineParts.length(); i++) {
                auto polydata = lineParts.at(i).split('/');
                int iv = polydata.at(0).toInt() - 1;
                int it = iv;
                if (polydata.size() >= 1) {
                    it = polydata.at(1).toInt() - 1;
                }
                int in = iv;
                if (polydata.size() >= 2) {
                    in = polydata.at(2).toInt() - 1;
                }
                indexes.append(std::make_tuple(iv, in, it));
            }
            out_indices.append(indexes);
//            if (indexes.length() > 3) {
//                out_indices.append({indexes[0], indexes[1], indexes[2]});
//                out_indices.append({indexes[2], indexes[3], indexes[0]});
//            } else {
//                out_indices.append(indexes);
//            }


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
