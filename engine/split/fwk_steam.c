// ----------------------------------------------------------------------------
// steam framework
// - rlyeh, public domain
//
// hints:
// - steam must be running in background
// - steamworks dll must be close to executable (may be "steam_api64.dll", "libsteam_api.dylib" or "libsteam_api.so")
// - family mode should be off (achievements not displayed otherwise)
//
// [src] https://steamdb.info/app/480/
// [src] https://steamdb.info/app/480/stats/
// [src] https://partner.steamgames.com/doc/
// [src] dumpbin /exports steam_api64.dll

#ifndef STEAM_APPID
#define STEAM_APPID 480
#endif

#ifndef STEAM_DLL
#define STEAM_DLL ifdef(win32, "steam_api64.dll", "libsteam_api" ifdef(osx, ".dylib", ".so"))
#endif

#define STEAM_API_DECL(ret,name,args) API ret (*name) args;
#define STEAM_API_DEFN(ret,name,args) ret (*name) args;
#define STEAM_API_LOAD(ret,name,args) name = dll(STEAM_DLL, #name); if(!name) PRINTF("Cannot load `" STEAM_DLL "@%s`\n", #name);

STEAM_API(STEAM_API_DECL);
STEAM_API(STEAM_API_DEFN);

struct steam_t {
    intptr_t iclient;
    intptr_t iapps;
    intptr_t ifriends;
    intptr_t ihtmlsurface;
    intptr_t imatchmaking;
    intptr_t imatchmakingservers;
    intptr_t inetworking;
    intptr_t iremoteplay;
    intptr_t iremotestorage;
    intptr_t iscreenshots;
    intptr_t iuser;
    intptr_t iuserstats;
    intptr_t iutils;
    uint64_t steamid;

    int num_friends;
    bool running, overlay;
    bool logged, behindnat;
    char status[256], username[256], language[32];
} steam = {0};

static
void steam_message_cb(int severity, const char *message) {
    /**/ if( severity == 0 ) printf("%s", message);
    else if( severity == 1 ) printf("Warning: %s", message);
    else printf("Unknown severity %d: %s", severity, message);
}

bool steam_init(unsigned app_id) {
    struct steam_t z = {0};
    steam = z;

    app_id = app_id ? app_id : STEAM_APPID;

    // Steam installed?
    HKEY hSteamProcess;
    if( RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_READ, &hSteamProcess) ) {
        return !strcpy(steam.status, "Err: steam not installed");
    }
    RegCloseKey(hSteamProcess);

    // dll present?
    if( !file_exist(STEAM_DLL) ) {
        return !strcpy(steam.status, "Err: " STEAM_DLL " not found");
    }

    // Load symbols
    STEAM_API(STEAM_API_LOAD);
    if( !SteamAPI_Init ) SteamAPI_Init = SteamAPI_InitSafe;

    // Initialize
    char *app_id_str = va("%d", app_id);
    //if( !file_exist("steam_appid.txt") ) file_write("steam_appid.txt", app_id_str, strlen(app_id_str));
    if( !getenv("SteamAppId") ) SetEnvironmentVariableA("SteamAppId", app_id_str);

    int started = SteamAPI_Init && SteamAPI_Init();
    if( !started ) {
        return !strcpy(steam.status, "Err: steam not running");
    }

    SteamAPI_RestartAppIfNecessary(app_id);

    // Create interfaces
    steam.iclient = (intptr_t)SteamInternal_CreateInterface("SteamClient020");
    if( !steam.iclient ) {
        return !strcpy(steam.status, "Err: incompatible dll");
    }

    HSteamPipe hpipe = SteamAPI_ISteamClient_CreateSteamPipe(steam.iclient);
    HSteamUser huser = SteamAPI_ISteamClient_ConnectToGlobalUser(steam.iclient, hpipe);

    steam.iapps = (intptr_t)SteamAPI_ISteamClient_GetISteamApps(steam.iclient, huser, hpipe, "STEAMAPPS_INTERFACE_VERSION008");
    steam.ifriends = (intptr_t)SteamAPI_ISteamClient_GetISteamFriends(steam.iclient, huser, hpipe, "SteamFriends017"); // 015
    steam.ihtmlsurface = (intptr_t)SteamAPI_ISteamClient_GetISteamHTMLSurface(steam.iclient, huser, hpipe, "STEAMHTMLSURFACE_INTERFACE_VERSION_005");
    steam.imatchmaking = (intptr_t)SteamAPI_ISteamClient_GetISteamMatchmaking(steam.iclient, huser, hpipe, "SteamMatchMaking009");
    steam.imatchmakingservers = (intptr_t)SteamAPI_ISteamClient_GetISteamMatchmakingServers(steam.iclient, huser, hpipe, "SteamMatchMakingServers002");
    steam.inetworking = (intptr_t)SteamAPI_ISteamClient_GetISteamNetworking(steam.iclient, huser, hpipe, "SteamNetworking006");
    //steam.iremoteplay = (intptr_t)SteamAPI_ISteamClient_GetISteamRemotePlay(steam.iclient, huser, hpipe, "STEAMREMOTEPLAY_INTERFACE_VERSION001");
    steam.iremotestorage = (intptr_t)SteamAPI_ISteamClient_GetISteamRemoteStorage(steam.iclient, huser, hpipe, "STEAMREMOTESTORAGE_INTERFACE_VERSION014");
    steam.iscreenshots = (intptr_t)SteamAPI_ISteamClient_GetISteamScreenshots(steam.iclient, huser, hpipe, "STEAMSCREENSHOTS_INTERFACE_VERSION003");
    steam.iuser = (intptr_t)SteamAPI_ISteamClient_GetISteamUser(steam.iclient, huser, hpipe, "SteamUser021"); // 019
    steam.iuserstats = (intptr_t)SteamAPI_ISteamClient_GetISteamUserStats(steam.iclient, huser, hpipe, "STEAMUSERSTATS_INTERFACE_VERSION012");
    steam.iutils = (intptr_t)SteamAPI_ISteamClient_GetISteamUtils(steam.iclient, hpipe, "SteamUtils010");

    SteamAPI_ISteamClient_SetWarningMessageHook(steam.iclient, steam_message_cb);

    // Retrieve some vars
    steam.running = SteamAPI_IsSteamRunning();
    steam.steamid = SteamAPI_ISteamUser_GetSteamID(steam.iuser);
    steam.logged = SteamAPI_ISteamUser_BLoggedOn(steam.iuser);
    steam.behindnat = SteamAPI_ISteamUser_BIsBehindNAT(steam.iuser);
    steam.num_friends = SteamAPI_ISteamFriends_GetFriendCount(steam.ifriends, k_EFriendFlagAll);
    strncpy(steam.username, SteamAPI_ISteamFriends_GetPersonaName(steam.ifriends), sizeof(steam.username));
    strncpy(steam.language, SteamAPI_ISteamUtils_GetSteamUILanguage(steam.iutils), sizeof(steam.language)); // SteamAPI_ISteamApps_GetCurrentGameLanguage(steam.iapps)

    if(steam.logged)
    SteamAPI_ISteamUserStats_RequestCurrentStats(steam.iuserstats);

    ASSERT(steam.iapps);
    ASSERT(steam.ifriends);
    ASSERT(steam.ihtmlsurface);
    ASSERT(steam.imatchmaking);
    ASSERT(steam.imatchmakingservers);
    ASSERT(steam.inetworking);
    // ASSERT(steam.iremoteplay);
    ASSERT(steam.iremotestorage);
    ASSERT(steam.iscreenshots);
    ASSERT(steam.iuser);
    ASSERT(steam.iuserstats);
    ASSERT(steam.iutils);

    strcpy(steam.status, "Ok");
    return true;
}

void steam_tick() {
    if( steam.iclient ) {
        SteamAPI_RunCallbacks();
        steam.overlay = SteamAPI_ISteamUtils_IsOverlayEnabled(steam.iutils);
   }
}

void steam_trophy(const char *trophy_id, bool redeem) {
    if( steam.iclient && steam.logged ) {
        if( redeem )
        SteamAPI_ISteamUserStats_SetAchievement(steam.iuserstats, trophy_id);
        else
        SteamAPI_ISteamUserStats_ClearAchievement(steam.iuserstats, trophy_id);
        SteamAPI_ISteamUserStats_StoreStats(steam.iuserstats);
    }
}

void steam_screenshot() {
    if( steam.iclient ) {
        SteamAPI_ISteamScreenshots_TriggerScreenshot(steam.iscreenshots);
    }
}

void steam_destroy() {
    if( steam.iclient ) {
        steam.iclient = 0;
        SteamAPI_Shutdown();
    }
}

int ui_steam() {
    ui_disable();

    ui_label2("Status", steam.status);
    ui_label2("Username", steam.username);
    ui_label2("Language", steam.language);
    ui_label2("Friends", va("%d", steam.num_friends));
    ui_label2("SteamID", va("%llu", steam.steamid));
    ui_bool("Overlay?", &steam.overlay);

    ui_enable();

    return 0;
}
