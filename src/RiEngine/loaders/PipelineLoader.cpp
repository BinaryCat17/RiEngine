#include "PipelineLoader.hpp"
#include "../Exception.hpp"
#include <cista/mmap.h>
#include <cista/serialization.h>

namespace rise {
    using namespace util;

    namespace {
        constexpr auto serializeMode = cista::mode::WITH_INTEGRITY | cista::mode::UNCHECKED;

        SPIRVBinary readSPIRV(fs::path const &path) {
            std::ifstream file(path, std::ios::ate | std::ios::binary);
            if (!file) throw FileError("Fail open file", path);

            return {std::istream_iterator<uint32_t>(file), std::istream_iterator<uint32_t>()};
        }
    }

    SPIRVBinary const &ImportedPipelines::spirvBinary(
            string_view pipe, ShaderType type, string_view shader) const {
        auto const& pipeline = mPipelines.at(pipe.data());

        switch (type) {
            case ShaderType::Vertex:
                return mVertexShaders.at(pipeline.vertexShader.data()).binary;
            case ShaderType::Fragment:
                return mVertexShaders.at(pipeline.fragmentShader.data()).binary;
        }
    }

    ResourceId ImportedPipelines::uniform(string_view pipelineName, string_view name) const {
        auto const& pipeline = mPipelines.at(pipelineName.data());
        if(mVertexShaders.at(pipeline.vertexShader.data()).resources.uniform_buffers.)
    }

    ImportedPipelines PipelineImporter::load() {
        ImportedPipelines result;

        for (auto const &loaded: mPipelinesToLoad) {
            fs::path path = mFolder / (loaded + ".rise");

            cista::mmap formatFile(path.c_str(), cista::mmap::protection::READ);
            auto pipelineData = cista::deserialize<util::PipelineData, serializeMode>(formatFile);

            Pipeline pipeline;
            pipeline.vertexShader = pipelineData->vertexShader.str();
            pipeline.fragmentShader = pipelineData->fragmentShader.str();
            pipeline.depthStencil = pipelineData->depthStencil;


            ShaderInfo shader(readSPIRV(path));

            if (!result.mVertexShaders.contains(pipeline.vertexShader.data())) {
                result.mVertexShaders.emplace(pipeline.vertexShader, shader);
            }
            if (!result.mVertexShaders.contains(pipeline.fragmentShader.data())) {
                result.mVertexShaders.emplace(pipeline.fragmentShader, shader);
            }
        }

        return result;
    }


}
