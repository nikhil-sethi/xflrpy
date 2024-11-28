#pragma once

#include <xflgeom/geom2d/vector2d.h>
#include <xflgeom/geom3d/triangle3d.h>

void makeSphere(double radius, int nSplit, QVector<Triangle3d> &triangles);

bool intersectSegment(Vector2d const &A0, Vector2d const &A1,
                      Vector2d const &B0, Vector2d const &B1,
                      Vector2d &IPt, bool bPointsInside, double vtx_precision);

double distanceToLine3d(Vector3d const &A, Vector3d const &B, Vector3d const &P);
