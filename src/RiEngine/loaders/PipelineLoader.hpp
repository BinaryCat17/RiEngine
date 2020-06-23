#pragma once
#include "MeshLoader.hpp"
#include <spirv_cross.hpp>

namespace rise {
    enum class ShaderType {
        Vertex,
        Fragment,
    };

    struct ResourceId {
        spirv_cross::ID id;
        Index shaderIndex = 0;
    };

    using SPIRVBinary = vector<uint32_t>;

    namespace util {
        namespace binary = cista::offset;

        struct PipelineData {
            binary::string vertexShader;
            binary::string fragmentShader;
            bool depthStencil;
        };

        struct ShaderInfo {
            explicit ShaderInfo(SPIRVBinary const& binary) :
                binary(binary), compiler(binary), resources(compiler.get_shader_resources()) {}

            SPIRVBinary binary;
            spirv_cross::Compiler compiler;
            spirv_cross::ShaderResources resources;
        };

        struct Pipeline {
            string_view vertexShader;
            string_view fragmentShader;
            bool depthStencil = false;
        };
    }

    class ImportedPipelines : public NonCopyable {
        friend class PipelineImporter;
    public:
        SPIRVBinary const& spirvBinary(string_view pipe, ShaderType type, string_view shader) const;

        ResourceId uniform(string_view pipeline, string_view name) const;

        ResourceId sampledImage(string_view pipeline,string_view name) const;

        ResourceId stageInput(string_view pipeline, string_view name) const;

        ResourceId stageOutput(string_view pipeline, string_view name) const;

        spirv_cross::SPIRType type(ResourceId id) const;

        unsigned decoration(ResourceId id, spv::Decoration decoration) const;

    private:
        map<string, util::Pipeline> mPipelines;
        map<string, util::ShaderInfo> mVertexShaders;
        map<string, util::ShaderInfo> mFragmentShaders;
    };

    class PipelineImporter : public NonCopyable  {
    public:
        explicit PipelineImporter(fs::path folder) : mFolder(std::move(folder)) {}

        void import(string const& name) {
            mPipelinesToLoad.push_back(name);
        }

        ImportedPipelines load();

    private:
        fs::path mFolder;
        vector<string> mPipelinesToLoad;
    };
}
