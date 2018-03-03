/*###############################################################################################
 *    ShipCAD											*
 *    Copyright 2018, by Greg Green <ggreen@bit-builder.com>					*
 *												*
 *    This program is free software; you can redistribute it and/or modify it under             *
 *    the terms of the GNU General Public License as published by the                           *
 *    Free Software Foundation; either version 2 of the License, or (at your option)            *
 *    any later version.                                                                        *
 *                                                                                              *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY           *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A           *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                  *
 *                                                                                              *
 *    You should have received a copy of the GNU General Public License along with              *
 *    this program; if not, write to the Free Software Foundation, Inc.,                        *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                     *
 *                                                                                              *
 *##############################################################################################*/

#include <QVector3D>

#include "drawfaces.h"
#include "subdivface.h"
#include "subdivpoint.h"
#include "utility.h"
#include "shader.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////


// add face to display list, face is constant color
static void addFaceToDL(QVector<QVector3D>& vertices, QVector<QVector3D>& normals,
                        const QVector3D& p1, const QVector3D& p2, const QVector3D& p3)
{
    QVector3D n = UnifiedNormal(p1, p2, p3);
    vertices << p1 << p2 << p3;
    normals << n << n << n;
}

//////////////////////////////////////////////////////////////////////////////////////

// FreeGeometry.pas:11995
void ShipCAD::DrawFaces(FaceShader* faceshader,
                        const Plane& waterlinePlane,
                        vector<SubdivisionFace*>& faces,
                        size_t& vertices1, size_t& vertices2,
                        bool shadeUnderwater, bool drawMirror,
                        QColor color, QColor underWaterColor)
{
    // make the vertex and color buffers
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector3D> vertices_underwater;
    QVector<QVector3D> normals_underwater;
    // reserve the size of the lists to save memory allocations
    if (vertices1 != 0) {
        vertices.reserve(vertices1);
        normals.reserve(vertices1);
    }
    if (vertices2 != 0) {
        vertices_underwater.reserve(vertices2);
        normals_underwater.reserve(vertices2);
    }

    if (shadeUnderwater) {
        for (size_t i=0; i<faces.size(); ++i) {
            // clip all triangles against the waterline plane
            for (size_t j=2; j<faces[i]->numberOfPoints(); ++j) {
                QVector3D p1 = faces[i]->getPoint(0)->getCoordinate();
                QVector3D p2 = faces[i]->getPoint(j-1)->getCoordinate();
                QVector3D p3 = faces[i]->getPoint(j)->getCoordinate();

                // check if clipping is required
                float min = waterlinePlane.distance(p1);
                float max = min;
                float tmp = waterlinePlane.distance(p2);
                if (tmp < min)
                    min = tmp;
                else if (tmp > max)
                    max = tmp;
                tmp = waterlinePlane.distance(p3);
                if (tmp < min)
                    min = tmp;
                else if (tmp > max)
                    max = tmp;
                if (max <= 0.0) {
                    // entirely below the plane
                    addFaceToDL(vertices_underwater, normals_underwater, p1, p2, p3);
                }
                else if (min >= 0.0) {
                    // entirely above the plane
                    addFaceToDL(vertices, normals, p1, p2, p3);
                }
                else {
                    // pierces water, clip triangle
                    std::vector<QVector3D> above;
                    above.reserve(6);
                    std::vector<QVector3D> below;
                    below.reserve(6);
                    ClipTriangle(p1, p2, p3, waterlinePlane, above, below);
                    for (size_t k=2; k<above.size(); ++k)
                        // shade triangle above
                        addFaceToDL(vertices, normals, above[0], above[k-1], above[k]);
                    for (size_t k=2; k<below.size(); ++k)
                        // shade triangle below
                        addFaceToDL(vertices_underwater, normals_underwater, below[0], below[k-1], below[k]);
                }
                if (drawMirror) {
                    p1.setY(-p1.y());
                    p2.setY(-p2.y());
                    p3.setY(-p3.y());

                    // check if clipping is required
                    float min = waterlinePlane.distance(p1);
                    float max = min;
                    float tmp = waterlinePlane.distance(p2);
                    if (tmp < min)
                        min = tmp;
                    else if (tmp > max)
                        max = tmp;
                    tmp = waterlinePlane.distance(p3);
                    if (tmp < min)
                        min = tmp;
                    else if (tmp > max)
                        max = tmp;
                    if (max <= 0.0) {
                        // entirely below the plane
                        addFaceToDL(vertices_underwater, normals_underwater, p1, p2, p3);
                    }
                    else if (min >= 0.0) {
                        // entirely above the plane
                        addFaceToDL(vertices, normals, p1, p2, p3);
                    }
                    else {
                        // pierces water, clip triangle
                        std::vector<QVector3D> above;
                        above.reserve(6);
                        std::vector<QVector3D> below;
                        below.reserve(6);
                        ClipTriangle(p1, p2, p3, waterlinePlane, above, below);
                        for (size_t k=2; k<above.size(); ++k)
                            // shade triangle above
                            addFaceToDL(vertices, normals, above[0], above[k-1], above[k]);
                        for (size_t k=2; k<below.size(); ++k)
                            // shade triangle below
                            addFaceToDL(vertices_underwater, normals_underwater, below[0], below[k-1], below[k]);
                    }
                }
            }
        }
    } else {
        for (size_t i=0; i<faces.size(); ++i) {
            for (size_t j=2; j<faces[i]->numberOfPoints(); ++j) {
                QVector3D p1 = faces[i]->getPoint(0)->getCoordinate();
                QVector3D p2 = faces[i]->getPoint(j-1)->getCoordinate();
                QVector3D p3 = faces[i]->getPoint(j)->getCoordinate();
                addFaceToDL(vertices, normals, p1, p2, p3);
                if (drawMirror) {
                    p1.setY(-p1.y());
                    p2.setY(-p2.y());
                    p3.setY(-p3.y());
                    addFaceToDL(vertices, normals, p1, p2, p3);
                }
            }
        }
    }
        
    // render above the waterline
    if (vertices.size() > 0) {
        faceshader->renderMesh(color, vertices, normals);
        vertices1 = vertices.size();
    }
    // render below the waterline
    if (vertices_underwater.size() > 0) {
        faceshader->renderMesh(underWaterColor, vertices_underwater, normals_underwater);
        vertices2 = vertices_underwater.size();
    }
}

//////////////////////////////////////////////////////////////////////////////////////

