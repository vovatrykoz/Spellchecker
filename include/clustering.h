#ifndef _CLUSTERING_H_
#define _CLUSTERING_H_

#include <functional>
#include <future>
#include <vector>
#include <thread>
#include <unordered_map>
#include <unordered_set>

template <typename T>
struct ObjectDistance {
    T object;
    int distance;
};

template <typename T>
inline int sumOfDistances(const T& input, const std::vector<T>& points,
                   const std::function<int(T, T)>& distanceFunction) {
    int distance = 0;

    for (const auto& point : points) {
        distance += distanceFunction(input, point);
    }

    return distance;
}

template <typename T>
inline void removeItemsFromSet(std::unordered_set<T>& set,
                        const std::vector<T>& itemsToRemove) {
    for (const auto& item : itemsToRemove) {
        auto result = set.find(item);

        if (result != set.end()) {
            set.erase(result);
        }
    }
}

template <typename T>
inline T findCentralMedoid(const std::vector<T>& points,
                    const std::function<int(T, T)>& distanceFunction) {
    if (points.size() == 0) {
        return T();
    }

    ObjectDistance<T> centralPoint = {
        points.front(),
        sumOfDistances(points.front(), points, distanceFunction)};

    auto blockStart = points.begin();

    // some of the code is taken from C++ Concurrency in Action, 2nd edition by
    // Anthony Williams
    unsigned long const length = std::distance(
        blockStart, points.end());  // linear, would be more efficient with
                                    // random access iterators

    if (!length) {
        return centralPoint.object;
    }

    unsigned long const minPerThread = 25;
    unsigned long const hardwareThreads = std::thread::hardware_concurrency();
    unsigned long const maxThreads = (length + minPerThread - 1) / minPerThread;
    // size of a block we want to send to the async function
    unsigned long const numThreads =
        std::min(hardwareThreads != 0 ? hardwareThreads : 2, maxThreads);
    unsigned long const blockSize = length / numThreads;

    std::list<std::future<ObjectDistance<T>>> centralCandidates;

    for (unsigned long i = 0; i < (numThreads - 1); i++) {
        auto blockEnd = blockStart;
        // linear
        std::advance(blockEnd, blockSize);

        // each task will find the most central object among the subset that has
        // been given to it
        centralCandidates.push_back(std::async(
            std::launch::async,
            [blockStart, blockEnd, centralPoint, &points, &distanceFunction]() {
                int currentDistance;
                ObjectDistance<T> innerCentralPoint = centralPoint;

                for (auto it = blockStart; it != blockEnd; ++it) {
                    currentDistance =
                        sumOfDistances(*it, points, distanceFunction);

                    if (currentDistance < innerCentralPoint.distance) {
                        innerCentralPoint = {*it, currentDistance};
                    }
                }

                return innerCentralPoint;
            }));

        blockStart = blockEnd;
    }

    // find the most central point among the candidates
    for (auto& futureCandidates : centralCandidates) {
        ObjectDistance<T> currentCandidate = futureCandidates.get();

        if (currentCandidate.distance < centralPoint.distance) {
            centralPoint = currentCandidate;
        }
    }

    return centralPoint.object;
}

template <typename T, typename Iterator>
inline T findFurthestElement(const T& input, Iterator first, Iterator end,
                      const std::function<int(T, T)>& distanceFunction) {
    T furthestPoint = *(first);
    int furthestDistance = distanceFunction(input, furthestPoint);
    int currentDistance;

    for (auto it = std::next(first); it != end; ++it) {
        currentDistance = distanceFunction(input, *it);

        if (currentDistance > furthestDistance) {
            furthestPoint = *it;
            furthestDistance = currentDistance;
        }
    }

    return furthestPoint;
}

/// @brief Partitions the given list of points into two clusters around the
/// given central points (also known as medoids)
/// @param firstMedoid first central point. Used as a center for the first
/// cluster
/// @param secondMedoid second central point. Used as a center for the second
/// cluster
/// @param points list of points to be sorted into two clusters
/// @param distanceFunction function used to calculate distance between two
/// points
/// @return map of clusters, where the key is the most central point in a
/// cluster and value is the cluster itself
template <typename T>
inline std::unordered_map<T, std::vector<T>> partitionIntoClusters(
    const T& firstMedoid, const T& secondMedoid,
    const std::unordered_set<T>& points,
    const std::function<int(T, T)>& distanceFunction) {
    std::unordered_map<T, std::vector<T>> clusterMap;
    int distanceToFirst, distanceToSecond;

    for (const auto& point : points) {
        distanceToFirst = distanceFunction(point, firstMedoid);
        distanceToSecond = distanceFunction(point, secondMedoid);

        if (distanceToFirst < distanceToSecond) {
            clusterMap[firstMedoid].push_back(point);
        } else {
            clusterMap[secondMedoid].push_back(point);
        }
    }

    return clusterMap;
}

template <typename T>
inline std::unordered_map<T, std::vector<T>> partitionIntoClusters(
    const std::vector<T>& medoids, const std::vector<T>& points,
    const std::function<int(T, T)>& distanceFunction) {
    std::unordered_map<T, std::vector<T>> clusterMap;
    int distanceToFirst, distanceToSecond;

    // for each of the points, find the closest medoid and assign it there
    for (const auto& point : points) {
        int shortestDist = distanceFunction(medoids.front(), point);
        T closestPoint = medoids.front();

        for (auto it = std::next(medoids.begin()); it != medoids.end(); ++it) {
            int currentDistance = distanceFunction(*it, point);

            if (currentDistance < shortestDist) {
                shortestDist = currentDistance;
                closestPoint = *it;
            }
        }

        clusterMap[closestPoint].push_back(point);
    }

    return clusterMap;
}

template <typename T>
inline std::vector<T> anomalousPatternInitialisation(
    const std::vector<T>& points,
    const std::function<int(T, T)>& distanceFunction,
    const std::function<T(const std::vector<T>&)>& centralityFunction) {
    std::vector<T> medoids;

    // find the most central element
    T startingMedoid = findCentralMedoid(points, distanceFunction);

    std::unordered_set<T> remaining;

    for (const auto& point : points) {
        remaining.insert(point);
    }

    // repeat until we run out of points
    while (remaining.size() != 0) {
        // find the element furthest away from the central
        T furthestMedoid =
            findFurthestElement<T>(startingMedoid, remaining.begin(),
                                   remaining.end(), distanceFunction);
        std::unordered_map<T, std::vector<T>> clusterMap = partitionIntoClusters(
            startingMedoid, furthestMedoid, remaining, distanceFunction);

        T newFurthestMedoid;
        do {
            newFurthestMedoid = centralityFunction(clusterMap[furthestMedoid]);
            furthestMedoid = newFurthestMedoid;
            clusterMap = partitionIntoClusters(startingMedoid, furthestMedoid,
                                               remaining, distanceFunction);
        } while (newFurthestMedoid != furthestMedoid);

        // remove all the points that are in the furthest cluster from the set
        // of remaining points
        removeItemsFromSet(remaining, clusterMap[newFurthestMedoid]);

        // add the medoid to the list
        medoids.push_back(newFurthestMedoid);
    }

    return medoids;
}

/// @brief Iteratively partitions the given list of points into clusters using
/// the given medoids as the starting point. The algorithm stops when the
/// central points do not change between iterations
/// @param startCentralMedoid central point
/// @param startFurthestMedoid the point that lies the furthest away from the
/// central point
/// @param points list of all points to be sorted into clusters
/// @param distanceFunction function used to calculate distance between points
/// @param centralityFunction function that calculates the most central point in
/// a list of points
/// @return map of clusters, where the key is the most central point in a
/// cluster and value is the cluster itself
template <typename T>
inline std::unordered_map<T, std::vector<T>> partitionAroundMedoids(
    const std::vector<T>& points, std::function<int(T, T)> distanceFunction,
    const std::function<T(const std::vector<T>&)>& centralityFunction) {
    std::unordered_map<T, std::vector<T>> clusterMap;

    // find the most optimal medoids (points that will be used to represent
    // clusters)
    auto medoids = anomalousPatternInitialisation(points, distanceFunction,
                                                  centralityFunction);

    // split the list of points into clusters using the medoids (central points)
    // calculated earlier
    return partitionIntoClusters(medoids, points, distanceFunction);
}

/// @brief Iteratively partitions the given list of points into clusters using
/// the given medoids as the starting point. The algorithm stops when the
/// central points do not change between iterations
/// @param startCentralMedoid central point
/// @param startFurthestMedoid the point that lies the furthest away from the
/// central point
/// @param points list of all points to be sorted into clusters
/// @param distanceFunction function used to calculate distance between points
/// @return map of clusters, where the key is the most central point in a
/// cluster and value is the cluster itself
template <typename T>
inline std::unordered_map<T, std::vector<T>> partitionAroundMedoids(
    const std::vector<T>& points,
    const std::function<int(T, T)>& distanceFunction) {
    return partitionAroundMedoids<T>(
        points, distanceFunction,
        [&distanceFunction](const std::vector<T>& innerPoints) {
            return findCentralMedoid(innerPoints, distanceFunction);
        });
}

#endif