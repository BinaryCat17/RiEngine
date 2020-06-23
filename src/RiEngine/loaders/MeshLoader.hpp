#pragma once
#include "../Format.hpp"
#include <cista/mmap.h>
#include <cista/serialization.h>
#include <glm/glm.hpp>

namespace rise {
    enum class MeshAttribute {
        Position,
        Normal,
        TextCoord,
    };

    struct VertexAttribute {
        Format format = Format::Undefined;
        Offset offset = 0;
    };

    struct MeshDrawInfo {
        Index firstIndex;
        Size indexCount;
    };

    class MeshGroup {
        friend class MeshDrawPlanner;
        friend class MeshImporter;
    public:
        using iterator = vector<MeshDrawInfo>::const_iterator;

        iterator begin() const {
            return mMeshes.begin();
        }

        iterator end() const {
            return mMeshes.end();
        }

        string_view name() const {
            return mName;
        }

    private:
        string mName;
        vector <MeshDrawInfo> mMeshes;
    };

    class MeshFormatGroup {
        friend class MeshDrawPlanner;
        friend class MeshImporter;
    public:
        using iterator = vector<MeshGroup>::const_iterator;

        iterator begin() const {
            return mGroups.begin();
        }

        iterator end() const {
            return mGroups.end();
        }

        VertexAttribute attribute(string_view name) const {
            return mFormat.at(name.data());
        }

        bool hasAttribute(string_view name) const {
            return mFormat.contains(name.data());
        }

        Offset vertexOffset() const {
            return mVertexOffset;
        }

        Offset indexOffset() const {
            return mIndexOffset;
        }

        string_view name() const {
            return mName;
        }

    private:
        string mName;
        vector <MeshGroup> mGroups;
        map <string, VertexAttribute> mFormat;
        Offset mVertexOffset = 0;
        Offset mIndexOffset = 0;
    };

    struct MeshConvertOp {
        string name;
        MeshAttribute type;
        Format format;
    };

    struct FolderMeshes {
        map<string, MeshDrawInfo> meshInfo;
        map<string, VertexAttribute> format;
    };

    namespace util {
        namespace binary = cista::offset;

        struct VertexAttributeData {
            Format format;
            uint64_t offset;
        };

        struct MeshInfoData {
            uint32_t firstIndex;
            uint32_t indexCount;
            uint32_t vertexCount;
        };

        struct VertexFormatData {
            binary::hash_map<binary::string, VertexAttributeData> attributes;
            binary::hash_map<binary::string, MeshInfoData> meshes;
        };

        struct MeshData {
            binary::vector<uint8_t> vertices;
            binary::vector<uint8_t> indices;
        };

        class MeshFolderImporter : NonCopyable {
        public:
            explicit MeshFolderImporter(fs::path const &workingDirectory, vector<string> const& meshes);

            FolderMeshes load(MemData vertexData, MemData indexData);

            Size sizeForVertices() const {
                return mSizeForVertices;
            }

            Size sizeForIndices() const {
                return mSizeForIndices;
            }

            string name() const {
                return mFolder.stem().string();
            }

        private:
            fs::path mFolder;
            FolderMeshes mMeshes;
            Size mSizeForVertices = 0;
            Size mSizeForIndices = 0;
        };

    }

    class MeshConverter : NonCopyable {
    public:
        MeshConverter() {
            spdlog::info("Mesh converter created");
        }

        void addConvertOp(MeshConvertOp const& op) {
            mConvertOps.push_back(op);
        }

        void load(fs::path const &path, optional<string> const& dstName = {});

        void convert(fs::path const &dst);
    private:
        util::VertexFormatData mData;
        map <string, util::MeshData> mDstMeshes;
        vector<MeshConvertOp> mConvertOps;
        Index currentIndex = 0;
    };
    
    class MeshDrawPlanner : NonCopyable {
        friend class MeshImporter;
    public:
        void draw(string_view mesh, string_view group);

        using iterator = vector<MeshFormatGroup>::const_iterator;

        iterator begin() const {
            return mFormatGroup.begin();
        }

        iterator end() const {
            return mFormatGroup.end();
        }

    private:
        struct MeshInfo {
            MeshDrawInfo drawInfo;
            string format;
        };
        vector<MeshFormatGroup> mFormatGroup;
        map<string, MeshInfo> mMeshInfo;
    };

    class MeshImporter : NonCopyable {
    public:
        explicit MeshImporter(fs::path const &workingDirectory, vector<string> const& meshes);

        // IMPORTANT: Not use this class after load call, mesh data will be moved!
        MeshDrawPlanner load(MemData vertexData, MemData indexData);

        Size sizeForVertices() const;

        Size sizeForIndices() const;

    private:
        vector <util::MeshFolderImporter> mFolders;
    };
}