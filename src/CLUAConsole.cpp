#include "CLUAConsole.h"

// 0x5838A0
void CLUAConsole::Input(LPCSTR pszInput)
{
    CString sInput(pszInput);

    if (sInput.GetLength() != 0) {
        sInput.MakeUpper();

        int nColon = sInput.Find(':');
        if (nColon >= 1) {
            sInput = sInput.Left(nColon);
            sInput = Encode(sInput);

            // 0x8AF630: "CTRLALTDELETE" run through Encode().
            if (strcmp(sInput, "\x96\xb8\xb4\xa8\x92\xa8\xb8\x98\x9a\xa8\x9a\xb8\x9a") == 0) {
                sInput = pszInput;
                sInput = sInput.Right(sInput.GetLength() - nColon - 1);
                sInput.MakeUpper();

                int nParen = sInput.Find('(');
                if (nParen < 1) {
                    m_nCommand = -1;
                    CommandNotFound();
                }

                GetCommandID(sInput.Left(nParen));
                ParseArguments(sInput.Right(sInput.GetLength() - nParen - 1));

                if (m_nCommand < 0) {
                    return;
                }

                switch (m_nCommand) {
                case 0xC: // FIRSTAID
                    m_cheat2.AddTestItems();
                    break;
                case 0xD: // HANS
                    m_cheat2.JumpPartyToPointer();
                    break;
                case 0xE: // MIDAS
                    m_cheat2.AddGold500();
                    break;
                case 0x10: // ADDGOLD
                    if (CountArgs() - 1 == 0) {
                        m_cheat.AddGold(m_aNumberArgs.GetAt(0));
                    } else {
                        m_nCommand = -2;
                        CommandNotFound();
                    }
                    break;
                case 0x11: // ADDSPELL
                    switch (CountArgs()) {
                    case 1:
                        m_cheat.AddSpell(m_aStringArgs[0], 10);
                        break;
                    case 2:
                        m_cheat.AddSpell(m_aStringArgs[0], m_aNumberArgs.GetAt(0));
                        break;
                    default:
                        m_nCommand = -2;
                        CommandNotFound();
                        break;
                    }
                    break;
                case 0x12: // CHECKVAR
                    if (CountArgs() - 2 == 0) {
                        m_cheat.CheckVar(m_aStringArgs[0], m_aStringArgs[1]);
                    } else {
                        m_nCommand = -2;
                        CommandNotFound();
                    }
                    break;
                case 0x15: // CREATEITEM
                    switch (CountArgs() - 1) {
                    case 0:
                        m_cheat.AddItem(m_aStringArgs[0], 1, 0, 0);
                        break;
                    case 1:
                        m_cheat.AddItem(m_aStringArgs[0],
                            static_cast<WORD>(m_aNumberArgs[0]),
                            0,
                            0);
                        break;
                    case 2:
                        m_cheat.AddItem(m_aStringArgs[0],
                            static_cast<WORD>(m_aNumberArgs[0]),
                            static_cast<WORD>(m_aNumberArgs[1]),
                            0);
                        break;
                    case 3:
                        m_cheat.AddItem(m_aStringArgs[0],
                            static_cast<WORD>(m_aNumberArgs[0]),
                            static_cast<WORD>(m_aNumberArgs[1]),
                            static_cast<WORD>(m_aNumberArgs.GetAt(2)));
                        break;
                    default:
                        m_nCommand = -2;
                        CommandNotFound();
                        break;
                    }
                    break;
                case 0x18: // ENABLECHEATKEYS
                    switch (CountArgs()) {
                    case 0:
                        m_cheat.EnableCheatKeys(1);
                        break;
                    case 1:
                        m_cheat.EnableCheatKeys(m_aNumberArgs.GetAt(0));
                        break;
                    }
                    break;
                case 0x19: // EXPLOREAREA
                    m_cheat.SetAreaExplored();
                    break;
                case 0x1A: // MOVETOAREA
                    if (CountArgs() - 1 == 0) {
                        m_cheat.MoveToArea(m_aStringArgs[0]);
                    } else {
                        m_nCommand = -2;
                        CommandNotFound();
                    }
                    break;
                case 0x1C: // SETCURRENTXP
                    if (CountArgs() - 1 == 0) {
                        m_cheat.SetCurrentXP(m_aNumberArgs.GetAt(0));
                    } else {
                        m_nCommand = -2;
                        CommandNotFound();
                    }
                    break;
                case 0x1D: // SETGLOBAL
                    if (CountArgs() == 3) {
                        m_cheat.SetVariable(m_aStringArgs[0],
                            m_aStringArgs[1],
                            m_aNumberArgs.GetAt(0));
                    } else {
                        m_nCommand = -2;
                        CommandNotFound();
                    }
                    break;
                case 0x25: // SETCHAPTER
                    switch (CountArgs()) {
                    case 0:
                        m_cheat.AddProtagonistAction161(-1);
                        break;
                    case 1:
                        m_cheat.AddProtagonistAction161(m_aNumberArgs.GetAt(0));
                        break;
                    default:
                        m_nCommand = -2;
                        CommandNotFound();
                        break;
                    }
                    break;
                case 0x26: // JEFFKATTACKS
                    m_cheat.JeffKAttacks();
                    break;
                case 0x27: // JEFFKDEFENDS
                    m_cheat.JeffKDefends();
                    break;
                default:
                    m_nCommand = -3;
                    CommandNotFound();
                    break;
                }

                return;
            }
        }

        m_cheat.DisplayText("Nice try cheater!");
    }
}

// 0x583E60
void CLUAConsole::GetCommandID(CString sCommand)
{
    m_sCommand = sCommand;
    sCommand = Encode(sCommand);

    // NOTE: The command names are stored as Encode() output; the plain name
    // is in the comment.

    // FIRSTAID
    if (strcmp(sCommand, "\x9c\xa2\xb4\xb6\xb8\x92\xa2\x98") == 0) {
        m_nCommand = 0xC;
        // HANS
    } else if (strcmp(sCommand, "\xa0\x92\xac\xb6") == 0) {
        m_nCommand = 0xD;
        // MIDAS
    } else if (strcmp(sCommand, "\xaa\xa2\x98\x92\xb6") == 0) {
        m_nCommand = 0xE;
        // ADDGOLD
    } else if (strcmp(sCommand, "\x92\x98\x98\x9e\xae\xa8\x98") == 0) {
        m_nCommand = 0x10;
        // ADDSPELL
    } else if (strcmp(sCommand, "\x92\x98\x98\xb6\xb0\x9a\xa8\xa8") == 0) {
        m_nCommand = 0x11;
        // CHECKVAR
    } else if (strcmp(sCommand, "\x96\xa0\x9a\x96\xa6\xbc\x92\xb4") == 0) {
        m_nCommand = 0x12;
        // CREATEITEM
    } else if (strcmp(sCommand, "\x96\xb4\x9a\x92\xb8\x9a\xa2\xb8\x9a\xaa") == 0) {
        m_nCommand = 0x15;
        // ENABLECHEATKEYS
    } else if (strcmp(sCommand, "\x9a\xac\x92\x94\xa8\x9a\x96\xa0\x9a\x92\xb8\xa6\x9a\xc2\xb6") == 0) {
        m_nCommand = 0x18;
        // EXPLOREAREA
    } else if (strcmp(sCommand, "\x9a\xc0\xb0\xa8\xae\xb4\x9a\x92\xb4\x9a\x92") == 0) {
        m_nCommand = 0x19;
        // MOVETOAREA
    } else if (strcmp(sCommand, "\xaa\xae\xbc\x9a\xb8\xae\x92\xb4\x9a\x92") == 0) {
        m_nCommand = 0x1A;
        // SETCURRENTXP
    } else if (strcmp(sCommand, "\xb6\x9a\xb8\x96\xba\xb4\xb4\x9a\xac\xb8\xc0\xb0") == 0) {
        m_nCommand = 0x1C;
        // SETGLOBAL
    } else if (strcmp(sCommand, "\xb6\x9a\xb8\x9e\xa8\xae\x94\x92\xa8") == 0) {
        m_nCommand = 0x1D;
        // SETCHAPTER
    } else if (strcmp(sCommand, "\xb6\x9a\xb8\x96\xa0\x92\xb0\xb8\x9a\xb4") == 0) {
        m_nCommand = 0x25;
        // JEFFKATTACKS
    } else if (strcmp(sCommand, "\xa4\x9a\x9c\x9c\xa6\x92\xb8\xb8\x92\x96\xa6\xb6") == 0) {
        m_nCommand = 0x26;
        // JEFFKDEFENDS
    } else if (strcmp(sCommand, "\xa4\x9a\x9c\x9c\xa6\x98\x9a\x9c\x9a\xac\x98\xb6") == 0) {
        m_nCommand = 0x27;
    } else {
        m_nCommand = -1;
        CommandNotFound();
    }
}

// 0x5840E0
void CLUAConsole::ParseArguments(CString sArguments)
{
    CString sToken;

    sArguments.TrimRight();
    sArguments.TrimLeft();

    // Look for the closing parenthesis within the first 128 characters.
    LPCSTR pszArguments = sArguments;
    int nIndex = 0;
    while (pszArguments[nIndex] != ')') {
        if (nIndex == 0x80 || nIndex == sArguments.GetLength() - 1) {
            m_nCommand = -2;
            CommandNotFound();
            return;
        }
        nIndex++;
    }

    while (sArguments.GetLength() != 0) {
        sToken = sArguments;

        int nSplit = sArguments.FindOneOf(",)");
        if (nSplit >= 0) {
            sToken = sToken.Left(nSplit);
            sArguments = sArguments.Right(sArguments.GetLength() - nSplit - 1);
        }

        sToken.TrimRight();
        sArguments.TrimLeft();

        if (sToken.GetLength() == 0) {
            continue;
        }

        char c = sToken[0];
        if (c == '"' && sToken.GetLength() >= 2) {
            if (sToken[sToken.GetLength() - 1] != '"') {
                m_nCommand = -2;
                CommandNotFound();
                return;
            }

            AddStringArgument(sToken.Mid(1, sToken.GetLength() - 2));
        } else {
            // NOTE: Any other token that is not a number ends the parsing
            // without an error.
            if (!isdigit(c) && sToken[0] != '-') {
                return;
            }

            m_aNumberArgs.SetAtGrow(m_aNumberArgs.GetSize(), atol(sToken));
            m_nNumberArgs++;
        }
    }
}

// 0x5842C0
void CLUAConsole::AddStringArgument(CString sArgument)
{
    m_aStringArgs.SetAtGrow(m_aStringArgs.GetSize(), sArgument);
    m_nStringArgs++;
}

// 0x584320
void CLUAConsole::CommandNotFound()
{
    CString sMessage;

    if (m_nCommand == -3) {
        m_cheat.DisplayText("I don't know how you go here, try again!");
    } else if (m_nCommand == -2) {
        m_cheat.DisplayText("Wrong Parameters : Check the parameter list.  Strings must be in quotes.  Make sure to enclose parameters in () and seperate parameters with a comma.");
    } else if (m_nCommand == -1) {
        sMessage = "Command not found : ";
        sMessage += GetCommandText();
        m_cheat.DisplayText(sMessage);
    }

    m_nCommand = -1;
}

// 0x584400
CString CLUAConsole::GetCommandText()
{
    return m_sCommand;
}

// 0x584420
int CLUAConsole::CountArgs()
{
    switch (m_nCommand) {
    case 0xC: // FIRSTAID
    case 0xD: // HANS
    case 0xE: // MIDAS
    case 0x19: // EXPLOREAREA
    case 0x26: // JEFFKATTACKS
    case 0x27: // JEFFKDEFENDS
        return 0;
    case 0x10: // ADDGOLD
    case 0x18: // ENABLECHEATKEYS
    case 0x1C: // SETCURRENTXP
        if (m_nNumberArgs == 1) {
            return 1;
        }
        return 0;
    case 0x11: // ADDSPELL
    case 0x15: // CREATEITEM
        if (m_nStringArgs == 1) {
            return m_nNumberArgs + 1;
        }
        return 0;
    case 0x12: // CHECKVAR
        if (m_nStringArgs == 2) {
            return 2;
        }
        return 0;
    case 0x1A: // MOVETOAREA
        if (m_nStringArgs == 1) {
            return 1;
        }
        return 0;
    case 0x1D: // SETGLOBAL
        if (m_nStringArgs == 2 && m_nNumberArgs == 1) {
            return 3;
        }
        return 0;
    case 0x25: // SETCHAPTER
        return m_nNumberArgs;
    default:
        m_nCommand = -3;
        CommandNotFound();
        return 0;
    }
}

// 0x5844E0
CString CLUAConsole::Encode(CString sText)
{
    for (int i = 0; i < sText.GetLength(); i++) {
        sText.SetAt(i, static_cast<char>((sText[i] + 8) << 1));
    }

    return sText;
}
