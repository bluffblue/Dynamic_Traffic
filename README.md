# Dynamic Traffic System for SA-MP

A sophisticated traffic AI system for SA-MP servers that creates realistic vehicle traffic with intelligent pathfinding and traffic light management.

## Features

- **Dynamic Vehicle Management**
  - Automatic vehicle spawning and despawning
  - Configurable traffic density
  - Intelligent pathfinding system
  - Vehicle collision detection and handling
  - Customizable vehicle speed limits

- **Traffic Light System**
  - Dynamic traffic light creation and management
  - Automatic light state changes
  - Intersection detection
  - Configurable update intervals

- **Performance Optimized**
  - Efficient distance calculations
  - Optimized path finding algorithms
  - Memory-efficient data structures
  - Built with SA-MP's limitations in mind

## Requirements

- SA-MP Server 0.3.7
- [YSI Library](https://github.com/pawn-lang/YSI-Includes)
- [Streamer Plugin](https://github.com/samp-incognito/samp-streamer-plugin)
- [foreach](https://github.com/Southclaws/foreach)

## Installation


```
Include in your script:
```pawn
#include <dynamic_traffic>
```
```

## Usage

1. Initialize the system in your gamemode:
```pawn
public OnGameModeInit() {
    InitializeTrafficSystem();
    AdjustTrafficDensity(1.0);
    return 1;
}
```

2. Clean up on gamemode exit:
```pawn
public OnGameModeExit() {
    ShutdownTrafficSystem();
    return 1;
}
```

## Configuration

You can modify these defines in `dynamic_traffic.inc`:

```pawn
#define MAX_TRAFFIC_VEHICLES 100      // Maximum number of AI vehicles
#define MAX_TRAFFIC_PATHS 50          // Maximum number of paths
#define MAX_PATH_POINTS 20            // Maximum points per path
#define VEHICLE_UPDATE_INTERVAL 500   // Update interval in milliseconds
#define TRAFFIC_LIGHT_UPDATE_INTERVAL 10000  // Traffic light update interval
```

## Functions

### Vehicle Management
- `CreateTrafficVehicle(modelid, Float:x, Float:y, Float:z, Float:angle)`
- `DestroyTrafficVehicle(vehicleid)`
- `AdjustTrafficDensity(Float:multiplier)`

### Traffic Light Management
- `CreateTrafficLight(Float:x, Float:y, Float:z)`
- `DestroyTrafficLight(lightid)`
- `UpdateTrafficLight(lightid)`

### System Control
- `InitializeTrafficSystem()`
- `ShutdownTrafficSystem()`

## Contributing

Feel free to contribute to this project by:
1. Reporting bugs
2. Suggesting new features
3. Creating pull requests

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Credits

Created by duo sigma n jomok[janeblue and xenojagocatur] for SA-MP community.
