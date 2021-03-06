/* Copyright 2013 David Axmark

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#include <windows.h>
#include <string>

#include <fstream>
#include <iostream>

#include <vector>

#include "IconFileStructs.h"

using namespace std;

typedef struct {
	ICONDIR iconDir;
	unsigned char *data;
} ICON;

class IconFileLoader
{
public:
	IconFileLoader(const char *filename);
	~IconFileLoader(void);	

	int getNumIcons();
	
	HICON getIcon(int index);

	ICON getIconData(int index);

private:
	vector<ICONDIRENTRY> icons;
	HANDLE iconFile;

	int numIcons;

	byte* createIconFromFile(ICONDIRENTRY icon);
};
