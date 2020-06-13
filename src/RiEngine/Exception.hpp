#pragma once

namespace rise {
    class FileError : public runtime_error {
    public:
        explicit FileError(string const& message, fs::path path) :
            runtime_error(message), mPath(std::move(path)) {
            spdlog::error("FileError '{}': {}", mPath.string(), message);
        }

        fs::path const& filePath() const {
            return mPath;
        }

    private:
        fs::path mPath;
    };

    void assertFileError(bool condition, std::string const& message, fs::path const& path);
}


