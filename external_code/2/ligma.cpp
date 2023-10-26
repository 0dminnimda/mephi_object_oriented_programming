#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

#include "./TestLib/test.h"

#if defined DYNAMIC
#include "./StackLib/stack.h"
#else
#include "./StackStaticLib/stack_static.h"
#endif

using namespace Lab2;
namespace fs = std::filesystem;

static const std::regex one_page = std::regex("(\\d+)");
static const std::regex range_of_pages = std::regex("(\\d+)-(\\d+)");

std::vector<fs::path> get_page_paths(const fs::path &dir) {
    std::vector<fs::path> paths;
    for (const auto &entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".txt") continue;

        std::string stem = entry.path().stem().string();
        std::smatch match;
        if (std::regex_match(stem, match, range_of_pages) ||
            std::regex_match(stem, match, one_page))
        {
            paths.push_back(entry.path());
        }
    }
    return paths;
}

bool all_the_pages_are_present(const std::vector<fs::path> &paths) {
    Stack stack;
    std::string surname = "g";

    for (const auto &path : paths) {
        std::string stem = path.stem().string();
        std::smatch match;
        if (std::regex_match(stem, match, range_of_pages)) {
            int start = std::stoi(match[1]);
            int end = std::stoi(match[2]);
            Test test(surname, 0, start, end);
            stack += test;
        } else if (std::regex_match(stem, match, one_page)) {
            int page = std::stoi(match[1]);
            Test test(surname, 0, page, page);
            stack += test;
        }
    }

    stack.union_stack();

    return stack.getC_size() == 1;
}

void merge_files(const fs::path &dir) {
    std::vector<fs::path> paths = get_page_paths(dir);

    if (!all_the_pages_are_present(paths)) {
        std::cout << dir << ": skipping, some pages are missing" << std::endl;
        return;
    }

    std::sort(paths.begin(), paths.end(), [](const auto &a, const auto &b) {
        return a.stem() < b.stem();
    });

    std::ofstream out(dir / "merged.txt");
    for (const auto &path : paths) {
        std::ifstream file(path);
        out << file.rdbuf();
    }

    std::cout << dir << ": done" << std::endl;
}

int sub_main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "Usage:\n    " << argv[0] << " path" << std::endl;
        return EXIT_FAILURE;
    }

    fs::path dir = argv[1];
    for (const auto &entry : fs::directory_iterator(dir)) {
        if (entry.is_directory()) {
            merge_files(entry.path());
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    try {
        return sub_main(argc, argv);
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "unknown exception" << std::endl;
    }
    return EXIT_FAILURE;
}
