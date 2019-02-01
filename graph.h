#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <limits>
#include <map>
#include <queue>


template <class dataStruct, class weight_type>
class edge;

template <class dataStruct, class weight_type>
class graph_node
{
public:
	dataStruct data;
	std::vector< edge<dataStruct, weight_type> > edges; 	//adjacency list of the nodes (contains pointers of all the neighbours)
	size_t prev;	//index of the previous node in the shortest path tree after running dijkstra
	weight_type dist;	//distance from the source after applying dijkstra
	/*
		the order of the vertices stays the same for each node in the network
	*/
	std::vector<std::vector<size_t>> matrix;//the distance vectors of all neighbours

	/* used for poisioned reverse part and for printing the shortest path */
	std::vector<size_t> forwarding_table;//forwarding_table[destination] = neighbour to which the message is passed through
	size_t own;//index of own
	
	graph_node(){
		/*never used here*/
		data = 0;	//interpret zero based on the type
		edges.clear();
		forwarding_table.clear();
		prev = 0;
		dist = std::numeric_limits<weight_type>::max();
	}

	graph_node(dataStruct d)
	{
		data = d;
		edges.clear();
		forwarding_table.clear();
		//edge<dataStruct, weight_type> new_edge(own, 0);
		//edges.push_back(new_edge);
		prev = 0;
		dist = std::numeric_limits<weight_type>::max();
	}
	graph_node(dataStruct d, weight_type own_index){
		own = own_index;
		data = d;
		edges.clear();
		forwarding_table.clear();
		//edge<dataStruct, weight_type> new_edge(own, 0);
		//edges.push_back(new_edge);
		prev = 0;
		dist = std::numeric_limits<weight_type>::max();
	}
	
	size_t edge_weight(size_t destination){
		/*returns edge weight from the node to the destination, returns infinite if no edge*/
		for (const auto& edge : edges){
			if (edge.dest == destination){
				return edge.weight;
			}
		}
		return std::numeric_limits<weight_type>::max();
	}
	void display_matrix(){
	/*SE*/
	std::cout<<std::endl<<own<<std::endl;
		for (const auto dv : matrix){
			for (const auto dist : dv){
				std::cout << dist << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	bool update_node_matrix( const std::vector<size_t>& distance_vector, size_t dv_of ){
		/* returns true if update to own distance vector*/
		bool update = false;
		size_t length = matrix[0].size();
		/*copy the recieved dv in the matrix*/
		for (size_t i = 0; i < length; i++){
			matrix[dv_of][i] = distance_vector[i];
		}
		for (size_t i = 0; i < length; i++){
			if (i == own){
				/*own will always remain zero*/
				continue;
			}
			/*set min to max value it can take, and calculate the minimum*/
			weight_type min = std::numeric_limits<weight_type>::max();
			for (const auto& edge : edges){
				if (min > edge.weight + matrix[edge.dest][i]){
					min = edge.weight + matrix[edge.dest][i];
					forwarding_table[i] = edge.dest;
				}
			}
			if (matrix[own][i] != min){
				update = true;
				matrix[own][i] = min;
			}
		}
		return update;
	}
};

template <class dataStruct, class weight_type>
class edge
{
public:
	size_t dest;
	weight_type weight;
	edge(size_t d, weight_type w = 0) {
		dest = d;
		weight = w;
	}
};

//only allowed weight_types are numerical types
template <class dataStruct, class weight_type>
class graph
{
public:
	const static weight_type INF = std::numeric_limits<weight_type>::max();	//stores the maximum value weight_type can take
	std::vector<graph_node<dataStruct, weight_type>> vertices;
	std::map<dataStruct, int> vertex_indices;	//a map to store the vertex indices

	/*using 2 queues in stead of makin a structure*/
	std::queue <std::vector<size_t> > q_messages; /*a messages queue for updates in stabalize*/
	std::queue <std::vector<size_t> > q_dest_dist_vec; /*queue to store the passed distance vectors*/
	/* the number of elements in both the queues remain the same at any point in time */

	graph()
	{
		vertex_indices.clear();
		vertices.clear();
	}

	void initialize_matrix(){
		for (auto& vertex : vertices){

			vertex.forwarding_table.resize(vertices.size());
			vertex.forwarding_table[vertex.own] = vertex.own;
			vertex.matrix.resize(vertices.size());

			size_t num = vertex.matrix.size();

			for (auto& dv : vertex.matrix){
				dv.clear();
				dv.resize(vertices.size());
				for (auto& distance : dv){
					/*set distance to infinite*/
					distance = INF;
				}
			}
			size_t own_ind = vertex.own;
			for (auto& edge : vertex.edges){
				/*add the edge weights to own dv*/
				vertex.matrix[vertex.own][edge.dest] = edge.weight;
			}
			vertex.matrix[vertex.own][vertex.own] = 0;
			//cout<<num<<endl;
			for (size_t i = 0; i < num; i++){
				/*in the metrix, the distance of any node to itseld will always remaing zero*/
				vertex.matrix[i][i] = 0;
			}
		}
	}

	size_t update_start(bool poisioned = 0){
		initialize_matrix();
		return update_all(poisioned);
	}

	size_t update_all(bool poisioned = 0){
		/* add each node neighbour pair to the messages queue */
		for (const auto& vertex : vertices){
			for (const auto& edge : vertex.edges){
				q_messages.push({ edge.dest, vertex.own });
				q_dest_dist_vec.push(vertex.matrix[vertex.own]);
			}
		}
		if (poisioned == 1){
			return poisioned_stabalize();
		}
		return stabalize();
	}

	size_t stabalize(){
		/*
			empty the queue
		*/
		size_t countt = 0;

		while (!q_messages.empty()){
			countt++;

			/*read the first message*/
			vector<size_t> message = q_messages.front();
			q_messages.pop();
			vector<size_t> dist_vec = q_dest_dist_vec.front();
			q_dest_dist_vec.pop();

			/*update*/
			bool update = vertices[message[0]].update_node_matrix(dist_vec, message[1]);
			if (update == true){
				for (const auto& edge : vertices[message[0]].edges){
					/*pass dv to all neighbours*/
					q_messages.push ( {edge.dest , message[0]} );
					q_dest_dist_vec.push( vertices[message[0]].matrix[message[0]]);
				}
			}
		}
		return countt;
	}

	size_t poisioned_stabalize(){
		size_t countt = 0;
		while (!q_messages.empty()){
			countt++;
			vector<size_t> message = q_messages.front();
			q_messages.pop();
			vector<size_t> dist_vec = q_dest_dist_vec.front();
			q_dest_dist_vec.pop();
			
			for (int i = 0; i < vertices.size(); i++){
				/* for each vertex in the distace vector*/
				/* if the the path to destination passes through a neighbour, lie to that neighbour: own distance to destination is infinite */
				/* when does the lying stop? */
				if (vertices[message[1]].forwarding_table[i] == message[0]){
					dist_vec[i] = INF;
				}
			}
			bool update = vertices[message[0]].update_node_matrix(dist_vec, message[1]);
			if (update == true){
				/*send to all neighbours if updated*/
				for (const auto& edge : vertices[message[0]].edges){
					q_messages.push({ edge.dest , message[0] });
					q_dest_dist_vec.push(vertices[message[0]].matrix[message[0]]);
				}
			}
		}

		/*stop lying and honestly update all*/		
		countt += update_all();

		return countt;
	}

	size_t update_edge(dataStruct source, dataStruct dest, weight_type new_weight, bool poisioned = 0){
		/*returns false if no such edge exists*/
		bool source_exists = 0; bool dest_exists = 0, successful = 0;
		size_t source_index = -1, dest_index = -1;
		/*check if source and dest exist*/
		for (const auto& vertex : vertices){
			if(vertex.data == source) {
				source_exists = 1;
				source_index = vertex.own;
			}
			if (vertex.data == dest) {
				dest_exists = 1;
				dest_index = vertex.own;
			}
		}
		if (source_exists & dest_exists){
			// both source and destination exist
			//if undirected, change edge weights in both
			for (auto& a_edge : vertices[vertex_indices[source]].edges){
				if (a_edge.dest == vertex_indices[dest]){
					a_edge.weight = new_weight;
					successful = true;
				}
			}
			for (auto& a_edge : vertices[vertex_indices[dest]].edges){
				if (a_edge.dest == vertex_indices[source]){
					a_edge.weight = new_weight;
					successful = true;
				}
			}
			if (successful = 0){
				// add edge? : no
				// insert_edge(source, dest, new_weight, 1);
				// successful = 1;
				// may change successful 
			}
			if (successful = 1){
				/* successful in changing the edge weight */
				/* returns zeri otherwise */
				q_messages.push({ source_index, dest_index });
				q_dest_dist_vec.push(vertices[dest_index].matrix[dest_index]);
				q_messages.push({ dest_index, source_index });
				q_dest_dist_vec.push(vertices[source_index].matrix[source_index]);
				if (poisioned == 1){
					return poisioned_stabalize();
				}
				return stabalize();
			}
		}else{
			return 0;
		}
	}

	int insert_node(dataStruct n)
	{
		//returns index of the inserted node in graph vertices
		int index = vertices.size();
		vertex_indices[n] = index;
		graph_node<dataStruct, weight_type> *new_node = new graph_node<dataStruct, weight_type>(n, index);
		vertices.push_back(n);
		return index;
	}

	int insert_node(graph_node<dataStruct, weight_type>* node)
	{
		//returns index of the inserted node in graph vertices
		dataStruct n = node->data;
		int index = vertices.size();
		vertex_indices[n] = index;
		vertices.push_back(&node);
		return index;
	}


	int insert_node(graph_node<dataStruct, weight_type> node)
	{
		//returns index of the inserted node in graph vertices
		dataStruct n = node.data;
		int index = vertices.size();
		vertex_indices[n] = index;
		vertices.push_back(node);
		return index;
	}

	int insert_edge(dataStruct source, dataStruct dest, weight_type w = 0, bool undirected = 0)
	{
		//weight assumed to be zero if none provided
		//default is directed edge addition
		//adds the source and destination vertices to the graoh if they dont exist
		//returns the number of new vertices added to the graph

		int no = 0;
		int src_index = -1;
		int dest_index = -1;
		if (vertex_indices.find(source) == vertex_indices.end()) {
			// the source vertex doesnt exist
			int index = vertices.size();
			graph_node<dataStruct, weight_type> source_node(source, index);
			src_index = insert_node(source_node);
			no += 1;
		}

		else {
			src_index = vertex_indices[source];
		}
		if (vertex_indices.find(dest) == vertex_indices.end()) {
			//the destination doesnt exist
			int indexd = vertices.size();
			graph_node<dataStruct, weight_type> dest_node(dest, indexd);
			dest_index = insert_node(dest_node);
			no += 1;
		}
		else {
			dest_index = vertex_indices[dest];
		}

		//make edge and insert to the graph
		edge<dataStruct, weight_type>* e1 = new edge<dataStruct, weight_type>(dest_index, w);
		vertices[src_index].edges.push_back(*e1);
		if (undirected == 1) {
			edge<dataStruct, weight_type>* e2 = new edge<dataStruct, weight_type>(src_index, w);
			vertices[dest_index].edges.push_back(*e2);
		}

		return no;
	}


	void display_vertices() {
		for (const auto vertex : vertices) {
			std::cout<<vertex.own<<" "<<vertex.data<<"\t";
			if(vertex.data.length() < 8){
				std::cout<<"\t";
			}
			std::cout<<std::endl;
			for (const auto dv : vertex.matrix){
				for (const auto dist : dv){
					std::cout<<dist<<" ";
				}
				std::cout<<std::endl;
			}
			std::cout << std::endl;
		}
		return;
	}

	std::string find_vertex_given_prefix(std::string prefix, std::string upper_b){
		//  upper_b is the upper bound
		typename std::map<std::string, int>::iterator it = vertex_indices.lower_bound(prefix);
		if (it != vertex_indices.upper_bound(upper_b)){
			return it->first;
		}
		else{
			return "";
		}
	}

	void display_edges() {
		for (auto vertex : vertices) {
			std::cout << vertex.data << " ";
			std::cout << std::endl;
			for (auto edge : vertex.edges) {
				std::cout<<"\t"<< vertices[edge.dest].data<<" "<<edge.weight<<std::endl;
			}
		}
		std::cout << std::endl;
		return;
	}

	void display_all(){
		std::cout << "all vertices: " << std::endl;
		display_vertices();
		std::cout<<"all edges: "<<std::endl;
		display_edges();
		return;
	}
};
