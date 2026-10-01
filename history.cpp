// history.cpp - undo / redo
#include "history.h"
using namespace std;

static vector< vector<Furniture> > undoStack, redoStack;

void commitUndo(const vector<Furniture>& before) {
    undoStack.push_back(before);
    if (undoStack.size() > 100) undoStack.erase(undoStack.begin());
    redoStack.clear();
}
void pushUndo() { commitUndo(items); }

void dropLastUndo() { if (!undoStack.empty()) undoStack.pop_back(); }

void clearHistory() { undoStack.clear(); redoStack.clear(); }

void doUndo() {
    if (undoStack.empty()) { statusMsg = "Nothing to undo."; return; }
    redoStack.push_back(items);
    items = undoStack.back();
    undoStack.pop_back();
    sel = -1;
    statusMsg = "Undo.";
}
void doRedo() {
    if (redoStack.empty()) { statusMsg = "Nothing to redo."; return; }
    undoStack.push_back(items);
    items = redoStack.back();
    redoStack.pop_back();
    sel = -1;
    statusMsg = "Redo.";
}
