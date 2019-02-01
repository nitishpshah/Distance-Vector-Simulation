/*

	writtten by : Nitish Shah 	160030005
				: Neha Shah 	160020001
	
		as part of the computer networks lab course, 
		year 2, semester 4, IIT Goa.

	Date: 16th April 2018	

*/


#include <iostream>
#include <vector>
#include <string>

using namespace std;

#include "graph.h"//includes <iostream> <vector> <algorithm><string><limits><map><queue>
#include "network.h"
#include <sstream>


string convert_itos(size_t x);
size_t convert_stoi(string temp);
string trim(const string& str, const string& whitespace = " \t");


int main(){

	/*
		the network only has routers
	*/
	graph<string, int> network;

	int router_count = 1;	//keeps the count of dummy nodes

	vector<string> router_interfaces;	//store all interfaces of a router
	router_interfaces.clear();

	stringstream routers_string(Routers);
	string router_string, interface;
	vector<string> routers;

	/*stores which router number of each interface, used in changing edge weights*/
	map<string, string> router_of;
	/*stores the interfaces of each router*/
	multimap<string, string> interfaces_of;

	/*add routers to the network and build router_of and interfaces_of maps*/
	while (getline(routers_string, router_string, ';')){

		string router_name = convert_itos(router_count);
		routers.push_back(router_name);
		router_count += 1;

		router_interfaces.clear();
		stringstream interfaces(router_string);
		while (getline(interfaces, interface, ',')){
			interface = trim(interface);
			//all routers stored in the vector router_interfaces
			router_interfaces.push_back(interface);
			router_of[interface] = router_name;
			interfaces_of.insert(make_pair(router_name, interface));
		}
	}

	//add edges between the routers
	stringstream interface_edges(InterfaceDistance);
	string interface_edge, source, destination, weight_string;
	int weight;
	while (getline(interface_edges, interface_edge, '(')){
		if (interface_edge == ""){
			continue;
		}
		stringstream edge(interface_edge);
		getline(edge, source, ',');
		getline(edge, destination, ',');
		getline(edge, weight_string, ')');
		source = trim(source, ", \t()");
		destination = trim(destination, " \t(),");
		weight_string = trim(weight_string, " \t(),");
		weight = convert_stoi(weight_string);
		//add undirected edges
		network.insert_edge(router_of[source], router_of[destination], weight, true);
	}
	
	/*stabalize and print the total messages passed*/
	cout<<"Total number of messages passed to stabalize: "<<network.update_start()<<endl;

	string source_ip_address, destination_ip_address, option;
	while (true){
		cout << "Give a new cost between nodes in this format: '(source_ip, destination_ip, new_edge_weight)' (braces are optional) to update the edge weight \nor \npress x to e(x)it, v to display all the (v)ertices, e for (e)dges: \n";
		getline(cin, option);
		if (option == "v"){
			for (const auto& vertex : network.vertices){
				string router_name = vertex.data;
				cout<<router_name<<endl;
				auto range = interfaces_of.equal_range(router_name);
				for (auto i = range.first; i != range.second; i++){
					cout<<"\tinterfaces: "<<i->second<<endl;
				}
			}
			network.display_vertices();
			continue;
		}
		if (option == "x"){
			return 0;
		}
		if (option == "e"){
			network.display_edges();
			continue;
		}else{
			/*the user didnt choose any other option, assume that the user wants to update an edge weight*/
			
			stringstream new_edge(option);
			getline(new_edge, source, ',');
			getline(new_edge, destination, ',');
			getline(new_edge, weight_string, ')');
			source = trim(source, ", \t()");
			destination = trim(destination, " \t(),");
			weight_string = trim(weight_string, " \t(),");
			weight = convert_stoi(weight_string);
			
			int previous_weight = network.vertices[network.vertex_indices[router_of[source]]].edge_weight(network.vertex_indices[router_of[destination]]);
			
			cout << "number of messages passed (without poisioned reverse): " << network.update_edge(router_of[source], router_of[destination], weight) << endl;
			/*change back the updated edge weight to previous*/
			network.update_edge(router_of[source], router_of[destination], previous_weight);
			cout << "number of messages passed (with poisioned reverse): " << network.update_edge(router_of[source], router_of[destination], weight, 1) << endl <<endl;
		}
	}
	return 0;
}

string convert_itos(size_t x){
	string ret = "";
	int rem = 0;
	while (x > 0){
		rem = x % 10;
		ret += (char)(x + 48);
		x = x / 10;
	}
	return ret;
}

size_t convert_stoi(string temp){
	size_t ret = 0;
	ret = int(temp[0]) - 48;
	for (size_t iface = 1; iface < temp.size(); ++iface){
		ret *= 10;
		ret += int(temp[iface]) - 48;
	}
	return ret;
}

string trim(const string& str, const string& whitespace){
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}