#include <fstream>
#include "PipelineImporter.hpp"
#include "Exception.hpp"

namespace rise {
    Shader loadShader(fs::path const& path, ShaderType type) {
        std::ifstream t(path);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

        return Shader {
            .type = type,
            .code = str,
        };
    }

    Shaders loadShaders(fs::path const& folder) {
        Shaders result;

        for (auto const &entry: fs::directory_iterator(folder)) {
            auto const& path = entry.path();
            auto filename = path.filename();
            auto extension = path.extension().string().substr(1);

            if(extension == "vert") {
                result.emplace(filename, loadShader(path, ShaderType::Vertex));
            } else if (extension == "frag") {
                result.emplace(filename, loadShader(path, ShaderType::Fragment));
            }
        }

        return result;
    }

    Pipelines loadPipelines(fs::path const& folder) {
        Pipelines result;

        for (auto const &entry: fs::directory_iterator(folder)) {
            auto const& path = entry.path();
            auto filename = path.filename();
            auto extension = path.extension().string().substr(1);

            if(extension == "ripipe") {

            }
        }

        return result;
    }

    PipelineImporter::PipelineImporter(const fs::path &folder) :
        mShaders(loadShaders(folder)) {

    }
}