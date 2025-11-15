#ifndef SPELLCHECKER_SPELLCHECKER_H
#define SPELLCHECKER_SPELLCHECKER_H

#include <string>
#include <unordered_map>
#include <vector>

/// @brief Calculates the levenshtein distance between two strings
/// @param a first string
/// @param b second string
/// @return number of edits needed to turn string a into b
int lev(const std::string& a, const std::string& b);

/// @brief calculate the distances from input to each word in the list and map
/// them
/// @param input word to calculate distnaces from
/// @param words list of words to which we want to calculate the distances
/// @return map containing each word in words as the key and distance from that
/// word to input as the value
std::unordered_map<std::string, int> baseListAroundWord(
    const std::string& input, const std::vector<std::string>& words);

/// @brief Finds the word that is closest to the input word in the list
/// @param input base word
/// @param words list of words
/// @param c constant that determines maximum tolerable deviation from the
/// closest distance
/// @return words from the list of words that are the closest to the input word
std::vector<std::string> findClosestWords(const std::string& input,
                                          const std::vector<std::string>& words,
                                          int c);

/// @brief Finds the word that is the closest to the input
/// @param input input word
/// @param clusterMap map of clusters where values are clusters (list of words)
/// and key is the most central word in the cluster
/// @return words closest to the input
std::vector<std::string> findClosestCandidates(
    const std::string& input,
    const std::unordered_map<std::string, std::vector<std::string>>&
        clusterMap);

#endif
