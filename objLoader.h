#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "vec3.h"
#include "texinfo.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QImage>


bool loadOBJ(
    QFile objFile,
    QVector < Math::Vec3 > &out_vertices,
    QVector < QVector<std::tuple<int, int, int>> > &out_indices, // TODO test
    QVector < Math::Vec3 > &out_normals,
    QVector < Math::Vec3 > &out_colors,
    QVector < Math::Vec3 > &out_textures,
    QVector < int > &out_texIDs,
    QVector < TexInfo > &out_texture
    )
{
    QMap<QString, TexInfo> textures;
    QFile mtlFile;
    int texId = 0;
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
            case 4: out_textures.append({
                    lineParts.at(1).toFloat(),
                    lineParts.at(2).toFloat(),
                    lineParts.at(3).toFloat()
                }); break;
            }
            out_texIDs.append(texId);
        }
        else if (!first.compare("mtllib", Qt::CaseInsensitive))
        {
            // open file named lineParts.at(1)
            QDir path = QFileInfo(objFile).absoluteDir();
            QFile mtlFile(path.filePath(lineParts.at(1)));
            //
            if (!mtlFile.open(QFile::ReadOnly | QFile::Text))
            {
                qWarning() << "mtl file specified is not found!";
                return false;
            }
            QString name;
            QString texDiffusePath, texNormalPath, texBumpPath, texBloomPath;
            Math::Vec3 defaultColor{1, 1, 1};
            //
            while (!mtlFile.atEnd()) {
                QString line = mtlFile.readLine().trimmed();
                auto lineParts = line.split(' ');
                auto first = lineParts.at(0);
                if (!first.compare("newmtl", Qt::CaseInsensitive)) {
                    // dump all received files
                    if (name.length() > 0) {
                        textures[name] = {QImage(texDiffusePath),
                                          QImage(texNormalPath),
                                          QImage(texBumpPath),
                                          QImage(texBloomPath),
                                          defaultColor};
                        //
                        texDiffusePath.clear();
                    }
                    // new file decl
                    name = lineParts.at(1);
                    qInfo() << "name " << name;
                } else if (!first.compare("map_Kd")) {
                    // diffuse color
                    texDiffusePath = path.filePath(lineParts.at(1));
                    qInfo() << "texDiffusePath " << texDiffusePath;
                } else if (!first.compare("map_Ke") || !first.compare("map_bump")) {
                    // diffuse color
                    texBumpPath = path.filePath(lineParts.at(1));
                    qInfo() << "texBumpPath " << texBumpPath;
                } else if (!first.compare("norm")) {
                    // normals color
                    texNormalPath = path.filePath(lineParts.at(1));
                    qInfo() << "texNormalPath " << texNormalPath;
                } else if (!first.compare("map_MRAO")) {
                    texBloomPath = path.filePath(lineParts.at(1));
                    qInfo() << "texBloomPath " << texBloomPath;
                } else if (!first.compare("Kd")) {
                    // its a color
                    defaultColor = Math::Vec3{
                        lineParts.at(1).toFloat(),
                        lineParts.at(2).toFloat(),
                        lineParts.at(3).toFloat()
                    };
                }
            }
            // dump all received files (DUPLICATE)
            if (name.length() > 0) {
                textures[name] = {QImage(texDiffusePath),
                                  QImage(texNormalPath),
                                  QImage(texBumpPath)};
            } else {
                qWarning() << "Empty mtl!";
            }
        }
        else if (!first.compare("usemtl", Qt::CaseInsensitive))
        {
            if (textures.value(lineParts.at(1)).tDiffuse.isNull()) {
                qWarning() << "mtl texture specified is not found in mtl file!";
                //return false;
            }
            texId++;
            out_texture.append(textures.value(lineParts.at(1)));
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
