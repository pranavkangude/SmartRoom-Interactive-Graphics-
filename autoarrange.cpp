// autoarrange.cpp - Auto-arrange Module (greedy, rule-based, no AI)
//
// Items are placed one by one in priority order (bed, wardrobe, sofa, desk, table, chairs).
// For each item we generate candidate positions, discard the invalid ones (outside the room,
// blocking the door swing, overlapping or too close to placed items) and pick the best by a
// score that encodes the rules:
//   Bed       headboard against a wall, as far from the door as possible
//   Wardrobe  back to a wall, in a corner, not in front of the window
//   Sofa      back to a wall, centred on it, preferably not on the door wall
//   Desk      back to a wall, as close to the window as possible
//   Table     as close to the centre of the room as possible
//   Chair     tucked in front of the desk first, then around the table, then along the walls
// Furniture "back" is the +y side of its local frame (backrest, headboard, monitor side).
// Walls are the edges of the room polygon, so this works for L-shaped rooms too.
#include "autoarrange.h"
#include "common.h"
#include "collision.h"
#include "history.h"
#include <cmath>
#include <algorithm>
using namespace std;

struct Cand { float x, y, angle, off; };   // off = distance from the middle of its wall

static int rankOf(int type) {
    switch (type) {
    case BED: return 0;
    case WARDROBE: return 1;
    case SOFA: return 2;
    case DESK: return 3;
    case TABLE: return 4;
    default: return 5;   // chairs last
    }
}

static float dist(Vec2 a, Vec2 b) { return hypotf(a.x - b.x, a.y - b.y); }

// Candidate positions with the item's back (+y local) flush against each wall (polygon edge).
//   outward normal up -> angle 0, left -> 90, down -> 180, right -> 270
static void wallCandidates(const Furniture& f, vector<Cand>& out) {
    int n = (int)roomPoly.size();
    for (int i = 0; i < n; i++) {
        Vec2 a = roomPoly[i], b = roomPoly[(i + 1) % n];
        float dx = b.x - a.x, dy = b.y - a.y, len = hypotf(dx, dy);
        if (len < 0.01f) continue;
        float nx = dy / len, ny = -dx / len;                 // outward normal (polygon is CCW)
        float ang = (ny > 0.5f) ? 0.0f : (nx < -0.5f) ? 90.0f : (ny < -0.5f) ? 180.0f : 270.0f;
        bool swapped = (ang == 90.0f || ang == 270.0f);
        float ex = swapped ? f.h / 2 : f.w / 2;              // half extents in world axes
        float ey = swapped ? f.w / 2 : f.h / 2;
        if (fabsf(dy) < 1e-4f) {                             // horizontal wall
            float lo = min(a.x, b.x) + ex, hi = max(a.x, b.x) - ex;
            if (hi < lo) continue;
            float mid = (min(a.x, b.x) + max(a.x, b.x)) / 2;
            float y = a.y - ny * ey;                         // centre sits half a depth inside the wall
            for (float x = lo; x < hi; x += 0.5f) out.push_back({x, y, ang, fabsf(x - mid)});
            out.push_back({hi, y, ang, fabsf(hi - mid)});
        } else {                                             // vertical wall
            float lo = min(a.y, b.y) + ey, hi = max(a.y, b.y) - ey;
            if (hi < lo) continue;
            float mid = (min(a.y, b.y) + max(a.y, b.y)) / 2;
            float x = a.x - nx * ex;
            for (float y = lo; y < hi; y += 0.5f) out.push_back({x, y, ang, fabsf(y - mid)});
            out.push_back({x, hi, ang, fabsf(hi - mid)});
        }
    }
}

// Is this furniture valid given the items already placed? 'margin' keeps a gap between items.
static bool okPlace(const Furniture& f, float margin) {
    if (!insideRoom(f) || doorBlocked(f)) return false;
    Furniture g = f;
    g.w += 2 * margin; g.h += 2 * margin;
    for (const Furniture& o : items)
        if (satOverlap(g, o)) return false;
    return true;
}

static float score(const Furniture& f, float off) {
    Vec2 c = {f.x, f.y};
    Vec2 doorP = {doorX + DOOR_W / 2, 0}, winP = {(winX0 + winX1) / 2, roomL}, mid = {roomCx, roomCy};
    switch (f.type) {
    case BED:
        return dist(c, doorP) - 0.3f * off;
    case WARDROBE: {
        float nearest = 1e9f;
        for (auto& k : roomPoly) nearest = min(nearest, dist(c, k));      // nearest room corner
        float s = -nearest;
        if (hasWindow() && f.angle == 0 && f.y + f.h / 2 > roomL - 0.05f &&
            f.x + f.w / 2 > winX0 && f.x - f.w / 2 < winX1) s -= 8.0f;    // in front of the window
        return s;
    }
    case SOFA: {
        float s = -0.5f * off;
        if (f.angle == 180 && fabsf(f.y - f.h / 2) < 0.05f) s -= 5.0f;    // door wall
        s += 0.1f * ((fmodf(f.angle, 180.0f) == 0.0f) ? roomW : roomL);   // prefer long walls
        return s;
    }
    case DESK:
        return -dist(c, winP);
    case TABLE:
        return -dist(c, mid);
    default:
        return -0.5f * dist(c, mid);
    }
}

struct Context { int deskIdx = -1, tableIdx = -1; bool deskChairDone = false; };

static bool placeChair(Furniture ch, Context& ctx, float margin) {
    // 1) tucked in front of the desk, facing it
    if (ctx.deskIdx >= 0 && !ctx.deskChairDone) {
        const Furniture& d = items[ctx.deskIdx];
        float t = d.angle * PI / 180.0f;
        float off = d.h / 2 + ch.h / 2 + 0.15f;
        ch.x = d.x + sinf(t) * off;               // direction of the desk's local -y
        ch.y = d.y - cosf(t) * off;
        ch.angle = fmodf(d.angle + 180.0f, 360.0f);
        if (okPlace(ch, 0.0f)) { items.push_back(ch); ctx.deskChairDone = true; return true; }
    }
    // 2) around the table, backrest facing away from it
    if (ctx.tableIdx >= 0) {
        const Furniture& tb = items[ctx.tableIdx];
        float r = tb.w / 2 + ch.h / 2 + 0.2f;
        static const float phis[8] = {270, 90, 180, 0, 225, 315, 135, 45};
        for (int k = 0; k < 8; k++) {
            float p = phis[k] * PI / 180.0f;
            ch.x = tb.x + r * cosf(p);
            ch.y = tb.y + r * sinf(p);
            ch.angle = fmodf(phis[k] - 90.0f + 360.0f, 360.0f);
            if (okPlace(ch, 0.0f)) { items.push_back(ch); return true; }
        }
    }
    // 3) along a wall
    vector<Cand> cands;
    wallCandidates(ch, cands);
    float best = -1e9f; Furniture bestF = ch; bool found = false;
    for (auto& c : cands) {
        ch.x = c.x; ch.y = c.y; ch.angle = c.angle;
        if (!okPlace(ch, margin)) continue;
        float s = score(ch, c.off);
        if (s > best) { best = s; bestF = ch; found = true; }
    }
    if (found) items.push_back(bestF);
    return found;
}

static bool placeOne(Furniture f, Context& ctx, float margin) {
    f.angle = 0;
    if (f.type == CHAIR) return placeChair(f, ctx, margin);

    vector<Cand> cands;
    if (f.type == TABLE) {                                        // anywhere on a 0.5 ft grid
        for (float y = f.h / 2; y <= roomL - f.h / 2 + 0.001f; y += 0.5f)
            for (float x = f.w / 2; x <= roomW - f.w / 2 + 0.001f; x += 0.5f)
                cands.push_back({x, y, 0, 0});
    } else {
        wallCandidates(f, cands);                                 // back against a wall
    }

    float best = -1e9f; Furniture bestF = f; bool found = false;
    for (auto& c : cands) {
        f.x = c.x; f.y = c.y; f.angle = c.angle;
        if (!okPlace(f, margin)) continue;
        float s = score(f, c.off);
        if (s > best) { best = s; bestF = f; found = true; }
    }
    if (!found) return false;
    items.push_back(bestF);
    if (f.type == DESK)  ctx.deskIdx = (int)items.size() - 1;
    if (f.type == TABLE) ctx.tableIdx = (int)items.size() - 1;
    return true;
}

bool autoArrange() {
    if (items.empty()) { statusMsg = "Add some furniture first, then press A to auto-arrange."; return false; }

    vector<Furniture> before = items;
    vector<Furniture> todo = items;
    stable_sort(todo.begin(), todo.end(),
                [](const Furniture& a, const Furniture& b) { return rankOf(a.type) < rankOf(b.type); });

    static const float margins[3] = {0.6f, 0.25f, 0.0f};         // roomy first, then tighter
    for (int pass = 0; pass < 3; pass++) {
        items.clear();
        Context ctx;
        bool allPlaced = true;
        for (const Furniture& f : todo)
            if (!placeOne(f, ctx, margins[pass])) { allPlaced = false; break; }
        if (allPlaced) {
            sel = -1;
            commitUndo(before);
            statusMsg = "Auto-arranged " + to_string(items.size()) + " items (U to undo).";
            return true;
        }
    }
    items = before;                                               // keep the user's layout
    statusMsg = "Auto-arrange: not enough space for all items (use a bigger room or fewer items).";
    return false;
}
