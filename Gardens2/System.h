#include <iostream>
#include <vector>
#include <list>

struct Coordinates
{
	double longitude;
	double latitude;

	Coordinates(double longitude_, double latitude_)
		: longitude(longitude_), latitude(latitude_)
	{

	}
};

double distance(Coordinates &, Coordinates &);
double probaFromDistance(double dist);

struct Garden;
struct House;
struct Link;


struct Garden
{
	std::string id;
	Coordinates coordinates;
	int capacity;
	double availability;
	std::vector<Link*> links;
	double avgDistance = 0;
	double weightedAvgDistance = 0;
	double weightedAvgDistanceUniformity = 0;

	Garden(std::string id_, Coordinates coordinates_, int capacity_)
		: id(id_),
		coordinates(coordinates_),
		capacity(capacity_),
		availability(capacity_)
	{}

	bool operator > (Garden const& b) { return avgDistance < b.avgDistance; }
	bool operator < (Garden const& b) { return avgDistance > b.avgDistance; }

private:
	Garden(Garden const &) : coordinates(Coordinates(0, 0)){}
	Garden(Garden const &&) : coordinates(Coordinates(0, 0)) {};
	Garden & operator= (const Garden &) {};

};

struct House
{
	std::string id;
	Coordinates coordinates;
	double weightedAvgDistance = 0;
	double weightedAvgDistanceUniformity = 0;
	double sufficiency = 0;

	double distSum = 0;
	double weightSum = 0;
	double localDeviation = 0;

	int childrenCount;
	double availability;

	std::vector<Link*> links;

	House(std::string id_, Coordinates coordinates_, int childrenCount_)
		: id(id_),
		coordinates(coordinates_),
		childrenCount(childrenCount_),
		availability(childrenCount_)
	{}

private:
	House(House const &) : coordinates(Coordinates(0, 0)){}
	House(House const &&) : coordinates(Coordinates(0, 0)) {};
	House & operator= (const House &) {};
};

double distance(House *, House *);
double distance(Garden *, Garden *);
double distance(Garden *, House *);
double distance(House *, Garden *);

struct Link
{
	Garden * garden;
	House * house;
	double availability = 0;
	double distance = 0;

	Link(Garden* garden_, House* house_, double availability_)
		: garden(garden_), house(house_), availability(availability_)
		, distance(::distance(garden, house))
	{}

private:
	Link(Link const &) {}
	Link(Link const &&) {};
	Link & operator= (const Link &) {};
};

struct System
{
	std::vector<Garden*> gardens;
	std::vector<House*> houses;

	int childrenCount = 0;
	int gardenCapacity = 0;
	double weightedAvgDistance = 0;
	double weightedAvgDistanceUniformity = 0;
	double sufficiency = 0;

	double minimalTransfer = 0.01;

	void createLinks();
	void calculateDistribution();
};