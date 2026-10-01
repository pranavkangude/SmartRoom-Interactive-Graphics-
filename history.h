// history.h - undo / redo (snapshot stacks of the whole layout)
#pragma once
#include "common.h"

void commitUndo(const std::vector<Furniture>& before); // save 'before' as an undo step
void pushUndo();                                       // save the current layout as an undo step
void dropLastUndo();                                   // discard the newest undo step
void clearHistory();
void doUndo();
void doRedo();
