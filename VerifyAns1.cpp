#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
using namespace std;

class City {
public:
    City() {};
    City(short _i, string _n, int _hapiness, int _s, int _c)
        :index(_i), name(_n), hapiness(_hapiness), start(_s), close(_c) {};
    string name;
    short index;
    int hapiness, start, close;
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

City* findCityByName(vector<City> &, const string &);
Edge* findEdgeByName(vector<Edge> &, const string &, const string &);
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout <<  "Missing command argument. Please specify the path to tp.data." << endl;
        return 1;
    }

    string filePath = string(argv[1]);
    ifstream in(filePath + "/tp.data");
    if (!in.is_open()) {
        cout << "Unable to open \"" 
            << filePath << "/tp.data\". Please check existance of file or path." << endl;
        return 1;
    }

    int nodes, edgeNum, totalTimeQuota, startTime;
    City *current, *next;
    vector<City> cities;
    vector<Edge> edges;
    int ans1Hapiness, ans1Time;
    vector<FootPrint> footPrints;
    
    string _tmp;
    getline(in, _tmp);
    stringstream ss(_tmp);
    ss >> nodes >> edgeNum >> totalTimeQuota >> startTime;
    cities.reserve(nodes);
    edges.reserve(edgeNum);

    map<string, int> cityNameAndIndex;
    short count = nodes;
    while(count--) {
        getline(in, _tmp);
        ss.clear(); ss.str(_tmp);
        string name;
        int hapiness, start, close;
        ss >> name >> hapiness >> start >> close;
        int index = cities.size();
        cities.emplace_back(index, name, hapiness, start, close);
        cityNameAndIndex[name] = index;
    }

    count = edgeNum;
    while(count--) {
        getline(in, _tmp);
        ss.clear(); ss.str(_tmp);
        string name1, name2;
        int travelTime;
        ss >> name1 >> name2 >> travelTime;

        int index_city1 = cityNameAndIndex.find(name1)->second;
        City &c1 = cities[index_city1];

        int index_city2 = cityNameAndIndex.find(name2)->second;
        City &c2 = cities[index_city2];
        edges.emplace_back(&c1, &c2, travelTime);
    }

    in.close(); in.clear(); in.open(filePath + "/ans1.txt");
    if (!in.is_open()) {
        cout << "Unable to open \"" 
            << filePath << "/ans1.txt\". Please check existance of file or path." << endl;
        return 1;
    }

    getline(in, _tmp);
    ss.clear();ss.str(_tmp);
    ss >> ans1Hapiness >> ans1Time;
    while(getline(in, _tmp)) {
        ss.clear(); ss.str(_tmp);
        string name; int arriving, leaving;
        ss >> name >> arriving >> leaving;
        footPrints.emplace_back(name, arriving, leaving);
    }

    ofstream o(filePath+"/verifyAns1.log");
    int earned = 0, timeQuota = totalTimeQuota, currentTime = startTime;
    bool hasError = false;

    // verify starting city
    FootPrint &f = footPrints.front();
    current = findCityByName(cities, f.name);
    if (f.name != current->name) {
        o << "start City name do not match !" << endl;
        cout << "ans1.txt has ERROR !" << endl << endl;
        return 0;
    }
    if (f.arriving != startTime) {
        o << "start City arriving time dost not match !" << endl;
        cout << "ans1.txt has ERROR !" << endl << endl;
        return 0;
    }
    if (f.leaving != startTime) {
        o << "start City leaving time dost not match !" << endl;
        cout << "ans1.txt has ERROR !" << endl << endl;
        return 0;
    }
    for (int i = 0 ; i < footPrints.size() ; ++i) {
        FootPrint &f = footPrints[i];
        City &next = *findCityByName(cities, f.name);
        o << "currentTime: " << currentTime << ", timeQuota: " << timeQuota << ", Hapiness: " << earned << ", "
            << "going to: ["<<next.name<<", visited:"<<(next.visited?"True":"False")<<", hapiness: "<<next.hapiness<<"]";
        o.flush();
        
        if (i == 0) {
            next.visited = true;
            earned += next.hapiness;
            o << endl;
            continue;
        }
        Edge &e = *findEdgeByName(edges, current->name, next.name);
        // cout << "current->name: " << current->name << ", next.name: " << next.name << endl;
        o << ", travelTime required: " << e.travelTime << ", ";
        o.flush();

        currentTime += e.travelTime;
        timeQuota -= e.travelTime;

        if (f.arriving != currentTime) {
            o << next.name << "'s arriving time does not match. Calculated arriving"; o.flush();
            cout << "ans1.txt has ERROR!" << endl << endl;
            return 0;
        }
        if (f.leaving != currentTime) {
            o << next.name << "'s arriving time does not match. Calculated arriving"; o.flush();
            cout << "ans1.txt has ERROR!" << endl << endl;
            return 0;
        }

        if (!next.visited) {
            next.visited = true;
            earned += next.hapiness;
        }
        current = &next;
        o << "[" << next.name << " " << f.arriving << " " << f.leaving << "] has been verified." << endl; o.flush();
    }
    if (earned != ans1Hapiness) {
        o  << "Earned happiness does not match! Calculted earned: " << earned << ", hapiness in ans1.txt: " << ans1Hapiness << endl; o.flush();
        cout << "ans1.txt has ERROR!" << endl << endl;
        hasError = true;
    } else {
        o  << "Earned happiness matched!" << endl;
    }
    
    int travelTime = currentTime - startTime;
    if (travelTime != ans1Time) {
        o  << "Traveling time does not match! Calculted time: " << currentTime << ", Traveling time in ans1.txt: " << ans1Time << endl; o.flush();
        cout << "ans1.txt has ERROR!" << endl << endl;
        hasError = true;
    } else {
        o  << "Taveling time matched!" << endl; o.flush();
    }
    o << ss.rdbuf();
    if (!hasError) {
        cout << "Verification complete. ans1.txt is OK!" << endl << endl;
    }
};

City* findCityByName(vector<City> &v, const string &name) {
    for (int i = 0 ; i < v.size() ; ++i) {
        if (name == v[i].name) {
            return &v[i];
        }
    }
    return nullptr;
};

Edge* findEdgeByName(vector<Edge> &e, const string &name1, const string &name2) {
    for (int i = 0 ; i < e.size() ; ++i) {
        string ecname1 = e[i].city1->name;
        string ecname2 = e[i].city2->name;
        if ((name1 == ecname1 && name2 == ecname2)
                || (name1 == ecname2 && name2 == ecname1) ) {
            return &e[i];
        }
    }
    return nullptr;
};