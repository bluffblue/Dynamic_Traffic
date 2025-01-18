#if defined _dynamic_traffic_included
    #endinput
#endif
#define _dynamic_traffic_included


#include <YSI_Data\y_foreach>
#include <streamer>
#include <YSI_Coding\y_hooks>

#define MAX_TRAFFIC_VEHICLES 100
#define MAX_TRAFFIC_PATHS 50
#define MAX_PATH_POINTS 20
#define VEHICLE_UPDATE_INTERVAL 500
#define MIN_SPEED_LIMIT 30.0
#define MAX_SPEED_LIMIT 120.0
#define TRAFFIC_DENSITY_MULTIPLIER 1.0
#define MAX_TRAFFIC_LIGHTS 50
#define TRAFFIC_LIGHT_UPDATE_INTERVAL 10000
#define DEFAULT_SPAWN_Z 3.0

#if !defined Timer
    #define Timer: _:
#endif

enum E_TRAFFIC_VEHICLE {
    vehID,
    vehModel,
    Float:vehSpeed,
    Float:vehHealth,
    currentPath,
    currentPoint,
    bool:isActive,
    Float:destinationX,
    Float:destinationY,
    Float:destinationZ,
    lastUpdateTime
}

enum E_PATH_INFO {
    Float:pathX[MAX_PATH_POINTS],
    Float:pathY[MAX_PATH_POINTS],
    Float:pathZ[MAX_PATH_POINTS],
    totalPoints,
    bool:isLoop,
    Float:speedLimit
}

enum E_TRAFFIC_LIGHT {
    objectID,
    Float:posX,
    Float:posY,
    Float:posZ,
    bool:isGreen,
    Timer:updateTimer
}

new 
    g_TrafficVehicles[MAX_TRAFFIC_VEHICLES][E_TRAFFIC_VEHICLE],
    g_TrafficPaths[MAX_TRAFFIC_PATHS][E_PATH_INFO],
    g_TrafficLights[MAX_TRAFFIC_LIGHTS][E_TRAFFIC_LIGHT],
    Iterator:ActiveVehicles<MAX_TRAFFIC_VEHICLES>,
    Timer:g_UpdateTimer = -1,
    bool:SystemActive;

stock Float:GetDistanceBetweenPoints(Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2) {
    return floatsqroot((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2));
}

stock bool:IsIntersectionNearby(Float:x, Float:y, Float:z) {
    new Float:distance = 20.0;
    
    for(new i = 0; i < sizeof(g_TrafficPaths); i++) {
        if(g_TrafficPaths[i][totalPoints] > 0) {
            for(new j = 0; j < g_TrafficPaths[i][totalPoints]; j++) {
                if(GetDistanceBetweenPoints(x, y, z, 
                    g_TrafficPaths[i][pathX][j], 
                    g_TrafficPaths[i][pathY][j], 
                    g_TrafficPaths[i][pathZ][j]) < distance) {
                    return true;
                }
            }
        }
    }
    return false;
}

forward OnTrafficSystemUpdate();
forward Float:GetSpeedLimit(pathid);
forward InitializeTrafficSystem();
forward CreateTrafficVehicle(modelid, Float:x, Float:y, Float:z, Float:angle);
forward DestroyTrafficVehicle(vehicleid);
forward UpdateVehiclePath(vehicleid);
forward AdjustTrafficDensity(Float:multiplier);
forward SetPathSpeedLimit(pathid, Float:speed);
forward CalculateNextPoint(vehicleid);
forward ProcessTrafficLights();
forward HandleVehicleCollision(vehicleid, targetid);
forward CreateTrafficLight(Float:x, Float:y, Float:z);
forward DestroyTrafficLight(lightid);
forward UpdateTrafficLight(lightid);
forward ShutdownTrafficSystem();

stock InitializeTrafficSystem() {
    if(SystemActive) return 0;
    SystemActive = true;
    Iter_Init(ActiveVehicles);
    g_UpdateTimer = SetTimer("OnTrafficSystemUpdate", VEHICLE_UPDATE_INTERVAL, true);
    return 1;
}

stock ShutdownTrafficSystem() {
    if(!SystemActive) return 0;
    
    if(g_UpdateTimer != -1) {
        KillTimer(g_UpdateTimer);
        g_UpdateTimer = -1;
    }
    
    foreach(new i : ActiveVehicles) {
        DestroyTrafficVehicle(g_TrafficVehicles[i][vehID]);
    }
    
    for(new i = 0; i < MAX_TRAFFIC_LIGHTS; i++) {
        if(g_TrafficLights[i][objectID] != INVALID_OBJECT_ID) {
            DestroyTrafficLight(i);
        }
    }
    
    SystemActive = false;
    return 1;
}

stock CreateTrafficVehicle(modelid, Float:x, Float:y, Float:z, Float:angle) {
    new idx = Iter_Free(ActiveVehicles);
    if(idx == ITER_NONE) return INVALID_VEHICLE_ID;
    
    g_TrafficVehicles[idx][vehID] = CreateVehicle(modelid, x, y, z, angle, -1, -1, -1);
    if(g_TrafficVehicles[idx][vehID] == INVALID_VEHICLE_ID) return INVALID_VEHICLE_ID;
    
    g_TrafficVehicles[idx][vehModel] = modelid;
    g_TrafficVehicles[idx][vehSpeed] = 0.0;
    g_TrafficVehicles[idx][vehHealth] = 1000.0;
    g_TrafficVehicles[idx][isActive] = true;
    g_TrafficVehicles[idx][currentPath] = -1;
    g_TrafficVehicles[idx][currentPoint] = 0;
    
    Iter_Add(ActiveVehicles, idx);
    return g_TrafficVehicles[idx][vehID];
}

stock DestroyTrafficVehicle(vehicleid) {
    foreach(new i : ActiveVehicles) {
        if(g_TrafficVehicles[i][vehID] == vehicleid) {
            DestroyVehicle(g_TrafficVehicles[i][vehID]);
            g_TrafficVehicles[i][isActive] = false;
            Iter_Remove(ActiveVehicles, i);
            return 1;
        }
    }
    return 0;
}

public OnTrafficSystemUpdate() {
    if(!SystemActive) return 0;
    
    new Float:x, Float:y, Float:z;
    foreach(new i : ActiveVehicles) {
        if(!g_TrafficVehicles[i][isActive]) continue;
        
        GetVehiclePos(g_TrafficVehicles[i][vehID], x, y, z);
        if(IsIntersectionNearby(x, y, z)) {
            ProcessTrafficLights();
        }
        
        UpdateVehiclePath(i);
        CalculateNextPoint(i);
    }
    return 1;
}

stock Float:GetSpeedLimit(pathid) {
    if(pathid < 0 || pathid >= MAX_TRAFFIC_PATHS) return MIN_SPEED_LIMIT;
    return g_TrafficPaths[pathid][speedLimit];
}

stock SetPathSpeedLimit(pathid, Float:speed) {
    if(pathid < 0 || pathid >= MAX_TRAFFIC_PATHS) return 0;
    g_TrafficPaths[pathid][speedLimit] = speed;
    return 1;
}

stock AdjustTrafficDensity(Float:multiplier) {
    if(multiplier < 0.0 || multiplier > 2.0) return 0;
    
    new targetCount = floatround(MAX_TRAFFIC_VEHICLES * multiplier);
    new currentCount = Iter_Count(ActiveVehicles);
    
    if(targetCount > currentCount) {
        for(new i = currentCount; i < targetCount; i++) {
            new Float:x = float(random(4000) - 2000);
            new Float:y = float(random(4000) - 2000);
            CreateTrafficVehicle(400 + random(211), x, y, DEFAULT_SPAWN_Z, float(random(360)));
        }
    }
    else if(targetCount < currentCount) {
        new removed = 0;
        foreach(new i : ActiveVehicles) {
            if(removed >= (currentCount - targetCount)) break;
            DestroyTrafficVehicle(g_TrafficVehicles[i][vehID]);
            removed++;
        }
    }
    return 1;
}

stock UpdateVehiclePath(vehicleid) {
    new Float:x, Float:y, Float:z, Float:angle;
    GetVehiclePos(vehicleid, x, y, z);
    GetVehicleZAngle(vehicleid, angle);
    
    new Float:distance = GetVehicleDistanceFromPoint(vehicleid, 
        g_TrafficVehicles[vehicleid][destinationX],
        g_TrafficVehicles[vehicleid][destinationY],
        g_TrafficVehicles[vehicleid][destinationZ]);
        
    if(distance < 5.0) {
        CalculateNextPoint(vehicleid);
    }
    return 1;
}

stock HandleVehicleCollision(vehicleid, targetid) {
    new Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(vehicleid, vx, vy, vz);
    
    new Float:speed = floatsqroot(vx * vx + vy * vy + vz * vz) * 100.0;
    if(speed > 30.0) {
        new Float:vh;
        GetVehicleHealth(vehicleid, vh);
        SetVehicleHealth(vehicleid, vh - (speed * 2));
    }
    return 1;
}

stock CalculateNextPoint(vehicleid) {
    new pathid = g_TrafficVehicles[vehicleid][currentPath];
    if(pathid == -1 || pathid >= MAX_TRAFFIC_PATHS) {
        pathid = random(MAX_TRAFFIC_PATHS);
        g_TrafficVehicles[vehicleid][currentPath] = pathid;
        g_TrafficVehicles[vehicleid][currentPoint] = 0;
    }
    
    new pointid = g_TrafficVehicles[vehicleid][currentPoint];
    if(pointid >= g_TrafficPaths[pathid][totalPoints]) {
        if(g_TrafficPaths[pathid][isLoop]) {
            pointid = 0;
        } else {
            pathid = random(MAX_TRAFFIC_PATHS);
            pointid = 0;
        }
        g_TrafficVehicles[vehicleid][currentPath] = pathid;
        g_TrafficVehicles[vehicleid][currentPoint] = pointid;
    }
    
    g_TrafficVehicles[vehicleid][destinationX] = g_TrafficPaths[pathid][pathX][pointid];
    g_TrafficVehicles[vehicleid][destinationY] = g_TrafficPaths[pathid][pathY][pointid];
    g_TrafficVehicles[vehicleid][destinationZ] = g_TrafficPaths[pathid][pathZ][pointid];
    
    return 1;
}

stock CreateTrafficLight(Float:x, Float:y, Float:z) {
    for(new i = 0; i < MAX_TRAFFIC_LIGHTS; i++) {
        if(g_TrafficLights[i][objectID] == INVALID_OBJECT_ID) {
            g_TrafficLights[i][objectID] = CreateDynamicObject(1283, x, y, z, 0.0, 0.0, 0.0);
            g_TrafficLights[i][posX] = x;
            g_TrafficLights[i][posY] = y;
            g_TrafficLights[i][posZ] = z;
            g_TrafficLights[i][isGreen] = true;
            g_TrafficLights[i][updateTimer] = SetTimerEx("UpdateTrafficLight", TRAFFIC_LIGHT_UPDATE_INTERVAL, true, "i", i);
            return i;
        }
    }
    return -1;
}

stock DestroyTrafficLight(lightid) {
    if(lightid >= 0 && lightid < MAX_TRAFFIC_LIGHTS) {
        if(g_TrafficLights[lightid][objectID] != INVALID_OBJECT_ID) {
            DestroyDynamicObject(g_TrafficLights[lightid][objectID]);
            KillTimer(g_TrafficLights[lightid][updateTimer]);
            g_TrafficLights[lightid][objectID] = INVALID_OBJECT_ID;
            return 1;
        }
    }
    return 0;
}

stock UpdateTrafficLight(lightid) {
    if(lightid >= 0 && lightid < MAX_TRAFFIC_LIGHTS) {
        if(g_TrafficLights[lightid][objectID] != INVALID_OBJECT_ID) {
            g_TrafficLights[lightid][isGreen] = !g_TrafficLights[lightid][isGreen];
            SetDynamicObjectMaterial(g_TrafficLights[lightid][objectID], 0, 18646, "matcolours", g_TrafficLights[lightid][isGreen] ? "green" : "red", 0);
            return 1;
        }
    }
    return 0;
}

stock ProcessTrafficLights() {
    for(new i = 0; i < MAX_TRAFFIC_LIGHTS; i++) {
        if(g_TrafficLights[i][objectID] != INVALID_OBJECT_ID) {
            UpdateTrafficLight(i);
        }
    }
    return 1;
}

public OnVehicleDamageStatusUpdate(vehicleid, playerid) {
    foreach(new i : ActiveVehicles) {
        if(g_TrafficVehicles[i][vehID] == vehicleid) {
            new Float:health;
            GetVehicleHealth(vehicleid, health);
            if(health < 300.0) {
                DestroyTrafficVehicle(vehicleid);
            }
            break;
        }
    }
    return 1;
}
