#include "Exception.hpp"

namespace rise {
    void assertFileError(bool condition, const std::string &message, const fs::path &path) {
        if (!condition) {
            throw FileError(message, path);
        }
    }
}
