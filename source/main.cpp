#include <clustering.h>
#include <spellchecker.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <ranges>
#include <span>
#include <string>
#include <unordered_map>

int readWordsFromFile(std::vector<std::string>& words,
                      const std::string& filePath);

void printDistanceMap(const std::unordered_map<std::string, int>& distanceMap);
void printWelcomeInfo();
void printClusterRepresentedBy(
    const std::string& representative,
    const std::unordered_map<std::string, int>& cluster);
void printClusterRepresentedBy(const std::string& representative,
                               const std::vector<std::string>& cluster);
void printClusterMap(
    const std::unordered_map<std::string, std::vector<std::string>>&
        clusterMap);
void printListOfWords(const std::vector<std::string>& words);

int main(int argc, char* argv[]) {
    const std::span<char*> args(argv, static_cast<std::size_t>(argc));

    if (argc < 2) {
        std::cout << "Provide the path to your lexicographical data\n";
        return 1;
    } else if (argc > 2) {
        std::cerr << "Too many arguments. Only one file allowed\n";
        return 1;
    }

    const std::string filePath = args[1];

    std::vector<std::string> words;

    if (readWordsFromFile(words, filePath) != 0) {
        return -1;
    }

    std::cout << "Done!" << "\n";

    std::cout << "Forming clusters" << "... " << std::flush;
    auto start = std::chrono::high_resolution_clock::now();
    auto clusterMap = partitionAroundMedoids<std::string>(words, &lev);
    auto stop = std::chrono::high_resolution_clock::now();

    const auto sduration =
        std::chrono::duration_cast<std::chrono::seconds>(stop - start);

    std::cout << "Done in " << sduration.count() << " s!" << "\n"
              << "\n";

    std::string input = "";

    std::cout << "Enter your word and the program will try to correct it"
              << "\n"
              << "\n";
    printWelcomeInfo();

    while (true) {
        std::cout << "Word: ";
        std::cin >> input;

        // unfortunatelly, C++ doesn't allow to have switch statements for
        // strings

        if (input == "/q") {
            break;
        }

        if (input == "/cent") {
            std::cout
                << "Finding the most central word in the original word list"
                << "... " << std::flush;

            const std::string centralWord =
                findCentralMedoid<std::string>(words, &lev);

            std::cout << "Done!" << "\n";

            std::cout << "Calculating distances to all other words" << "... "
                      << std::flush;

            const auto distanceMap = baseListAroundWord(centralWord, words);
            std::cout << "Done!" << "\n";

            printClusterRepresentedBy(centralWord, distanceMap);

            continue;
        }

        if (input == "/clus") {
            printClusterMap(clusterMap);
            continue;
        }

        if (input == "/help") {
            std::cout << "\n";
            printWelcomeInfo();
            continue;
        }

        start = std::chrono::high_resolution_clock::now();

        std::vector<std::string> suggestions =
            findClosestCandidates(input, clusterMap);

        stop = std::chrono::high_resolution_clock::now();

        const auto mduration =
            std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::ranges::sort(suggestions,
                          [&input](const std::string& a, const std::string& b) {
                              return lev(input, a) < lev(input, b);
                          });

        std::cout << "Corrections (" << mduration.count()
                  << " microseconds):" << "\n";
        printListOfWords(suggestions);
        std::cout << "\n";
    }

    return 0;
}

/// @brief Reads all lines from a file into the list of words
/// @param words list of words to be populated
/// @param filePath path to the file to read from
/// @return 0 on success, -1 if something went wrong
int readWordsFromFile(std::vector<std::string>& words,
                      const std::string& filePath) {
    std::cout << "Reading file at " << filePath << " ... \n\n" << std::flush;

    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "File at " << filePath << " could not be opened"
                  << "\n";
        return -1;
    }

    std::string line;

    std::ofstream duplicateLog;
    std::ofstream longLog;

    int longCounter = 0;
    bool longLimitExceeded = false;

    int repeatedCounter = 0;
    bool repeatedLimitExceeded = false;

    std::unordered_set<std::string> loadedWords;

    while (getline(file, line)) {
        if (line.size() > 50) {
            longCounter++;

            if (!longLimitExceeded) {
                if (longCounter <= 10) {
                    if (!longLog.is_open()) {
                        longLog.open("too_long_words.txt");

                        std::cout << "Logging too-long words to "
                                     "'too_long_words.txt'.\n";
                    }
                    longLog << line << "\n";

                    std::cout
                        << "Word exceeds 50 characters: \"" << line
                        << "\".\n"
                           "Words longer than 50 characters are not allowed. "
                           "Skipping.\n\n";
                } else {
                    longLimitExceeded = true;
                    std::cout
                        << "More than 10 words exceed 50 characters. Further "
                           "messages will be suppressed.\n\n";
                }
            }
            continue;
        }

        std::string lowerCaseLine = line;
        std::transform(lowerCaseLine.begin(), lowerCaseLine.end(),
                       lowerCaseLine.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (loadedWords.contains(lowerCaseLine)) {
            repeatedCounter++;

            if (!duplicateLog.is_open()) {
                duplicateLog.open("duplicates.txt");
                std::cout << "Logging duplicate words to 'duplicates.txt'.\n";
            }

            duplicateLog << line << "\n";

            if (!repeatedLimitExceeded) {
                if (repeatedCounter <= 15) {
                    std::cout << "Duplicate word found: \"" << line
                              << "\". Skipping.\n\n";
                } else {
                    repeatedLimitExceeded = true;
                    std::cout << "More than 15 duplicate words found. Further "
                                 "messages will be suppressed.\n\n";
                }
            }
            continue;
        }

        loadedWords.insert(lowerCaseLine);
    }

    if (duplicateLog.is_open()) {
        duplicateLog.close();
        std::cout << "Duplicates logged to 'duplicates.txt'\n";
    }

    if (longLog.is_open()) {
        longLog.close();
        std::cout << "Long words logged to 'too_long_words.txt'\n";
    }

    file.close();

    words = std::vector(loadedWords.begin(), loadedWords.end());

    if (words.empty()) {
        std::cerr << "\n"
                  << "File at " << filePath << " was empty" << "\n";
        return -1;
    }

    std::cout << "Skipped " << longCounter
              << " word(s) longer than 50 characters\n";
    std::cout << "Skipped " << repeatedCounter << " duplicate word(s)\n\n";

    return 0;
}

void printDistanceMap(const std::unordered_map<std::string, int>& distanceMap) {
    std::cout << "\n";
    for (const auto& wordDistancePair : distanceMap) {
        std::cout << wordDistancePair.first << ": " << wordDistancePair.second
                  << "\n";
    }
    std::cout << "\n";
}

void printWelcomeInfo() {
    std::cout << "Special commands:" << "\n" << "\n";
    std::cout << "/q - quit the program" << "\n";
    std::cout << "/cent - calculate the most central element of the list and "
                 "print the Levenshtein distance between all elements and the "
                 "most central node"
              << "\n";
    std::cout << "/clus - print the clusters found by the program" << "\n"
              << "\n";
    std::cout << "/help - print this information again" << "\n"
              << "\n";
}

void printClusterRepresentedBy(
    const std::string& representative,
    const std::unordered_map<std::string, int>& cluster) {
    std::cout << "\n" << representative << ":" << "\n";
    for (const auto& wordDistancePair : cluster) {
        std::cout << "\t" << wordDistancePair.first << ": "
                  << wordDistancePair.second << "\n";
    }
    std::cout << "\n";
}

void printClusterRepresentedBy(const std::string& representative,
                               const std::vector<std::string>& cluster) {
    std::cout << "\n" << representative << ":" << "\n";
    for (const auto& wordDistancePair : cluster) {
        std::cout << "\t" << wordDistancePair << "\n";
    }
    std::cout << "\n";
}

void printClusterMap(
    const std::unordered_map<std::string, std::vector<std::string>>&
        clusterMap) {
    std::cout << "\n";

    for (const auto& wordClusterPair : clusterMap) {
        std::cout << wordClusterPair.first << " ("
                  << wordClusterPair.second.size() << "):" << "\n";

        for (const auto& word : wordClusterPair.second) {
            std::cout << "\t" << word << "\n";
        }
    }

    std::cout << "\n";
}

void printListOfWords(const std::vector<std::string>& words) {
    for (const auto& word : words) {
        std::cout << "\t" << word << "\n";
    }
}
