#include <RiEngine.hpp>
#include <iostream>
#include "spdlog/sinks/basic_file_sink.h"

using namespace rise;
using std::cout, std::endl, std::cerr;

struct NormalVertex {
    friend ostream &operator<<(ostream &os, const NormalVertex &vertex) {
        os << "x: " << vertex.x << " y: " << vertex.y << " z: " << vertex.z << " nx: "
           << vertex.nx << " ny: " << vertex.ny << " nxz: " << vertex.nxz;
        return os;
    }

    float x;
    float y;
    float z;
    float nx;
    float ny;
    float nxz;
};

struct Vertex {
    friend ostream &operator<<(ostream &os, const Vertex &vertex) {
        os << "x: " << vertex.x << " y: " << vertex.y << " z: " << vertex.z;
        return os;
    }

    float x;
    float y;
    float z;
};

void convertNormals() {
    MeshConverter converter;
    converter.addConvertOp({"inPositions", MeshAttribute::Position, Format::R32G32B32Sfloat});
    converter.addConvertOp({"inNormals", MeshAttribute::Normal, Format::R32G32B32Sfloat});
    converter.load("objMeshes/cube.obj", "normalsCube");
    converter.load("objMeshes/sphere.obj", "normalsSphere");
    converter.convert("game/meshes/withNormals");
}

void convertNoNormals() {
    MeshConverter converter;
    converter.addConvertOp({"inPositions", MeshAttribute::Position, Format::R32G32B32Sfloat});
    converter.load("objMeshes/cube.obj", "noNormalsCube");
    converter.load("objMeshes/sphere.obj", "noNormalsSphere");
    converter.convert("game/meshes/noNormals");
}

void convert() {
    convertNormals();
    convertNoNormals();
}

auto load(vector<uint8_t>& vout, vector<uint8_t>& iout) {
    vector<string> meshes;

    meshes.emplace_back("noNormalsCube");
    meshes.emplace_back("noNormalsSphere");
    meshes.emplace_back("normalsCube");
    meshes.emplace_back("normalsSphere");

    MeshImporter importer("game/meshes", meshes);

    vout.resize(importer.sizeForVertices());
    iout.resize(importer.sizeForIndices());

    return importer.load(MemData(vout), MemData(iout));
}


int main() {
    try {

        fs::remove("logs/debug-log.txt");
        spdlog::set_default_logger(spdlog::basic_logger_mt("debug-logger", "logs/debug-log.txt"));

        convert();

        vector<uint8_t> vertices;
        vector<uint8_t> indices;

        auto planner = load(vertices, indices);

        planner.draw("normalsCube", "phong");
        planner.draw("noNormalsSphere", "phong");

        planner.draw("normalsSphere", "flat");
        planner.draw("noNormalsCube", "flat");
        planner.draw("normalsCube", "flat");
        planner.draw("normalsCube", "flat");
        planner.draw("noNormalsSphere", "flat");

        for(auto const& format: planner) {
            cout << "Format has: ";
            if(format.hasAttribute("inPositions")) {
                cout << "positions ";
            }
            if(format.hasAttribute("inNormals")) {
                cout << "normals ";
            }
            cout << endl;

            for(auto const& group : format) {
                cout << "\tGroup: " << group.name() << endl;
                for(auto const& mesh : group) {
                    cout << "\t\tMesh first index: " << mesh.firstIndex << endl;
                    cout << "\t\tMesh index count: " << mesh.indexCount << endl;
                }
            }
        }

    } catch (std::exception const& ex) {
        cerr << "--------------EXCEPTION---------------" << endl;
        cerr << ex.what() << endl;
    }
}