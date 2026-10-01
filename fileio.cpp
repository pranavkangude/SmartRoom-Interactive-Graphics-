// fileio.cpp - File Module
#include "fileio.h"
#include "common.h"
#include "furniture.h"
#include "history.h"
#include "transform.h"
#include <fstream>
#include <iomanip>
using namespace std;

static const char* SAVE_FILE = "layout.txt";

// Format:  SMARTROOM 1 / roomW roomL / count / then one line per item: type x y w h angle
void saveLayout() {
    ofstream out(SAVE_FILE);
    if (!out) { statusMsg = "Could not write layout.txt!"; return; }
    out << "SMARTROOM 1\n" << roomW << " " << roomL << "\n" << items.size() << "\n";
    out << fixed << setprecision(3);
    for (auto& f : items)
        out << f.type << " " << f.x << " " << f.y << " " << f.w << " " << f.h << " " << f.angle << "\n";
    statusMsg = "Layout saved to layout.txt";
}

void loadLayout() {
    ifstream in(SAVE_FILE);
    if (!in) { statusMsg = "No layout.txt found (press S to save first)."; return; }
    string magic; int ver = 0;
    float rw, rl; int n;
    in >> magic >> ver >> rw >> rl >> n;
    if (!in || magic != "SMARTROOM" || rw < 4 || rw > 60 || rl < 4 || rl > 60 || n < 0 || n > 500) {
        statusMsg = "layout.txt is invalid or corrupted.";
        return;
    }
    vector<Furniture> loaded;
    for (int i = 0; i < n; i++) {
        int t; float x, y, w, h, a;
        in >> t >> x >> y >> w >> h >> a;
        if (!in || t < 0 || t >= FTYPE_COUNT) { statusMsg = "layout.txt is invalid or corrupted."; return; }
        Furniture f = makeItem(t);
        f.x = x; f.y = y; f.w = w; f.h = h; f.angle = a;
        loaded.push_back(f);
    }
    roomW = rw; roomL = rl;
    items = loaded;
    sel = -1;
    clearHistory();
    zoom = 1; panX = panY = 0;
    computeView();
    statusMsg = "Layout loaded (" + to_string(n) + " items).";
}
