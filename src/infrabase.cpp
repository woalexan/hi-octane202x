/*
 Copyright (C) 2025 Wolf Alexander

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.                                          */

#include "infrabase.h"

//get a random int in the range between min and max
int InfrastructureBase::randRangeInt(int min, int max) {
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

InfrastructureBase::InfrastructureBase() {
}

InfrastructureBase::~InfrastructureBase() {
}
