# TSP
[Traveling Salesman Problem]
Bonus project for 2018 NTHU CS Data Structure Course. The project is to solve 2 variations of classical TSP problem. 

In the first variation, each city has offered happiness point to the salseman. However, happiness point of each city can only be earned once. Also, the salse man has limited traveling time to explore cities. This means that the salseman need not explore all the cities. The goal is to earn maximum happiness point within the limited time. The route has to be start and end at the same city.

The second variation inherit the first one. However, each city offer its happiness point only during the period of open and close time. The salseman can spend time staying at the city until its open time to earn the happiness point.

To compile the project:
    g++ -std=c++11 -O3 main.cpp Tsp.cpp -o m.exe

Exection the project:
    ./m.exe [foldername]
    (The folder has to contain the map file: tp.data)
