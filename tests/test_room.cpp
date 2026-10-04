// Unit tests for the room polygon, boundary check and door/window placement (no OpenGL needed).
// Build and run (from the project folder):
//   g++ tests/test_room.cpp state.cpp roomshape.cpp matrix.cpp clip.cpp transform.cpp collision.cpp -I. -o test_room_v1.exe
//   .\test_room_v1.exe
#include "common.h"
#include "roomshape.h"
#include "collision.h"
#include <cstdio>
#include <cmath>

static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { printf("FAIL: %s\n", msg); fails++; } else printf("ok:   %s\n", msg); } while (0)

static Furniture box(float x, float y, float w, float h, float angle) {
    Furniture f = {0, x, y, w, h, angle, {0.5f, 0.5f, 0.5f}};
    return f;
}

int main() {
    roomW = 12; roomL = 10;

    // ---- shapes: area and polygon orientation ----
    setRoomShape(-1, 0, 0);
    CHECK(fabsf(roomArea - 120) < 0.01f && floorRects.size() == 1, "rectangle: area 120, one floor rectangle");
    for (int c = 0; c < 4; c++) {
        setRoomShape(c, 6, 5);
        char msg[80];
        snprintf(msg, sizeof msg, "L-shape corner %d: area 90 (120 - 30), two floor rectangles", c);
        CHECK(fabsf(roomArea - 90) < 0.01f && floorRects.size() == 2, msg);
        snprintf(msg, sizeof msg, "L-shape corner %d: centroid is inside the room", c);
        CHECK(pointInPolygon({roomCx, roomCy}, roomPoly), msg);
    }

    // ---- point in polygon (notch at top-right: x 6..12, y 5..10 is cut away) ----
    setRoomShape(0, 6, 5);
    CHECK(pointInPolygon({3, 8}, roomPoly), "point in the upper arm is inside");
    CHECK(pointInPolygon({9, 2}, roomPoly), "point in the lower arm is inside");
    CHECK(!pointInPolygon({9, 8}, roomPoly), "point in the notch is outside");
    CHECK(!pointInPolygon({-1, 2}, roomPoly), "point left of the room is outside");

    // ---- insideRoom for furniture ----
    CHECK(insideRoom(box(3, 8, 4, 2, 0)), "item in the upper arm is inside");
    CHECK(insideRoom(box(2, 1, 4, 2, 0)), "item flush with the bottom-left corner is inside");
    CHECK(insideRoom(box(10, 2.5, 4, 5, 0)), "item flush with the notch step and right wall is inside");
    CHECK(!insideRoom(box(9, 8, 3, 2, 0)), "item inside the notch is outside");
    CHECK(!insideRoom(box(6, 6, 4, 4, 0)), "item sticking from the arm into the notch is outside");
    CHECK(!insideRoom(box(6, 5, 1.6f, 1.6f, 45)), "item tilted 45 deg over the inner corner is outside");
    CHECK(!insideRoom(box(6, 5, 40, 40, 0)), "item bigger than the room is outside");
    CHECK(insideRoom(box(3, 2, 2, 2, 45)), "rotated item well inside the lower arm is inside");
    CHECK(!insideRoom(box(-0.5f, 2, 2, 2, 0)), "item through the left wall is outside");

    // rectangle behaves like before
    setRoomShape(-1, 0, 0);
    CHECK(insideRoom(box(6, 5, 12, 10, 0)), "rectangle: item the size of the room is inside");
    CHECK(!insideRoom(box(12, 5, 2, 2, 0)), "rectangle: item half through the right wall is outside");
    CHECK(insideRoom(box(6, 5, 4, 2, 30)), "rectangle: rotated item in the middle is inside");

    // ---- door and window for every orientation ----
    static const char* names[4] = {"top-right", "top-left", "bottom-right", "bottom-left"};
    for (int c = 0; c < 4; c++) {
        setRoomShape(c, 4, 4);                       // arms stay wide, so door and window exist
        char msg[100];
        bool zoneOk = true;
        for (float x = 0; x <= DOOR_W; x += 0.25f)
            for (float y = 0; y <= DOOR_W; y += 0.25f)
                if (x * x + y * y <= DOOR_W * DOOR_W && !pointInPolygon({doorX + x + 0.01f, y + 0.01f}, roomPoly)) zoneOk = false;
        snprintf(msg, sizeof msg, "notch %s: door exists and its swing zone is inside the room", names[c]);
        CHECK(hasDoor() && zoneOk, msg);
        snprintf(msg, sizeof msg, "notch %s: window exists on the top wall", names[c]);
        CHECK(hasWindow() && winX1 - winX0 == 4.0f, msg);
    }
    setRoomShape(0, 9, 4);                           // top wall becomes only 3 ft long
    CHECK(!hasWindow(), "top wall shorter than 6 ft has no window");

    // ---- too-small rooms ----
    roomW = 6; roomL = 6;
    CHECK(!setRoomShape(0, 3, 3), "6 x 6 room cannot be made L-shaped");
    roomW = 12; roomL = 10;

    // ---- cycling through the shapes ----
    setRoomShape(-1, 0, 0);
    int shapesSeen = 0;
    for (int i = 0; i < 5; i++) { cycleRoomShape(); shapesSeen++; }
    CHECK(!isLShaped() && shapesSeen == 5, "cycling 5 times returns to the rectangle");

    printf("\n%s (%d failures)\n", fails ? "SOME TESTS FAILED" : "ALL TESTS PASSED", fails);
    return fails;
}
