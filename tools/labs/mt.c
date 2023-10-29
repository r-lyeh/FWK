https://devblogs.microsoft.com/oldnewthing/20200306-00/?p=103538

#include "engine/fwk.c"

char *trimspace(char *str) {
    for( char *s = str; *s; ++s )
        if(*s <= 32) memmove(s, s+1, strlen(s));
    return str;
}
char *trimleads(char *str) {
	int cnt = 0;
	while( str[cnt] && str[cnt] <= 32 ) ++cnt;
	if( cnt ) memmove(str, str+cnt, strlen(str) - cnt);
    return str;
}
char *trimends(char *str) {
	int len = strlen(str);
	while( len && str[len-1] <= 32 ) str[--len] = 0;
	return str;
}

int exec_popen(void* cmd) {
	int rc;
    for( FILE *fp = popen( cmd, "r" ); fp; rc = pclose(fp), fp = 0)
    {}
	return 0;
}

int exec_system(void* cmd) {
	int rc = system(cmd);
	return 0;
}

int exec_win( const char *cmd ) {
    bool ok = WinExec(va("cmd /c \"%s\"", cmd), SW_HIDE) > 31; // this is async
    return 0;
}

int exec_win2( const char *cmd ) {
    bool ok = WinExec(va("start \"\" /wait \"%s\"", cmd), SW_HIDE) > 31;
    return 0;
}

int exec_process(void *cmd) {
    char output[4096] = {0};
    // sscanf((const char *)cmd, "%[^|>\r\n]", output); // exclude pipes
	const char *pipe = strchr(cmd, '>');
    if( !pipe )
	    snprintf(output, 4096, "cmd /c \"%s\"", (const char *)cmd);
	else
    	snprintf(output, 4096, "cmd /c \"%.*s\"", (int)(pipe - (const char*)cmd), (char*)cmd);
    trimleads(output);
    trimends(output);

    STARTUPINFOA si = {0}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {0};

		HANDLE m_hChildStd_OUT_Rd = NULL;
		HANDLE m_hChildStd_OUT_Wr = NULL;
	 	// Create a pipe for the child process's STDOUT.
	    SECURITY_ATTRIBUTES saAttr = {0};
	    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	    saAttr.bInheritHandle = TRUE;
	    saAttr.lpSecurityDescriptor = NULL;
	    if (!CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0))
	        return (int)GetLastError();
	    // Ensure the read handle to the pipe for STDOUT is not inherited.
	    if (!SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	        return (int)GetLastError();

    si.hStdError = m_hChildStd_OUT_Wr;
    si.hStdOutput = m_hChildStd_OUT_Wr;
    si.dwFlags |= STARTF_USESTDHANDLES;

    if( !CreateProcessA(NULL, output,
        NULL, NULL, // dont inherit
		TRUE, // inherit handles
        0 /*CREATE_DEFAULT_ERROR_MODE|CREATE_NO_WINDOW*/, // 0|HIGH_PRIORITY_CLASS
        NULL, // "", // NULL would inherit env
        NULL, // current dir
        &si, &pi) ) {
        return (int) GetLastError();
    }

    DWORD dwExitCode = WaitForSingleObject(pi.hProcess, INFINITE);

    if( pipe )
    {
        FILE* pipe_fp = fopen(trimspace(pipe + 1), "wb");
        if (pipe_fp)
        {
            enum { BUFSIZE = 1024 };
            DWORD dwRead;
            CHAR chBuf[BUFSIZE];
            BOOL bSuccess = FALSE;

            for (;;)
            {
                bSuccess = ReadFile(m_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
                if (!bSuccess || dwRead == 0) continue;

                // Log chBuf
                fwrite(chBuf, dwRead, 1, pipe_fp);

                if (!bSuccess) break;
            }

            fclose(pipe_fp);
        }
	}

	CloseHandle(m_hChildStd_OUT_Rd);
	CloseHandle(m_hChildStd_OUT_Wr);

    return 0;
}

// https://github.com/hasherezade/exe_to_dll
// rundll32.exe test_case1.dll,Start

int main(int argc, char **argv) {
	void* thd[64] = {0};

#define TEST(fn) do { \
	printf("%11s ", #fn); remove("nul0"); \
	benchmark { \
	for( int i = 0; i < 64; ++i ) thd[i] = thread( fn, va("dir > nul%d",i)); \
	for( int i = 0; i < 64; ++i ) thread_destroy( thd[i] ); \
	} \
	if(!file_size("nul0")) puts("**Error!**"); } while(0)

	TEST( exec_process );
	TEST( exec_popen );
	TEST( exec_system );
	TEST( exec_win2 );
    TEST( exec_win );
}
