#if defined _weather_traffic_included
    #endinput
#endif
#define _weather_traffic_included

#include <YSI_Data\y_foreach>
#include <streamer>
#include <dynamic_traffic.inc>

forward ProcessWeatherForVehicle(vehicleid);
forward UpdateWeatherSystem();

enum {
    WEATHER_CLEAR,
    WEATHER_RAINY,
    WEATHER_FOGGY,
    WEATHER_STORMY
}

enum {
    currentState,
    visibilityMultiplier,
    speedMultiplier,
    brakingMultiplier,
    slipChance,
    isTransitioning
}

static 
    g_WeatherData[6],
    g_WeatherTimer,
    bool:g_WeatherSystemActive;

static Float:GetWeatherSpeedMultiplier(weatherState) {
    switch(weatherState) {
        case WEATHER_CLEAR: return 1.0;
        case WEATHER_RAINY: return 0.7;
        case WEATHER_FOGGY: return 0.5;
        case WEATHER_STORMY: return 0.4;
    }
    return 1.0;
}

static Float:GetWeatherBrakingMultiplier(weatherState) {
    switch(weatherState) {
        case WEATHER_CLEAR: return 1.0;
        case WEATHER_RAINY: return 1.5;
        case WEATHER_FOGGY: return 1.3;
        case WEATHER_STORMY: return 2.0;
    }
    return 1.0;
}

static Float:GetWeatherVisibilityMultiplier(weatherState) {
    switch(weatherState) {
        case WEATHER_CLEAR: return 1.0;
        case WEATHER_RAINY: return 0.6;
        case WEATHER_FOGGY: return 0.3;
        case WEATHER_STORMY: return 0.2;
    }
    return 1.0;
}

static Float:GetWeatherSlipChance(weatherState) {
    switch(weatherState) {
        case WEATHER_CLEAR: return 0.0;
        case WEATHER_RAINY: return 0.3;
        case WEATHER_FOGGY: return 0.1;
        case WEATHER_STORMY: return 0.5;
    }
    return 0.0;
}

stock InitializeWeatherSystem() {
    if(g_WeatherSystemActive) return 0;
    
    g_WeatherData[currentState] = WEATHER_CLEAR;
    g_WeatherData[visibilityMultiplier] = _:1.0;
    g_WeatherData[speedMultiplier] = _:1.0;
    g_WeatherData[brakingMultiplier] = _:1.0;
    g_WeatherData[slipChance] = _:0.0;
    g_WeatherData[isTransitioning] = false;
    
    g_WeatherTimer = SetTimer("UpdateWeatherSystem", 1000, true);
    g_WeatherSystemActive = true;
    return 1;
}

stock ShutdownWeatherSystem() {
    if(!g_WeatherSystemActive) return 0;
    
    if(g_WeatherTimer != -1) {
        KillTimer(g_WeatherTimer);
        g_WeatherTimer = -1;
    }
    
    g_WeatherSystemActive = false;
    return 1;
}

stock SetWeatherState(weatherState) {
    if(!g_WeatherSystemActive) return 0;
    
    g_WeatherData[currentState] = weatherState;
    g_WeatherData[visibilityMultiplier] = _:GetWeatherVisibilityMultiplier(weatherState);
    g_WeatherData[speedMultiplier] = _:GetWeatherSpeedMultiplier(weatherState);
    g_WeatherData[brakingMultiplier] = _:GetWeatherBrakingMultiplier(weatherState);
    g_WeatherData[slipChance] = _:GetWeatherSlipChance(weatherState);
    
    return 1;
}

stock Float:GetWeatherSpeed() {
    return Float:g_WeatherData[speedMultiplier];
}

stock Float:GetWeatherBraking() {
    return Float:g_WeatherData[brakingMultiplier];
}

stock Float:GetWeatherVisibility() {
    return Float:g_WeatherData[visibilityMultiplier];
}

stock Float:GetWeatherSlip() {
    return Float:g_WeatherData[slipChance];
}

stock ProcessVehicleWeatherBehavior(vehicleid) {
    if(!g_WeatherSystemActive) return 0;
    
    new Float:speed, Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(vehicleid, vx, vy, vz);
    speed = floatsqroot(vx*vx + vy*vy + vz*vz) * 100.0;
    
    new Float:maxSpeed = GetSpeedLimit(GetVehiclePath(vehicleid)) * Float:g_WeatherData[speedMultiplier];
    if(speed > maxSpeed) {
        vx *= maxSpeed/speed;
        vy *= maxSpeed/speed;
        SetVehicleVelocity(vehicleid, vx, vy, vz);
    }
    
    if(random(100) < floatround(Float:g_WeatherData[slipChance] * 100.0) && speed > 50.0) {
        new Float:angle;
        GetVehicleZAngle(vehicleid, angle);
        angle += float(random(20) - 10);
        SetVehicleZAngle(vehicleid, angle);
    }
    
    return 1;
}

public UpdateWeatherSystem() {
    if(!g_WeatherSystemActive) return 0;
    CallRemoteFunction("ProcessWeatherUpdate", "");
    return 1;
}
