#include <clustering.h>
#include <spellchecker.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <ranges>
#include <string>
#include <unordered_map>

int readWordsFromFile(std::vector<std::string>& words,
                      const std::string& filePath);
int validateArgumentCount(int argc);

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

// Drivers code
int main(int argc, char* argv[]) {
    std::string filePath;
    if (validateArgumentCount(argc) != 0) {
        filePath = "../data/5000_words.txt";
    } else {
        filePath = argv[1];
    }

    std::vector<std::string> words;

    std::cout << "Reading file at " << filePath << " ... " << std::flush;

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

    int total = 0;

    for (const auto& clust : clusterMap) {
        total += clust.second.size();
    }

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

int validateArgumentCount(int argc) {
    if (argc < 2) {
        std::cout << "Provide the path to your lexicographical data\n";
        return -1;
    }

    if (argc > 2) {
        std::cerr << "Too many arguments. The program can only take one file "
                     "at a time\n";
        return -1;
    }

    return 0;
}

/// @brief Reads all lines from a file into the list of words
/// @param words list of words to be populated
/// @param filePath path to the file to read from
/// @return 0 on success, -1 if something went wrong
int readWordsFromFile(std::vector<std::string>& words,
                      const std::string& filePath) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "\n"
                  << "File at " << filePath << " could not be opened"
                  << "\n";
        return -1;
    }

    std::string line;

    while (getline(file, line)) {
        if (line.size() > 50) {
            std::cout << "The word " << line
                      << "was longer than 50 characters\n. Words longer "
                         "than 50 characters are not allowed\n. Ignoring "
                      << line << "\n";
            continue;
        }

        words.push_back(line);
    }

    file.close();

    if (words.empty()) {
        std::cerr << "\n"
                  << "File at " << filePath << " was empty" << "\n";
        return -1;
    }

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