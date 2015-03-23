#include "System.h"
#include<math.h>
#include <algorithm>
sss
#define pi 3.14159265358979323846

double deg2rad(double deg) { return (deg * pi/ 180); }
double rad2deg(double rad) { return (rad * 180 / pi); }

double distance(Coordinates & a, Coordinates & b)
{
		double theta, dist;
		theta = a.longitude - b.longitude;
		dist = sin(deg2rad(a.latitude)) * sin(deg2rad(b.latitude)) 
			+ cos(deg2rad(a.latitude)) * cos(deg2rad(b.latitude)) * cos(deg2rad(theta));
		dist = acos(dist);
		dist = rad2deg(dist);
		dist = dist * 111189.577;// 60 * 1.1515 * 1.609344 * 1000;
		return (dist);
}


double distance(House * a, House * b) { return distance(a->coordinates, b->coordinates); }
double distance(Garden * a, Garden * b) { return distance(a->coordinates, b->coordinates); }
double distance(Garden * a, House * b) { return distance(a->coordinates, b->coordinates); }
double distance(House * a, Garden * b) { return distance(a->coordinates, b->coordinates); }

bool compareGardens(Garden const * g1, Garden const * g2)
{
	return g1->avgDistance > g2->avgDistance;
}

bool compareLinks(Link const * l1, Link const * l2)
{
	return l1->distance < l2->distance;
}

void System::createLinks()
{
	std::cout << "Phase 1 ..." << std::endl;
	for (auto & garden : gardens)
	{
		double distSum = 0;
		for (auto & house : houses)
		{
			garden->links.push_back(new Link(garden, house, 0));
			distSum += garden->links.back()->distance;
		}

		std::sort(garden->links.begin(), garden->links.end(), compareLinks);

		garden->avgDistance = distSum / houses.size();
	}
	std::sort(gardens.begin(), gardens.end(), compareGardens);

	std::cout << "Phase 2 ..." << std::endl;
	auto garden = gardens.begin();
	for (;;)
	{
		if ((*garden)->unusedCapacity != 0)
			for (auto link : (*garden)->links)
				if (link->house->childrenWithNoGardenCount != 0)
				{
					int transfer = ((*garden)->unusedCapacity < link->house->childrenWithNoGardenCount)
						? (*garden)->unusedCapacity : link->house->childrenWithNoGardenCount;
					(*garden)->unusedCapacity -= transfer;
					link->house->childrenWithNoGardenCount -= transfer;
					link->childrenCount += transfer;
					break;
				}

		garden++;
		if (garden == gardens.end())
		{
			int totalUnusedCapacity = 0;
			for (auto & garden : gardens)
				totalUnusedCapacity += garden->unusedCapacity;
			int totalChildrenWithNoGardenCount = 0;
			for (auto & house : houses)
				totalChildrenWithNoGardenCount += house->childrenWithNoGardenCount;

			if (totalUnusedCapacity == 0 || totalChildrenWithNoGardenCount == 0) 
				break;

			garden = gardens.begin();
		}
	}


}

void System::calculateDistribution()
{
	std::cout << "Phase 1 ..." << std::endl;
	double overallDistSum = 0;
	for (auto & garden : gardens)
	{
		gardenCapacity += garden->capacity;
		double distSum = 0;
		double weightSum = 0;
		for (auto & link : garden->links)
		{
			distSum += link->distance * link->childrenCount;
			weightSum += link->childrenCount;

			link->house->distSum += link->distance * link->childrenCount;
			link->house->weightSum += link->childrenCount;
		}
		overallDistSum += distSum;
		garden->weightedAvgDistance = (weightSum == 0) ? 0 : distSum / weightSum;
	}

	std::cout << "Phase 2 ..." << std::endl;
	for (auto & house : houses)
	{
		childrenCount += house->childrenCount;
		house->sufficiency = (house->childrenCount == 0) ? 0
			: house->weightSum / house->childrenCount;
		house->weightedAvgDistance = (house->weightSum == 0) ? 0
			: house->distSum / house->weightSum;
	}
	std::cout << "Phase 2.5 ..." << std::endl;

	sufficiency = (childrenCount == 0) ? 0 : (double)gardenCapacity / (double) childrenCount;
	weightedAvgDistance = (childrenCount == 0) ? 0 : overallDistSum / childrenCount;

	std::cout << "Phase 3 ..." << std::endl;
	double overallLocalDeviation = 0;
	for (auto & garden : gardens)
	{
		double localDeviation = 0;
		double weightSum = 0;
		for (auto & link : garden->links)
		{
			overallLocalDeviation += (weightedAvgDistance - link->distance)*
				(weightedAvgDistance - link->distance) * link->childrenCount;
			localDeviation += (garden->weightedAvgDistance - link->distance)*
				(garden->weightedAvgDistance - link->distance) * link->childrenCount;

			link->house->localDeviation += (link->house->weightedAvgDistance - link->distance)*
				(link->house->weightedAvgDistance - link->distance) * link->childrenCount;

			weightSum += link->childrenCount;
		}
		garden->weightedAvgDistanceUniformity
			= (weightSum == 0) ? 0 : sqrt(localDeviation / weightSum);
		weightedAvgDistanceUniformity
			= (childrenCount == 0) ? 0 : sqrt(overallLocalDeviation / childrenCount);
	}

	std::cout << "Phase 4 ..." << std::endl;
	for (auto & house : houses)
		house->weightedAvgDistanceUniformity
		= (house->weightSum == 0) ? 0 : sqrt(house->localDeviation / house->weightSum);

}