#ifndef _SPELLCHECKER_H_
#define _SPELLCHECKER_H_

#include <string>
#include <unordered_map>
#include <list>

#include "clustering.h"

/// @brief Calculates the levenshtein distance between two strings
/// @param a first string
/// @param b second string
/// @return number of edits needed to turn string a into b
int lev(std::string a, std::string b);

/// @brief calculate the distances from input to each word in the list and map them
/// @param input word to calculate distnaces from
/// @param words list of words to which we want to calculate the distances
/// @return map containing each word in words as the key and distance from that word to input as the value
std::unordered_map<std::string, int> baseListAroundWord(const std::string& input, const std::list<std::string>& words);

/// @brief Finds the word that is closest to the input word in the list
/// @param input base word
/// @param words list of words
/// @param c constant that determines maximum tolerable deviation from the closest distance
/// @return word from the list of words that is the closest to the input word
std::list<std::string> findClosestWords(const std::string& input, const std::list<std::string>& words, int c);

/// @brief Finds the word that is the closest to the input
/// @param input input word
/// @param clusterMap map of clusters where values are clusters (list of words) and key is the most central word in the cluster 
/// @return the closest word to the input
std::list<std::string> findClosestCandidates(const std::string& input, const std::unordered_map<std::string, std::list<std::string>>& clusterMap);

#endif