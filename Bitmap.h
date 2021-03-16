//
// Created by D. A. Hosek on 3.2.21.
//

#ifndef GFTODVI_BITMAP_H
#define GFTODVI_BITMAP_H

#include <vector>

class BlackLine {
public:
    BlackLine(int x, int y, int w) : x(x), y(y), w(w) {}

    inline int getX() const {
        return x;
    }

    inline int getY() const {
        return y;
    }

    inline int getW() const {
        return w;
    }

private:
    int x;
    int y;
    int w;
};

class Bitmap {
public:

    inline std::vector<BlackLine> get_bitmap() {
        return bitmap;
    }

    void add_blackline(int x, int y, int w);

private:
    int height;
    int width;
    std::vector<BlackLine> bitmap;
};



#endif //GFTODVI_BITMAP_H
