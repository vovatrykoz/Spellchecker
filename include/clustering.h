#include <unordered_map>
#include <functional>
#include <unordered_set>

template<typename T>
int sumOfDistances(const T& input, const std::list<T>& points, std::function<int(T, T)> distanceFunction) {
    int distance = 0;

    for(const auto& point : points) {
       distance += distanceFunction(input, point);
    }

    return distance;
}

template<typename T>
T findCentralMedoid(const std::list<T>& points, std::function<int(T, T)> distanceFunction) {
    T centralPoint = points.front();
    int shortestDistance = sumOfDistances(centralPoint, points, distanceFunction);
    int currentDistance;

    for(auto it = std::next(points.begin()); it != points.end(); ++it) {
        currentDistance = sumOfDistances(*it, points, distanceFunction);

        if(currentDistance < shortestDistance) {
            centralPoint = *it;
            shortestDistance = currentDistance;
        }
    }

    return centralPoint;
}

template<typename T, typename Container>
T findFurthestElement(const T& input, const Container& points, std::function<int(T, T)> distanceFunction) {
    T furthestPoint = *(points.begin());
    int furthestDistance = distanceFunction(input, furthestPoint);
    int currentDistance;

    for(auto it = std::next(points.begin()); it != points.end(); ++it) {
        currentDistance = distanceFunction(input, *it);

        if(currentDistance > furthestDistance) {
            furthestPoint = *it;
            furthestDistance = currentDistance;
        }
    }

    return furthestPoint;
}

/// @brief Partitions the given list of points into two clusters around the given central points (also known as medoids)
/// @param firstMedoid first central point. Used as a center for the first cluster
/// @param secondMedoid second central point. Used as a center for the second cluster
/// @param points list of points to be sorted into two clusters
/// @param distanceFunction function used to calculate distance between two points
/// @return map of clusters, where the key is the most central point in a cluster and value is the cluster itself
template<typename T>
std::unordered_map<T, std::list<T>> partitionIntoClusters(const T& firstMedoid, const T& secondMedoid, const std::unordered_set<T>& points, std::function<int(T, T)> distanceFunction) {
    std::unordered_map<T, std::list<T>> clusterMap;
    int distanceToFirst, distanceToSecond;

    for(const auto& point : points) {
        distanceToFirst = distanceFunction(point, firstMedoid);
        distanceToSecond = distanceFunction(point, secondMedoid);
            
        if(distanceToFirst < distanceToSecond) {
            clusterMap[firstMedoid].push_back(point);
        }
        else {
            clusterMap[secondMedoid].push_back(point);
        }
    }

    return clusterMap;
}

template<typename T>
std::unordered_map<T, std::list<T>> partitionIntoClusters(const std::list<T>& medoids, const std::list<T>& points, std::function<int(T, T)> distanceFunction) {
    std::unordered_map<T, std::list<T>> clusterMap;
    int distanceToFirst, distanceToSecond;

    //for each of the points, find the closest medoid and assign it there
    for(const auto& point : points) {
        int shortestDist = distanceFunction(medoids.front(), point);
        T closestPoint = medoids.front();

        for(auto it = std::next(medoids.begin()); it != medoids.end(); ++it) {
            int currentDistance = distanceFunction(*it, point);

            if(currentDistance < shortestDist) {
                shortestDist = currentDistance;
                closestPoint = *it;
            }
        }

        clusterMap[closestPoint].push_back(point);
    }

    return clusterMap;
}

template<typename T>
std::list<T> anomalousPatternInitialisation(const std::list<T>& points, std::function<int(T, T)> distanceFunction, std::function<T(const std::list<T>&)> centralityFunction) {
    std::list<T> medoids;

    // find the most central element
    T startingMedoid = findCentralMedoid(points, distanceFunction);

    std::unordered_set<T> remaining;

    for(const auto& point: points) {
        remaining.insert(point);
    }

    // repeat until we run out of points
    while(remaining.size() != 0) {

        //find the element furthest away from the central
        T furthestMedoid = findFurthestElement<T, std::unordered_set<T>>(startingMedoid, remaining, distanceFunction);
        std::unordered_map<T, std::list<T>> clusterMap = partitionIntoClusters(startingMedoid, furthestMedoid, remaining, distanceFunction);
        T newFurthestMedoid = centralityFunction(clusterMap[furthestMedoid]);

        // repeat until the furthest medoid stays the same between iterations
        while(newFurthestMedoid != furthestMedoid) {
            furthestMedoid = newFurthestMedoid;
            clusterMap = partitionIntoClusters(startingMedoid, furthestMedoid, remaining, distanceFunction);
            newFurthestMedoid = centralityFunction(clusterMap[furthestMedoid]);
        }

        // remove all the points that are in the furthest cluster from the list of remaining points
        for(const auto& point : clusterMap[newFurthestMedoid]) {
            auto result = remaining.find(point);
            if(result != remaining.end()) {
                remaining.erase(result);
            }
        }

        //add the medoid to the list
        medoids.push_back(newFurthestMedoid);
    }

    return medoids;
}

/// @brief Iteratively partitions the given list of points into clusters using the given medoids as the starting point. 
/// The algorithm stops when the central points do not change between iterations
/// @param startCentralMedoid central point
/// @param startFurthestMedoid the point that lies the furthest away from the central point
/// @param points list of all points to be sorted into clusters
/// @param distanceFunction function used to calculate distance between points
/// @param centralityFunction function that calculates the most central point in a list of points
/// @return map of clusters, where the key is the most central point in a cluster and value is the cluster itself
template<typename T>
std::unordered_map<T, std::list<T>> partitionAroundMedoids(const std::list<T>& points,
                                                            std::function<int(T, T)> distanceFunction,
                                                            std::function<T(const std::list<T>&)> centralityFunction) {
    
    std::unordered_map<T, std::list<T>> clusterMap;

    // find the most optimal medoids (points that will be used to represent clusters)
    auto medoids = anomalousPatternInitialisation(points, distanceFunction, centralityFunction);

    // split the list of points into clusters using the medoids (central points) calculated earlier
    return partitionIntoClusters(medoids, points, distanceFunction);
}

/// @brief Iteratively partitions the given list of points into clusters using the given medoids as the starting point. 
/// The algorithm stops when the central points do not change between iterations
/// @param startCentralMedoid central point
/// @param startFurthestMedoid the point that lies the furthest away from the central point
/// @param points list of all points to be sorted into clusters
/// @param distanceFunction function used to calculate distance between points
/// @return map of clusters, where the key is the most central point in a cluster and value is the cluster itself
template<typename T>
std::unordered_map<T, std::list<T>> partitionAroundMedoids(const std::list<T>& points, std::function<int(T, T)> distanceFunction) {
    return partitionAroundMedoids<T>(points, distanceFunction, [&distanceFunction](const std::list<T>& innerPoints) {
        return findCentralMedoid(innerPoints, distanceFunction);
    });
}