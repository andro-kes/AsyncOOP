# AsyncOOP - Laboratory Work #7

## Asynchronous Programming: Wandering Knight

This project implements an asynchronous NPC combat simulation using C++ threading primitives.

### Variant 4: Wandering Knight

**Specifications:**
- Movement range: 30 units
- Kill range: 10 units
- Map size: 100x100
- Number of NPCs: 50
- Simulation duration: 30 seconds

### Architecture

**Three concurrent threads:**
1. **Movement Thread**: Handles NPC movement and combat detection
2. **Combat Thread**: Processes combat events using d6 dice (attack/defense)
3. **Print Thread**: Displays map state every 1 second

### Synchronization

- `std::shared_mutex` for NPCs collection (allows multiple readers)
- `std::lock_guard` for exclusive access to shared resources
- `std::mutex` for protecting std::cout
- `std::condition_variable` for combat queue notification

### NPC Types

1. **Knight** (K)
   - Movement range: 30
   - Kill range: 10
   - Attack bonus: +1
   - Defense bonus: +1

2. **Squirrel** (S)
   - Movement range: 5
   - Kill range: 5
   - Attack bonus: 0
   - Defense bonus: +1

3. **Pegasus** (P)
   - Movement range: 30
   - Kill range: 10
   - Attack bonus: 0
   - Defense bonus: +2

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Running

```bash
./lab7
```

The program will:
1. Initialize 50 NPCs randomly on a 100x100 map
2. Run for 30 seconds with three concurrent threads
3. Display map updates every second
4. Show combat events in real-time
5. Print final statistics at the end

### Combat System

Combat occurs when two NPCs are within kill range. Each combatant rolls a d6 (1-6) and adds their respective bonuses:
- Attacker: d6 + attack bonus
- Defender: d6 + defense bonus

If the attack roll is higher than the defense roll, the defender is eliminated.

### Thread Safety

All shared data structures are protected:
- NPC collection: `std::shared_mutex` (read-write lock)
- Combat queue: `std::mutex` with condition variable
- Console output: `std::mutex` to prevent interleaved output