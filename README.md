# Campus-Sync

A C++ console app for getting around the FAST-NUCES Karachi campus. The campus is a 3D graph: buildings have floors, and the floors are linked by stairs. The app finds the shortest walking route between any two rooms, picks a meeting point for a group, lists free rooms to sit in, keeps your class schedule with undo and redo, and plans the fastest order for errands in a break.
Every data structure (linked list, stacks, heaps, hash map, trie, graph) is written by hand. No STL containers are used apart from `std::vector` as backing storage.

## Build

You need a compiler that supports C++23, such as GCC 13 or newer. From the repo root, run:

```sh
g++ -std=c++23 -Wall -Iinclude main.cpp src/BunkerRadar.cpp src/CampusNode.cpp src/DoublyLinkedList.cpp src/Graph.cpp src/HashMap.cpp src/MaxHeap.cpp src/MinHeap.cpp src/PrefixTrie.cpp src/RoomAvailability.cpp src/Scheduler.cpp src/Speedrunner.cpp src/UndoStack.cpp -o campus_sync.exe
```

On Windows, add `-static` if an older MinGW is also installed. That way the program doesn't load that MinGW's incompatible runtime DLLs.

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
| 2 | Find a place | Search by any part of a name with autocomplete. Shows the place's type, building and floor, its free hours (rooms) or queue wait (service places). |
| 3 | Route between two places | Shortest path with Dijkstra for the hour you give, with rush-hour crowding taken into account. Shows the path with runs of stairs collapsed, e.g. `EE Floor D -> (stairs down 3) -> EE Floor A`, plus the number of floor changes, the distance and the walking time. |
| 4 | Squad-Sync | 2-4 people choose where they are. Shows the meeting point that minimises everyone's total walk, plus each person's distance and time. |
| 5 | Bunker Radar | The top 3 free rooms near you for a given hour, ranked by how long they stay free and how far away they are. |
| 6 | My Schedule | View the day, add an event (overlaps are rejected), remove an event by title, undo, redo. |
| 7 | Next class | Finds your next event, gives the route there, and warns you if you'll be late. |
| 8 | Plan a break (Speedrunner) | The fastest order for up to 4 errands within a break, including queue time. If it doesn't fit, it tells you how many minutes short you are. |

Places are always chosen by typing part of a name and picking from a numbered list. Every match is tagged with its location, e.g. `Lab 2 [CS, Floor 1]`, `Library [MPB, Basement]` or `Masjid [Outdoor]`. Names are matched case-insensitively, and digits and hyphens count, so `Lab 1`, `Lab 10` and `LLC-1` are all different places. Hours run from 8 to 15, and times are written `HH:MM`.

**Sun avoidance has been removed from the menu.** The dashboard always routes with it off. `Graph` still accepts an `avoidSun` flag, which applies a 2.5× penalty to outdoor paths, but nothing in the menu asks for it.

## Campus map

There are 83 places: 11 outdoor places, 11 floor hubs and 61 rooms.

**Floor-hub model.** Each floor of a building has one corridor node, its *floor hub*, named after the building and floor, e.g. `EE Floor D`, `CS Basement` or `MPB Ground`.
- Each room connects **only** to its own floor hub. The indoor distance is 5–15 m, rising in room-number order along the corridor.
- Neighbouring floor hubs are joined by **stairs**. Every flight counts as 20 m in every building, and there are no lifts.
- A route from a room on one floor to a room on another therefore always goes room → floor hub → stairs → floor hub → room.
- Outdoor paths link the ground-floor hubs (the building doors) to the outdoor places.

| Building | Floors (bottom → top) | Rooms |
|----------|-----------------------|-------|
| EE | A (ground), B, C, D, E | A: Lab 6, A1–A8 · B: B9–B11 · C: C12–C21, Lab 7 · D: D22–D28, Lab 8–10, LLC-2 · E: E29–E35, Lab 11–14 |
| CS | Basement, Ground, 1 | Basement: Lab 4 · Ground: LLC-1, S2 · 1: E1–E6, R109, Lab 1–3 |
| MPB (Multipurpose) | Basement, Ground, 1 | Basement: Library · Ground: Auditorium · 1: Cafe |

**Outdoor places:**
- Main Gate, South Junction, North Junction and Sports Junction;
- Futsal Court, Badminton Court, Sports Room and Basketball Court;
- CS Canteen and Masjid;
- Lab 5, which stands on its own where the old EE canteen was.

Classrooms, labs and LLCs are academic rooms, and each has an 8-hour schedule. Library, Auditorium, Cafe, the canteen, the Masjid and the sports facilities are service places with a queue.

**Distances and crowding:**
- **Outdoor distances** are measured on the hand-drawn map: the straight-line distance in pixels × 0.5, giving metres.
- **Rush hour** (11:00–14:00) makes paths take longer by this factor:

| Kind of path | Factor |
|---|---|
| Classroom corridors | 1.4 |
| Stairs | 1.5 |
| Outdoor paths | 1.2 |
| Paths into the canteen or Cafe | 1.6 |
| Everything else | 1.0 |

## Modules

| Owner | Modules |
|-------|---------|
| Umair | Graph, MinHeap, CampusNode, routing (Dijkstra), Squad-Sync |
| Hamza | DoublyLinkedList, UndoStack, Scheduler, Speedrunner, `data/schedule.csv` |
| Tahir | PrefixTrie, HashMap, MaxHeap, BunkerRadar, dashboard (`main.cpp`) |

## Data files

All files are CSV with a header row. Windows line endings are fine.

**`data/nodes.csv`**: `id,name,type,building,floor,x,y,schedule`
- `id`: a whole number starting at 0, the same one used in the other files
- `type`: `ACADEMIC`, `SERVICE` or `TRANSIT` (floor hubs and junctions)
- `building`: `EE`, `CS`, `MPB`, or `-` for outdoor places
- `floor`: `A`–`E` for EE, `Basement` / `Ground` / `1` for CS and MPB, or `-` for outdoor places
- `x,y`: map pixel coordinates. Everything inside a building uses the building's door position.
- `schedule`: 8 characters, one per hour from 08:00 to 15:00. `1` means busy and `0` means free. It only matters for `ACADEMIC` rooms.

**`data/edges.csv`**: `from,to,distance,is_outdoor,rush_multiplier`
- `from,to`: node ids. Paths can be walked in both directions.
- `distance`: metres. Stairs count as 20.
- `is_outdoor`: `1` for paths between buildings or outdoor places, `0` for corridors and stairs
- `rush_multiplier`: how much longer the path takes from 11:00 to 14:00 (see the factors above)

**`data/schedule.csv`**: `title,start,end,locationId`
- `start,end`: `HH:MM` in 24-hour time, zero-padded. `end` must be after `start`.
- `locationId`: a node id from `nodes.csv`
- Bad lines (wrong number of columns, bad time, unknown place, or an event that overlaps another) are skipped, and a warning is shown at startup.
