#ifndef CLUACONSOLE_H_
#define CLUACONSOLE_H_

#include "CVariableHash.h"

// NOTE: Name is Ghidra's. This is the cheat console command parser. The only
// known user (0x77BF30) keeps it as a stack local, so its size is unknown
// (at least 0x41); its constructor and destructor are not translated yet.
class CLUAConsole {
public:
    // 0x5838A0
    void Input(LPCSTR pszInput);

    // 0x583E60
    void GetCommandID(CString sCommand);

    // 0x5840E0
    void ParseArguments(CString sArguments);

    // 0x5842C0
    void AddStringArgument(CString sArgument);

    // 0x584320
    void CommandNotFound();

    // 0x584400
    CString GetCommandText();

    // 0x584420
    int CountArgs();

    // 0x5844E0
    CString Encode(CString sText);

    /* 0000 */ int m_nCommand;
    /* 0004 */ CString m_sCommand;
    /* 0008 */ int m_nNumberArgs;
    /* 000C */ int m_nStringArgs;
    /* 0010 */ CDWordArray m_aNumberArgs;
    /* 0024 */ CStringArray m_aStringArgs;
    /* 0038 */ CCtrlAltDelete m_cheat;
    /* 0039 */ BYTE field_39[7];
    /* 0040 */ CCtrlAltDelete m_cheat2;
};

#endif /* CLUACONSOLE_H_ */
