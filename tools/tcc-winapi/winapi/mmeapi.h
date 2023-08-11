//< @r-lyeh, @todo: complete this file. for now, it handles only what I needed to support MIDI out on tcc+win32

#ifndef MMEAPI_H
#define MMEAPI_H

//DECLARE_HANDLE(HMIDIOUT);
typedef HMIDIOUT FAR *LPHMIDIOUT;

#define _Out_
#define _In_
#define _In_opt_

/* MIDI function prototypes */
WINMMAPI
UINT
WINAPI
midiOutGetNumDevs(
    void
    );


WINMMAPI
MMRESULT
WINAPI
midiOutOpen(
    _Out_ LPHMIDIOUT phmo,
    _In_ UINT uDeviceID,
    _In_opt_ DWORD_PTR dwCallback,
    _In_opt_ DWORD_PTR dwInstance,
    _In_ DWORD fdwOpen
    );


WINMMAPI
MMRESULT
WINAPI
midiOutClose(
    _In_ HMIDIOUT hmo
    );

WINMMAPI
MMRESULT
WINAPI
midiOutShortMsg(
    _In_ HMIDIOUT hmo,
    _In_ DWORD dwMsg
    );

#undef _In_opt_
#undef _In_
#undef _Out_

#endif
