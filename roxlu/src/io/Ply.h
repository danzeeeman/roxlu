#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "VertexData.h"

using namespace std;
namespace roxlu {

class Ply {
public:
	Ply();
	bool save(string path, VertexData& vd);
};

}; // roxlu