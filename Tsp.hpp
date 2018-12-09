/* This code is designed to calculate 2 kinds of variation TSP problem
    with no more than 100 citys
    with execution time less than 30 sec.
*/
#pragma once
#ifndef _TSP_YUNJING_
#define _TSP_YUNJING_
#include <vector>
#include <map>
using namespace std;

class City {
public:
    City() {};
    City(short _i, string _n, int _hapiness, int _s = 0, int _c = 1400)
        :index(_i), name(_n), hapiness(_hapiness), start(_s), close(_c) {};
    string name;
    short index;
    int hapiness = 0,
        start = 0, close = 1440;
    bool visited = false;
};

class Edge {
public:
    Edge() {};
    Edge(City* _c1, City* _c2, int _t)
        : city1(_c1), city2(_c2), travelTime(_t) {};
    City* city1;
    City* city2;
    int travelTime;
};
class FootPrint {
public:
    FootPrint() {};
    FootPrint(string _name, int _arriving, int _leaving)
        :name(_name), arriving(_arriving), leaving(_leaving) {};
    string name;
    int arriving, leaving;
};
class Record {
public:
    int hapiness, time;
    vector<FootPrint> path;
};

class Tsp {
public:
    Tsp() {};
    ~Tsp();
    bool hasError();
    void openTime(bool b);
    void read(int &argc, char* argv[]);
    void simulatedAnnealing();
    void findSolution();
    // void outputResult();

    /* for test */
    void printData();
    void saveShortestTravelTime();
    void saveCameFrom();

private:
    bool errorFlag = false;
    bool takeOpentime = true;
    bool reseted = true;

    short chooseCity = 4;

    int nodes = 0, edgeNum = 0,
        totalTimeQuota = 0, startTime = 480;
    vector<vector<int>> shortestTravelTime, cameFrom;

    string filePath;
    vector<City> cities;
    vector<Edge> edges;
    Record greedyResult;
    
    void reset(); // reset data for a new round of algorithm.
    void saveFileSub1(Record &record);
    void calcWarshall(); // use warshall to calculate all shortestTravelTime
    // void recordPath(Record &record, City* city, int arriving, int leaving);

    void sub1_greedy();
    void greedy_filterUnvisited(vector<City*> &unvisited, City* start, City* current, int timeQuota);
    void greedy_calcCP(vector<City*> &unvisited, map<float, City*> &cpRatio, City* source);
    void greedy_goto(City* source, City* destination, 
        int &currentTime, int &totalEarned, int &timeQuota);

    /* for test*/
    void printCities();
    void printEdges();
    void printShortestTravelTime();
    void printCamefrom();
    void showVisitStatus();
};
#endif /* _TSP_YUNJING_ */