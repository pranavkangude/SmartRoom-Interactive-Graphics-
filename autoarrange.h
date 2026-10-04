// autoarrange.h - rule-based "Auto-arrange" suggestion
#pragma once

// Re-places every furniture item in the room using simple layout rules.
// Returns true if all items were placed (one undo step is recorded).
// If the room is too small, the original layout is kept and false is returned.
bool autoArrange();
