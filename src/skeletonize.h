#pragma once

#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/vertex_position_geometry.h"

#include "polyscope/curve_network.h"

using namespace geometrycentral;
using namespace geometrycentral::surface;

// extract a skeleton from the input mesh using CGAL's
// Triangulated Surface Mesh Skeletonization module
// https://doc.cgal.org/latest/Surface_mesh_skeletonization/index.html
std::vector<std::vector<Vector3>> skeletonize(ManifoldSurfaceMesh& mesh,
                                              VertexPositionGeometry& geom);

// draw a skeleton on screen using Polyscope
polyscope::CurveNetwork*
drawSkeleton(const std::vector<std::vector<Vector3>>& skeleton,
             std::string name = "skeleton");
