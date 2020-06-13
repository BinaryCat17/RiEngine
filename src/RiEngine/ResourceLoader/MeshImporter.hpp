#pragma once
#include "../Format.hpp"
#include <cista/mmap.h>
#include <cista/serialization.h>

namespace rise {
    namespace binary = cista::offset;

    struct Mesh {
        binary::string name;
        uint32_t firstVertex;
        uint32_t lastVertex;
        uint32_t firstIndex;
        uint32_t lastIndex;
    };

    struct VertexAttribute {
        binary::string name;
        Format format = Format::Undefined;
    };

    struct VertexFormat {
        binary::vector<VertexAttribute> attributes;
        binary::vector<Mesh> meshes;
        uint64_t bufferOffset;
        uint64_t bufferSize;
    };

    struct MeshImportData {
        binary::vector<VertexFormat> mFormats;
        binary::vector<uint8_t> vertices;
        binary::vector<uint8_t> indices;
    };

    class MeshImporter {
    public:
        explicit MeshImporter(fs::path const& file);

        MeshImporter(MeshImporter const&) = delete;

        MeshImporter(MeshImporter &&) = default;

        MeshImporter& operator=(MeshImporter const&) = delete;

        MeshImporter& operator=(MeshImporter &&) = default;

        void load(MemData vertexData, MemData indexData);

        Size sizeForVertices() const {
            return Size(mData->vertices.size());
        }

        Size sizeForIndices() const {
            return Size(mData->indices.size());
        }

    private:
        cista::mmap mMemMap;
        MeshImportData const* mData;
    };

    enum class MeshAttribute {
        Position,
        Normal,
        TextCoord,
    };

    struct ConvertOption {
        string name;
        MeshAttribute type;
        Format format;
    };

    class MeshDrawPlanner {
    public:

    private:

    };

    class MeshConverter {
    public:
        MeshConverter() = default;

        MeshConverter(MeshConverter const&) = delete;

        MeshConverter(MeshConverter &&) = default;

        MeshConverter& operator=(MeshConverter const&) = delete;

        MeshConverter& operator=(MeshConverter &&) = default;

        MeshDrawPlanner load(vector<fs::path> const& paths, vector<ConvertOption> const& options);

        void convert(fs::path const& dst, string const& name) const;

    private:
        MeshImportData data;
        Offset currentOffset;
    };
}