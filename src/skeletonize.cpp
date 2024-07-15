#include "skeletonize.h"

//===== CGAL configuration code
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/boost/graph/split_graph_into_polylines.h>
#include <CGAL/extract_mean_curvature_flow_skeleton.h>
#include <fstream>
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef boost::graph_traits<Polyhedron>::vertex_descriptor vertex_descriptor;
typedef CGAL::Mean_curvature_flow_skeletonization<Polyhedron> Skeletonization;
typedef Skeletonization::Skeleton Skeleton;
typedef Skeleton::vertex_descriptor Skeleton_vertex;
typedef Skeleton::edge_descriptor Skeleton_edge;

//====== Helper functions to convert between types
// convert geometry-central mesh to CGAL Polyhedron_3
Polyhedron toCGAL(ManifoldSurfaceMesh& mesh, VertexPositionGeometry& geometry) {
    Polyhedron P;
    CGAL::Polyhedron_incremental_builder_3<Polyhedron::HalfedgeDS> builder(
        P.hds());

    builder.begin_surface(mesh.nVertices(), mesh.nFaces());

    // Add vertices
    for (Vertex v : mesh.vertices()) {
        Vector3 pos = geometry.vertexPositions[v];
        builder.add_vertex(Kernel::Point_3(pos.x, pos.y, pos.z));
    }

    // Add faces
    for (Face f : mesh.faces()) {
        builder.begin_facet();
        for (Vertex v : f.adjacentVertices()) {
            builder.add_vertex_to_facet(v.getIndex());
        }
        builder.end_facet();
    }

    builder.end_surface();

    return P;
}

// helper to partition skeleton into polylines
struct list_polylines {
    const Skeleton& skeleton;
    std::vector<std::vector<Vector3>>& polylines;
    list_polylines(const Skeleton& skeleton,
                   std::vector<std::vector<Vector3>>& polylines)
        : skeleton(skeleton), polylines(polylines) {}
    void start_new_polyline() { polylines.push_back(std::vector<Vector3>{}); }
    void add_node(Skeleton_vertex v) {
        Point p = skeleton[v].point;
        polylines.back().push_back(Vector3{p.x(), p.y(), p.z()});
    }
    void end_polyline() {}
};

//====== Important functions
// extract a skeleton from the input mesh using CGAL's
// Triangulated Surface Mesh Skeletonization module
// https://doc.cgal.org/latest/Surface_mesh_skeletonization/index.html
std::vector<std::vector<Vector3>> skeletonize(ManifoldSurfaceMesh& mesh,
                                              VertexPositionGeometry& geom) {
    // convert mesh to CGAL's data structure
    Polyhedron tmesh = toCGAL(mesh, geom);

    Skeleton skeleton; // run CGAL's algorithm to compute skeleton
    CGAL::extract_mean_curvature_flow_skeleton(tmesh, skeleton);

    // express skeleton as a list of polylines
    std::vector<std::vector<Vector3>> result;
    list_polylines helper(skeleton, result);
    CGAL::split_graph_into_polylines(skeleton, helper);
    return result;
}

polyscope::CurveNetwork*
drawSkeleton(const std::vector<std::vector<Vector3>>& skeleton,
             std::string name) {
    std::vector<Vector3> vertices;
    std::vector<std::array<size_t, 2>> edges;

    size_t vIndex = 0;
    for (const std::vector<Vector3>& polyline : skeleton) {
        for (size_t iP = 0; iP < polyline.size(); iP++) {
            vertices.push_back(polyline[iP]);
            if (iP + 1 < polyline.size()) {
                edges.push_back(std::array<size_t, 2>{vIndex, vIndex + 1});
            }
            vIndex++;
        }
    }

    return polyscope::registerCurveNetwork(name, vertices, edges);
}
