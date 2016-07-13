/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file graph.h
//! \brief Describes the Graph class.

#ifndef GRAPH_H_
#define GRAPH_H_

//! \class Graph
//! \brief Contains graph theoretic representation of a pipe network.
//!
//! A Graph object contains data structures (e.g., adjacency lists) and
//! algorithms (e.g., spanning tree) for describing the connectivity of
//! a pipe network.

#include <vector>

class Network;

class Graph
{
  public:

    Graph();
    ~Graph();

    void    createAdjLists(Network* nw);

  private:
    std::vector<int> adjLists;        // packed nodal adjacency lists
    std::vector<int> adjListBeg;      // starting index of each node's list
};

#endif // GRAPH_H_
