# Dynamic Traffic System for SA-MP

Advanced traffic AI system for SA-MP servers that creates realistic vehicle traffic with intelligent pathfinding and dynamic weather effects.

## Core Features

### Traffic Management
- Automated vehicle spawning and despawning
- Adjustable traffic density in real-time
- Smart pathfinding with collision avoidance
- Dynamic speed limits per road
- Realistic vehicle behavior at intersections

### Weather System
- Dynamic weather effects on traffic behavior
- Speed adjustments based on weather conditions
- Enhanced vehicle handling during adverse weather
- Realistic braking distance calculations
- Slip risk system during rain and storms

### Traffic Light System
- Intelligent traffic light management
- Automatic intersection detection
- Smart timing adjustments
- Priority handling for emergency routes

## Technical Specifications

### Performance
- Optimized for high-performance
- Efficient memory usage
- Minimal CPU impact
- Smart resource allocation
- Automatic cleanup systems

### System Requirements
- SA-MP Server 0.3.7
- YSI Library
- Streamer Plugin
- foreach Include

## Installation

1. Copy files to your server:
```
scriptfiles/
└── include/
    ├── dynamic_traffic.inc
    └── weather_traffic.inc
```

2. Include in your script:
```pawn
#include <dynamic_traffic>
```

## Usage

### Basic Implementation
```pawn
public OnGameModeInit() {
    InitializeTrafficSystem();
    InitializeWeatherSystem();
    AdjustTrafficDensity(1.0);
    SetWeatherState(WEATHER_CLEAR);
    return 1;
}

public OnGameModeExit() {
    ShutdownTrafficSystem();
    ShutdownWeatherSystem();
    return 1;
}
```

### Weather States
- WEATHER_CLEAR: Normal traffic behavior
- WEATHER_RAINY: Reduced speeds, increased braking distance
- WEATHER_FOGGY: Significantly reduced speeds
- WEATHER_STORMY: Maximum safety measures

### Traffic Control
```pawn
AdjustTrafficDensity(1.0);        // Normal density
SetWeatherState(WEATHER_RAINY);    // Enable rain effects
CreateTrafficLight(x, y, z);       // Add traffic light
```

## Configuration

### System Limits
```pawn
#define MAX_TRAFFIC_VEHICLES     100
#define MAX_TRAFFIC_PATHS       50
#define MAX_PATH_POINTS        20
#define MAX_TRAFFIC_LIGHTS     50
```

### Update Intervals
```pawn
#define VEHICLE_UPDATE_INTERVAL         500
#define TRAFFIC_LIGHT_UPDATE_INTERVAL  10000
```

## API Functions

### Traffic Management
- `CreateTrafficVehicle(modelid, Float:x, Float:y, Float:z, Float:angle)`
- `DestroyTrafficVehicle(vehicleid)`
- `AdjustTrafficDensity(Float:multiplier)`
- `SetPathSpeedLimit(pathid, Float:speed)`

### Weather Control
- `SetWeatherState(weatherState)`
- `GetWeatherSpeed()`
- `GetWeatherBraking()`
- `GetWeatherVisibility()`
- `GetWeatherSlip()`

### Traffic Light Control
- `CreateTrafficLight(Float:x, Float:y, Float:z)`
- `UpdateTrafficLight(lightid)`
- `DestroyTrafficLight(lightid)`

## Examples

### Complete System Implementation
```pawn
#include <a_samp>
#include <YSI_Data\y_foreach>
#include <streamer>
#include <dynamic_traffic>

new Float:g_TrafficSpawnPoints[][] = {
    {1234.5, 2345.6, 10.5, 90.0},
    {2345.6, 3456.7, 10.5, 180.0},
    {3456.7, 4567.8, 10.5, 270.0}
};

new g_VehicleModels[] = {
    400, 401, 402, 405, 410,  // Civilian cars
    416, 420, 421, 426, 436   // More variety
};

public OnGameModeInit() {
    // Initialize core systems
    InitializeTrafficSystem();
    InitializeWeatherSystem();
    
    // Set up traffic paths
    for(new i = 0; i < sizeof(g_TrafficSpawnPoints); i++) {
        CreateTrafficVehicle(
            g_VehicleModels[random(sizeof(g_VehicleModels))],
            g_TrafficSpawnPoints[i][0],
            g_TrafficSpawnPoints[i][1],
            g_TrafficSpawnPoints[i][2],
            g_TrafficSpawnPoints[i][3]
        );
    }
    
    // Configure system settings
    AdjustTrafficDensity(1.0);
    SetWeatherState(WEATHER_CLEAR);
    
    // Create traffic lights at major intersections
    CreateTrafficLight(1234.5, 2345.6, 10.5);
    CreateTrafficLight(3456.7, 4567.8, 10.5);
    
    return 1;
}

public OnGameModeExit() {
    ShutdownTrafficSystem();
    ShutdownWeatherSystem();
    return 1;
}

// Dynamic weather changes
public OnWeatherUpdate() {
    new hour;
    gettime(hour);
    
    // Morning: Clear weather
    if(hour >= 6 && hour < 12) {
        SetWeatherState(WEATHER_CLEAR);
        AdjustTrafficDensity(1.0);
    }
    // Afternoon: Possible rain
    else if(hour >= 12 && hour < 18) {
        if(random(100) < 30) {
            SetWeatherState(WEATHER_RAINY);
            AdjustTrafficDensity(0.7);
        }
    }
    // Night: Foggy conditions
    else if(hour >= 18 || hour < 6) {
        SetWeatherState(WEATHER_FOGGY);
        AdjustTrafficDensity(0.5);
    }
    return 1;
}

// Emergency vehicle priority
public OnEmergencyVehicleSpawn(vehicleid) {
    // Clear traffic in radius
    new Float:x, Float:y, Float:z;
    GetVehiclePos(vehicleid, x, y, z);
    
    foreach(new i : ActiveVehicles) {
        new trafficveh = g_TrafficVehicles[i][vehID];
        if(GetVehicleDistanceFromPoint(trafficveh, x, y, z) < 50.0) {
            new Float:angle;
            GetVehicleZAngle(trafficveh, angle);
            SetVehiclePos(trafficveh, x + (50.0 * floatsin(-angle, degrees)),
                y + (50.0 * floatcos(-angle, degrees)), z);
        }
    }
    return 1;
}

// Custom path speed limits
public OnPathCreate(pathid) {
    new Float:length = CalculatePathLength(pathid);
    
    if(length < 100.0) {
        SetPathSpeedLimit(pathid, 30.0);  // Slow for short paths
    }
    else if(length < 500.0) {
        SetPathSpeedLimit(pathid, 60.0);  // Medium for regular roads
    }
    else {
        SetPathSpeedLimit(pathid, 90.0);  // Fast for highways
    }
    return 1;
}
```

### Weather System Examples
```pawn
// Dynamic weather cycle
public OnGameModeInit() {
    SetTimer("WeatherCycle", 1800000, true);  // Every 30 minutes
    return 1;
}

forward WeatherCycle();
public WeatherCycle() {
    switch(random(4)) {
        case 0: {
            SetWeatherState(WEATHER_CLEAR);
            AdjustTrafficDensity(1.0);
        }
        case 1: {
            SetWeatherState(WEATHER_RAINY);
            AdjustTrafficDensity(0.7);
        }
        case 2: {
            SetWeatherState(WEATHER_FOGGY);
            AdjustTrafficDensity(0.5);
        }
        case 3: {
            SetWeatherState(WEATHER_STORMY);
            AdjustTrafficDensity(0.3);
        }
    }
}
```

### Traffic Management Examples
```pawn
// Rush hour traffic adjustment
public OnRushHour(bool:isRushHour) {
    if(isRushHour) {
        AdjustTrafficDensity(2.0);
        foreach(new i : ActiveVehicles) {
            new pathid = GetVehiclePath(g_TrafficVehicles[i][vehID]);
            SetPathSpeedLimit(pathid, GetSpeedLimit(pathid) * 0.7);
        }
    }
    else {
        AdjustTrafficDensity(1.0);
        foreach(new i : ActiveVehicles) {
            new pathid = GetVehiclePath(g_TrafficVehicles[i][vehID]);
            SetPathSpeedLimit(pathid, GetSpeedLimit(pathid) / 0.7);
        }
    }
    return 1;
}
```

## Support

For issues, suggestions, or contributions:
1. Open an issue on GitHub
2. Describe the problem or enhancement
3. Provide relevant code examples
4. Include server details and error messages

## License

This project uses the MIT License. See LICENSE file for details.

## Credits

Created by duo sigma n jomok [bluffblue] for SA-MP community.
