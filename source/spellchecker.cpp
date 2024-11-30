#include "../include/spellchecker.h"

#include <vector>

int lev(const std::string& a, const std::string& b) {
    int a_size = a.size();
    int b_size = b.size();
    std::vector<std::vector<int>> mat(a_size + 1,
                                      std::vector<int>(b_size + 1, 0));

    for (int row = 1; row <= a_size; row++) {
        mat[row][0] = row;
    }

    for (int col = 1; col <= b_size; col++) {
        mat[0][col] = col;
    }

    int addition = 0;

    for (int row = 1; row <= a_size; row++) {
        for (int col = 1; col <= b_size; col++) {
            if (a[row - 1] == b[col - 1])
                addition = 0;
            else
                addition = 1;

            int left = mat[row][col - 1] + 1;
            int up = mat[row - 1][col] + 1;
            int diag = mat[row - 1][col - 1] + addition;

            mat[row][col] = std::min(left, std::min(up, diag));
        }
    }

    return mat[a_size][b_size];
}

std::unordered_map<std::string, int> baseListAroundWord(
    const std::string& input, const std::list<std::string>& words) {
    std::unordered_map<std::string, int> distanceMap;

    for (const auto& word : words) {
        if (word == input) {
            continue;
        }

        distanceMap[word] = lev(input, word);
    }

    return distanceMap;
}

std::list<std::string> findClosestWords(const std::string& input,
                                        const std::list<std::string>& words,
                                        int c) {
    std::list<std::string> closest = {words.front()};
    int closestDistance = lev(input, closest.front());
    int currentDistance;

    for (auto it = std::next(words.begin()); it != words.end(); ++it) {
        currentDistance = lev(input, *it);

        if (currentDistance < closestDistance) {
            closest.remove_if([&it, currentDistance, c](std::string val) {
                return lev(val, *it) > currentDistance + c;
            });

            closest.push_back(*it);
            closestDistance = currentDistance;
        } else if (currentDistance == closestDistance ||
                   currentDistance == closestDistance + c) {
            closest.push_back(*it);
        }
    }

    return closest;
}

std::list<std::string> findClosestCandidates(
    const std::string& input,
    const std::unordered_map<std::string, std::list<std::string>>& clusterMap) {
    std::list<std::string> clusterKeys;
    std::list<std::string> closestClusterRepresentatives;

    // put all the cluster representatives in a list
    for (const auto& wordClusterPairs : clusterMap) {
        clusterKeys.push_back(wordClusterPairs.first);
    }

    closestClusterRepresentatives = findClosestWords(input, clusterKeys, 0);

    std::list<std::string> closestWords;

    for (const auto& representative : closestClusterRepresentatives) {
        std::list<std::string> closest =
            findClosestWords(input, clusterMap.at(representative), 0);
        closestWords.insert(closestWords.end(), closest.begin(), closest.end());
    }

    return findClosestWords(input, closestWords, 0);
}