#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <list>
#include <set>
#include <string>
#include <ctime>
#include "kanonizator_dlk_1.03/kanonizator.h"

using namespace std;

const int ch_srez = 3;
const int max_trans = 5504;
const int timeout = 5 * CLOCKS_PER_SEC;

typedef array<unsigned char, raz> kvadrat;
typedef array<unsigned char, por> transver;
