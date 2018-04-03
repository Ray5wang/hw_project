#ifndef __ROUTE_H__
#define __ROUTE_H__

#include <stdio.h>
#include <iostream>
#include "lib_io.h"
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <numeric>
#include "_matrix.h"

using namespace std;

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);
void get_info(char * info[MAX_INFO_NUM]);
void get_data(char * data[MAX_DATA_NUM], int data_num);
time_t str2time(string str, string format="%Y-%m-%d");
void three_predict();
void average_predict();

#endif
