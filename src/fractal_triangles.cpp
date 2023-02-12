#include <iostream>
#include <deque>
#include <fstream>
#include <format>

using namespace std;

#define msg(s) cerr << s << endl;

class Triangle {
public:

    double x0, y0, x1, y1, x2, y2;
    int level;

    Triangle(double x0, double y0, double x1, double y1, double x2, double y2, int l) {
        this->x0 = x0;
        this->y0 = y0;
        this->x1 = x1;
        this->y1 = y1;
        this->x2 = x2;
        this->y2 = y2;
        level = l;
    }
};


deque<Triangle*> fractal_triangle(int l) {
    deque<Triangle*> tris{new Triangle(1, sqrt(3), 0, 0, 2 , 0, 0)};
    int n_triangles = 1;
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < n_triangles; j++) {
            Triangle* t = tris.front();
            tris.pop_front();

            double x0 = t->x0, y0 = t->y0, x1 = t->x1, y1 = t->y1, x2 = t->x2, y2 = t->y2;

            double x3 = 0.5 * (x1 + x2), y3 = 0.5 * (y1 + y2);
            double x4 = 0.5 * (x0 + x2), y4 = 0.5 * (y0 + y2);
            double x5 = 0.5 * (x0 + x1), y5 = 0.5 * (y0 + y1);

            tris.push_back(new Triangle(x0, y0, x5, y5, x4, y4, t->level));
            tris.push_back(new Triangle(x5, y5, x1, y1, x3, y3, t->level));
            tris.push_back(new Triangle(x4, y4, x3, y3, x2, y2, t->level));
            tris.push_back(new Triangle(x3, y3, x4, y4, x5, y5, t->level + 1));
            
        }
        n_triangles *= 4;
    }
    return tris;
}

void write_to_svg(string fname, int size, deque<Triangle*> tris) {
    string colors[] = { "#8dd3c7","#ffffb3","#bebada","#fb8072","#80b1d3","#fdb462","#b3de69","#fccde5",
            "#d9d9d9", "#bc80bd", "#ccebc5", "#ffed6f"};
    int n_colors = colors->size();
    ofstream fout;
    fout.open(fname);

    char buffer[500];

    sprintf(buffer, "<svg version=\"1.1\" id = \"Layer_1\" xmlns = \"http://www.w3.org/2000/svg\" xmlns:xlink = \"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\" " \
        "width=\"%dpx\" height=\"%dpx\" viewBox=\"0 0 %d %d\" enable-background=\"new 0 0 %d %d\" xml:space=\"preserve\">", size, size, size, size, size, size);
    fout << buffer << endl;

    for (auto tri : tris) {
        sprintf(buffer, "<polygon fill=\"%s\" points=\"%lf,%lf %lf,%lf %lf,%lf\"/>",
            colors[tri->level % n_colors],
            tri->x0 * 0.5 * size,
            tri->y0 * 0.5 * size,
            tri->x2 * 0.5 * size,
            tri->y2 * 0.5 * size,
            tri->x1 * 0.5 * size,
            tri->y1 * 0.5 * size
        );
        fout << buffer << endl;
    }
    fout << "</svg>";
    fout.close();
}


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Provide the recursion level" << endl;
        return 0;
    }
    deque<Triangle*> tris = fractal_triangle(atoi(argv[1]));
    write_to_svg("../../../svg/frac_tri" + string(argv[1]) + ".svg", 1000, tris);
}