#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/meshio.h"
#include "geometrycentral/surface/simple_idt.h"
#include "geometrycentral/surface/vertex_position_geometry.h"

#include "polyscope/curve_network.h"
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"

#include "args/args.hxx"

#include "skeletonize.h"

using namespace geometrycentral;
using namespace geometrycentral::surface;

// == Geometry-central data for performing computations on
std::unique_ptr<ManifoldSurfaceMesh> inputMesh;
std::unique_ptr<VertexPositionGeometry> inputGeom;

// == Polyscope visualization handles
polyscope::SurfaceMesh* psMesh;
polyscope::CurveNetwork* psSkeleton;

// function to compute the mesh's skeleton and display it
void computeSkeleton(ManifoldSurfaceMesh& mesh, VertexPositionGeometry& geom) {
    std::vector<std::vector<Vector3>> skeleton = skeletonize(mesh, geom);
    psSkeleton                                 = drawSkeleton(skeleton);

    psMesh->setTransparency(.8);
}

// A user-defined callback, for creating control panels (etc)
// Use ImGUI commands to build whatever you want here, see
// https://github.com/ocornut/imgui/blob/master/imgui.h
void myCallback() {
    if (ImGui::Button("Compute Skeleton")) {
        computeSkeleton(*inputMesh, *inputGeom);
    }
}

int main(int argc, char** argv) {
    polyscope::options::programName = "Mesh Skeletonization";
    // Configure the argument parser
    args::ArgumentParser parser("Mesh Skeletonization");
    args::Positional<std::string> inputFilename(parser, "mesh",
                                                "Mesh to be processed.");
    args::HelpFlag help(parser, "help", "Display this help menu",
                        {'h', "help"});

    // Parse args
    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    if (!inputFilename) { // if no input filename was given, complain and quit
        std::cout << "ERROR: you must provide an input mesh to skeletonize"
                  << std::endl;
        std::cout << parser; // print out help message
        return 1;
    }

    std::string filename = args::get(inputFilename);

    // Initialize polyscope
    polyscope::init();

    // Set the callback function
    polyscope::state::userCallback = myCallback;

    // Load mesh
    std::tie(inputMesh, inputGeom) = readManifoldSurfaceMesh(filename);

    // Register the mesh with polyscope
    psMesh = polyscope::registerSurfaceMesh(
        polyscope::guessNiceNameFromPath(filename), inputGeom->vertexPositions,
        inputMesh->getFaceVertexList(), polyscopePermutations(*inputMesh));

    // Give control to the polyscope gui
    polyscope::show();

    return EXIT_SUCCESS;
}
