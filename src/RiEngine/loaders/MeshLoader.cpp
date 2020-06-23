#include "MeshLoader.hpp"
#include "../Exception.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

namespace rise {
    namespace {
        using namespace util;

        constexpr auto serializeMode = cista::mode::WITH_INTEGRITY | cista::mode::UNCHECKED;

        auto getAttributes(const vector <MeshConvertOp> &options) {
            binary::hash_map<binary::string, VertexAttributeData> result;
            Size vertexSize = 0;

            for (auto const &opt: options) {
                result.emplace(opt.name.c_str(), VertexAttributeData{opt.format, vertexSize});
                vertexSize += formatSize(opt.format);
            }
            return result;
        }

        template<typename T>
        void writeToBuffer(binary::vector<uint8_t> &buffer, T const &obj) {
            auto currentPos = buffer.size();
            buffer.resize(currentPos + sizeof(T));
            memcpy(buffer.data() + currentPos, &obj, sizeof(T));
        }

        void writeAttrib(MeshData &data, Format format, aiVector3D vec) {
            switch (format) {
                case Format::R32G32B32Sfloat:
                    return writeToBuffer(data.vertices, glm::vec<3, float>(vec.x, vec.y, vec.z));
                case Format::R32G32Sfloat:
                    return writeToBuffer(data.vertices, glm::vec<2, float>(vec.x, vec.y));
                default:
                    throw std::runtime_error("not implemented format!");
            }
        }

        void writeVertices(aiMesh const *mesh, vector <MeshConvertOp> const &ops, MeshData &data) {
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

        void writeIndices(aiMesh const *mesh, MeshData &data, Offset &vertexOffset) {
            for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
                for (uint32_t j = 0; j < 3; j++) {
                    writeToBuffer(
                            data.indices,
                            uint32_t(vertexOffset + mesh->mFaces[f].mIndices[j]));
                }
            }
            vertexOffset += mesh->mNumVertices;
        }

        MeshData convertMesh(fs::path const &path, vector <MeshConvertOp> const &ops,
                Size &vertexCount, Size &indexCount) {
            MeshData data;

            Assimp::Importer importer;
            auto scene = importer.ReadFile(path.string(),
                    aiProcessPreset_TargetRealtime_MaxQuality);

            for (size_t i = 0; i != scene->mNumMeshes; ++i) {
                writeVertices(scene->mMeshes[i], ops, data);
                vertexCount += scene->mMeshes[i]->mNumVertices;
            }

            Offset vertexOffset;
            for (size_t i = 0; i != scene->mNumMeshes; ++i) {
                writeIndices(scene->mMeshes[i], data, vertexOffset);
                indexCount += scene->mMeshes[i]->mNumFaces * 3;
            }

            return data;
        }

        auto convertVertexFormat(VertexFormatData const *data, Size &vertexSize) {
            map <string, VertexAttribute> result;

            for (auto const &attribute : data->attributes) {
                spdlog::info("Attribute {}:", attribute.first);
                spdlog::info("\tFormat: {}", toString(attribute.second.format));
                spdlog::info("\tOffset: {}", attribute.second.offset);

                result.emplace(attribute.first.data(),
                        VertexAttribute{attribute.second.format, Offset(attribute.second.offset)});
                vertexSize += formatSize(attribute.second.format);
            }

            return result;
        }

        auto convertMeshes(util::VertexFormatData const *data, Size vertexSize,
                Size &sizeForVertices, Size &sizeForIndices, vector <string> const &meshes) {
            map <string, MeshDrawInfo> result;

            for (auto const &mesh : data->meshes) {
                if (ranges::find(meshes, mesh.first.str()) != meshes.end()) {
                    spdlog::info("Found mesh: {}", mesh.first);

                    result.emplace(mesh.first.str(), MeshDrawInfo{
                            mesh.second.firstIndex, mesh.second.indexCount});

                    sizeForVertices += mesh.second.vertexCount * vertexSize;
                    sizeForIndices += mesh.second.indexCount * sizeof(uint32_t);
                }
            }

            return result;
        }
    }

    MeshFolderImporter::MeshFolderImporter(fs::path const &folder, vector <string> const &meshes)
            : mFolder(folder) {
        auto formatPath = folder / "format.rise";
        if (!fs::exists(formatPath)) {
            throw std::runtime_error("mesh imported format not found");
        }
        spdlog::info("Load mesh format: {}", formatPath.string());

        cista::mmap formatFile(formatPath.c_str(), cista::mmap::protection::READ);
        auto formatData = cista::deserialize<util::VertexFormatData, serializeMode>(formatFile);

        if (!formatData) {
            throw std::runtime_error("Fail to load mesh format");
        }

        Size vertexSize = 0;
        mMeshes.format = convertVertexFormat(formatData, vertexSize);
        mMeshes.meshInfo = convertMeshes(formatData, vertexSize,
                mSizeForVertices, mSizeForIndices, meshes);
    }

    FolderMeshes MeshFolderImporter::load(MemData vertexData, MemData indexData) {
        assert(vertexData.size >= sizeForVertices() || indexData.size >= sizeForIndices());

        spdlog::info("Loading vertices and indices to buffer");

        Offset currentVertexOffset = 0, currentIndexOffset = 0;
        for (auto const &info : mMeshes.meshInfo) {
            auto const &meshName = info.first;
            auto path = mFolder / (meshName + ".rim");
            if (!fs::exists(path)) {
                throw FileError("Mesh file not found: " + meshName, path);
            }

            spdlog::info("Import mesh from: {}", path.string());

            cista::mmap mmap(path.c_str(), cista::mmap::protection::READ);
            auto meshData = cista::deserialize<MeshData, serializeMode>(mmap);

            memcpy(reinterpret_cast<uint8_t *>(vertexData.data) + currentVertexOffset,
                    meshData->vertices.data(), meshData->vertices.size());
            memcpy(reinterpret_cast<uint8_t *>(indexData.data) + currentIndexOffset,
                    meshData->indices.data(), meshData->indices.size());

            currentVertexOffset += meshData->vertices.size();
            currentIndexOffset += meshData->indices.size();
        }

        return std::move(mMeshes);
    }

    void MeshConverter::load(fs::path const &path, optional <string> const &dstName) {
        if (!fs::exists(path)) {
            throw FileError("File not exist: ", path);
        }

        spdlog::info("Loading for converting: {}", path.string());

        Size totalVertices, totalIndices;
        auto meshData = convertMesh(path, mConvertOps, totalVertices, totalIndices);
        spdlog::info("Total vertices {} Total indices {}", totalVertices, totalIndices);

        auto meshName = dstName.value_or(path.stem());

        mDstMeshes.emplace(meshName, meshData);

        MeshInfoData mesh = {};
        mesh.firstIndex = currentIndex;
        mesh.indexCount = totalIndices;
        mesh.vertexCount = totalVertices;

        currentIndex += totalIndices;

        mData.meshes.emplace(meshName.c_str(), mesh);
    }

    void MeshConverter::convert(fs::path const &dst) {
        spdlog::info("Converting to folder {}", dst.string());

        mData.attributes = getAttributes(mConvertOps);

        cista::buf formatMap{cista::mmap{(dst / "format.rise").c_str()}};
        cista::serialize<serializeMode>(formatMap, mData);

        for (auto const &mesh : mDstMeshes) {
            cista::buf mmap{cista::mmap{(dst.string() + "/" + mesh.first + ".rim").c_str()}};
            cista::serialize<serializeMode>(mmap, mesh.second);
        }
    }

    MeshImporter::MeshImporter(fs::path const &folder, vector <string> const &meshes) {
        spdlog::info("Mesh importer on: {}", folder.string());
        for (auto const &entry: fs::directory_iterator(folder)) {
            if (entry.is_directory()) {
                mFolders.emplace_back(entry.path(), meshes);
            }
        }
    }

    void MeshDrawPlanner::draw(string_view mesh, string_view group) {
        spdlog::info("plan draw: {}, {}", mesh.data(), group.data());

        auto findFormat = [this, &mesh](auto &&val) {
            return val.name() == mMeshInfo.at(mesh.data()).format;
        };
        auto formatIter = ranges::find_if(mFormatGroup, findFormat);
        if (formatIter == mFormatGroup.end()) {
            throw std::runtime_error("mesh format not found");
        }

        auto &drawGroups = formatIter->mGroups;

        auto findDrawGroup = [&group](auto &&val) { return group == val.mName; };
        auto groupIter = ranges::find_if(drawGroups, findDrawGroup);

        if (groupIter == drawGroups.end()) {
            MeshGroup drawGroup;
            drawGroup.mName = group;
            drawGroups.push_back(std::move(drawGroup));
            groupIter = --drawGroups.end();
        }

        groupIter->mMeshes.push_back(mMeshInfo.at(mesh.data()).drawInfo);
    }

    MeshDrawPlanner MeshImporter::load(MemData vertexData, MemData indexData) {
        assert(vertexData.size >= sizeForVertices() && indexData.size >= sizeForIndices());

        MeshDrawPlanner planner;
        planner.mFormatGroup.reserve(mFolders.size());

        Offset vertexOffset = 0, indexOffset = 0;
        for (auto &folder: mFolders) {
            vertexData.size = folder.sizeForVertices();
            indexData.size = folder.sizeForIndices();

            vertexData.data = reinterpret_cast<uint8_t *>(vertexData.data) + vertexOffset;
            indexData.data = reinterpret_cast<uint8_t *>(indexData.data) + indexOffset;

            auto meshInfo = folder.load(vertexData, indexData);
            MeshFormatGroup formatGroup;
            formatGroup.mName = folder.name();
            formatGroup.mFormat = meshInfo.format;
            formatGroup.mVertexOffset = vertexOffset;
            formatGroup.mIndexOffset = indexOffset;

            vertexOffset += vertexData.size;
            indexOffset += indexData.size;

            planner.mFormatGroup.push_back(std::move(formatGroup));
            for (auto const &p : meshInfo.meshInfo) {
                planner.mMeshInfo.emplace(p.first,
                        MeshDrawPlanner::MeshInfo{p.second, folder.name()});
            }
        }

        return planner;
    }

    Size MeshImporter::sizeForVertices() const {
        auto sumVertices = [](Size total, MeshFolderImporter const &importer) {
            return total + importer.sizeForVertices();
        };

        return std::accumulate(mFolders.begin(), mFolders.end(), Size(), sumVertices);
    }

    Size MeshImporter::sizeForIndices() const {
        auto sumIndices = [](Size total, MeshFolderImporter const &importer) {
            return total + importer.sizeForIndices();
        };

        return std::accumulate(mFolders.begin(), mFolders.end(), Size(), sumIndices);
    }

}