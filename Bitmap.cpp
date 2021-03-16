//
// Created by D. A. Hosek on 3.2.21.
//

#include "Bitmap.h"


void Bitmap::add_blackline(int x, int y, int w) {
    bitmap.emplace_back(BlackLine(x, y, w));
}
