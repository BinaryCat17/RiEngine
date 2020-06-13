#pragma once
#include <filesystem>
#include <map>
#include <variant>
#include <optional>
#include "MeshImporter.hpp"

namespace rise {
    namespace fs = std::filesystem;

    enum class PrimitiveType {
        Float,
        Int,
        Uint,
        Vec2,
        Vec3,
        Vec4,
        Mat3x3,
        Mat4x4,
    };

    struct StructType;

    using VariableInfo = std::variant<StructType const*, PrimitiveType>;

    struct MemberInfo {
        Offset offset;
        VariableInfo variable;
        std::string name;
    };

    struct StructType {
        std::vector<MemberInfo> members;
        Size size;
    };

    using Uniforms = std::map<std::string, VariableInfo const*>;

    struct Pipeline {
        std::string const* vertexShader;
        std::string const* fragmentShader;
        Uniforms uniforms;
        VertexFormat const* inputFormat;
    };

    enum class ShaderType {
        Vertex,
        Fragment,
    };

    struct Shader {
        ShaderType type;
        std::string code;
        Uniforms uniforms;
        VertexFormat vertexFormat;
    };

    using Shaders = std::map<std::string, Shader>;
    using Pipelines = std::map<std::string, Pipeline>;

    class PipelineImporter {
    public:
        explicit PipelineImporter(fs::path const& folder);

    private:
        Shaders mShaders;
        Pipelines mPipelines;
    };
}



