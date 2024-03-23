#include <string>
#include <unordered_map>
#include <queue>
#include <list>

#include "clustering.h"

int lev(std::string a, std::string b) {
    int a_size = a.size();
    int b_size = b.size();
    std::vector<std::vector<int>> mat(a_size + 1, std::vector<int>(b_size + 1, 0));

    for(int row = 1; row <= a_size; row++) {
        mat[row][0] = row;
    }

    for(int col = 1; col <= b_size; col++) {
        mat[0][col] = col;
    }

    int addition = 0;

    for(int row = 1; row <= a_size; row++) {
        for(int col = 1; col <= b_size; col++) {
            if(a[row - 1] == b[col - 1]) addition = 0;
            else addition = 1;

            int left = mat[row][col - 1] + 1;
            int up = mat[row - 1][col] + 1;
            int diag = mat[row - 1][col - 1] + addition;

            mat[row].insert(mat[row].begin() + col, std::min(left, std::min(up, diag)));
        }
    }

    return mat[a_size][b_size];
}

/// @brief calculate the distances from input to each word in the list and map them
/// @param input word to calculate distnaces from
/// @param words list of words to which we want to calculate the distances
/// @return map containing each word in words as the key and distance from that word to input as the value
std::unordered_map<std::string, int> baseListAroundWord(const std::string& input, const std::list<std::string>& words) {
    std::unordered_map<std::string, int> distanceMap;

    for(const auto& word : words) {
        if(word == input) {
            continue;
        }

        distanceMap[word] = lev(input, word);
    }

    return distanceMap;
}

/// @brief Finds the word that is closest to the input word in the list
/// @param input base word
/// @param words list of words
/// @return word from the list of words that is the closest to the input word
std::list<std::string> findClosestWords(const std::string& input, const std::list<std::string> words, int c) {
    std::list<std::string> closest = { words.front()};
    int closestDistance = lev(input, closest.front());
    int currentDistance;

    for(auto it = std::next(words.begin()); it != words.end(); ++it) {
        currentDistance = lev(input, *it);

        if(currentDistance < closestDistance) {
            for(auto jt = closest.begin(); jt != closest.end(); ++jt) {
                if(lev(*jt, *it) > currentDistance + c) {
                    jt = closest.erase(jt);
                    --jt;
                }
            }

            closest.push_back(*it);
            closestDistance = currentDistance;
        }
        else if(currentDistance == closestDistance || currentDistance == closestDistance + c) {
            closest.push_back(*it);
        }
    }

    return closest;
}

/// @brief Finds the word that is the closest to the input
/// @param input input word
/// @param clusterMap map of clusters where values are clusters (list of words) and key is the most central word in the cluster 
/// @return the closest word to the input
std::list<std::string> findClosestCandidates(const std::string& input, const std::unordered_map<std::string, std::list<std::string>>& clusterMap) {
    std::list<std::string> clusterKeys;
    std::list<std::string> closestClusterRepresentatives;

    // put all the cluster representatives in a list
    for(const auto& wordClusterPairs : clusterMap) {
        clusterKeys.push_back(wordClusterPairs.first);
    }

    closestClusterRepresentatives = findClosestWords(input, clusterKeys, 0);

    std::list<std::string> closestWords;

    for (const auto& representative : closestClusterRepresentatives) {
        std::list<std::string> closest = findClosestWords(input, clusterMap.at(representative), 0);
        closestWords.insert(closestWords.end(), closest.begin(), closest.end());
    }

    return findClosestWords(input, closestWords, 0);
}