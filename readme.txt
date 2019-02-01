compile using 
g++ -std=c++11 ./main.cpp -o dv

execute using (no argments) :
./dv

* problem 1 and 2 are done in the same program as shown below

* -std=c++11 is used while compiling because we are using range based for loops and 'auto' keyword.

* the program can also output the vertices ( with their tables ) and the edges for debugging.

sample output:
$ g++ -std=c++11 ./main.cpp -o dv
$ ./dv
Total number of messages passed to stabalize: 10

Give a new cost between nodes in this format: '(source_ip, destination_ip, new_edge_weight)' (braces are optional) to update the edge weight 
or 
press x to e(x)it, v to display all the (v)ertices, e for (e)dges: 
1.0.0.2, 1.0.0.4, 60
number of messages passed (without poisioned reverse): 98
number of messages passed (with poisioned reverse): 16


network used in the sample output: 
#include <string>

const std::string IPaddress = "0.0.0.0, 4.0.0.0, 0.0.0.1, 1.0.0.2, 4.0.0.4, 1.0.0.4";

const std::string Routers = "0.0.0.0, 4.0.0.0; 0.0.0.1, 1.0.0.2; 4.0.0.4, 1.0.0.4";

const std::string InterfaceDistance = "(4.0.0.0, 4.0.0.4, 1), (1.0.0.2, 1.0.0.4, 4), (0.0.0.0, 0.0.0.1, 50)";



