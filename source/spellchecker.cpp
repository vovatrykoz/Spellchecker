#include "../include/spellchecker.h"

#include <algorithm>
#include <cstring>
#include <vector>

#ifdef __GNUC__

int lev(const std::string& a, const std::string& b) {
    const std::size_t a_size = a.size();
    const std::size_t b_size = b.size();

    int mat[a_size + 1][b_size + 1];
    mat[0][0] = 0;

    for (std::size_t row = 1; row <= a_size; row++) {
        mat[row][0] = static_cast<int>(row);
    }

    for (std::size_t col = 1; col <= b_size; col++) {
        mat[0][col] = static_cast<int>(col);
    }

    for (std::size_t row = 1; row <= a_size; row++) {
        for (std::size_t col = 1; col <= b_size; col++) {
            const int adder = a[row - 1] == b[col - 1] ? 0 : 1;

            const int left = mat[row][col - 1] + 1;
            const int up = mat[row - 1][col] + 1;
            const int diag = mat[row - 1][col - 1] + adder;

            mat[row][col] = std::min(left, std::min(up, diag));
        }
    }

    return mat[a_size][b_size];
}

#else  // If compiling with MSVC

#define NOMINMAX  // to fix the conflicts between the Window's min/max and c++
                  // min/max
#include <windows.h>  // for alloca

#pragma warning(push)

#pragma warning( \
    disable : 6255)  // _alloca indicates failure by raising a stack overflow
                     // exception. Consider using _malloca instead
// REASON FOR SUPPRESSION:
//      Given the typical word length, it is pretty much impossible to overflow
//      the stack That is unless someone deliberatelly feeds extremely long
//      strings into the program Since this is a toy repo that is not intended
//      to be used in a real prod environment, I decided to suppress the warning
//      connected to the use of _alloca

#pragma warning(disable : 6386)  // Buffer overrun: accessing 'buffer name', the
                                 // writable size is 'size1' bytes, but 'size2'
                                 // bytes may be written
// REASON FOR SUPPRESSION:
//      The compiler thinks we might overrun the buffer when doing mat[col] =
//      static_cast<int>(col); That is impossible with the current logic given
//      that b_size is always going to be smaller than matrixSize Since this is
//      a toy repo that is not intended to be used in a real prod environment,
//      I decided to suppress this warning

#pragma warning(disable \
                : 6385)  // Invalid data: accessing buffer-name, the readable
                         // size is size1 bytes, but size2 bytes may be read
// REASON FOR SUPPRESSION:
//      The compiler thinks we might overflow the buffer when setting the "up"
//      value below That is impossible with the current logic given that
//      matrixSize is always going to be bigger than (row - 1) * cols + col
//      Since this is a toy repo that is not intended to be used in a real prod
//      environment, I decided to suppress this warning

// MSVS does not support variable length arrays
// we have to use _alloca to manually allocate the memory on the stack instead
int lev(const std::string& a, const std::string& b) {
    const std::size_t a_size = a.size();
    const std::size_t b_size = b.size();
    const std::size_t rows = a_size + 1;
    const std::size_t cols = b_size + 1;
    const std::size_t matrixSize = rows * cols * sizeof(int);

    int* mat = (int*)alloca(matrixSize);
    mat[0] = 0;

    for (std::size_t row = 1; row <= a_size; row++) {
        mat[row * cols] = static_cast<int>(row);
    }

    for (std::size_t col = 1; col <= b_size; col++) {
        mat[col] = static_cast<int>(col);
    }

    for (std::size_t row = 1; row <= a_size; row++) {
        const int rowBase = row * cols;
        const int prevRowBase = (row - 1) * cols;

        for (std::size_t col = 1; col <= b_size; col++) {
            const int adder = a[row - 1] == b[col - 1] ? 0 : 1;

            const int left = mat[rowBase + (col - 1)] + 1;
            const int up = mat[prevRowBase + col] + 1;
            const int diag = mat[prevRowBase + (col - 1)] + adder;

            mat[row * cols + col] = std::min(left, std::min(up, diag));
        }
    }

    return mat[a_size * cols + b_size];
}

#pragma warning(pop)

#endif

std::unordered_map<std::string, int> baseListAroundWord(
    const std::string& input, const std::vector<std::string>& words) {
    std::unordered_map<std::string, int> distanceMap;

    for (const auto& word : words) {
        if (word == input) {
            continue;
        }

        distanceMap[word] = lev(input, word);
    }

    return distanceMap;
}

std::vector<std::string> findClosestWords(const std::string& input,
                                          const std::vector<std::string>& words,
                                          int c) {
    std::vector<std::string> closest = {words.front()};
    int closestDistance = lev(input, closest.front());

    for (auto it = std::next(words.begin()); it != words.end(); ++it) {
        const int currentDistance = lev(input, *it);

        if (currentDistance < closestDistance) {
            const auto newEnd = std::remove_if(
                closest.begin(), closest.end(),
                [&it, currentDistance, c](const std::string& val) {
                    return lev(val, *it) > currentDistance + c;
                });

            closest = std::vector<std::string>(closest.begin(), newEnd);

            closest.push_back(*it);
            closestDistance = currentDistance;
        } else if (currentDistance == closestDistance ||
                   currentDistance == closestDistance + c) {
            closest.push_back(*it);
        }
    }

    return closest;
}

std::vector<std::string> findClosestCandidates(
    const std::string& input,
    const std::unordered_map<std::string, std::vector<std::string>>&
        clusterMap) {
    std::vector<std::string> clusterKeys;

    clusterKeys.reserve(clusterMap.size());
    // put all the cluster representatives in a list
    for (const auto& wordClusterPairs : clusterMap) {
        clusterKeys.push_back(wordClusterPairs.first);
    }

    const std::vector<std::string> closestClusterRepresentatives =
        findClosestWords(input, clusterKeys, 0);

    std::vector<std::string> closestWords;

    for (const auto& representative : closestClusterRepresentatives) {
        const std::vector<std::string> closest =
            findClosestWords(input, clusterMap.at(representative), 0);
        closestWords.insert(closestWords.end(), closest.begin(), closest.end());
    }

    return findClosestWords(input, closestWords, 0);
}