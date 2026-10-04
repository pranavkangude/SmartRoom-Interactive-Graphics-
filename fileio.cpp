// fileio.cpp - File Module
#include "fileio.h"
#include "common.h"
#include "furniture.h"
#include "history.h"
#include "transform.h"
#include "view3d.h"
#include "roomshape.h"
#include <fstream>
#include <iomanip>
using namespace std;

static const char* SAVE_FILE = "layout.txt";

// Format (version 2):
//   SMARTROOM 2 / roomW roomL notchW notchL notchCorner / count / one line per item: type x y w h angle
// Version 1 files (rectangular room, no notch line) can still be loaded.
void saveLayout() {
    ofstream out(SAVE_FILE);
    if (!out) { statusMsg = "Could not write layout.txt!"; return; }
    out << "SMARTROOM 2\n" << roomW << " " << roomL << " " << notchW << " " << notchL << " " << notchCorner
        << "\n" << items.size() << "\n";
    out << fixed << setprecision(3);
    for (auto& f : items)
        out << f.type << " " << f.x << " " << f.y << " " << f.w << " " << f.h << " " << f.angle << "\n";
    statusMsg = "Layout saved to layout.txt";
}

void loadLayout() {
    ifstream in(SAVE_FILE);
    if (!in) { statusMsg = "No layout.txt found (press S to save first)."; return; }
    string magic; int ver = 0;
    float rw, rl, nw = 0, nl = 0; int corner = 0, n;
    in >> magic >> ver >> rw >> rl;
    if (ver >= 2) in >> nw >> nl >> corner;                  // version 2 adds the L-shape notch
    in >> n;
    if (!in || magic != "SMARTROOM" || ver < 1 || ver > 2 || rw < 4 || rw > 60 || rl < 4 || rl > 60 ||
        n < 0 || n > 500 || nw < 0 || nl < 0 || corner < 0 || corner > 3) {
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
    if (nw > 0 && nl > 0) setRoomShape(corner, nw, nl);    // L-shaped room
    else                  setRoomShape(-1, 0, 0);          // rectangle
    items = loaded;
    sel = -1;
    clearHistory();
    zoom = 1; panX = panY = 0;
    computeView();
    resetCamera3D();
    statusMsg = "Layout loaded (" + to_string(n) + " items).";
}
