#include <a_samp>
#include <dynamic_traffic>

main() {
    print("Dynamic Traffic System loaded!");
}

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