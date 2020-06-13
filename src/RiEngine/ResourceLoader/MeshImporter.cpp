#include "MeshImporter.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

namespace rise {
    constexpr auto serializeMode =
            cista::mode::WITH_VERSION | cista::mode::WITH_INTEGRITY | cista::mode::UNCHECKED;

    MeshImporter::MeshImporter(fs::path const &file) :
            mMemMap(file.c_str(), cista::mmap::protection::READ),
            mData(cista::deserialize<MeshImportData, serializeMode>(mMemMap)) {

    }

    void MeshImporter::load(MemData vertexData, MemData indexData) {
        assert(vertexData.size >= sizeForVertices() || indexData.size >= sizeForIndices());
        memcpy(vertexData.data, mData->vertices.data(), sizeForVertices().value);
        memcpy(indexData.data, mData->indices.data(), sizeForIndices().value);
    }

    binary::vector<VertexAttribute> getAttributes(const vector <ConvertOption> &options, Size &vs) {
        binary::vector<VertexAttribute> result;

        for (auto const &opt: options) {
            result.push_back(VertexAttribute{opt.name, opt.format});
            vs += formatSize(opt.format);
        }

        return result;
    }

    template<typename T>
    void writeToBuffer(binary::vector<uint8_t> &buffer, T const &obj) {
        auto currentPos = buffer.size();
        buffer.resize(currentPos + sizeof(T));
        memcpy(buffer.data() + currentPos, &obj, sizeof(T));
    }

    void writeAttrib(MeshImportData &data, Format format, aiVector3D vec) {
        switch (format) {
            case Format::R32G32B32Sfloat:
                return writeToBuffer(data.vertices, glm::vec<3, float>(vec.x, vec.y, vec.z));
            case Format::R32G32Sfloat:
                return writeToBuffer(data.vertices, glm::vec<2, float>(vec.x, vec.y));
            default:
                throw std::runtime_error("not implemented format!");
        }
    }

    void
    writeVertices(aiMesh const *mesh, vector <ConvertOption> const &ops, MeshImportData &data) {
        for (size_t i = 0; i != mesh->mNumVertices; ++i) {
            for (auto const &op : ops) {
                switch (op.type) {
                    case MeshAttribute::Position:
                        writeAttrib(data, op.format, mesh->mVertices[i]);
                        break;
                    case MeshAttribute::Normal:
                        writeAttrib(data, op.format, mesh->mNormals[i]);
                        break;
                    case MeshAttribute::TextCoord:
                        writeAttrib(data, op.format, mesh->mTextureCoords[i][0]);
                        break;
                    default:
                        throw std::runtime_error("not implemented format!");
                }
            }
        }
    }

    void writeIndices(aiMesh const *mesh, MeshImportData &data, size_t &vertexOffset) {
        for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
            for (uint32_t j = 0; j < 3; j++) {
                writeToBuffer(data.indices, uint32_t(vertexOffset + mesh->mFaces[f].mIndices[j]));
            }
        }
        vertexOffset += mesh->mNumVertices;
    }

    Mesh convertMesh(uint32_t &currentVertex, uint32_t &currentIndex, fs::path const &path,
            vector <ConvertOption> const &ops, MeshImportData &data) {
        Assimp::Importer importer;
        auto scene = importer.ReadFile(path.string(), aiProcessPreset_TargetRealtime_MaxQuality);

        Mesh mesh = {
                .name = path.filename().string(),
                .firstVertex = currentVertex,
                .firstIndex = currentIndex,
        };

        for (size_t i = 0; i != scene->mNumMeshes; ++i) {
            writeVertices(scene->mMeshes[i], ops, data);
            currentVertex += scene->mMeshes[i]->mNumVertices;
        }

        size_t vertexOffset = 0;
        for (size_t i = 0; i != scene->mNumMeshes; ++i) {
            writeIndices(scene->mMeshes[i], data, vertexOffset);
            currentIndex = +scene->mMeshes[i]->mNumFaces * 3;
        }

        mesh.lastVertex = currentVertex;
        mesh.lastIndex = currentIndex;

        return mesh;
    }

    MeshDrawPlanner
    MeshConverter::load(vector <fs::path> const &paths, vector <ConvertOption> const &options) {
        Size vertexSize;
        VertexFormat format{
                .attributes = getAttributes(options, vertexSize),
                .bufferOffset = currentOffset.value,
        };

        uint32_t currentVertex = 0, currentIndex = 0;
        for (auto const &path : paths) {
            format.meshes.push_back(convertMesh(currentVertex, currentIndex, path, options, data));
        }

        format.bufferSize = currentIndex * vertexSize.value + currentIndex * sizeof(uint32_t);
        currentOffset.value += format.bufferSize;
    }

    void MeshConverter::convert(fs::path const &dst, string const &name) const {
        cista::buf mmap{cista::mmap{(dst.string() + "." + name).c_str()}};
        cista::serialize<serializeMode>(mmap, data);
    }
}