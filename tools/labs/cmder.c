#include <windows.h>
#include <stdio.h>
#include <tchar.h>

void _tmain( int argc, TCHAR *argv[] )
{
    if( argc < 2 )
    {
        printf("Usage: %s [cmdline]\n", argv[0]);
        return;
    }

    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0}; si.cb = sizeof(si);

    char cmdline[32767], *ptr = cmdline;
    for( int i = 1; i < argc; ++i ) ptr += sprintf(ptr, " %s", argv[i]);

    // Start the child process.
    if( !CreateProcess( NULL,   // No module name (use command line)
        cmdline+1,      // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return;
    }

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}
