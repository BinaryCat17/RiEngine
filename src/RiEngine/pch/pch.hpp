#ifndef RISE_PCH_H
#define RISE_PCH_H

#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <algorithm>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include <chrono>
#include <deque>
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <span>
#include <RiUtil.hpp>
#include <spdlog/spdlog.h>

namespace rise {
    namespace fs = std::filesystem;
    namespace time = std::chrono;

    using std::vector;
    using std::deque;
    using std::string;
    using std::map;
    using std::multimap;
    using std::unordered_multimap;
    using std::string_view;
    using std::ifstream;
    using std::ofstream;
    using std::istream;
    using std::ostream;
    using std::variant;
    using std::optional;
    using std::runtime_error;
    using std::exception;
    using std::range_error;
    using std::span;
}

#endif