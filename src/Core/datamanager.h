/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file datamanager.h
//! \brief Functions for acessing a project's network data base.


#ifndef DATAMANAGER_H_
#define DATAMANAGER_H_

class Network;

struct DataManager
{
    static int getCount(int element, int* count, Network* nw);

    static int getNodeIndex(char* name, int* index, Network* nw);
    static int getNodeId(int index, char* id, Network* nw);
    static int getNodeType(int index, int* type, Network* nw);
    static int getNodeValue(int index, int param, double* value, Network* nw);

    static int getLinkIndex(char* name, int* index, Network* nw);
    static int getLinkId(int index, char* id, Network* nw);
    static int getLinkType(int index, int* type, Network* nw);
    static int getLinkNodes(int index, int* fromNode, int* toNode, Network* nw);
    static int getLinkValue(int index, int param, double* value, Network* nw);
};

#endif // DATAMANAGER_H_
