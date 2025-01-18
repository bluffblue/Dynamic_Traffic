#include <a_samp>
#include <dynamic_traffic>

main() {
    print("Dynamic Traffic System loaded!");
}

public OnGameModeInit() {
    InitializeTrafficSystem();
    AdjustTrafficDensity(1.0);
    return 1;
}

public OnGameModeExit() {
    ShutdownTrafficSystem();
    return 1;
}
