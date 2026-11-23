# Campus-Sync

A C++ console app for getting around campus. It finds the shortest walking route between places, picks a meeting point for a group, lists free rooms to sit in, keeps your class schedule with undo and redo, and plans the fastest order for errands in a break.
Every data structure (linked list, stacks, heaps, hash map, trie, graph) is written by hand. No STL containers are used apart from `std::vector` as backing storage.

## Build

You need a compiler that supports C++23, such as GCC 13 or newer. From the repo root, run:

```sh
g++ -std=c++23 -Wall -Iinclude main.cpp src/BunkerRadar.cpp src/CampusNode.cpp src/DoublyLinkedList.cpp src/Graph.cpp src/HashMap.cpp src/MaxHeap.cpp src/MinHeap.cpp src/PrefixTrie.cpp src/RoomAvailability.cpp src/Scheduler.cpp src/Speedrunner.cpp src/UndoStack.cpp -o campus_sync.exe
```

## Run

Run it **from the repo root** so it can find the `data/` folder:

```sh
./campus_sync.exe
```

If the graph files can't be loaded, the program prints an error and exits with code 1. Pressing Ctrl+Z (Windows) or Ctrl+D ends the input and closes the program.

To run only the self-tests without typing anything:

```sh
printf '1\n0\n' | ./campus_sync.exe
```

## Menu

| # | Option | What it does |
|---|--------|--------------|
| 1 | Run self-tests | Runs every `testX()` check. A failed `assert` stops the program. |
| 2 | Find a place | Search by any part of a name with autocomplete. Shows the place's type, position and number of paths, plus its free hours (rooms) or queue wait (service places). |
| 3 | Route between two places | Shortest path with Dijkstra, taking the hour (rush-hour crowding) and sun avoidance into account. Shows each step, the distance and the walking time. |
| 4 | Squad-Sync | 2-4 people choose where they are. Shows the meeting point that minimises everyone's total walk, plus each person's distance and time. |
| 5 | Bunker Radar | The top 3 free rooms near you for a given hour, ranked by how long they stay free and how far away they are. |
| 6 | My Schedule | View the day, add an event (overlaps are rejected), remove an event by title, undo, redo. |
| 7 | Next class | Finds your next event, gives the route there, and warns you if you'll be late. |
| 8 | Plan a break (Speedrunner) | The fastest order for up to 4 errands within a break, including queue time. If it doesn't fit, it tells you how many minutes short you are. |

Places are always chosen by typing part of a name and picking from a numbered list. Hours run from 8 to 15, and times are written `HH:MM`.

## Modules

| Owner | Modules |
|-------|---------|
| Umair | Graph, MinHeap, CampusNode, routing (Dijkstra), Squad-Sync |
| Hamza | DoublyLinkedList, UndoStack, Scheduler, Speedrunner, `data/schedule.csv` |
| Tahir | PrefixTrie, HashMap, MaxHeap, BunkerRadar, dashboard (`main.cpp`) |

## Data files

All files are CSV with a header row. Windows line endings are fine.

**`data/nodes.csv`**: `id,name,type,x,y,schedule`
- `id`: a whole number starting at 0, the same one used in the other files
- `type`: `ACADEMIC`, `SERVICE` or `TRANSIT`
- `x,y`: map coordinates
- `schedule`: 8 characters, one per hour from 08:00 to 15:00. `1` means busy and `0` means free. It only matters for `ACADEMIC` rooms.

**`data/edges.csv`**: `from,to,distance,is_outdoor,rush_multiplier`
- `from,to`: node ids. Paths can be walked in both directions.
- `distance`: length in metres
- `is_outdoor`: `1` if the path is in the sun. When sun avoidance is on, outdoor paths cost 2.5×.
- `rush_multiplier`: how much longer the path takes from 11:00 to 14:00 because of crowds

**`data/schedule.csv`**: `title,start,end,locationId`
- `start,end`: `HH:MM` in 24-hour time, zero-padded. `end` must be after `start`.
- `locationId`: a node id from `nodes.csv`
- Bad lines (wrong number of columns, bad time, unknown place, or an event that overlaps another) are skipped, and a warning is shown at startup.
