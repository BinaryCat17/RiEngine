#pragma once

namespace rise {
    class FileError : public exception {
    public:
        explicit FileError(string const& message, fs::path path) :
            mPath(std::move(path)), message(message + mPath.string()) {
            spdlog::error("FileError '{}': {}", mPath.string(), message);
        }

        fs::path const& filePath() const {
            return mPath;
        }

        char const *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
            return message.c_str();
        }

    private:
        fs::path mPath;
        string message;
    };

    void assertFileError(bool condition, std::string const& message, fs::path const& path);
}


