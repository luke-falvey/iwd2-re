#include "CInfGame.h"

#include <process.h>

#include "CAIScript.h"
#include "CBaldurChitin.h"
#include "CBaldurEngine.h"
#include "CBaldurProjector.h"
#include "CCharacterFile.h"
#include "CCreatureFile.h"
#include "CGameAnimationType.h"
#include "CGameArea.h"
#include "CGameContainer.h"
#include "CGameEffect.h"
#include "CGameObject.h"
#include "CGameSprite.h"
#include "CGameTrigger.h"
#include "CInfCursor.h"
#include "CItem.h"
#include "CPathSearch.h"
#include "CResCache.h"
#include "CResFile.h"
#include "CScreenChapter.h"
#include "CScreenCharacter.h"
#include "CScreenCreateChar.h"
#include "CScreenInventory.h"
#include "CScreenJournal.h"
#include "CScreenLoad.h"
#include "CScreenMap.h"
#include "CScreenMultiPlayer.h"
#include "CScreenOptions.h"
#include "CScreenSave.h"
#include "CScreenSinglePlayer.h"
#include "CScreenSpellbook.h"
#include "CScreenStore.h"
#include "CScreenWorld.h"
#include "CScreenWorldMap.h"
#include "CSearchBitmap.h"
#include "CSpell.h"
#include "CStore.h"
#include "CUIControlBase.h"
#include "CUIManager.h"
#include "CUIPanel.h"
#include "CUtil.h"
#include "Icewind586B70.h"
#include "IcewindMisc.h"

#define FIFTY_THREE 53

// 0x851884
const SHORT CInfGame::KILL_INNOCENT = 0;

// 0x851886
const SHORT CInfGame::INJURE_INNOCENT = 1;

// 0x851888
const SHORT CInfGame::STEAL = 2;

// 0x85188A
const SHORT CInfGame::KILL_FIST = 3;

// 0x85188C
const SHORT CInfGame::MONEY_CHURCH = 4;

// 0x85188E
const SHORT CInfGame::MONEY_BEGGAR = 5;

// 0x851890
const SHORT CInfGame::MAGIC_CHURCH = 6;

// 0x851892
const SHORT CInfGame::KILL_EVIL = 7;

// 0x851894
const SHORT CInfGame::BASE_CHURCH = 8;

// 0x851896
const SHORT CInfGame::REPUTATION_MULTIPLIER = 10;

// 0x8518AC
const DWORD CInfGame::PROGRESSBAR_AREA_ADDITIONAL = 3000000;

// 0x8518B0
const DWORD CInfGame::PROGRESSBAR_GAME_ADDITIONAL = 5000000;

// 0x8518B4
const DWORD CInfGame::PROGRESSBAR_REQUESTS_ADDITIONAL = 5000000;

// 0x8518B8
const DWORD CInfGame::PROGRESSBAR_CACHING_ADDITIONAL = 3000000;

// NOTE: Travel screen image index that shows a parchment instead of a picture.
//
// 0x8518BC
const BYTE CInfGame::byte_8518BC = 5;

// 0x8518BE
const SHORT CInfGame::FEEDBACK_GOLD = 1;

// 0x8518C0
const SHORT CInfGame::FEEDBACK_EXPERIENCE = 2;

// 0x8518C2
const SHORT CInfGame::FEEDBACK_ITEMGAINED = 3;

// 0x8518C4
const SHORT CInfGame::FEEDBACK_ITEMLOST = 4;

// 0x8518C6
const SHORT CInfGame::FEEDBACK_REPUTATION = 5;

// 0x8518C8
const SHORT CInfGame::FEEDBACK_TIMEPASSEDJOURNEY = 6;

// 0x8518CA
const SHORT CInfGame::FEEDBACK_TIMEPASSEDREST = 7;

// 0x8518CC
const SHORT CInfGame::FEEDBACK_INVENTORYFULL_ITEMDROPPED = 8;

// 0x8518CE
const SHORT CInfGame::FEEDBACK_JOURNAL_UPDATE = 9;

// 0x8518D0
const SHORT CInfGame::FEEDBACK_WORLDMAP_UPDATE = 10;

// 0x8AFC50
static const char* off_8AFC50[FIFTY_THREE] = {
    "Inventory",
    "Character Record",
    "Return to Game",
    "Journal",
    "Map",
    "Spellbook",
    "Options",
    "Character Arbitration",
    "Guard / Protect",
    "Turn Undead",
    "Battle Song",
    "Cast Spell",
    "Use Item",
    "Dialog",
    "Thieving",
    "Stealth",
    "Trap Detection",
    "Group Stop",
    "Group Attack",
    "Special Abilities",
    "Equip Melee Weapon",
    "Equip Range Weapon",
    "Quick Load",
    "Hide Interface",
    "Chat Window",
    "Location",
    "Quick Save",
    "Toggle AI",
    "Toggle Interface",
    "Rest",
    "Toggle Portrait HP",
    "Formation",
    "Multiselect 1-2",
    "Multiselect 3-4",
    "Multiselect 5-6",
    "Multiselect 1-3",
    "Multiselect 4-6",
    "Multiselect All",
    "Quick Spell Slot1",
    "Quick Spell Slot2",
    "Quick Spell Slot3",
    "Quick Weapon Slot1",
    "Quick Weapon Slot2",
    "Quick Weapon Slot3",
    "Quick Weapon Slot4",
    "Quick Item Slot1",
    "Quick Item Slot2",
    "Quick Item Slot3",
    "Quick Formation Slot1",
    "Quick Formation Slot2",
    "Quick Formation Slot3",
    "Quick Formation Slot4",
    "Quick Formation Slot5",
};

// 0x8AFD24
static const char* off_8AFD24[FIFTY_THREE] = {
    "I",
    "R",
    "G",
    "J",
    "M",
    "S",
    "O",
    "C",
    "",
    "",
    "",
    "",
    "",
    "D",
    "",
    "",
    "",
    "",
    "",
    "V",
    "",
    "",
    "L",
    "H",
    "T",
    "X",
    "Q",
    "A",
    "Y",
    "Z",
    ".",
    "F",
    "7",
    "8",
    "9",
    "0",
    "-",
    "=",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

// 0x8B4F84
const char* off_8B4F84[] = {
    "SPPR101",
    "SPPR102",
    "SPPR103",
    "SPPR105",
    "SPPR106",
    "SPPR107",
    "SPPR108",
    "SPPR109",
    "SPPR110",
    "SPPR111",
    "SPPR112",
    "SPPR113",
    "SPPR114",
    "SPPR115",
    "SPPR116",
    "SPPR117",
    "SPPR201",
    "SPPR202",
    "SPPR203",
    "SPPR204",
    "SPPR205",
    "SPPR206",
    "SPPR207",
    "SPPR208",
    "SPPR210",
    "SPPR211",
    "SPPR212",
    "SPPR213",
    "SPPR214",
    "SPPR215",
    "SPPR216",
    "SPPR217",
    "SPPR218",
    "SPPR219",
    "SPPR220",
    "SPPR221",
    "SPPR222",
    "SPPR301",
    "SPPR302",
    "SPPR303",
    "SPPR304",
    "SPPR305",
    "SPPR306",
    "SPPR307",
    "SPPR308",
    "SPPR309",
    "SPPR310",
    "SPPR311",
    "SPPR312",
    "SPPR313",
    "SPPR314",
    "SPPR315",
    "SPPR316",
    "SPPR318",
    "SPPR319",
    "SPPR320",
    "SPPR322",
    "SPPR323",
    "SPPR324",
    "SPPR325",
    "SPPR326",
    "SPPR327",
    "SPPR328",
    "SPPR401",
    "SPPR402",
    "SPPR403",
    "SPPR404",
    "SPPR405",
    "SPPR406",
    "SPPR407",
    "SPPR408",
    "SPPR409",
    "SPPR410",
    "SPPR411",
    "SPPR414",
    "SPPR415",
    "SPPR416",
    "SPPR417",
    "SPPR418",
    "SPPR420",
    "SPPR422",
    "SPPR423",
    "SPPR424",
    "SPPR425",
    "SPPR426",
    "SPPR427",
    "SPPR428",
    "SPPR429",
    "SPPR501",
    "SPPR502",
    "SPPR503",
    "SPPR504",
    "SPPR507",
    "SPPR508",
    "SPPR510",
    "SPPR511",
    "SPPR512",
    "SPPR513",
    "SPPR514",
    "SPPR515",
    "SPPR516",
    "SPPR517",
    "SPPR518",
    "SPPR519",
    "SPPR520",
    "SPPR521",
    "SPPR522",
    "SPPR523",
    "SPPR524",
    "SPPR602",
    "SPPR605",
    "SPPR606",
    "SPPR607",
    "SPPR608",
    "SPPR609",
    "SPPR610",
    "SPPR611",
    "SPPR612",
    "SPPR613",
    "SPPR614",
    "SPPR615",
    "SPPR616",
    "SPPR702",
    "SPPR704",
    "SPPR705",
    "SPPR707",
    "SPPR709",
    "SPPR712",
    "SPPR714",
    "SPPR715",
    "SPPR716",
    "SPPR717",
    "SPPR718",
    "SPPR719",
    "SPPR720",
    "SPPR721",
    "SPPR722",
    "SPPR723",
    "SPPR724",
    "SPPR725",
    "SPPR726",
    "SPPR727",
    "SPPR728",
    "SPPR729",
    "SPWI101",
    "SPWI102",
    "SPWI103",
    "SPWI104",
    "SPWI105",
    "SPWI107",
    "SPWI108",
    "SPWI112",
    "SPWI113",
    "SPWI114",
    "SPWI115",
    "SPWI116",
    "SPWI117",
    "SPWI118",
    "SPWI119",
    "SPWI120",
    "SPWI121",
    "SPWI122",
    "SPWI201",
    "SPWI203",
    "SPWI205",
    "SPWI206",
    "SPWI207",
    "SPWI209",
    "SPWI210",
    "SPWI211",
    "SPWI212",
    "SPWI213",
    "SPWI214",
    "SPWI215",
    "SPWI217",
    "SPWI218",
    "SPWI219",
    "SPWI220",
    "SPWI221",
    "SPWI222",
    "SPWI223",
    "SPWI224",
    "SPWI225",
    "SPWI226",
    "SPWI227",
    "SPWI228",
    "SPWI302",
    "SPWI303",
    "SPWI304",
    "SPWI305",
    "SPWI306",
    "SPWI308",
    "SPWI309",
    "SPWI310",
    "SPWI311",
    "SPWI312",
    "SPWI313",
    "SPWI314",
    "SPWI316",
    "SPWI317",
    "SPWI318",
    "SPWI319",
    "SPWI320",
    "SPWI321",
    "SPWI322",
    "SPWI323",
    "SPWI401",
    "SPWI402",
    "SPWI404",
    "SPWI405",
    "SPWI406",
    "SPWI407",
    "SPWI408",
    "SPWI410",
    "SPWI411",
    "SPWI412",
    "SPWI413",
    "SPWI414",
    "SPWI417",
    "SPWI418",
    "SPWI419",
    "SPWI420",
    "SPWI421",
    "SPWI422",
    "SPWI423",
    "SPWI424",
    "SPWI425",
    "SPWI426",
    "SPWI427",
    "SPWI428",
    "SPWI501",
    "SPWI503",
    "SPWI504",
    "SPWI505",
    "SPWI507",
    "SPWI508",
    "SPWI509",
    "SPWI510",
    "SPWI511",
    "SPWI512",
    "SPWI513",
    "SPWI514",
    "SPWI515",
    "SPWI516",
    "SPWI517",
    "SPWI518",
    "SPWI519",
    "SPWI520",
    "SPWI521",
    "SPWI522",
    "SPWI523",
    "SPWI524",
    "SPWI525",
    "SPWI526",
    "SPWI601",
    "SPWI603",
    "SPWI605",
    "SPWI606",
    "SPWI607",
    "SPWI608",
    "SPWI609",
    "SPWI610",
    "SPWI611",
    "SPWI612",
    "SPWI613",
    "SPWI614",
    "SPWI615",
    "SPWI616",
    "SPWI617",
    "SPWI618",
    "SPWI619",
    "SPWI620",
    "SPWI621",
    "SPWI622",
    "SPWI623",
    "SPWI624",
    "SPWI626",
    "SPWI627",
    "SPWI628",
    "SPWI629",
    "SPWI630",
    "SPWI702",
    "SPWI703",
    "SPWI704",
    "SPWI705",
    "SPWI706",
    "SPWI707",
    "SPWI708",
    "SPWI709",
    "SPWI710",
    "SPWI711",
    "SPWI712",
    "SPWI713",
    "SPWI714",
    "SPWI715",
    "SPWI716",
    "SPWI717",
    "SPWI718",
    "SPWI719",
    "SPWI802",
    "SPWI803",
    "SPWI804",
    "SPWI805",
    "SPWI806",
    "SPWI807",
    "SPWI808",
    "SPWI809",
    "SPWI810",
    "SPWI902",
    "SPWI903",
    "SPWI905",
    "SPWI906",
    "SPWI907",
    "SPWI908",
    "SPWI909",
    "SPWI910",
    "SPWI911",
    NULL,
    NULL,
};

// 0x8ABA24
static char byte_8ABA24[CINFGAME_SPECIAL_SYMBOLS] = {
    ';',
    '=',
    ',',
    '-',
    '.',
    '/',
    '`',
    '[',
    '\\',
    ']',
    '\'',
    '\0',
};

// 0x8ABA3C
static BYTE byte_8ABA3C[CINFGAME_SPECIAL_SYMBOLS] = {
    VK_OEM_1,
    VK_OEM_PLUS,
    VK_OEM_COMMA,
    VK_OEM_MINUS,
    VK_OEM_PERIOD,
    VK_OEM_2,
    VK_OEM_3,
    VK_OEM_4,
    VK_OEM_5,
    VK_OEM_6,
    VK_OEM_7,
    0,
};

// 0x8E707C
const CString CInfGame::FOGOWAR_RESREF("FOGOWAR");

// 0x8E74AC
const CString CInfGame::SAVE_OBJECT_LIST_NAME("SVTRIOBJ");

// 0x8E74F4
const CString CInfGame::CHAPTER_GLOBAL("CHAPTER");

// 0x8E74E8
const CString CInfGame::TOKEN_CURRENT_CHAPTER("CurrentChapter");

// 0x8E7510
const CString CInfGame::TOKEN_CHARACTER("CHARNAME");

// 0x8E7498
const CString CInfGame::TOKEN_GABBER("GABBER");

// 0x8E7064
const CString CInfGame::TOKEN_SIRMAAM("PROTAGONIST_SIRMAAM");

// 0x8E7084
const CString CInfGame::TOKEN_GIRLBOY("PROTAGONIST_GIRLBOY");

// 0x8E74FC
const CString CInfGame::TOKEN_BROTHERSISTER("PROTAGONIST_BROTHERSISTER");

// 0x8E7060
const CString CInfGame::TOKEN_LADYLORD("PROTAGONIST_LADYLORD");

// 0x8E7520
const CString CInfGame::TOKEN_MALEFEMALE("PROTAGONIST_MALEFEMALE");

// 0x8E7518
const CString CInfGame::TOKEN_HESHE("PROTAGONIST_HESHE");

// 0x8E7504
const CString CInfGame::TOKEN_HISHER("PROTAGONIST_HISHER");

// 0x8E74DC
const CString CInfGame::TOKEN_RACE("PROTAGONIST_RACE");

// 0x8E74D0
const CString CInfGame::TOKEN_HIMHER("PROTAGONIST_HIMHER");

// 0x8E7074
const CString CInfGame::TOKEN_MANWOMAN("PROTAGONIST_MANWOMAN");

// 0x8E74C4
const CString CInfGame::TOKEN_GABBER_SIRMAAM("SIRMAAM");

// 0x8E7070
const CString CInfGame::TOKEN_GABBER_GIRLBOY("GIRLBOY");

// 0x8E74E4
const CString CInfGame::TOKEN_GABBER_BROTHERSISTER("BROTHERSISTER");

// 0x8E7088
const CString CInfGame::TOKEN_GABBER_LADYLORD("LADYLORD");

// 0x8E7080
const CString CInfGame::TOKEN_GABBER_MALEFEMALE("MALEFEMALE");

// 0x8E751C
const CString CInfGame::TOKEN_GABBER_HESHE("HESHE");

// 0x8E74EC
const CString CInfGame::TOKEN_GABBER_HISHER("HISHER");

// 0x8E708C
const CString CInfGame::TOKEN_GABBER_RACE("RACE");

// 0x8E74C8
const CString CInfGame::TOKEN_GABBER_HIMHER("HIMHER");

// 0x8E74F0
const CString CInfGame::TOKEN_GABBER_MANWOMAN("MANWOMAN");

// 0x8E7494
const CString CInfGame::TOKEN_PLAYER1("PLAYER1");

// 0x8E74BC
const CString CInfGame::TOKEN_PLAYER2("PLAYER2");

// 0x8E7490
const CString CInfGame::TOKEN_PLAYER3("PLAYER3");

// 0x8E74A4
const CString CInfGame::TOKEN_PLAYER4("PLAYER4");

// 0x8E74C0
const CString CInfGame::TOKEN_PLAYER5("PLAYER5");

// 0x8E749C
const CString CInfGame::TOKEN_PLAYER6("PLAYER6");

// 0x8E74D8
const CString CInfGame::TOKEN_SURFACE_UNDERDARK("SURFACE_UNDERDARK");

// 0x8E74D4
const CString CInfGame::AUTO_SAVE_NAME("000000000-Auto-Save");

// 0x8E7508
const CString CInfGame::QUICK_SAVE_NAME("000000002-Quick-Save");

// 0x8E750C
const CString CInfGame::QUICK_SAVE_BACKUP_NAME("000000001-Quick-Save-Backup");

// 0x8E706C
const CString CInfGame::SILHOUETTE_PORTRAIT_SM("noportsm");

// 0x8E705C
const CString CInfGame::SILHOUETTE_PORTRAIT_LG("noportlg");

// 0x8E7524
BOOL CInfGame::m_bHealPartyOnRest;

// 0x8E7528
BOOLEAN CInfGame::byte_8E7528;

// 0x8E752C
int CInfGame::dword_8E752C;

// 0x8E52EC
int CInfGame::dword_8E52EC;

// 0x8E52F0
int CInfGame::dword_8E52F0;

// 0x59CC30
CInfGame::CInfGame()
    : m_rgbMasterBitmap(CResRef("MPALETTE"), FALSE, 24)
{
    // 0x59CDB2
    field_38A6 = 0;

    m_nUniqueAreaID = 0;
    m_nReputation = 0;
    m_nProtagonistId = 0;
    m_bPartyAI = FALSE;
    m_bShowAreaNotes = FALSE;
    m_dwLastProgressRenderTickCount = 0;
    m_dwLastProgressMsgTickCount = 0;
    m_nLastSaveTime = 0;
    m_lastTarget = 0;
    field_4AB2 = 0;
    m_nCharacterTerminationSequenceDelay = 0;
    m_nAIIndex = 0;
    m_nTimeStop = 0;
    m_nTimeStopCaster = 0;
    memset(m_appearanceColorsWeapon, 0, sizeof(m_appearanceColorsWeapon));
    memset(m_appearanceColorsBody, 0, sizeof(m_appearanceColorsBody));
    memset(m_appearanceColorsShield, 0, sizeof(m_appearanceColorsShield));
    memset(m_appearanceColorsHelmet, 0, sizeof(m_appearanceColorsHelmet));
    m_nDifficultyLevel = 0;
    m_nCharacters = 0;
    for (INT nIndex = 0; nIndex < CINFGAME_MAXCHARACTERS; nIndex++) {
        m_characters[nIndex] = CGameObjectArray::INVALID_INDEX;
        m_characterPortraits[nIndex] = CGameObjectArray::INVALID_INDEX;
    }

    m_nState = 0;
    m_tempCursor = 4;
    m_iconIndex = -1;
    m_bGameLoaded = FALSE;
    m_bInLoadGame = FALSE;
    m_bInLoadArea = FALSE;
    m_bFromNewGame = FALSE;
    m_bInDestroyGame = FALSE;
    m_bInAreaTransition = FALSE;
    m_visibleArea = 0;
    memset(m_gameAreas, 0, sizeof(m_gameAreas));
    m_pGameAreaMaster = NULL;
    m_pathSearch = NULL;
    m_listGrid = NULL;
    m_searchRequestListEmpty = TRUE;
    m_iContainer = CGameObjectArray::INVALID_INDEX;
    m_iContainerSprite = CGameObjectArray::INVALID_INDEX;
    m_bForceDither = FALSE;
    m_sSaveDir = ".\\save\\";
    m_sMultiplayerSaveDir = ".\\mpsave\\";
    m_sTempDir = ".\\temp\\";
    m_sTempSaveDir = ".\\tempsave\\";
    m_sScriptsDir = ".\\scripts\\";
    m_sPortraitsDir = ".\\portraits\\";
    m_sSoundsDir = ".\\sounds\\";
    m_sCharactersDir = ".\\characters\\";

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 2608
        UTIL_ASSERT_MSG(FALSE, "Problems removing files from Temp directory, delete the directory and try again");
    }

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 2610
        UTIL_ASSERT_MSG(FALSE, "Problems removing files from TempSave directory, delete the directory and try again");
    }

    g_pChitin->cDimm.AddToDirectoryList(m_sScriptsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sSoundsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sPortraitsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sCharactersDir, TRUE);

    m_gameSave.m_curFormation = TRUE;
    m_gameSave.field_1AC = FALSE;

    // 0x8AFC3C
    static const DWORD s_quickFormations[5] = { 6, 8, 1, 9, 5 };
    for (int nFormation = 0; nFormation < 5; nFormation++) {
        m_gameSave.m_quickFormations[nFormation] = static_cast<SHORT>(s_quickFormations[nFormation]);
    }

    field_4A42 = 48;
    field_4A44 = 48;
    m_gameSave.m_mode = -1;
    m_gameSave.m_cutScene = FALSE;
    m_currArmor = '1';
    field_4A46 = -1;
    m_currAnimation = -1;
    m_vcLocator.SetResRef(CResRef("locater"), FALSE, TRUE, TRUE);
    memset(m_defaultFamiliarResRefs, 0, sizeof(m_defaultFamiliarResRefs));
    field_4204 = 0;
    m_nCharacterOverflowCount = 0;
    m_saveObjectList.LoadList(CResRef(SAVE_OBJECT_LIST_NAME), FALSE);
    m_listGrid = new CPathNode*[CPathSearch::GRID_ACTUALX * CPathSearch::GRID_ACTUALY];
    m_pathSearch = new CPathSearch(m_listGrid);
    m_searchShutdown = FALSE;
    m_hSearchThread = CreateSemaphoreA(NULL, 0, 1, NULL);
    if (m_hSearchThread != NULL) {
        m_nTravelScreenImageToUse = 0;
        m_bPlayerScriptStyle = FALSE;
        field_4B38 = 0;
        field_366E = 0;
        m_nAreaFirstObject = 0;

        m_INISounds.SetFileName(CString("sounds"));
        if (m_INISounds.Load(CString("sounds")) == 0) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 2671
            UTIL_ASSERT_MSG(FALSE, "Problem loading the sounds.ini file!");
        }

        memset(m_aServerStore, 0, sizeof(m_aServerStore));
        memset(m_nServerStoreDemands, 0, sizeof(m_nServerStoreDemands));

        field_50D8 = FALSE;
        field_50DC = 0;

        CString sFileName;
        g_pChitin->cDimm.WriteSetUp(m_sTempDir + "foo.bar", sFileName);
        g_pChitin->cDimm.WriteSetUp(m_sTempSaveDir + "foo.bar", sFileName);

        field_4BD4 = 0;
        m_bExpansion = FALSE;
        field_4BD6 = FALSE;

        C2DArray tSpells;
        tSpells.Load(CResRef(CRuleTables::LISTSPLL));
        m_spells.Load(tSpells, 7);

        for (unsigned int spellcasterClassIndex = 0; spellcasterClassIndex < 7; spellcasterClassIndex++) {
            m_spellsByClass[spellcasterClassIndex].Load(tSpells,
                spellcasterClassIndex,
                7,
                NULL);
        }

        C2DArray tDomainSpells;
        tDomainSpells.Load(CResRef(CRuleTables::LISTDOMN));

        for (unsigned int domainIndex = 0; domainIndex < 9; domainIndex++) {
            m_spellsByDomain[domainIndex].Load(tDomainSpells,
                domainIndex,
                9,
                &m_spells);
        }

        C2DArray tInnateSpells;
        tInnateSpells.Load(CResRef(CRuleTables::LISTINNT));
        m_innateSpells.Load(tInnateSpells, 0);

        C2DArray tSongs;
        tSongs.Load(CResRef(CRuleTables::LISTSONG));
        m_songs.Load(tSongs, 0);

        C2DArray tShapeshifts;
        tShapeshifts.Load(CResRef(CRuleTables::LISTSHAP));
        m_shapeshifts.Load(tShapeshifts, 0);

        // NOTE: The following validation code looks incomplete. Logging stuff
        // was likely compiled out via debug/release switches.
        CString sError(" ");
        UINT nIndex;

        for (nIndex = 0; nIndex < m_spells.m_nCount; nIndex++) {
            CSpell cSpell(m_spells.Get(nIndex));

            sError.Format("Unable to load Class Spell file: %s\n",
                (LPCSTR)m_spells.Get(nIndex).GetResRefStr());

            if (cSpell.Demand()) {
                sError.Format("Invalid Class Spell level: %s - %d\n",
                    (LPCSTR)m_spells.Get(nIndex).GetResRefStr(),
                    cSpell.GetLevel());
            }

            cSpell.Release();
        }

        for (nIndex = 0; nIndex < m_innateSpells.m_nCount; nIndex++) {
            CSpell cSpell(m_innateSpells.Get(nIndex));

            sError.Format("Unable to load Innate Spell file: %s\n",
                (LPCSTR)m_innateSpells.Get(nIndex).GetResRefStr());

            if (cSpell.Demand()) {
                sError.Format("Invalid Innate Spell level: %s - %d\n",
                    (LPCSTR)m_innateSpells.Get(nIndex).GetResRefStr(),
                    cSpell.GetLevel());
            }

            cSpell.Release();
        }

        for (nIndex = 0; nIndex < m_songs.m_nCount; nIndex++) {
            CSpell cSpell(m_songs.Get(nIndex));

            sError.Format("Unable to load Song file: %s\n",
                (LPCSTR)m_songs.Get(nIndex).GetResRefStr());

            if (cSpell.Demand()) {
                sError.Format("Invalid Song level: %s - %d\n",
                    (LPCSTR)m_songs.Get(nIndex).GetResRefStr(),
                    cSpell.GetLevel());
            }

            cSpell.Release();
        }

        for (nIndex = 0; nIndex < m_shapeshifts.m_nCount; nIndex++) {
            CSpell cSpell(m_shapeshifts.Get(nIndex));

            sError.Format("Unable to load Shapeshift file: %s\n",
                (LPCSTR)m_shapeshifts.Get(nIndex).GetResRefStr());

            if (cSpell.Demand()) {
                sError.Format("Invalid Shapeshift level: %s - %d\n",
                    (LPCSTR)m_shapeshifts.Get(nIndex).GetResRefStr(),
                    cSpell.GetLevel());
            }

            cSpell.Release();
        }
    }
}

// 0x59ECB0
CInfGame::~CInfGame()
{
    CSingleLock searchLock(&field_1B58, FALSE);

    if (m_bGameLoaded) {
        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pChitin->cNetwork.CloseSession(TRUE);
        }
        DestroyGame(FALSE, FALSE);
    }

    if (m_hSearchThread != NULL) {
        searchLock.Lock(INFINITE);
        m_searchShutdown = TRUE;
        ReleaseSemaphore(m_hSearchThread, 1, NULL);
        searchLock.Unlock();

        while (m_hSearchThread != NULL) {
            SleepEx(1, FALSE);
        }

        if (m_pathSearch != NULL) {
            delete m_pathSearch;
            m_pathSearch = NULL;
        }

        if (m_listGrid != NULL) {
            delete[] m_listGrid;
            m_listGrid = NULL;
        }

        m_allies.RemoveAll();
        m_familiars.RemoveAll();

        g_pChitin->cDimm.RemoveFromDirectoryList(m_sScriptsDir, FALSE);
        g_pChitin->cDimm.RemoveFromDirectoryList(m_sSoundsDir, FALSE);
        g_pChitin->cDimm.RemoveFromDirectoryList(m_sPortraitsDir, FALSE);
        g_pChitin->cDimm.RemoveFromDirectoryList(m_sCharactersDir, FALSE);

        Icewind586B70::Destroy();
    }
}

// 0x59F500
void CInfGame::StartSearchThread()
{
    if (_beginthread(SearchThreadMain, 0, NULL) == -1) {
        CloseHandle(m_hSearchThread);
        m_hSearchThread = NULL;
    }
}

// 0x59F540
void CInfGame::InitGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    m_cVRamPool.AttachSurfaces(g_pBaldurChitin->GetCurrentVideoMode());

    if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL) {
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::AREA_TRANSITION, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::CHAR_RECORDS, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::DIALOG, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::GROUP_POOL, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::LEADER, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::MODIFY_CHARS, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::PAUSING, TRUE);
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::PURCHASING, TRUE);
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        m_singlePlayerPermissions.SetSinglePermission(CGamePermission::LEADER, TRUE);
    }

    LoadKeymap();
    LoadOptions();

    g_pBaldurChitin->EnginesGameInit();

    m_cButtonArray.SetState(CInfButtonArray::STATE_NONE, 0);

    g_pBaldurChitin->GetCurrentVideoMode()->rgbGlobalTint = RGB(255, 255, 255);

    m_worldTime.m_gameTime = CTimerWorld::TIME_DAY + 1;
    m_worldTime.CheckForTriggerEventPast();

    g_pBaldurChitin->GetTlkTable().Fetch(16484, field_4814);
    g_pBaldurChitin->GetTlkTable().Fetch(15307, field_487C);

    for (BYTE nIndex = 0; nIndex < 6; nIndex++) {
        EnablePortrait(nIndex, FALSE);
    }

    m_gameSave.field_1B0 = 0;
    memset(m_gameSave.m_groupInventory, 0, sizeof(m_gameSave.m_groupInventory));
    m_gameSave.m_mode = -1;
    m_gameSave.m_cutScene = FALSE;
    m_appearanceWeapon = "";
    m_appearanceBody = "";
    m_appearanceShield = "";
    m_appearanceHelmet = "";

    m_pGameAreaMaster = NULL;
    m_bPartyAI = TRUE;
    m_bShowAreaNotes = TRUE;
    field_4AB2 = 0;
    m_bGameLoaded = TRUE;

    m_allies.RemoveAll();
    m_familiars.RemoveAll();

    memset(m_defaultFamiliarResRefs, 0, sizeof(m_defaultFamiliarResRefs));

    field_4204 = 0;

    m_cJournal.ClearAllEntries();

    m_variables.Resize(2048);
    m_namedCreatures.Resize(2048);
    m_variables.ClearAll();
    m_namedCreatures.ClearAll();

    m_entanglePalette.SetType(CVidPalette::TYPE_RANGE);
    m_entanglePalette.SetRange(0, 54, m_rgbMasterBitmap);

    m_webHoldPalette.SetType(CVidPalette::TYPE_RANGE);
    m_webHoldPalette.SetRange(0, 65, m_rgbMasterBitmap);

    sub_5A0160();

    m_nCharacterTerminationSequenceDelay = 0;

    m_cMoveList.ClearAll();
    m_cLimboList.ClearAll();

    CGameAIGame* pAIGame = new CGameAIGame();
    CAIScript* pScript = new CAIScript(CResRef("BALDUR"));

    // NOTE: Uninline.
    pAIGame->SetDefaultScript(pScript);

    m_nAIIndex = pAIGame->m_id;
    m_nTimeStop = 0;
    m_nTimeStopCaster = -1;

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 3213
        UTIL_ASSERT_MSG(FALSE, "Could not clean out Temp directory");
    }

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 3215
        UTIL_ASSERT_MSG(FALSE, "Could not clean out TempSave directory");
    }

    // TODO: Use loop.
    memset(m_aServerStore, 0, sizeof(m_aServerStore));
    memset(m_nServerStoreDemands, 0, sizeof(m_nServerStoreDemands));

    dword_8E752C = 0;

    ResetMultiPlayerPermissions();
}

// 0x59F9A0
void CInfGame::BeginListManipulation(CGameArea* pArea)
{
    if (pArea != NULL) {
        EnterCriticalSection(&(pArea->field_214));
        pArea->SetListManipulationThreadId(GetCurrentThreadId());
    }
}

// 0x59F9D0
INT CInfGame::EndListManipulation(CGameArea* pArea)
{
    INT nCounter = 0;

    if (pArea != NULL) {
        nCounter = pArea->SetListManipulationThreadId(0);
        LeaveCriticalSection(&(pArea->field_214));
    }

    return nCounter;
}

// 0x59FA00
void CInfGame::sub_59FA00(BOOL a1)
{
    CGameArea* pArea = GetVisibleArea();
    if (pArea != NULL) {
        INT nCounter = pArea->m_nListManipulationThreadCounter;
        if (pArea->m_ListManipulationThreadId == GetCurrentThreadId()) {
            while (1) {
                CGameArea* pArea = GetVisibleArea();
                if (pArea == NULL) {
                    break;
                }

                INT nRemainingCounter = pArea->SetListManipulationThreadId(0);
                LeaveCriticalSection(&(pArea->field_214));

                if (nRemainingCounter == 0) {
                    break;
                }
            }

            while (g_pChitin->m_bDisplayStale == TRUE
                || (g_pChitin->m_bInSynchronousUpdate == TRUE && a1)) {
                SleepEx(5, FALSE);
            }

            while (nCounter != 0) {
                CGameArea* pArea = GetVisibleArea();
                if (pArea != NULL) {
                    EnterCriticalSection(&(pArea->field_214));
                    pArea->SetListManipulationThreadId(GetCurrentThreadId());
                }
                nCounter--;
            }
        } else {
            while (g_pChitin->m_bInSynchronousUpdate == TRUE
                || (g_pChitin->m_bDisplayStale == TRUE && a1)) {
                SleepEx(5, FALSE);
            }
        }
    } else {
        while (g_pChitin->m_bInSynchronousUpdate == TRUE
            || (g_pChitin->m_bDisplayStale == TRUE && a1)) {
            SleepEx(5, FALSE);
        }
    }
}

// 0x59FB50
void CInfGame::DestroyGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    DWORD dwPerSegment;
    BYTE cnt;

    if (!bProgressBarInPlace) {
        if (bProgressBarRequired == TRUE) {
            sub_59FA00(TRUE);
            g_pChitin->SetProgressBar(TRUE,
                11831,
                0,
                0,
                FALSE,
                0,
                FALSE,
                0,
                FALSE,
                FALSE,
                255);
            g_pChitin->cProgressBar.m_nActionProgress = 0;
            g_pChitin->cProgressBar.m_nActionTarget = 5000000;
            g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
        }
    }

    dwPerSegment = 555555;

    CSingleLock lock(&(g_pBaldurChitin->GetObjectGame()->field_1B58), TRUE);
    m_bInDestroyGame = TRUE;

    for (cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
        if (m_gameAreas[cnt] != NULL) {
            while (m_gameAreas[cnt]->m_bInPathSearch != FALSE) {
                SleepEx(25, FALSE);
            }
        }
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    if (g_pBaldurChitin->GetScreenWorld() != NULL) {
        g_pBaldurChitin->GetScreenWorld()->m_weather.CancelCurrentWeather(NULL, 0);
    }

    g_pChitin->cDimm.RemoveFromDirectoryList(m_sTempDir, TRUE);
    m_cObjectArray.Clean();

    EnterCriticalSection(&(g_pBaldurChitin->GetScreenWorld()->field_106));
    for (cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
        if (m_gameAreas[cnt] != NULL) {
            while (m_gameAreas[cnt]->m_bInPathSearch != FALSE) {
                SleepEx(25, FALSE);
            }

            delete m_gameAreas[cnt];
            m_gameAreas[cnt] = NULL;
        }
    }
    m_visibleArea = 0;
    LeaveCriticalSection(&(g_pBaldurChitin->GetScreenWorld()->field_106));

    lock.Unlock();

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    for (cnt = 0; cnt < CINFGAME_MAXCHARACTERS; cnt++) {
        EnablePortrait(cnt, FALSE);
        if (m_characterPortraits[cnt] != -1) {
            CGameObject* pObject;
            if (m_cObjectArray.Delete(m_characterPortraits[cnt], CGameObjectArray::THREAD_ASYNCH, &pObject, -1) == CGameObjectArray::SUCCESS
                && pObject != NULL) {
                delete pObject;
            }
            m_characterPortraits[cnt] = -1;
        }
    }

    memset(m_characters, -1, sizeof(m_characters));
    m_nCharacters = 0;

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    while (!m_lstGlobalCreatures.IsEmpty()) {
        LONG iObject = static_cast<LONG>(reinterpret_cast<INT_PTR>(m_lstGlobalCreatures.RemoveHead()));
        CGameObject* pObject;
        if (m_cObjectArray.Delete(iObject, CGameObjectArray::THREAD_ASYNCH, &pObject, -1) == CGameObjectArray::SUCCESS
            && pObject != NULL) {
            delete pObject;
        }
    }

    for (BYTE nAlignment = 0; nAlignment < 9; nAlignment++) {
        for (BYTE nLevel = 0; nLevel < 9; nLevel++) {
            while (!m_familiarResRefs[nAlignment][nLevel].IsEmpty()) {
                delete m_familiarResRefs[nAlignment][nLevel].RemoveHead();
            }
        }
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    field_4A42 = 48;
    field_4A44 = 48;
    m_nState = 0;
    m_tempCursor = 4;
    m_iconIndex = -1;
    m_searchRequestListEmpty = TRUE;
    m_currArmor = '1';
    field_4A46 = -1;
    m_currAnimation = -1;
    m_bGameLoaded = FALSE;

    for (cnt = 0; cnt < 100; cnt++) {
        if (m_gameSave.m_groupInventory[cnt] != NULL) {
            delete m_gameSave.m_groupInventory[cnt];
            m_gameSave.m_groupInventory[cnt] = NULL;
        }
    }

    g_pBaldurChitin->GetTlkTable().m_override.CloseFiles();

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    m_group.m_memberList.RemoveAll();
    m_group.m_groupChanged = TRUE;
    m_allies.RemoveAll();
    m_familiars.RemoveAll();
    g_pBaldurChitin->EnginesGameUninit();

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
        // FIXME: Redundant.
        if (bProgressBarInPlace || bProgressBarRequired) {
            ProgressBarCallback(dwPerSegment, FALSE);
        }
    }

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 3592
        UTIL_ASSERT_MSG(FALSE, "Could not remove files from Temp directory");
    }

    if (!g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 3596
        UTIL_ASSERT_MSG(FALSE, "Could not remove files from TempSave directory");
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    m_bInDestroyGame = FALSE;

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(dwPerSegment, FALSE);
    }

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
        g_pChitin->m_bDisplayStale = TRUE;
        sub_59FA00(TRUE);
        g_pChitin->SetProgressBar(FALSE,
            0,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
    }

    m_cVRamPool.DetachSurfaces();

    for (cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL) {
            delete m_aServerStore[cnt];
        }
        m_aServerStore[cnt] = NULL;
    }

    POSITION pos = field_4BDC.GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        CString* node = field_4BDC.GetNext(pos);
        if (node != NULL) {
            delete node;
        }
        field_4BDC.RemoveAt(posOld);
    }
}

// 0x5A0160
void CInfGame::sub_5A0160()
{
    CFileFind fileFind;
    CString sDirName;
    CString sPattern;
    CString sResolved;
    CString sFileName;
    CString sBaseName;
    CString sExtension;
    CString sValue;

    sDirName = g_pChitin->cDimm.cResCache.m_sDirName;
    sPattern = sDirName + "data\\*.*";

    if (!g_pChitin->lAliases.ResolveFileName(sPattern, sResolved)) {
        sResolved = sPattern;
    }

    BOOL bWorking = fileFind.FindFile(sResolved);
    while (bWorking) {
        bWorking = fileFind.FindNextFile();
        sFileName = fileFind.GetFileName();

        int nSlash;
        while ((nSlash = sFileName.Find('\\')) != -1) {
            sFileName = sFileName.Right(sFileName.GetLength() - nSlash - 1);
        }

        sBaseName = sFileName.Left(sFileName.GetLength() - 4);
        sExtension = sFileName.Right(4);
        sExtension.MakeUpper();

        if (strcmp(sExtension, ".BIF") == 0) {
            CPoint ptLocation;
            if (m_ruleTables.m_tCacheValidation.Find(sBaseName, ptLocation, FALSE) == TRUE) {
                // NOTE: Uninline.
                sValue = m_ruleTables.m_tCacheValidation.GetAt(CPoint(1, ptLocation.y));
            } else {
                // NOTE: Uninline.
                sValue = m_ruleTables.m_tCacheValidation.GetDefault();
            }

            LONG nValue = atoi(sValue.GetBuffer(sValue.GetLength()));

            g_pChitin->cDimm.cResCache.ValidateFile(g_pChitin->cDimm.GetResFileID(sBaseName), nValue);
        }
    }
}

// 0x5A04B0
LONG CInfGame::CachingRequirements(const CString& areaName)
{
    CAreaFile cAreaFile;
    LONG nTotal = 0;

    // NOTE: Uninline.
    cAreaFile.SetResRef(CResRef(areaName), TRUE, TRUE);

    BYTE* pData = cAreaFile.GetData();
    if (pData == NULL) {
        return 0;
    }

    // NOTE: The WED resref is at offset 8 of the ARE file (after signature
    // and version).
    CResFile* pResFile = g_pChitin->cDimm.GetResFilePtr(g_pChitin->cDimm.GetResID(CResRef(pData + 8), 1003));
    cAreaFile.ReleaseData();

    if (pResFile == NULL) {
        return 0;
    }

    if (!g_pChitin->cNetwork.m_bConnectionEstablished) {
        return pResFile->GetCacheSize();
    }

    CString sResFileName;
    CString sXLabel;
    CString sYLabel;
    CPoint ptLocation;

    pResFile->sub_78A0F0(sResFileName);

    if ((m_ruleTables.m_tAreaLinkageCaching.Find(sResFileName, ptLocation, TRUE)
            & m_ruleTables.m_tAreaLinkageCaching.Find(sResFileName, sXLabel, sYLabel, TRUE))
        == TRUE) {
        // NOTE: Uninline.
        CString sCount = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(0, ptLocation.y));
        int nCount = atoi(sCount.GetBuffer(sCount.GetLength()));

        if (nCount != 0) {
            int index = 0;
            UINT nResFileID = g_pChitin->cDimm.GetResFileID(sYLabel);
            while (index <= nCount) {
                pResFile = g_pChitin->cDimm.GetResFilePtr(nResFileID);
                if (pResFile == NULL) {
                    return nTotal;
                }

                nTotal += pResFile->GetCacheSize();

                index++;

                // NOTE: Uninline.
                sResFileName = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(index, ptLocation.y));
                nResFileID = g_pChitin->cDimm.GetResFileID(sResFileName);
            }

            return nTotal;
        }
    }

    return pResFile->GetCacheSize();
}

// 0x5A0950
BOOL CInfGame::CacheResFileWithResource(const CString& areaName)
{
    CAreaFile cAreaFile;

    for (int index = 1; index < 7; index++) {
        CResCache::stru_8FB90C[index] = "";
    }

    CResCache::stru_8FB90C[0] = areaName;
    CResCache::dword_8FB92C = 1;

    // NOTE: Uninline.
    cAreaFile.SetResRef(CResRef(areaName), TRUE, TRUE);

    // NOTE: The WED resref is at offset 8 of the ARE file (after signature
    // and version).
    BYTE* pData = cAreaFile.GetData();
    CResFile* pResFile = g_pChitin->cDimm.GetResFilePtr(g_pChitin->cDimm.GetResID(CResRef(pData + 8), 1003));
    cAreaFile.ReleaseData();

    if (pResFile == NULL) {
        return FALSE;
    }

    if (!g_pChitin->cNetwork.m_bConnectionEstablished) {
        if (pResFile->Cache() == TRUE) {
            pResFile->UnCache();
        }
        return TRUE;
    }

    CString sResFileName;
    CString sXLabel;
    CString sYLabel;
    CPoint ptLocation;

    pResFile->sub_78A0F0(sResFileName);

    if ((m_ruleTables.m_tAreaLinkageCaching.Find(sResFileName, ptLocation, TRUE)
            & m_ruleTables.m_tAreaLinkageCaching.Find(sResFileName, sXLabel, sYLabel, TRUE))
        == TRUE) {
        // NOTE: Uninline.
        CString sCount = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(0, ptLocation.y));
        int nCount = atoi(sCount.GetBuffer(sCount.GetLength()));

        if (nCount != 0) {
            int index;
            UINT nResFileID;

            index = 0;
            nResFileID = g_pChitin->cDimm.GetResFileID(sYLabel);
            while (index <= nCount) {
                pResFile = g_pChitin->cDimm.GetResFilePtr(nResFileID);
                if (pResFile == NULL) {
                    return FALSE;
                }

                if (!pResFile->GetCacheSize()) {
                    if (pResFile->Cache() == TRUE) {
                        pResFile->UnCache();
                    }
                }

                index++;

                // NOTE: Uninline.
                sResFileName = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(index, ptLocation.y));
                nResFileID = g_pChitin->cDimm.GetResFileID(sResFileName);
            }

            index = 0;
            nResFileID = g_pChitin->cDimm.GetResFileID(sYLabel);
            while (index <= nCount) {
                pResFile = g_pChitin->cDimm.GetResFilePtr(nResFileID);
                if (pResFile == NULL) {
                    return FALSE;
                }

                // NOTE: On the first iteration `sResFileName` still holds the
                // value read past the last column by the first loop.
                CResCache::stru_8FB90C[CResCache::dword_8FB92C++] = sResFileName;

                if (pResFile->GetCacheSize()) {
                    if (pResFile->Cache() == TRUE) {
                        pResFile->UnCache();
                    }
                }

                index++;

                // NOTE: Uninline.
                sResFileName = m_ruleTables.m_tAreaLinkageCaching.GetAt(CPoint(index, ptLocation.y));
                nResFileID = g_pChitin->cDimm.GetResFileID(sResFileName);
            }

            return TRUE;
        }
    }

    if (pResFile->Cache() == TRUE) {
        pResFile->UnCache();
    }

    return TRUE;
}

// 0x5A0F50
LONG CInfGame::ImportCharacter(const CString& sFileName, INT nIndex)
{
    CCharacterFile cCharacterData;
    BOOLEAN bTemporaryKey = FALSE;
    CBiographyFile cBiographyData;
    STR_RES strRes;

    // NOTE: Uninline.
    cCharacterData.SetResRef(CResRef(sFileName), TRUE, TRUE);

    if (cCharacterData.GetResRef() == "") {
        // Not in the resource index yet, add the file from the characters
        // folder.
        bTemporaryKey = TRUE;
        g_pChitin->cDimm.AddTemporaryKey(m_sCharactersDir, CResRef(sFileName), 1018);

        // NOTE: Uninline.
        cCharacterData.SetResRef(CResRef(sFileName), TRUE, TRUE);
    }

    BYTE* pData = cCharacterData.GetData();
    DWORD nExportCharacter = cCharacterData.GetDataSize();

    if (pData != NULL && memcmp(pData, "CHR V2.2", 8) == 0) {
        EXPORT_CHARACTER_HEADER* pHeader = reinterpret_cast<EXPORT_CHARACTER_HEADER*>(pData);

        nExportCharacter -= sizeof(EXPORT_CHARACTER_HEADER);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 4172
        UTIL_ASSERT(static_cast<LONG>(nExportCharacter) > 0);

        nExportCharacter -= pHeader->nCreatureSize;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 4175
        UTIL_ASSERT(static_cast<LONG>(nExportCharacter) >= 0);

        CGameSprite* pSprite = new CGameSprite(pData + pHeader->nCreatureOffset,
            pHeader->nCreatureSize,
            0,
            0,
            -1,
            0,
            0,
            INT_MAX,
            CPoint(-1, -1),
            0);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 4180
        UTIL_ASSERT(pSprite != NULL);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 4181
        UTIL_ASSERT(nExportCharacter == 0);

        pSprite->SetResRef(CResRef(sFileName));

        char szName[33] = { 0 };
        strncpy(szName, pHeader->szName, 32);
        pSprite->m_sName = CString(szName);

        for (int index = 0; index < 8; index++) {
            pSprite->InitQuickItemData(pHeader->quickWeaponSlots[index],
                pHeader->quickWeaponAbilities[index],
                pHeader->quickWeaponsField_1A4[index],
                pSprite->m_quickWeapons[index]);
        }

        for (int index = 0; index < 9; index++) {
            pSprite->InitQuickSpellData(CResRef(pHeader->quickSpells[index]),
                1,
                pSprite->m_quickSpells[index],
                pHeader->quickSpellsClass[index],
                pHeader->quickSpellsField_164[index],
                pHeader->quickSpellsKit[index]);
        }

        for (int index = 0; index < 3; index++) {
            pSprite->InitQuickItemData(pHeader->quickItemSlots[index],
                pHeader->quickItemAbilities[index],
                0,
                pSprite->m_quickItems[index]);
        }

        for (int index = 0; index < 9; index++) {
            pSprite->InitQuickSpellData(CResRef(pHeader->quickInnates[index]),
                2,
                pSprite->m_quickInnates[index],
                0,
                0,
                0);
        }

        for (int index = 0; index < 9; index++) {
            pSprite->InitQuickSpellData(CResRef(pHeader->quickSongs[index]),
                3,
                pSprite->m_quickSongs[index],
                0,
                0,
                0);
        }

        for (int index = 0; index < 9; index++) {
            pSprite->field_3D14[index] = pHeader->field_13E[index];
        }

        pSprite->m_nLastSpellbookClassIndex = pHeader->nLastSpellbookClassIndex;
        pSprite->m_nLastSpellbookSpellLevel = pHeader->nLastSpellbookSpellLevel;

        if (bTemporaryKey == TRUE) {
            g_pChitin->cDimm.RemoveTemporaryKey(m_sCharactersDir, CResRef(sFileName), 1018);
        }

        // NOTE: Uninline.
        cBiographyData.SetResRef(CResRef(sFileName), TRUE, TRUE);

        if (cBiographyData.GetResRef() != "") {
            // Biographies of imported characters go to user talk table
            // entries, one per party slot.
            STRREF strBiography = static_cast<BYTE>(nIndex) + 62016;
            strRes.szText = CString(cBiographyData.GetData());
            strRes.szText = strRes.szText.Left(cBiographyData.pRes->nSize);
            g_pBaldurChitin->GetTlkTable().m_override.AddUserEntry(strBiography, strRes);
            pSprite->m_baseStats.m_biography = strBiography;
            cBiographyData.pRes->Release();
        } else {
            pSprite->m_baseStats.m_biography = -1;
        }

        BYTE nClass;

        nClass = CAIOBJECTTYPE_C_CLERIC;
        pSprite->AddKnownDivineSpells(nClass);

        nClass = CAIOBJECTTYPE_C_DRUID;
        pSprite->AddKnownDivineSpells(nClass);

        nClass = CAIOBJECTTYPE_C_PALADIN;
        pSprite->AddKnownDivineSpells(nClass);

        nClass = CAIOBJECTTYPE_C_RANGER;
        pSprite->AddKnownDivineSpells(nClass);

        AddPartyGold(g_pBaldurChitin->GetObjectGame()->GetRuleTables().GetStartingGold(pSprite));

        CString sSoundSet(pHeader->szSoundSet);
        DWORD nLength = sSoundSet.GetLength();
        memcpy(pSprite->field_725A, sSoundSet.GetBuffer(nLength), min(nLength, 32));
        if (nLength < 32) {
            pSprite->field_725A[nLength] = '\0';
        }

        CString sPath = g_pBaldurChitin->GetObjectGame()->GetDirSounds() + sSoundSet + '\\';
        g_pChitin->cDimm.AddToDirectoryList(sPath, TRUE);

        pSprite->m_secondarySounds = CResRef(pHeader->secondarySounds);

        cCharacterData.pRes->dwFlags |= 0x100;
        cCharacterData.ReleaseData();

        pSprite->m_bGlobal = TRUE;
        pSprite->sub_726810(pSprite->m_nWeaponSet);

        return pSprite->GetId();
    }

    cCharacterData.ReleaseData();

    if (bTemporaryKey) {
        g_pChitin->cDimm.RemoveTemporaryKey(m_sCharactersDir, CResRef(sFileName), 1018);
        cCharacterData.ReleaseData();
    }

    return 0;
}

// 0x5A1A80
CString CInfGame::GetDirSounds()
{
    return m_sSoundsDir;
}

// 0x5A1AB0
void CInfGame::CharacterExport(LONG nCharacterId, CString sFileName)
{
    CCharacterFile cCharacterData;
    CBiographyFile cBiographyData;
    BOOLEAN bTemporaryKey = FALSE;
    CGameSprite* pSprite;

    BYTE rc = m_cObjectArray.GetShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pSprite),
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    BYTE* pCharacter;
    LONG nCharacterSize;
    WORD nFacing;
    pSprite->Marshal(&pCharacter, &nCharacterSize, &nFacing, FALSE, FALSE);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 4302
    UTIL_ASSERT(pCharacter != NULL);

    LONG nExportCharacter = sizeof(EXPORT_CHARACTER_HEADER) + nCharacterSize;
    BYTE* pExportCharacter = new BYTE[nExportCharacter];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 4307
    UTIL_ASSERT(pExportCharacter != NULL);

    memset(pExportCharacter, 0, nExportCharacter);

    EXPORT_CHARACTER_HEADER* pHeader = reinterpret_cast<EXPORT_CHARACTER_HEADER*>(pExportCharacter);

    memcpy(pHeader, "CHR V2.2", 8);
    memset(pHeader->szName, 0, sizeof(pHeader->szName));
    strncpy(pHeader->szName, pSprite->GetName(), 32);

    for (int index = 0; index < 8; index++) {
        pHeader->quickWeaponSlots[index] = pSprite->m_quickWeapons[index].m_abilityId.m_itemNum;
        pHeader->quickWeaponAbilities[index] = pSprite->m_quickWeapons[index].m_abilityId.m_abilityNum;
        pHeader->quickWeaponsField_1A4[index] = pSprite->field_3D3A[index];
    }

    for (int index = 0; index < 9; index++) {
        pSprite->m_quickSpells[index].m_abilityId.m_res.GetResRef(pHeader->quickSpells[index]);
        pHeader->quickSpellsClass[index] = pSprite->m_quickSpells[index].m_abilityId.m_nClass;
        pHeader->quickSpellsField_164[index] = pSprite->m_quickSpells[index].m_abilityId.field_1D;
        pHeader->quickSpellsKit[index] = static_cast<BYTE>(pSprite->m_quickSpells[index].m_abilityId.field_1E);
    }

    for (int index = 0; index < 3; index++) {
        pHeader->quickItemSlots[index] = pSprite->m_quickItems[index].m_abilityId.m_itemNum;
        pHeader->quickItemAbilities[index] = pSprite->m_quickItems[index].m_abilityId.m_abilityNum;
    }

    for (int index = 0; index < 9; index++) {
        pSprite->m_quickInnates[index].m_abilityId.m_res.GetResRef(pHeader->quickInnates[index]);
    }

    for (int index = 0; index < 9; index++) {
        pSprite->m_quickSongs[index].m_abilityId.m_res.GetResRef(pHeader->quickSongs[index]);
    }

    for (int index = 0; index < 9; index++) {
        pHeader->field_13E[index] = pSprite->field_3D14[index];
    }

    pHeader->nLastSpellbookClassIndex = pSprite->m_nLastSpellbookClassIndex;
    pHeader->nLastSpellbookSpellLevel = pSprite->m_nLastSpellbookSpellLevel;
    pHeader->field_176[2] = pSprite->m_nLastSong;
    pHeader->nCreatureOffset = sizeof(EXPORT_CHARACTER_HEADER);
    pHeader->nCreatureSize = nCharacterSize;

    CResRef cSecondarySounds = pSprite->m_secondarySounds;
    cSecondarySounds.GetResRef(pHeader->secondarySounds);

    CString sSoundSet;
    sSoundSet = reinterpret_cast<char*>(pSprite->field_725A);
    memcpy(pHeader->szSoundSet, sSoundSet, sSoundSet.GetLength());

    LONG cnt = sizeof(EXPORT_CHARACTER_HEADER);
    memcpy(pExportCharacter + cnt, pCharacter, nCharacterSize);
    cnt += nCharacterSize;
    delete[] pCharacter;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 4364
    UTIL_ASSERT(cnt == nExportCharacter);

    // NOTE: Uninline.
    cCharacterData.SetResRef(CResRef(sFileName), TRUE, TRUE);

    if (cCharacterData.GetResRef() == "") {
        bTemporaryKey = TRUE;
        g_pChitin->cDimm.AddTemporaryKey(m_sCharactersDir, CResRef(sFileName), 1018);

        // NOTE: Uninline.
        cCharacterData.SetResRef(CResRef(sFileName), TRUE, TRUE);
    }

    cCharacterData.GetRes()->Write(m_sCharactersDir, pExportCharacter, nExportCharacter);

    if (bTemporaryKey == TRUE) {
        g_pChitin->cDimm.RemoveTemporaryKey(m_sCharactersDir, CResRef(sFileName), 1018);
    }

    delete[] pExportCharacter;

    STRREF strBiography = pSprite->m_baseStats.m_biography;
    STR_RES strRes;
    g_pBaldurChitin->GetTlkTable().Fetch(strBiography, strRes);

    // NOTE: Uninline.
    cBiographyData.SetResRef(CResRef(sFileName), TRUE, TRUE);

    bTemporaryKey = FALSE;
    if (cBiographyData.GetResRef() == "") {
        bTemporaryKey = TRUE;
        g_pChitin->cDimm.AddTemporaryKey(m_sCharactersDir, CResRef(sFileName), 1022);

        // NOTE: Uninline.
        cBiographyData.SetResRef(CResRef(sFileName), TRUE, TRUE);
    }

    LONG nLength = strRes.szText.GetLength();
    cBiographyData.GetRes()->Write(m_sCharactersDir,
        strRes.szText.GetBuffer(0),
        (nLength > 0) + nLength);

    if (bTemporaryKey == TRUE) {
        g_pChitin->cDimm.RemoveTemporaryKey(m_sCharactersDir, CResRef(sFileName), 1022);
    }

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x5A24D0
CGameArea* CInfGame::LoadArea(CString areaName, BYTE nTravelScreenImageToUse, BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    CAreaFile areaData;
    CString sLevel;
    CString sResRef;
    LONG highRequests;
    LONG mediumRequests;
    LONG lowRequests;
    DWORD nTotalCachingRequirements;
    BYTE cnt;
    CGameArea* pArea;

    areaName.MakeUpper();

    pArea = GetArea(areaName);
    if (pArea != NULL) {
        if (bProgressBarInPlace) {
            ProgressBarCallback(3000000, FALSE);
        }
        return pArea;
    }

    if (!bProgressBarInPlace) {
        g_pChitin->m_bDisplayStale = TRUE;
        SleepEx(25, FALSE);
    }

    BYTE nAverageLevel = 0;
    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        CGameSprite* pSprite;
        LONG nCharacterId = m_characterPortraits[nPortrait];
        BYTE rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            nAverageLevel += pSprite->GetDerivedStats()->m_nLevel;
            m_cObjectArray.ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (m_nCharacters > 0) {
        sLevel.Format("%d", nAverageLevel / m_nCharacters);
    }

    highRequests = g_pChitin->cDimm.RequestsPendingCount(CDimm::PRIORITY_HIGH);
    mediumRequests = g_pChitin->cDimm.RequestsPendingCount(CDimm::PRIORITY_MEDIUM);
    lowRequests = g_pChitin->cDimm.RequestsPendingCount(CDimm::PRIORITY_LOW);

    cnt = 0;
    while (cnt < CINFGAME_MAX_AREAS) {
        if (m_gameAreas[cnt] == NULL) {
            break;
        }
        cnt++;
    }

    if (cnt >= CINFGAME_MAX_AREAS) {
        return NULL;
    }

    m_gameAreas[cnt] = new CGameArea(cnt);
    if (m_gameAreas[cnt] == NULL) {
        return NULL;
    }

    if (nTravelScreenImageToUse != 0xFF) {
        m_nTravelScreenImageToUse = nTravelScreenImageToUse;
    } else {
        m_nTravelScreenImageToUse = rand() % 5;
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->GetListenToJoinOption() == TRUE
            && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineLoad) {
            g_pBaldurChitin->GetBaldurMessage()->ResourceSuggestLoad(areaName,
                1010,
                m_nTravelScreenImageToUse | (bProgressBarInPlace | bProgressBarRequired) << 8);
        }
        SleepEx(200, FALSE);
    }

    g_pChitin->m_bDisplayStale = TRUE;
    sub_59FA00(TRUE);

    nTotalCachingRequirements = CachingRequirements(areaName);
    if (bProgressBarRequired == TRUE || nTotalCachingRequirements > 0) {
        bProgressBarRequired = TRUE;
        nTotalCachingRequirements += 5000000;
    } else {
        bProgressBarRequired = FALSE;
    }

    nTotalCachingRequirements += 3000000;

    if (bProgressBarInPlace == TRUE) {
        if (bProgressBarRequired == TRUE) {
            g_pChitin->cProgressBar.AddActionTarget(nTotalCachingRequirements - 3000000);
        } else {
            ProgressBarCallback(3000000 - nTotalCachingRequirements, FALSE);
        }
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !g_pChitin->cNetwork.GetSessionHosting()
        && !g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(areaName, 1010, TRUE, FALSE, TRUE)) {
        g_pChitin->cNetwork.CloseSession(TRUE);
        m_bInDestroyGame = TRUE;
        delete m_gameAreas[cnt];
        m_gameAreas[cnt] = NULL;
        m_bInDestroyGame = FALSE;
        return NULL;
    }

    if (bProgressBarRequired == TRUE
        && !bProgressBarInPlace) {
        field_1B86 = areaName;
        g_pChitin->SetProgressBar(TRUE,
            9886,
            0,
            0,
            TRUE,
            9885,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = nTotalCachingRequirements;
    }

    CacheResFileWithResource(areaName);

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        if (!g_pChitin->cNetwork.GetSessionHosting()) {
            if (bProgressBarInPlace || bProgressBarRequired) {
                if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
                    g_pChitin->cProgressBar.m_nWaitingReason = 16513;
                    g_pChitin->cProgressBar.m_bWaiting = TRUE;
                }

                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(areaName, 1010, FALSE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    g_pChitin->SetProgressBar(FALSE,
                        0,
                        0,
                        0,
                        FALSE,
                        0,
                        FALSE,
                        0,
                        FALSE,
                        FALSE,
                        255);
                    m_bInDestroyGame = TRUE;
                    delete m_gameAreas[cnt];
                    m_gameAreas[cnt] = NULL;
                    m_bInDestroyGame = FALSE;
                }

                if (bProgressBarInPlace || bProgressBarRequired) {
                    g_pChitin->cProgressBar.m_bWaiting = FALSE;
                    g_pBaldurChitin->m_cCachingStatus.InvalidateScreen();
                }
            }
        }

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            m_gameAreas[cnt]->m_nInitialAreaID = 0;
            g_pBaldurChitin->GetObjectGame()->m_nAreaFirstObject = -1;

            // TODO: Unclear.
            DWORD idPlayer = -1;
            BOOLEAN bIDInUse = TRUE;
            while (bIDInUse) {
                bIDInUse = g_pChitin->cNetwork.FindPlayerLocationByID(idPlayer, FALSE) != -1;
                for (INT nArea = 0; nArea < CINFGAME_MAX_AREAS; nArea++) {
                    if (m_gameAreas[nArea] != NULL
                        && m_gameAreas[nArea]->m_nInitialAreaID == idPlayer) {
                        bIDInUse = TRUE;
                        break;
                    }
                }
                if (bIDInUse) {
                    idPlayer--;
                }
            }

            m_gameAreas[cnt]->m_nInitialAreaID = idPlayer;
            m_nUniqueAreaID = idPlayer;
        }
    }

    sub_59FA00(TRUE);

    m_bInLoadArea = TRUE;

    areaData.SetResRef(CResRef(areaName), TRUE, TRUE);
    m_gameAreas[cnt]->m_resRef = areaName;
    m_gameAreas[cnt]->m_cGameAreaNotes.m_rArea = CResRef(areaName);
    m_gameAreas[cnt]->Unmarshal(areaData.GetData(),
        areaData.GetDataSize(),
        areaName,
        bProgressBarInPlace);

    areaData.ReleaseData();

    if (g_pBaldurChitin->cNetwork.GetSessionHosting() == TRUE
        && !g_pBaldurChitin->GetObjectGame()->m_bInLoadGame) {
        m_gameAreas[cnt]->SortLists();
        m_gameAreas[cnt]->EnterSpawn();
    }

    POSITION pos = m_lstGlobalCreatures.GetHeadPosition();
    while (pos != NULL) {
        CGameSprite* pSprite;
        LONG iSprite = static_cast<LONG>(reinterpret_cast<INT_PTR>(m_lstGlobalCreatures.GetNext(pos)));
        BYTE rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->m_currentArea == areaName) {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 4774
                UTIL_ASSERT(pSprite->GetArea() == NULL);

                rc = m_cObjectArray.GetDeny(iSprite,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
                if (rc == CGameObjectArray::SUCCESS) {
                    if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) != 0
                        && pSprite->GetAnimation()->CanLieDown()) {
                        pSprite->AddToArea(m_gameAreas[cnt],
                            pSprite->GetPos(),
                            0,
                            CGameObject::LIST_BACK);
                    } else {
                        pSprite->AddToArea(m_gameAreas[cnt],
                            pSprite->GetPos(),
                            0,
                            CGameObject::LIST_FRONT);
                    }
                    m_cObjectArray.ReleaseDeny(iSprite,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    m_bInLoadArea = FALSE;

    m_gameAreas[cnt]->m_nFirstObject = g_pBaldurChitin->GetObjectGame()->m_nAreaFirstObject;

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        g_pBaldurChitin->GetObjectGame()->GetRemoteObjectArray()->ChangeControlOnLoadGame();
    }

    if (bProgressBarRequired) {
        if (g_pChitin->field_4C == 1 && !g_pChitin->m_bExitRSThread) {
            int v1 = highRequests + mediumRequests + lowRequests;
            int v6 = 0;
            g_pChitin->cDimm.MoveRequests(CDimm::PRIORITY_HIGH, CDimm::PRIORITY_LOW, highRequests);
            g_pChitin->cDimm.MoveRequests(CDimm::PRIORITY_MEDIUM, CDimm::PRIORITY_LOW, mediumRequests);
            g_pChitin->cDimm.MoveRequests(CDimm::PRIORITY_LOW, CDimm::PRIORITY_LOW, lowRequests);
            int v2 = g_pChitin->cDimm.RequestsPendingCount() - v1;
            int v3 = v2;
            while (v3 > 0) {
                if (v6 >= 100) {
                    break;
                }

                if (!g_pChitin->cDimm.ResumeServicing()
                    || g_pChitin->cDimm.MemoryAlmostFull()) {
                    int v4 = v1 - g_pChitin->cDimm.RequestsPendingCount();
                    g_pChitin->cDimm.ReduceServicedList();
                    v1 = g_pChitin->cDimm.RequestsPendingCount() + v4;
                    g_pChitin->cDimm.ResumeServicing();
                }

                g_pChitin->cDimm.field_0 = 0;
                g_pChitin->cDimm.field_4 = 0;
                SleepEx(50, FALSE);

                int v5 = g_pChitin->cDimm.RequestsPendingCount() - v1;
                if (v5 != v3) {
                    g_pChitin->cProgressBar.m_nActionProgress += (v3 - v5) * (5000000 / v2);
                    v3 = v5;
                    g_pChitin->m_bDisplayStale = TRUE;
                }

                v6++;
            }
        }

        if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
            g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;
            g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
            g_pChitin->m_bDisplayStale = TRUE;
            sub_59FA00(TRUE);
            g_pChitin->SetProgressBar(FALSE,
                0,
                0,
                0,
                FALSE,
                0,
                FALSE,
                0,
                FALSE,
                FALSE,
                255);
        }
    }

    m_nTravelScreenImageToUse = rand() % 5;

    return m_gameAreas[cnt];
}

// 0x5A3230
void CInfGame::Marshal(BYTE** pGame, LONG* nGame, BOOLEAN bProgressBarInPlace)
{
    CSavedGamePartyCreature partyCreatures[6];
    CSavedGamePartyCreature* pGlobalCreature = NULL;
    CVariable* pVariables = NULL;
    WORD nVariables;
    SHORT nPCAreaViewed = 0;
    DWORD dwProgressPerCreature = 0;
    DWORD dwProgressMisc = 0;
    CGameSprite* pSprite;
    SHORT cnt3;
    SHORT index;
    DWORD nUnknownSize;
    POSITION pos;
    BYTE rc;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 4924
    UTIL_ASSERT(pGame != NULL && nGame != NULL);

    if (bProgressBarInPlace) {
        ProgressBarCallback(0, TRUE);

        DWORD nCreatures = m_nCharacters + m_lstGlobalCreatures.GetCount();
        if (nCreatures != 0) {
            dwProgressPerCreature = 2187500 / nCreatures;
            dwProgressMisc = 312500;
        } else {
            dwProgressPerCreature = 0;
            dwProgressMisc = 2500000;
        }
    }

    memset(partyCreatures, 0, sizeof(partyCreatures));

    *nGame = sizeof(CSavedGameHeader) + 8;
    *nGame += m_nCharacters * sizeof(CSavedGamePartyCreature);

    cnt3 = 0;
    for (index = 0; index < 6; index++) {
        if (m_characters[index] != -1) {
            if (bProgressBarInPlace) {
                ProgressBarCallback(dwProgressPerCreature, FALSE);
            }

            rc = m_cObjectArray.GetShare(m_characters[index],
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                pSprite->Marshal(partyCreatures[cnt3], FALSE);
                *nGame += partyCreatures[cnt3].m_creatureSize;

                if (pSprite->m_pArea == m_gameAreas[m_visibleArea]) {
                    nPCAreaViewed = index;
                }

                m_cObjectArray.ReleaseShare(m_characters[index],
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);

                cnt3++;
            } else {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 4979
                UTIL_ASSERT(FALSE);
            }
        }
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 4990
    UTIL_ASSERT(cnt3 == m_nCharacters);

    if (m_lstGlobalCreatures.GetCount() > 0) {
        pGlobalCreature = new CSavedGamePartyCreature[m_lstGlobalCreatures.GetCount()];

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 4995
        UTIL_ASSERT(pGlobalCreature != NULL);

        memset(pGlobalCreature,
            0,
            sizeof(CSavedGamePartyCreature) * m_lstGlobalCreatures.GetCount());

        *nGame += sizeof(CSavedGamePartyCreature) * m_lstGlobalCreatures.GetCount();

        pos = m_lstGlobalCreatures.GetHeadPosition();
        index = 0;
        while (pos != NULL) {
            LONG iSprite = static_cast<LONG>(reinterpret_cast<INT_PTR>(m_lstGlobalCreatures.GetNext(pos)));

            if (bProgressBarInPlace) {
                ProgressBarCallback(dwProgressPerCreature, FALSE);
            }

            rc = m_cObjectArray.GetShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                pSprite->Marshal(pGlobalCreature[index], FALSE);
                *nGame += pGlobalCreature[index].m_creatureSize;

                m_cObjectArray.ReleaseShare(iSprite,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            index++;
        }
    }

    m_variables.Marshal(&pVariables, &nVariables);
    *nGame += nVariables * sizeof(CVariable);

    *nGame += m_cJournal.CountEntries() * sizeof(CSavedGameJournalEntry)
        + sizeof(CSavedGameFamiliars);

    for (int alignment = 0; alignment < 9; alignment++) {
        for (int level = 0; level < 9; level++) {
            *nGame += m_familiarResRefs[alignment][level].GetCount() * sizeof(RESREF);
        }
    }

    if (bProgressBarInPlace) {
        ProgressBarCallback(dwProgressMisc, FALSE);
    }

    // NOTE: Inlined.
    nUnknownSize = sizeof(DWORD);
    pos = field_4BDC.GetHeadPosition();
    while (pos != NULL) {
        CInfGameUnknownEntry* pEntry = reinterpret_cast<CInfGameUnknownEntry*>(field_4BDC.GetNext(pos));
        nUnknownSize += pEntry->m_sName.GetLength() + sizeof(CSavedGameUnknownEntry);
    }

    *nGame += nUnknownSize;

    *pGame = new BYTE[*nGame];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 5063
    UTIL_ASSERT(*pGame != NULL);

    memset(*pGame, 0, *nGame);

    DWORD cnt = 0;

    memcpy(*pGame + cnt, "GAMEV2.2", 8);
    cnt += 8;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 5067
    UTIL_ASSERT(cnt <= (DWORD)*nGame);

    CSavedGameHeader* pHeader = reinterpret_cast<CSavedGameHeader*>(*pGame + cnt);
    cnt += sizeof(CSavedGameHeader);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 5070
    UTIL_ASSERT(cnt <= (DWORD)*nGame);

    pHeader->m_worldTime = m_worldTime.m_gameTime / CTimerWorld::TIMESCALE_MSEC_PER_SEC;
    m_gameSave.m_cResCurrentWorldArea.GetResRef(pHeader->m_currentWorldArea);
    pHeader->m_currentLink = m_gameSave.m_nCurrentWorldLink;
    pHeader->m_nPCAreaViewed = nPCAreaViewed;

    pHeader->m_curFormation = 0;
    for (SHORT nFormation = 4; nFormation >= 0; nFormation--) {
        pHeader->m_quickFormations[nFormation] = m_gameSave.m_quickFormations[nFormation];
        if (m_gameSave.m_curFormation == m_gameSave.m_quickFormations[nFormation]) {
            pHeader->m_curFormation = nFormation;
        }
    }

    pHeader->m_nPartyGold = m_gameSave.m_nPartyGold;
    pHeader->m_wWeatherFlags = g_pBaldurChitin->m_pEngineWorld->m_weather.Marshal();

    if (!m_bPartyAI) {
        pHeader->m_dwFlags |= 0x1;
    }

    if (!m_bShowAreaNotes) {
        pHeader->m_dwFlags |= 0x80;
    }

    if (g_pBaldurChitin->m_pEngineWorld->field_11F0 == 0) {
        pHeader->m_dwFlags |= 0x20;
    }

    if (g_pBaldurChitin->m_pEngineWorld->field_11F4 == 0) {
        pHeader->m_dwFlags |= 0x40;
    }

    if (g_pBaldurChitin->m_pEngineWorld->GetManager()->m_bHidden) {
        pHeader->m_dwFlags |= 0x10;
    }

    pHeader->m_versionNumber = 3;

    BYTE panels[2];
    BYTE controls[2];
    panels[0] = 7;
    panels[1] = 19;
    controls[0] = 0;
    controls[1] = 0;

    for (BYTE nPanel = 0; nPanel < 2; nPanel++) {
        CUIPanel* pPanel = g_pBaldurChitin->m_pEngineWorld->GetManager()->GetPanel(panels[nPanel]);
        if (pPanel->m_bActive) {
            if (pPanel->GetControl(controls[nPanel])->m_bActive) {
                pHeader->m_dwFlags |= 0x2 | 0x4;
            }
            break;
        }
    }

    pHeader->m_reputation = m_nReputation;

    if (m_pGameAreaMaster != NULL) {
        m_pGameAreaMaster->m_resRef.GetResRef(pHeader->m_masterArea);
    }

    // Party creatures.
    pHeader->m_partyCreatureTableOffset = cnt;
    pHeader->m_partyCreatureTableCount = m_nCharacters;
    cnt += m_nCharacters * sizeof(CSavedGamePartyCreature);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 5136
    UTIL_ASSERT(cnt <= (DWORD)*nGame);

    cnt3 = 0;
    for (index = 0; index < 6; index++) {
        if (m_characters[index] != -1) {
            if (bProgressBarInPlace) {
                ProgressBarCallback(dwProgressPerCreature, FALSE);
            }

            CSavedGamePartyCreature* pPartyCreature = reinterpret_cast<CSavedGamePartyCreature*>(*pGame
                + pHeader->m_partyCreatureTableOffset
                + cnt3 * sizeof(CSavedGamePartyCreature));
            memcpy(pPartyCreature, &(partyCreatures[cnt3]), sizeof(CSavedGamePartyCreature));

            if (partyCreatures[cnt3].m_creatureSize > 0) {
                memcpy(*pGame + cnt,
                    reinterpret_cast<BYTE*>(partyCreatures[cnt3].m_creatureOffset),
                    partyCreatures[cnt3].m_creatureSize);
                delete reinterpret_cast<BYTE*>(partyCreatures[cnt3].m_creatureOffset);

                pPartyCreature->m_creatureOffset = cnt;
                cnt += partyCreatures[cnt3].m_creatureSize;

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 5156
                UTIL_ASSERT(cnt <= (DWORD)*nGame);
            }

            cnt3++;
        }
    }

    // Global creatures.
    pHeader->m_globalCreatureTableOffset = cnt;
    pHeader->m_globalCreatureTableCount = m_lstGlobalCreatures.GetCount();
    cnt += m_lstGlobalCreatures.GetCount() * sizeof(CSavedGamePartyCreature);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 5166
    UTIL_ASSERT(cnt <= (DWORD)*nGame);

    for (index = 0; index < m_lstGlobalCreatures.GetCount(); index++) {
        if (bProgressBarInPlace) {
            ProgressBarCallback(dwProgressPerCreature, FALSE);
        }

        CSavedGamePartyCreature* pPartyCreature = reinterpret_cast<CSavedGamePartyCreature*>(*pGame
            + pHeader->m_globalCreatureTableOffset
            + index * sizeof(CSavedGamePartyCreature));
        memcpy(pPartyCreature, &(pGlobalCreature[index]), sizeof(CSavedGamePartyCreature));

        if (pGlobalCreature[index].m_creatureSize > 0) {
            memcpy(*pGame + cnt,
                reinterpret_cast<BYTE*>(pGlobalCreature[index].m_creatureOffset),
                pGlobalCreature[index].m_creatureSize);
            delete reinterpret_cast<BYTE*>(pGlobalCreature[index].m_creatureOffset);

            pPartyCreature->m_creatureOffset = cnt;
            cnt += pGlobalCreature[index].m_creatureSize;

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 5183
            UTIL_ASSERT(cnt <= (DWORD)*nGame);
        }
    }

    if (pGlobalCreature != NULL) {
        delete[] pGlobalCreature;
    }

    // Global variables.
    if (nVariables > 0) {
        pHeader->m_globalVariablesOffset = cnt;
        pHeader->m_globalVariablesCount = nVariables;
        memcpy(*pGame + cnt, pVariables, nVariables * sizeof(CVariable));
        delete[] pVariables;
        cnt += nVariables * sizeof(CVariable);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 5197
        UTIL_ASSERT(cnt <= (DWORD)*nGame);
    }

    // Journal.
    pHeader->m_journalEntriesOffset = cnt;
    pHeader->m_journalEntriesCount = m_cJournal.CountEntries();
    m_cJournal.Marshal(reinterpret_cast<CSavedGameJournalEntry*>(*pGame + cnt));
    cnt += pHeader->m_journalEntriesCount * sizeof(CSavedGameJournalEntry);

    // Familiars.
    pHeader->m_storedLocationsOffset = m_cOptions.m_nNightmareMode;
    pHeader->m_familiarsOffset = cnt;

    CSavedGameFamiliars* pFamiliarInfo = reinterpret_cast<CSavedGameFamiliars*>(*pGame + cnt);
    cnt += sizeof(CSavedGameFamiliars);

    memcpy(pFamiliarInfo->m_defaultFamiliarResRefs, m_defaultFamiliarResRefs, sizeof(m_defaultFamiliarResRefs));
    pFamiliarInfo->m_familiarResRefsOffset = cnt;

    for (int alignment = 0; alignment < 9; alignment++) {
        for (int level = 0; level < 9; level++) {
            pFamiliarInfo->m_familiarResRefsCount[alignment][level] = m_familiarResRefs[alignment][level].GetCount();

            pos = m_familiarResRefs[alignment][level].GetHeadPosition();
            while (pos != NULL) {
                CResRef* pResRef = m_familiarResRefs[alignment][level].GetNext(pos);
                memcpy(*pGame + cnt, pResRef->GetResRef(), sizeof(RESREF));
                cnt += sizeof(RESREF);
            }
        }
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 5225
    UTIL_ASSERT(cnt <= (DWORD)*nGame);

    // `field_4BDC` entries.
    BYTE* pData = *pGame + cnt;
    *reinterpret_cast<DWORD*>(pData) = field_4BDC.GetCount();
    pData += sizeof(DWORD);

    pos = field_4BDC.GetHeadPosition();
    while (pos != NULL) {
        CInfGameUnknownEntry* pEntry = reinterpret_cast<CInfGameUnknownEntry*>(field_4BDC.GetNext(pos));
        CSavedGameUnknownEntry* pSavedEntry = reinterpret_cast<CSavedGameUnknownEntry*>(pData);
        DWORD nSize = pEntry->m_sName.GetLength() + sizeof(CSavedGameUnknownEntry) - sizeof(DWORD);

        pSavedEntry->m_nSize = nSize;
        pSavedEntry->m_nLength = pEntry->m_sName.GetLength();
        pSavedEntry->field_8 = pEntry->field_4;

        // NOTE: The original writes `field_8` as a zero-extended `DWORD`, then
        // overwrites its upper three bytes with `field_9`.
        *reinterpret_cast<DWORD*>(&(pSavedEntry->field_C)) = pEntry->field_8;
        memcpy(pSavedEntry->field_D, pEntry->field_9, sizeof(pSavedEntry->field_D));

        memcpy(pData + sizeof(CSavedGameUnknownEntry),
            static_cast<LPCSTR>(pEntry->m_sName),
            pEntry->m_sName.GetLength());

        pData += nSize + sizeof(DWORD);
    }

    // NOTE: Inlined.
    nUnknownSize = sizeof(DWORD);
    pos = field_4BDC.GetHeadPosition();
    while (pos != NULL) {
        CInfGameUnknownEntry* pEntry = reinterpret_cast<CInfGameUnknownEntry*>(field_4BDC.GetNext(pos));
        nUnknownSize += pEntry->m_sName.GetLength() + sizeof(CSavedGameUnknownEntry);
    }

    cnt += nUnknownSize;

    if (bProgressBarInPlace) {
        ProgressBarCallback(dwProgressMisc, FALSE);
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 5238
    UTIL_ASSERT(cnt == (DWORD)*nGame);
}

// 0x5AC110
BOOLEAN CInfGame::CanSaveGame(STRREF& strError, BOOLEAN bInRestGame, BOOLEAN bInStore)
{
    if (!bInRestGame
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !g_pChitin->cNetwork.GetSessionHosting()) {
        // "Client machines are not allowed to save in a multiplayer game."
        strError = 17194;
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 386
        || g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_mode == 1282) {
        // "Cannot save game while in dialog mode."
        strError = 19253;
        return FALSE;
    }

    if (g_pBaldurChitin->GetScreenWorld()->m_comingOutOfDialog > 0 && !bInStore) {
        // "Cannot save game while in dialog mode."
        strError = 19253;
        return FALSE;
    }

    if (g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->m_bHostPermittedDialog == TRUE) {
        // "Cannot save because dialog is about to start."
        strError = 19254;
        return FALSE;
    }

    if (g_pBaldurChitin->GetScreenWorld()->m_bHardPaused == TRUE) {
        // "Cannot save during a rest, chapter information or movie."
        strError = 19255;
        return FALSE;
    }

    if (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineProjector
        || g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineChapter
        || g_pBaldurChitin->GetObjectGame()->GetGameSave()->m_cutScene) {
        // "Cannot save during a rest, chapter information or movie."
        strError = 19255;
        return FALSE;
    }

    if (!bInRestGame
        && g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineStore) {
        // "Cannot save while someone is in the Store Screen."
        strError = 10841;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && !bInRestGame
        && sub_5C93E0() > 0) {
        // "Cannot save while someone is in the Store Screen."
        strError = 10841;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && !bInRestGame
        && g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->sub_5186A0() > 0) {
        // "Cannot save while someone is levelling up."
        strError = 41397;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && !bInRestGame
        && g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->sub_518560() > 0) {
        // "Cannot save while someone is in the Inventory Screen."
        strError = 26546;
        return FALSE;
    }

    for (BYTE cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
        if (m_gameAreas[cnt] != NULL) {
            if (!m_gameAreas[cnt]->CanSaveGame(strError)) {
                return FALSE;
            }
        }
    }

    strError = -1;
    return TRUE;
}

// 0x5AC340
BOOL CInfGame::BackupQuickSave()
{
    BOOL bResult = FALSE;

    m_sSaveGame = QUICK_SAVE_BACKUP_NAME;
    CScreenCharacter::SAVE_NAME = QUICK_SAVE_BACKUP_NAME;

    CString sDirSave = GetDirSave();
    if (g_pChitin->cDimm.DirectoryRemoveFiles(sDirSave)) {
        m_sSaveGame = QUICK_SAVE_NAME;
        CScreenCharacter::SAVE_NAME = QUICK_SAVE_NAME;
        bResult = g_pChitin->cDimm.DirectoryCopyFiles(GetDirSave(), sDirSave);
    }

    return bResult;
}

// 0x5AC430
BOOL CInfGame::SaveGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace, BOOLEAN a3)
{
    CGameFile cGameFile;
    CAreaFile cAreaFile; // NOTE: Unused.
    BOOL bResult = TRUE;
    DWORD dwSectorsPerCluster;
    DWORD dwBytesPerSector;
    DWORD dwNumberOfFreeClusters;
    DWORD dwTotalNumberOfClusters;
    ULARGE_INTEGER nFreeBytesAvailable;
    ULARGE_INTEGER nTotalNumberOfBytes;
    HMODULE hModule;
    BOOL bHaveGetDiskFreeSpaceExA;
    INT cnt;

    for (cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
        if (m_gameAreas[cnt] != NULL) {
            m_gameAreas[cnt]->sub_47A190();
        }
    }

    if (m_sSaveGame == "") {
        return FALSE;
    }

    if (GetDiskFreeSpaceA(NULL, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters) != TRUE
        || dwNumberOfFreeClusters * dwSectorsPerCluster < CBaldurChitin::MINIMUM_DRIVE_SECTORS
        || dwNumberOfFreeClusters * dwSectorsPerCluster < CBaldurChitin::MINIMUM_DRIVE_SPACE / dwBytesPerSector) {
        hModule = LoadLibraryA("kernel32.dll");
        if (hModule == NULL) {
            if (bProgressBarInPlace) {
                g_pChitin->SetProgressBar(FALSE,
                    0,
                    0,
                    0,
                    FALSE,
                    0,
                    FALSE,
                    0,
                    FALSE,
                    FALSE,
                    255);
            }

            g_pBaldurChitin->m_dwCloseConfirmationStrId = 11790;
            g_pBaldurChitin->m_dwCloseConfirmationFlags = 0x34;
            PostMessageA(g_pBaldurChitin->cWnd.GetSafeHwnd(), WM_CLOSE, 0, 0);

            return FALSE;
        }

        bHaveGetDiskFreeSpaceExA = GetProcAddress(hModule, "GetDiskFreeSpaceExA") != NULL;
        CResCache::pfnGetDiskFreeSpaceExA = reinterpret_cast<GetDiskFreeSpaceExAFunc*>(GetProcAddress(hModule, "GetDiskFreeSpaceExA"));

        if (!bHaveGetDiskFreeSpaceExA
            || !CResCache::pfnGetDiskFreeSpaceExA(NULL, &nFreeBytesAvailable, &nTotalNumberOfBytes, NULL)
            || (nFreeBytesAvailable.HighPart == 0 && nFreeBytesAvailable.LowPart < CBaldurChitin::MINIMUM_DRIVE_SPACE)) {
            if (bProgressBarInPlace) {
                g_pChitin->SetProgressBar(FALSE,
                    0,
                    0,
                    0,
                    FALSE,
                    0,
                    FALSE,
                    0,
                    FALSE,
                    FALSE,
                    255);
            }

            g_pBaldurChitin->m_dwCloseConfirmationStrId = 11790;
            g_pBaldurChitin->m_dwCloseConfirmationFlags = 0x34;
            PostMessageA(g_pBaldurChitin->cWnd.GetSafeHwnd(), WM_CLOSE, 0, 0);

            FreeLibrary(hModule);

            return FALSE;
        }

        FreeLibrary(hModule);
    }

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        sub_59FA00(TRUE);

        g_pChitin->SetProgressBar(TRUE,
            9890,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = 5000000;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
    }

    m_cMoveList.sub_5170F0(g_pBaldurChitin->GetObjectGame()->m_pGameAreaMaster);
    m_cLimboList.sub_5170F0(g_pBaldurChitin->GetObjectGame()->m_pGameAreaMaster);

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(0, TRUE);

        g_pChitin->cProgressBar.AddActionTarget(2500000);

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
            && !g_pBaldurChitin->m_pEngineMultiPlayer->m_bMultiplayerStartup) {
            g_pChitin->cProgressBar.AddActionTarget(6000000);
        }

        for (cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
            if (m_gameAreas[cnt] != NULL && (m_gameAreas[cnt]->m_header.m_flags & 0x1) == 0) {
                g_pChitin->cProgressBar.AddActionTarget(3000000);
            }
        }
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !g_pBaldurChitin->m_pEngineMultiPlayer->m_bMultiplayerStartup) {
        g_pBaldurChitin->m_pEngineWorld->StartSaveGameMultiplayerHost();

        g_pBaldurChitin->GetBaldurMessage()->UpdateDemandCharacters(TRUE,
            0,
            bProgressBarRequired | bProgressBarInPlace);
        g_pBaldurChitin->GetBaldurMessage()->PurgeDroppedPlayers();
        g_pBaldurChitin->GetBaldurMessage()->BroadcastUpdateCharacterSlot(TRUE,
            0,
            bProgressBarRequired | bProgressBarInPlace);
        g_pBaldurChitin->GetBaldurMessage()->RequestClientSignal(CBaldurMessage::SIGNAL_END_MAJOR_EVENT);
    }

    if (a3) {
        field_366E = TRUE;
    }

    std::vector<int> ids = Icewind586B70::Instance()->sub_587510();
    for (std::vector<int>::iterator it = ids.begin(); it != ids.end(); it++) {
        g_pBaldurChitin->GetBaldurMessage()->DemandTimedEffects(*it);
    }
    Icewind586B70::Instance()->sub_587190();

    g_pBaldurChitin->GetActiveEngine()->GetManager()->InvalidateRect(NULL);
    g_pBaldurChitin->m_cCachingStatus.InvalidateScreen();

    if (bProgressBarInPlace) {
        ProgressBarCallback(312500, FALSE);
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(312500, FALSE);
    }

    for (cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL) {
            m_aServerStore[cnt]->Marshal(m_sTempDir);
        }
    }

    BOOLEAN bProgressBar = bProgressBarRequired | bProgressBarInPlace;

    BYTE* pGame;
    LONG nGame;
    Marshal(&pGame, &nGame, bProgressBar);

    if (pGame != NULL) {
        // NOTE: Uninline.
        cGameFile.SetResRef(CResRef("ICEWIND2"), TRUE, TRUE);

        cGameFile.GetRes()->Write(m_sTempDir, pGame, nGame);
        delete pGame;
    } else {
        bResult = FALSE;
    }

    for (cnt = 0; cnt < CINFGAME_MAX_AREAS; cnt++) {
        if (m_gameAreas[cnt] != NULL && (m_gameAreas[cnt]->m_header.m_flags & 0x1) == 0) {
            BYTE* pArea;
            LONG nArea;
            m_gameAreas[cnt]->Marshal(&pArea, &nArea, bProgressBar);

            if (pArea != NULL) {
                CAreaFile cAreaData;

                // NOTE: Uninline.
                cAreaData.SetResRef(m_gameAreas[cnt]->m_resRef, TRUE, TRUE);

                cAreaData.GetRes()->Write(m_sTempDir, pArea, nArea);
                delete pArea;
            } else {
                bResult = FALSE;
            }
        }
    }

    m_cWorldMap.Marshal(m_sTempDir);

    g_pBaldurChitin->GetTlkTable().m_override.Save();
    g_pBaldurChitin->GetTlkTable().m_override.CloseFiles();

    if (bResult) {
        if (!g_pChitin->cDimm.DirectoryCopyFiles(m_sTempDir, m_sTempSaveDir)) {
            bResult = FALSE;
        } else if (!g_pChitin->cDimm.CompressDirectory(m_sTempSaveDir, m_sTempSaveDir)) {
            bResult = FALSE;
        } else if (!g_pChitin->cDimm.DirectoryRemoveFiles(GetDirSave())
            || !g_pChitin->cDimm.DirectoryCopyFiles(m_sTempSaveDir, GetDirSave())) {
            bResult = FALSE;
        }
    }

    g_pChitin->cDimm.DirectoryRemoveFiles(m_sTempSaveDir);

    g_pBaldurChitin->GetTlkTable().m_override.Open(CString("temp//default.toh"),
        CString("temp//default.tot"));

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(312500, FALSE);
    }

    EnterCriticalSection(&(g_pChitin->field_314));
    g_pChitin->cDimm.m_cKeyTable.RescanEverything();
    LeaveCriticalSection(&(g_pChitin->field_314));

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(312500, FALSE);
    }

    SaveOptions();
    g_pBaldurChitin->SaveOptions();

    m_nLastSaveTime = m_worldTime.m_gameTime;

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !g_pBaldurChitin->m_pEngineMultiPlayer->m_bMultiplayerStartup) {
        if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL) {
            g_pChitin->cProgressBar.m_nWaitingReason = 19714;
            g_pChitin->cProgressBar.m_bWaiting = TRUE;
        }

        while (g_pBaldurChitin->m_pEngineWorld->m_bHardPaused == TRUE
            && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pBaldurChitin->m_pEngineWorld->CheckEndOfHardPause();
            g_pBaldurChitin->GetBaldurMessage()->HandleBlockingMessages();

            g_pChitin->m_bDisplayStale = TRUE;

            if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL) {
                SynchronousUpdate();
            } else {
                SleepEx(60, FALSE);
            }
        }

        if (!g_pChitin->cNetwork.GetSessionOpen()) {
            if (bProgressBarInPlace || bProgressBarRequired) {
                g_pChitin->SetProgressBar(FALSE,
                    0,
                    0,
                    0,
                    FALSE,
                    0,
                    FALSE,
                    0,
                    FALSE,
                    FALSE,
                    255);
            }

            return TRUE;
        }

        g_pChitin->cProgressBar.m_bWaiting = FALSE;
        g_pBaldurChitin->m_cCachingStatus.InvalidateScreen();
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(1250000, FALSE);
    }

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        g_pChitin->cProgressBar.m_nActionProgress += g_pChitin->cProgressBar.m_nActionTarget - g_pChitin->cProgressBar.m_nActionProgress;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
        g_pChitin->m_bDisplayStale = TRUE;

        sub_59FA00(TRUE);

        g_pChitin->SetProgressBar(FALSE,
            0,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
    }

    if (bResult) {
        if (GetDiskFreeSpaceA(NULL, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters) != TRUE
            || dwNumberOfFreeClusters * dwSectorsPerCluster < CBaldurChitin::MINIMUM_DRIVE_SECTORS
            || dwNumberOfFreeClusters * dwSectorsPerCluster < CBaldurChitin::MINIMUM_DRIVE_SPACE / dwBytesPerSector) {
            hModule = LoadLibraryA("kernel32.dll");
            if (hModule == NULL) {
                g_pBaldurChitin->m_dwCloseConfirmationStrId = 10249;
                g_pBaldurChitin->m_dwCloseConfirmationFlags = 0x10;
                PostMessageA(g_pBaldurChitin->cWnd.GetSafeHwnd(), WM_CLOSE, 0, 0);

                return FALSE;
            }

            bHaveGetDiskFreeSpaceExA = GetProcAddress(hModule, "GetDiskFreeSpaceExA") != NULL;
            CResCache::pfnGetDiskFreeSpaceExA = reinterpret_cast<GetDiskFreeSpaceExAFunc*>(GetProcAddress(hModule, "GetDiskFreeSpaceExA"));

            if (!bHaveGetDiskFreeSpaceExA
                || !CResCache::pfnGetDiskFreeSpaceExA(NULL, &nFreeBytesAvailable, &nTotalNumberOfBytes, NULL)
                || (nFreeBytesAvailable.HighPart == 0 && nFreeBytesAvailable.LowPart < CBaldurChitin::MINIMUM_DRIVE_SPACE)) {
                g_pBaldurChitin->m_dwCloseConfirmationStrId = 10249;
                g_pBaldurChitin->m_dwCloseConfirmationFlags = 0x10;
                PostMessageA(g_pBaldurChitin->cWnd.GetSafeHwnd(), WM_CLOSE, 0, 0);

                FreeLibrary(hModule);

                return FALSE;
            }

            FreeLibrary(hModule);
        }
    } else {
        g_pBaldurChitin->m_dwCloseConfirmationStrId = 23554;
        g_pBaldurChitin->m_dwCloseConfirmationFlags = 0x34;
        PostMessageA(g_pBaldurChitin->cWnd.GetSafeHwnd(), WM_CLOSE, 0, 0);
    }

    return TRUE;
}

// Converts "GAMEV2.0" party creature to "GAMEV2.1" format.
static void sub_5A79E0(const CSavedGamePartyCreature20* pCreature20, CSavedGamePartyCreature21* pCreature21);

// Converts "GAMEV2.1" party creature to "GAMEV2.2" format.
static void sub_5A7BF0(const CSavedGamePartyCreature21* pCreature21, CSavedGamePartyCreature* pCreature22);

// 0x5A7E40
void CInfGame::Unmarshal(BYTE* pGame, LONG nGame, BOOLEAN bProgressBarInPlace)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 7006
    UTIL_ASSERT(pGame != NULL && nGame > 0);

    CBaldurMessage* pMessage = g_pBaldurChitin->GetBaldurMessage();
    CSavedGamePartyCreature21 creature21;
    CSavedGamePartyCreature creature22;
    CSavedGamePartyCreature21* pCreature21 = NULL;
    CSavedGamePartyCreature* pCreature22 = NULL;
    CCreatureFile creatureData;
    CGameSprite* pSprite;
    BYTE* pData;
    INT nMsgFrom;
    DWORD dwSize;
    DWORD dwProgress;
    DWORD dwPerCreature;

    INT nVersion = 20;
    if (memcmp(pGame, "GAMEV2.1", 8) == 0) {
        nVersion = 21;
    } else if (memcmp(pGame, "GAMEV2.2", 8) == 0) {
        nVersion = 22;
    }

    CString sFileType(reinterpret_cast<char*>(pGame), 4);
    CString sFileVersion(reinterpret_cast<char*>(pGame + 4), 4);

    DWORD cnt = 8;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 7042
    UTIL_ASSERT(cnt < (DWORD)nGame);

    CSavedGameHeader* pHeader = reinterpret_cast<CSavedGameHeader*>(pGame + cnt);
    cnt += sizeof(CSavedGameHeader);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 7045
    UTIL_ASSERT(cnt < (DWORD)nGame);

    m_worldTime.m_gameTime = CTimerWorld::TIMESCALE_MSEC_PER_SEC * pHeader->m_worldTime;
    m_worldTime.CheckForTriggerEventPast();

    if (g_pChitin->cNetwork.m_nServiceProvider == CNetwork::SERV_PROV_NULL
        || g_pChitin->cNetwork.m_bIsHost == TRUE) {
        m_gameSave.m_curFormation = pHeader->m_quickFormations[pHeader->m_curFormation];
    }

    m_gameSave.m_cResCurrentWorldArea = pHeader->m_currentWorldArea;
    m_gameSave.m_nCurrentWorldLink = pHeader->m_currentLink;

    if (g_pChitin->cNetwork.m_nServiceProvider == CNetwork::SERV_PROV_NULL
        || g_pChitin->cNetwork.m_bIsHost == TRUE) {
        for (int index = 0; index < 5; index++) {
            m_gameSave.m_quickFormations[index] = pHeader->m_quickFormations[index];
        }
    }

    m_gameSave.m_nPartyGold = pHeader->m_nPartyGold;

    // NOTE: In IWD2 this header field is the nightmare mode flag.
    m_cOptions.m_nNightmareMode = pHeader->m_storedLocationsOffset;
    if (m_cOptions.m_nNightmareMode == 1 && m_cOptions.m_nDifficultyLevel != 5) {
        m_cOptions.m_nDifficultyLevel = 5;

        if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE) {
            m_cOptions.m_nDifficultyMultiplier = 100;
            m_cOptions.m_nMPDifficultyMultiplier = 100;
        }

        CString sDifficultyLevel;
        sDifficultyLevel.Format("%d", m_cOptions.m_nDifficultyLevel);
        WritePrivateProfileStringA("Game Options",
            "Difficulty Level",
            sDifficultyLevel,
            g_pBaldurChitin->GetIniFileName());
    }

    m_bShowAreaNotes = (pHeader->m_dwFlags & 0x80) == 0;
    m_nReputation = static_cast<SHORT>(pHeader->m_reputation);

    if (bProgressBarInPlace) {
        ProgressBarCallback(0, TRUE);

        DWORD nCreatures = pHeader->m_globalCreatureTableCount + pHeader->m_partyCreatureTableCount;

        if (pHeader->m_partyCreatureTableCount > 1) {
            g_pChitin->cProgressBar.AddActionTarget(3000000 * pHeader->m_partyCreatureTableCount - 3000000);
        }

        if (nCreatures > 0) {
            dwPerCreature = 4375000 / nCreatures;
            dwProgress = 156250;
        } else {
            dwPerCreature = 0;
            dwProgress = 1250000;
        }

        ProgressBarCallback(dwProgress, FALSE);
    }

    if (pHeader->m_masterArea[0] != '\0' && bProgressBarInPlace) {
        g_pChitin->cProgressBar.AddActionTarget(3000000);
    }

    g_pBaldurChitin->m_pEngineWorld->m_weather.Unmarshal(pHeader->m_wWeatherFlags);

    // NOTE: The loop counter is never advanced and every iteration reads the
    // first entry of the party creature table, so this only terminates when
    // there are no global creatures. Also in "GAMEV2.2" saves `pCreature21` is
    // never set. IWD2 saves don't have global creatures.
    for (DWORD nCreature = 0; nCreature < pHeader->m_globalCreatureTableCount;) {
        if (nVersion == 20) {
            sub_5A79E0(reinterpret_cast<CSavedGamePartyCreature20*>(pGame + pHeader->m_partyCreatureTableOffset), &creature21);
            pCreature21 = &creature21;
            cnt += sizeof(CSavedGamePartyCreature20);
        } else if (nVersion == 21) {
            pCreature21 = reinterpret_cast<CSavedGamePartyCreature21*>(pGame + pHeader->m_partyCreatureTableOffset);
            sub_5A7BF0(pCreature21, &creature22);
            pCreature22 = &creature22;
            cnt += sizeof(CSavedGamePartyCreature21);
        } else {
            pCreature22 = reinterpret_cast<CSavedGamePartyCreature*>(pGame + pHeader->m_partyCreatureTableOffset);
            cnt += sizeof(CSavedGamePartyCreature);
        }

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 7187
        UTIL_ASSERT(cnt <= (DWORD)nGame);

        if (bProgressBarInPlace) {
            ProgressBarCallback(dwPerCreature, FALSE);

            if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
                pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_RESOURCE,
                    CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND,
                    nMsgFrom,
                    dwSize);
                while (pData != NULL) {
                    pMessage->OnResourceDemanded(nMsgFrom, pData, dwSize);
                    delete pData;

                    pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_RESOURCE,
                        CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND,
                        nMsgFrom,
                        dwSize);
                }

                g_pChitin->m_bDisplayStale = TRUE;
                SleepEx(60, FALSE);
            }
        }

        if (pCreature21->m_creatureResRef[0] != '\0') {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 7227
            UTIL_ASSERT(pCreature21->m_creatureSize == 0);

            // NOTE: Uninline.
            creatureData.SetResRef(CResRef(pCreature21->m_creatureResRef), TRUE, TRUE);

            DWORD nCreatureDataSize = creatureData.GetDataSize();

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 7232
            UTIL_ASSERT_MSG(nCreatureDataSize,
                CString("Creature file not found ") + CString(reinterpret_cast<char*>(pCreature21->m_creatureResRef), RESREF_SIZE).GetBuffer(RESREF_SIZE + 1));

            pSprite = new CGameSprite(creatureData.GetData(),
                nCreatureDataSize,
                0,
                0,
                -1,
                0,
                0,
                INT_MAX,
                CPoint(-1, -1),
                0);

            // NOTE: The original passes a `CString` made from the resref.
            pSprite->SetResRef(CResRef(pCreature21->m_creatureResRef));

            creatureData.ReleaseData();
        } else {
            cnt += pCreature21->m_creatureSize;

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 7242
            UTIL_ASSERT(cnt <= (DWORD)nGame);

            if (pCreature21->m_creatureOffset == 0 || pCreature21->m_creatureSize == 0) {
                continue;
            }

            pSprite = new CGameSprite(pGame + pCreature21->m_creatureOffset,
                pCreature21->m_creatureSize,
                0,
                0,
                -1,
                0,
                0,
                INT_MAX,
                CPoint(-1, -1),
                0);
        }

        if (pSprite == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 7253
            UTIL_ASSERT(FALSE);
            continue;
        }

        pSprite->Unmarshal(pCreature22, FALSE, FALSE);
        pSprite->SetResRef(CResRef(pCreature22->m_creatureResRef));

        m_lstGlobalCreatures.AddTail(reinterpret_cast<int*>(static_cast<INT_PTR>(pSprite->m_id)));

        if (pSprite->GetAIType().m_nEnemyAlly == CAIObjectType::EA_FAMILIAR) {
            // NOTE: Uninline.
            if (GetCharacterPortraitNum(pSprite->m_id) == -1) {
                if (m_allies.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(pSprite->m_id))) == NULL
                    && m_familiars.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(pSprite->m_id))) == NULL) {
                    m_familiars.AddTail(reinterpret_cast<int*>(static_cast<INT_PTR>(pSprite->m_id)));
                }
            }
        }

        if (pHeader->m_versionNumber == 0) {
            if (pSprite->m_baseStats.m_xp >= 89000) {
                pSprite->m_baseStats.m_xp = 89000;
            }
        }

        if (pHeader->m_versionNumber == 1) {
            if (pSprite->m_baseStats.m_xp >= 160000) {
                pSprite->m_baseStats.m_xp = 160000;
            }
        }

        CVariable cVariable;
        cVariable.SetName(CString(pSprite->GetScriptName()));
        cVariable.m_intValue = pSprite->m_id;
        m_namedCreatures.AddKey(cVariable);
    }

    if (pHeader->m_masterArea[0] != '\0') {
        LoadArea(CString(pHeader->m_masterArea), 255, FALSE, bProgressBarInPlace);
    }

    for (DWORD nCharacter = 0; nCharacter < pHeader->m_partyCreatureTableCount; nCharacter++) {
        if (nVersion == 20) {
            sub_5A79E0(reinterpret_cast<CSavedGamePartyCreature20*>(pGame + pHeader->m_partyCreatureTableOffset) + nCharacter, &creature21);
            sub_5A7BF0(&creature21, &creature22);
            pCreature22 = &creature22;
            cnt += sizeof(CSavedGamePartyCreature20);
        } else if (nVersion == 21) {
            sub_5A7BF0(reinterpret_cast<CSavedGamePartyCreature21*>(pGame + pHeader->m_partyCreatureTableOffset) + nCharacter,
                &creature22);
            pCreature22 = &creature22;
            cnt += sizeof(CSavedGamePartyCreature21);
        } else {
            pCreature22 = reinterpret_cast<CSavedGamePartyCreature*>(pGame + pHeader->m_partyCreatureTableOffset) + nCharacter;
            cnt += sizeof(CSavedGamePartyCreature);
        }

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 7331
        UTIL_ASSERT(cnt <= (DWORD)nGame);

        if (bProgressBarInPlace) {
            ProgressBarCallback(dwPerCreature, FALSE);

            if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
                pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_RESOURCE,
                    CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND,
                    nMsgFrom,
                    dwSize);
                while (pData != NULL) {
                    pMessage->OnResourceDemanded(nMsgFrom, pData, dwSize);
                    delete pData;

                    pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_RESOURCE,
                        CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND,
                        nMsgFrom,
                        dwSize);
                }

                g_pChitin->m_bDisplayStale = TRUE;
                SleepEx(60, FALSE);
            }
        }

        m_bPlayerScriptStyle = TRUE;

        if (pCreature22->m_creatureResRef[0] != '\0') {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 7371
            UTIL_ASSERT(pCreature22->m_creatureSize == 0);

            // NOTE: Uninline.
            creatureData.SetResRef(CResRef(pCreature22->m_creatureResRef), TRUE, TRUE);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 7373
            UTIL_ASSERT_MSG(creatureData.GetDataSize(),
                CString("Creature file not found ") + CString(reinterpret_cast<char*>(pCreature22->m_creatureResRef), RESREF_SIZE).GetBuffer(RESREF_SIZE + 1));

            BYTE* pCreatureData = creatureData.GetData();
            pSprite = new CGameSprite(pCreatureData,
                creatureData.GetDataSize(),
                0,
                0,
                -1,
                0,
                0,
                INT_MAX,
                CPoint(-1, -1),
                0);

            // NOTE: The original passes a `CString` made from the resref.
            pSprite->SetResRef(CResRef(pCreature22->m_creatureResRef));

            creatureData.ReleaseData();

            m_bPlayerScriptStyle = FALSE;
        } else {
            cnt += pCreature22->m_creatureSize;

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 7384
            UTIL_ASSERT(cnt <= (DWORD)nGame);

            pSprite = new CGameSprite(pGame + pCreature22->m_creatureOffset,
                pCreature22->m_creatureSize,
                0,
                0,
                -1,
                0,
                0,
                INT_MAX,
                CPoint(-1, -1),
                0);

            m_bPlayerScriptStyle = FALSE;
        }

        if (pSprite == NULL) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 7391
            UTIL_ASSERT(FALSE);
            continue;
        }

        pSprite->SetResRef(CResRef(pCreature22->m_creatureResRef));

        // NOTE: Adds zero.
        if (bProgressBarInPlace && nCharacter >= 1) {
            g_pChitin->cProgressBar.AddActionTarget(0);
        }

        EnablePortrait(static_cast<BYTE>(nCharacter), TRUE);

        pSprite->Unmarshal(pCreature22, TRUE, bProgressBarInPlace);

        if (nCharacter == pHeader->m_nPCAreaViewed && pSprite->m_pArea != NULL) {
            m_visibleArea = pSprite->m_pArea->m_id;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(pSprite->m_id,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);

        if (pHeader->m_versionNumber == 0) {
            if (pSprite->m_baseStats.m_xp >= 89000) {
                pSprite->m_baseStats.m_xp = 89000;
            }
        }

        if (pHeader->m_versionNumber == 1) {
            if (pSprite->m_baseStats.m_xp >= 160000) {
                pSprite->m_baseStats.m_xp = 160000;
            }
        }

        CVariable cVariable;
        cVariable.SetName(CString(pSprite->GetScriptName()));
        cVariable.m_intValue = pSprite->m_id;
        m_namedCreatures.AddKey(cVariable);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pSprite->m_id,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    SelectToolbar();
    SetProtagonist(m_characters[0]);

    if (pHeader->m_versionNumber != 3) {
        ReputationAdjustment(100, TRUE);
    }

    if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE
        && g_pBaldurChitin->field_4A24 != 0) {
        CScreenWorld* pWorldScreen = g_pBaldurChitin->m_pEngineWorld;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 7462
        UTIL_ASSERT(pWorldScreen != NULL);

        CRect rViewSize = pWorldScreen->GetNewViewSize();
        CUIPanel* pDialogPanel = pWorldScreen->GetManager()->GetPanel(1);
        CUIPanel* pPanel = pWorldScreen->GetManager()->GetPanel(pWorldScreen->GetPanel_22_0());

        if ((pHeader->m_dwFlags & 0x20) != 0) {
            pWorldScreen->field_11F0 = FALSE;
            pWorldScreen->field_11F8 = rViewSize.bottom;
            pDialogPanel->m_bActive = FALSE;

            rViewSize.bottom = CInfinity::stru_8E79E0.bottom;
            CInfinity::stru_8E79B8.bottom = rViewSize.bottom;
            pWorldScreen->SetNewViewSize(rViewSize, FALSE);

            pWorldScreen->GetManager()->InvalidateRect(NULL);
        } else if ((pHeader->m_dwFlags & 0x40) != 0) {
            pWorldScreen->field_11FC = rViewSize.bottom;
            pWorldScreen->field_11F4 = FALSE;
            pPanel->m_bActive = FALSE;
            pDialogPanel->m_ptOrigin.x = pDialogPanel->m_ptBaseOrigin.x;
            pDialogPanel->m_ptOrigin.y = pDialogPanel->m_ptBaseOrigin.y + pPanel->m_size.cy;
            pDialogPanel->m_bActive = TRUE;

            rViewSize.bottom = CInfinity::stru_8E79C8.bottom;
            CInfinity::stru_8E79B8.bottom = rViewSize.bottom;
            pWorldScreen->SetNewViewSize(rViewSize, FALSE);

            pWorldScreen->GetManager()->InvalidateRect(NULL);
        }

        if ((pHeader->m_dwFlags & 0x10) != 0) {
            g_pBaldurChitin->m_pEngineWorld->GetManager()->m_bHidden = TRUE;
            g_pBaldurChitin->m_pEngineWorld->field_11D0 = rViewSize;
            g_pBaldurChitin->m_pEngineWorld->SetNewViewSize(CInfinity::stru_8E7548, FALSE);
        }
    }

    cnt += pHeader->m_partyInventoryTableCount * 20;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 7503
    UTIL_ASSERT(cnt <= (DWORD)nGame);

    if (bProgressBarInPlace) {
        ProgressBarCallback(dwProgress, FALSE);
    }

    field_1B84 = 1;

    CVariable cVariable;
    for (DWORD nVariable = 0; nVariable < pHeader->m_globalVariablesCount; nVariable++) {
        if ((nVariable & 0xF) == 0) {
            g_pChitin->m_bDisplayStale = TRUE;
            SleepEx(60, FALSE);
        }

        cVariable = reinterpret_cast<CVariable*>(pGame + pHeader->m_globalVariablesOffset)[nVariable];
        cnt += sizeof(CVariable);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 7525
        UTIL_ASSERT(cnt <= (DWORD)nGame);

        m_variables.AddKey(cVariable);
    }

    field_1B84 = 0;

    if (bProgressBarInPlace) {
        ProgressBarCallback(dwProgress, FALSE);
    }

    m_cJournal.ClearAllEntries();

    if (pHeader->m_journalEntriesCount > 0) {
        m_cJournal.Unmarshal(reinterpret_cast<CSavedGameJournalEntry*>(pGame + pHeader->m_journalEntriesOffset),
            pHeader->m_journalEntriesCount);
        cnt += pHeader->m_journalEntriesCount * sizeof(CSavedGameJournalEntry);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 7596
        UTIL_ASSERT(cnt <= (DWORD)nGame);
    }

    if (pHeader->m_familiarsOffset != 0) {
        CSavedGameFamiliars* pFamiliars = reinterpret_cast<CSavedGameFamiliars*>(pGame + pHeader->m_familiarsOffset);
        memcpy(m_defaultFamiliarResRefs, pFamiliars->m_defaultFamiliarResRefs, sizeof(m_defaultFamiliarResRefs));

        DWORD nFamiliar = 0;
        for (int nAlignment = 0; nAlignment < 9; nAlignment++) {
            for (int nLevel = 0; nLevel < 9; nLevel++) {
                for (DWORD nIndex = 0; nIndex < pFamiliars->m_familiarResRefsCount[nAlignment][nLevel]; nIndex++) {
                    CResRef* pResRef = new CResRef(pGame + pFamiliars->m_familiarResRefsOffset + nFamiliar * RESREF_SIZE);
                    m_familiarResRefs[nAlignment][nLevel].AddTail(pResRef);
                    nFamiliar++;
                }
            }
        }

        cnt += sizeof(CSavedGameFamiliars) + nFamiliar * RESREF_SIZE;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 7615
        UTIL_ASSERT(cnt <= (DWORD)nGame);
    }

    if (sFileVersion != "V1.1") {
        POSITION pos = field_4BDC.GetHeadPosition();
        while (pos != NULL) {
            POSITION posOld = pos;
            CString* node = field_4BDC.GetNext(pos);
            if (node != NULL) {
                delete node;
            }
            field_4BDC.RemoveAt(posOld);
        }

        BYTE* pEntries = pGame + cnt;
        LONG nEntries = *reinterpret_cast<LONG*>(pEntries);
        pEntries += sizeof(LONG);

        for (; nEntries > 0; nEntries--) {
            CInfGameUnknownEntry* pEntry = new CInfGameUnknownEntry();
            if (pEntry != NULL) {
                CSavedGameUnknownEntry* pSavedEntry = reinterpret_cast<CSavedGameUnknownEntry*>(pEntries);
                DWORD nSize = pSavedEntry->m_nSize;
                DWORD nLength = pSavedEntry->m_nLength;

                pEntry->field_4 = pSavedEntry->field_8;
                pEntry->field_8 = pSavedEntry->field_C;
                memcpy(pEntry->field_9, pSavedEntry->field_D, sizeof(pEntry->field_9));

                memcpy(pEntry->m_sName.GetBuffer(nLength),
                    pEntries + sizeof(CSavedGameUnknownEntry),
                    nLength);
                pEntry->m_sName.ReleaseBuffer(nLength);

                field_4BDC.AddTail(&(pEntry->m_sName));

                pEntries += nSize + sizeof(DWORD);
            }
        }

        DWORD nEntriesSize = sizeof(LONG);
        pos = field_4BDC.GetHeadPosition();
        while (pos != NULL) {
            nEntriesSize += sizeof(CSavedGameUnknownEntry) + field_4BDC.GetNext(pos)->GetLength();
        }

        cnt += nEntriesSize;
    }

    if (bProgressBarInPlace) {
        ProgressBarCallback(dwProgress, FALSE);
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 7634
    UTIL_ASSERT(cnt == (DWORD)nGame);
}

// Converts "CRE V2.0" creature to "CRE V2.1" format (rewrites
// `*ppCreature` in place; the size does not change).
//
// NOTE: Uses a register calling convention (`ppCreature` in ECX,
// `pCreatureSize` in EDX).
//
// 0x5A3DB0
static void sub_5A3DB0(BYTE** ppCreature, LONG* pCreatureSize)
{
#define OLD(type, offset) (*reinterpret_cast<type*>(pOld + (offset)))
#define NEW(type, offset) (*reinterpret_cast<type*>(pNew + (offset)))
#define OLD_AT(ptr, type) (*reinterpret_cast<type*>(ptr))
#define HALF(n) (((n) & 0xFE) > 2 ? (n) >> 1 : 1)
#define THIRD(n) ((n) / 3 > 1 ? (n) / 3 : 1)

    BYTE* pOld = *ppCreature;
    BYTE* pNew = new BYTE[*pCreatureSize];
    memset(pNew, 0, *pCreatureSize);

    memcpy(pNew, "CRE V2.1", 8);

    memset(pNew + 8, 0, 0xDF * sizeof(DWORD));

    NEW(DWORD, 0x08) = OLD(DWORD, 0x08);
    NEW(DWORD, 0x0C) = OLD(DWORD, 0x0C);
    NEW(DWORD, 0x10) = OLD(DWORD, 0x10);

    DWORD nXP = OLD(DWORD, 0x14);
    NEW(DWORD, 0x14) = nXP;

    if (nXP > 50000) {
        NEW(BYTE, 0x25A) = 17;
    } else if (nXP > 30000) {
        NEW(BYTE, 0x25A) = 16;
    } else if (nXP > 20000) {
        NEW(BYTE, 0x25A) = 15;
    } else if (nXP > 15000) {
        NEW(BYTE, 0x25A) = 14;
    } else if (nXP > 12000) {
        NEW(BYTE, 0x25A) = 13;
    } else if (nXP > 10000) {
        NEW(BYTE, 0x25A) = 12;
    } else if (nXP > 8000) {
        NEW(BYTE, 0x25A) = 11;
    } else if (nXP > 6000) {
        NEW(BYTE, 0x25A) = 10;
    } else if (nXP > 4000) {
        NEW(BYTE, 0x25A) = 9;
    } else if (nXP > 2000) {
        NEW(BYTE, 0x25A) = 8;
    } else if (nXP > 1400) {
        NEW(BYTE, 0x25A) = 7;
    } else if (nXP > 420) {
        NEW(BYTE, 0x25A) = 6;
    } else if (nXP > 175) {
        NEW(BYTE, 0x25A) = 5;
    } else if (nXP > 120) {
        NEW(BYTE, 0x25A) = 4;
    } else if (nXP > 90) {
        NEW(BYTE, 0x25A) = 3;
    } else if (nXP > 60) {
        NEW(BYTE, 0x25A) = 2;
    } else if (nXP > 0) {
        NEW(BYTE, 0x25A) = 1;
    } else {
        NEW(BYTE, 0x25A) = 0;
    }

    NEW(DWORD, 0x18) = OLD(DWORD, 0x18);
    NEW(DWORD, 0x1C) = OLD(DWORD, 0x1C);
    NEW(DWORD, 0x20) = OLD(DWORD, 0x20);
    NEW(WORD, 0x24) = OLD(WORD, 0x24);
    NEW(WORD, 0x26) = OLD(WORD, 0x26);
    NEW(DWORD, 0x28) = OLD(DWORD, 0x28);
    NEW(DWORD, 0x2C) = OLD(DWORD, 0x2C);
    NEW(WORD, 0x30) = OLD(WORD, 0x30);
    NEW(BYTE, 0x32) = OLD(BYTE, 0x32);
    NEW(BYTE, 0x33) = OLD(BYTE, 0x33);
    NEW(DWORD, 0x34) = OLD(DWORD, 0x34);
    NEW(DWORD, 0x38) = OLD(DWORD, 0x38);
    NEW(DWORD, 0x3C) = OLD(DWORD, 0x3C);
    NEW(DWORD, 0x40) = OLD(DWORD, 0x40);
    NEW(BYTE, 0x44) = OLD(BYTE, 0x44);
    NEW(WORD, 0x46) = OLD(WORD, 0x48);
    NEW(WORD, 0x48) = OLD(WORD, 0x4A);
    NEW(WORD, 0x4A) = OLD(WORD, 0x4C);
    NEW(WORD, 0x4C) = OLD(WORD, 0x4E);
    NEW(WORD, 0x4E) = OLD(WORD, 0x50);
    NEW(BYTE, 0x50) = OLD(BYTE, 0x52);
    NEW(BYTE, 0x51) = OLD(BYTE, 0x53);
    NEW(BYTE, 0x52) = OLD(BYTE, 0x54);
    NEW(BYTE, 0x53) = OLD(BYTE, 0x55);
    NEW(BYTE, 0x54) = OLD(BYTE, 0x56);
    NEW(BYTE, 0x55) = OLD(BYTE, 0x59);
    NEW(BYTE, 0x56) = OLD(BYTE, 0x5A);
    NEW(BYTE, 0x57) = OLD(BYTE, 0x5B);
    NEW(BYTE, 0x58) = OLD(BYTE, 0x5C);
    NEW(BYTE, 0x59) = OLD(char, 0x5D) / 10;
    NEW(BYTE, 0x5A) = OLD(BYTE, 0x5E);
    NEW(BYTE, 0x5B) = OLD(BYTE, 0x5F);
    NEW(BYTE, 0x5C) = OLD(BYTE, 0x60);
    NEW(BYTE, 0x5D) = OLD(BYTE, 0x61);
    NEW(BYTE, 0x5E) = OLD(BYTE, 0x62);
    NEW(BYTE, 0x5F) = OLD(BYTE, 0x63);
    NEW(BYTE, 0x65) = OLD(BYTE, 0x6B);
    NEW(BYTE, 0x66) = OLD(BYTE, 0x6C);
    NEW(BYTE, 0x67) = OLD(BYTE, 0x6D);
    NEW(BYTE, 0x68) = OLD(BYTE, 0x82);
    memcpy(pNew + 0x6A, pOld + 0x84, 8 * sizeof(DWORD));
    memcpy(pNew + 0xAC, pOld + 0xA4, 0x40 * sizeof(DWORD));
    NEW(DWORD, 0x1AC) = OLD(DWORD, 0x1A4);
    NEW(DWORD, 0x1B0) = OLD(DWORD, 0x1A8);
    NEW(DWORD, 0x1B4) = OLD(DWORD, 0x1AC);
    NEW(DWORD, 0x1B8) = OLD(DWORD, 0x1B0);
    NEW(DWORD, 0x1BC) = OLD(DWORD, 0x1B4);
    NEW(BYTE, 0x258) = OLD(BYTE, 0x220);
    NEW(BYTE, 0x259) = OLD(BYTE, 0x221);
    NEW(DWORD, 0x25B) = OLD(DWORD, 0x224);
    NEW(DWORD, 0x25F) = OLD(DWORD, 0x228);
    NEW(BYTE, 0x263) = OLD(BYTE, 0x230);
    NEW(BYTE, 0x265) = OLD(BYTE, 0x237);
    NEW(BYTE, 0x266) = OLD(BYTE, 0x238);
    NEW(BYTE, 0x267) = OLD(BYTE, 0x23A);
    NEW(BYTE, 0x268) = OLD(BYTE, 0x23B);
    NEW(BYTE, 0x269) = OLD(BYTE, 0x23C);
    NEW(BYTE, 0x26A) = OLD(BYTE, 0x23D);
    NEW(BYTE, 0x26B) = OLD(BYTE, 0x23E);
    NEW(BYTE, 0x26C) = OLD(BYTE, 0x23F);
    NEW(BYTE, 0x26D) = OLD(BYTE, 0x240);
    NEW(WORD, 0x26E) = OLD(WORD, 0x242);

    NEW(BYTE, 0x218) = 0;
    NEW(BYTE, 0x219) = 0;
    NEW(BYTE, 0x21A) = 0;
    NEW(BYTE, 0x21B) = 0;
    NEW(BYTE, 0x21C) = 0;
    NEW(BYTE, 0x21D) = 0;
    NEW(BYTE, 0x21F) = 0;
    NEW(BYTE, 0x21E) = OLD(BYTE, 0x45) / 10;
    NEW(BYTE, 0x220) = OLD(BYTE, 0x66);
    NEW(BYTE, 0x221) = OLD(BYTE, 0x68) / 10;
    NEW(BYTE, 0x222) = OLD(BYTE, 0x67) / 10;
    NEW(BYTE, 0x223) = OLD(BYTE, 0x6A) / 10;
    NEW(BYTE, 0x225) = 0;
    NEW(BYTE, 0x224) = OLD(BYTE, 0x69) / 10;
    NEW(BYTE, 0x226) = OLD(BYTE, 0x231) / 10;
    NEW(BYTE, 0x227) = OLD(BYTE, 0x83) / 10;

    NEW(DWORD, 0x270) = 0;
    NEW(DWORD, 0x274) = OLD(DWORD, 0x248);
    NEW(DWORD, 0x278) = OLD(DWORD, 0x24C);
    NEW(DWORD, 0x27C) = OLD(DWORD, 0x250);
    NEW(DWORD, 0x280) = OLD(DWORD, 0x254);
    NEW(DWORD, 0x284) = OLD(DWORD, 0x258);
    NEW(DWORD, 0x288) = OLD(DWORD, 0x25C);
    NEW(DWORD, 0x28C) = OLD(DWORD, 0x260);
    NEW(DWORD, 0x290) = OLD(DWORD, 0x264);
    NEW(DWORD, 0x294) = OLD(DWORD, 0x268);
    NEW(DWORD, 0x298) = OLD(DWORD, 0x26C);
    NEW(BYTE, 0x29C) = OLD(BYTE, 0x270);
    NEW(BYTE, 0x29D) = OLD(BYTE, 0x271);
    NEW(BYTE, 0x29E) = OLD(BYTE, 0x272);
    NEW(DWORD, 0x2A0) = OLD(DWORD, 0x274);
    NEW(DWORD, 0x2A4) = OLD(DWORD, 0x278);
    NEW(WORD, 0x2A8) = OLD(WORD, 0x27C);
    memcpy(pNew + 0x2AA, pOld + 0x27E, 8 * sizeof(DWORD));
    memcpy(pNew + 0x2CA, pOld + 0x29E, 8 * sizeof(DWORD));
    NEW(BYTE, 0x2EA) = OLD(BYTE, 0x2BE);
    NEW(WORD, 0x2EC) = OLD(WORD, 0x2C0);
    NEW(WORD, 0x2EE) = OLD(WORD, 0x2C2);
    NEW(BYTE, 0x2F0) = OLD(BYTE, 0x2C4);
    NEW(BYTE, 0x2F1) = OLD(BYTE, 0x2C5);
    NEW(DWORD, 0x2F2) = OLD(DWORD, 0x2C6);
    NEW(DWORD, 0x2F6) = OLD(DWORD, 0x2CA);
    NEW(DWORD, 0x2FA) = OLD(DWORD, 0x2CE);
    NEW(BYTE, 0x2FE) = OLD(BYTE, 0x2D2);
    NEW(BYTE, 0x2FF) = OLD(BYTE, 0x2D3);
    NEW(BYTE, 0x300) = OLD(BYTE, 0x2D4);
    NEW(BYTE, 0x301) = OLD(BYTE, 0x2D5);
    NEW(BYTE, 0x302) = OLD(BYTE, 0x2D6);
    NEW(BYTE, 0x303) = OLD(BYTE, 0x2D7);

    BYTE* pTable = pNew + 0x384;
    memset(pTable, 0, 0x1A * sizeof(DWORD) + sizeof(WORD));
    pTable[0] = OLD(BYTE, 0x2D8);
    pTable[1] = OLD(BYTE, 0x2D9);
    pTable[2] = OLD(BYTE, 0x2DA);
    *reinterpret_cast<DWORD*>(pTable + 0x32) = 0;

    if (OLD(BYTE, 0x2DB) == 0 || OLD(BYTE, 0x2DB) == 0xFF) {
        OLD(BYTE, 0x2DB) = 2;
    }

    OLD(BYTE, 0x234) = OLD(BYTE, 0x234) > 1 ? OLD(BYTE, 0x234) : 1;

    switch (OLD(BYTE, 0x2DB)) {
    case 1:
    case 202:
        NEW(DWORD, 0x3B6) = 0x400;
        NEW(BYTE, 0x95) = OLD(BYTE, 0x234);
        pTable[3] = 0xB;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        NEW(DWORD, 0x270) = 0x4000;
        break;
    case 2:
    case 203:
        // NOTE: The mask is wider than the 16-bit value, so the test is
        // always false.
        if ((OLD(WORD, 0x246) & 0x40000000) != 0) {
            *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x1;
            NEW(BYTE, 0x8B) = OLD(BYTE, 0x234);
            pTable[3] = 0x1;
            NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        } else {
            *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x10;
            NEW(BYTE, 0x8F) = OLD(BYTE, 0x234);
            pTable[3] = 0x5;
            NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        }
        break;
    case 3:
    case 204:
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x4;
        NEW(BYTE, 0x8D) = OLD(BYTE, 0x234);
        pTable[3] = 0x3;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        break;
    case 4:
    case 205:
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x100;
        NEW(BYTE, 0x93) = OLD(BYTE, 0x234);
        pTable[3] = 0x9;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        break;
    case 5:
    case 206:
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x2;
        NEW(BYTE, 0x8C) = OLD(BYTE, 0x234);
        pTable[3] = 0x2;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        break;
    case 6:
    case 207:
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x40;
        NEW(BYTE, 0x91) = OLD(BYTE, 0x234);
        pTable[3] = 0x7;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        NEW(DWORD, 0x270) = 0x1;
        break;
    case 7:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x410;
        NEW(BYTE, 0x8F) = OLD(BYTE, 0x234);
        pTable[3] = 0x5;
        NEW(BYTE, 0x8F) = OLD(BYTE, 0x235);
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x235);
        break;
    case 8:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x14;
        NEW(BYTE, 0x8F) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x8D) = OLD(BYTE, 0x235);
        pTable[3] = 0x5;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x235);
        break;
    case 9:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x110;
        NEW(BYTE, 0x8F) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x93) = OLD(BYTE, 0x235);
        pTable[3] = 0x5;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x235);
        break;
    case 10:
        OLD(BYTE, 0x234) = THIRD(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = THIRD(OLD(BYTE, 0x235));
        OLD(BYTE, 0x236) = THIRD(OLD(BYTE, 0x236));
        NEW(DWORD, 0x3B6) = 0x510;
        NEW(BYTE, 0x8F) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x95) = OLD(BYTE, 0x235);
        NEW(BYTE, 0x93) = OLD(BYTE, 0x236);
        pTable[3] = 0x5;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x236) + OLD(BYTE, 0x235);
        break;
    case 11:
    case 208:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x8;
        NEW(BYTE, 0x8E) = OLD(BYTE, 0x234);
        pTable[3] = 0x4;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        break;
    case 12:
    case 209:
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x80;
        NEW(BYTE, 0x92) = OLD(BYTE, 0x234);
        pTable[3] = 0x8;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        break;
    case 13:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x500;
        NEW(BYTE, 0x95) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x93) = OLD(BYTE, 0x235);
        pTable[3] = 0xB;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x235);
        break;
    case 14:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        NEW(DWORD, 0x3B6) = 0x404;
        NEW(BYTE, 0x8D) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x95) = OLD(BYTE, 0x235);
        pTable[3] = 0x3;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x235);
        break;
    case 15:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        NEW(DWORD, 0x3B6) = 0x104;
        NEW(BYTE, 0x8D) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x93) = OLD(BYTE, 0x235);
        pTable[3] = 0x3;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x235);
        break;
    case 16:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        NEW(DWORD, 0x3B6) = 0x18;
        NEW(BYTE, 0x8F) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x8E) = OLD(BYTE, 0x235);
        pTable[3] = 0x5;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x235);
        break;
    case 17:
        OLD(BYTE, 0x234) = THIRD(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = THIRD(OLD(BYTE, 0x235));
        OLD(BYTE, 0x236) = THIRD(OLD(BYTE, 0x236));
        NEW(DWORD, 0x3B6) = 0x414;
        NEW(BYTE, 0x8F) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x95) = OLD(BYTE, 0x235);
        NEW(BYTE, 0x8D) = OLD(BYTE, 0x236);
        pTable[3] = 0x5;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x236) + OLD(BYTE, 0x235);
        break;
    case 18:
        OLD(BYTE, 0x234) = HALF(OLD(BYTE, 0x234));
        OLD(BYTE, 0x235) = HALF(OLD(BYTE, 0x235));
        NEW(DWORD, 0x3B6) = 0x84;
        NEW(BYTE, 0x8D) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x92) = OLD(BYTE, 0x235);
        pTable[3] = 0x3;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234) + OLD(BYTE, 0x235);
        break;
    case 19:
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x200;
        NEW(BYTE, 0x94) = OLD(BYTE, 0x234);
        pTable[3] = 0xA;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        break;
    case 20:
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x20;
        NEW(BYTE, 0x90) = OLD(BYTE, 0x234);
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        pTable[3] = 0x6;
        NEW(DWORD, 0x270) = 0x8;
        break;
    default:
        *reinterpret_cast<DWORD*>(pTable + 0x32) = 0x10;
        NEW(BYTE, 0x8F) = OLD(BYTE, 0x234);
        pTable[3] = 0x5;
        NEW(BYTE, 0x8A) = OLD(BYTE, 0x234);
        break;
    }

    NEW(BYTE, 0x3B4) = pNew[0x387];
    NEW(BYTE, 0x388) = OLD(BYTE, 0x2DC);
    NEW(BYTE, 0x389) = OLD(BYTE, 0x2DD);
    NEW(DWORD, 0x38A) = OLD(DWORD, 0x2DE);
    NEW(BYTE, 0x38E) = OLD(BYTE, 0x2E2);
    NEW(BYTE, 0x38F) = OLD(BYTE, 0x2E3);
    NEW(DWORD, 0x390) = OLD(DWORD, 0x2E4);
    memcpy(pNew + 0x394, pOld + 0x2E8, 8 * sizeof(DWORD));
    NEW(DWORD, 0x3BA) = OLD(DWORD, 0x308) + 0xB2;
    NEW(DWORD, 0x3BE) = OLD(DWORD, 0x30C);
    NEW(DWORD, 0x3C2) = OLD(DWORD, 0x310) + 0xB2;
    NEW(DWORD, 0x3C6) = OLD(DWORD, 0x314);
    NEW(DWORD, 0x3CA) = OLD(DWORD, 0x318) + 0xB2;
    NEW(DWORD, 0x3CE) = OLD(DWORD, 0x31C);
    NEW(DWORD, 0x3D6) = OLD(DWORD, 0x324) + 0xB2;
    NEW(DWORD, 0x3DA) = OLD(DWORD, 0x328);
    NEW(DWORD, 0x3D2) = OLD(DWORD, 0x320) + 0xB2;
    NEW(DWORD, 0x3DE) = OLD(DWORD, 0x32C) + 0xB2;
    NEW(DWORD, 0x3E2) = OLD(DWORD, 0x330);
    NEW(DWORD, 0x3E6) = OLD(DWORD, 0x334);
    NEW(DWORD, 0x3EA) = OLD(DWORD, 0x338);

    if (NEW(LONG, 0x3C6) + NEW(LONG, 0x3CE) + NEW(LONG, 0x3BE) > 0) {
        DWORD nSize = OLD(DWORD, 0x30C) * 3 * 4;

        DWORD nCount = OLD(DWORD, 0x314);
        if (nCount != 0) {
            BYTE* pEntry = pOld + OLD(DWORD, 0x310) + 0xC;
            do {
                nSize += OLD_AT(pEntry, DWORD) * 3 * 4;
                pEntry += 0x10;
                nCount--;
            } while (nCount != 0);
        }

        nSize += OLD(DWORD, 0x314) << 4;

        memcpy(pNew + 0x3EE, pOld + 0x33C, nSize);
    }

    BYTE* pDest = pNew + NEW(DWORD, 0x3D2);
    BYTE* pSource = pOld + OLD(DWORD, 0x320);

    memcpy(pNew + NEW(DWORD, 0x3D6), pOld + OLD(DWORD, 0x324), OLD(DWORD, 0x328) * 5 * 4);

    if (NEW(DWORD, 0x3E2) != 0) {
        DWORD nSize;
        if (NEW(BYTE, 0x33) == 0) {
            nSize = NEW(DWORD, 0x3E2) * 3 * 16;
        } else {
            nSize = (NEW(DWORD, 0x3E2) * 33) * 8;
        }

        memcpy(pNew + NEW(DWORD, 0x3DE), pOld + OLD(DWORD, 0x32C), nSize);
    }

    memset(pDest, 0xFF, 0x1A * sizeof(DWORD));

    *reinterpret_cast<WORD*>(pDest + 0x00) = *reinterpret_cast<WORD*>(pSource + 0x00);
    *reinterpret_cast<WORD*>(pDest + 0x02) = *reinterpret_cast<WORD*>(pSource + 0x02);
    *reinterpret_cast<WORD*>(pDest + 0x04) = *reinterpret_cast<WORD*>(pSource + 0x04);
    *reinterpret_cast<WORD*>(pDest + 0x06) = *reinterpret_cast<WORD*>(pSource + 0x06);
    *reinterpret_cast<WORD*>(pDest + 0x08) = *reinterpret_cast<WORD*>(pSource + 0x08);
    *reinterpret_cast<WORD*>(pDest + 0x0A) = *reinterpret_cast<WORD*>(pSource + 0x0A);
    *reinterpret_cast<WORD*>(pDest + 0x0C) = *reinterpret_cast<WORD*>(pSource + 0x0C);
    *reinterpret_cast<WORD*>(pDest + 0x0E) = *reinterpret_cast<WORD*>(pSource + 0x0E);
    *reinterpret_cast<WORD*>(pDest + 0x10) = *reinterpret_cast<WORD*>(pSource + 0x10);
    *reinterpret_cast<DWORD*>(pDest + 0x12) = *reinterpret_cast<DWORD*>(pSource + 0x12);
    *reinterpret_cast<DWORD*>(pDest + 0x16) = *reinterpret_cast<DWORD*>(pSource + 0x16);
    *reinterpret_cast<DWORD*>(pDest + 0x22) = *reinterpret_cast<DWORD*>(pSource + 0x1A);
    *reinterpret_cast<DWORD*>(pDest + 0x26) = *reinterpret_cast<DWORD*>(pSource + 0x1E);
    *reinterpret_cast<WORD*>(pDest + 0x2A) = *reinterpret_cast<WORD*>(pSource + 0x22);
    memcpy(pDest + 0x2C, pSource + 0x24, 0xA * sizeof(DWORD));
    *reinterpret_cast<WORD*>(pDest + 0x64) = *reinterpret_cast<WORD*>(pSource + 0x4C);
    *reinterpret_cast<WORD*>(pDest + 0x66) = *reinterpret_cast<WORD*>(pSource + 0x4E);

    memcpy(*ppCreature, pNew, *pCreatureSize);

    delete[] pNew;

#undef OLD
#undef NEW
#undef HALF
#undef THIRD
#undef OLD_AT
}

// Converts "CRE V2.1" creature to "CRE V2.2" format.
//
// NOTE: Uses a register calling convention (`ppCreature` in ECX,
// `pCreatureSize` in EDX, the rest on stack).
//
// 0x5A5060
static BOOL sub_5A5060(BYTE** ppCreature, LONG* pCreatureSize, BYTE* pOldCreature, LONG* pOldCreatureSize)
{
#define OLD(type, offset) (*reinterpret_cast<type*>(pOld + (offset)))
#define NEW(type, offset) (*reinterpret_cast<type*>(pNew + (offset)))

    // NOTE: Not original. Layouts of the "CRE V2.1" tables read below.
    struct OldKnownSpell {
        RESREF m_spellId;
        WORD m_spellLevel;
        WORD m_spellType;
    };

    struct OldMemorizationInfo {
        WORD m_spellLevel;
        WORD field_2;
        WORD field_4;
        WORD m_spellType;
        DWORD m_index;
        DWORD m_count;
    };

    struct OldMemorizedSpell {
        RESREF m_spellId;
        DWORD m_flags;
    };

    BYTE* pOld = pOldCreature != NULL ? pOldCreature : *ppCreature;

    BYTE& nClass = pOld[0x387];
    if (nClass == 0 || nClass == 0xFF) {
        nClass = CAIOBJECTTYPE_C_FIGHTER;
    }

    UINT nClassIndex = 0;
    if (nClass == CAIOBJECTTYPE_C_BARD
        || nClass == CAIOBJECTTYPE_C_CLERIC
        || nClass == CAIOBJECTTYPE_C_DRUID
        || nClass == CAIOBJECTTYPE_C_PALADIN
        || nClass == CAIOBJECTTYPE_C_RANGER
        || nClass == CAIOBJECTTYPE_C_SORCERER
        || nClass == CAIOBJECTTYPE_C_WIZARD) {
        nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);
    }

    CGameSpriteSpells spells;
    CGameSpriteGroupedSpellList domainSpells;
    CGameSpriteSpellList innateSpells;
    CGameSpriteSpellList songs;
    CGameSpriteSpellList shapeshifts;

    // NOTE: Original reads spell type from a constant at 0x85BB63 (2).
    const WORD SPELLTYPE_INNATE = 2;

    DWORD nOffset = 0;
    for (DWORD nSpell = 0; nSpell < OLD(DWORD, 0x3BE); nSpell++) {
        OldKnownSpell* tempOldSpell = reinterpret_cast<OldKnownSpell*>(pOld + OLD(DWORD, 0x3BA) + nOffset);

        // FIXME: `m_spellLevel >= 0` is always true.
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 5905
        UTIL_ASSERT(tempOldSpell->m_spellLevel >= 0 && tempOldSpell->m_spellLevel < CSPELLLIST_MAX_LEVELS);

        if (tempOldSpell->m_spellType == SPELLTYPE_INNATE) {
            UINT nID = 0;
            CResRef resRef(tempOldSpell->m_spellId);

            // NOTE: Uninline.
            if (g_pBaldurChitin->GetObjectGame()->m_songs.Find(resRef, nID)) {
                songs.Add(nID, 1, 1, 0);
            } else {
                nID = 0;
                CResRef resRef2(tempOldSpell->m_spellId);

                // NOTE: Uninline.
                if (g_pBaldurChitin->GetObjectGame()->m_shapeshifts.Find(resRef2, nID)) {
                    shapeshifts.Add(nID, 1, 1, 0);
                } else {
                    nID = 0;
                    CResRef resRef3(tempOldSpell->m_spellId);

                    // NOTE: Uninline.
                    if (g_pBaldurChitin->GetObjectGame()->m_innateSpells.Find(resRef3, nID)) {
                        innateSpells.Add(nID, 1, 1, 0);
                    } else {
                        // NOTE: Uninline.
                        char szResRef[RESREF_SIZE + 1];
                        memcpy(szResRef, tempOldSpell->m_spellId, RESREF_SIZE);
                        szResRef[RESREF_SIZE] = '\0';

                        CString sError("");
                        sError.Format("*** OBSOLETE SPELL FILE: %s", szResRef);
                    }
                }
            }
        } else {
            UINT nID = 0;
            CResRef resRef(tempOldSpell->m_spellId);

            // NOTE: Uninline.
            if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID)) {
                UINT nLevel = tempOldSpell->m_spellLevel;

                // NOTE: Uninline.
                spells.Get(nClassIndex)->Add(nID, nLevel, 1, 1, 0);
            } else {
                // NOTE: Uninline.
                char szResRef[RESREF_SIZE + 1];
                memcpy(szResRef, tempOldSpell->m_spellId, RESREF_SIZE);
                szResRef[RESREF_SIZE] = '\0';

                CString sError("");
                sError.Format("*** OBSOLETE SPELL FILE: %s", szResRef);
            }
        }

        nOffset += sizeof(OldKnownSpell);
    }

    DWORD nOldSize = OLD(DWORD, 0x3BE) * sizeof(OldKnownSpell);

    DWORD nInfoOffset = 0;
    for (DWORD nInfo = 0; nInfo < OLD(DWORD, 0x3C6); nInfo++) {
        OldMemorizationInfo* pInfo = reinterpret_cast<OldMemorizationInfo*>(pOld + OLD(DWORD, 0x3C2) + nInfoOffset);

        for (DWORD nMemorized = 0; nMemorized < pInfo->m_count; nMemorized++) {
            OldMemorizedSpell* pSpell = reinterpret_cast<OldMemorizedSpell*>(pOld + (pInfo->m_index + nMemorized) * sizeof(OldMemorizedSpell) + OLD(DWORD, 0x3CA));

            if (pInfo->m_spellType == SPELLTYPE_INNATE) {
                UINT nID = 0;
                CResRef resRef(pSpell->m_spellId);

                // NOTE: Uninline.
                if (g_pBaldurChitin->GetObjectGame()->m_songs.Find(resRef, nID)) {
                    UINT nIndex = 0;
                    if (songs.Find(nID, nIndex) == TRUE) {
                        UINT nCount = 0;
                        if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                            nCount = 1;
                        }

                        songs.Add(nID, 1, nCount, 0);
                    }
                } else {
                    nID = 0;
                    CResRef resRef2(pSpell->m_spellId);

                    // NOTE: Uninline.
                    if (g_pBaldurChitin->GetObjectGame()->m_shapeshifts.Find(resRef2, nID)) {
                        UINT nIndex = 0;
                        if (shapeshifts.Find(nID, nIndex) == TRUE) {
                            UINT nCount = 0;
                            if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                                nCount = 1;
                            }

                            shapeshifts.Add(nID, 1, nCount, 0);
                        }
                    } else {
                        nID = 0;
                        CResRef resRef3(pSpell->m_spellId);

                        // NOTE: Uninline.
                        if (g_pBaldurChitin->GetObjectGame()->m_innateSpells.Find(resRef3, nID)) {
                            UINT nIndex = 0;
                            if (innateSpells.Find(nID, nIndex) == TRUE) {
                                UINT nCount = 0;
                                if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                                    nCount = 1;
                                }

                                innateSpells.Add(nID, 1, nCount, 0);
                            }
                        } else {
                            // NOTE: Uninline.
                            char szResRef[RESREF_SIZE + 1];
                            memcpy(szResRef, pSpell->m_spellId, RESREF_SIZE);
                            szResRef[RESREF_SIZE] = '\0';

                            CString sError("");
                            sError.Format("*** OBSOLETE SPELL FILE: %s", szResRef);
                        }
                    }
                }
            } else {
                UINT nID = 0;
                CResRef resRef(pSpell->m_spellId);

                // NOTE: Uninline.
                if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID)) {
                    UINT nLevel = pInfo->m_spellLevel;
                    UINT nIndex = 0;

                    // NOTE: Uninline.
                    if (spells.Get(nClassIndex)->GetSpellsAtLevel(nLevel)->Find(nID, nIndex) == TRUE) {
                        UINT nCount = 0;
                        if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                            nCount = 1;
                        }

                        UINT nLevelAdd = pInfo->m_spellLevel;

                        // NOTE: Uninline.
                        spells.Get(nClassIndex)->Add(nID, nLevelAdd, 1, nCount, 0);
                    }

                    UINT nDomainLevel = pInfo->m_spellLevel;

                    // NOTE: Uninline.
                    if (domainSpells.GetSpellsAtLevel(nDomainLevel)->Find(nID, nIndex) == TRUE) {
                        UINT nCount = 0;
                        if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                            nCount = 1;
                        }

                        UINT nLevelAdd = pInfo->m_spellLevel;
                        domainSpells.Add(nID, nLevelAdd, 1, nCount, 0);
                    }
                } else {
                    // NOTE: Uninline.
                    char szResRef[RESREF_SIZE + 1];
                    memcpy(szResRef, pSpell->m_spellId, RESREF_SIZE);
                    szResRef[RESREF_SIZE] = '\0';

                    CString sError("");
                    sError.Format("*** OBSOLETE SPELL FILE: %s", szResRef);
                }
            }
        }

        nOldSize += pInfo->m_count * sizeof(OldMemorizedSpell);
        nInfoOffset += sizeof(OldMemorizationInfo);
    }

    nOldSize += OLD(DWORD, 0x3C6) * sizeof(OldMemorizationInfo);

    DWORD nNewSize = 0;

    for (UINT nClassLoop = 0; nClassLoop < CSPELLLIST_NUM_CLASSES; nClassLoop++) {
        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            // NOTE: Uninline.
            nNewSize += spells.Get(nClassLoop)->GetSpellsAtLevel(nLevel)->m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;
        }
    }

    for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
        // NOTE: Uninline.
        nNewSize += domainSpells.GetSpellsAtLevel(nLevel)->m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;
    }

    nNewSize += innateSpells.m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;
    nNewSize += songs.m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;
    nNewSize += shapeshifts.m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;

    DWORD nDelta = nNewSize - nOldSize;

    BYTE* pNew = new BYTE[*pCreatureSize];
    memset(pNew, 0, *pCreatureSize);

    memcpy(pNew, "CRE V2.2", 8);
    memcpy(pNew + 8, pOld + 8, 0x37C);

    // NOTE: Inlined `CCreatureFileOffsets` constructor.
    CCreatureFileOffsets* pOffsets = reinterpret_cast<CCreatureFileOffsets*>(pNew + 0x384);
    memset(pOffsets, 0, sizeof(CCreatureFileOffsets));

    pOffsets->m_enemyAlly = OLD(BYTE, 0x384);
    pOffsets->m_general = OLD(BYTE, 0x385);
    pOffsets->m_race = OLD(BYTE, 0x386);
    pOffsets->m_classMask = 0;

    // NOTE: Different default than above (2, not `CAIOBJECTTYPE_C_FIGHTER`).
    if (OLD(BYTE, 0x387) == 0 || OLD(BYTE, 0x387) == 0xFF) {
        OLD(BYTE, 0x387) = 2;
    }

    pOffsets->m_class = OLD(BYTE, 0x387);
    pOffsets->m_specifics = OLD(BYTE, 0x388);
    pOffsets->m_gender = OLD(BYTE, 0x389);
    *reinterpret_cast<DWORD*>(&(pOffsets->m_specialCase[0])) = OLD(DWORD, 0x38A);
    pOffsets->m_specialCase[4] = OLD(BYTE, 0x38E);
    pOffsets->m_alignment = OLD(BYTE, 0x38F);
    pOffsets->m_instance = OLD(LONG, 0x390);
    memcpy(pOffsets->m_name, pOld + 0x394, 8 * sizeof(DWORD));
    pOffsets->m_avClass = OLD(BYTE, 0x3B4);
    pOffsets->m_classMask = OLD(DWORD, 0x3B6);

    // NOTE: 900 is "CRE V2.2" signature followed by `CCreatureFileHeader`
    // (0x37C bytes in the original).
    DWORD nSpellOffset = 900 + sizeof(CCreatureFileOffsets);

    for (UINT nClassLoop = 0; nClassLoop < CSPELLLIST_NUM_CLASSES; nClassLoop++) {
        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            pOffsets->m_spellListOffset[nClassLoop][nLevel] = nSpellOffset;

            // NOTE: Uninline.
            UINT nCount = spells.Get(nClassLoop)->GetSpellsAtLevel(nLevel)->m_List.size();
            pOffsets->m_spellListCount[nClassLoop][nLevel] = nCount;

            nSpellOffset += nCount * sizeof(CCreatureFileSpell) + 8;
        }
    }

    for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
        pOffsets->m_domainListOffset[nLevel] = nSpellOffset;

        // NOTE: Uninline.
        UINT nCount = domainSpells.GetSpellsAtLevel(nLevel)->m_List.size();
        pOffsets->m_domainListCount[nLevel] = nCount;

        nSpellOffset += nCount * sizeof(CCreatureFileSpell) + 8;
    }

    pOffsets->m_innateListOffset = nSpellOffset;
    pOffsets->m_innateListCount = innateSpells.m_List.size();
    nSpellOffset += innateSpells.m_List.size() * sizeof(CCreatureFileSpell) + 8;

    pOffsets->m_songListOffset = nSpellOffset;
    pOffsets->m_songListCount = songs.m_List.size();
    nSpellOffset += songs.m_List.size() * sizeof(CCreatureFileSpell) + 8;

    pOffsets->m_shapeListOffset = nSpellOffset;
    pOffsets->m_shapeListCount = shapeshifts.m_List.size();
    nSpellOffset += shapeshifts.m_List.size() * sizeof(CCreatureFileSpell) + 8;

    pOffsets->m_equipmentListOffset = OLD(DWORD, 0x3D2) + nDelta + 0x240;
    pOffsets->m_itemListOffset = OLD(DWORD, 0x3D6) + nDelta + 0x240;
    pOffsets->m_itemListCount = OLD(DWORD, 0x3DA);
    pOffsets->m_effectListOffset = OLD(DWORD, 0x3DE) + nDelta + 0x240;
    pOffsets->m_effectListCount = OLD(DWORD, 0x3E2);
    *reinterpret_cast<DWORD*>(&(pOffsets->m_dialog[0])) = OLD(DWORD, 0x3E6);
    *reinterpret_cast<DWORD*>(&(pOffsets->m_dialog[4])) = OLD(DWORD, 0x3EA);

    for (UINT nClassLoop = 0; nClassLoop < CSPELLLIST_NUM_CLASSES; nClassLoop++) {
        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            DWORD nCursor = pOffsets->m_spellListOffset[nClassLoop][nLevel];

            // NOTE: Uninline.
            UINT nCount = spells.Get(nClassLoop)->GetSpellsAtLevel(nLevel)->m_List.size();
            if (nCount != 0) {
                DWORD nSize = nCount * sizeof(CCreatureFileSpell);
                CCreatureFileSpell* pTemp = new CCreatureFileSpell[nCount];

                for (UINT nIndex = 0; nIndex < nCount; nIndex++) {
                    // NOTE: Uninline.
                    *reinterpret_cast<CGameSpriteSpellListEntry*>(&(pTemp[nIndex])) = *spells.Get(nClassLoop)->GetSpellsAtLevel(nLevel)->Get(nIndex);
                }

                memcpy(pNew + nCursor, pTemp, nSize);
                nCursor += nSize;
                delete[] pTemp;
            }

            // NOTE: Uninline.
            *reinterpret_cast<unsigned int*>(pNew + nCursor) = spells.Get(nClassLoop)->GetSpellsAtLevel(nLevel)->field_14;
            nCursor += sizeof(unsigned int);

            // NOTE: Uninline.
            *reinterpret_cast<unsigned int*>(pNew + nCursor) = spells.Get(nClassLoop)->GetSpellsAtLevel(nLevel)->field_18;
        }
    }

    for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
        DWORD nCursor = pOffsets->m_domainListOffset[nLevel];

        // NOTE: Uninline.
        UINT nCount = domainSpells.GetSpellsAtLevel(nLevel)->m_List.size();
        if (nCount != 0) {
            DWORD nSize = nCount * sizeof(CCreatureFileSpell);
            CCreatureFileSpell* pTemp = new CCreatureFileSpell[nCount];

            for (UINT nIndex = 0; nIndex < nCount; nIndex++) {
                // NOTE: Uninline.
                *reinterpret_cast<CGameSpriteSpellListEntry*>(&(pTemp[nIndex])) = *domainSpells.GetSpellsAtLevel(nLevel)->Get(nIndex);
            }

            memcpy(pNew + nCursor, pTemp, nSize);
            nCursor += nSize;
            delete[] pTemp;
        }

        // NOTE: Uninline.
        *reinterpret_cast<unsigned int*>(pNew + nCursor) = domainSpells.GetSpellsAtLevel(nLevel)->field_14;
        nCursor += sizeof(unsigned int);

        // NOTE: Uninline.
        *reinterpret_cast<unsigned int*>(pNew + nCursor) = domainSpells.GetSpellsAtLevel(nLevel)->field_18;
    }

    DWORD nCursor = pOffsets->m_innateListOffset;
    UINT nCount = innateSpells.m_List.size();
    if (nCount != 0) {
        DWORD nSize = nCount * sizeof(CCreatureFileSpell);
        CCreatureFileSpell* pTemp = new CCreatureFileSpell[nCount];

        for (UINT nIndex = 0; nIndex < nCount; nIndex++) {
            *reinterpret_cast<CGameSpriteSpellListEntry*>(&(pTemp[nIndex])) = *innateSpells.Get(nIndex);
        }

        memcpy(pNew + nCursor, pTemp, nSize);
        nCursor += nSize;
        delete[] pTemp;
    }

    *reinterpret_cast<unsigned int*>(pNew + nCursor) = innateSpells.field_14;
    nCursor += sizeof(unsigned int);
    *reinterpret_cast<unsigned int*>(pNew + nCursor) = innateSpells.field_18;

    nCursor = pOffsets->m_songListOffset;
    nCount = songs.m_List.size();
    if (nCount != 0) {
        DWORD nSize = nCount * sizeof(CCreatureFileSpell);
        CCreatureFileSpell* pTemp = new CCreatureFileSpell[nCount];

        for (UINT nIndex = 0; nIndex < nCount; nIndex++) {
            *reinterpret_cast<CGameSpriteSpellListEntry*>(&(pTemp[nIndex])) = *songs.Get(nIndex);
        }

        memcpy(pNew + nCursor, pTemp, nSize);
        nCursor += nSize;
        delete[] pTemp;
    }

    *reinterpret_cast<unsigned int*>(pNew + nCursor) = songs.field_14;
    nCursor += sizeof(unsigned int);
    *reinterpret_cast<unsigned int*>(pNew + nCursor) = songs.field_18;

    nCursor = pOffsets->m_shapeListOffset;
    nCount = shapeshifts.m_List.size();
    if (nCount != 0) {
        DWORD nSize = nCount * sizeof(CCreatureFileSpell);
        CCreatureFileSpell* pTemp = new CCreatureFileSpell[nCount];

        for (UINT nIndex = 0; nIndex < nCount; nIndex++) {
            *reinterpret_cast<CGameSpriteSpellListEntry*>(&(pTemp[nIndex])) = *shapeshifts.Get(nIndex);
        }

        memcpy(pNew + nCursor, pTemp, nSize);
        nCursor += nSize;
        delete[] pTemp;
    }

    *reinterpret_cast<unsigned int*>(pNew + nCursor) = shapeshifts.field_14;
    nCursor += sizeof(unsigned int);
    *reinterpret_cast<unsigned int*>(pNew + nCursor) = shapeshifts.field_18;

    DWORD nLastBlockSize = OLD(DWORD, 0x3DA) * 5 * 4 + 0x68;
    if (NEW(BYTE, 0x33) == 0) {
        nLastBlockSize += OLD(DWORD, 0x3E2) * 3 * 16;
    } else {
        nLastBlockSize += (OLD(DWORD, 0x3E2) * 33) * 8;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 6423
    UTIL_ASSERT((nSpellOffset + nLastBlockSize) == (*pCreatureSize));

    memcpy(pNew + nSpellOffset, pOld + nOldSize + 0x3EE, nLastBlockSize);

    memcpy(*ppCreature, pNew, *pCreatureSize);

    delete[] pNew;

#undef OLD
#undef NEW

    return TRUE;
}

// Calculates how much "CRE V2.1" creature grows when converted to
// "CRE V2.2" format (minus 0x240).
//
// 0x5A69B0
INT CInfGame::sub_5A69B0(BYTE* pCreature)
{
    // NOTE: Not original. Layouts of the "CRE V2.1" tables read below.
    struct OldKnownSpell {
        RESREF m_spellId;
        WORD m_spellLevel;
        WORD m_spellType;
    };

    struct OldMemorizationInfo {
        WORD m_spellLevel;
        WORD field_2;
        WORD field_4;
        WORD m_spellType;
        DWORD m_index;
        DWORD m_count;
    };

    struct OldMemorizedSpell {
        RESREF m_spellId;
        DWORD m_flags;
    };

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 6440
    UTIL_ASSERT(pCreature != NULL);

    if (memcmp(pCreature, "CRE V2.0", 8) != 0
        && memcmp(pCreature, "CRE V2.1", 8) != 0) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 6446
        UTIL_ASSERT(FALSE);
        return 0;
    }

    BYTE& nClass = pCreature[0x387];
    if (nClass == 0 || nClass == 0xFF) {
        nClass = CAIOBJECTTYPE_C_FIGHTER;
    }

    UINT nClassIndex = 0;
    if (nClass == CAIOBJECTTYPE_C_BARD
        || nClass == CAIOBJECTTYPE_C_CLERIC
        || nClass == CAIOBJECTTYPE_C_DRUID
        || nClass == CAIOBJECTTYPE_C_PALADIN
        || nClass == CAIOBJECTTYPE_C_RANGER
        || nClass == CAIOBJECTTYPE_C_SORCERER
        || nClass == CAIOBJECTTYPE_C_WIZARD) {
        nClassIndex = g_pBaldurChitin->GetObjectGame()->GetSpellcasterIndex(nClass);
    }

    CGameSpriteSpells spells;
    CGameSpriteGroupedSpellList domainSpells;
    CGameSpriteSpellList innateSpells;
    CGameSpriteSpellList songs;
    CGameSpriteSpellList shapeshifts;

    // NOTE: Original reads spell type from a constant at 0x85BB63 (2).
    const WORD SPELLTYPE_INNATE = 2;

    DWORD nOffset = 0;
    for (DWORD nSpell = 0; nSpell < *reinterpret_cast<DWORD*>(pCreature + 0x3BE); nSpell++) {
        OldKnownSpell* tempOldSpell = reinterpret_cast<OldKnownSpell*>(pCreature + *reinterpret_cast<DWORD*>(pCreature + 0x3BA) + nOffset);

        // FIXME: `m_spellLevel >= 0` is always true.
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 6486
        UTIL_ASSERT(tempOldSpell->m_spellLevel >= 0 && tempOldSpell->m_spellLevel < CSPELLLIST_MAX_LEVELS);

        if (tempOldSpell->m_spellType == SPELLTYPE_INNATE) {
            UINT nID = 0;
            CResRef resRef(tempOldSpell->m_spellId);

            // NOTE: Uninline.
            if (g_pBaldurChitin->GetObjectGame()->m_songs.Find(resRef, nID)) {
                songs.Add(nID, 1, 1, 0);
            } else {
                nID = 0;
                CResRef resRef2(tempOldSpell->m_spellId);

                // NOTE: Uninline.
                if (g_pBaldurChitin->GetObjectGame()->m_shapeshifts.Find(resRef2, nID)) {
                    shapeshifts.Add(nID, 1, 1, 0);
                } else {
                    nID = 0;
                    CResRef resRef3(tempOldSpell->m_spellId);

                    // NOTE: Uninline.
                    if (g_pBaldurChitin->GetObjectGame()->m_innateSpells.Find(resRef3, nID)) {
                        innateSpells.Add(nID, 1, 1, 0);
                    } else {
                        // NOTE: Uninline.
                        char szResRef[RESREF_SIZE + 1];
                        memcpy(szResRef, tempOldSpell->m_spellId, RESREF_SIZE);
                        szResRef[RESREF_SIZE] = '\0';

                        CString sError("");
                        sError.Format("*** OBSOLETE SPELL FILE: %s", szResRef);
                    }
                }
            }
        } else {
            UINT nID = 0;
            CResRef resRef(tempOldSpell->m_spellId);

            // NOTE: Uninline.
            if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID)) {
                UINT nLevel = tempOldSpell->m_spellLevel;

                // NOTE: Uninline.
                spells.Get(nClassIndex)->Add(nID, nLevel, 1, 1, 0);
            } else {
                // NOTE: Uninline.
                char szResRef[RESREF_SIZE + 1];
                memcpy(szResRef, tempOldSpell->m_spellId, RESREF_SIZE);
                szResRef[RESREF_SIZE] = '\0';

                CString sError("");
                sError.Format("*** OBSOLETE SPELL FILE: %s", szResRef);
            }
        }

        nOffset += sizeof(OldKnownSpell);
    }

    DWORD nOldSize = *reinterpret_cast<DWORD*>(pCreature + 0x3BE) * sizeof(OldKnownSpell);

    DWORD nInfoOffset = 0;
    for (DWORD nInfo = 0; nInfo < *reinterpret_cast<DWORD*>(pCreature + 0x3C6); nInfo++) {
        OldMemorizationInfo* pInfo = reinterpret_cast<OldMemorizationInfo*>(pCreature + *reinterpret_cast<DWORD*>(pCreature + 0x3C2) + nInfoOffset);

        for (DWORD nMemorized = 0; nMemorized < pInfo->m_count; nMemorized++) {
            OldMemorizedSpell* pSpell = reinterpret_cast<OldMemorizedSpell*>(pCreature + (pInfo->m_index + nMemorized) * sizeof(OldMemorizedSpell) + *reinterpret_cast<DWORD*>(pCreature + 0x3CA));

            if (pInfo->m_spellType == SPELLTYPE_INNATE) {
                UINT nID = 0;
                CResRef resRef(pSpell->m_spellId);

                // NOTE: Uninline.
                if (g_pBaldurChitin->GetObjectGame()->m_songs.Find(resRef, nID)) {
                    UINT nIndex = 0;
                    if (songs.Find(nID, nIndex) == TRUE) {
                        UINT nCount = 0;
                        if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                            nCount = 1;
                        }

                        songs.Add(nID, 1, nCount, 0);
                    }
                } else {
                    nID = 0;
                    CResRef resRef2(pSpell->m_spellId);

                    // NOTE: Uninline.
                    if (g_pBaldurChitin->GetObjectGame()->m_shapeshifts.Find(resRef2, nID)) {
                        UINT nIndex = 0;
                        if (shapeshifts.Find(nID, nIndex) == TRUE) {
                            UINT nCount = 0;
                            if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                                nCount = 1;
                            }

                            shapeshifts.Add(nID, 1, nCount, 0);
                        }
                    } else {
                        nID = 0;
                        CResRef resRef3(pSpell->m_spellId);

                        // NOTE: Uninline.
                        if (g_pBaldurChitin->GetObjectGame()->m_innateSpells.Find(resRef3, nID)) {
                            UINT nIndex = 0;
                            if (innateSpells.Find(nID, nIndex) == TRUE) {
                                UINT nCount = 0;
                                if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                                    nCount = 1;
                                }

                                innateSpells.Add(nID, 1, nCount, 0);
                            }
                        } else {
                            // NOTE: Uninline.
                            char szResRef[RESREF_SIZE + 1];
                            memcpy(szResRef, pSpell->m_spellId, RESREF_SIZE);
                            szResRef[RESREF_SIZE] = '\0';

                            CString sError("");
                            sError.Format("*** OBSOLETE SPELL FILE: %s", szResRef);
                        }
                    }
                }
            } else {
                UINT nID = 0;
                CResRef resRef(pSpell->m_spellId);

                // NOTE: Uninline.
                if (g_pBaldurChitin->GetObjectGame()->m_spells.Find(resRef, nID)) {
                    UINT nLevel = pInfo->m_spellLevel;
                    UINT nIndex = 0;

                    // NOTE: Uninline.
                    if (spells.Get(nClassIndex)->GetSpellsAtLevel(nLevel)->Find(nID, nIndex) == TRUE) {
                        UINT nCount = 0;
                        if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                            nCount = 1;
                        }

                        UINT nLevelAdd = pInfo->m_spellLevel;

                        // NOTE: Uninline.
                        spells.Get(nClassIndex)->Add(nID, nLevelAdd, 1, nCount, 0);
                    }

                    UINT nDomainLevel = pInfo->m_spellLevel;

                    // NOTE: Uninline.
                    if (domainSpells.GetSpellsAtLevel(nDomainLevel)->Find(nID, nIndex) == TRUE) {
                        UINT nCount = 0;
                        if ((static_cast<BYTE>(pSpell->m_flags) & 1) != 0) {
                            nCount = 1;
                        }

                        UINT nLevelAdd = pInfo->m_spellLevel;
                        domainSpells.Add(nID, nLevelAdd, 1, nCount, 0);
                    }
                } else {
                    // NOTE: Uninline.
                    char szResRef[RESREF_SIZE + 1];
                    memcpy(szResRef, pSpell->m_spellId, RESREF_SIZE);
                    szResRef[RESREF_SIZE] = '\0';

                    CString sError("");
                    sError.Format("*** OBSOLETE SPELL FILE: %s", szResRef);
                }
            }
        }

        nOldSize += pInfo->m_count * sizeof(OldMemorizedSpell);
        nInfoOffset += sizeof(OldMemorizationInfo);
    }

    nOldSize += *reinterpret_cast<DWORD*>(pCreature + 0x3C6) * sizeof(OldMemorizationInfo);

    DWORD nNewSize = 0;

    for (UINT nClassLoop = 0; nClassLoop < CSPELLLIST_NUM_CLASSES; nClassLoop++) {
        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            // NOTE: Uninline.
            nNewSize += spells.Get(nClassLoop)->GetSpellsAtLevel(nLevel)->m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;
        }
    }

    for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
        // NOTE: Uninline.
        nNewSize += domainSpells.GetSpellsAtLevel(nLevel)->m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;
    }

    nNewSize += innateSpells.m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;
    nNewSize += songs.m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;
    nNewSize += shapeshifts.m_List.size() * sizeof(CGameSpriteSpellListEntry) + 8;

    return nNewSize - nOldSize;
}

// 0x5A7740
void CInfGame::sub_5A7740(BYTE** ppCreature, LONG* pCreatureSize)
{
    BYTE* pCreature = *ppCreature;

    if (memcmp(pCreature, "CRE V2.1", 8) == 0) {
        return;
    }

    if (memcmp(pCreature, "CRE V9.2", 8) != 0
        && memcmp(pCreature, "CRE V2.0", 8) != 0) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 6768
        UTIL_ASSERT_MSG(FALSE, "Corrupt or Obsolete creature data was encountered during load! Aborting");
    }

    pCreature = *ppCreature;
    if (memcmp(pCreature, "CRE V9.2", 8) == 0) {
        if (pCreature[0x53] > 5) {
            pCreature[0x53] -= 6;
        }

        pCreature[0x220] = 0;
        pCreature[0x221] = 0;

        DWORD nXP = *reinterpret_cast<DWORD*>(pCreature + 0x14);
        if (nXP > 50000) {
            pCreature[0x222] = 15;
        } else if (nXP > 30000) {
            pCreature[0x222] = 14;
        } else if (nXP > 20000) {
            pCreature[0x222] = 13;
        } else if (nXP > 15000) {
            pCreature[0x222] = 12;
        } else if (nXP > 12000) {
            pCreature[0x222] = 11;
        } else if (nXP > 10000) {
            pCreature[0x222] = 10;
        } else if (nXP > 8000) {
            pCreature[0x222] = 9;
        } else if (nXP > 6000) {
            pCreature[0x222] = 8;
        } else if (nXP > 4000) {
            pCreature[0x222] = 7;
        } else if (nXP > 2000) {
            pCreature[0x222] = 6;
        } else if (nXP > 1400) {
            pCreature[0x222] = 5;
        } else if (nXP > 420) {
            pCreature[0x222] = 4;
        } else if (nXP > 175) {
            pCreature[0x222] = 3;
        } else if (nXP > 120) {
            pCreature[0x222] = 2;
        } else {
            pCreature[0x222] = 1;
        }

        pCreature[0x52] = 20 - pCreature[0x52];
        *reinterpret_cast<WORD*>(pCreature + 0x48) = 20 - *reinterpret_cast<WORD*>(pCreature + 0x48);

        if (pCreature[0x2DB] == 0xFF) {
            pCreature[0x2DB] = 2;
        }

        memcpy(*ppCreature, "CRE V2.0", 8);
    }

    if (memcmp(*ppCreature, "CRE V2.0", 8) == 0) {
        sub_5A3DB0(ppCreature, pCreatureSize);
    }
}

// 0x5A7940
void CInfGame::sub_5A7940(BYTE** ppCreature, LONG* pCreatureSize, BYTE* pOldCreature, LONG* pOldCreatureSize)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 6876
    UTIL_ASSERT(pOldCreature != NULL);

    if (memcmp(pOldCreature, "CRE V2.2", 8) == 0) {
        return;
    }

    if (memcmp(pOldCreature, "CRE V2.1", 8) != 0) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 6885
        UTIL_ASSERT_MSG(FALSE, "Corrupt or Obsolete creature data was encountered during load! Aborting");
        return;
    }

    if (memcmp(pOldCreature, "CRE V2.1", 8) == 0) {
        sub_5A5060(ppCreature, pCreatureSize, pOldCreature, pOldCreatureSize);
    }
}

// 0x5A79E0
static void sub_5A79E0(const CSavedGamePartyCreature20* pCreature20, CSavedGamePartyCreature21* pCreature21)
{
    pCreature21->m_wFlags = pCreature20->m_wFlags;
    pCreature21->m_portraitId = pCreature20->m_portraitId;
    pCreature21->m_creatureOffset = pCreature20->m_creatureOffset;
    pCreature21->m_creatureSize = pCreature20->m_creatureSize;
    memcpy(pCreature21->m_creatureResRef, pCreature20->m_creatureResRef, RESREF_SIZE);
    pCreature21->m_creatureFacing = pCreature20->m_creatureFacing;
    memcpy(pCreature21->m_areaName, pCreature20->m_areaName, RESREF_SIZE);
    pCreature21->m_posX = pCreature20->m_posX;
    pCreature21->m_posY = pCreature20->m_posY;
    pCreature21->m_posViewX = pCreature20->m_posViewX;
    pCreature21->m_posViewY = pCreature20->m_posViewY;
    pCreature21->m_nModalState = pCreature20->m_nModalState;
    pCreature21->m_nHappiness = pCreature20->m_nHappiness;
    memcpy(pCreature21->m_nNumberOfTimesInteractedWith, pCreature20->m_nNumberOfTimesInteractedWith, sizeof(pCreature20->m_nNumberOfTimesInteractedWith));

    // NOTE: 2.0 has 4 quick weapon slots and 3 quick spells, the rest stay
    // as they are.
    memcpy(pCreature21->m_quickWeaponsItemNum, pCreature20->m_quickWeaponsItemNum, sizeof(pCreature20->m_quickWeaponsItemNum));
    memcpy(pCreature21->m_quickWeaponsAbilityNum, pCreature20->m_quickWeaponsAbilityNum, sizeof(pCreature20->m_quickWeaponsAbilityNum));
    memcpy(pCreature21->m_quickSpellsSpellId, pCreature20->m_quickSpellsSpellId, sizeof(pCreature20->m_quickSpellsSpellId));
    memcpy(pCreature21->m_quickItemsItemNum, pCreature20->m_quickItemsItemNum, sizeof(pCreature20->m_quickItemsItemNum));
    memcpy(pCreature21->m_quickItemsAbilityNum, pCreature20->m_quickItemsAbilityNum, sizeof(pCreature20->m_quickItemsAbilityNum));

    memcpy(pCreature21->m_name, pCreature20->m_name, SCRIPTNAME_SIZE);
    pCreature21->m_nNumberOfTimesTalkedTo = pCreature20->m_nNumberOfTimesTalkedTo;
    pCreature21->m_strStrongestKillName = pCreature20->m_strStrongestKillName;
    pCreature21->m_nStrongestKillXPValue = pCreature20->m_nStrongestKillXPValue;
    pCreature21->m_nPreviousTimeWithParty = pCreature20->m_nPreviousTimeWithParty;
    pCreature21->m_nJoinPartyTime = pCreature20->m_nJoinPartyTime;
    pCreature21->m_bWithParty = pCreature20->m_bWithParty;
    pCreature21->m_nChapterKillsXPValue = pCreature20->m_nChapterKillsXPValue;
    pCreature21->m_nChapterKillsNumber = pCreature20->m_nChapterKillsNumber;
    pCreature21->m_nGameKillsXPValue = pCreature20->m_nGameKillsXPValue;
    pCreature21->m_nGameKillsNumber = pCreature20->m_nGameKillsNumber;
    memcpy(pCreature21->m_lSpellStatsName, pCreature20->m_lSpellStatsName, sizeof(pCreature20->m_lSpellStatsName));
    memcpy(pCreature21->m_lSpellStatsCount, pCreature20->m_lSpellStatsCount, sizeof(pCreature20->m_lSpellStatsCount));
    memcpy(pCreature21->m_lWeaponStatsName, pCreature20->m_lWeaponStatsName, sizeof(pCreature20->m_lWeaponStatsName));
    memcpy(pCreature21->m_lWeaponStatsCount, pCreature20->m_lWeaponStatsCount, sizeof(pCreature20->m_lWeaponStatsCount));
    memcpy(pCreature21->m_secondarySounds, pCreature20->m_secondarySounds, RESREF_SIZE);
    memcpy(pCreature21->field_200, pCreature20->field_160, sizeof(pCreature20->field_160));
}

// 0x5A7BF0
static void sub_5A7BF0(const CSavedGamePartyCreature21* pCreature21, CSavedGamePartyCreature* pCreature22)
{
    pCreature22->m_wFlags = pCreature21->m_wFlags;
    pCreature22->m_portraitId = pCreature21->m_portraitId;
    pCreature22->m_creatureOffset = pCreature21->m_creatureOffset;
    pCreature22->m_creatureSize = pCreature21->m_creatureSize;
    memcpy(pCreature22->m_creatureResRef, pCreature21->m_creatureResRef, RESREF_SIZE);
    pCreature22->m_creatureFacing = pCreature21->m_creatureFacing;
    memcpy(pCreature22->m_areaName, pCreature21->m_areaName, RESREF_SIZE);
    pCreature22->m_posX = pCreature21->m_posX;
    pCreature22->m_posY = pCreature21->m_posY;
    pCreature22->m_posViewX = pCreature21->m_posViewX;
    pCreature22->m_posViewY = pCreature21->m_posViewY;
    pCreature22->m_nModalState = pCreature21->m_nModalState;
    pCreature22->m_nHappiness = pCreature21->m_nHappiness;
    memcpy(pCreature22->m_nNumberOfTimesInteractedWith, pCreature21->m_nNumberOfTimesInteractedWith, sizeof(pCreature21->m_nNumberOfTimesInteractedWith));
    memcpy(pCreature22->m_quickWeaponsItemNum, pCreature21->m_quickWeaponsItemNum, sizeof(pCreature21->m_quickWeaponsItemNum));
    memcpy(pCreature22->m_quickWeaponsAbilityNum, pCreature21->m_quickWeaponsAbilityNum, sizeof(pCreature21->m_quickWeaponsAbilityNum));

    // NOTE: 2.1 has 6 quick spells, innates and songs and no spell classes,
    // the rest stay as they are.
    memcpy(pCreature22->m_quickSpellsSpellId, pCreature21->m_quickSpellsSpellId, sizeof(pCreature21->m_quickSpellsSpellId));
    memcpy(pCreature22->m_quickItemsItemNum, pCreature21->m_quickItemsItemNum, sizeof(pCreature21->m_quickItemsItemNum));
    memcpy(pCreature22->m_quickItemsAbilityNum, pCreature21->m_quickItemsAbilityNum, sizeof(pCreature21->m_quickItemsAbilityNum));
    memcpy(pCreature22->m_quickInnatesSpellId, pCreature21->m_quickInnatesSpellId, sizeof(pCreature21->m_quickInnatesSpellId));
    memcpy(pCreature22->m_quickSongsSpellId, pCreature21->m_quickSongsSpellId, sizeof(pCreature21->m_quickSongsSpellId));
    memcpy(pCreature22->field_19A, pCreature21->field_148, sizeof(pCreature21->field_148));

    memcpy(pCreature22->m_name, pCreature21->m_name, SCRIPTNAME_SIZE);
    pCreature22->m_nNumberOfTimesTalkedTo = pCreature21->m_nNumberOfTimesTalkedTo;
    pCreature22->m_strStrongestKillName = pCreature21->m_strStrongestKillName;
    pCreature22->m_nStrongestKillXPValue = pCreature21->m_nStrongestKillXPValue;
    pCreature22->m_nPreviousTimeWithParty = pCreature21->m_nPreviousTimeWithParty;
    pCreature22->m_nJoinPartyTime = pCreature21->m_nJoinPartyTime;
    pCreature22->m_bWithParty = pCreature21->m_bWithParty;
    pCreature22->m_nChapterKillsXPValue = pCreature21->m_nChapterKillsXPValue;
    pCreature22->m_nChapterKillsNumber = pCreature21->m_nChapterKillsNumber;
    pCreature22->m_nGameKillsXPValue = pCreature21->m_nGameKillsXPValue;
    pCreature22->m_nGameKillsNumber = pCreature21->m_nGameKillsNumber;
    memcpy(pCreature22->m_lSpellStatsName, pCreature21->m_lSpellStatsName, sizeof(pCreature21->m_lSpellStatsName));
    memcpy(pCreature22->m_lSpellStatsCount, pCreature21->m_lSpellStatsCount, sizeof(pCreature21->m_lSpellStatsCount));
    memcpy(pCreature22->m_lWeaponStatsName, pCreature21->m_lWeaponStatsName, sizeof(pCreature21->m_lWeaponStatsName));
    memcpy(pCreature22->m_lWeaponStatsCount, pCreature21->m_lWeaponStatsCount, sizeof(pCreature21->m_lWeaponStatsCount));
    memcpy(pCreature22->m_secondarySounds, pCreature21->m_secondarySounds, RESREF_SIZE);
    memcpy(pCreature22->field_25E, pCreature21->field_200, sizeof(pCreature21->field_200));
}

// 0x5A9680
void CInfGame::ProgressBarCallback(DWORD dwSize, BOOLEAN bInitialize)
{
    if (bInitialize == TRUE) {
        m_dwLastProgressRenderTickCount = GetTickCount() - 250;
        m_dwLastProgressMsgTickCount = GetTickCount() - 5000;
    }

    g_pChitin->cProgressBar.m_nActionProgress += dwSize;

    if (GetTickCount() - m_dwLastProgressMsgTickCount >= 1000) {
        m_dwLastProgressMsgTickCount = GetTickCount();

        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            g_pBaldurChitin->GetBaldurMessage()->SendProgressBarStatus(g_pChitin->cProgressBar.m_nActionProgress,
                g_pChitin->cProgressBar.m_nActionTarget,
                g_pChitin->cProgressBar.m_bWaiting,
                g_pChitin->cProgressBar.m_nWaitingReason,
                g_pChitin->cProgressBar.m_bTimeoutVisible,
                g_pChitin->cProgressBar.m_nSecondsToTimeout);
        }
    }

    if (GetTickCount() - m_dwLastProgressRenderTickCount >= 250) {
        m_dwLastProgressRenderTickCount = GetTickCount();

        g_pChitin->m_bDisplayStale = TRUE;
        g_pChitin->cDimm.field_0 = 1;
        g_pChitin->cDimm.field_4 = 1;
        SleepEx(25, TRUE);

        sub_59FA00(FALSE);
    }
}

// 0x5A9780
char CInfGame::sub_5A9780(BYTE nKey)
{
    if (nKey >= 'A' && nKey <= 'Z') {
        return static_cast<char>(nKey);
    }

    if (nKey >= '0' && nKey <= '9') {
        return static_cast<char>(nKey);
    }

    for (int sym = 0; sym < CINFGAME_SPECIAL_SYMBOLS; sym++) {
        if (byte_8ABA3C[sym] == nKey) {
            return byte_8ABA24[sym];
        }
    }

    return -1;
}

// 0x5A97D0
INT CInfGame::sub_5A97D0(char ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return static_cast<BYTE>(ch - 32);
    }

    if (ch >= 'A' && ch <= 'Z') {
        return static_cast<BYTE>(ch);
    }

    if (ch >= '0' && ch <= '9') {
        return static_cast<BYTE>(ch);
    }

    for (int sym = 0; sym < CINFGAME_SPECIAL_SYMBOLS; sym++) {
        if (byte_8ABA24[sym] == ch) {
            return byte_8ABA3C[sym];
        }
    }

    return -1;
}

// 0x5A9830
BYTE CInfGame::sub_5A9830(int index)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 7779
    UTIL_ASSERT(index >= 0 && index < CINFGAME_KEYMAP_SIZE);

    if (index >= FIFTY_THREE) {
        return 0;
    }

    // NOTE: Uninline.
    return static_cast<BYTE>(sub_5A97D0(off_8AFD24[index][0]));
}

// 0x5A98F0
void CInfGame::LoadKeymap()
{
    CString sKey;

    memset(m_pKeymap, 0, sizeof(m_pKeymap));
    memset(m_pKeymapFlags, 0, sizeof(m_pKeymapFlags));

    // NOTE: Unsigned compare in both loops.
    WORD index;
    for (index = 0; index < FIFTY_THREE; index++) {
        char* buffer = sKey.GetBuffer(128);
        GetPrivateProfileStringA("Keymap",
            off_8AFC50[index],
            off_8AFD24[index],
            buffer,
            128,
            ".\\Keymap.ini");
        sKey.ReleaseBuffer();
        LoadKey(sKey, index);
    }

    for (; index < CINFGAME_KEYMAP_SIZE; index++) {
        char* buffer = sKey.GetBuffer(128);
        if (index >= 199) {
            GetPrivateProfileStringA("Wizard Spells",
                off_8B4F84[index - FIFTY_THREE],
                "",
                buffer,
                128,
                ".\\Keymap.ini");
        } else {
            GetPrivateProfileStringA("Priest Spells",
                off_8B4F84[index - FIFTY_THREE],
                "",
                buffer,
                128,
                ".\\Keymap.ini");
        }
        sKey.ReleaseBuffer();
        LoadKey(sKey, index);
    }
}

// 0x5A9A30
void CInfGame::LoadKey(const CString& sKey, SHORT index)
{
    INT nOffset = 0;
    if (sKey.GetLength() > 0) {
        if (sKey.Find("CTRL-") == 0
            || sKey.Find("ctrl-") == 0
            || sKey.Find("Ctrl-") == 0) {
            nOffset = strlen("CTRL-");
            m_pKeymapFlags[index] = 1;
        }

        if (sKey[nOffset] >= 'a' && sKey[nOffset] <= 'z') {
            m_pKeymap[index] = sKey[nOffset] - 32;
        } else {
            m_pKeymap[index] = sKey[nOffset];

            for (int sym = 0; sym < CINFGAME_SPECIAL_SYMBOLS; sym++) {
                if (byte_8ABA24[sym] == sKey[nOffset]) {
                    m_pKeymap[index] = byte_8ABA3C[sym];
                    break;
                }
            }
        }
    }
}

// 0x5A9B00
void CInfGame::SetKey(INT nIndex, BYTE nKey, BYTE nKeyFlag)
{
    if (nIndex >= 0 && nIndex < CINFGAME_KEYMAP_SIZE) {
        m_pKeymap[nIndex] = nKey;
        m_pKeymapFlags[nIndex] = nKeyFlag;

        for (int sym = 0; sym < CINFGAME_SPECIAL_SYMBOLS; sym++) {
            if (byte_8ABA3C[sym] == nKey) {
                nKey = byte_8ABA24[sym];
                break;
            }
        }

        if (nIndex < FIFTY_THREE) {
            char buffer[16];
            sprintf(buffer, "%s%c", nKeyFlag ? "CTRL-" : "", nKey);
            WritePrivateProfileStringA("Keymap",
                off_8AFC50[nIndex],
                buffer,
                ".\\Keymap.ini");
        }
    }
}

// 0x5A9BA0
void CInfGame::LoadOptions()
{
    m_cOptions.m_nTilesPrecachePercent = GetPrivateProfileIntA("Game Options",
        "Tiles Precache Percent",
        100,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nTilesPrecachePercent > 100) {
        m_cOptions.m_nTilesPrecachePercent = 100;
    }

    m_cOptions.m_bDuplicateFloatingText = GetPrivateProfileIntA("Game Options",
        "Duplicate Floating Text",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bTranslucentBlts = GetPrivateProfileIntA("Game Options",
        "Translucent Blts",
        1,
        g_pBaldurChitin->GetIniFileName());
    CInfinity::TRANSLUCENT_BLTS_ON = m_cOptions.m_bTranslucentBlts;
    CVidCell::TRANSLUCENT_BLTS_ON = m_cOptions.m_bTranslucentBlts;

    m_cOptions.m_bMaxHP = GetPrivateProfileIntA("Game Options",
        "Maximum HP",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bAutoPauseOnTrap = GetPrivateProfileIntA("Game Options",
        "Pause On Trap",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.field_AC = 0;

    m_cOptions.m_bEnvironmentalAudio = GetPrivateProfileIntA("Game Options",
        "Environmental Audio",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bCheatsEnabled = GetPrivateProfileIntA("Game Options",
        "Cheats",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nGoreOption = GetPrivateProfileIntA("Game Options",
        "Grant 2000",
        0,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nGoreOption) {
        m_cOptions.m_nGore = GetPrivateProfileIntA("Game Options",
            "Chucknomitron",
            1,
            g_pBaldurChitin->GetIniFileName());
    } else {
        m_cOptions.m_nGore = 0;
    }

    m_cOptions.m_bFootStepsSounds = GetPrivateProfileIntA("Game Options",
        "Footsteps",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nMouseScrollSpeed = GetPrivateProfileIntA("Game Options",
        "Mouse Scroll Speed",
        28,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nMouseScrollSpeed < 6 || m_cOptions.m_nMouseScrollSpeed > 50) {
        m_cOptions.m_nMouseScrollSpeed = 28;
    }

    m_cOptions.m_nGuiFeedbackLevel = GetPrivateProfileIntA("Game Options",
        "GUI Feedback Level",
        5,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nGuiFeedbackLevel < 1 || m_cOptions.m_nGuiFeedbackLevel > 5) {
        m_cOptions.m_nGuiFeedbackLevel = 5;
    }

    m_cOptions.m_nLocatorFeedbackLevel = GetPrivateProfileIntA("Game Options",
        "Locator Feedback Level",
        3,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nLocatorFeedbackLevel < 1 || m_cOptions.m_nLocatorFeedbackLevel > 5) {
        m_cOptions.m_nLocatorFeedbackLevel = 3;
    }

    m_cOptions.m_nBoredTime = GetPrivateProfileIntA("Game Options",
        "Bored Timeout",
        CScreenWorld::BORED_TIME,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nBoredTime == 0 || m_cOptions.m_nBoredTime > CScreenWorld::BORED_TIME) {
        m_cOptions.m_nBoredTime = CScreenWorld::BORED_TIME;
    }

    m_cOptions.m_bAlwaysDither = GetPrivateProfileIntA("Game Options",
        "Always Dither",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bSubtitles = GetPrivateProfileIntA("Game Options",
        "Subtitles",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nKeyboardScrollSpeed = GetPrivateProfileIntA("Game Options",
        "Keyboard Scroll Speed",
        36,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nKeyboardScrollSpeed < 18 || m_cOptions.m_nKeyboardScrollSpeed > 150) {
        m_cOptions.m_nKeyboardScrollSpeed = 36;
    }

    m_cOptions.m_nCommandSoundsFrequency = GetPrivateProfileIntA("Game Options",
        "Command Sounds Frequency",
        2,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nCommandSoundsFrequency == 0 || m_cOptions.m_nCommandSoundsFrequency > 3) {
        m_cOptions.m_nCommandSoundsFrequency = 2;
    }

    m_cOptions.m_nSelectionSoundsFrequency = GetPrivateProfileIntA("Game Options",
        "Selection Sounds Frequency",
        3,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nSelectionSoundsFrequency == 0 || m_cOptions.m_nSelectionSoundsFrequency > 3) {
        m_cOptions.m_nSelectionSoundsFrequency = 3;
    }

    m_cOptions.m_nEffectTextLevel = GetPrivateProfileIntA("Game Options",
        "Effect Text Level",
        62,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bDarkvision = GetPrivateProfileIntA("Game Options",
        "Darkvision",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bWeatherEnabled = GetPrivateProfileIntA("Game Options",
        "Weather",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nTutorialState = GetPrivateProfileIntA("Game Options",
        "Tutorial State",
        1,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nTutorialState > 1) {
        m_cOptions.m_nTutorialState = 1;
    }

    m_cOptions.m_bAttackSounds = GetPrivateProfileIntA("Game Options",
        "Attack Sounds",
        1,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_bAttackSounds > 1) {
        m_cOptions.m_bAttackSounds = 1;
    }

    m_cOptions.m_nAutoPauseState = GetPrivateProfileIntA("Game Options",
        "Auto Pause State",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bAutoPauseCenter = GetPrivateProfileIntA("Game Options",
        "Auto Pause Center",
        1,
        g_pBaldurChitin->GetIniFileName());

    if (g_pBaldurChitin->cNetwork.GetSessionHosting() == TRUE) {
        m_cOptions.m_nDifficultyLevel = GetPrivateProfileIntA("Game Options",
            "Difficulty Level",
            3,
            g_pBaldurChitin->GetIniFileName());

        if (m_cOptions.m_nDifficultyLevel < 1 || m_cOptions.m_nDifficultyLevel > 5) {
            m_cOptions.m_nDifficultyLevel = 3;
        }

        switch (m_cOptions.m_nDifficultyLevel) {
        case 1:
            m_cOptions.m_nDifficultyMultiplier = -50;
            break;
        case 2:
            m_cOptions.m_nDifficultyMultiplier = -25;
            break;
        case 3:
            m_cOptions.m_nDifficultyMultiplier = 0;
            break;
        case 4:
            m_cOptions.m_nDifficultyMultiplier = 50;
            break;
        case 5:
            m_cOptions.m_nDifficultyMultiplier = 100;
            break;
        }

        m_cOptions.m_nMPDifficultyMultiplier = m_cOptions.m_nDifficultyMultiplier;
        m_nDifficultyLevel = -1;
    } else {
        m_nDifficultyLevel = GetPrivateProfileIntA("Game Options",
            "Difficulty Level",
            3,
            g_pBaldurChitin->GetIniFileName());

        // NOTE: Difficulty level is not normalized to 1..5 range as in the
        // code path above.

        m_cOptions.m_nDifficultyLevel = m_nDifficultyLevel;
    }

    m_cOptions.m_bQuickItemMapping = GetPrivateProfileIntA("Game Options",
        "Quick Item Mapping",
        1,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_bQuickItemMapping > 1) {
        m_cOptions.m_bQuickItemMapping = 1;
    }

    m_cOptions.m_bTerrainHugging = GetPrivateProfileIntA("Game Options",
        "Terrain Hugging",
        0,
        g_pBaldurChitin->GetIniFileName());

    CGameSprite::SHOW_CHARACTER_HP = GetPrivateProfileIntA("Game Options",
        "Show Character HP",
        0,
        g_pBaldurChitin->GetIniFileName());

    CGameSprite::GRAVITY_IS_DOWN = GetPrivateProfileIntA("Game Options",
        "Gravity Is Down",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bShowQuestXP = GetPrivateProfileIntA("Game Options",
        "Show Quest XP",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bStaticAnimations = GetPrivateProfileIntA("Game Options",
        "Static Animations",
        1,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_bCriticalHitScreenShake = GetPrivateProfileIntA("Game Options",
        "Critical Hit Screen Shake",
        1,
        g_pBaldurChitin->GetIniFileName());

    GetPrivateProfileIntA("Game Options",
        "Suppress Extra Difficulty Damage",
        0,
        g_pBaldurChitin->GetIniFileName());

    // NOTE: The INI value is read and discarded.
    m_cOptions.m_bSuppressExtraDifficultyDamage = 0;

    if (!g_pBaldurChitin->cNetwork.GetSessionHosting()) {
        g_pBaldurChitin->m_cBaldurMessage.DemandSettingsNightmareMode(FALSE);
    }

    m_cOptions.m_nDefaultCharGenPointsPool = GetPrivateProfileIntA("Game Options",
        "DefaultCharGenPointsPool",
        16,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nDefaultCharGenPointsPool < 10 || m_cOptions.m_nDefaultCharGenPointsPool > 108) {
        m_cOptions.m_nDefaultCharGenPointsPool = 16;
    }

    m_cOptions.m_nDefaultCharGenPoints = GetPrivateProfileIntA("Game Options",
        "DefaultCharGenPoints",
        10,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nDefaultCharGenPoints < 3 || m_cOptions.m_nDefaultCharGenPoints > 18) {
        m_cOptions.m_nDefaultCharGenPoints = 10;
    }

    m_cOptions.m_bHotkeysOnToolTips = GetPrivateProfileIntA("Game Options",
        "Hotkeys On Tooltips",
        1,
        g_pBaldurChitin->GetIniFileName());

    g_pBaldurChitin->m_cBaldurMessage.SetSignalDefaultSecondsToTimeout();

    m_cOptions.m_bAllScreenShake = (GetPrivateProfileIntA("Game Options",
                                        "All Screen Shake",
                                        1,
                                        g_pBaldurChitin->GetIniFileName())
        == 1);

    m_cOptions.m_nOldPortraitHealth = GetPrivateProfileIntA("Game Options",
        "Old Portrait Health",
        0,
        g_pBaldurChitin->GetIniFileName());

    m_cOptions.m_nHitPointBarSequence = GetPrivateProfileIntA("Game Options",
        "Hit Point Bar Sequence",
        0,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nHitPointBarSequence < 0 || m_cOptions.m_nHitPointBarSequence > 5) {
        m_cOptions.m_nHitPointBarSequence = 0;
    }
}

// 0x5AA350
void CInfGame::SaveOptions()
{
    CString sValue;

    sValue.Format("%d", m_cOptions.m_bFootStepsSounds);
    WritePrivateProfileStringA("Game Options", "Footsteps", sValue, g_pBaldurChitin->GetIniFileName());

    if ((!g_pChitin->cNetwork.GetSessionOpen() || g_pChitin->cNetwork.GetSessionHosting())
        && m_cOptions.m_nGoreOption != 0) {
        sValue.Format("%d", m_cOptions.m_nGoreOption);
        WritePrivateProfileStringA("Game Options", "Grant 2000", sValue, g_pBaldurChitin->GetIniFileName());

        sValue.Format("%d", m_cOptions.m_nGore);
        WritePrivateProfileStringA("Game Options", "Chucknomitron", sValue, g_pBaldurChitin->GetIniFileName());
    }

    sValue.Format("%d", m_cOptions.m_nMouseScrollSpeed);
    WritePrivateProfileStringA("Game Options", "Mouse Scroll Speed", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nGuiFeedbackLevel);
    WritePrivateProfileStringA("Game Options", "GUI Feedback Level", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nLocatorFeedbackLevel);
    WritePrivateProfileStringA("Game Options", "Locator Feedback Level", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nBoredTime);
    WritePrivateProfileStringA("Game Options", "Bored Timeout", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAlwaysDither);
    WritePrivateProfileStringA("Game Options", "Always Dither", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bSubtitles);
    WritePrivateProfileStringA("Game Options", "Subtitles", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nKeyboardScrollSpeed);
    WritePrivateProfileStringA("Game Options", "Keyboard Scroll Speed", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nCommandSoundsFrequency);
    WritePrivateProfileStringA("Game Options", "Command Sounds Frequency", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nSelectionSoundsFrequency);
    WritePrivateProfileStringA("Game Options", "Selection Sounds Frequency", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nEffectTextLevel);
    WritePrivateProfileStringA("Game Options", "Effect Text Level", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bDarkvision);
    WritePrivateProfileStringA("Game Options", "Darkvision", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bWeatherEnabled);
    WritePrivateProfileStringA("Game Options", "Weather", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nTutorialState);
    WritePrivateProfileStringA("Game Options", "Tutorial State", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAttackSounds);
    WritePrivateProfileStringA("Game Options", "Attack Sounds", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nAutoPauseState);
    WritePrivateProfileStringA("Game Options", "Auto Pause State", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAutoPauseCenter);
    WritePrivateProfileStringA("Game Options", "Auto Pause Center", sValue, g_pBaldurChitin->GetIniFileName());

    if (m_nDifficultyLevel == -1) {
        sValue.Format("%d", m_cOptions.m_nDifficultyLevel);
    } else {
        sValue.Format("%d", m_nDifficultyLevel);
    }

    WritePrivateProfileStringA("Game Options", "Difficulty Level", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bQuickItemMapping);
    WritePrivateProfileStringA("Game Options", "Quick Item Mapping", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bEnvironmentalAudio);
    WritePrivateProfileStringA("Game Options", "Environmental Audio", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bTerrainHugging);
    WritePrivateProfileStringA("Game Options", "Terrain Hugging", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bStaticAnimations);
    WritePrivateProfileStringA("Game Options", "Static Animations", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bTranslucentBlts);
    WritePrivateProfileStringA("Game Options", "Translucent Blts", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bMaxHP);
    WritePrivateProfileStringA("Game Options", "Maximum HP", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAutoPauseOnTrap);
    WritePrivateProfileStringA("Game Options", "Pause On Trap", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", CGameSprite::SHOW_CHARACTER_HP);
    WritePrivateProfileStringA("Game Options", "Show Character HP", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bShowQuestXP);
    WritePrivateProfileStringA("Game Options", "Show Quest XP", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", g_pBaldurChitin->field_1A0);
    WritePrivateProfileStringA("Program Options", "Double Byte Character Support", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bCriticalHitScreenShake);
    WritePrivateProfileStringA("Game Options", "Critical Hit Screen Shake", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bHotkeysOnToolTips);
    WritePrivateProfileStringA("Game Options", "Hotkeys On Tooltips", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bDuplicateFloatingText);
    WritePrivateProfileStringA("Game Options", "Duplicate Floating Text", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nTilesPrecachePercent);
    WritePrivateProfileStringA("Game Options", "Tiles Precache Percent", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bSuppressExtraDifficultyDamage);
    WritePrivateProfileStringA("Game Options", "Suppress Extra Difficulty Damage", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_bAllScreenShake);
    WritePrivateProfileStringA("Game Options", "All Screen Shake", sValue, g_pBaldurChitin->GetIniFileName());

    sValue.Format("%d", m_cOptions.m_nHitPointBarSequence);
    WritePrivateProfileStringA("Game Options", "Hit Point Bar Sequence", sValue, g_pBaldurChitin->GetIniFileName());
}

// 0x5AAC30
void CInfGame::LoadMultiPlayerPermissions()
{
    if (!g_pBaldurChitin->cNetwork.GetSessionOpen()
        || !g_pBaldurChitin->cNetwork.GetSessionHosting()) {
        return;
    }

    CMultiplayerSettings* pSettings = GetMultiplayerSettings();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 8323
    UTIL_ASSERT(pSettings != NULL);

    CString sPermissions;
    CString sDefault("");

    GetPrivateProfileStringA("Multiplayer",
        "Default Permissions",
        sDefault,
        sPermissions.GetBuffer(10),
        10,
        g_pBaldurChitin->GetIniFileName());
    sPermissions.ReleaseBuffer(-1);

    if (sPermissions.GetLength() == 8) {
        BOOL bPurchasing = sPermissions[0] == '1';
        BOOL bAreaTransition = sPermissions[1] == '1';
        BOOL bDialog = sPermissions[2] == '1';
        BOOL bCharRecords = sPermissions[3] == '1';
        BOOL bPausing = sPermissions[4] == '1';
        BOOL bGroupPool = sPermissions[5] == '1';
        BOOL bLeader = sPermissions[6] == '1';
        BOOL bModifyChars = sPermissions[7] == '1';

        pSettings->SetPermission(-1,
            CGamePermission::PURCHASING,
            bPurchasing,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::AREA_TRANSITION,
            bAreaTransition,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::DIALOG,
            bDialog,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::CHAR_RECORDS,
            bCharRecords,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::PAUSING,
            bPausing,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::GROUP_POOL,
            bGroupPool,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::LEADER,
            bLeader,
            TRUE);
        pSettings->SetPermission(-1,
            CGamePermission::MODIFY_CHARS,
            bModifyChars,
            TRUE);
    }

    INT nRestrictStore = GetPrivateProfileIntA("Multiplayer",
        "Restrict Stores",
        0,
        g_pBaldurChitin->GetIniFileName());
    pSettings->SetRestrictStoreOption(nRestrictStore != 0);

    BYTE nImportingBitField = static_cast<BYTE>(GetPrivateProfileIntA("Multiplayer",
        "Import Character",
        CMultiplayerSettings::IMPORT_ALL,
        g_pBaldurChitin->GetIniFileName()));
    pSettings->SetImportingCharacterOption(nImportingBitField);
}

// 0x5AAEC0
void CInfGame::SaveMultiPlayerPermissions()
{
    if (!g_pBaldurChitin->cNetwork.GetSessionOpen()
        || !g_pBaldurChitin->cNetwork.GetSessionHosting()) {
        return;
    }

    CMultiplayerSettings* pSettings = GetMultiplayerSettings();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 3290
    UTIL_ASSERT(pSettings != NULL);

    BOOL bPurchasing = pSettings->GetPermission(-1, CGamePermission::PURCHASING);
    BOOL bAreaTransition = pSettings->GetPermission(-1, CGamePermission::AREA_TRANSITION);
    BOOL bDialog = pSettings->GetPermission(-1, CGamePermission::DIALOG);
    BOOL bCharRecords = pSettings->GetPermission(-1, CGamePermission::CHAR_RECORDS);
    BOOL bPausing = pSettings->GetPermission(-1, CGamePermission::PAUSING);
    BOOL bGroupPool = pSettings->GetPermission(-1, CGamePermission::GROUP_POOL);
    BOOL bLeader = pSettings->GetPermission(-1, CGamePermission::LEADER);
    BOOL bModifyChars = pSettings->GetPermission(-1, CGamePermission::MODIFY_CHARS);

    CString sPermissions("00000000");
    sPermissions.SetAt(0, bPurchasing + '0');
    sPermissions.SetAt(1, bAreaTransition + '0');
    sPermissions.SetAt(2, bDialog + '0');
    sPermissions.SetAt(3, bCharRecords + '0');
    sPermissions.SetAt(4, bPausing + '0');
    sPermissions.SetAt(5, bGroupPool + '0');
    sPermissions.SetAt(6, bLeader + '0');
    sPermissions.SetAt(7, bModifyChars + '0');

    WritePrivateProfileStringA("Multiplayer",
        "Default Permissions",
        sPermissions,
        g_pBaldurChitin->GetIniFileName());

    CString sTemp;

    sTemp.Format("%d", pSettings->m_bRestrictStoreOption);
    WritePrivateProfileStringA("Multiplayer",
        "Restrict Stores",
        sTemp,
        g_pBaldurChitin->GetIniFileName());

    sTemp.Format("%d", pSettings->m_nImportingBitField);
    WritePrivateProfileStringA("Multiplayer",
        "Import Character",
        sTemp,
        g_pBaldurChitin->GetIniFileName());
}

// 0x5AB190
void CInfGame::LoadGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 8447
    UTIL_ASSERT(m_sSaveGame != "");

    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->sub_518650();
    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->sub_518690();

    g_pBaldurChitin->GetTlkTable().m_override.CloseFiles();

    dword_8E52EC = 0;
    dword_8E52F0 = 0;

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        sub_59FA00(TRUE);
        g_pChitin->SetProgressBar(TRUE,
            9888,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = 8000000;
        g_pChitin->cProgressBar.m_bDisableMinibars = 1;
    }

    g_pChitin->m_bDisplayStale = TRUE;
    SleepEx(25, FALSE);

    if (bProgressBarInPlace || bProgressBarRequired) {
        g_pChitin->cProgressBar.AddActionTarget(625000);
    }

    m_bInLoadGame = TRUE;

    BOOL bMusicThreadPriorityChanged = FALSE;
    int nMusicThreadPriority = GetThreadPriority(g_pChitin->m_hMusicThread);

    if (GetPrivateProfileIntA("Program Options", "Volume Music", 0, g_pChitin->GetIniFileName())) {
        bMusicThreadPriorityChanged = SetThreadPriority(g_pChitin->m_hMusicThread, 15);
    }

    InitGame(FALSE, FALSE);

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(312500, FALSE);
    }

    m_bFromNewGame = FALSE;

    CString sDir = GetDirSave();
    if (sDir == m_sSaveDir + "default\\") {
        sDir = m_sMultiplayerSaveDir + m_sSaveGame + "\\";
    }

    if (!g_pChitin->cDimm.DirectoryCopyFiles(sDir, m_sTempDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8524
        UTIL_ASSERT_MSG(FALSE, "Couldn't copy files from Temp to Save directories");
    }

    if (!g_pChitin->cDimm.UncompressDirectory(m_sTempDir, sDir)) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8526
        UTIL_ASSERT_MSG(FALSE, "Couldn't uncompress save game into Temp directory");
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(312500, FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "ICEWIND2.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8541
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT0.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8548
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT1.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8555
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT2.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8562
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT3.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8569
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT4.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8576
        UTIL_ASSERT(FALSE);
    }

    if (!DeleteFileA(m_sTempDir + "PORTRT5.BMP")
        && GetLastError() != ERROR_FILE_NOT_FOUND) {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 8583
        UTIL_ASSERT(FALSE);
    }

    g_pChitin->cDimm.AddToDirectoryList(m_sTempDir, TRUE);

    g_pBaldurChitin->GetTlkTable().OpenOverride(CString("temp/default.toh"),
        CString("temp/default.tot"));

    m_cWorldMap.SetResRef(CResRef("WORLDMAP"));

    CGameFile cGameFile;
    cGameFile.SetResRef(CResRef("ICEWIND2"), TRUE, TRUE);

    Unmarshal(cGameFile.GetData(),
        cGameFile.GetDataSize(),
        bProgressBarInPlace | bProgressBarRequired);

    g_pBaldurChitin->cSoundMixer.StartSong(-1, 0x1 | 0x2);
    SleepEx(500, FALSE);
    g_pBaldurChitin->cSoundMixer.StopMusic(TRUE);

    cGameFile.Release();

    m_bAnotherPlayerJoinedGame = FALSE;

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        m_cRemoteObjectArray.ChangeControlOnLoadGame();
    }

    m_bInLoadGame = FALSE;

    if (bMusicThreadPriorityChanged) {
        SetThreadPriority(g_pChitin->m_hMusicThread, nMusicThreadPriority);
    }

    m_nLastSaveTime = m_worldTime.m_gameTime;

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;

        g_pChitin->m_bDisplayStale = TRUE;
        sub_59FA00(TRUE);

        g_pChitin->SetProgressBar(FALSE,
            0,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
    }

    SelectAll(FALSE);

    g_pBaldurChitin->GetObjectGame()->m_cButtonArray.UpdateState();
}

// 0x5ABA20
void CInfGame::NewGame(BOOLEAN bProgressBarRequired, BOOLEAN bProgressBarInPlace)
{
    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        sub_59FA00(TRUE);

        g_pChitin->SetProgressBar(TRUE,
            9889,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = 8000000;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        g_pChitin->cProgressBar.AddActionTarget(625000);
    }

    m_bInLoadGame = TRUE;

    BOOL bMusicThreadPriorityChanged = FALSE;
    int nMusicThreadPriority = GetThreadPriority(g_pChitin->m_hMusicThread);

    if (GetPrivateProfileIntA("Program Options", "Volume Music", 0, g_pChitin->GetIniFileName())) {
        bMusicThreadPriorityChanged = SetThreadPriority(g_pChitin->m_hMusicThread, 15);
    }

    g_pBaldurChitin->GetTlkTable().m_override.CloseFiles();

    if (g_pChitin->cDimm.DirectoryRemoveFiles(CString(".\\mpsave\\default\\"))) {
        g_pChitin->cDimm.m_cKeyTable.RescanEverything();
    }

    InitGame(FALSE, FALSE);

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(156250, FALSE);
    }

    CVariable chapter;
    chapter.SetName(CHAPTER_GLOBAL);
    chapter.m_intValue = -1;
    m_variables.AddKey(chapter);

    m_bFromNewGame = TRUE;

    g_pChitin->cDimm.AddToDirectoryList(m_sScriptsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sSoundsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sPortraitsDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sCharactersDir, TRUE);
    g_pChitin->cDimm.AddToDirectoryList(m_sTempDir, TRUE);

    g_pBaldurChitin->GetTlkTable().OpenOverride(CString("temp/default.toh"), CString("temp/default.tot"));

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(156250, FALSE);
    }

    m_cWorldMap.SetResRef(CResRef("WORLDMAP"));

    CGameFile cGameFile;
    cGameFile.SetResRef(CResRef("ICEWIND2"), TRUE, TRUE);

    Unmarshal(cGameFile.GetData(),
        cGameFile.GetDataSize(),
        bProgressBarInPlace | bProgressBarRequired);

    m_cOptions.m_nNightmareMode = GetPrivateProfileIntA("Game Options",
        "Nightmare Mode",
        0,
        g_pBaldurChitin->GetIniFileName());

    if (m_cOptions.m_nNightmareMode == 1 && m_cOptions.m_nDifficultyLevel != 5) {
        m_cOptions.m_nDifficultyLevel = 5;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 8769
    UTIL_ASSERT(m_nCharacters == 0);

    m_bAnotherPlayerJoinedGame = FALSE;

    m_sSaveGame = "default";
    CScreenCharacter::SAVE_NAME = "default";

    m_bInLoadGame = FALSE;

    if (bMusicThreadPriorityChanged == TRUE) {
        SetThreadPriority(g_pChitin->m_hMusicThread, nMusicThreadPriority);
    }

    if (bProgressBarInPlace || bProgressBarRequired) {
        ProgressBarCallback(156250, FALSE);
    }

    if (!bProgressBarInPlace && bProgressBarRequired == TRUE) {
        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;
        g_pChitin->cProgressBar.m_bDisableMinibars = TRUE;
        g_pChitin->m_bDisplayStale = TRUE;

        sub_59FA00(TRUE);

        g_pChitin->SetProgressBar(FALSE,
            0,
            0,
            0,
            FALSE,
            0,
            FALSE,
            0,
            FALSE,
            FALSE,
            255);
    }

    m_nReputation = -10;
}

// 0x5AC0A0
BOOLEAN CInfGame::sub_5AC0A0(CGameArea* pArea)
{
    if (!m_bInLoadGame
        && pArea != NULL
        && pArea->m_bAreaLoaded) {
        return pArea->field_B16;
    } else {
        return FALSE;
    }
}

// 0x5AC0D0
void CInfGame::sub_5AC0D0()
{
    STRREF strError;

    for (int index = 0; index < CINFGAME_MAX_AREAS; index++) {
        if (m_gameAreas[index] != NULL && m_gameAreas[index]->m_bAreaLoaded) {
            m_gameAreas[index]->field_B16 = !m_gameAreas[index]->CanSaveGame(strError);
        }
    }
}

// 0x5AD470
void CInfGame::SelectAll(BOOLEAN bPlaySound)
{
    LONG iSprite;
    CGameSprite* pSprite;
    BYTE rc;
    SHORT cnt;
    POSITION pos;

    for (cnt = 0; cnt < m_nCharacters; cnt++) {
        iSprite = m_characterPortraits[cnt];
        do {
            rc = m_cObjectArray.GetShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->m_pArea == GetVisibleArea()) {
                if (pSprite->Orderable(FALSE) && !pSprite->m_bSelected) {
                    do {
                        rc = m_cObjectArray.GetDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        pSprite->Select();
                        m_cObjectArray.ReleaseDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
                if (pSprite->Orderable(FALSE) && bPlaySound) {
                    bPlaySound = FALSE;
                    pSprite->PlaySound(CGameSprite::SOUND_SELECT, TRUE, FALSE, FALSE);
                }
            } else if (pSprite->m_bSelected) {
                do {
                    rc = m_cObjectArray.GetDeny(iSprite,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    pSprite->Unselect();
                    m_cObjectArray.ReleaseDeny(iSprite,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    UpdatePortrait(cnt, 1);
                }
            }
            m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    pos = m_familiars.GetHeadPosition();
    while (pos != NULL) {
        iSprite = static_cast<LONG>(reinterpret_cast<INT_PTR>(m_familiars.GetNext(pos)));
        rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->GetArea() == GetVisibleArea()) {
                if (pSprite->Orderable(FALSE) && !pSprite->m_bSelected) {
                    do {
                        rc = m_cObjectArray.GetDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        pSprite->Select();
                        m_cObjectArray.ReleaseDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
                if (pSprite->Orderable(FALSE) && bPlaySound) {
                    bPlaySound = FALSE;
                    pSprite->PlaySound(CGameSprite::SOUND_SELECT, TRUE, FALSE, FALSE);
                }
            }
            rc = m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    pos = m_allies.GetHeadPosition();
    while (pos != NULL) {
        iSprite = static_cast<LONG>(reinterpret_cast<INT_PTR>(m_allies.GetNext(pos)));
        rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->GetArea() == GetVisibleArea()) {
                if (pSprite->Orderable(FALSE) && !pSprite->m_bSelected) {
                    do {
                        rc = m_cObjectArray.GetDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        pSprite->Select();
                        m_cObjectArray.ReleaseDeny(iSprite,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
                if (pSprite->Orderable(FALSE) && bPlaySound) {
                    bPlaySound = FALSE;
                    pSprite->PlaySound(CGameSprite::SOUND_SELECT, TRUE, FALSE, FALSE);
                }
            }
            rc = m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    SelectToolbar();
}

// 0x5AD7E0
void CInfGame::UnselectAll()
{
    LONG* pGroup = m_group.GetGroupList();
    SHORT nGroup = m_group.GetCount();
    CGameSprite* pSprite;

    for (SHORT cnt = 0; cnt < nGroup; cnt++) {
        BYTE rc;
        do {
            rc = m_cObjectArray.GetDeny(pGroup[cnt],
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->Unselect();

            m_cObjectArray.ReleaseDeny(pGroup[cnt],
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            // NOTE: Uninline.
            UpdatePortrait(GetFixedOrderCharacterPortraitNum(pGroup[cnt]), 1);
        }
    }

    delete pGroup;
}

// 0x5AD8A0
BOOL CInfGame::SelectCharacter(LONG characterId, BOOLEAN bPlaySelectSound)
{
    CGameSprite* pSprite;
    BYTE rc;
    SHORT nPortrait = GetCharacterPortraitNum(characterId);

    if (nPortrait == -1) {
        if (!IsAlly(characterId) && !IsFamiliar(characterId)) {
            return FALSE;
        }
    }

    do {
        rc = m_cObjectArray.GetShare(characterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    if (pSprite->m_bSelected
        && !g_pBaldurChitin->GetScreenWorld()->GetShiftKey()) {
        do {
            rc = m_cObjectArray.GetDeny(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->Unselect();

            m_cObjectArray.ReleaseDeny(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else if ((GetVisibleArea() == pSprite->GetArea()
                   || GetGroup()->GetCount() == 0)
        && pSprite->Orderable(FALSE)
        && pSprite->GetArea() != NULL) {
        if (GetVisibleArea() != pSprite->GetArea()) {
            if (GetVisibleArea() != NULL) {
                GetVisibleArea()->m_bPicked = FALSE;
                GetVisibleArea()->m_iPicked = -1;
                GetVisibleArea()->m_nToolTip = 0;
                GetVisibleArea()->OnDeactivation();
            }

            m_visibleArea = pSprite->GetArea()->GetId();

            GetVisibleArea()->OnActivation();
        }

        do {
            rc = m_cObjectArray.GetDeny(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->Select();

            m_cObjectArray.ReleaseDeny(characterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (bPlaySelectSound && pSprite->Orderable(FALSE)) {
        pSprite->PlaySound(CGameSprite::SOUND_SELECT, TRUE, FALSE, FALSE);
    }

    m_cObjectArray.ReleaseShare(characterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    if (nPortrait != -1) {
        UpdatePortrait(nPortrait, 1);
    }

    return TRUE;
}

// 0x5ADAE0
void CInfGame::SelectToolbar()
{
    m_nState = 0;
    m_cButtonArray.m_nSelectedButton = 100;

    // NOTE: Uninline.
    switch (m_group.GetCount()) {
    case 0:
        m_cButtonArray.SetState(CInfButtonArray::STATE_NONE, 0);
        break;
    case 1:
        if (1) {
            LONG* pGroupList = m_group.GetGroupList();
            if (pGroupList == NULL) {
                m_cButtonArray.SetState(CInfButtonArray::STATE_NONE, 0);
                delete pGroupList;
                return;
            }

            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = m_cObjectArray.GetShare(pGroupList[0],
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                m_cButtonArray.SetState(CInfButtonArray::STATE_NONE, 0);
                delete pGroupList;
                return;
            }

            if (IcewindMisc::IsPC(pSprite) == TRUE) {
                m_cButtonArray.UpdateCustomizableButtonTypes(pSprite->field_3D14);
                m_cButtonArray.UpdateSelectedWeaponSet(pSprite->m_nWeaponSet);
                m_cButtonArray.SetState(CInfButtonArray::byte_85170E, 0);
            } else {
                m_cButtonArray.UpdateSelectedWeaponSet(pSprite->m_nWeaponSet);
                m_cButtonArray.SetState(CInfButtonArray::byte_851701, 0);
            }

            m_cObjectArray.ReleaseShare(pGroupList[0],
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            delete pGroupList;
        }
        break;
    default:
        m_cButtonArray.SetState(CInfButtonArray::byte_851701, 0);
        break;
    }

    if (g_pBaldurChitin->m_pEngineWorld->GetActivePanelID() == 8) {
        g_pBaldurChitin->m_pEngineWorld->StopContainer();
    }
}

// 0x5ADC90
void CInfGame::DemandServerStore(const CResRef& store, BOOL bSaveToDisk)
{
    int cnt;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 9987
    UTIL_ASSERT(g_pChitin->cNetwork.GetSessionOpen() && g_pChitin->cNetwork.GetSessionHosting());

    for (cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL
            && store == m_aServerStore[cnt]->m_resRef) {
            if (bSaveToDisk) {
                m_aServerStore[cnt]->Marshal(m_sTempDir);
            }

            m_nServerStoreDemands[cnt]++;

            return;
        }
    }

    for (cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] == NULL) {
            m_aServerStore[cnt] = new CStore(store);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 10007
            UTIL_ASSERT(m_aServerStore[cnt] != NULL);

            if (m_aServerStore[cnt]->m_resRef == "") {
                delete m_aServerStore[cnt];
                m_aServerStore[cnt] = NULL;
            } else {
                m_nServerStoreDemands[cnt] = 1;
            }

            return;
        }
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 10019
    UTIL_ASSERT(FALSE);
}

// 0x5ADE90
void CInfGame::ReleaseServerStore(const CResRef& store)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 10042
    UTIL_ASSERT(g_pChitin->cNetwork.GetSessionOpen() && g_pChitin->cNetwork.GetSessionHosting());

    for (int cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL
            && store == m_aServerStore[cnt]->m_resRef) {
            m_nServerStoreDemands[cnt]--;

            if (m_nServerStoreDemands[cnt] == 0) {
                m_aServerStore[cnt]->Marshal(m_sTempDir);

                delete m_aServerStore[cnt];

                m_aServerStore[cnt] = NULL;
            }

            return;
        }
    }
}

// 0x5ADF60
CStore* CInfGame::GetServerStore(const CResRef& store)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 10079
    UTIL_ASSERT(g_pChitin->cNetwork.GetSessionOpen() && g_pChitin->cNetwork.GetSessionHosting());

    for (int cnt = 0; cnt < 12; cnt++) {
        if (m_aServerStore[cnt] != NULL
            && store == m_aServerStore[cnt]->m_resRef) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 10084
            UTIL_ASSERT(m_nServerStoreDemands[cnt] > 0);

            return m_aServerStore[cnt];
        }
    }

    return NULL;
}

// 0x5AF160
void CInfGame::SwapCharacters(DWORD pos1, DWORD pos2)
{
    CGameSprite* pSprite;
    BYTE rc;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 10546
    UTIL_ASSERT(pos1 < (DWORD)m_nCharacters && pos2 < (DWORD)m_nCharacters);

    LONG id2 = m_characterPortraits[pos2];
    LONG id1 = m_characterPortraits[pos1];
    m_characterPortraits[pos1] = id2;
    m_characterPortraits[pos2] = id1;

    do {
        rc = g_pBaldurChitin->GetObjectGame()->m_cObjectArray.GetDeny(id1,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        pSprite->FetchCommonStrings();
        UpdatePortraitToolTip(pos2, pSprite->GetNameRef());

        g_pBaldurChitin->GetObjectGame()->m_cObjectArray.ReleaseDeny(id1,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        do {
            rc = g_pBaldurChitin->GetObjectGame()->m_cObjectArray.GetDeny(id2,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->FetchCommonStrings();
            UpdatePortraitToolTip(pos1, pSprite->GetNameRef());

            g_pBaldurChitin->GetObjectGame()->m_cObjectArray.ReleaseDeny(id2,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            // NOTE: `2` is the sound constant at `0x85BB65`.
            if (pos1 == 0) {
                CMessage* message = new CMessagePlaySound(2,
                    TRUE,
                    FALSE,
                    m_characterPortraits[0],
                    m_characterPortraits[0]);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            } else if (pos2 == 0) {
                CMessage* message = new CMessagePlaySound(2,
                    TRUE,
                    FALSE,
                    m_characterPortraits[0],
                    m_characterPortraits[0]);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    }
}

// 0x5AF360
void CInfGame::UpdatePortrait(SHORT nPortrait, DWORD dwPanelId)
{
    if (nPortrait != -1) {
        CBaldurEngine* pEngine = g_pBaldurChitin->GetActiveEngine();
        if (pEngine != g_pBaldurChitin->m_pEngineProjector
            && pEngine != g_pBaldurChitin->m_pEngineMultiPlayer
            && pEngine != g_pBaldurChitin->m_pEngineSinglePlayer
            && pEngine != g_pBaldurChitin->m_pEngineWorldMap
            && pEngine != g_pBaldurChitin->m_pEngineSave) {
            CUIManager* pManager = pEngine->GetManager();
            CUIPanel* pPanel = pManager->GetPanel(dwPanelId);
            if (pPanel != NULL) {
                CUIControlBase* pControl = pPanel->GetControl(nPortrait);
                if (pControl != NULL) {
                    pControl->InvalidateRect();
                }
            } else {
                // NOTE: Obtaining active engine second time, probably some
                // inlining. Looks similar to `CBaldurEngine::OnPortraitLClick`.
                CUIManager* pManager = g_pBaldurChitin->GetActiveEngine()->GetManager();
                CUIPanel* pPanel = pManager->GetPanel(1);
                if (pPanel != NULL) {
                    CUIControlBase* pControl = pPanel->GetControl(nPortrait);
                    if (pControl != NULL) {
                        pControl->InvalidateRect();
                    }
                }
            }
        }
    }
}

// 0x5AF420
void CInfGame::sub_5AF420(SHORT nPortrait, DWORD dwPanelId)
{
    if (nPortrait != -1) {
        CBaldurEngine* pEngine = g_pBaldurChitin->GetActiveEngine();
        if (pEngine != g_pBaldurChitin->m_pEngineProjector
            && pEngine != g_pBaldurChitin->m_pEngineMultiPlayer
            && pEngine != g_pBaldurChitin->m_pEngineSinglePlayer
            && pEngine != g_pBaldurChitin->m_pEngineWorldMap
            && pEngine != g_pBaldurChitin->m_pEngineSave) {
            CUIManager* pManager = pEngine->GetManager();
            CUIPanel* pPanel = pManager->GetPanel(dwPanelId);
            if (pPanel != NULL) {
                CUIControlBase* pControl = pPanel->GetControl(nPortrait);
                if (pControl != NULL) {
                    pControl->func_54();
                }
            } else {
                CUIManager* pManager = g_pBaldurChitin->GetActiveEngine()->GetManager();
                CUIPanel* pPanel = pManager->GetPanel(1);
                if (pPanel != NULL) {
                    CUIControlBase* pControl = pPanel->GetControl(nPortrait);
                    if (pControl != NULL) {
                        pControl->func_54();
                    }
                }
            }
        }
    }
}

// 0x5AF4E0
SHORT CInfGame::GetCharacterPortraitNum(LONG nCharacterId)
{
    if (nCharacterId != -1) {
        for (SHORT nPortrait = 0; nPortrait < 6; nPortrait++) {
            if (m_characterPortraits[nPortrait] == nCharacterId) {
                return nPortrait;
            }
        }
    }

    return -1;
}

// 0x5AF510
SHORT CInfGame::GetFixedOrderCharacterPortraitNum(LONG nCharacterId)
{
    if (nCharacterId != -1) {
        for (SHORT nPortrait = 0; nPortrait < 6; nPortrait++) {
            if (m_characters[nPortrait] == nCharacterId) {
                return nPortrait;
            }
        }
    }

    return -1;
}

// 0x5AF700
void CInfGame::RemoveFamiliarResRef(const CResRef& resRef, BYTE nAlignment, BYTE nLevel)
{
    if (nAlignment < 9 && nLevel < 9) {
        POSITION pos = m_familiarResRefs[nAlignment][nLevel].GetHeadPosition();
        while (pos != NULL) {
            POSITION curr = pos;
            CResRef* pResRef = m_familiarResRefs[nAlignment][nLevel].GetNext(pos);
            if (*pResRef == resRef) {
                m_familiarResRefs[nAlignment][nLevel].RemoveAt(curr);
                break;
            }
        }
    }
}

// 0x5AF770
void CInfGame::RenderPortrait(DWORD portraitId, const CPoint& cpRenderPosition, const CSize& szControl, BOOL bPressed, BOOL reorderHighlight, BOOL selectFromMarker, const CRect& rClip, BOOL bDoubleSize)
{
    LONG iSprite = GetCharacterId(static_cast<SHORT>(portraitId));
    if (iSprite != CGameObjectArray::INVALID_INDEX) {
        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = m_cObjectArray.GetShare(iSprite,
                CGameObjectArray::THREAD_1,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            pSprite->RenderPortrait(cpRenderPosition,
                szControl,
                bPressed,
                reorderHighlight,
                selectFromMarker,
                rClip,
                bDoubleSize);

            m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_1,
                INFINITE);
        }
    }
}

// 0x5AF7F0
void CInfGame::WorldEngineActivated(CVidMode* pVidMode)
{
    m_nState = 0;
    m_cButtonArray.m_nSelectedButton = 100;
    m_cVRamPool.DetachSurfaces();
    m_cVRamPool.AttachSurfaces(pVidMode);
    m_gameAreas[m_visibleArea]->OnActivation();
    m_cButtonArray.ResetState();
}

// 0x5AF850
void CInfGame::WorldEngineDeactivated()
{
    if (m_gameAreas[m_visibleArea] != NULL) {
        m_gameAreas[m_visibleArea]->OnDeactivation();
    }
}

// 0x5AF870
void CInfGame::OnPortraitLClick(DWORD id)
{
    if (m_nState != 0 && GetVisibleArea()->m_iPicked != -1) {
        CGameObject* pObject;
        BYTE rc = m_cObjectArray.GetShare(GetVisibleArea()->m_iPicked,
            CGameObjectArray::THREAD_ASYNCH,
            &pObject,
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (m_nState == 1) {
                GetVisibleArea()->OnActionButtonClickGround(pObject->GetPos());
            } else {
                pObject->OnActionButton(pObject->GetPos());
            }
            m_cObjectArray.ReleaseShare(GetVisibleArea()->m_iPicked,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    } else {
        if (static_cast<SHORT>(id) < GetNumCharacters()) {
            CGameSprite* pSprite;
            LONG nCharacterId = m_characterPortraits[id];
            BYTE rc = m_cObjectArray.GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
            if (rc == CGameObjectArray::SUCCESS) {
                if (!pSprite->InControl()) {
                    if (g_pBaldurChitin->GetObjectGame()->GetGroup()->GetCount() > 0
                        && pSprite->GetArea() == g_pBaldurChitin->GetObjectGame()->GetVisibleArea()) {
                        CAIAction action(CAIAction::MOVETOOBJECTFOLLOW, pSprite->GetAIType(), 0, 0, 0);
                        g_pBaldurChitin->GetObjectGame()->GetGroup()->GroupAction(action, TRUE, NULL);
                    } else if (pSprite->GetArea() != NULL) {
                        UnselectAll();

                        if (GetVisibleArea() != NULL) {
                            GetVisibleArea()->m_bPicked = FALSE;
                            GetVisibleArea()->m_iPicked = -1;
                            GetVisibleArea()->m_nToolTip = 0;
                            GetVisibleArea()->OnDeactivation();
                        }

                        m_visibleArea = pSprite->GetArea()->m_id;

                        GetVisibleArea()->OnActivation();
                    }
                }

                m_cObjectArray.ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            if (!g_pBaldurChitin->GetScreenWorld()->GetCtrlKey()
                && !g_pBaldurChitin->GetScreenWorld()->GetShiftKey()) {
                UnselectAll();
            }

            SelectCharacter(m_characterPortraits[id], TRUE);
            SelectToolbar();
        }
    }
}

// 0x5AFCB0
void CInfGame::OnPortraitLDblClick(DWORD id)
{
    LONG iSprite = GetCharacterId(static_cast<SHORT>(id));
    if (iSprite != CGameObjectArray::INVALID_INDEX) {
        CGameSprite* pSprite;

        BYTE rc = m_cObjectArray.GetShare(iSprite,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);

        if (rc == CGameObjectArray::SUCCESS) {
            CGameArea* pArea = pSprite->GetArea();
            if (pArea != NULL) {
                CRect rView(pArea->GetInfinity()->rViewPort);
                INT x = rView.Width() / -2;
                INT y = rView.Height() / -2;

                CPoint& pos = pSprite->GetPos();

                pArea->GetInfinity()->SetViewPosition(x + pos.x, y + pos.y, TRUE);
            }

            m_cObjectArray.ReleaseShare(iSprite,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5AFD90
void CInfGame::EnablePortrait(BYTE nControlId, BOOL bEnable)
{
    g_pBaldurChitin->m_pEngineCharacter->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineInventory->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineJournal->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineMap->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineOptions->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineSpellbook->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineStore->EnablePortrait(1, nControlId, bEnable);
    g_pBaldurChitin->m_pEngineWorld->EnablePortrait(1, nControlId, bEnable);
}

// 0x5AFE60
BOOLEAN CInfGame::ExtendedDayNightAreaActive()
{
    for (int index = 0; index < CINFGAME_MAX_AREAS; index++) {
        if (m_gameAreas[index] != NULL && (m_gameAreas[index]->m_header.m_areaType & 0x40) != 0) {
            return TRUE;
        }
    }

    return FALSE;
}

// 0x5AFE90
void CInfGame::UpdatePortraitToolTip(DWORD dwControlId, STRREF dwStrId)
{
    CUIManager* pManager;
    CUIPanel* pPanel;
    CUIControlBase* pControl;

    pManager = g_pBaldurChitin->m_pEngineCharacter->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineInventory->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineJournal->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineMap->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineOptions->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineSpellbook->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineStore->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);

    pManager = g_pBaldurChitin->m_pEngineWorld->GetManager();
    pPanel = pManager->GetPanel(1);
    pControl = pPanel->GetControl(dwControlId);
    pControl->SetToolTipStrRef(dwStrId, -1, -1);
}

// 0x5B0020
void CInfGame::StepAnimation(BYTE direction)
{
    // FIXME: Should be static global.
    WORD animationList[ANIM_TYPE_NUM] = {
        ANIM_DEATH_KNIGHT,
        ANIM_STATIC_NOBLE_MAN_CHAIR,
        ANIM_STATIC_NOBLE_MAN_MATTE,
        ANIM_STATIC_NOBLE_WOMAN_CHAIR,
        ANIM_STATIC_NOBLE_WOMAN_MATTE,
        ANIM_STATIC_PEASANT_MAN_CHAIR,
        ANIM_STATIC_PEASANT_MAN_STOOL,
        ANIM_STATIC_PEASANT_MAN_MATTE,
        ANIM_STATIC_PEASANT_WOMAN_CHAIR,
        ANIM_STATIC_PEASANT_WOMAN_MATTE,
        ANIM_STATIC_HUMANCLERIC_MAN_CHAIR,
        ANIM_SLEEPING_MAN_HUMAN,
        ANIM_SLEEPING_WOMAN_HUMAN,
        ANIM_SLEEPING_FAT_MAN_HUMAN,
        ANIM_SLEEPING_DWARF,
        ANIM_SLEEPING_MAN_ELF,
        ANIM_SLEEPING_WOMAN_ELF,
        ANIM_SLEEPING_MAN_HALFLING,
        ANIM_CLERIC_MALE_HUMAN,
        ANIM_CLERIC_MALE_ELF,
        ANIM_CLERIC_MALE_DWARF,
        ANIM_CLERIC_MALE_HALFLING,
        ANIM_CLERIC_MALE_GNOME,
        ANIM_CLERIC_MALE_HALFORC,
        ANIM_CLERIC_FEMALE_HUMAN,
        ANIM_CLERIC_FEMALE_ELF,
        ANIM_CLERIC_FEMALE_DWARF,
        ANIM_CLERIC_FEMALE_HALFLING,
        ANIM_CLERIC_FEMALE_GNOME,
        ANIM_CLERIC_FEMALE_HALFORC,
        ANIM_FIGHTER_MALE_HUMAN,
        ANIM_FIGHTER_MALE_ELF,
        ANIM_FIGHTER_MALE_DWARF,
        ANIM_FIGHTER_MALE_HALFLING,
        ANIM_FIGHTER_MALE_GNOME,
        ANIM_FIGHTER_MALE_HALFORC,
        ANIM_FIGHTER_FEMALE_HUMAN,
        ANIM_FIGHTER_FEMALE_ELF,
        ANIM_FIGHTER_FEMALE_DWARF,
        ANIM_FIGHTER_FEMALE_HALFLING,
        ANIM_FIGHTER_FEMALE_GNOME,
        ANIM_FIGHTER_FEMALE_HALFORC,
        ANIM_MAGE_MALE_HUMAN,
        ANIM_MAGE_MALE_ELF,
        ANIM_MAGE_MALE_DWARF,
        ANIM_MAGE_MALE_GNOME,
        ANIM_MAGE_MALE_HALFORC,
        ANIM_MAGE_FEMALE_HUMAN,
        ANIM_MAGE_FEMALE_ELF,
        ANIM_MAGE_FEMALE_DWARF,
        ANIM_MAGE_FEMALE_GNOME,
        ANIM_MAGE_FEMALE_HALFORC,
        ANIM_THIEF_MALE_HUMAN,
        ANIM_THIEF_MALE_ELF,
        ANIM_THIEF_MALE_DWARF,
        ANIM_THIEF_MALE_HALFLING,
        ANIM_THIEF_MALE_GNOME,
        ANIM_THIEF_MALE_HALFORC,
        ANIM_THIEF_FEMALE_HUMAN,
        ANIM_THIEF_FEMALE_ELF,
        ANIM_THIEF_FEMALE_DWARF,
        ANIM_THIEF_FEMALE_HALFLING,
        ANIM_THIEF_FEMALE_GNOME,
        ANIM_THIEF_FEMALE_HALFORC,
        ANIM_MONK_MALE_HUMAN,
        ANIM_MONK_FEMALE_HUMAN,
        ANIM_MONK,
        ANIM_SKELETON,
        ANIM_DOOM_GUARD,
        ANIM_BEAR_BLACK,
        ANIM_BEAR_BROWN,
        ANIM_BEAR_CAVE,
        ANIM_BEAR_POLAR,
        ANIM_SPIDER_GIANT,
        ANIM_SPIDER_HUGE,
        ANIM_SPIDER_PHASE,
        ANIM_SPIDER_SWORD,
        ANIM_SPIDER_WRAITH,
        ANIM_WOLF,
        ANIM_WOLF_WINTER,
        ANIM_WEREWOLF,
        ANIM_WEREWOLF_GREATER,
        ANIM_OGRE,
        ANIM_CARRION_CRAWLER,
        ANIM_COW,
        ANIM_STATIC_BEGGAR_MAN,
        ANIM_STATIC_HARLOT_WOMAN,
        ANIM_STATIC_BOY,
        ANIM_STATIC_GIRL,
        ANIM_STATIC_FAT_MAN,
        ANIM_STATIC_FAT_WOMAN,
        ANIM_STATIC_PEASANT_MAN,
        ANIM_STATIC_PEASANT_WOMAN,
        ANIM_STATIC_NOBLE_MAN,
        ANIM_STATIC_NOBLE_WOMAN,
        ANIM_STATIC_SLAVE,
        ANIM_BAT_INSIDE,
        ANIM_CAT,
        ANIM_CHICKEN,
        ANIM_RAT,
        ANIM_SQUIRREL,
        ANIM_BAT_OUTSIDE,
        ANIM_BEGGAR_MAN,
        ANIM_HARLOT_WOMAN,
        ANIM_BOY,
        ANIM_GIRL,
        ANIM_FAT_MAN,
        ANIM_FAT_WOMAN,
        ANIM_PEASANT_MAN,
        ANIM_PEASANT_WOMAN,
        ANIM_NOBLE_MAN,
        ANIM_NOBLE_WOMAN,
        ANIM_SLAVE,
        ANIM_EAGLE,
        ANIM_VULTURE,
        ANIM_BIRD,
        ANIM_BIRD_INSIDE,
        ANIM_BEETLE,
        ANIM_BEETLE,
        ANIM_BEETLE_BOMBADIER,
        ANIM_BEETLE_BORING,
        ANIM_BEETLE_FIRE,
        ANIM_BEETLE_RHINOCEROUS,
        ANIM_CYCLOPS,
        ANIM_ETTIN,
        ANIM_GIANT_FIRE,
        ANIM_GIANT_FROST,
        ANIM_VERBEEG,
        ANIM_ELEMENTAL_EARTH,
        ANIM_ELEMENTAL_FIRE,
        ANIM_ELEMENTAL_WATER,
        ANIM_GHOUL,
        ANIM_GHOUL_2,
        ANIM_WIGHT,
        ANIM_ZOMBIE,
        ANIM_ZOMBIE_2,
        ANIM_WIGHT_2,
        ANIM_WIGHT_3,
        ANIM_MUMMY,
        ANIM_MUMMY_HISTACHII,
        ANIM_GOBLIN_W_AXE,
        ANIM_GOBLIN_W_BOW,
        ANIM_GOBLIN_ELITE_W_AXE,
        ANIM_GOBLIN_ELITE_W_BOW,
        ANIM_SVIRFNEBLIN,
        ANIM_GOLEM_IRON,
        ANIM_SKELETON_BONEGUARD,
        ANIM_MINOTAUR,
        ANIM_TROLL,
        ANIM_TROLL_ICE,
        ANIM_TROLL_SNOW,
        ANIM_UMBER_HULK,
        ANIM_TUNDRA_YETI,
        ANIM_ORC_W_AXE,
        ANIM_ORC_W_BOW,
        ANIM_ORC_ELITE_W_AXE,
        ANIM_ORC_ELITE_W_BOW,
        ANIM_ORC_SHAMAN,
        ANIM_OROG,
        ANIM_OROG_ELITE,
        ANIM_OROG_CHIEFTAN,
        ANIM_LIZARD_MAN_KING,
        ANIM_YUAN_TI_PRIEST,
        ANIM_SHADOW,
        ANIM_GHOST,
        ANIM_ANIMATED_PLATE,
        ANIM_ANIMATED_PLATE_2,
        ANIM_ANIMATED_PLATE_3,
        ANIM_BELHIFET,
        ANIM_BELHIFET_PRIEST,
        ANIM_LICH_LARREL,
        ANIM_MYCONID_RED,
        ANIM_MYCONID_BLUE,
        ANIM_SHRIEKER,
        ANIM_TANARI,
        ANIM_SALAMANDER,
        ANIM_SALAMANDER_FROST,
        ANIM_SKELETON_ARMORED,
        ANIM_YUAN_TI,
        ANIM_YUAN_TI_ELITE,
        ANIM_SKELETON_TATTERED,
        ANIM_LIZARD_MAN_2H,
        ANIM_LIZARD_MAN_1H,
        ANIM_ARUNDEL,
        ANIM_CREEPING_DOOM,
        ANIM_LICH_WHITE,
        ANIM_SVIRVNEBLIN_NOAXE,
        ANIM_BARBARIAN_1,
        ANIM_BARBARIAN_2,
        ANIM_BARBARIAN_3,
        ANIM_BARBARIAN_SHAMAN_1,
        ANIM_BARBARIAN_SHAMAN_2,
        ANIM_BARBARIAN_SHAMAN_3,
        ANIM_BARBARIAN_ICASARACHT,
        ANIM_CORNUGON,
        ANIM_DRAGON_WHITE,
        ANIM_DROWNED_DEAD,
        ANIM_GLABREZU,
        ANIM_GOLEM_ICE,
        ANIM_REMORHAZ,
        ANIM_SAHUAGIN,
        ANIM_SAHUAGIN_LARGE,
        ANIM_SEER,
        ANIM_ELEMENTAL_SHAMBLING_MOUND,
        ANIM_TROLL_SCRAG,
        ANIM_UMBER_HULK_VODYANOI,
        ANIM_WAILING_VIRGIN,
        ANIM_WATER_WEIRD,
        ANIM_WIGHT_BARROW,
        ANIM_WYVERN,
        ANIM_WYVERN_BIG_WHITE,
        ANIM_DOG_WILD,
        ANIM_DOG_WAR,
        ANIM_DOG_MOON,
        ANIM_HARPY,
        ANIM_BEHOLDER,
        ANIM_RAKSHASA,
        ANIM_SLIME_GREEN,
        ANIM_SLIME_OLIVE,
        ANIM_SLIME_MUSTARD,
        ANIM_SLIME_OCHRE,
        ANIM_DEER,
        ANIM_DJINNI,
        ANIM_DJINNI_LEGS,
        ANIM_DOPPLEGANGER,
        ANIM_DOPPLEGANGER_GREATER,
        ANIM_DRAGON_BLACK,
        ANIM_ELEMENTAL_AIR,
        ANIM_ELEMENTAL_AIR_SMALL,
        ANIM_ELEMENTAL_FIRE_BG2,
        ANIM_ETTERCAP,
        ANIM_GNOLL,
        ANIM_GOLEM_CLAY,
        ANIM_GOLEM_IRON_BG2,
        ANIM_GREATCAT_PANTHER,
        ANIM_GREATCAT_LEOPARD,
        ANIM_HOBGOBLIN,
        ANIM_IMP,
        ANIM_MINDFLAYER,
        ANIM_OGRE_HALF,
        ANIM_OTYUGH,
        ANIM_RAVER,
        ANIM_SLAYER,
        ANIM_SNAKE,
        ANIM_SPIDER_SMALL,
        ANIM_ABISHAI_WHITE,
        ANIM_BOAR_ARCTIC,
        ANIM_BUGBEAR,
        ANIM_BUGBEAR_CAPTAIN,
        ANIM_CHIMERA,
        ANIM_DARKTREE,
        ANIM_DRAGON_WHITE_BABY,
        ANIM_DRIDER_MALE,
        ANIM_DRIDER_FEMALE,
        ANIM_DROW,
        ANIM_DROW_FEMALE,
        ANIM_ELEMENTAL_WATER_SMALL,
        ANIM_FEYR,
        ANIM_FEYR_GREATER,
        ANIM_GIANT_FOMORIAN,
        ANIM_GOBLIN_CAPTAIN,
        ANIM_GOBLIN_SHAMAN,
        ANIM_GOBLIN_WORG,
        ANIM_GOBLIN_WORG_CAPTAIN,
        ANIM_HALFDRAGON_BLACK,
        ANIM_HALFDRAGON_RED,
        ANIM_HOOK_HORROR,
        ANIM_INVISIBLE_TINY,
        ANIM_INVISIBLE_SMALL,
        ANIM_INVISIBLE_MEDIUM,
        ANIM_INVISIBLE_LARGE,
        ANIM_INVISIBLE_GIANT,
        ANIM_INVISIBLE_HUGE,
        ANIM_ISAIR,
        ANIM_KEG_1,
        ANIM_KEG_2,
        ANIM_KEG_3,
        ANIM_LEMURE,
        ANIM_MADAE,
        ANIM_MALARITE,
        ANIM_ORC_W_AXE_UNDEAD,
        ANIM_ORC_SHAMAN_UNDEAD,
        ANIM_OROG_SHAMAN,
        ANIM_SHENRICAL,
        ANIM_WEREBADGER,
        ANIM_WERERAT,
        ANIM_WORG,
        ANIM_WILL_O_WISP,
        ANIM_YUAN_TI_HALF,
    };

    // FIXME: Should be static global.
    CString animationListNames[ANIM_TYPE_NUM] = {
        "DEATH_KNIGHT",
        "STATIC_NOBLE_MAN_CHAIR",
        "STATIC_NOBLE_MAN_MATTE",
        "STATIC_NOBLE_WOMAN_CHAIR",
        "STATIC_NOBLE_WOMAN_MATTE",
        "STATIC_PEASANT_MAN_CHAIR",
        "STATIC_PEASANT_MAN_STOOL",
        "STATIC_PEASANT_MAN_MATTE",
        "STATIC_PEASANT_WOMAN_CHAIR",
        "STATIC_PEASANT_WOMAN_MATTE",
        "STATIC_HUMANCLERIC_MAN_CHAIR",
        "SLEEPING_MAN_HUMAN",
        "SLEEPING_WOMAN_HUMAN",
        "SLEEPING_FAT_MAN_HUMAN",
        "SLEEPING_DWARF",
        "SLEEPING_MAN_ELF",
        "SLEEPING_WOMAN_ELF",
        "SLEEPING_MAN_HALFLING",
        "CLERIC_MALE_HUMAN",
        "CLERIC_MALE_ELF",
        "CLERIC_MALE_DWARF",
        "CLERIC_MALE_HALFLING",
        "CLERIC_MALE_GNOME",
        "CLERIC MALE HALFORC",
        "CLERIC_FEMALE_HUMAN",
        "CLERIC_FEMALE_ELF",
        "CLERIC_FEMALE_DWARF",
        "CLERIC_FEMALE_HALFLING",
        "CLERIC_FEMALE_GNOME",
        "CLERIC FEMALE HALFORC",
        "FIGHTER_MALE_HUMAN",
        "FIGHTER_MALE_ELF",
        "FIGHTER_MALE_DWARF",
        "FIGHTER_MALE_HALFLING",
        "FIGHTER_MALE_GNOME",
        "FIGHTER MALE HALFORC",
        "FIGHTER_FEMALE_HUMAN",
        "FIGHTER_FEMALE_ELF",
        "FIGHTER_FEMALE_DWARF",
        "FIGHTER_FEMALE_HALFLING",
        "FIGHTER_FEMALE_GNOME",
        "FIGHTER FEMALE HALFORC",
        "MAGE_MALE_HUMAN",
        "MAGE_MALE_ELF",
        "MAGE_MALE_DWARF",
        "MAGE_MALE_GNOME",
        "MAGE MALE HALFORC",
        "MAGE_FEMALE_HUMAN",
        "MAGE_FEMALE_ELF",
        "MAGE_FEMALE_DWARF",
        "MAGE_FEMALE_GNOME",
        "MAGE FEMALE HALFORC",
        "THIEF_MALE_HUMAN",
        "THIEF_MALE_ELF",
        "THIEF_MALE_DWARF",
        "THIEF_MALE_HALFLING",
        "THIEF_MALE_GNOME",
        "THIEF MALE HALFORC",
        "THIEF_FEMALE_HUMAN",
        "THIEF_FEMALE_ELF",
        "THIEF_FEMALE_DWARF",
        "THIEF_FEMALE_HALFLING",
        "THIEF_FEMALE_GNOME",
        "THIEF FEMALE HALFORC",
        "MONK MALE HUMAN",
        "MONK FEMALE HUMAN",
        "MONK",
        "SKELETON",
        "DOOM_GUARD",
        "BEAR_BLACK",
        "BEAR_BROWN",
        "BEAR_CAVE",
        "BEAR_POLAR",
        "SPIDER_GIANT",
        "SPIDER_HUGE",
        "SPIDER_PHASE",
        "SPIDER_SWORD",
        "SPIDER_WRAITH",
        "WOLF",
        "WOLF_WINTER",
        "WEREWOLF",
        "WEREWOLF_GREATER",
        "OGRE",
        "CARRION_CRAWLER",
        "COW",
        "STATIC_BEGGAR_MAN",
        "STATIC_HARLOT_WOMAN",
        "STATIC_BOY",
        "STATIC_GIRL",
        "STATIC_FAT_MAN",
        "STATIC_FAT_WOMAN",
        "STATIC_PEASANT_MAN",
        "STATIC_PEASANT_WOMAN",
        "STATIC_NOBLE_MAN",
        "STATIC_NOBLE_WOMAN",
        "STATIC_SLAVE",
        "BAT_INSIDE",
        "CAT",
        "CHICKEN",
        "RAT",
        "SQUIRREL",
        "BAT_OUTSIDE",
        "BEGGAR_MAN",
        "HARLOT_WOMAN",
        "BOY",
        "GIRL",
        "FAT_MAN",
        "FAT_WOMAN",
        "PEASANT_MAN",
        "PEASANT_WOMAN",
        "NOBLE_MAN",
        "NOBLE_WOMAN",
        "SLAVE",
        "EAGLE",
        "VULTURE",
        "BIRD",
        "BIRD_INSIDE",
        "BEETLE",
        "BEETLE",
        "BEETLE_BOMBADIER",
        "BEETLE_BORING",
        "BEETLE_FIRE",
        "BEETLE_RHINOCEROUS",
        "CYCLOPS",
        "ETTIN",
        "GIANT_FIRE",
        "GIANT_FROST",
        "VERBEEG",
        "ELEMENTAL_EARTH",
        "ELEMENTAL_FIRE",
        "ELEMENTAL_WATER",
        "GHOUL",
        "GHOUL_2",
        "WIGHT",
        "ZOMBIE",
        "ZOMBIE_2",
        "WIGHT_2",
        "WIGHT_3",
        "MUMMY",
        "MUMMY_HISTACHII",
        "GOBLIN_W_AXE",
        "GOBLIN_W_BOW",
        "GOBLIN_ELITE_W_AXE",
        "GOBLIN_ELITE_W_BOW",
        "SVIRFNEBLIN",
        "GOLEM_IRON",
        "SKELETON_BONEGUARD",
        "MINOTAUR",
        "TROLL",
        "TROLL_ICE",
        "TROLL_SNOW",
        "UMBER_HULK",
        "TUNDRA_YETI",
        "ORC_W_AXE",
        "ORC_W_BOW",
        "ORC_ELITE_W_AXE",
        "ORC_ELITE_W_BOW",
        "ORC_SHAMAN",
        "OROG",
        "OROG_ELITE",
        "OROG_CHIEFTAN",
        "LIZARD_MAN_KING",
        "YUAN_TI_PRIEST",
        "SHADOW",
        "GHOST",
        "ANIMATED_PLATE",
        "ANIMATED_PLATE_2",
        "ANIMATED_PLATE_3",
        "BELHIFET",
        "BELHIFET_PRIEST",
        "LICH_LARREL",
        "MYCONID_RED",
        "MYCONID_BLUE",
        "SHRIEKER",
        "TANARI",
        "SALAMANDER",
        "SALAMANDER_FROST",
        "SKELETON_ARMORED",
        "YUAN_TI",
        "YUAN_TI_ELITE",
        "SKELETON_TATTERED",
        "LIZARD_MAN_2H",
        "LIZARD_MAN_1H",
        "ARUNDEL",
        "CREEPING_DOOM",
        "LICH_WHITE",
        "SVIRVNEBLIN_NOAXE",
        "BARBARIAN_1",
        "BARBARIAN_2",
        "BARBARIAN_3",
        "BARBARIAN_SHAMAN_1",
        "BARBARIAN_SHAMAN_2",
        "BARBARIAN_SHAMAN_3",
        "BARBARIAN_ICASARACHT",
        "CORNUGON",
        "DRAGON_WHITE",
        "DROWNED_DEAD",
        "GLABREZU",
        "GOLEM_ICE",
        "REMORHAZ",
        "SAHUAGIN",
        "SAHUAGIN_LARGE",
        "SEER",
        "ELEMENTAL_SHAMBLING_MOUND",
        "TROLL_SCRAG",
        "UMBER_HULK_VODYANOI",
        "WAILING_VIRGIN",
        "WATER_WEIRD",
        "WIGHT_BARROW",
        "WYVERN",
        "WYVERN_BIG_WHITE",
        "DOG_WILD",
        "DOG_WAR",
        "DOG_MOON",
        "HARPY",
        "BEHOLDER",
        "RAKSHASA",
        "SLIME_GREEN",
        "SLIME_OLIVE",
        "SLIME_MUSTARD",
        "SLIME_OCHRE",
        "DEER",
        "DJINNI",
        "DJINNI_LEGS",
        "DOPPLEGANGER",
        "DOPPLEGANGER_GREATER",
        "DRAGON_BLACK",
        "ELEMENTAL_AIR",
        "ELEMENTAL_AIR_SMALL",
        "ELEMENTAL_FIRE_BG2",
        "ETTERCAP",
        "GNOLL",
        "GOLEM_CLAY",
        "GOLEM_IRON_BG2",
        "GREATCAT_PANTHER",
        "GREATCAT_LEOPARD",
        "HOBGOBLIN",
        "IMP",
        "MINDFLAYER",
        "OGRE_HALF",
        "OTYUGH",
        "RAVER",
        "SLAYER",
        "SNAKE",
        "SPIDER_SMALL",
        "ABISHAI_WHITE",
        "BOAR_ARCTIC",
        "BUGBEAR",
        "BUGBEAR_CAPTAIN",
        "CHIMERA",
        "DARKTREE",
        "DRAGON_WHITE_BABY",
        "DRIDER_MALE",
        "DRIDER_FEMALE",
        "DROW",
        "DROW_FEMALE",
        "ELEMENTAL_WATER_SMALL",
        "FEYR",
        "FEYR_GREATER",
        "GIANT_FOMORIAN",
        "GOBLIN_CAPTAIN",
        "GOBLIN_SHAMAN",
        "GOBLIN_WORG",
        "GOBLIN_WORG_CAPTAIN",
        "HALFDRAGON_BLACK",
        "HALFDRAGON_RED",
        "HOOK_HORROR",
        "INVISIBLE_TINY",
        "INVISIBLE_SMALL",
        "INVISIBLE_MEDIUM",
        "INVISIBLE_LARGE",
        "INVISIBLE_GIANT",
        "INVISIBLE_HUGE",
        "ISAIR",
        "KEG_1",
        "KEG_2",
        "KEG_3",
        "LEMURE",
        "MADAE",
        "MALARITE",
        "ORC_W_AXE_UNDEAD",
        "ORC_SHAMAN_UNDEAD",
        "OROG_SHAMAN",
        "SHENRICAL",
        "WEREBADGER",
        "WERERAT",
        "WORG",
        "WILL_O_WISP",
        "YUAN_TI_HALF",
    };

    if (!m_group.m_memberList.IsEmpty()) {
        CRect rFx;
        CPoint ptRef;

        LONG nCharacterId = m_group.GetGroupLeader();

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = m_cObjectArray.GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            switch (direction) {
            case '1':
                m_currArmor++;
                if (m_currArmor > '4') {
                    m_currArmor = '1';
                }

                pSprite->GetAnimation()->EquipArmor(static_cast<CHAR>(m_currArmor),
                    pSprite->GetBaseStats()->m_colors);

                g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                    CString("Armor Level ") + static_cast<CHAR>(m_currArmor),
                    -1,
                    0);
                break;
            case '4':
                for (int index = 0; index < ANIM_TYPE_NUM; index++) {
                    pSprite->UnequipAll(FALSE);
                    pSprite->GetAnimation()->SetAnimationType(animationList[index],
                        pSprite->GetBaseStats()->m_colors,
                        0);
                    pSprite->EquipAll(FALSE);

                    for (SHORT nSequence = 1; nSequence <= 15; nSequence++) {
                        if (nSequence != 8) {
                            pSprite->GetAnimation()->SetSequence(nSequence);

                            for (SHORT nDirection = 0; nDirection <= 15; nDirection++) {
                                pSprite->GetAnimation()->ChangeDirection(nDirection);

                                for (INT nFrame = 0; nFrame < 40; nFrame++) {
                                    pSprite->GetAnimation()->CalculateFxRect(rFx, ptRef, pSprite->m_posZ);
                                    pSprite->GetAnimation()->IncrementFrame();
                                }
                            }
                        }
                    }
                }
                break;
            case '5':
                for (int index = 0; index < ANIM_TYPE_NUM; index++) {
                    pSprite->UnequipAll(FALSE);
                    pSprite->GetAnimation()->SetAnimationType(animationList[index],
                        pSprite->GetBaseStats()->m_colors,
                        0);
                    pSprite->EquipAll(FALSE);

                    for (SHORT nSequence = 1; nSequence <= 15; nSequence++) {
                        if (nSequence == 8) {
                            pSprite->GetAnimation()->SetSequence(nSequence);

                            for (SHORT nDirection = 0; nDirection <= 15; nDirection++) {
                                pSprite->GetAnimation()->ChangeDirection(nDirection);

                                for (INT nFrame = 0; nFrame < 40; nFrame++) {
                                    pSprite->GetAnimation()->CalculateFxRect(rFx, ptRef, pSprite->m_posZ);
                                    pSprite->GetAnimation()->IncrementFrame();
                                }
                            }
                        }
                    }
                }
                break;
            case '6':
                if (pSprite->GetVertListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.RemoveObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                                  pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->field_54A8,
                        pSprite->field_7430);
                }

                m_currAnimation--;
                if (m_currAnimation < 0) {
                    m_currAnimation = ANIM_TYPE_NUM - 1;
                }

                pSprite->UnequipAll(FALSE);
                pSprite->GetAnimation()->SetAnimationType(animationList[m_currAnimation],
                    pSprite->GetBaseStats()->m_colors,
                    pSprite->m_nDirection);
                pSprite->EquipAll(FALSE);

                pSprite->GetAnimation()->SetSequence(pSprite->m_nSequence);

                g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                    animationListNames[m_currAnimation],
                    -1,
                    FALSE);

                if (pSprite->GetVertListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.AddObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                               pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->field_54A8,
                        pSprite->field_7430);
                }
                break;
            case '7':
                if (pSprite->GetVertListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.RemoveObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                                  pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->field_54A8,
                        pSprite->field_7430);
                }

                m_currAnimation++;
                if (m_currAnimation == ANIM_TYPE_NUM) {
                    m_currAnimation = 0;
                }

                pSprite->UnequipAll(FALSE);
                pSprite->GetAnimation()->SetAnimationType(animationList[m_currAnimation],
                    pSprite->GetBaseStats()->m_colors,
                    pSprite->m_nDirection);
                pSprite->EquipAll(FALSE);

                pSprite->GetAnimation()->SetSequence(pSprite->m_nSequence);

                g_pBaldurChitin->m_pEngineWorld->DisplayText(CString(""),
                    animationListNames[m_currAnimation],
                    -1,
                    FALSE);

                if (pSprite->GetVertListType() != CGameObject::LIST_FLIGHT) {
                    pSprite->GetArea()->m_search.AddObject(CPoint(pSprite->GetPos().x / CPathSearch::GRID_SQUARE_SIZEX,
                                                               pSprite->GetPos().y / CPathSearch::GRID_SQUARE_SIZEY),
                        pSprite->GetAIType().m_nEnemyAlly,
                        pSprite->GetAnimation()->GetPersonalSpace(),
                        pSprite->field_54A8,
                        pSprite->field_7430);
                }
                break;
            }

            m_cObjectArray.ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// FIXME: Use reference instead of object instance.
//
// 0x5B7510
CGameArea* CInfGame::GetArea(CString sName)
{
    for (int index = 0; index < CINFGAME_MAX_AREAS; index++) {
        if (m_gameAreas[index] != NULL) {
            if (m_gameAreas[index]->m_resRef == sName) {
                return m_gameAreas[index];
            }
        }
    }

    return NULL;
}

// 0x68DE70
CGameArea* CInfGame::GetArea(SHORT nArea)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1416
    UTIL_ASSERT(nArea < CINFGAME_MAX_AREAS);

    return m_gameAreas[nArea];
}

// 0x5B75C0
LONG CInfGame::GetGroundPile(LONG iSprite)
{
    CGameSprite* pSprite;

    BYTE rc = m_cObjectArray.GetShare(iSprite,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pSprite),
        INFINITE);

    if (rc != CGameObjectArray::SUCCESS) {
        return CGameObjectArray::INVALID_INDEX;
    }

    CPoint ptSprite = pSprite->GetPos();
    LONG iGroundPile = pSprite->GetArea()->GetGroundPile(ptSprite);
    if (iGroundPile == CGameObjectArray::INVALID_INDEX) {
        ptSprite.x += CPathSearch::GRID_SQUARE_SIZEX / 2
            - ptSprite.x % CPathSearch::GRID_SQUARE_SIZEX;
        ptSprite.y += CPathSearch::GRID_SQUARE_SIZEY / 2
            - ptSprite.y % CPathSearch::GRID_SQUARE_SIZEY;

        CGameContainer* pContainer = new CGameContainer(pSprite->GetArea(),
            CRect(ptSprite.x - CPathSearch::GRID_SQUARE_SIZEX / 2,
                ptSprite.y - CPathSearch::GRID_SQUARE_SIZEY / 2,
                ptSprite.x + CPathSearch::GRID_SQUARE_SIZEX / 2 + 1,
                ptSprite.y + CPathSearch::GRID_SQUARE_SIZEY / 2 + 1));
        iGroundPile = pContainer->GetId();
    }

    m_cObjectArray.ReleaseShare(iSprite,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return iGroundPile;
}

// 0x5B7730
BOOL CInfGame::sub_5B7730(LONG nContainerId, SHORT nSlotNum, CGameSprite* pSprite, SHORT nPortraitNum)
{
    CItem* pItem;
    STRREF description;
    CResRef cResIcon;
    CResRef cResItem;
    WORD wCount;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    CGameContainer* pContainer;
    if (m_cObjectArray.GetDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pContainer),
            INFINITE)
        != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    CItem* pGroundItem = pContainer->GetItem(nSlotNum);

    m_cObjectArray.ReleaseDeny(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    if (pGroundItem == NULL) {
        return FALSE;
    }

    // Gold can always be picked up.
    if (pGroundItem->GetResRef() == "MISC07") {
        return TRUE;
    }

    INT nSlot = 18;
    do {
        pGame->InventoryInfoPersonal(nPortraitNum,
            nSlot,
            pItem,
            description,
            cResIcon,
            cResItem,
            wCount,
            TRUE);
        if (cResIcon != "") {
            nSlot++;
        }
    } while (cResIcon != "" && nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE + 18);

    return nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE + 18;
}

// 0x5B7850
BOOL CInfGame::SwapItemGround(LONG nContainerId, SHORT nSlotNum, CItem*& pNewItem, STRREF& errorCode, WORD wCount, BOOLEAN bFromServer)
{
    CGameContainer* pContainer = NULL;
    CItem* pOldItem;
    CMessage* message;

    BYTE rc = m_cObjectArray.GetDeny(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.m_idLocalPlayer != pContainer->m_remotePlayerID
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !bFromServer) {
        BOOLEAN bResult = g_pBaldurChitin->GetBaldurMessage()->SwapItemRequest(CBaldurMessage::SWAPITEM_LOCATION_GROUND,
            nContainerId,
            nSlotNum,
            pNewItem,
            errorCode,
            wCount);

        m_cObjectArray.ReleaseDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        return bResult;
    }

    pOldItem = pContainer->GetItem(nSlotNum);

    if (pOldItem != NULL
        && pNewItem == NULL
        && pOldItem->GetMaxStackable() > 1
        && wCount < pOldItem->GetUsageCount(0)) {
        // Split stack.

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 12276
        UTIL_ASSERT(wCount != 0);

        pOldItem->SetUsageCount(0, static_cast<WORD>(pOldItem->GetUsageCount(0) - wCount));

        pNewItem = new CItem(pOldItem->GetResRef(), 0, 0, 0, 0, 0);
        pNewItem->SetUsageCount(0, wCount);
        pNewItem->m_flags = (pNewItem->m_flags & ~0x1) | (pOldItem->m_flags & 0x1);
    } else if (pOldItem != NULL
        && pNewItem != NULL
        && pOldItem->GetMaxStackable() > 1
        && pNewItem->GetResRef() == pOldItem->GetResRef()
        && ((pNewItem->m_flags ^ pOldItem->m_flags) & 0x1) == 0) {
        // Merge stacks.

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 12286
        UTIL_ASSERT(wCount != 0);

        INT nCount = wCount;
        INT nSpace = pOldItem->GetMaxStackable() - pOldItem->GetUsageCount(0);
        if (nCount >= (pNewItem->GetUsageCount(0) < nSpace ? pNewItem->GetUsageCount(0) : nSpace)) {
            nSpace = pOldItem->GetMaxStackable() - pOldItem->GetUsageCount(0);
            nCount = pNewItem->GetUsageCount(0) < nSpace ? pNewItem->GetUsageCount(0) : nSpace;
        }

        pOldItem->SetUsageCount(0, static_cast<WORD>(pOldItem->GetUsageCount(0) + nCount));

        if (pNewItem->GetUsageCount(0) == static_cast<WORD>(nCount)) {
            AddDisposableItem(pNewItem);
            pNewItem = NULL;
        } else {
            pNewItem->SetUsageCount(0, static_cast<WORD>(pNewItem->GetUsageCount(0) - nCount));
        }
    } else if (pNewItem == NULL) {
        pContainer->SetItem(nSlotNum, NULL);

        message = new CMessageContainerAddItem(CItem(),
            nSlotNum,
            FALSE,
            pContainer->m_id,
            pContainer->m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        pNewItem = pOldItem;
    } else if (pOldItem != NULL && pOldItem->GetResRef() == "MISC07") {
        // Gold.
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
        pGame->AddPartyGold(pOldItem->GetUsageCount(0));
        pGame->AddDisposableItem(pOldItem);

        pContainer->SetItem(nSlotNum, NULL);

        message = new CMessageContainerAddItem(CItem(),
            nSlotNum,
            FALSE,
            pContainer->m_id,
            pContainer->m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        // NOTE: `pNewItem` is left intact.
    } else {
        message = new CMessageContainerAddItem(*pNewItem,
            nSlotNum,
            FALSE,
            pContainer->m_id,
            pContainer->m_id);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        g_pBaldurChitin->GetObjectGame()->AddDisposableItem(pNewItem);

        pNewItem = pOldItem;
    }

    m_cObjectArray.ReleaseDeny(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    errorCode = -1;
    return TRUE;
}

// 0x5B7FF0
SHORT CInfGame::GetNumGroundSlots(LONG nContainerId)
{
    SHORT nSlots;

    CGameContainer* pContainer;

    BYTE rc = m_cObjectArray.GetShare(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);

    if (rc == CGameObjectArray::SUCCESS) {
        nSlots = static_cast<SHORT>(pContainer->m_lstItems.GetCount());

        m_cObjectArray.ReleaseShare(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    } else {
        nSlots = 0;
    }

    return nSlots;
}

// 0x5B8040
void CInfGame::InventoryInfoGround(LONG nContainerId, SHORT nSlotNum, CItem*& pItem, STRREF& description, CResRef& cResIcon, CResRef& cResItem, WORD& wCount)
{
    description = -1;
    cResIcon = "";
    cResItem = "";
    wCount = 0;

    CGameContainer* pContainer;

    BYTE rc = m_cObjectArray.GetShare(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);

    if (rc == CGameObjectArray::SUCCESS) {
        pItem = pContainer->GetItem(nSlotNum);
        if (pItem != NULL) {
            description = pItem->GetGenericName();
            cResIcon = pItem->GetItemIcon();
            cResItem = pItem->GetResRef();

            if (pItem->GetMaxStackable() > 1) {
                wCount = pItem->GetUsageCount(0);
            }
        }

        m_cObjectArray.ReleaseShare(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x5B8130
WORD CInfGame::GetContainerType(LONG nContainerId)
{
    CGameContainer* pContainer;

    BYTE rc = m_cObjectArray.GetShare(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);
    if (rc != CGameObjectArray::SUCCESS) {
        return -1;
    }

    WORD nContainerType = pContainer->m_containerType;

    m_cObjectArray.ReleaseShare(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return nContainerType;
}

// 0x5B8180
void CInfGame::CompressContainer(LONG nContainerId)
{
    CGameContainer* pContainer;

    BYTE rc = m_cObjectArray.GetDeny(nContainerId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pContainer),
        INFINITE);
    if (rc == CGameObjectArray::SUCCESS) {
        pContainer->CompressContainer();

        m_cObjectArray.ReleaseDeny(nContainerId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x5B9C60
BOOL CInfGame::CheckIfPlayerInPortraitSlotCanUseItem(SHORT nPortraitNum, CItem* pItem, STRREF& strError, BOOL a4)
{
    LONG nCharacterId = nPortraitNum < m_nCharacters
        ? m_characterPortraits[nPortraitNum]
        : CGameObjectArray::INVALID_INDEX;

    BYTE threadNum = a4 ? CGameObjectArray::THREAD_ASYNCH : CGameObjectArray::THREAD_1;

    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            threadNum,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        strError = 9373;
        return FALSE;
    }

    BOOL bResult = CheckIfSpriteCanUseItem(pSprite, pItem, strError, a4);

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        threadNum,
        INFINITE);

    return bResult;
}

// 0x5B9BA0
BOOL CInfGame::CheckItemForClassAndKitUsability(DWORD dwClassMask, DWORD dwKitMask, CItem* pItem)
{
    WORD nItemType = pItem->GetItemType();
    if (nItemType == 35 || nItemType == 11) {
        return TRUE;
    }

    DWORD dwNotUsableBy = pItem->GetNotUsableBy();
    DWORD dwNotUsableBy2 = pItem->GetNotUsableBy2();

    if ((dwNotUsableBy & dwClassMask) == dwClassMask) {
        return (dwNotUsableBy2 & dwKitMask) != dwKitMask;
    }

    DWORD dwKits = dwNotUsableBy2 & dwKitMask;
    if (dwKits == 0) {
        return TRUE;
    }

    if ((~(dwNotUsableBy & dwClassMask) & dwClassMask & 0x39B) != 0) {
        return TRUE;
    }

    if ((dwNotUsableBy & 0x4) == 0 && (dwClassMask & 0x4) != 0) {
        if ((dwKits & 0xFF8000) == 0) {
            return TRUE;
        }
    }

    if ((dwNotUsableBy & 0x20) == 0 && (dwClassMask & 0x20) != 0) {
        if ((dwKits & 0x38) == 0) {
            return TRUE;
        }
    }

    if ((dwNotUsableBy & 0x40) == 0 && (dwClassMask & 0x40) != 0) {
        if ((dwKits & 0x7) == 0) {
            return TRUE;
        }
    }

    if ((dwNotUsableBy & 0x400) == 0 && (dwClassMask & 0x400) != 0) {
        if ((dwKits & 0x7FC0) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

// 0x5B9D20
BOOL CInfGame::CheckIfSpriteCanUseItem(CGameSprite* pSprite, CItem* pItem, STRREF& strError, BOOL a4)
{
    BOOL bResult = TRUE;

    // NOTE: Result is unused.
    if (pItem != NULL) {
        pItem->GetItemType();
    }

    strError = -1;

    if (pItem == NULL) {
        return bResult;
    }

    DWORD dwNotUsableBy = pItem->GetNotUsableBy();

    CAIObjectType typeAI;
    typeAI.Set(pSprite->GetAIType());

    if (!g_pBaldurChitin->GetObjectGame()->GetRuleTables().IsUsableByAlignment(dwNotUsableBy, typeAI.m_nAlignment)) {
        bResult = FALSE;
        strError = 9382;
    }

    if ((dwNotUsableBy & 0x800000) != 0 && typeAI.m_nRace == CAIObjectType::R_ELF) {
        bResult = FALSE;
        strError = 9382;
    }

    if ((dwNotUsableBy & 0x1000000) != 0 && typeAI.m_nRace == CAIObjectType::R_DWARF) {
        bResult = FALSE;
        strError = 9382;
    }

    if ((dwNotUsableBy & 0x2000000) != 0 && typeAI.m_nRace == CAIObjectType::R_HALF_ELF) {
        bResult = FALSE;
        strError = 9382;
    }

    if ((dwNotUsableBy & 0x4000000) != 0 && typeAI.m_nRace == CAIObjectType::R_HALFLING) {
        bResult = FALSE;
        strError = 9382;
    }

    if ((dwNotUsableBy & 0x8000000) != 0 && typeAI.m_nRace == CAIObjectType::R_HUMAN) {
        bResult = FALSE;
        strError = 9382;
    }

    if ((dwNotUsableBy & 0x10000000) != 0 && typeAI.m_nRace == CAIObjectType::R_GNOME) {
        bResult = FALSE;
        strError = 9382;
    }

    if ((dwNotUsableBy & 0x20000000) != 0 && typeAI.m_nRace == CAIObjectType::R_HALF_ORC) {
        bResult = FALSE;
        strError = 9382;
    }

    if (bResult) {
        if (g_pBaldurChitin->GetObjectGame()->GetRuleTables().ShouldCheckItemRequirements(pItem)) {
            // NOTE: Unsigned compare.
            if (pSprite->GetActiveStats()->m_nLevel < pItem->GetMinLevelRequired()) {
                bResult = FALSE;
                strError = 9382;
            }

            if (pSprite->GetActiveStats()->m_nSTR < static_cast<SHORT>(pItem->GetMinSTRRequired())) {
                bResult = FALSE;
                strError = 9382;
            }

            if (pSprite->GetActiveStats()->m_nINT < static_cast<SHORT>(pItem->GetMinINTRequired())) {
                bResult = FALSE;
                strError = 9382;
            }

            if (pSprite->GetActiveStats()->m_nDEX < static_cast<SHORT>(pItem->GetMinDEXRequired())) {
                bResult = FALSE;
                strError = 9382;
            }

            if (pSprite->GetActiveStats()->m_nWIS < static_cast<SHORT>(pItem->GetMinWISRequired())) {
                bResult = FALSE;
                strError = 9382;
            }

            if (pSprite->GetActiveStats()->m_nCON < static_cast<SHORT>(pItem->GetMinCONRequired())) {
                bResult = FALSE;
                strError = 9382;
            }

            if (pSprite->GetActiveStats()->m_nCHR < static_cast<SHORT>(pItem->GetMinCHRRequired())) {
                bResult = FALSE;
                strError = 9382;
                return bResult;
            }
        }

        if (bResult) {
            bResult = CheckItemForClassAndKitUsability(pSprite->GetActiveStats()->m_classMask,
                pSprite->GetActiveStats()->m_nSpecialization,
                pItem);
            if (!bResult) {
                strError = 9382;
            }
        }
    }

    return bResult;
}

// 0x5BA080
INT CInfGame::CheckItemUsable(CGameSprite* pSprite, CItem* pItem)
{
    INT nType = pItem->GetItemType();
    if (nType != 35 && nType != 11) {
        return 1;
    }

    DWORD nNotUsable1 = pItem->GetNotUsableBy();
    DWORD nNotUsable2 = pItem->GetNotUsableBy2();
    DWORD nClassMask = pSprite->GetDerivedStats()->m_classMask;
    DWORD nSpecialization = pSprite->GetDerivedStats()->m_nSpecialization;

    if ((nNotUsable1 & 0x1) == 0
        && (nClassMask & CLASSMASK_BARBARIAN) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x2) == 0
        && (nClassMask & CLASSMASK_BARD) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x4) == 0
        && (nClassMask & CLASSMASK_CLERIC) != 0
        && ((nNotUsable2 & nSpecialization) & SPECMASK_CLERIC) == 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x8) == 0
        && (nClassMask & CLASSMASK_DRUID) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x10) == 0
        && (nClassMask & CLASSMASK_FIGHTER) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x20) == 0
        && (nClassMask & CLASSMASK_MONK) != 0
        && ((nNotUsable2 & nSpecialization) & SPECMASK_MONK) == 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x40) == 0
        && (nClassMask & CLASSMASK_PALADIN) != 0
        && ((nNotUsable2 & nSpecialization) & SPECMASK_PALADIN) == 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x80) == 0
        && (nClassMask & CLASSMASK_RANGER) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x100) == 0
        && (nClassMask & CLASSMASK_ROGUE) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x200) == 0
        && (nClassMask & CLASSMASK_SORCERER) != 0) {
        return 1;
    }

    if ((nNotUsable1 & 0x400) == 0
        && (nClassMask & CLASSMASK_WIZARD) != 0
        && ((nNotUsable2 & nSpecialization) & SPECMASK_WIZARD) == 0) {
        return 1;
    }

    if ((nClassMask & CLASSMASK_ROGUE) != 0
        && pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] > 0) {
        return 2;
    }

    if ((nClassMask & CLASSMASK_BARD) != 0
        && pSprite->GetDerivedStats()->m_nSkills[CGAMESPRITE_SKILL_USE_MAGIC_DEVICE] > 0) {
        return 2;
    }

    return 0;
}
// 0x5BA230
BOOL CInfGame::sub_5BA230(SHORT nPortrait, SHORT nSlot, CItem*& pItem, STRREF& strError)
{
    LONG nCharacterId = GetCharacterId(nPortrait);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        strError = 0x249D;
        return FALSE;
    }

    BOOL bResult = sub_5BA2E0(pSprite, nSlot, pItem, strError);

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return bResult;
}

// 0x5BA2E0
BOOL CInfGame::sub_5BA2E0(CGameSprite* pSprite, SHORT nSlot, CItem*& pItem, STRREF& strError)
{
    CGameEffect* pEffect;
    STRREF strEffectError;
    BOOL bQuickItem = FALSE;

    // NOTE: The original keeps `nSlot` in this register when `pItem` is NULL. It is
    // only compared with item types that no slot number equals.
    WORD nItemType = 0;
    if (pItem != NULL) {
        nItemType = pItem->GetItemType();
    }

    strError = -1;

    if (nSlot >= CGameSpriteEquipment::NUM_SLOT) {
        strError = 9374;
        return FALSE;
    }

    BOOL bResult = CheckIfSpriteCanUseItem(pSprite, pItem, strError, TRUE);
    if (!bResult) {
        return bResult;
    }

    switch (nSlot) {
    case 0:
        // SLOT_AMULET
        if (pItem != NULL && nItemType != 1 && nItemType != 70) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 1:
        // SLOT_ARMOR
        if (pItem != NULL
            && nItemType != 60
            && nItemType != 61
            && nItemType != 66
            && nItemType != 62
            && nItemType != 63
            && nItemType != 64
            && nItemType != 65
            && nItemType != 67
            && nItemType != 68) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 2:
        // SLOT_BELT
        if (pItem != NULL && nItemType != 3) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 3:
        // SLOT_BOOTS
        if (pItem != NULL && nItemType != 4) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 4:
        // SLOT_CLOAK
        if (pItem != NULL && nItemType != 32) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 5:
        // SLOT_GAUNTLETS
        if (pItem != NULL && nItemType != 6 && nItemType != 73) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 6:
        // SLOT_HELMET
        if (pItem != NULL && nItemType != 7 && nItemType != 72) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 7:
        // SLOT_RING_LEFT
        if (pItem != NULL && nItemType != 10) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 8:
        // SLOT_RING_RIGHT
        if (pItem != NULL && nItemType != 10) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 10:
        // SLOT_FIST
        bResult = FALSE;
        strError = 9375;
        return bResult;
    case 11:
    case 12:
    case 13:
    case 14:
        // SLOT_AMMO
        if (pItem != NULL && nItemType != 5 && nItemType != 31 && nItemType != 14) {
            bResult = FALSE;
            strError = 9375;
            return bResult;
        }
        break;
    case 15:
    case 16:
    case 17:
        // SLOT_MISC
        bQuickItem = TRUE;

        if (pItem != NULL) {
            if (nItemType != 0
                && nItemType != 8
                && nItemType != 35
                && nItemType != 9
                && nItemType != 11
                && nItemType != 13
                && nItemType != 71) {
                bResult = FALSE;
                strError = 9375;
                return bResult;
            }

            if (pItem->GetResRef() == "SCRL75") {
                bResult = FALSE;
                strError = 9375;
                return bResult;
            }

            if (pItem->GetResRef() == "MISC3P") {
                bResult = FALSE;
                strError = 9375;
                return bResult;
            }

            if (pItem->GetResRef() == "MISC86") {
                bResult = FALSE;
                strError = 9375;
                return bResult;
            }

            if (pItem->GetAbilityCount() == 0) {
                bResult = FALSE;
                strError = 9375;
                return bResult;
            }

            if (!g_pBaldurChitin->GetObjectGame()->CheckItemUsable(pSprite, pItem)) {
                bResult = FALSE;
                strError = 9382;
                return bResult;
            }
        }
        break;
    case 43:
    case 45:
    case 47:
    case 49:
        // SLOT_WEAPON
        if (pItem != NULL && nItemType != 15) {
            if (nItemType != 16
                && nItemType != 17
                && nItemType != 18
                && nItemType != 19
                && nItemType != 20
                && nItemType != 69
                && nItemType != 21
                && nItemType != 22
                && nItemType != 23
                && nItemType != 24
                && nItemType != 25
                && nItemType != 26
                && nItemType != 27
                && nItemType != 29
                && nItemType != 30
                && nItemType != 57
                && nItemType != 44) {
                strError = 9375;
                bResult = FALSE;
            }
        }

        if (nItemType == 15 || nItemType == 27 || nItemType == 18) {
            // Launcher.
            if (pItem != NULL
                && pSprite->m_equipment.m_items[nSlot + 1] != NULL
                // FIXME: Calls `GetItemType` four times.
                && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 47
                && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 53
                && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 49
                && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 41) {
                strError = 1313;
                bResult = FALSE;
            }
        } else {
            if (pItem != NULL
                && pSprite->m_equipment.m_items[nSlot + 1] != NULL
                // FIXME: Calls `GetItemType` four times.
                && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 47
                && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 53
                && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 49
                && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 41) {
                pItem->Demand();

                for (INT nAbility = 0; nAbility < pItem->GetAbilityCount(); nAbility++) {
                    if (!bResult) {
                        break;
                    }

                    ITEM_ABILITY* pAbility = pItem->GetAbility(nAbility);
                    if (pAbility != NULL) {
                        // NOTE: Only the low byte of `type` is tested.
                        if (static_cast<BYTE>(pAbility->type) == 2) {
                            strError = 1312;
                            bResult = FALSE;
                        } else if (static_cast<BYTE>(pAbility->type) == 4) {
                            strError = 1313;
                            bResult = FALSE;
                        }
                    }
                }

                pItem->Release();

                if (!bResult) {
                    return bResult;
                }
            }
        }

        if (pItem != NULL
            && (pItem->GetFlagsFile() & 0x2) != 0
            && pSprite->m_equipment.m_items[nSlot + 1] != NULL) {
            // FIXME: Calls `GetItemType` four times.
            if (pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() == 47
                || pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() == 53
                || pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() == 49
                || pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() == 41) {
                strError = 9380;
            } else {
                strError = 28758;
            }

            bResult = FALSE;
            return bResult;
        }

        break;
    case 44:
    case 46:
    case 48:
    case 50:
        // SLOT_WEAPON
        if (pItem != NULL) {
            if (nItemType == 16
                || nItemType == 17
                || nItemType == 44
                || nItemType == 18
                || nItemType == 19
                || nItemType == 20
                || nItemType == 69
                || nItemType == 21
                || nItemType == 22
                || nItemType == 23
                || nItemType == 24
                || nItemType == 25
                || nItemType == 26
                || nItemType == 27
                || nItemType == 29
                || nItemType == 30
                || nItemType == 57) {
                if ((pItem->GetFlagsFile() & 0x2) != 0) {
                    strError = 26342;
                    bResult = FALSE;
                    return bResult;
                }

                pItem->Demand();

                for (INT nAbility = 0; nAbility < pItem->GetAbilityCount(); nAbility++) {
                    if (!bResult) {
                        break;
                    }

                    ITEM_ABILITY* pAbility = pItem->GetAbility(nAbility);
                    if (pAbility != NULL) {
                        // NOTE: Only the low byte of `type` is tested.
                        if (static_cast<BYTE>(pAbility->type) == 2
                            || static_cast<BYTE>(pAbility->type) == 4) {
                            strError = 1314;
                            bResult = FALSE;
                        }
                    }
                }

                pItem->Release();
            } else if (nItemType != 47
                && nItemType != 53
                && nItemType != 49
                && nItemType != 41) {
                strError = 9375;
                bResult = FALSE;
                return bResult;
            }
        }

        {
            CItem* pWeapon = pSprite->m_equipment.m_items[nSlot - 1];
            if (pWeapon != NULL && pItem != NULL) {
                if ((pWeapon->GetFlagsFile() & 0x2) != 0
                    && pWeapon != pSprite->m_equipment.m_items[10]) {
                    strError = 9381;
                    bResult = FALSE;
                    return bResult;
                }

                if (nItemType != 47
                    && nItemType != 53
                    && nItemType != 49
                    && nItemType != 41) {
                    pWeapon->Demand();

                    for (INT nAbility = 0; nAbility < pWeapon->GetAbilityCount(); nAbility++) {
                        if (!bResult) {
                            break;
                        }

                        ITEM_ABILITY* pAbility = pWeapon->GetAbility(nAbility);
                        if (pAbility != NULL) {
                            // NOTE: Only the low byte of `type` is tested.
                            if (static_cast<BYTE>(pAbility->type) == 2) {
                                strError = 1310;
                                bResult = FALSE;
                            } else if (static_cast<BYTE>(pAbility->type) == 4) {
                                strError = 1311;
                                bResult = FALSE;
                            }
                        }
                    }

                    pWeapon->Release();
                }
            }
        }

        break;
    default:
        break;
    }

    if (!bResult) {
        return bResult;
    }

    if (pItem != NULL) {
        if (pSprite->GetActiveStats()->m_cImmunitiesItemEquip.OnList(pItem->GetResRef(), strEffectError, pEffect)) {
            strError = strEffectError;
            bResult = FALSE;

            if (pEffect != NULL) {
                delete pEffect;
            }

            return bResult;
        }
    }

    if (pItem != NULL) {
        if (pSprite->GetActiveStats()->m_cImmunitiesItemTypeEquip.OnList(pItem->GetItemType(), strEffectError, pEffect)) {
            strError = strEffectError;
            bResult = FALSE;

            if (pEffect != NULL) {
                delete pEffect;
            }

            return bResult;
        }
    }

    if (!bQuickItem) {
        return bResult;
    }

    pItem->Demand();

    ITEM_ABILITY* pAbility = pItem->GetAbility(0);
    if (pAbility == NULL) {
        strError = 17317;
        bResult = FALSE;
        pItem->Release();
        return bResult;
    }

    if ((pItem->m_flags & 0x1) == 0 && (pItem->GetFlagsFile() & 0x100) != 0) {
        strError = 17316;
        bResult = FALSE;
        pItem->Release();
        return bResult;
    }

    if ((pItem->m_flags & 0x1) != 0 && (pItem->GetFlagsFile() & 0x200) != 0) {
        strError = 17316;
        bResult = FALSE;
    }

    pItem->Release();

    return bResult;
}


// 0x5BACE0
SHORT CInfGame::GetNumQuickWeaponSlots(SHORT nPortrait)
{
    SHORT nSlots;

    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortrait);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return 0;
    }

    nSlots = static_cast<SHORT>(pSprite->GetNumQuickWeaponSlots());

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return nSlots;
}

// 0x5B81C0
BOOL CInfGame::SwapItemPersonal(SHORT nPortraitNum, SHORT nSlotNum, CItem*& pNewItem, STRREF& errorCode, WORD wCount, BOOLEAN bFromServer)
{
    CGameSprite* pSprite;
    CItem* pOldItem;
    CMessage* message;
    CGameEffect* pEffect = NULL;
    STRREF strError;

    LONG nCharacterId = nPortraitNum < m_nCharacters
        ? m_characterPortraits[nPortraitNum]
        : CGameObjectArray::INVALID_INDEX;

    BOOL bResult = TRUE;
    BOOL bEquip = TRUE;
    BOOL bQuickItem = FALSE;
    INT nSlot;

    CItem* pItem = pNewItem;

    // NOTE: Uninitialized in the original when `pNewItem` is NULL.
    WORD nItemType = 0;
    if (pItem != NULL) {
        nItemType = pItem->GetItemType();
    }

    errorCode = -1;

    if (nSlotNum >= CGameSpriteEquipment::NUM_SLOT) {
        errorCode = 9374;
        return FALSE;
    }

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        errorCode = 9373;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.m_idLocalPlayer != pSprite->m_remotePlayerID) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        errorCode = 20695;
        return FALSE;
    }

    CAIObjectType typeAI;
    typeAI.Set(pSprite->GetAIType());

    if (pNewItem != NULL) {
        DWORD dwNotUsableBy = pNewItem->GetNotUsableBy();

        // NOTE: Uninline.
        if (!g_pBaldurChitin->GetObjectGame()->GetRuleTables().IsUsableByAlignment(dwNotUsableBy, typeAI.m_nAlignment)) {
            bResult = FALSE;
            errorCode = 9382;
        }

        if ((dwNotUsableBy & 0x800000) != 0 && typeAI.m_nRace == CAIObjectType::R_ELF) {
            bResult = FALSE;
            errorCode = 9382;
        }

        if ((dwNotUsableBy & 0x1000000) != 0 && typeAI.m_nRace == CAIObjectType::R_DWARF) {
            bResult = FALSE;
            errorCode = 9382;
        }

        if ((dwNotUsableBy & 0x2000000) != 0 && typeAI.m_nRace == CAIObjectType::R_HALF_ELF) {
            bResult = FALSE;
            errorCode = 9382;
        }

        if ((dwNotUsableBy & 0x4000000) != 0 && typeAI.m_nRace == CAIObjectType::R_HALFLING) {
            bResult = FALSE;
            errorCode = 9382;
        }

        if ((dwNotUsableBy & 0x8000000) != 0 && typeAI.m_nRace == CAIObjectType::R_HUMAN) {
            bResult = FALSE;
            errorCode = 9382;
        }

        if ((dwNotUsableBy & 0x10000000) != 0 && typeAI.m_nRace == CAIObjectType::R_GNOME) {
            bResult = FALSE;
            errorCode = 9382;
        }

        if ((dwNotUsableBy & 0x20000000) != 0 && typeAI.m_nRace == CAIObjectType::R_HALF_ORC) {
            bResult = FALSE;
            errorCode = 9382;
        }

        if (!bResult) {
            goto END;
        }

        if (g_pBaldurChitin->GetObjectGame()->GetRuleTables().ShouldCheckItemRequirements(pNewItem)) {
            // NOTE: Unsigned compare.
            if (pSprite->m_derivedStats.m_nLevel < pNewItem->GetMinLevelRequired()) {
                bResult = FALSE;
                errorCode = 9382;
            }

            if (pSprite->m_derivedStats.m_nSTR < static_cast<SHORT>(pNewItem->GetMinSTRRequired())) {
                bResult = FALSE;
                errorCode = 9382;
            }

            if (pSprite->m_derivedStats.m_nINT < static_cast<SHORT>(pNewItem->GetMinINTRequired())) {
                bResult = FALSE;
                errorCode = 9382;
            }

            if (pSprite->m_derivedStats.m_nDEX < static_cast<SHORT>(pNewItem->GetMinDEXRequired())) {
                bResult = FALSE;
                errorCode = 9382;
            }

            if (pSprite->m_derivedStats.m_nWIS < static_cast<SHORT>(pNewItem->GetMinWISRequired())) {
                bResult = FALSE;
                errorCode = 9382;
            }

            if (pSprite->m_derivedStats.m_nCON < static_cast<SHORT>(pNewItem->GetMinCONRequired())) {
                bResult = FALSE;
                errorCode = 9382;
            }

            if (pSprite->m_derivedStats.m_nCHR < static_cast<SHORT>(pNewItem->GetMinCHRRequired())) {
                bResult = FALSE;
                errorCode = 9382;
                goto END;
            }
        }

        if (!bResult) {
            goto END;
        }
    }

    nSlot = nSlotNum;

    switch (nSlot) {
    case 0:
        // SLOT_AMULET
        if (pNewItem != NULL && nItemType != 1 && nItemType != 70) {
            errorCode = 9375;
            bResult = FALSE;
            goto END;
        }
        break;
    case 1:
        // SLOT_ARMOR
        if (pNewItem != NULL
            && nItemType != 60
            && nItemType != 61
            && nItemType != 66
            && nItemType != 62
            && nItemType != 63
            && nItemType != 64
            && nItemType != 65
            && nItemType != 67
            && nItemType != 68) {
            errorCode = 9375;
            bResult = FALSE;
            goto END;
        }
        break;
    case 2:
        // SLOT_BELT
        if (pNewItem != NULL && nItemType != 3) {
            errorCode = 9375;
            bResult = FALSE;
            goto END;
        }
        break;
    case 3:
        // SLOT_BOOTS
        if (pNewItem != NULL && nItemType != 4) {
            errorCode = 9375;
            bResult = FALSE;
            goto END;
        }
        break;
    case 4:
        // SLOT_CLOAK
        if (pNewItem != NULL && nItemType != 32) {
            errorCode = 9375;
            bResult = FALSE;
            goto END;
        }
        break;
    case 5:
        // SLOT_GAUNTLETS
        if (pNewItem != NULL && nItemType != 6 && nItemType != 73) {
            errorCode = 9375;
            bResult = FALSE;
            goto END;
        }
        break;
    case 6:
        // SLOT_HELMET
        if (pNewItem != NULL && nItemType != 7 && nItemType != 72) {
            errorCode = 9375;
            bResult = FALSE;
            goto END;
        }
        break;
    case 7:
        // SLOT_RING_LEFT
    case 8:
        // SLOT_RING_RIGHT
        if (pNewItem != NULL && nItemType != 10) {
            errorCode = 9375;
            bResult = FALSE;
            goto END;
        }
        break;
    case 10:
        // SLOT_FIST
        errorCode = 9375;
        bResult = FALSE;
        goto END;
    case 11:
    case 12:
    case 13:
    case 14:
        // SLOT_AMMO
        if (pNewItem != NULL && nItemType != 5 && nItemType != 31 && nItemType != 14) {
            bResult = FALSE;
            errorCode = 9375;
        }
        bEquip = FALSE;
        break;
    case 15:
    case 16:
    case 17:
        // SLOT_MISC
        bQuickItem = TRUE;

        if (pNewItem != NULL) {
            if (nItemType == 0
                || nItemType == 8
                || nItemType == 9
                || nItemType == 11
                || nItemType == 35
                || nItemType == 71) {
                if (pNewItem->GetResRef() == "SPWI110Z") {
                    errorCode = 9375;
                    bResult = FALSE;
                } else if (pNewItem->GetResRef() == "MISC86") {
                    errorCode = 9375;
                    bResult = FALSE;
                } else if (pNewItem->GetAbilityCount() == 0) {
                    errorCode = 9375;
                    bResult = FALSE;
                } else if (!g_pBaldurChitin->GetObjectGame()->CheckItemUsable(pSprite, pNewItem)) {
                    errorCode = 9382;
                    bResult = FALSE;
                }
            } else {
                errorCode = 9375;
                bResult = FALSE;
            }
        }

        bEquip = FALSE;
        break;
    case 43:
    case 45:
    case 47:
    case 49: {
        // SLOT_WEAPON
        BYTE nWeaponSet = pSprite->m_nWeaponSet;
        if (nWeaponSet != (nSlot - 43) / 2) {
            bEquip = FALSE;
        }

        if (pNewItem != NULL && nItemType != 15) {
            if (nItemType != 16
                && nItemType != 17
                && nItemType != 44
                && nItemType != 18
                && nItemType != 19
                && nItemType != 20
                && nItemType != 57
                && nItemType != 69
                && nItemType != 21
                && nItemType != 22
                && nItemType != 23
                && nItemType != 24
                && nItemType != 25
                && nItemType != 26
                && nItemType != 27
                && nItemType != 29
                && nItemType != 30) {
                bResult = FALSE;
                errorCode = 9375;
            }
        }

        if (nItemType == 15 || nItemType == 27 || nItemType == 18) {
            // Launcher.
            if (pSprite->m_equipment.m_items[nSlot + 1] != NULL) {
                // FIXME: Calls `GetItemType` four times.
                if (pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 47
                    && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 53
                    && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 49
                    && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 41) {
                    errorCode = 28765;
                    bResult = FALSE;
                    goto END;
                }

                if (nItemType != 18) {
                    bResult = FALSE;
                    errorCode = 9380;
                }
            }

            bEquip = FALSE;
            break;
        }

        // FIXME: Calls `GetItemType` four times.
        if (pSprite->m_equipment.m_items[nSlot + 1] != NULL
            && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 47
            && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 53
            && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 49
            && pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() != 41
            && pNewItem != NULL) {
            pNewItem->Demand();

            for (INT nAbility = 0; nAbility < pNewItem->GetAbilityCount(); nAbility++) {
                if (!bResult) {
                    break;
                }

                ITEM_ABILITY* pAbility = pNewItem->GetAbility(nAbility);
                if (pAbility != NULL) {
                    // NOTE: Only the low byte of `type` is tested.
                    if (static_cast<BYTE>(pAbility->type) == 2) {
                        errorCode = 28765;
                        bResult = FALSE;
                    } else if (static_cast<BYTE>(pAbility->type) == 4) {
                        errorCode = 28766;
                        bResult = FALSE;
                    }
                }
            }

            pNewItem->Release();

            if (!bResult) {
                goto END;
            }
        }

        if (pNewItem != NULL
            && (pNewItem->GetFlagsFile() & 0x2) != 0
            && pSprite->m_equipment.m_items[nSlot + 1] != NULL) {
            // FIXME: Calls `GetItemType` four times.
            if (pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() == 47
                || pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() == 53
                || pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() == 49
                || pSprite->m_equipment.m_items[nSlot + 1]->GetItemType() == 41) {
                errorCode = 9380;
            } else {
                errorCode = 28758;
            }

            bResult = FALSE;
            goto END;
        }

        break;
    }
    case 44:
    case 46:
    case 48:
    case 50: {
        // SLOT_WEAPON
        BYTE nWeaponSet = pSprite->m_nWeaponSet;
        if (nWeaponSet != (nSlot - 43) / 2) {
            bEquip = FALSE;
        }

        if (pNewItem != NULL) {
            if (nItemType == 16
                || nItemType == 17
                || nItemType == 44
                || nItemType == 18
                || nItemType == 19
                || nItemType == 20
                || nItemType == 69
                || nItemType == 57
                || nItemType == 21
                || nItemType == 22
                || nItemType == 23
                || nItemType == 24
                || nItemType == 25
                || nItemType == 26
                || nItemType == 27
                || nItemType == 29
                || nItemType == 30) {
                if ((pNewItem->GetFlagsFile() & 0x2) != 0) {
                    errorCode = 26342;
                    bResult = FALSE;
                    goto END;
                }

                pNewItem->Demand();

                for (INT nAbility = 0; nAbility < pNewItem->GetAbilityCount(); nAbility++) {
                    if (!bResult) {
                        break;
                    }

                    ITEM_ABILITY* pAbility = pNewItem->GetAbility(nAbility);
                    if (pAbility != NULL) {
                        // NOTE: Only the low byte of `type` is tested.
                        if (static_cast<BYTE>(pAbility->type) == 2
                            || static_cast<BYTE>(pAbility->type) == 4) {
                            errorCode = 1314;
                            bResult = FALSE;
                        }
                    }
                }

                pNewItem->Release();
            } else if (nItemType != 47
                && nItemType != 49
                && nItemType != 53
                && nItemType != 41) {
                errorCode = 9375;
                bResult = FALSE;
                goto END;
            }
        }

        if (pSprite->m_equipment.m_items[42] != NULL
            && (pSprite->m_equipment.m_items[42]->GetFlagsFile() & 0x2) != 0
            && pNewItem != NULL) {
            errorCode = 9381;
            bResult = FALSE;
            goto END;
        }

        CItem* pWeapon = pSprite->m_equipment.m_items[nSlot - 1];
        if (pWeapon != NULL && pNewItem != NULL) {
            if ((pWeapon->GetFlagsFile() & 0x2) != 0
                && pWeapon != pSprite->m_equipment.m_items[10]) {
                errorCode = 9381;
                bResult = FALSE;
                goto END;
            }

            if (nItemType != 47
                && nItemType != 53
                && nItemType != 49
                && nItemType != 41) {
                pWeapon->Demand();

                for (INT nAbility = 0; nAbility < pWeapon->GetAbilityCount(); nAbility++) {
                    if (!bResult) {
                        break;
                    }

                    ITEM_ABILITY* pAbility = pWeapon->GetAbility(nAbility);
                    if (pAbility != NULL) {
                        // NOTE: Only the low byte of `type` is tested.
                        if (static_cast<BYTE>(pAbility->type) == 2) {
                            errorCode = 1310;
                            bResult = FALSE;
                        } else if (static_cast<BYTE>(pAbility->type) == 4) {
                            errorCode = 1311;
                            bResult = FALSE;
                        }
                    }
                }

                pWeapon->Release();
            }
        }

        break;
    }
    default:
        bEquip = FALSE;
        break;
    }

    if (!bResult) {
        goto END;
    }

    if (pItem != NULL) {
        if (pSprite->m_derivedStats.m_cImmunitiesItemEquip.OnList(pItem->GetResRef(), strError, pEffect)) {
            bResult = FALSE;
            errorCode = strError;

            if (pEffect != NULL) {
                delete pEffect;
            }

            goto END;
        }
    }

    if (pItem != NULL) {
        if (pSprite->m_derivedStats.m_cImmunitiesItemTypeEquip.OnList(pItem->GetItemType(), strError, pEffect)) {
            bResult = FALSE;
            errorCode = strError;

            if (pEffect != NULL) {
                delete pEffect;
            }

            goto END;
        }
    }

    if (pSprite->m_equipment.m_items[42] != NULL
        && (nSlotNum == 43
            || nSlotNum == 44
            || nSlotNum == 45
            || nSlotNum == 46
            || nSlotNum == 47
            || nSlotNum == 48
            || nSlotNum == 49
            || nSlotNum == 50
            || nSlotNum == 11
            || nSlotNum == 12
            || nSlotNum == 13
            || nSlotNum == 14)) {
        errorCode = 10141;
        bResult = FALSE;
        goto END;
    }

    if (pNewItem != NULL) {
        if (IsItemExclusive(pSprite, nSlot, pNewItem, errorCode)) {
            bResult = FALSE;
            goto END;
        }
    }

    if (pSprite->m_equipment.m_items[nSlot] != NULL) {
        CString sScriptName(pSprite->m_scriptName);

        if (sScriptName == "MINSC" && pSprite->m_equipment.m_items[nSlot]->GetResRef() == "MISC84") {
            errorCode = 10218;
            bResult = FALSE;
        } else if (sScriptName == "ALORA" && pSprite->m_equipment.m_items[nSlot]->GetResRef() == "MISC88") {
            errorCode = 10219;
            bResult = FALSE;
        } else if (sScriptName == "EDWIN" && pSprite->m_equipment.m_items[nSlot]->GetResRef() == "MISC89") {
            errorCode = 10222;
            bResult = FALSE;
        }
    }

    if (!bResult) {
        goto END;
    }

    if (pSprite->m_equipment.m_items[nSlot] != NULL) {
        DWORD dwFlags = pSprite->m_equipment.m_items[nSlot]->GetFlagsFile();

        if ((dwFlags & 0x10) != 0) {
            pSprite->m_equipment.m_items[nSlot]->m_flags |= 0x1;
            bResult = FALSE;
            errorCode = 16304;
        }

        if ((dwFlags & 0x4) == 0) {
            errorCode = 25697;
            pSprite->m_equipment.m_items[nSlot]->m_flags |= 0x1;
            bResult = FALSE;
            goto END;
        }
    }

    if (!bResult) {
        goto END;
    }

    if (bQuickItem && pNewItem != NULL) {
        pNewItem->Demand();

        ITEM_ABILITY* pAbility = pNewItem->GetAbility(0);
        if (pAbility == NULL) {
            errorCode = 17317;
            bResult = FALSE;
        } else {
            // NOTE: Tests the high byte of `type`.
            if ((((pNewItem->m_flags & 0x1) == 0) && (pAbility->type & 0x100) != 0)
                || (((pNewItem->m_flags & 0x1) != 0) && (pAbility->type & 0x200) != 0)) {
                errorCode = 17316;
                bResult = FALSE;
            }
        }

        pNewItem->Release();
    }

    if (!bResult) {
        goto END;
    }

    if (pNewItem != NULL && (pNewItem->GetFlagsFile() & 0x10) != 0) {
        pNewItem->m_flags |= 0x1;
        errorCode = 16304;
        bEquip = TRUE;
    }

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        errorCode = 9373;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.m_idLocalPlayer != pSprite->m_remotePlayerID
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !bFromServer) {
        BOOLEAN bRequestResult = g_pBaldurChitin->GetBaldurMessage()->SwapItemRequest(CBaldurMessage::SWAPITEM_LOCATION_PERSONAL,
            nCharacterId,
            nSlotNum,
            pNewItem,
            errorCode,
            wCount);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        return bRequestResult;
    }

    pOldItem = pSprite->m_equipment.m_items[nSlot];

    if (pOldItem != NULL
        && pNewItem == NULL
        && pOldItem->GetMaxStackable() > 1
        && wCount < pOldItem->GetUsageCount(0)) {
        // Split stack.

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 13383
        UTIL_ASSERT(wCount != 0);

        pOldItem->SetUsageCount(0, static_cast<WORD>(pOldItem->GetUsageCount(0) - wCount));

        pNewItem = new CItem(pOldItem->GetResRef(), 0, 0, 0, 0, 0);
        pNewItem->SetUsageCount(0, wCount);
        pNewItem->m_flags = (pNewItem->m_flags & ~0x1) | (pOldItem->m_flags & 0x1);

        goto UPDATE;
    }

    if (pOldItem != NULL
        && pNewItem != NULL
        && pOldItem->GetMaxStackable() > 1
        && pNewItem->GetResRef() == pOldItem->GetResRef()
        && ((pNewItem->m_flags ^ pOldItem->m_flags) & 0x1) == 0) {
        // Merge stacks.

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 13393
        UTIL_ASSERT(wCount != 0);

        INT nCount = wCount;
        INT nSpace = pOldItem->GetMaxStackable() - pOldItem->GetUsageCount(0);
        if (nCount >= (pNewItem->GetUsageCount(0) < nSpace ? pNewItem->GetUsageCount(0) : nSpace)) {
            nSpace = pOldItem->GetMaxStackable() - pOldItem->GetUsageCount(0);
            nCount = pNewItem->GetUsageCount(0) < nSpace ? pNewItem->GetUsageCount(0) : nSpace;
        }

        pOldItem->SetUsageCount(0, static_cast<WORD>(pOldItem->GetUsageCount(0) + nCount));

        if (pNewItem->GetUsageCount(0) == static_cast<WORD>(nCount)) {
            // NOTE: Uninline.
            AddDisposableItem(pNewItem);
            pNewItem = NULL;
        } else {
            pNewItem->SetUsageCount(0, static_cast<WORD>(pNewItem->GetUsageCount(0) - nCount));
        }

        goto UPDATE;
    }

    if (pSprite->m_equipment.m_selectedWeapon == nSlotNum) {
        pSprite->m_equipment.m_selectedWeapon = CGameSpriteEquipment::SLOT_FIST;
        pSprite->m_equipment.m_selectedWeaponAbility = 0;

        if (!bEquip) {
            if (pSprite->m_equipment.m_items[nSlot] != NULL) {
                if (nSlotNum >= 43 && nSlotNum <= 50) {
                    CButtonData buttonData;
                    pSprite->SetQuickWeapon(static_cast<BYTE>(nSlotNum - 43), buttonData);
                    pSprite->SetQuickWeapon(static_cast<BYTE>(nSlotNum - 43), 0);
                }

                pSprite->m_equipment.m_items[nSlot]->Demand();
                pSprite->m_equipment.m_items[nSlot]->Unequip(pSprite, nSlot, TRUE, FALSE);

                ITEM_ABILITY* pAbility = pSprite->m_equipment.m_items[nSlot]->GetAbility(pSprite->m_equipment.m_selectedWeaponAbility);
                if (pAbility != NULL) {
                    SHORT nLauncherSlot;
                    CItem* pLauncher = pSprite->GetLauncher(pAbility, nLauncherSlot);
                    if (pLauncher != NULL) {
                        pLauncher->Unequip(pSprite, nLauncherSlot, TRUE, FALSE);
                    }
                }

                pSprite->m_equipment.m_items[nSlot]->Release();
            }
        }
    } else {
        INT nSelectedWeapon = pSprite->m_equipment.m_selectedWeapon;

        pSprite->m_equipment.m_items[nSelectedWeapon]->Demand();

        ITEM_ABILITY* pAbility = pSprite->m_equipment.m_items[pSprite->m_equipment.m_selectedWeapon]->GetAbility(pSprite->m_equipment.m_selectedWeaponAbility);
        if (pAbility != NULL) {
            SHORT nLauncherSlot;
            CItem* pLauncher = pSprite->GetLauncher(pAbility, nLauncherSlot);
            if (pLauncher != NULL) {
                if (pLauncher == pSprite->m_equipment.m_items[nSlot]) {
                    pSprite->m_equipment.m_items[nSlot]->Unequip(pSprite, nSlot, TRUE, FALSE);
                    pSprite->m_equipment.m_items[nSlot] = NULL;

                    pSprite->m_equipment.m_items[pSprite->m_equipment.m_selectedWeapon]->Unequip(pSprite,
                        pSprite->m_equipment.m_selectedWeapon,
                        TRUE,
                        FALSE);

                    pSprite->m_equipment.m_selectedWeapon = CGameSpriteEquipment::SLOT_FIST;
                    pSprite->m_equipment.m_selectedWeaponAbility = 0;
                }
            }
        }

        pSprite->m_equipment.m_items[nSelectedWeapon]->Release();
    }

    if (bEquip) {
        if (pSprite->m_equipment.m_items[nSlot] != NULL) {
            pSprite->m_equipment.m_items[nSlot]->Unequip(pSprite, nSlot, TRUE, FALSE);
            pSprite->m_equipment.m_items[nSlot] = NULL;
        }

        if (pNewItem != NULL) {
            pNewItem->Equip(pSprite, nSlot, FALSE);
        }
    }

    pSprite->m_equipment.m_items[nSlot] = pNewItem;
    pNewItem = pOldItem;

    if (nSlotNum == 43
        || nSlotNum == 44
        || nSlotNum == 45
        || nSlotNum == 46
        || nSlotNum == 47
        || nSlotNum == 48
        || nSlotNum == 49
        || nSlotNum == 50) {
        if (bEquip != TRUE) {
            BYTE nWeaponSet = pSprite->m_nWeaponSet;
            if (nWeaponSet != (nSlot - 43) / 2) {
                pSprite->sub_726F20();
                goto UPDATE;
            }
        }

        BOOL bShield = nSlotNum == 44
            || nSlotNum == 46
            || nSlotNum == 48
            || nSlotNum == 50;

        if (pOldItem != NULL) {
            if (!pSprite->sub_737910(bShield)) {
                bResult = FALSE;
                goto UPDATE;
            }
        }

        bResult = TRUE;
        pSprite->sub_726810(static_cast<BYTE>((nSlot - 43) / 2));
    }

UPDATE:
    g_pBaldurChitin->GetActiveEngine()->UpdatePersonalItemStatus(pSprite->m_id);

    if ((g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
            || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID)
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        message = new CMessageSpriteEquipment(pSprite, nCharacterId, nCharacterId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    pSprite->sub_723830();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

END:
    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return bResult;
}

// 0x5BE840
SHORT CInfGame::FindEmptyItemPersonalInventory(SHORT nPortraitNum)
{
    CGameSprite* pSprite;

    LONG nCharacterId = nPortraitNum < m_nCharacters
        ? m_characterPortraits[nPortraitNum]
        : CGameObjectArray::INVALID_INDEX;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return -1;
    }

    for (SHORT nIndex = 0; nIndex < CScreenInventory::PERSONAL_INVENTORY_SIZE; nIndex++) {
        if (pSprite->m_equipment.m_items[18 + nIndex] == NULL) {
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            return nIndex + 18;
        }
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return -1;
}

// 0x5BAD70
BOOL CInfGame::SwapItemPersonalInventory(SHORT nPortraitNum, CItem*& pNewItem, SHORT nSlotNum, STRREF& errorCode, WORD wCount, BOOLEAN bFromServer, BOOL bAutoStacking)
{
    CGameSprite* pSprite;
    CItem* pOldItem;
    CMessage* message;

    LONG nCharacterId = nPortraitNum < m_nCharacters
        ? m_characterPortraits[nPortraitNum]
        : CGameObjectArray::INVALID_INDEX;

    BOOL bResult = TRUE;
    errorCode = -1;

    if ((nSlotNum < 18 || nSlotNum >= CScreenInventory::PERSONAL_INVENTORY_SIZE + 18)
        && nSlotNum != 0x7FFF) {
        errorCode = 9374;
        return FALSE;
    }

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        errorCode = 9373;
        return FALSE;
    }

    if (g_pChitin->cNetwork.GetServiceProvider() != CNetwork::SERV_PROV_NULL
        && g_pChitin->cNetwork.m_idLocalPlayer != pSprite->m_remotePlayerID
        && g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && !bFromServer) {
        BOOLEAN bRequestResult = g_pBaldurChitin->GetBaldurMessage()->SwapItemRequest(CBaldurMessage::SWAPITEM_LOCATION_PERSONAL_INVENTORY,
            nCharacterId,
            nSlotNum,
            pNewItem,
            errorCode,
            wCount);

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        return bRequestResult;
    }

    if (nSlotNum != 0x7FFF
        && pNewItem != NULL
        && pSprite->m_equipment.m_items[nSlotNum] != NULL
        && pSprite->m_equipment.m_items[nSlotNum]->GetItemType() == 58) {
        // Drop item into a bag.
        DWORD nAdded = AddItemToStore(pSprite->m_equipment.m_items[nSlotNum]->GetResRef(), pNewItem, errorCode);
        if (nAdded != 0) {
            if (pNewItem->GetMaxStackable() > 1 && pNewItem->GetUsageCount(0) != nAdded) {
                pNewItem->SetUsageCount(0, static_cast<WORD>(pNewItem->GetUsageCount(0) - nAdded));
            } else {
                // NOTE: Uninline.
                AddDisposableItem(pNewItem);
                pNewItem = NULL;
            }

            g_pBaldurChitin->GetActiveEngine()->UpdatePersonalItemStatus(pSprite->m_id);

            if ((g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID)
                && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
                message = new CMessageSpriteEquipment(pSprite, nCharacterId, nCharacterId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            return TRUE;
        }

        bResult = FALSE;
    } else if (bAutoStacking && pNewItem != NULL && pNewItem->GetMaxStackable() > 1) {
        for (SHORT nSlot = 18; nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE + 18; nSlot++) {
            CItem* pSlotItem = pSprite->m_equipment.m_items[nSlot];
            if (pSlotItem != NULL
                && pSlotItem->GetResRef() == pNewItem->GetResRef()
                && ((pSlotItem->m_flags ^ pNewItem->m_flags) & 0x1) == 0
                && pSlotItem->GetUsageCount(0) < pSlotItem->GetMaxStackable()) {
                if (pSlotItem->GetUsageCount(0) + pNewItem->GetUsageCount(0) <= pSlotItem->GetMaxStackable()) {
                    nSlotNum = nSlot;
                    break;
                }

                pNewItem->SetUsageCount(0, static_cast<WORD>(pNewItem->GetUsageCount(0) - pSlotItem->GetMaxStackable() + pSlotItem->GetUsageCount(0)));
                pSlotItem->SetUsageCount(0, pSlotItem->GetMaxStackable());
            }
        }
    }

    if (nSlotNum == 0x7FFF) {
        for (SHORT nSlot = 18; nSlot < CScreenInventory::PERSONAL_INVENTORY_SIZE + 18; nSlot++) {
            if (nSlotNum != 0x7FFF) {
                break;
            }

            if (pSprite->m_equipment.m_items[nSlot] == NULL) {
                nSlotNum = nSlot;
            }
        }

        if (nSlotNum == 0x7FFF) {
            // "Inventory full"
            errorCode = 17999;

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            return FALSE;
        }
    }

    pOldItem = pSprite->m_equipment.m_items[nSlotNum];

    if (bResult) {
        if (pOldItem != NULL
            && ((pOldItem->GetFlagsFile() & 0x4) == 0 || (pOldItem->m_flags & 0x10) != 0)) {
            bResult = FALSE;
            errorCode = 25697;
        } else {
            if (pOldItem != NULL
                && pNewItem == NULL
                && pOldItem->GetMaxStackable() > 1
                && wCount < pOldItem->GetUsageCount(0)) {
                // Split stack.

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 14884
                UTIL_ASSERT(wCount != 0);

                pOldItem->SetUsageCount(0, static_cast<WORD>(pOldItem->GetUsageCount(0) - wCount));

                pNewItem = new CItem(pOldItem->GetResRef(), 0, 0, 0, 0, 0);
                pNewItem->SetUsageCount(0, wCount);
                pNewItem->m_flags = (pNewItem->m_flags & ~0x1) | (pOldItem->m_flags & 0x1);
            } else if (pOldItem != NULL
                && pNewItem != NULL
                && pOldItem->GetMaxStackable() > 1
                && pNewItem->GetResRef() == pOldItem->GetResRef()
                && ((pNewItem->m_flags ^ pOldItem->m_flags) & 0x1) == 0) {
                // Merge stacks.

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 14894
                UTIL_ASSERT(wCount != 0);

                INT nCount = wCount;
                INT nSpace = pOldItem->GetMaxStackable() - pOldItem->GetUsageCount(0);
                if (nCount >= (pNewItem->GetUsageCount(0) < nSpace ? pNewItem->GetUsageCount(0) : nSpace)) {
                    nSpace = pOldItem->GetMaxStackable() - pOldItem->GetUsageCount(0);
                    nCount = pNewItem->GetUsageCount(0) < nSpace ? pNewItem->GetUsageCount(0) : nSpace;
                }

                pOldItem->SetUsageCount(0, static_cast<WORD>(pOldItem->GetUsageCount(0) + nCount));

                if (pNewItem->GetUsageCount(0) == static_cast<WORD>(nCount)) {
                    // NOTE: Uninline.
                    AddDisposableItem(pNewItem);
                    pNewItem = NULL;
                } else {
                    pNewItem->SetUsageCount(0, static_cast<WORD>(pNewItem->GetUsageCount(0) - nCount));
                }
            } else {
                pSprite->m_equipment.m_items[nSlotNum] = pNewItem;
                pNewItem = pOldItem;
            }

            g_pBaldurChitin->GetActiveEngine()->UpdatePersonalItemStatus(pSprite->m_id);

            if ((g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID)
                && g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
                message = new CMessageSpriteEquipment(pSprite, nCharacterId, nCharacterId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    }

    pSprite->field_562C = 1;
    pSprite->ProcessEffectList();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return bResult;
}

// 0x5BB600
void CInfGame::InventoryInfoPersonal(SHORT nPortraitNum, SHORT nSlotNum, CItem*& pItem, STRREF& description, CResRef& cResIcon, CResRef& cResItem, WORD& wCount, BOOL a8)
{
    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortraitNum);

    description = -1;
    cResIcon = "";
    cResItem = "";
    wCount = 0;

    if (nSlotNum < CGameSpriteEquipment::NUM_SLOT) {
        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (nSlotNum == 43
                || nSlotNum == 45
                || nSlotNum == 47
                || nSlotNum == 49) {
                if (pSprite->GetEquipment()->m_items[nSlotNum] == NULL && a8 == TRUE) {
                    if (pSprite->GetEquipment()->m_items[nSlotNum + 1] != NULL) {
                        // FIXME: Calls `GetItemType` four times.
                        if (pSprite->GetEquipment()->m_items[nSlotNum + 1]->GetItemType() == 47
                            || pSprite->GetEquipment()->m_items[nSlotNum + 1]->GetItemType() == 53
                            || pSprite->GetEquipment()->m_items[nSlotNum + 1]->GetItemType() == 49
                            || pSprite->GetEquipment()->m_items[nSlotNum + 1]->GetItemType() == 41) {
                            nSlotNum = 10;
                        }
                    } else {
                        if (pSprite->m_nWeaponSet == (nSlotNum - 43) / 2) {
                            nSlotNum = 10;
                        }
                    }
                }
            }

            pItem = pSprite->GetEquipment()->m_items[nSlotNum];
            if (pItem != NULL) {
                description = pItem->GetGenericName();
                cResIcon = pItem->GetItemIcon();
                cResItem = pItem->GetResRef();

                if (pItem->GetMaxStackable() > 1) {
                    wCount = pItem->GetUsageCount(0);
                }
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// 0x5BB800
BOOL CInfGame::Is3DSound(int nSoundChannel)
{
    return m_ruleTables.Is3DSound(nSoundChannel);
}

// 0x5BB810
float CInfGame::GetSoundReverbMix(int nSoundChannel, int nReverb)
{
    return m_ruleTables.GetSoundReverbMix(nSoundChannel, nReverb);
}

// 0x5BB830
CResRef CInfGame::GetAnimationBam(SHORT nPortrait, BYTE range)
{
    CString sResRef;

    // NOTE: Uninline.
    LONG nCharacterId = GetCharacterId(nPortrait);

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return CResRef("");
    }

    pSprite->GetAnimation()->GetAnimationResRef(sResRef, range);

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return CResRef(sResRef);
}

// 0x5BB960
BYTE CInfGame::GetFrameRate()
{
    return static_cast<BYTE>(g_pBaldurChitin->GetActiveEngine()->pVidMode->field_94);
}

// 0x5BE900
void CInfGame::SynchronousUpdate()
{
    if (field_366E) {
        CResHelper<CResBitmap, 1> cResScreenShot;
        CResHelper<CResBitmap, 1> cResPortrait;
        LPBYTE pScreenShotData = NULL;
        LPBYTE pPortraitData = NULL;
        CVidBitmap cPortraitBitmap;
        CString sPortraitResRef;

        if (field_50D8 && !field_50DC) {
            CString sSource;
            CString sDestination;

            sSource = GetDirSaveRoot() + "default\\" + "ICEWIND2.BMP";
            sDestination = GetDirSave() + "ICEWIND2.BMP";
            CopyFileA(sSource, sDestination, FALSE);
            DeleteFileA(sSource);

            for (int nPortrait = 0; nPortrait < 6; nPortrait++) {
                sSource.Format("%sdefault\\%s%d.BMP", (LPCSTR)GetDirSaveRoot(), "PORTRT", nPortrait);
                sDestination.Format("%s%s%d.BMP", (LPCSTR)GetDirSave(), "PORTRT", nPortrait);
                CopyFileA(sSource, sDestination, FALSE);
                DeleteFileA(sSource);
            }

            field_366E = 0;
            field_50D8 = FALSE;

            return;
        }

        CGameArea* pArea = m_gameAreas[m_visibleArea];
        if (pArea != NULL) {
            EnterCriticalSection(&(pArea->field_214));
            pArea->SetListManipulationThreadId(GetCurrentThreadId());
        }

        EnterCriticalSection(&(m_gameAreas[m_visibleArea]->field_1FC));

        Sleep(100);

        CSingleLock renderLock(&(g_pBaldurChitin->m_pEngineWorld->GetManager()->field_36), FALSE);
        renderLock.Lock(INFINITE);

        // NOTE: Uninline.
        BYTE nBrightnessCorrection = g_pBaldurChitin->GetCurrentVideoMode()->m_nBrightnessCorrection;
        g_pBaldurChitin->GetCurrentVideoMode()->m_nBrightnessCorrection = 0;

        // NOTE: Uninline.
        BYTE nGammaCorrection = g_pBaldurChitin->GetCurrentVideoMode()->m_nGammaCorrection;
        g_pBaldurChitin->GetCurrentVideoMode()->m_nGammaCorrection = 0;

        m_gameAreas[m_visibleArea]->m_cInfinity.pVRPool->InvalidateAll();

        // NOTE: Uninline.
        g_pBaldurChitin->GetCurrentVideoMode()->LoadFogOWarSurfaces(FOGOWAR_RESREF);

        field_366E = 0;

        BOOL bSaved = CCacheStatus::dword_8D0BA8;
        CCacheStatus::dword_8D0BA8 = FALSE;
        g_pBaldurChitin->m_pEngineWorld->SaveScreen();
        CCacheStatus::dword_8D0BA8 = bSaved;

        LONG nScreenShotSize;

        // NOTE: Uninline.
        g_pBaldurChitin->GetCurrentVideoMode()->PrintSurfaceToBmp(pScreenShotData,
            0,
            CInfinity::stru_8E79A8,
            nScreenShotSize,
            5);

        CString sDirName;
        if (!field_50DC && g_pChitin->cNetwork.m_nServiceProvider != CNetwork::SERV_PROV_NULL) {
            sDirName = GetDirSave();
        } else if (!g_pChitin->cVideo.m_bIs3dAccelerated) {
            sDirName = GetDirSaveRoot() + "default\\";
        } else if (field_50DC) {
            sDirName = GetDirSave();
            field_50DC = 0;
            field_50D8 = FALSE;
        } else {
            sDirName = GetDirSaveRoot() + "default\\";
            field_50D8 = TRUE;
        }

        if (pScreenShotData != NULL) {
            // NOTE: Uninline.
            cResScreenShot.SetResRef(CResRef("ICEWIND2"), TRUE, TRUE);

            cResScreenShot.pRes->Write(sDirName, pScreenShotData, nScreenShotSize);
            delete pScreenShotData;
        }

        WORD nPortraitIndex = 0;
        for (int nCharacter = 0; nCharacter < 6; nCharacter++) {
            LONG nCharacterId = m_characters[nCharacter];
            if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
                CGameSprite* pSprite;

                BYTE rc;
                do {
                    rc = m_cObjectArray.GetShare(nCharacterId,
                        CGameObjectArray::THREAD_1,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    cPortraitBitmap.SetResRef(CResRef(pSprite->m_baseStats.m_portraitSmall), TRUE, TRUE);
                    cPortraitBitmap.m_bDoubleSize = FALSE;

                    if (cPortraitBitmap.pRes == NULL) {
                        BOOL bDoubleSize = g_pBaldurChitin->field_4A28;
                        cPortraitBitmap.SetResRef(CResRef(SILHOUETTE_PORTRAIT_SM), TRUE, TRUE);
                        cPortraitBitmap.m_bDoubleSize = bDoubleSize;
                    }

                    if (cPortraitBitmap.GetBitCount(FALSE) != 24
                        && cPortraitBitmap.GetBitCount(FALSE) != 8) {
                        BOOL bDoubleSize = g_pBaldurChitin->field_4A28;
                        cPortraitBitmap.SetResRef(CResRef(SILHOUETTE_PORTRAIT_SM), TRUE, TRUE);
                        cPortraitBitmap.m_bDoubleSize = bDoubleSize;
                    }

                    m_cObjectArray.ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_1,
                        INFINITE);

                    CSize size;
                    cPortraitBitmap.GetImageDimensions(size, FALSE);

                    CRect rPortrait;
                    SetRect(&rPortrait, 0, 0, size.cx, size.cy);

                    cPortraitBitmap.RenderDirect(2, 0, 0, rPortrait, 0, FALSE);

                    LONG nPortraitSize;

                    // NOTE: Uninline.
                    g_pBaldurChitin->GetCurrentVideoMode()->PrintSurfaceToBmp(pPortraitData,
                        2,
                        rPortrait,
                        nPortraitSize,
                        2);

                    if (pPortraitData != NULL) {
                        sPortraitResRef.Format("%s%d", "PORTRT", nPortraitIndex);

                        // NOTE: Uninline.
                        cResPortrait.SetResRef(CResRef(sPortraitResRef), TRUE, TRUE);

                        cResPortrait.pRes->Write(sDirName, pPortraitData, nPortraitSize);
                        nPortraitIndex++;
                        delete pPortraitData;
                    }
                }
            }
        }

        g_pChitin->cDimm.RemoveFromDirectoryList(GetDirSave(), TRUE);

        // NOTE: Uninline.
        g_pBaldurChitin->GetCurrentVideoMode()->m_nBrightnessCorrection = nBrightnessCorrection;

        // NOTE: Uninline.
        g_pBaldurChitin->GetCurrentVideoMode()->m_nGammaCorrection = nGammaCorrection;

        m_gameAreas[m_visibleArea]->m_cInfinity.pVRPool->InvalidateAll();

        // NOTE: Uninline.
        g_pBaldurChitin->GetCurrentVideoMode()->LoadFogOWarSurfaces(FOGOWAR_RESREF);

        renderLock.Unlock();

        LeaveCriticalSection(&(m_gameAreas[m_visibleArea]->field_1FC));

        pArea = m_gameAreas[m_visibleArea];
        if (pArea != NULL) {
            pArea->SetListManipulationThreadId(0);
            LeaveCriticalSection(&(pArea->field_214));
        }
    }

    DestroyDisposableItems();
}

// 0x5C0520
CStringList* CInfGame::GetSaveGames()
{
    CStringList* pList = new CStringList();

    CString sFileName;

    WIN32_FIND_DATAA ffd;
    HANDLE hFindFile = FindFirstFileA(GetDirSaveRoot() + "*.*", &ffd);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sFileName = ffd.cFileName;
            if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                && sFileName != "."
                && sFileName != ".."
                && SaveGameExists(sFileName)) {
                POSITION pos = pList->GetHeadPosition();
                while (pos != NULL) {
                    if (sFileName < pList->GetAt(pos)) {
                        break;
                    }
                    pList->GetNext(pos);
                }

                if (pos != NULL) {
                    pList->InsertBefore(pos, sFileName);
                } else {
                    pList->AddTail(sFileName);
                }
            }
        } while (FindNextFileA(hFindFile, &ffd));

        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C0730
BOOL CInfGame::DeleteSaveGame(const CString& sFileName)
{
    CString sDirectoryName = GetDirSaveRoot() + sFileName;

    CFileStatus cFileStatus;
    if (!CFile::GetStatus(sDirectoryName, cFileStatus)) {
        return FALSE;
    }

    sDirectoryName += "\\";
    if (!g_pChitin->cDimm.DirectoryRemoveFiles(sDirectoryName)) {
        return FALSE;
    }

    WIN32_FIND_DATAA ffd;
    HANDLE hFindFile = FindFirstFileA(sDirectoryName + "*.*", &ffd);
    if (hFindFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    do {
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 || ffd.cFileName[0] != '.') {
            FindClose(hFindFile);
            return FALSE;
        }
    } while (FindNextFileA(hFindFile, &ffd));

    FindClose(hFindFile);

    return RemoveDirectoryA(sDirectoryName);
}

// 0x5C0890
BOOL CInfGame::SaveGameExists(const CString& sSaveGame)
{
    CString sFileName = GetDirSaveRoot() + sSaveGame + "\\ICEWIND2.GAM";

    HANDLE hFile = CreateFileA(sFileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    // NOTE: Original code is slightly different (conditions inverted).
    BOOL bSuccess = FALSE;
    if (hFile != INVALID_HANDLE_VALUE) {
        unsigned char signature[8];
        DWORD dwBytesRead;
        ReadFile(hFile, signature, sizeof(signature), &dwBytesRead, NULL);
        if (dwBytesRead == sizeof(signature)
            && memcmp(signature, "GAMEV2.2", sizeof(signature)) == 0) {
            bSuccess = TRUE;
        }

        CloseHandle(hFile);
    }

    return bSuccess;
}

// 0x5C0EA0
void CInfGame::ApplyVolumeSliders(BOOLEAN a2)
{
    g_pBaldurChitin->cSoundMixer.SetMusicVolume(m_cOptions.m_nVolumeMusic);

    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(0, m_cOptions.m_nVolumeVoices);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(1, m_cOptions.m_nVolumeAmbients);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(2, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(3, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(4, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(5, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(6, m_cOptions.m_nVolumeVoices);

    for (int k = 7; k < 13; k++) {
        g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(k, m_cOptions.m_nVolumeVoices);
    }

    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(13, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(14, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(15, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(20, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(18, m_cOptions.m_nVolumeSFX);
    g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(19, m_cOptions.m_nVolumeSFX);

    if (a2 == TRUE) {
        g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(16, m_cOptions.m_nVolumeAmbients);
        g_pBaldurChitin->cSoundMixer.SetChannelVolumeFast(17, m_cOptions.m_nVolumeAmbients);
    }

    g_pBaldurChitin->cSoundMixer.UpdateSoundList();
}

// 0x5BB980
void CInfGame::UseMagicOnGround(CPoint pt)
{
    CAIAction action;

    LONG nCharacterId = static_cast<LONG>(reinterpret_cast<INT_PTR>(m_group.m_memberList.GetHead()));

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->SetLastClick(CPoint(-1, -1));

        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        BOOL addAction = FALSE;
        switch (pSprite->m_currentUseButton.m_abilityId.m_itemType) {
        case 2:
            action = CAIAction(CAIAction::USEITEMPOINT,
                pt,
                pSprite->m_currentUseButton.m_abilityId.m_itemNum,
                pSprite->m_currentUseButton.m_abilityId.m_abilityNum);
            addAction = TRUE;
            break;
        case 3:
            if (1) {
                pSprite->m_castCounter = -1;

                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::SPELLPOINTNODEC,
                    string,
                    pt,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.field_1E << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.field_1D;
                addAction = TRUE;
            }
            break;
        case 4:
            if (1) {
                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::FORCESPELLPOINT,
                    string,
                    pt,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.field_1E << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.field_1D;
                addAction = TRUE;
            }
            break;
        case 5:
            pSprite->FireSpellPoint(pSprite->m_currentUseButton.m_abilityId.m_res, pt);
            break;
        default:
            if (1) {
                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::SPELLPOINT,
                    string,
                    pt,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.field_1E << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.field_1D;
                addAction = TRUE;
            }
            break;
        }

        if (addAction) {
            pSprite->ClearActions(FALSE);
            pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
            pSprite->m_triggerId = -1;
            pSprite->AddAction(action);
            pSprite->m_interrupt = TRUE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x5BC450
void CInfGame::UseMagicOnObject(LONG target)
{
    CAIAction action;
    CAIObjectType targetType(0, 0, 0, 0, 0, 0, 0, 0, target, 0, 0);

    LONG nCharacterId = static_cast<LONG>(reinterpret_cast<INT_PTR>(m_group.m_memberList.GetHead()));

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->SetLastClick(CPoint(-1, -1));

        // NOTE: Uninline.
        g_pBaldurChitin->GetObjectGame()->SetLastTarget(CGameObjectArray::INVALID_INDEX);

        BOOL addAction = FALSE;
        switch (pSprite->m_currentUseButton.m_abilityId.m_itemType) {
        case 2:
            action = CAIAction(CAIAction::USEITEM,
                targetType,
                pSprite->m_currentUseButton.m_abilityId.m_itemNum,
                pSprite->m_currentUseButton.m_abilityId.m_abilityNum,
                0);
            addAction = TRUE;
            break;
        case 3:
            if (1) {
                pSprite->m_castCounter = -1;

                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::SPELLNODEC,
                    targetType,
                    string,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.field_1E << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.field_1D;
                addAction = TRUE;
            }
            break;
        case 4:
            if (1) {
                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::FORCESPELL,
                    targetType,
                    string,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.field_1E << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.field_1D;
                addAction = TRUE;
            }
            break;
        case 5:
            if (1) {
                CGameObject* pTarget;

                BYTE rc;
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(target,
                        CGameObjectArray::THREAD_ASYNCH,
                        &pTarget,
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    pSprite->FireSpell(pSprite->m_currentUseButton.m_abilityId.m_res, pTarget);

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(target,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
            break;
        default:
            if (1) {
                CString string;
                pSprite->m_currentUseButton.m_abilityId.m_res.CopyToString(string);

                action = CAIAction(CAIAction::SPELL,
                    targetType,
                    string,
                    0,
                    pSprite->m_currentUseButton.m_abilityId.m_nClass | (pSprite->m_currentUseButton.m_abilityId.field_1E << 8));
                action.m_specificID3 = pSprite->m_currentUseButton.m_abilityId.field_1D;
                addAction = TRUE;
            }
            break;
        }

        if (addAction) {
            pSprite->ClearActions(FALSE);
            pSprite->m_userCommandPause = CGameSprite::USER_OVERRIDE_COUNT;
            pSprite->m_triggerId = -1;
            pSprite->AddAction(action);
            pSprite->m_interrupt = TRUE;
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }
}

// 0x5BD070
BOOL CInfGame::AddCharacterToParty(LONG id, SHORT nPortraitNumber)
{
    // NOTE: Uninline.
    if (GetCharacterPortraitNum(id) != -1) {
        return FALSE;
    }

    if (m_nCharacters >= CINFGAME_MAXCHARACTERS) {
        return FALSE;
    }

    if (m_allies.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(id))) != NULL) {
        // NOTE: Uninline.
        RemoveCharacterFromAllies(id);
    }

    if (m_familiars.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(id))) != NULL) {
        // NOTE: Uninline.
        RemoveCharacterFromFamiliars(id);
    }

    POSITION pos = m_lstGlobalCreatures.GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        if (static_cast<int>(reinterpret_cast<INT_PTR>(m_lstGlobalCreatures.GetNext(pos))) == id) {
            m_lstGlobalCreatures.RemoveAt(posOld);
            break;
        }
    }

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(id,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    if (nPortraitNumber == -1 || nPortraitNumber >= CINFGAME_MAXCHARACTERS) {
        nPortraitNumber = m_nCharacters;
    }

    m_characterPortraits[nPortraitNumber] = id;

    // NOTE: Inlined: put the character in the first free character slot.
    for (INT nCharacterSlot = 0; nCharacterSlot < CINFGAME_MAXCHARACTERS; nCharacterSlot++) {
        if (m_characters[nCharacterSlot] == -1) {
            m_characters[nCharacterSlot] = id;
            break;
        }
    }

    UpdatePortraitToolTip(nPortraitNumber, pSprite->GetNameRef());
    pSprite->SetFootstepChannel();

    if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) != 0) {
        pSprite->m_canBeSeen = 4 * (CGameObject::VISIBLE_DELAY + 1);
    }

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(id,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    EnablePortrait(static_cast<BYTE>(nPortraitNumber), TRUE);

    m_nCharacters++;

    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;
    pWorld->GetManager()->GetPanel(pWorld->GetPanel_22_0())->InvalidateRect(NULL);

    return TRUE;
}

// 0x5BD270
BOOL CInfGame::RemoveCharacterFromParty(LONG id, BOOL& overflow)
{
    CGameSprite* pSprite;
    BYTE rc;
    SHORT cnt;

    // NOTE: Uninline.
    SHORT nPortrait = GetCharacterPortraitNum(id);

    overflow = FALSE;

    if (nPortrait == -1) {
        for (int index = 0; index < m_characterOverflow.GetCount(); index++) {
            if (static_cast<int>(reinterpret_cast<INT_PTR>(m_characterOverflow.GetAt(index))) == id) {
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(id,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    overflow = TRUE;
                    m_characterOverflow.RemoveAt(index);

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(id,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    return TRUE;
                } else {
                    return FALSE;
                }
            }
        }
        return FALSE;
    }

    for (cnt = 0; cnt < CINFGAME_MAXCHARACTERS; cnt++) {
        if (m_characters[cnt] == id) {
            m_characters[cnt] = -1;
        }
    }

    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(id,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    m_group.Remove(pSprite);
    pSprite->SetFootstepChannel();
    pSprite->m_cGameStats.RecordLeaveParty();

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(id,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    for (cnt = nPortrait; cnt < m_nCharacters - 1; cnt++) {
        m_characterPortraits[cnt] = m_characterPortraits[cnt + 1];

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_characterPortraits[cnt],
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            UpdatePortraitToolTip(cnt, pSprite->GetNameRef());
            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_characterPortraits[cnt],
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    m_lstGlobalCreatures.AddHead(reinterpret_cast<int*>(static_cast<INT_PTR>(id)));

    m_nCharacters--;
    m_characterPortraits[m_nCharacters] = -1;

    EnablePortrait(static_cast<BYTE>(m_nCharacters), FALSE);

    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;
    pWorld->GetManager()->GetPanel(pWorld->GetPanel_22_0())->InvalidateRect(NULL);

    return TRUE;
}

// 0x5BD4E0
BOOL CInfGame::SetCharacterSlot(INT nCharacterSlot, LONG nCharacterId)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 15670
    UTIL_ASSERT(0 <= nCharacterSlot && nCharacterSlot < CINFGAME_MAXCHARACTERS);

    if (m_characters[nCharacterSlot] != -1) {
        return FALSE;
    }

    m_characters[nCharacterSlot] = nCharacterId;

    return TRUE;
}

// 0x5BD530
BOOL CInfGame::ClearCharacterSlot(INT nCharacterSlot)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 15717
    UTIL_ASSERT(0 <= nCharacterSlot && nCharacterSlot < CINFGAME_MAXCHARACTERS);

    LONG id = m_characters[nCharacterSlot];
    if (id == -1) {
        return FALSE;
    }

    m_characters[nCharacterSlot] = -1;

    BOOL overflow;
    RemoveCharacterFromParty(id, overflow);

    POSITION pos = m_lstGlobalCreatures.GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        if (static_cast<LONG>(reinterpret_cast<INT_PTR>(m_lstGlobalCreatures.GetNext(pos))) == id) {
            m_lstGlobalCreatures.RemoveAt(posOld);
            break;
        }
    }

    CGameSprite* pSprite;
    if (m_cObjectArray.GetDeny(id, CGameObjectArray::THREAD_ASYNCH, reinterpret_cast<CGameObject**>(&pSprite), INFINITE) != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    if (pSprite->GetArea() != NULL) {
        pSprite->GetArea()->m_visibility.RemoveCharacter(pSprite->GetPos(),
            pSprite->GetId(),
            pSprite->GetTerrainTable());

        if (pSprite->GetArea()->m_nCharacters != 0xFF) {
            pSprite->GetArea()->m_nCharacters--;
        }

        pSprite->RemoveFromArea();
    }

    m_cObjectArray.ReleaseDeny(id, CGameObjectArray::THREAD_ASYNCH, INFINITE);

    return TRUE;
}

// 0x5BD650
LONG CInfGame::GetCharacterSlot(INT nCharacterSlot)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 15787
    UTIL_ASSERT(0 <= nCharacterSlot && nCharacterSlot < CINFGAME_MAXCHARACTERS);

    return m_characters[nCharacterSlot];
}

// 0x5AE020
BOOLEAN CInfGame::sub_5AE020(LONG* pCharacterIds, SHORT nOverflowCount, LONG* pOverflowIds)
{
    CGameSprite* pSprite;
    BYTE rc;
    BOOL bOverflow;

    for (INT nIndex = 0; nIndex < m_characterOverflow.GetCount(); nIndex++) {
        LONG nCharacterId = static_cast<int>(reinterpret_cast<INT_PTR>(m_characterOverflow.GetAt(nIndex)));

        BOOLEAN bFound = FALSE;
        for (SHORT nOverflow = 0; nOverflow < nOverflowCount; nOverflow++) {
            if (pOverflowIds[nOverflow] == nCharacterId) {
                bFound = TRUE;
            }
        }

        if (bFound) {
            continue;
        }

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            return FALSE;
        }

        if (!RemoveCharacterFromParty(nCharacterId, bOverflow)) {
            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 10163
            UTIL_ASSERT_MSG(FALSE, "Failed to remove character from party");

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            return FALSE;
        }

        CAIObjectType typeAI(pSprite->GetAIType());
        CAIObjectType typeLive(pSprite->m_liveTypeAI);
        CAIObjectType typeStart(pSprite->m_liveTypeAI);
        typeAI.m_nEnemyAlly = CAIObjectType::EA_NEUTRAL;
        typeLive.m_nEnemyAlly = CAIObjectType::EA_NEUTRAL;
        typeStart.m_nEnemyAlly = CAIObjectType::EA_NEUTRAL;
        pSprite->SetAIType(typeAI, FALSE, FALSE);
        pSprite->m_liveTypeAI.Set(typeLive);
        pSprite->m_startTypeAI.Set(typeStart);

        if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
            || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
            CAITrigger trigger(CAITrigger::LEAVES, typeAI, 0);

            for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->m_nCharacters; nPortrait++) {
                // NOTE: Uninline.
                LONG nTargetId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);
                CMessage* message = new CMessageSetTrigger(trigger, pSprite->GetId(), nTargetId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }

            // NOTE: Animate() tests the dead bit of the derived general state.
            if (!pSprite->Animate()) {
                pSprite->DropInventory(FALSE, CString(""));

                if ((pSprite->GetBaseStats()->m_flags & 0x2) != 0) {
                    pSprite->m_expirationTime = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    for (SHORT nOverflow = 0; nOverflow < nOverflowCount; nOverflow++) {
        LONG nCharacterId = pOverflowIds[nOverflow];

        BOOLEAN bFound = FALSE;
        for (INT nIndex = 0; nIndex < m_characterOverflow.GetCount(); nIndex++) {
            if (static_cast<int>(reinterpret_cast<INT_PTR>(m_characterOverflow.GetAt(nIndex))) == nCharacterId) {
                bFound = TRUE;
            }
        }

        if (bFound) {
            continue;
        }

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc != CGameObjectArray::SUCCESS) {
            return FALSE;
        }

        m_characterOverflow.Add(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId)));
        m_nCharacterOverflowCount++;

        CAIObjectType typeAI(pSprite->GetAIType());
        CAIObjectType typeLive(pSprite->m_liveTypeAI);
        CAIObjectType typeStart(pSprite->m_liveTypeAI);
        typeAI.m_nEnemyAlly = CAIObjectType::EA_PC;
        typeLive.m_nEnemyAlly = CAIObjectType::EA_PC;
        typeStart.m_nEnemyAlly = CAIObjectType::EA_PC;
        pSprite->SetAIType(typeAI, FALSE, FALSE);
        pSprite->m_liveTypeAI.Set(typeLive);
        pSprite->m_startTypeAI.Set(typeStart);

        pSprite->m_dialog = "";

        if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
            || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
            CAITrigger trigger(CAITrigger::JOINS, typeAI, 0);

            for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->m_nCharacters; nPortrait++) {
                // NOTE: Uninline.
                LONG nTargetId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);
                CMessage* message = new CMessageSetTrigger(trigger, pSprite->GetId(), nTargetId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    for (INT nSlot = 0; nSlot < CINFGAME_MAXCHARACTERS; nSlot++) {
        if (m_characters[nSlot] != pCharacterIds[nSlot]) {
            if (m_characters[nSlot] != -1) {
                LONG nOldId = m_characters[nSlot];

                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nOldId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                if (!RemoveCharacterFromParty(nOldId, bOverflow)) {
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                    // __LINE__: 10357
                    UTIL_ASSERT_MSG(FALSE, "Failed to remove character from party");

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nOldId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    return FALSE;
                }

                POSITION pos = m_lstGlobalCreatures.GetHeadPosition();
                while (pos != NULL) {
                    POSITION posOld = pos;
                    if (static_cast<LONG>(reinterpret_cast<INT_PTR>(m_lstGlobalCreatures.GetNext(pos))) == nOldId) {
                        m_lstGlobalCreatures.RemoveAt(posOld);
                        break;
                    }
                }

                if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
                    if (!bOverflow) {
                        CMessage* message = new CMessageVisibilityMapMove(FALSE, nOldId, nOldId);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    }
                }

                if (!bOverflow) {
                    if (pSprite->GetArea() != NULL) {
                        if (pSprite->GetArea()->m_nCharacters != 0xFF) {
                            pSprite->GetArea()->m_nCharacters--;
                        }
                    }
                }

                CAIObjectType typeAI(pSprite->GetAIType());
                CAIObjectType typeLive(pSprite->m_liveTypeAI);
                CAIObjectType typeStart(pSprite->m_liveTypeAI);
                typeAI.m_nEnemyAlly = CAIObjectType::EA_NEUTRAL;
                typeLive.m_nEnemyAlly = CAIObjectType::EA_NEUTRAL;
                typeStart.m_nEnemyAlly = CAIObjectType::EA_NEUTRAL;
                pSprite->SetAIType(typeAI, FALSE, FALSE);
                pSprite->m_liveTypeAI.Set(typeLive);
                pSprite->m_startTypeAI.Set(typeStart);

                if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
                    CAITrigger trigger(CAITrigger::LEAVES, typeAI, 0);

                    for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->m_nCharacters; nPortrait++) {
                        // NOTE: Uninline.
                        LONG nTargetId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);
                        CMessage* message = new CMessageSetTrigger(trigger, pSprite->GetId(), nTargetId);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    }

                    // NOTE: Animate() tests the dead bit of the derived general state.
                    if (!pSprite->Animate()) {
                        pSprite->DropInventory(FALSE, CString(""));

                        if ((pSprite->GetBaseStats()->m_flags & 0x2) != 0) {
                            pSprite->m_expirationTime = g_pBaldurChitin->GetObjectGame()->GetWorldTimer()->m_gameTime;
                        }
                    }
                }

                if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE) {
                    m_multiplayerSettings.SetCharacterControlledByPlayer(nSlot, 0, FALSE, FALSE);
                    m_multiplayerSettings.SignalCharacterStatus(nSlot,
                        CMultiplayerSettings::CHARSTATUS_SIGNAL_DELETED,
                        TRUE,
                        FALSE);
                    m_multiplayerSettings.SetCharacterReady(nSlot, FALSE, FALSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nOldId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            if (pCharacterIds[nSlot] != -1) {
                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(pCharacterIds[nSlot],
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc != CGameObjectArray::SUCCESS) {
                    return FALSE;
                }

                if (!AddCharacterToParty(pCharacterIds[nSlot], -1)) {
                    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                    // __LINE__: 10455
                    UTIL_ASSERT_MSG(FALSE, "Failed to add character to party");

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pCharacterIds[nSlot],
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);

                    return FALSE;
                }

                if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
                    CMessage* message = new CMessageVisibilityMapMove(TRUE, pSprite->GetId(), pSprite->GetId());
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }

                if (pSprite->GetArea() != NULL) {
                    if (pSprite->GetArea()->m_nCharacters == 0xFF) {
                        pSprite->GetArea()->m_nCharacters = 1;
                    } else {
                        pSprite->GetArea()->m_nCharacters++;
                    }
                }

                CAIObjectType typeAI(pSprite->GetAIType());
                CAIObjectType typeLive(pSprite->m_liveTypeAI);
                CAIObjectType typeStart(pSprite->m_liveTypeAI);
                typeAI.m_nEnemyAlly = CAIObjectType::EA_PC;
                typeLive.m_nEnemyAlly = CAIObjectType::EA_PC;
                typeStart.m_nEnemyAlly = CAIObjectType::EA_PC;
                pSprite->SetAIType(typeAI, FALSE, FALSE);
                pSprite->m_liveTypeAI.Set(typeLive);
                pSprite->m_startTypeAI.Set(typeStart);

                pSprite->m_dialog = "";

                if (g_pChitin->cNetwork.GetServiceProvider() == CNetwork::SERV_PROV_NULL
                    || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
                    CAITrigger trigger(CAITrigger::JOINS, typeAI, 0);

                    for (SHORT nPortrait = 0; nPortrait < g_pBaldurChitin->GetObjectGame()->m_nCharacters; nPortrait++) {
                        // NOTE: Uninline.
                        LONG nTargetId = g_pBaldurChitin->GetObjectGame()->GetCharacterId(nPortrait);
                        CMessage* message = new CMessageSetTrigger(trigger, pSprite->GetId(), nTargetId);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    }
                }

                if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE) {
                    m_multiplayerSettings.SetCharacterControlledByPlayer(nSlot, 0, FALSE, FALSE);
                    m_multiplayerSettings.SignalCharacterStatus(nSlot,
                        CMultiplayerSettings::CHARSTATUS_SIGNAL_IMPORTED,
                        TRUE,
                        FALSE);
                    m_multiplayerSettings.SetCharacterReady(nSlot, TRUE, FALSE);
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(pCharacterIds[nSlot],
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }

            m_characters[nSlot] = pCharacterIds[nSlot];
        }
    }

    if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
        g_pBaldurChitin->GetBaldurMessage()->sub_432610();
        g_pBaldurChitin->GetBaldurMessage()->SendFullSettingsToClients(CString(""));
    }

    return TRUE;
}

// 0x5BD690
INT CInfGame::GetCharacterSlotFromId(LONG nCharacterId)
{
    for (int index = 0; index < CINFGAME_MAXCHARACTERS; index++) {
        if (m_characters[index] == nCharacterId) {
            return index;
        }
    }

    return -1;
}

// 0x5BD6B0
void CInfGame::UpdateCharacterSlots()
{
    CGameSprite* pSprite;
    BYTE rc;
    INT nCharacterSlot;
    INT nPortraits;
    INT index;

    m_nCharacters = 0;

    for (nCharacterSlot = 0; nCharacterSlot < CINFGAME_MAXCHARACTERS; nCharacterSlot++) {
        if (m_characters[nCharacterSlot] != -1
            && (!g_pChitin->cNetwork.GetSessionOpen()
                || m_multiplayerSettings.GetCharacterStatus(nCharacterSlot) == CMultiplayerSettings::CHARSTATUS_CHARACTER)) {
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(m_characters[nCharacterSlot],
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc != CGameObjectArray::SUCCESS) {
                return;
            }

            m_characters[m_nCharacters] = m_characters[nCharacterSlot];

            if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
                m_multiplayerSettings.SetCharacterControlledByPlayer(m_nCharacters,
                    m_multiplayerSettings.GetCharacterControlledByPlayer(nCharacterSlot),
                    FALSE,
                    FALSE);

                if (m_multiplayerSettings.GetCharacterStatus(nCharacterSlot) == CMultiplayerSettings::CHARSTATUS_CHARACTER) {
                    m_multiplayerSettings.SignalCharacterStatus(m_nCharacters,
                        CMultiplayerSettings::CHARSTATUS_SIGNAL_IMPORTED,
                        TRUE,
                        FALSE);
                } else {
                    m_multiplayerSettings.SignalCharacterStatus(m_nCharacters,
                        CMultiplayerSettings::CHARSTATUS_SIGNAL_DELETED,
                        TRUE,
                        FALSE);
                }

                m_multiplayerSettings.SetCharacterReady(m_nCharacters, TRUE, FALSE);
            }

            pSprite->m_cGameStats.RecordJoinParty();
            EnablePortrait(static_cast<BYTE>(m_nCharacters), TRUE);
            UpdatePortraitToolTip(m_nCharacters, pSprite->GetNameRef());

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(m_characters[nCharacterSlot],
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);

            m_nCharacters++;
        }
    }

    // Keep characters that already have a portrait in their portrait order.
    nPortraits = 0;
    for (nCharacterSlot = 0; nCharacterSlot < CINFGAME_MAXCHARACTERS; nCharacterSlot++) {
        for (index = 0; index < m_nCharacters; index++) {
            if (m_characters[index] == m_characterPortraits[nCharacterSlot]) {
                m_characterPortraits[nPortraits] = m_characters[index];
                nPortraits++;
                break;
            }
        }
    }

    // Append the rest.
    index = 0;
    while (nPortraits < m_nCharacters && index < m_nCharacters) {
        for (nCharacterSlot = 0; nCharacterSlot < CINFGAME_MAXCHARACTERS; nCharacterSlot++) {
            if (m_characters[index] == m_characterPortraits[nCharacterSlot]) {
                break;
            }
        }

        if (nCharacterSlot >= CINFGAME_MAXCHARACTERS) {
            m_characterPortraits[nPortraits] = m_characters[index];
            nPortraits++;
        }

        index++;
    }

    for (nCharacterSlot = m_nCharacters; nCharacterSlot < CINFGAME_MAXCHARACTERS; nCharacterSlot++) {
        m_characters[nCharacterSlot] = -1;
        m_characterPortraits[nCharacterSlot] = -1;

        if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
            m_multiplayerSettings.SetCharacterControlledByPlayer(nCharacterSlot, 0, FALSE, FALSE);
            m_multiplayerSettings.SignalCharacterStatus(nCharacterSlot,
                CMultiplayerSettings::CHARSTATUS_SIGNAL_DELETED,
                TRUE,
                FALSE);
            m_multiplayerSettings.SetCharacterReady(nCharacterSlot, FALSE, FALSE);
        }
    }

    if (g_pChitin->cNetwork.GetSessionHosting() == TRUE) {
        g_pBaldurChitin->GetBaldurMessage()->SendFullSettingsToClients(CString(""));
    }
}

// 0x5BD9D0
BOOL CInfGame::AddCharacterToAllies(LONG nCharacterId)
{
    // NOTE: Uninline.
    if (GetCharacterPortraitNum(nCharacterId) != -1) {
        return FALSE;
    }

    if (m_allies.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId))) != NULL) {
        return FALSE;
    }

    if (m_familiars.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId))) != NULL) {
        return FALSE;
    }

    m_allies.AddTail(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId)));

    return TRUE;
}

// 0x5BDA50
BOOL CInfGame::RemoveCharacterFromAllies(LONG nCharacterId)
{
    POSITION pos = m_allies.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId)));
    if (pos == NULL) {
        return FALSE;
    }

    m_allies.RemoveAt(pos);

    // FIXME: What for?
    if (m_allies.GetCount() == 0) {
        m_allies.RemoveAll();
    }

    return TRUE;
}

// 0x5BDAA0
BOOL CInfGame::AddCharacterToFamiliars(LONG nCharacterId)
{
    // NOTE: Uninline.
    if (GetCharacterPortraitNum(nCharacterId) != -1) {
        return FALSE;
    }

    if (m_allies.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId))) != NULL) {
        return FALSE;
    }

    if (m_familiars.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId))) != NULL) {
        return FALSE;
    }

    m_familiars.AddTail(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId)));

    return TRUE;
}

// 0x5BDB20
BOOL CInfGame::RemoveCharacterFromFamiliars(LONG nCharacterId)
{
    POSITION pos = m_familiars.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(nCharacterId)));
    if (pos == NULL) {
        return FALSE;
    }

    m_familiars.RemoveAt(pos);

    // FIXME: What for?
    if (m_familiars.GetCount() == 0) {
        m_familiars.RemoveAll();
    }

    return TRUE;
}

// 0x5BDB70
DWORD CInfGame::GetScrollSpeed()
{
    if (m_gameAreas[m_visibleArea]->m_nKeyScrollState != 0) {
        return m_cOptions.m_nKeyboardScrollSpeed;
    } else {
        return m_cOptions.m_nMouseScrollSpeed;
    }
}

// 0x5BDBA0
void CInfGame::sub_5BDBA0(int a1, BOOL a2)
{
    m_nReputation = a1 * 10;

    if (a2) {
        if (m_nReputation > 200) {
            m_nReputation = 200;
        }

        if (m_nReputation < 10) {
            m_nReputation = 10;
        }
    }
}

// 0x5BDC00
void CInfGame::ReputationAdjustment(SHORT reputationAdjustment, BOOL set)
{
    CGameSprite* pSprite;
    BYTE rc;

    if (set) {
        reputationAdjustment -= m_nReputation;
    }

    if (!g_pBaldurChitin->GetBaldurMessage()->m_bInReputationChange) {
        CMessage* message = new CMessageReputationChange(reputationAdjustment,
            GetProtagonist(),
            GetProtagonist());
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
    }

    m_nReputation += reputationAdjustment;

    if (m_nReputation > 200) {
        m_nReputation = 200;
    }

    if (m_nReputation < 10) {
        m_nReputation = 10;
    }

    ITEM_EFFECT effect;
    CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_REPUTATION);
    // NOTE: Read from an unnamed WORD constant at 0x8491AA.
    effect.durationType = 1;
    effect.dwFlags = 1;
    effect.effectAmount = m_nReputation;

    for (SHORT nIndex = 0; nIndex < m_nCharacters; nIndex++) {
        // NOTE: Uninline.
        LONG nCharacterId = GetCharacterId(nIndex);

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (g_pChitin->cNetwork.m_nServiceProvider == CNetwork::SERV_PROV_NULL
                || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
                SHORT nColumn = 0;
                if ((pSprite->GetAIType().m_nAlignment & 0xF) == 2) {
                    nColumn = 1;
                } else if ((pSprite->GetAIType().m_nAlignment & 0xF) == 3) {
                    nColumn = 2;
                }

                SHORT nRow = static_cast<SHORT>(m_nReputation / REPUTATION_MULTIPLIER - 1);
                SHORT nHappiness = static_cast<SHORT>(atol(m_ruleTables.m_tHappiness.GetAt(CPoint(nColumn, nRow))));

                CMessage* pHappiness = new CMessageSetHappiness(nHappiness,
                    nCharacterId,
                    nCharacterId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pHappiness, FALSE);

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    CPoint(-1, -1),
                    -1,
                    CPoint(-1, -1));

                // NOTE: Uninline (both ids).
                CMessage* pMessage = new CMessageAddEffect(pEffect,
                    GetCharacterId(nIndex),
                    GetCharacterId(nIndex));
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

                pSprite->CheckReputation();
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (!g_pChitin->cNetwork.GetSessionOpen()
        || g_pChitin->cNetwork.m_bIsHost == TRUE) {
        FeedBack(FEEDBACK_REPUTATION, reputationAdjustment / REPUTATION_MULTIPLIER, TRUE);
    }
}

// 0x5BDF80
void CInfGame::ChangeReputation(SHORT eventCode)
{
    LONG repMod = atol(m_ruleTables.m_tReputationMod.GetAt(CPoint(eventCode, m_nReputation / REPUTATION_MULTIPLIER - 1)));
    ReputationAdjustment(static_cast<SHORT>(repMod), FALSE);
}

// 0x5BF4B0
void CInfGame::AddDisposableItem(CItem* pItem)
{
    if (pItem != NULL) {
        CSingleLock disposableItemsLock(&m_disposableItemsCritSect, FALSE);
        disposableItemsLock.Lock(INFINITE);

        m_lDisposableItems.AddTail(pItem);

        disposableItemsLock.Unlock();
    }
}

// 0x5BF540
void CInfGame::DestroyDisposableItems()
{
    if (m_lDisposableItems.GetCount() > 0) {
        CSingleLock disposableItemsLock(&m_disposableItemsCritSect, FALSE);
        disposableItemsLock.Lock(INFINITE);

        POSITION pos = m_lDisposableItems.GetHeadPosition();
        while (pos != NULL) {
            CItem* pItem = m_lDisposableItems.GetAt(pos);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 16931
            UTIL_ASSERT(pItem != NULL);

            delete pItem;

            // NOTE: Meaningless.
            m_lDisposableItems.SetAt(pos, NULL);

            m_lDisposableItems.GetNext(pos);
        }

        m_lDisposableItems.RemoveAll();

        disposableItemsLock.Unlock();
    }
}

// 0x5BF610
void CInfGame::AddPartyGold(LONG dwAddPartyGold)
{
    if (dwAddPartyGold != 0) {
        if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
            CMessagePartyGold* pMessage = new CMessagePartyGold(FALSE,
                TRUE,
                dwAddPartyGold,
                CGameObjectArray::INVALID_INDEX,
                CGameObjectArray::INVALID_INDEX);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        } else {
            if (dwAddPartyGold >= 0 || static_cast<DWORD>(-dwAddPartyGold) <= m_gameSave.m_nPartyGold) {
                m_gameSave.m_nPartyGold += dwAddPartyGold;
            } else {
                m_gameSave.m_nPartyGold = 0;
            }
        }
    }
}

// 0x5BF6A0
void CInfGame::SetupCharacters(BOOLEAN bProgressBarInPlace)
{
    CNetwork* pNetwork = &(g_pBaldurChitin->cNetwork);
    CResRef cResArea;
    CString sArea;
    CPoint ptView;
    CPoint ptStart;
    int nFacing;
    CGameArea* pArea;
    CGameSprite* pSprite;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    pGame->GetRuleTables().GetStartArea(cResArea, ptView);
    cResArea.CopyToString(sArea);

    if (m_bFromNewGame) {
        pArea = LoadArea(sArea, -1, TRUE, bProgressBarInPlace);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 17025
        UTIL_ASSERT(pArea != NULL);

        pArea->GetInfinity()->SetViewPosition(ptView.x - (pArea->GetInfinity()->rViewPort.right - pArea->GetInfinity()->rViewPort.left) / 2,
            ptView.y - (pArea->GetInfinity()->rViewPort.bottom - pArea->GetInfinity()->rViewPort.top) / 2,
            TRUE);
    } else {
        pArea = LoadArea(m_multiplayerSettings.GetAreaStartName(), -1, FALSE, bProgressBarInPlace);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 17032
        UTIL_ASSERT(pArea != NULL);
    }

    for (SHORT nCharacterSlot = 0; nCharacterSlot < CINFGAME_MAXCHARACTERS; nCharacterSlot++) {
        // NOTE: Inlined.
        LONG nCharacterId = nCharacterSlot < CINFGAME_MAXCHARACTERS
            ? m_characters[nCharacterSlot]
            : CGameObjectArray::INVALID_INDEX;

        if (pNetwork->GetSessionHosting()
            && m_multiplayerSettings.GetCharacterStatus(nCharacterSlot) != CMultiplayerSettings::CHARSTATUS_CHARACTER) {
            continue;
        }

        if (nCharacterId == CGameObjectArray::INVALID_INDEX) {
            continue;
        }

        BYTE rc = m_cObjectArray.GetDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            continue;
        }

        if (nCharacterSlot == 0 && m_nReputation / REPUTATION_MULTIPLIER < 0) {
            m_nReputation = pGame->GetRuleTables().GetStartingReputation(pSprite->m_startTypeAI.m_nAlignment) * 10;
            m_nReputation = m_nReputation > 200 ? 200 : m_nReputation;
            m_nReputation = m_nReputation < 10 ? 10 : m_nReputation;
        }

        pSprite->GetBaseStats()->m_gold = 0;

        CPoint& pos = pSprite->GetPos();
        if (pos.x == -1 && pos.y == -1) {
            if (m_bFromNewGame) {
                ptStart = pGame->GetRuleTables().GetStartPoint(nCharacterSlot);
                nFacing = pGame->GetRuleTables().GetStartRotation(nCharacterSlot);
            } else if (pNetwork->GetSessionOpen()) {
                CPoint ptAreaStart = m_multiplayerSettings.m_ptAreaStart;
                BYTE nPersonalSpace = pSprite->GetAnimation()->GetPersonalSpace();
                CPoint ptSearch = pArea->m_search.GetNearestOpenSquare(CPoint(ptAreaStart.x / CPathSearch::GRID_SQUARE_SIZEX, ptAreaStart.y / CPathSearch::GRID_SQUARE_SIZEY),
                    pSprite->GetTerrainTable(),
                    nPersonalSpace,
                    -1);
                ptStart.x = ptSearch.x * CPathSearch::GRID_SQUARE_SIZEX;
                ptStart.y = ptSearch.y * CPathSearch::GRID_SQUARE_SIZEY;
            } else {
                ptStart.x = -1;
                ptStart.y = -1;
            }

            m_bInLoadGame = TRUE;

            BOOL bThreadPriorityChanged = FALSE;
            int nOldThreadPriority = GetThreadPriority(g_pChitin->m_hMusicThread);
            if (GetPrivateProfileIntA("Program Options", "Volume Music", 0, g_pChitin->GetIniFileName())) {
                bThreadPriorityChanged = SetThreadPriority(g_pChitin->m_hMusicThread, 15);
            }

            if (pSprite->GetArea() != NULL) {
                pSprite->RemoveFromArea();
            }

            if ((pSprite->GetBaseStats()->m_generalState & STATE_DEAD) != 0
                && pSprite->GetAnimation()->CanLieDown()) {
                pSprite->AddToArea(pArea,
                    ptStart,
                    0,
                    CGameObject::LIST_BACK);
            } else {
                pSprite->AddToArea(pArea,
                    ptStart,
                    0,
                    CGameObject::LIST_FRONT);
            }

            if (m_bFromNewGame) {
                pSprite->SetFacing(static_cast<SHORT>(nFacing));
            }

            m_gameSave.m_cResCurrentWorldArea = pArea->m_resRef;
            m_gameSave.m_nCurrentWorldLink = -1;
            m_bInLoadGame = FALSE;

            if (bThreadPriorityChanged == TRUE) {
                SetThreadPriority(g_pChitin->m_hMusicThread, nOldThreadPriority);
            }
        }

        m_cObjectArray.ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    if (m_characters[0] != -1) {
        SetProtagonist(m_characters[0]);
    }
}

// 0x5BDFE0
LONG CInfGame::sub_5BDFE0(SHORT nType)
{
    CGameSprite* pSprite;
    BYTE rc;
    SHORT nIndex;
    SHORT nBest;
    INT nClassLevel = 0;
    INT nValue;
    LONG nResult = CGameObjectArray::INVALID_INDEX;
    LONG nCharacterId;

    switch (nType) {
    case 1:
        nBest = 0x7FFF;
        for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
            // NOTE: Uninline.
            nCharacterId = GetCharacterId(nIndex);

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                    SHORT nLevel = pSprite->m_derivedStats.m_nLevel;
                    if (nLevel < nBest) {
                        nResult = nCharacterId;
                        nBest = nLevel;
                        nClassLevel = pSprite->m_derivedStats.GetClassMaskLevel(0xD1);
                    } else if (nLevel == nBest) {
                        nValue = pSprite->m_derivedStats.GetClassMaskLevel(0xD1);
                        if (nValue < nClassLevel) {
                            nResult = nCharacterId;
                            nBest = pSprite->m_derivedStats.m_nLevel;
                            nClassLevel = nValue;
                        }
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    case 2:
        nBest = 0;
        for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
            // NOTE: Uninline.
            nCharacterId = GetCharacterId(nIndex);

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                    SHORT nLevel = pSprite->m_derivedStats.m_nLevel;
                    if (nLevel > nBest) {
                        nResult = nCharacterId;
                        nBest = nLevel;
                        nClassLevel = pSprite->m_derivedStats.GetClassMaskLevel(0xD1);
                    } else if (nLevel == nBest) {
                        nValue = pSprite->m_derivedStats.GetClassMaskLevel(0xD1);
                        if (nValue > nClassLevel) {
                            nResult = nCharacterId;
                            nBest = pSprite->m_derivedStats.m_nLevel;
                            nClassLevel = nValue;
                        }
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    case 3:
        nBest = 0;
        for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
            // NOTE: Uninline.
            nCharacterId = GetCharacterId(nIndex);

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                    INT nDamage = pSprite->m_derivedStats.m_nMaxHitPoints - pSprite->m_baseStats.m_hitPoints;
                    if (nDamage > nBest) {
                        nResult = nCharacterId;
                        nBest = nDamage;
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    case 4:
        nBest = 0x7FFF;
        for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
            // NOTE: Uninline.
            nCharacterId = GetCharacterId(nIndex);

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                    INT nDamage = pSprite->m_derivedStats.m_nMaxHitPoints - pSprite->m_baseStats.m_hitPoints;
                    if (nDamage < nBest) {
                        nResult = nCharacterId;
                        nBest = nDamage;
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    case 5:
        nBest = 0;
        for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
            // NOTE: Uninline.
            nCharacterId = GetCharacterId(nIndex);

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0
                    && pSprite->GetAIType().m_nGender == CAIObjectType::SEX_MALE) {
                    SHORT nLevel = pSprite->m_derivedStats.m_nLevel;
                    if (nLevel > nBest) {
                        nResult = nCharacterId;
                        nBest = nLevel;
                        nClassLevel = pSprite->m_derivedStats.GetClassMaskLevel(0xD1);
                    } else if (nLevel == nBest) {
                        nValue = pSprite->m_derivedStats.GetClassMaskLevel(0xD1);
                        if (nValue > nClassLevel) {
                            nResult = nCharacterId;
                            nBest = pSprite->m_derivedStats.m_nLevel;
                            nClassLevel = nValue;
                        }
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    case 6:
        nBest = -20;
        for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
            // NOTE: Uninline.
            nCharacterId = GetCharacterId(nIndex);

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                    if (pSprite->GetAC() > nBest) {
                        nResult = nCharacterId;
                        nBest = pSprite->GetAC();
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    case 7:
        nBest = 20;
        for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
            // NOTE: Uninline.
            nCharacterId = GetCharacterId(nIndex);

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                    if (pSprite->GetAC() < nBest) {
                        nResult = nCharacterId;
                        nBest = pSprite->GetAC();
                    }
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }
        break;
    }

    return nResult;
}

// 0x5BFC40
LONG CInfGame::GetProtagonist()
{
    LONG nProtagonist = 0;
    LONG nCharacterId;
    CGameSprite* pSprite;
    BYTE rc;
    SHORT nIndex;

    if (g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.field_54) {
        nProtagonist = g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.m_characterIndex;
    }

    if (nProtagonist == 0) {
        // NOTE: Loop runs one extra time (`<=`), the last iteration with
        // `INVALID_INDEX`.
        for (nIndex = 0; nIndex <= m_nCharacters; nIndex++) {
            // NOTE: Inlined.
            nCharacterId = nIndex < m_nCharacters
                ? m_characterPortraits[nIndex]
                : CGameObjectArray::INVALID_INDEX;

            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                if (pSprite->m_bSelected && !IcewindMisc::IsDead(pSprite)) {
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                    nProtagonist = nCharacterId;
                    break;
                }

                g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
            }
        }

        if (nProtagonist == 0) {
            // NOTE: Loop runs one extra time (`<=`).
            for (nIndex = 0; nIndex <= m_nCharacters; nIndex++) {
                // NOTE: Inlined.
                nCharacterId = nIndex < CINFGAME_MAXCHARACTERS
                    ? m_characters[nIndex]
                    : CGameObjectArray::INVALID_INDEX;

                do {
                    rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    if (!IcewindMisc::IsDead(pSprite)) {
                        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                        nProtagonist = nCharacterId;
                        break;
                    }

                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }

            if (nProtagonist == 0) {
                nProtagonist = m_characters[0];
            }
        }
    }

    CMessageSetProtagonist* pMessage = new CMessageSetProtagonist(nProtagonist,
        nProtagonist,
        nProtagonist);
    g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);

    return nProtagonist;
}

// 0x5BFE10
void CInfGame::SetProtagonist(LONG nId)
{
    STR_RES strRes;

    if (g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.field_54) {
        nId = g_pBaldurChitin->m_pEngineWorld->m_internalLoadedDialog.m_characterIndex;
    }

    STRREF strSurfaceUnderdark = 25688; // "sun-lander"

    if (nId == -1) {
        return;
    }

    CGameSprite* pSprite;

    BYTE rc;
    do {
        rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    // NOTE: Original code is slightly different. It sets token only if
    // `GetName` returns non-NULL, but this implementation already returns
    // reference.
    g_pBaldurChitin->GetTlkTable().SetToken(TOKEN_CHARACTER, pSprite->GetName());

    STRREF strRace;
    switch (pSprite->GetAIType().m_nRace) {
    case CAIOBJECTTYPE_R_HUMAN:
        strRace = 7193;
        break;
    case CAIOBJECTTYPE_R_ELF:
        strRace = 7194;
        if (pSprite->GetAIType().m_nSubRace == 1) {
            strSurfaceUnderdark = 25689; // "outsider"
        }
        break;
    case CAIOBJECTTYPE_R_HALF_ELF:
        strRace = 7197;
        break;
    case CAIOBJECTTYPE_R_DWARF:
        strRace = 7182;
        if (pSprite->GetAIType().m_nSubRace == 2
            || pSprite->GetAIType().m_nSubRace == 1) {
            strSurfaceUnderdark = 25689; // "outsider"
        }
        break;
    case CAIOBJECTTYPE_R_HALFLING:
        strRace = 7195;
        break;
    case CAIOBJECTTYPE_R_GNOME:
        strRace = 7196;
        if (pSprite->GetAIType().m_nSubRace == 1) {
            strSurfaceUnderdark = 25689; // "outsider"
        }
        break;
    case CAIOBJECTTYPE_R_HALF_ORC:
        strRace = 22;
        break;
    default:
        strRace = -1;
        break;
    }

    g_pBaldurChitin->GetTlkTable().Fetch(strRace, strRes);
    g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_RACE, strRes.szText);

    g_pBaldurChitin->GetTlkTable().Fetch(strSurfaceUnderdark, strRes);
    g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_SURFACE_UNDERDARK, strRes.szText);

    if (pSprite->GetAIType().m_nGender == CAIObjectType::SEX_FEMALE) {
        g_pBaldurChitin->GetTlkTable().Fetch(27475, strRes); // "ma'am"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_SIRMAAM, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27476, strRes); // "girl"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_GIRLBOY, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27479, strRes); // "sister"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_BROTHERSISTER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27480, strRes); // "lady"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_LADYLORD, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27483, strRes); // "female"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_MALEFEMALE, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27485, strRes); // "she"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HESHE, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27487, strRes); // "her"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HISHER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27487, strRes); // "her"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HIMHER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27490, strRes); // "woman"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_MANWOMAN, strRes.szText);
    } else {
        g_pBaldurChitin->GetTlkTable().Fetch(27473, strRes); // "sir"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_SIRMAAM, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27477, strRes); // "boy"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_GIRLBOY, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27478, strRes); // "brother"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_BROTHERSISTER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27481, strRes); // "lord"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_LADYLORD, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27482, strRes); // "male"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_MALEFEMALE, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27484, strRes); // "he"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HESHE, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27486, strRes); // "his"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HISHER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27488, strRes); // "him"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_HIMHER, strRes.szText);

        g_pBaldurChitin->GetTlkTable().Fetch(27489, strRes); // "man"
        g_pBaldurChitin->GetTlkTable().SetToken(CInfGame::TOKEN_MANWOMAN, strRes.szText);
    }

    m_nProtagonistId = nId;

    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x5C04F0
BOOLEAN CInfGame::FindAreaID(DWORD nAreaId)
{
    for (int index = 0; index < CINFGAME_MAX_AREAS; index++) {
        if (m_gameAreas[index] != NULL) {
            if (m_gameAreas[index]->m_nInitialAreaID == nAreaId) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

// 0x5C09C0
CString CInfGame::GetDirSaveRoot()
{
    CString sResolvedFileName;
    CString sFileName;

    sFileName = m_sMultiplayerSaveDir;
    if (!g_pBaldurChitin->lAliases.ResolveFileName(sFileName, sResolvedFileName)) {
        sResolvedFileName = sFileName;
    }

    return sResolvedFileName;
}

// 0x5C0A80
CString CInfGame::GetDirSave()
{
    return m_sMultiplayerSaveDir + m_sSaveGame + "\\";
}

// TODO: Somehow its different from function above. Figure out.
//
// 0x5C0B30
CString CInfGame::sub_5C0B30()
{
    return m_sMultiplayerSaveDir + m_sSaveGame + "\\";
}

// 0x5C20E0
BOOL CInfGame::CanEnterStore(STRREF& strError)
{
    BOOL bScattered = FALSE;
    CResRef cResArea;

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        if (bScattered) {
            break;
        }

        LONG nCharacterId = GetCharacterId(nPortrait);

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = m_cObjectArray.GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->Animate()) {
                if (cResArea != "") {
                    if (cResArea != pSprite->GetArea()->m_resRef) {
                        bScattered = TRUE;
                    }
                } else {
                    cResArea = pSprite->GetArea()->m_resRef;
                }
            }

            m_cObjectArray.ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bScattered = TRUE;
        }
    }

    if (!CanSaveGame(strError, TRUE, TRUE)) {
        if (strError == 16501) {
            // "You cannot enter the store; there are monsters nearby."
            strError = 20049;
        } else if (strError == 16502) {
            // "You may not enter the store at this time."
            strError = 261;
        } else {
            // "Not permitted to buy/sell items."
            strError = 10847;
        }
        return FALSE;
    }

    if (bScattered) {
        // "You may not enter the store when your party is scattered."
        strError = 20690;
        return FALSE;
    }

    return TRUE;
}

// 0x5C2250
BOOL CInfGame::AddCharacterToOverflow(LONG id)
{
    m_characterOverflow.Add(reinterpret_cast<int*>(static_cast<INT_PTR>(id)));
    m_nCharacterOverflowCount++;
    return TRUE;
}

// 0x5C2280
CStringList* CInfGame::GetScripts()
{
    CStringList* pList = new CStringList();

    CString sFileName;
    CString sPattern;
    CString sTemp;

    sPattern = GetDirScripts() + "*.*";

    INT nCount = 0;
    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFile = FindFirstFileA(sPattern, &findFileData);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sFileName = findFileData.cFileName;
            if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0
                && sFileName != "."
                && sFileName != "..") {
                sFileName = sFileName.SpanExcluding(".");
                if (sFileName.GetLength() <= 8) {
                    if (nCount >= 16383) {
                        break;
                    }

                    sFileName.MakeUpper();

                    POSITION pos = pList->GetHeadPosition();
                    while (pos != NULL) {
                        sTemp = pList->GetAt(pos);
                        if (sFileName < pList->GetAt(pos)) {
                            pList->InsertBefore(pos, sFileName);
                            nCount++;
                            break;
                        }

                        pList->GetNext(pos);
                    }

                    if (pos == NULL) {
                        pList->AddTail(sFileName);
                        nCount++;
                    }
                }
            }
        } while (FindNextFileA(hFindFile, &findFileData));
        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C24C0
CString CInfGame::GetDirScripts()
{
    return m_sScriptsDir;
}

// 0x5C24F0
void CInfGame::StartCharacterTerminationSequence()
{
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    if (pGame->m_gameSave.m_mode == 322) {
        pGame->m_gameSave.m_mode = -1;
    }

    pGame->m_gameSave.m_cutScene = FALSE;

    if (g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineWorld) {
        g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineWorld);
    }

    g_pBaldurChitin->m_pEngineWorld->m_bGameOverPanel = TRUE;
}

// 0x5C2570
void CInfGame::ReadyCharacterTerminationSequence(int a1, int a2)
{
    if (!m_gameSave.field_1AC) {
        // FIXME: What for (this function is not static as
        // `StartCharacterTerminationSequence`).
        CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

        pGame->m_gameSave.m_mode = 322;
        pGame->m_gameSave.m_cutScene = TRUE;

        g_pBaldurChitin->GetObjectCursor()->SetCursor(0, FALSE);

        pGame->m_tempCursor = 4;
        pGame->m_gameAreas[pGame->m_visibleArea]->m_nScrollState = 0;
        pGame->m_nCharacterTerminationSequenceDelay = 150;

        g_pBaldurChitin->m_pEngineWorld->field_10F0 = a1;
        g_pBaldurChitin->m_pEngineWorld->field_10F4 = a2;
    }
}

// 0x5C2630
void CInfGame::FeedBack(SHORT feedBackId, LONG value, BOOLEAN a3)
{
    CString sTime;

    if (!a3) {
        a3 = m_cOptions.m_bShowQuestXP;
    }

    switch (feedBackId) {
    case FEEDBACK_GOLD:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            CString sText;
            STR_RES strRes;

            if (value != 0) {
                if (value > 0) {
                    // "The party has gained gold"
                    g_pBaldurChitin->GetTlkTable().Fetch(17572, strRes);
                } else {
                    // "The party has lost gold"
                    g_pBaldurChitin->GetTlkTable().Fetch(17573, strRes);
                    value = -value;
                }
                sText.Format("%d", value);

                if (strRes.cSound.GetRes() != NULL) {
                    if (!strRes.cSound.GetLooping()) {
                        strRes.cSound.SetFireForget(TRUE);
                    }
                    strRes.cSound.SetChannel(14, reinterpret_cast<DWORD_PTR>(GetVisibleArea()));
                    strRes.cSound.Play(FALSE);
                }

                if (strRes.szText != "") {
                    if (a3) {
                        sText = strRes.szText + ": " + sText;
                    } else {
                        sText = strRes.szText + ".";
                    }
                }

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sText,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_EXPERIENCE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            CString sText;
            STR_RES strRes;

            if (value != 0) {
                if (value > 0) {
                    // "The party has gained experience"
                    g_pBaldurChitin->GetTlkTable().Fetch(17574, strRes);
                } else {
                    // "The party has lost experience"
                    g_pBaldurChitin->GetTlkTable().Fetch(17575, strRes);
                    value = -value;
                }
                sText.Format("%d", value);

                if (strRes.cSound.GetRes() != NULL) {
                    if (!strRes.cSound.GetLooping()) {
                        strRes.cSound.SetFireForget(TRUE);
                    }
                    strRes.cSound.SetChannel(14, reinterpret_cast<DWORD_PTR>(GetVisibleArea()));
                    strRes.cSound.Play(FALSE);
                }

                if (strRes.szText != "") {
                    if (a3) {
                        sText = strRes.szText + ": " + sText;
                    } else {
                        sText = strRes.szText + ".";
                    }
                }

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sText,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_ITEMGAINED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "The party has gained an item."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                17576,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    case FEEDBACK_ITEMLOST:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "The party has lost an item."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                17577,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    case FEEDBACK_REPUTATION:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            CString sText;
            STR_RES strRes;

            if (value != 0) {
                if (value > 0) {
                    // "The party's reputation has increased"
                    g_pBaldurChitin->GetTlkTable().Fetch(19686, strRes);
                } else {
                    // "The party's reputation has decreased"
                    g_pBaldurChitin->GetTlkTable().Fetch(19687, strRes);
                    value = -value;
                }
                sText.Format("%d", value);

                if (strRes.cSound.GetRes() != NULL) {
                    if (!strRes.cSound.GetLooping()) {
                        strRes.cSound.SetFireForget(TRUE);
                    }
                    strRes.cSound.SetChannel(14, reinterpret_cast<DWORD_PTR>(GetVisibleArea()));
                    strRes.cSound.Play(FALSE);
                }

                if (strRes.szText != "") {
                    if (a3) {
                        sText = strRes.szText + ": " + sText;
                    } else {
                        sText = strRes.szText + ".";
                    }
                }

                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sText,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_TIMEPASSEDJOURNEY:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // NOTE: Signed compare.
            if (value >= static_cast<LONG>(CTimerWorld::TIMESCALE_MSEC_PER_HOUR)) {
                // "The journey took <DURATION>."
                CTimerWorld::GetCurrentTimeString(value, 10689, sTime);
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sTime,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_TIMEPASSEDREST:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // NOTE: Signed compare.
            if (value >= static_cast<LONG>(CTimerWorld::TIMESCALE_MSEC_PER_HOUR)) {
                // "You have rested for <DURATION>."
                CTimerWorld::GetCurrentTimeString(value, 10690, sTime);
                g_pBaldurChitin->GetBaldurMessage()->DisplayText(CString(""),
                    sTime,
                    RGB(0, 0, 0),
                    RGB(255, 255, 0),
                    -1,
                    0,
                    0);
            }
        }
        break;
    case FEEDBACK_INVENTORYFULL_ITEMDROPPED:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "Inventory Full: The item has been dropped."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                10959,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    case FEEDBACK_JOURNAL_UPDATE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "Your journal has been updated."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                11359,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    case FEEDBACK_WORLDMAP_UPDATE:
        if ((g_pBaldurChitin->GetObjectGame()->GetOptions()->m_nEffectTextLevel & 0x10) != 0) {
            // "Your map has been updated."
            g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
                11360,
                RGB(0, 0, 0),
                RGB(255, 255, 0),
                -1,
                0,
                0);
        }
        break;
    }
}

// 0x5C3210
CString CInfGame::GetDirPortraits()
{
    return m_sPortraitsDir;
}

// 0x5C3510
CString CInfGame::GetDirCharacters()
{
    return m_sCharactersDir;
}

// 0x5C0BB0
LONG CInfGame::AddPartyXP(DWORD nXP, BOOLEAN bFeedback, BOOLEAN bChallengeRating)
{
    CGameSprite* pSprite;
    BYTE rc;
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    SHORT nIndex;
    SHORT nLevels = 0;
    SHORT nAlive = 0;

    for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
        // NOTE: Uninline.
        LONG nCharacterId = GetCharacterId(nIndex);

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                nAlive++;
                nLevels += pSprite->m_derivedStats.m_nLevel;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (nAlive == 0) {
        nAlive = 1;
    }

    if (bChallengeRating) {
        nXP = static_cast<DWORD>(pGame->m_ruleTables.sub_542560(static_cast<unsigned short>(nXP),
                  static_cast<unsigned short>(nLevels / nAlive)))
            >> 1;
    }

    DWORD nShare = nXP / nAlive;
    DWORD nRemainder = nXP % nAlive;

    ITEM_EFFECT effect;
    CGameEffect::ClearItemEffect(&effect, CGAMEEFFECT_XP);
    // NOTE: Read from an unnamed WORD constant at 0x8491AA.
    effect.durationType = 1;
    effect.effectAmount = nShare;

    LONG nTotal = 0;

    for (nIndex = 0; nIndex < m_nCharacters; nIndex++) {
        // NOTE: Uninline.
        LONG nCharacterId = GetCharacterId(nIndex);

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if ((pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                // NOTE: The original compares the remainder as SHORT.
                if (static_cast<SHORT>(nRemainder) > 0) {
                    effect.effectAmount = nShare + 1;
                    nRemainder--;
                } else {
                    effect.effectAmount = nShare;
                }

                INT nPenalty = pSprite->GetMulticlassingPenalty();
                if (nPenalty > 0) {
                    effect.effectAmount = (100 - nPenalty) * effect.effectAmount / 100;
                }

                nTotal += effect.effectAmount;

                CGameEffect* pEffect = CGameEffect::DecodeEffect(&effect,
                    CPoint(-1, -1),
                    -1,
                    CPoint(-1, -1));

                // NOTE: Uninline (both ids).
                CMessageAddEffect* pMessage = new CMessageAddEffect(pEffect,
                    GetCharacterId(nIndex),
                    GetCharacterId(nIndex));
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    FeedBack(FEEDBACK_EXPERIENCE, nTotal, bFeedback);

    return nTotal;
}

// 0x5C1090
BYTE CInfGame::GetCharactersControlled()
{
    BYTE bControlled = 0;
    CGameObject* pObject;

    for (SHORT nIndex = 0; nIndex < m_nCharacters; nIndex++) {
        LONG id = m_characterPortraits[nIndex];

        BYTE rc;
        do {
            // NOTE: Using global. Implying inlined static function.
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(id,
                CGameObjectArray::THREAD_ASYNCH,
                &pObject,
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            // NOTE: Uninline.
            if (pObject->InControl()) {
                bControlled |= 1 << nIndex;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(id,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    return bControlled;
}

// 0x5C1160
BOOLEAN CInfGame::RestParty(INT nHP, BOOLEAN bRenting)
{
    if (bRenting) {
        for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
            // NOTE: Uninline.
            LONG nCharacterId = GetCharacterId(nPortrait);

            CMessage101* pMessage = new CMessage101(FALSE,
                nCharacterId,
                nCharacterId,
                TRUE);

            g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
        }
    }

    if (!g_pChitin->cNetwork.m_bConnectionEstablished) {
        return RestPartyPrivate(nHP, bRenting);
    }

    if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE) {
        if (!g_pChitin->cNetwork.m_bIsHost) {
            if (g_pBaldurChitin->m_cBaldurMessage.m_bInOnRestAnnounce) {
                return RestPartyPrivate(nHP, bRenting);
            }

            g_pBaldurChitin->m_cBaldurMessage.SendRestRequestToServer(nHP,
                bRenting,
                static_cast<BYTE>(m_bHealPartyOnRest));

            return TRUE;
        }

        if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
            BOOLEAN bCanStart = TRUE;

            if (g_pBaldurChitin->GetObjectGame()->m_gameSave.m_mode == 386
                || g_pBaldurChitin->GetObjectGame()->m_gameSave.m_mode == 1282) {
                bCanStart = FALSE;
            }

            if (g_pBaldurChitin->GetObjectGame()->m_multiplayerSettings.m_bHostPermittedDialog == TRUE) {
                bCanStart = FALSE;
            }

            if (g_pBaldurChitin->m_pEngineWorld->m_bHardPaused == TRUE || !bCanStart) {
                g_pBaldurChitin->m_pEngineWorld->SetPendingRest(nHP, bRenting, FALSE);
            } else {
                g_pBaldurChitin->m_pEngineWorld->StartRestMultiplayerHost(nHP,
                    bRenting,
                    static_cast<BYTE>(m_bHealPartyOnRest));
            }

            return TRUE;
        }
    }

    return FALSE;
}

// 0x5C1350
BOOLEAN CInfGame::RestPartyPrivate(INT nHP, BOOLEAN bRenting)
{
    // NOTE: Original reads action ID from a constant at 0x847834.
    CAIAction action;
    action.m_actionID = 96;

    // NOTE: Original reads trigger ID from a constant.
    CAITrigger trigger(147, 0);

    g_pBaldurChitin->GetObjectGame()->sub_5CADF0();

    if (!g_pChitin->cNetwork.m_bConnectionEstablished || g_pChitin->cNetwork.m_bIsHost == TRUE) {
        g_pBaldurChitin->GetObjectGame()->GetVariables()->FindKey(CHAPTER_GLOBAL);

        g_pBaldurChitin->m_cBaldurMessage.SendRestAnnounce(nHP, bRenting, FALSE);

        BOOLEAN bFlag = TRUE;
        INT nTotal = 0;
        INT nRate = nHP;

        if (bRenting || !GetVisibleArea()->CheckRestEncounter()) {
            SHORT nCharacters = m_nCharacters;

            if (m_bHealPartyOnRest) {
                CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

                for (SHORT nPortrait = 0; nPortrait < nCharacters; nPortrait++) {
                    // NOTE: Uninline.
                    LONG nCharacterId = pGame->GetCharacterId(nPortrait);

                    CGameSprite* pSprite;

                    BYTE rc;
                    do {
                        rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        if ((pSprite->m_baseStats.m_generalState & STATE_DEAD) == 0) {
                            INT nDiff = pSprite->m_baseStats.m_maxHitPointsBase - pSprite->m_baseStats.m_hitPoints;
                            if (nDiff > 1) {
                                bFlag = FALSE;
                            }

                            nTotal += nDiff;

                            INT nLevel = pSprite->m_derivedStats.GetClassMaskLevel(0xC);
                            if (nLevel > 0) {
                                nRate += nLevel * 19 + 5;
                            }

                            nLevel = pSprite->m_derivedStats.GetClassMaskLevel(0xC0);
                            if (nLevel > 9) {
                                nRate += nLevel * 19 - 147;
                            }
                        }

                        pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
            } else {
                nTotal = nHP;
            }

            if (nRate > 0) {
                INT nCount = nTotal / nRate;
                if (nTotal % nRate != 0) {
                    nCount++;
                }
                nTotal = nCount * nRate;
            }

            if (m_bHealPartyOnRest) {
                if (nTotal > nRate) {
                    m_worldTime.AddCurrentTime(CRuleTables::TIME_RESTING * nTotal * 100 / (nRate * 100) + CRuleTables::TIME_RESTING);
                    FeedBack(FEEDBACK_TIMEPASSEDREST,
                        CRuleTables::TIME_RESTING * nTotal * 100 / (nRate * 100) + CRuleTables::TIME_RESTING,
                        TRUE);
                } else if (bFlag == TRUE) {
                    m_worldTime.AddCurrentTime(CRuleTables::TIME_RESTING);
                    FeedBack(FEEDBACK_TIMEPASSEDREST, CRuleTables::TIME_RESTING, TRUE);
                } else {
                    m_worldTime.AddCurrentTime(CRuleTables::TIME_RESTING * 2);
                    FeedBack(FEEDBACK_TIMEPASSEDREST, CRuleTables::TIME_RESTING * 2, TRUE);
                }
            } else {
                m_worldTime.AddCurrentTime(CRuleTables::TIME_RESTING);
                FeedBack(FEEDBACK_TIMEPASSEDREST, CRuleTables::TIME_RESTING, TRUE);
            }

            for (SHORT nPortrait = 0; nPortrait < nCharacters; nPortrait++) {
                // NOTE: Uninline.
                LONG nCharacterId = GetCharacterId(nPortrait);

                CGameEffectHeal* pEffect = new CGameEffectHeal();
                if (m_bHealPartyOnRest) {
                    pEffect->m_effectAmount = 1000;
                } else {
                    pEffect->m_effectAmount = nTotal;
                }

                CMessageAddEffect* pAddEffect = new CMessageAddEffect(pEffect,
                    nCharacterId,
                    nCharacterId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pAddEffect, FALSE);

                CMessageAddAction* pAddAction = new CMessageAddAction(action,
                    nCharacterId,
                    nCharacterId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pAddAction, FALSE);

                CMessageSetTrigger* pSetTrigger = new CMessageSetTrigger(trigger,
                    nCharacterId,
                    nCharacterId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(pSetTrigger, FALSE);
            }

            // NOTE: Original reads action ID from a constant at 0x847908
            // and duration from a constant at 0x847F84.
            CAIAction leaderAction;
            leaderAction = CAIAction(203, CPoint(35, 0), 0, -1);
            g_pBaldurChitin->GetObjectGame()->GetGroup()->GroupAction(CAIAction(CAIAction::NO_ACTION, CPoint(-1, -1), 0, -1),
                TRUE,
                &leaderAction);
        }
    }

    BOOLEAN bResult = FALSE;

    if (!bRenting) {
        CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 18368
        UTIL_ASSERT(pWorld != NULL);

        if (g_pBaldurChitin->GetActiveEngine() != pWorld) {
            g_pBaldurChitin->GetActiveEngine()->SelectEngine(pWorld);
        }
    } else {
        bResult = TRUE;
    }

    if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE && !bRenting) {
        g_pBaldurChitin->m_cBaldurMessage.SendSignal(CBaldurMessage::SIGNAL_SERVER,
            CBaldurMessage::SIGNAL_END_MAJOR_EVENT);
    }

    CScreenWorld* pWorld = g_pBaldurChitin->m_pEngineWorld;

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 18481
    UTIL_ASSERT(pWorld != NULL);

    if (g_pBaldurChitin->GetActiveEngine() != pWorld) {
        CCacheStatus::dword_8D0BA8 = TRUE;

        g_pBaldurChitin->GetActiveEngine()->SelectEngine(pWorld);

        if (bRenting
            && !g_pChitin->cNetwork.m_bIsHost
            && g_pChitin->cNetwork.m_nServiceProvider != CNetwork::SERV_PROV_NULL) {
            g_pBaldurChitin->m_cBaldurMessage.sub_436C00();
        }
    }

    g_pBaldurChitin->GetObjectGame()->m_cButtonArray.ResetState();

    return bResult;
}

// 0x5C1E30
BOOL CInfGame::CanRestParty(STRREF& strError, unsigned char a2, unsigned char a3, unsigned char a4)
{
    BOOL bResult = FALSE;
    BOOL bNoLivingCharacter = TRUE;
    BOOL bFlag28 = TRUE;
    BOOL bCannotRest = FALSE;
    BOOL bAreaRestricted = FALSE;
    CResRef areaResRef("");
    CPoint ptFirst;

    strError = 10309;

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters && !bCannotRest; nPortrait++) {
        // NOTE: Uninline.
        LONG nCharacterId = GetCharacterId(nPortrait);

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = m_cObjectArray.GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->m_pArea != NULL
                && (static_cast<BYTE>(pSprite->m_pArea->m_header.m_flags) & 0x2) != 0
                && !a2
                && !a4) {
                bAreaRestricted = TRUE;
            }

            if (pSprite->Animate()) {
                bNoLivingCharacter = FALSE;

                if (bFlag28) {
                    bFlag28 = (static_cast<BYTE>(pSprite->m_pArea->m_header.m_areaType) & 0x20) != 0;
                } else {
                    bFlag28 = FALSE;
                }

                CPoint pt = pSprite->GetPos();

                if (areaResRef == "") {
                    areaResRef = pSprite->m_pArea->m_resRef;
                    ptFirst = pt;
                } else {
                    if ((pSprite->m_baseStats.m_generalState & STATE_DEAD) != 0
                        || !(areaResRef != pSprite->m_pArea->m_resRef)) {
                        // NOTE: Original reads range from a constant at
                        // 0x85512C.
                        if ((ptFirst.x - pt.x) * (ptFirst.x - pt.x) + (ptFirst.y - pt.y) * (ptFirst.y - pt.y) * 16 / 9 > CScreenInventory::RANGE_SWAPITEM) {
                            bCannotRest = TRUE;
                        }
                    } else {
                        bCannotRest = TRUE;
                    }
                }
            }

            m_cObjectArray.ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        } else {
            bCannotRest = TRUE;
        }
    }

    if (bAreaRestricted == TRUE) {
        strError = 10309;
        return FALSE;
    }

    if (a3 == TRUE && !bCannotRest) {
        return TRUE;
    }

    if (!bNoLivingCharacter) {
        if (!CanSaveGame(strError, TRUE, FALSE)) {
            if (strError == 16501) {
                strError = 16500;
                bResult = FALSE;
            } else {
                strError = 10309;
                bResult = FALSE;
            }
        } else if (bCannotRest) {
            strError = 16457;
            bResult = FALSE;
        } else if (g_pBaldurChitin->GetObjectGame()->m_singlePlayerPermissions.m_bPermissions[CGamePermission::LEADER]) {
            strError = 15358;
            bResult = TRUE;
        } else {
            strError = 17198;
            bResult = FALSE;
        }
    }

    if (!m_cObjectArray.AnySpriteIsDiseased()) {
        bResult = FALSE;
        strError = 10309;
    }

    return bResult;
}

// 0x5C3010
CStringList* CInfGame::GetPortraits()
{
    CStringList* pList = new CStringList();

    CString sFileName;
    CString sPattern;
    CString sTemp;

    sPattern = GetDirPortraits() + "*.BMP";

    INT nCount = 0;
    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFile = FindFirstFileA(sPattern, &findFileData);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sFileName = findFileData.cFileName;
            if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                sFileName = sFileName.SpanExcluding(".");
                if (sFileName.GetLength() <= 8) {
                    if (nCount >= 32767) {
                        break;
                    }

                    POSITION pos = pList->GetHeadPosition();
                    while (pos != NULL) {
                        sTemp = pList->GetAt(pos);
                        if (sFileName < pList->GetAt(pos)) {
                            pList->InsertBefore(pos, sFileName);
                            nCount++;
                            break;
                        }

                        pList->GetNext(pos);
                    }

                    if (pos == NULL) {
                        pList->AddTail(sFileName);
                        nCount++;
                    }
                }
            }
        } while (FindNextFileA(hFindFile, &findFileData));
        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C3240
CStringList* CInfGame::GetImportCharacters()
{
    CStringList* pList = new CStringList();

    CString sFileName;
    CString sPattern;
    CString sTemp;

    sPattern = GetDirCharacters() + "*.chr";

    INT nCount = 0;
    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFile = FindFirstFileA(sPattern, &findFileData);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sFileName = findFileData.cFileName;
            if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                sFileName = sFileName.SpanExcluding(".");
                if (sFileName.GetLength() <= 8) {
                    if (nCount >= 32767) {
                        break;
                    }

                    FILE* stream = fopen(GetDirCharacters() + findFileData.cFileName, "rb");
                    if (stream != NULL) {
                        unsigned char data[8];
                        size_t bytesRead = fread(data, 1, sizeof(data), stream);
                        fclose(stream);

                        if (bytesRead == 8 && memcmp(data, "CHR V2.2", 8) == 0) {
                            sFileName.MakeUpper();

                            POSITION pos = pList->GetHeadPosition();
                            while (pos != NULL) {
                                sTemp = pList->GetAt(pos);
                                if (sFileName < pList->GetAt(pos)) {
                                    pList->InsertBefore(pos, sFileName);
                                    nCount++;
                                    break;
                                }

                                pList->GetNext(pos);
                            }

                            if (pos == NULL) {
                                pList->AddTail(sFileName);
                                nCount++;
                            }
                        }
                    }
                }
            }
        } while (FindNextFileA(hFindFile, &findFileData));
        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C3540
CStringList* CInfGame::GetSounds()
{
    CStringList* pList = new CStringList();

    CString sSoundSetDirName;
    CString sPattern;
    CString sTemp;

    sPattern = GetDirSounds() + "*";
    INT nCount = 0;

    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFile = FindFirstFileA(sPattern, &findFileData);
    if (hFindFile != INVALID_HANDLE_VALUE) {
        do {
            sSoundSetDirName = findFileData.cFileName;
            if (sSoundSetDirName != "."
                && sSoundSetDirName != ".."
                && (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                sSoundSetDirName.MakeUpper();

                INT nLength = sSoundSetDirName.GetLength();
                if (nLength > 0 && nLength < 32) {
                    if (nCount >= 32766) {
                        break;
                    }

                    POSITION pos = pList->GetHeadPosition();
                    while (pos != NULL) {
                        sTemp = pList->GetAt(pos);
                        if (sSoundSetDirName == sTemp) {
                            break;
                        }

                        if (sSoundSetDirName < pList->GetAt(pos)) {
                            pList->InsertBefore(pos, sSoundSetDirName);
                            nCount++;
                            break;
                        }

                        pList->GetNext(pos);
                    }

                    if (pos == NULL) {
                        pList->AddTail(sSoundSetDirName);
                        nCount++;
                    }
                }
            }
        } while (FindNextFileA(hFindFile, &findFileData));
        FindClose(hFindFile);
    }

    return pList;
}

// 0x5C3770
STRREF CInfGame::sub_5C3770(CString a1)
{
    return 9885;
}

// 0x5C3790
void CInfGame::ChangeBiography(BYTE nFixedPartyId, CString szBiography)
{
    STR_RES strRes;
    CString sString1;
    CString sString2;
    CGameSprite* pSprite;
    BYTE rc;

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE) {
        if (!g_pChitin->cNetwork.m_bIsHost) {
            if (!g_pBaldurChitin->m_cBaldurMessage.m_bInOnAnnounceBiography) {
                g_pBaldurChitin->GetBaldurMessage()->SendBiographyToServer(nFixedPartyId, szBiography);
                return;
            }
        } else {
            g_pBaldurChitin->GetBaldurMessage()->AnnounceBiography(nFixedPartyId, szBiography);
        }
    }

    CScreenCharacter* pEngineCharacter = g_pBaldurChitin->m_pEngineCharacter;

    // NOTE: Both strings are unused.
    sString1 = CBaldurEngine::FetchString(15882);
    sString2 = CBaldurEngine::FetchString(11863);

    // NOTE: Uninline.
    LONG nCharacterId = GetFixedOrderCharacterId(nFixedPartyId);

    if (nCharacterId == -1) {
        if (g_pBaldurChitin->pActiveEngine != g_pBaldurChitin->m_pEngineCreateChar) {
            return;
        }

        nCharacterId = g_pBaldurChitin->m_pEngineCreateChar->m_nGameSprite;
    }

    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc == CGameObjectArray::SUCCESS) {
        if (g_pBaldurChitin->GetTlkTable().m_override.Fetch(pSprite->m_baseStats.m_biography, strRes)) {
            g_pBaldurChitin->GetTlkTable().m_override.Remove(pSprite->m_baseStats.m_biography);
        }

        pSprite->m_baseStats.m_biography = nFixedPartyId + 0xF240;
        strRes.szText = szBiography;
        g_pBaldurChitin->GetTlkTable().m_override.AddUserEntry(pSprite->m_baseStats.m_biography, strRes);

        m_cObjectArray.ReleaseShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);
    }

    if (g_pChitin->cNetwork.GetSessionOpen() == TRUE
        && g_pBaldurChitin->pActiveEngine == pEngineCharacter
        && pEngineCharacter->GetTopPopup() != NULL
        && pEngineCharacter->GetTopPopup()->m_nID == 12) {
        pEngineCharacter->GetManager()->GetPanel(12)->GetControl(0);
    }
}

// 0x5C3B20
void CInfGame::sub_5C3B20(LONG nCharacterId, CPoint ptStart, LONG nFace, CString sArea, CString sParchment)
{
    CGameSprite* pSprite;
    BYTE rc;
    INT nMsgFrom;
    DWORD dwSize;
    BYTE* pData;
    CPoint ptDest;
    BOOLEAN bListenToJoin;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CBaldurMessage* pMessage = g_pBaldurChitin->GetBaldurMessage();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 19880
    UTIL_ASSERT(pGame != NULL && pMessage != NULL);

    // NOTE: Uninline.
    if (g_pBaldurChitin->GetObjectGame()->GetVisibleArea() != NULL) {
        g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->sub_47A190();
    }

    if (g_pChitin->cNetwork.m_bConnectionEstablished) {
        if (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineMap) {
            g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineWorld);
        }

        if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE
            && g_pChitin->cNetwork.m_bIsHost == TRUE
            && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineWorld) {
            g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineWorld);
        }

        g_pBaldurChitin->cSoundMixer.StartSong(-1, 3);
        g_pBaldurChitin->cSoundMixer.m_nActiveArea = -1;
        g_pBaldurChitin->cSoundMixer.UpdateSoundList();
        g_pBaldurChitin->m_pEngineWorld->m_weather.OnAreaChange(TRUE);

        if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
            g_pBaldurChitin->GetBaldurMessage()->sub_437D70(nCharacterId, ptStart, nFace, sArea, sParchment);
        }

        sub_59FA00(TRUE);

        if (strcmp(sParchment, "") != 0) {
            g_pBaldurChitin->GetObjectGame()->m_nTravelScreenImageToUse = 5;

            CResRef cResParchment(sParchment);
            g_pBaldurChitin->GetObjectGame()->m_nTravelScreenImageToUse = CInfGame::byte_8518BC;
            g_pBaldurChitin->GetObjectGame()->m_cResParchment = cResParchment;
        } else {
            g_pBaldurChitin->GetObjectGame()->m_nTravelScreenImageToUse = rand() % 5;
        }

        STRREF strParchment = g_pBaldurChitin->GetObjectGame()->sub_5C3770(sParchment);
        field_1B86 = sArea;

        g_pChitin->SetProgressBar(TRUE, 0x26A3, 0, 0, TRUE, strParchment, FALSE, 0, FALSE, FALSE, 0xFF);

        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = 6000000;
        g_pChitin->cProgressBar.m_bDisableMinibars = FALSE;

        if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE) {
            bListenToJoin = pGame->m_multiplayerSettings.m_bJoinRequests;
            pGame->m_multiplayerSettings.SetListenToJoinOption(FALSE, FALSE);
        }

        if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE) {
            SHORT nPlayers = 0;
            for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
                if (g_pChitin->cNetwork.GetPlayerID(nPlayer) != 0) {
                    nPlayers++;
                }
            }

            if (g_pChitin->cNetwork.m_nServiceProvider != CNetwork::SERV_PROV_NULL) {
                g_pChitin->cProgressBar.m_nWaitingReason = 0x4D03;
                g_pChitin->cProgressBar.m_bWaiting = TRUE;
            }

            SHORT nTimeout = 40;
            SHORT nResponses = 0;
            while (nResponses < nPlayers - 1) {
                pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_RESOURCE,
                    CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND,
                    nMsgFrom,
                    dwSize);
                if (pData != NULL) {
                    pMessage->OnResourceDemanded(nMsgFrom, pData, dwSize);
                    delete pData;
                    nResponses++;
                }

                g_pChitin->m_bDisplayStale = TRUE;
                SleepEx(60, FALSE);

                nTimeout--;

                nPlayers = 0;
                for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
                    if (g_pChitin->cNetwork.GetPlayerID(nPlayer) != 0) {
                        nPlayers++;
                    }
                }

                if (nTimeout <= 0) {
                    break;
                }
            }

            if (nPlayers > 1 && nTimeout > 0) {
                for (INT nRemaining = nTimeout; nRemaining > 0; nRemaining--) {
                    g_pChitin->m_bDisplayStale = TRUE;
                    SleepEx(60, FALSE);
                }
            }

            g_pChitin->cProgressBar.m_bWaiting = FALSE;

            g_pBaldurChitin->m_cCachingStatus.InvalidateScreen();
        }

        CGameArea* pArea;
        if (strcmp(sParchment, "") != 0) {
            CResRef cResParchment(sParchment);
            g_pBaldurChitin->GetObjectGame()->m_nTravelScreenImageToUse = CInfGame::byte_8518BC;
            g_pBaldurChitin->GetObjectGame()->m_cResParchment = cResParchment;

            pArea = g_pBaldurChitin->GetObjectGame()->LoadArea(sArea, 5, FALSE, TRUE);
        } else {
            pArea = g_pBaldurChitin->GetObjectGame()->LoadArea(sArea, 0xFF, FALSE, TRUE);
        }

        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED);

        if (pArea == NULL) {
            // NOTE: The object was denied above and is never released on this path.
            g_pChitin->SetProgressBar(FALSE, 0, 0, 0, FALSE, 0, FALSE, 0, FALSE, FALSE, 0xFF);
            return;
        }

        if (rc == CGameObjectArray::SUCCESS) {
            if (g_pChitin->cNetwork.m_nServiceProvider == CNetwork::SERV_PROV_NULL
                || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
                Icewind586B70::Instance()->sub_5872C0(pSprite);

                ptStart.x = ptStart.x / CPathSearch::GRID_SQUARE_SIZEX;
                ptStart.y = ptStart.y / CPathSearch::GRID_SQUARE_SIZEY;

                pSprite->m_clearAIOnRemoveFromArea = FALSE;

                CGameArea* pOldArea = pSprite->GetArea();

                CResHelper<CResArea, 1010> cResArea;

                if (pOldArea != NULL) {
                    if ((!g_pChitin->cNetwork.m_bConnectionEstablished || g_pChitin->cNetwork.m_bIsHost == TRUE)
                        && (pOldArea->m_header.m_areaType & 0x1) == 0
                        && pOldArea->field_436 == 0) {
                        BYTE* pBuffer;
                        LONG nBufferSize;
                        pOldArea->Marshal(&pBuffer, &nBufferSize, TRUE);

                        if (pBuffer != NULL) {
                            cResArea.SetResRef(pOldArea->m_resRef, TRUE, TRUE);

                            cResArea.pRes->Write(g_pBaldurChitin->GetObjectGame()->m_sTempDir,
                                pBuffer,
                                nBufferSize);

                            pOldArea->field_436 = 1;
                            pOldArea->field_438 = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime;

                            delete pBuffer;
                        }
                    }
                }

                g_pBaldurChitin->GetObjectGame()->m_bInAreaTransition = TRUE;

                if (pSprite->GetArea() != NULL) {
                    pSprite->RemoveFromArea();
                    pSprite->m_clearAIOnRemoveFromArea = TRUE;

                    BYTE nPersonalSpace = pSprite->m_animation.GetPersonalSpace();

                    CPoint ptOpen = pArea->m_search.GetNearestOpenSquare(ptStart,
                        pSprite->GetTerrainTable(),
                        nPersonalSpace,
                        static_cast<SHORT>(nFace));

                    ptDest.x = CPathSearch::GRID_SQUARE_SIZEX * ptOpen.x + CPathSearch::GRID_SQUARE_SIZEX / 2;
                    ptDest.y = CPathSearch::GRID_SQUARE_SIZEY * ptOpen.y + CPathSearch::GRID_SQUARE_SIZEY / 2;

                    pSprite->AddToArea(pArea, ptDest, 0, CGameObject::LIST_FRONT);
                }

                g_pBaldurChitin->GetObjectGame()->m_bInAreaTransition = FALSE;

                pSprite->SetFacing(static_cast<SHORT>(nFace));

                if (g_pBaldurChitin->GetObjectGame()->GetVisibleArea() != pSprite->GetArea()
                    && pSprite->GetArea() != NULL) {
                    CRect rNewViewSize = g_pBaldurChitin->m_pEngineWorld->GetNewViewSize();

                    if (!EqualRect(rNewViewSize, pSprite->GetArea()->GetInfinity()->rViewPort)) {
                        g_pBaldurChitin->m_pEngineWorld->SetNewViewSize(rNewViewSize, FALSE);
                    }

                    pArea->GetInfinity()->SetViewPosition(ptDest.x - (rNewViewSize.right - rNewViewSize.left) / 2,
                        ptDest.y - (rNewViewSize.bottom - rNewViewSize.top) / 2,
                        TRUE);

                    if (g_pChitin->cNetwork.m_nServiceProvider == CNetwork::SERV_PROV_NULL
                        || g_pChitin->cNetwork.m_idLocalPlayer == pSprite->m_remotePlayerID) {
                        g_pBaldurChitin->GetObjectGame()->UnselectAll();
                    }
                } else if (pOldArea == pArea) {
                    const CRect& rViewPort = pArea->GetInfinity()->rViewPort;

                    pArea->GetInfinity()->SetViewPosition(ptDest.x - (rViewPort.right - rViewPort.left) / 2,
                        ptDest.y - (rViewPort.bottom - rViewPort.top) / 2,
                        TRUE);
                }

                g_pBaldurChitin->GetObjectGame()->SelectCharacter(nCharacterId, FALSE);
                g_pBaldurChitin->GetObjectGame()->SelectToolbar();

                if (pSprite->GetPos().x != -1
                    || pSprite->GetPos().y != -1
                    || pSprite->GetArea() != NULL) {
                    CMessage* message = new CMessageSpriteUpdate(pSprite, nCharacterId, nCharacterId);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            }
        }

        g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget - 1;
        g_pChitin->m_bDisplayStale = TRUE;
        sub_59FA00(TRUE);
        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;

        if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE) {
            g_pBaldurChitin->GetBaldurMessage()->SendProgressBarStatus(g_pChitin->cProgressBar.m_nActionProgress,
                g_pChitin->cProgressBar.m_nActionTarget,
                g_pChitin->cProgressBar.m_bWaiting,
                g_pChitin->cProgressBar.m_nWaitingReason,
                g_pChitin->cProgressBar.m_bTimeoutVisible,
                g_pChitin->cProgressBar.m_nSecondsToTimeout);
        }

        if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE) {
            g_pChitin->cProgressBar.m_bTimeoutVisible = TRUE;
            g_pChitin->cProgressBar.m_nSecondsToTimeout = g_pBaldurChitin->m_cBaldurMessage.m_dwSignalSecondsToTimeout;

            DWORD dwStartTime = GetTickCount();

            // NOTE: 0x84CFBF is the unnamed signal '3'.
            if (!pMessage->NonBlockingWaitForSignal(CBaldurMessage::SIGNAL_SERVER, 51)) {
                do {
                    DWORD nSecondsToTimeout;
                    DWORD dwElapsed = GetTickCount() - dwStartTime;
                    if (dwElapsed >= g_pBaldurChitin->m_cBaldurMessage.m_dwSignalSecondsToTimeout * 1000) {
                        pMessage->KickOutWaitingForSignal(CBaldurMessage::SIGNAL_SERVER, 51);
                        nSecondsToTimeout = 0;
                    } else {
                        nSecondsToTimeout = g_pBaldurChitin->m_cBaldurMessage.m_dwSignalSecondsToTimeout - dwElapsed / 1000;
                    }

                    if (g_pChitin->cProgressBar.m_nSecondsToTimeout != nSecondsToTimeout) {
                        g_pChitin->cProgressBar.m_nSecondsToTimeout = nSecondsToTimeout;

                        g_pBaldurChitin->GetBaldurMessage()->SendProgressBarStatus(g_pChitin->cProgressBar.m_nActionProgress,
                            g_pChitin->cProgressBar.m_nActionTarget,
                            g_pChitin->cProgressBar.m_bWaiting,
                            g_pChitin->cProgressBar.m_nWaitingReason,
                            g_pChitin->cProgressBar.m_bTimeoutVisible,
                            g_pChitin->cProgressBar.m_nSecondsToTimeout);
                    }

                    while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_RESOURCE,
                                CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND,
                                nMsgFrom,
                                dwSize))
                        != NULL) {
                        pMessage->OnResourceDemanded(nMsgFrom, pData, dwSize);
                        delete pData;
                    }

                    while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_PROGRESSBAR,
                                CBaldurMessage::MSG_SUBTYPE_PROGRESSBAR_STATUS,
                                nMsgFrom,
                                dwSize))
                        != NULL) {
                        pMessage->OnProgressBarStatus(nMsgFrom, pData, dwSize);
                        delete pData;
                    }

                    g_pChitin->m_bDisplayStale = TRUE;
                    SleepEx(60, FALSE);
                } while (!pMessage->NonBlockingWaitForSignal(CBaldurMessage::SIGNAL_SERVER, 51));
            }

            // NOTE: 0x84CFC0 is the unnamed signal '4'.
            pMessage->SendSignal(CBaldurMessage::SIGNAL_ALL_CLIENTS, 52);

            pGame->m_multiplayerSettings.SetListenToJoinOption(bListenToJoin, FALSE);
        } else if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE) {
            pMessage->SendSignal(CBaldurMessage::SIGNAL_SERVER, 51);

            if (!pMessage->NonBlockingWaitForSignal(CBaldurMessage::SIGNAL_ALL_CLIENTS, 52)) {
                do {
                    while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_PROGRESSBAR,
                                CBaldurMessage::MSG_SUBTYPE_PROGRESSBAR_STATUS,
                                nMsgFrom,
                                dwSize))
                        != NULL) {
                        pMessage->OnProgressBarStatus(nMsgFrom, pData, dwSize);
                        delete pData;
                    }

                    g_pChitin->m_bDisplayStale = TRUE;
                    SleepEx(60, FALSE);
                } while (!pMessage->NonBlockingWaitForSignal(CBaldurMessage::SIGNAL_ALL_CLIENTS, 52));
            }
        }

        g_pChitin->cProgressBar.m_bTimeoutVisible = FALSE;
        g_pChitin->cProgressBar.m_nActionProgress += g_pChitin->cProgressBar.m_nActionTarget - g_pChitin->cProgressBar.m_nActionProgress;
        g_pChitin->m_bDisplayStale = TRUE;
        sub_59FA00(TRUE);

        g_pChitin->SetProgressBar(FALSE, 0, 0, 0, FALSE, 0, FALSE, 0, FALSE, FALSE, 0xFF);

        g_pBaldurChitin->cSoundMixer.m_nActiveArea = reinterpret_cast<DWORD_PTR>(g_pBaldurChitin->GetObjectGame()->GetVisibleArea());
        g_pBaldurChitin->cSoundMixer.UpdateSoundList();
        g_pBaldurChitin->m_pEngineWorld->m_weather.OnAreaChange(FALSE);
    }
}

// 0x5C4AD0
void CInfGame::sub_5C4AD0(LONG nCharacterId, LONG nTriggerId, CString sArea)
{
    CGameSprite* pCharacter;
    CGameTrigger* pTrigger;
    BYTE rc;
    INT nMsgFrom;
    DWORD dwSize;
    BYTE* pData;
    BOOLEAN bListenToJoin;

    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();
    CBaldurMessage* pMessage = g_pBaldurChitin->GetBaldurMessage();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 20366
    UTIL_ASSERT(pGame != NULL && pMessage != NULL);

    // NOTE: Uninline.
    if (g_pBaldurChitin->GetObjectGame()->GetVisibleArea() != NULL) {
        g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->sub_47A190();
    }

    if (g_pChitin->cNetwork.m_bConnectionEstablished) {
        if (g_pBaldurChitin->GetActiveEngine() == g_pBaldurChitin->m_pEngineMap) {
            g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineWorld);
        }

        if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE
            && g_pChitin->cNetwork.m_bIsHost == TRUE
            && g_pBaldurChitin->GetActiveEngine() != g_pBaldurChitin->m_pEngineWorld) {
            g_pBaldurChitin->GetActiveEngine()->SelectEngine(g_pBaldurChitin->m_pEngineWorld);
        }

        g_pBaldurChitin->cSoundMixer.m_nActiveArea = -1;
        g_pBaldurChitin->cSoundMixer.UpdateSoundList();
        g_pBaldurChitin->m_pEngineWorld->m_weather.OnAreaChange(TRUE);

        if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
            pGame->m_sSaveGame = CInfGame::AUTO_SAVE_NAME;
            CScreenCharacter::SAVE_NAME = CInfGame::AUTO_SAVE_NAME;

            if (g_pChitin->cNetwork.m_nServiceProvider == CNetwork::SERV_PROV_NULL) {
                pGame->field_366E = 1;
                pGame->field_50DC = 0;

                if (!g_pChitin->cVideo.m_bIs3dAccelerated) {
                    if (!pGame->field_50D8) {
                        pGame->SynchronousUpdate();
                    }
                    pGame->field_50D8 = TRUE;
                }
            }

            pGame->SaveGame(TRUE, FALSE, TRUE);

            if (g_pChitin->cNetwork.m_nServiceProvider == CNetwork::SERV_PROV_NULL
                && g_pChitin->cVideo.m_bIs3dAccelerated) {
                pGame->field_366E = 1;
                pGame->field_50DC = 1;

                // NOTE: Never true here.
                if (!g_pChitin->cVideo.m_bIs3dAccelerated) {
                    if (!pGame->field_50D8) {
                        pGame->SynchronousUpdate();
                    }
                    pGame->field_50D8 = TRUE;
                }
            }

            g_pBaldurChitin->GetBaldurMessage()->sub_438F90(nCharacterId, nTriggerId, sArea);
        }

        BOOLEAN bSaved = FALSE;

        LONG nProgressTarget = 6000000;
        if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
            nProgressTarget = 11000000;
        }

        sub_59FA00(TRUE);

        g_pBaldurChitin->GetObjectGame()->m_nTravelScreenImageToUse = rand() % 5;
        field_1B86 = sArea;

        g_pChitin->SetProgressBar(TRUE, 0x26A3, 0, 0, TRUE, 0x269D, FALSE, 0, FALSE, FALSE, 0xFF);

        g_pChitin->cProgressBar.m_nActionProgress = 0;
        g_pChitin->cProgressBar.m_nActionTarget = nProgressTarget;
        g_pChitin->cProgressBar.m_bDisableMinibars = FALSE;

        if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE) {
            bListenToJoin = pGame->m_multiplayerSettings.m_bJoinRequests;
            pGame->m_multiplayerSettings.SetListenToJoinOption(FALSE, FALSE);
        }

        rc = pGame->GetObjectArray()->GetShare(nTriggerId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pTrigger),
            INFINITE);
        if (rc != CGameObjectArray::SUCCESS) {
            return;
        }

        CString sTriggerArea;

        // FIXME: `m_newArea` is not null-terminated when the area name takes all
        // 8 characters.
        sTriggerArea = reinterpret_cast<const char*>(pTrigger->m_newArea);

        CString sEntryPoint(pTrigger->m_newEntryPoint);

        pGame->GetObjectArray()->ReleaseShare(nTriggerId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        if (g_pChitin->cNetwork.m_bIsHost == TRUE) {
            SHORT nPlayers = 0;
            for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
                if (g_pChitin->cNetwork.GetPlayerID(nPlayer) != 0) {
                    nPlayers++;
                }
            }

            if (g_pChitin->cNetwork.m_nServiceProvider != CNetwork::SERV_PROV_NULL) {
                g_pChitin->cProgressBar.m_nWaitingReason = 0x4D03;
                g_pChitin->cProgressBar.m_bWaiting = TRUE;
            }

            SHORT nTimeout = 40;
            SHORT nResponses = 0;
            while (nResponses < nPlayers - 1) {
                pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_RESOURCE,
                    CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND,
                    nMsgFrom,
                    dwSize);
                if (pData != NULL) {
                    pMessage->OnResourceDemanded(nMsgFrom, pData, dwSize);
                    delete pData;
                    nResponses++;
                }

                g_pChitin->m_bDisplayStale = TRUE;
                SleepEx(60, FALSE);

                nTimeout--;

                nPlayers = 0;
                for (INT nPlayer = 0; nPlayer < CNETWORK_MAX_PLAYERS; nPlayer++) {
                    if (g_pChitin->cNetwork.GetPlayerID(nPlayer) != 0) {
                        nPlayers++;
                    }
                }

                if (nTimeout <= 0) {
                    break;
                }
            }

            if (nPlayers > 1 && nTimeout > 0) {
                for (INT nRemaining = nTimeout; nRemaining > 0; nRemaining--) {
                    g_pChitin->m_bDisplayStale = TRUE;
                    SleepEx(60, FALSE);
                }
            }

            g_pChitin->cProgressBar.m_bWaiting = FALSE;

            g_pBaldurChitin->m_cCachingStatus.InvalidateScreen();
        }

        CGameArea* pArea = pGame->LoadArea(sTriggerArea, 0xFF, FALSE, TRUE);

        do {
            rc = pGame->GetObjectArray()->GetDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pCharacter),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED);

        if (rc != CGameObjectArray::SUCCESS) {
            // NOTE: The object was denied above and is never released on this path.
            g_pChitin->SetProgressBar(FALSE, 0, 0, 0, FALSE, 0, FALSE, 0, FALSE, FALSE, 0xFF);
            return;
        }

        Icewind586B70::Instance()->sub_5872C0(pCharacter);

        if (pArea == NULL) {
            g_pChitin->SetProgressBar(FALSE, 0, 0, 0, FALSE, 0, FALSE, 0, FALSE, FALSE, 0xFF);

            pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return;
        }

        CPoint ptEnter;
        SHORT nFacing;
        if (!pArea->GetEntryPoint(sEntryPoint, ptEnter, nFacing)) {
            g_pChitin->SetProgressBar(FALSE, 0, 0, 0, FALSE, 0, FALSE, 0, FALSE, FALSE, 0xFF);

            pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
            return;
        }

        ptEnter.x /= CPathSearch::GRID_SQUARE_SIZEX;
        ptEnter.y /= CPathSearch::GRID_SQUARE_SIZEY;

        SHORT nFormation;
        if (g_pChitin->cNetwork.m_bConnectionEstablished) {
            nFormation = CAIGroup::FORMATION_2BY3;
        } else {
            nFormation = pGame->m_gameSave.m_curFormation;
        }

        CPoint* pOffsets = pGame->m_group.GetOffsets(nFormation,
            static_cast<SHORT>(((nFacing + 8) * 360 / 16) % 360),
            TRUE);

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 20610
        UTIL_ASSERT(pOffsets != NULL);

        SHORT nOffset = 0;

        SHORT nCount;
        if (g_pChitin->cNetwork.m_nServiceProvider != CNetwork::SERV_PROV_NULL) {
            nCount = 6;
        } else {
            nCount = m_nCharacters;
        }

        for (SHORT nSlot = 0; nSlot < nCount; nSlot++) {
            LONG nId;
            if (g_pChitin->cNetwork.m_nServiceProvider != CNetwork::SERV_PROV_NULL) {
                nId = nSlot < 6
                    ? pGame->m_characters[nSlot]
                    : CGameObjectArray::INVALID_INDEX;
                if (nId == CGameObjectArray::INVALID_INDEX) {
                    continue;
                }
            } else {
                // NOTE: Uninline.
                nId = pGame->GetCharacterId(nSlot);
            }

            CGameSprite* pSprite;
            do {
                rc = pGame->GetObjectArray()->GetDeny(nId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED);

            if (rc != CGameObjectArray::SUCCESS) {
                g_pBaldurChitin->GetObjectGame()->SelectToolbar();
                delete[] pOffsets;

                pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return;
            }

            // NOTE: Uninline.
            if (pSprite->InControl()) {
                pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;
                pSprite->ClearActions(FALSE);
                pSprite->m_targetPoint.x = -1;
                pSprite->m_targetPoint.y = -1;

                CGameArea* pOldArea = pSprite->m_pArea;

                CResHelper<CResArea, 1010> cResArea;

                if (pOldArea != NULL
                    && (!g_pChitin->cNetwork.m_bConnectionEstablished || g_pChitin->cNetwork.m_bIsHost == TRUE)
                    && !bSaved
                    && (pOldArea->m_header.m_areaType & 0x1) == 0
                    && pOldArea->field_436 == 0) {
                    BYTE* pBuffer;
                    LONG nBufferSize;
                    pOldArea->Marshal(&pBuffer, &nBufferSize, TRUE);

                    if (pBuffer != NULL) {
                        cResArea.SetResRef(pOldArea->m_resRef, TRUE, TRUE);

                        cResArea.pRes->Write(g_pBaldurChitin->GetObjectGame()->m_sTempDir,
                            pBuffer,
                            nBufferSize);

                        pOldArea->field_436 = 1;
                        pOldArea->field_438 = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime;

                        delete pBuffer;

                        bSaved = TRUE;
                    }
                }

                g_pBaldurChitin->GetObjectGame()->m_bInAreaTransition = TRUE;

                if (pSprite->m_pArea != NULL) {
                    pSprite->RemoveFromArea();

                    CPoint* pOffset;
                    if (g_pChitin->cNetwork.m_bConnectionEstablished) {
                        pOffset = &(pOffsets[nSlot]);
                    } else {
                        pOffset = &(pOffsets[nOffset]);
                    }

                    LONG nOffsetX = pOffset->x / CAIGroup::OFFSET_MULTIPLIER;
                    LONG nOffsetY = pOffset->y / CAIGroup::OFFSET_MULTIPLIER;
                    nOffset++;

                    BYTE nPersonalSpace = pSprite->m_animation.GetPersonalSpace();

                    CPoint ptOpen = pArea->m_search.GetNearestOpenSquare(CPoint(ptEnter.x + nOffsetX, ptEnter.y + nOffsetY),
                        pSprite->GetTerrainTable(),
                        nPersonalSpace,
                        -1);

                    CPoint ptDest;
                    ptDest.x = CPathSearch::GRID_SQUARE_SIZEX * ptOpen.x + CPathSearch::GRID_SQUARE_SIZEX / 2;
                    ptDest.y = CPathSearch::GRID_SQUARE_SIZEY * ptOpen.y + CPathSearch::GRID_SQUARE_SIZEY / 2;

                    // NOTE: `Animate` is the function at 0x728560 which tests
                    // the "not dead" state bit.
                    if (!pSprite->Animate() && pSprite->m_animation.CanLieDown()) {
                        // NOTE: Dead characters are neither turned nor
                        // followed by the viewport.
                        pSprite->AddToArea(pArea, ptDest, 0, CGameObject::LIST_BACK);
                    } else {
                        pSprite->AddToArea(pArea, ptDest, 0, CGameObject::LIST_FRONT);
                        pSprite->SetFacing(nFacing);

                        if (pGame->GetVisibleArea() != pArea) {
                            CRect rNewViewSize = g_pBaldurChitin->m_pEngineWorld->GetNewViewSize();

                            if (!EqualRect(rNewViewSize, pArea->GetInfinity()->rViewPort)) {
                                g_pBaldurChitin->m_pEngineWorld->SetNewViewSize(rNewViewSize, FALSE);
                            }

                            pArea->GetInfinity()->SetViewPosition(ptDest.x - (rNewViewSize.right - rNewViewSize.left) / 2,
                                ptDest.y - (rNewViewSize.bottom - rNewViewSize.top) / 2,
                                TRUE);

                            if (pOldArea != pArea) {
                                pGame->UnselectAll();
                                pGame->SelectCharacter(pSprite->m_id, FALSE);
                            }
                        } else if (pOldArea == pArea) {
                            const CRect& rViewPort = pArea->GetInfinity()->rViewPort;

                            pArea->GetInfinity()->SetViewPosition(ptDest.x - (rViewPort.right - rViewPort.left) / 2,
                                ptDest.y - (rViewPort.bottom - rViewPort.top) / 2,
                                TRUE);
                        } else {
                            pGame->SelectCharacter(pSprite->m_id, FALSE);
                        }
                    }
                }

                g_pBaldurChitin->GetObjectGame()->m_bInAreaTransition = FALSE;

                if (pSprite->GetPos().x != -1
                    || pSprite->GetPos().y != -1
                    || pSprite->m_pArea != NULL) {
                    CMessage* message = new CMessageSpriteUpdate(pSprite, nId, nId);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            }

            pGame->GetObjectArray()->ReleaseDeny(nId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        POSITION posFamiliar = pGame->m_familiars.GetHeadPosition();
        while (posFamiliar != NULL) {
            LONG nId = static_cast<LONG>(reinterpret_cast<INT_PTR>(pGame->m_familiars.GetNext(posFamiliar)));

            CGameSprite* pSprite;
            do {
                rc = pGame->GetObjectArray()->GetDeny(nId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED);

            if (rc != CGameObjectArray::SUCCESS) {
                g_pBaldurChitin->GetObjectGame()->SelectToolbar();
                delete[] pOffsets;

                pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    INFINITE);
                return;
            }

            // NOTE: Uninline.
            if (pSprite->InControl()) {
                pSprite->m_triggerId = CGameObjectArray::INVALID_INDEX;
                pSprite->ClearActions(FALSE);
                pSprite->m_targetPoint.x = -1;
                pSprite->m_targetPoint.y = -1;

                CGameArea* pOldArea = pSprite->m_pArea;

                CResHelper<CResArea, 1010> cResArea;

                if (pOldArea != NULL
                    && (!g_pChitin->cNetwork.m_bConnectionEstablished || g_pChitin->cNetwork.m_bIsHost == TRUE)
                    && !bSaved
                    && (pOldArea->m_header.m_areaType & 0x1) == 0
                    && pOldArea->field_436 == 0) {
                    BYTE* pBuffer;
                    LONG nBufferSize;
                    pOldArea->Marshal(&pBuffer, &nBufferSize, TRUE);

                    if (pBuffer != NULL) {
                        cResArea.SetResRef(pOldArea->m_resRef, TRUE, TRUE);

                        cResArea.pRes->Write(g_pBaldurChitin->GetObjectGame()->m_sTempDir,
                            pBuffer,
                            nBufferSize);

                        pOldArea->field_436 = 1;
                        pOldArea->field_438 = g_pBaldurChitin->GetObjectGame()->m_worldTime.m_gameTime;

                        delete pBuffer;

                        bSaved = TRUE;
                    }
                }

                g_pBaldurChitin->GetObjectGame()->m_bInAreaTransition = TRUE;

                if (pSprite->m_pArea != NULL) {
                    pSprite->RemoveFromArea();

                    // NOTE: The connection flag is read here, but the exe
                    // selects the first offset either way.
                    CPoint* pOffset = &(pOffsets[0]);

                    LONG nOffsetX = pOffset->x / CAIGroup::OFFSET_MULTIPLIER;
                    LONG nOffsetY = pOffset->y / CAIGroup::OFFSET_MULTIPLIER;

                    BYTE nPersonalSpace = pSprite->m_animation.GetPersonalSpace();

                    CPoint ptOpen = pArea->m_search.GetNearestOpenSquare(CPoint(ptEnter.x + nOffsetX, ptEnter.y + nOffsetY),
                        pSprite->GetTerrainTable(),
                        nPersonalSpace,
                        -1);

                    CPoint ptDest;
                    ptDest.x = CPathSearch::GRID_SQUARE_SIZEX * ptOpen.x + CPathSearch::GRID_SQUARE_SIZEX / 2;
                    ptDest.y = CPathSearch::GRID_SQUARE_SIZEY * ptOpen.y + CPathSearch::GRID_SQUARE_SIZEY / 2;

                    // NOTE: `Animate` is the function at 0x728560 which tests
                    // the "not dead" state bit.
                    if (!pSprite->Animate() && pSprite->m_animation.CanLieDown()) {
                        // NOTE: Dead characters are neither turned nor
                        // followed by the viewport.
                        pSprite->AddToArea(pArea, ptDest, 0, CGameObject::LIST_BACK);
                    } else {
                        pSprite->AddToArea(pArea, ptDest, 0, CGameObject::LIST_FRONT);
                        pSprite->SetFacing(nFacing);

                        if (pGame->GetVisibleArea() != pArea) {
                            CRect rNewViewSize = g_pBaldurChitin->m_pEngineWorld->GetNewViewSize();

                            if (!EqualRect(rNewViewSize, pArea->GetInfinity()->rViewPort)) {
                                g_pBaldurChitin->m_pEngineWorld->SetNewViewSize(rNewViewSize, FALSE);
                            }

                            pArea->GetInfinity()->SetViewPosition(ptDest.x - (rNewViewSize.right - rNewViewSize.left) / 2,
                                ptDest.y - (rNewViewSize.bottom - rNewViewSize.top) / 2,
                                TRUE);

                            if (pOldArea != pArea) {
                                pGame->UnselectAll();
                                pGame->SelectCharacter(pSprite->m_id, FALSE);
                            }
                        } else if (pOldArea == pArea) {
                            const CRect& rViewPort = pArea->GetInfinity()->rViewPort;

                            pArea->GetInfinity()->SetViewPosition(ptDest.x - (rViewPort.right - rViewPort.left) / 2,
                                ptDest.y - (rViewPort.bottom - rViewPort.top) / 2,
                                TRUE);
                        } else {
                            pGame->SelectCharacter(pSprite->m_id, FALSE);
                        }
                    }
                }

                g_pBaldurChitin->GetObjectGame()->m_bInAreaTransition = FALSE;

                if (pSprite->GetPos().x != -1
                    || pSprite->GetPos().y != -1
                    || pSprite->m_pArea != NULL) {
                    CMessage* message = new CMessageSpriteUpdate(pSprite, nId, nId);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }
            }

            pGame->GetObjectArray()->ReleaseDeny(nId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }

        delete[] pOffsets;

        g_pBaldurChitin->GetObjectGame()->SelectToolbar();

        // NOTE: Uninline.
        if (pCharacter->InControl()) {
            if (pCharacter->GetPos().x != -1
                || pCharacter->GetPos().y != -1
                || pCharacter->m_pArea != NULL) {
                CMessage* message = new CMessageSpriteUpdate(pCharacter, nCharacterId, nCharacterId);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        pGame->GetObjectArray()->ReleaseDeny(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            INFINITE);

        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget - 1;
        g_pChitin->m_bDisplayStale = TRUE;
        sub_59FA00(TRUE);
        g_pChitin->cProgressBar.m_nActionProgress = g_pChitin->cProgressBar.m_nActionTarget;

        if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE) {
            g_pBaldurChitin->GetBaldurMessage()->SendProgressBarStatus(g_pChitin->cProgressBar.m_nActionProgress,
                g_pChitin->cProgressBar.m_nActionTarget,
                g_pChitin->cProgressBar.m_bWaiting,
                g_pChitin->cProgressBar.m_nWaitingReason,
                g_pChitin->cProgressBar.m_bTimeoutVisible,
                g_pChitin->cProgressBar.m_nSecondsToTimeout);
        }

        ptEnter.x *= CPathSearch::GRID_SQUARE_SIZEX;
        ptEnter.y *= CPathSearch::GRID_SQUARE_SIZEY;

        if (g_pBaldurChitin->cNetwork.m_bIsHost == TRUE) {
            g_pChitin->cProgressBar.m_bTimeoutVisible = TRUE;
            g_pChitin->cProgressBar.m_nSecondsToTimeout = g_pBaldurChitin->m_cBaldurMessage.m_dwSignalSecondsToTimeout;

            DWORD dwStartTime = GetTickCount();

            // NOTE: 0x84CFBF is the unnamed signal '3'.
            if (!pMessage->NonBlockingWaitForSignal(CBaldurMessage::SIGNAL_SERVER, 51)) {
                do {
                    DWORD nSecondsToTimeout;
                    DWORD dwElapsed = GetTickCount() - dwStartTime;
                    if (dwElapsed >= g_pBaldurChitin->m_cBaldurMessage.m_dwSignalSecondsToTimeout * 1000) {
                        pMessage->KickOutWaitingForSignal(CBaldurMessage::SIGNAL_SERVER, 51);
                        nSecondsToTimeout = 0;
                    } else {
                        nSecondsToTimeout = g_pBaldurChitin->m_cBaldurMessage.m_dwSignalSecondsToTimeout - dwElapsed / 1000;
                    }

                    if (nSecondsToTimeout <= 10) {
                        if (g_pChitin->cProgressBar.m_nSecondsToTimeout > 10) {
                            for (BYTE nSlot = 0; nSlot < CMultiplayerSettings::MAX_CHARACTERS; nSlot++) {
                                LONG nId;
                                if (nSlot < 6) {
                                    nId = pGame->m_characters[nSlot];
                                } else {
                                    nId = CGameObjectArray::INVALID_INDEX;
                                }

                                INT nPlayerSlot = pGame->m_multiplayerSettings.GetCharacterControlledByPlayer(nSlot);

                                CGameSprite* pSprite;
                                do {
                                    rc = pGame->GetObjectArray()->GetShare(nId,
                                        CGameObjectArray::THREAD_ASYNCH,
                                        reinterpret_cast<CGameObject**>(&pSprite),
                                        INFINITE);
                                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                                if (rc == CGameObjectArray::SUCCESS) {
                                    // NOTE: `Animate` is the function at 0x728560 which tests
                                    // the "not dead" state bit.
                                    if (pSprite->Animate()
                                        && pSprite->m_pArea != NULL
                                        && pSprite->m_pArea != pArea
                                        // NOTE: The local player is compared as a byte.
                                        && nPlayerSlot != static_cast<BYTE>(g_pChitin->cNetwork.m_nLocalPlayer)) {
                                        // NOTE: Unused.
                                        CString sPlayerName;

                                        if (nPlayerSlot != -1) {
                                            PLAYER_ID idPlayer = g_pChitin->cNetwork.GetPlayerID(nPlayerSlot);
                                            if (idPlayer != 0) {
                                                g_pChitin->cNetwork.KickPlayer(idPlayer, TRUE);
                                            }
                                        }
                                    }

                                    pGame->GetObjectArray()->ReleaseShare(nId,
                                        CGameObjectArray::THREAD_ASYNCH,
                                        INFINITE);
                                }
                            }
                        }
                    }

                    if (g_pChitin->cProgressBar.m_nSecondsToTimeout != nSecondsToTimeout) {
                        g_pChitin->cProgressBar.m_nSecondsToTimeout = nSecondsToTimeout;

                        g_pBaldurChitin->GetBaldurMessage()->SendProgressBarStatus(g_pChitin->cProgressBar.m_nActionProgress,
                            g_pChitin->cProgressBar.m_nActionTarget,
                            g_pChitin->cProgressBar.m_bWaiting,
                            g_pChitin->cProgressBar.m_nWaitingReason,
                            g_pChitin->cProgressBar.m_bTimeoutVisible,
                            g_pChitin->cProgressBar.m_nSecondsToTimeout);
                    }

                    while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_RESOURCE,
                                CBaldurMessage::MSG_SUBTYPE_RESOURCE_DEMAND,
                                nMsgFrom,
                                dwSize))
                        != NULL) {
                        pMessage->OnResourceDemanded(nMsgFrom, pData, dwSize);
                        delete pData;
                    }

                    while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_PROGRESSBAR,
                                CBaldurMessage::MSG_SUBTYPE_PROGRESSBAR_STATUS,
                                nMsgFrom,
                                dwSize))
                        != NULL) {
                        pMessage->OnProgressBarStatus(nMsgFrom, pData, dwSize);
                        delete pData;
                    }

                    while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_CMESSAGE,
                                CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_UPDATE,
                                nMsgFrom,
                                dwSize))
                        != NULL) {
                        CMessageSpriteUpdate* pUpdate = new CMessageSpriteUpdate(NULL, 0, 0);
                        if (pUpdate != NULL && pUpdate->UnmarshalMessage(pData, dwSize) == TRUE) {
                            if (pUpdate->sub_50D660(sTriggerArea) == TRUE) {
                                pUpdate->Run();
                            }
                        }

                        delete pData;
                        delete pUpdate;
                    }

                    for (BYTE nSlot = 0; nSlot < CMultiplayerSettings::MAX_CHARACTERS; nSlot++) {
                        LONG nId;
                        if (nSlot < 6) {
                            nId = pGame->m_characters[nSlot];
                        } else {
                            nId = CGameObjectArray::INVALID_INDEX;
                        }

                        BOOLEAN bMoved = FALSE;
                        BOOLEAN bInControl = TRUE;

                        CGameSprite* pSprite;
                        do {
                            rc = pGame->GetObjectArray()->GetShare(nId,
                                CGameObjectArray::THREAD_ASYNCH,
                                reinterpret_cast<CGameObject**>(&pSprite),
                                INFINITE);
                        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                        if (rc == CGameObjectArray::SUCCESS) {
                            // NOTE: `Animate` is the function at 0x728560 which tests
                            // the "not dead" state bit.
                            if (pSprite->Animate()
                                && pSprite->m_pArea != NULL
                                && pSprite->m_pArea != pArea
                                // NOTE: The local player is compared as a byte.
                                && pGame->m_multiplayerSettings.GetCharacterControlledByPlayer(nSlot) == static_cast<BYTE>(g_pChitin->cNetwork.m_nLocalPlayer)) {
                                bMoved = TRUE;

                                // NOTE: Uninline.
                                bInControl = pSprite->InControl();
                            }

                            pGame->GetObjectArray()->ReleaseShare(nId,
                                CGameObjectArray::THREAD_ASYNCH,
                                INFINITE);
                        }

                        if (bMoved == TRUE) {
                            if (!bInControl) {
                                g_pBaldurChitin->GetObjectGame()->m_multiplayerSettings.SetCharacterControlledByPlayer(nSlot,
                                    static_cast<BYTE>(g_pChitin->cNetwork.m_nLocalPlayer),
                                    TRUE,
                                    TRUE);
                            }

                            do {
                                rc = pGame->GetObjectArray()->GetDeny(nId,
                                    CGameObjectArray::THREAD_ASYNCH,
                                    reinterpret_cast<CGameObject**>(&pSprite),
                                    INFINITE);
                            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                            if (rc == CGameObjectArray::SUCCESS) {
                                pSprite->m_cLastSpriteUpdate.Initialize(TRUE);

                                CAIAction action(CAIAction::LEAVEAREALUA, sTriggerArea, ptEnter, 0, 1);

                                pSprite->ClearActions(FALSE);
                                pSprite->m_targetPoint.x = -1;
                                pSprite->m_targetPoint.y = -1;
                                pSprite->m_interrupt = TRUE;
                                pSprite->SetCurrAction(action);
                                pSprite->ExecuteAction();

                                if (g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_nCharacters == 0) {
                                    g_pBaldurChitin->GetObjectGame()->UnselectAll();
                                    g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_bPicked = FALSE;
                                    g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_iPicked = -1;
                                    g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_nToolTip = 0;
                                    g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->OnDeactivation();
                                    g_pBaldurChitin->GetObjectGame()->m_visibleArea = pSprite->m_pArea->m_id;
                                    g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->OnActivation();
                                }

                                pGame->GetObjectArray()->ReleaseDeny(nId,
                                    CGameObjectArray::THREAD_ASYNCH,
                                    INFINITE);
                            }
                        }
                    }

                    g_pChitin->m_bDisplayStale = TRUE;
                    SleepEx(60, FALSE);
                } while (!pMessage->NonBlockingWaitForSignal(CBaldurMessage::SIGNAL_SERVER, 51));
            }

            while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_CMESSAGE,
                        CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_UPDATE,
                        nMsgFrom,
                        dwSize))
                != NULL) {
                CMessageSpriteUpdate* pUpdate = new CMessageSpriteUpdate(NULL, 0, 0);
                if (pUpdate != NULL && pUpdate->UnmarshalMessage(pData, dwSize) == TRUE) {
                    if (pUpdate->sub_50D660(sTriggerArea) == TRUE) {
                        pUpdate->Run();
                    }
                }

                delete pData;
                delete pUpdate;
            }

            // NOTE: 0x84CFC0 is the unnamed signal '4'.
            pMessage->SendSignal(CBaldurMessage::SIGNAL_ALL_CLIENTS, 52);

            pGame->m_multiplayerSettings.SetListenToJoinOption(bListenToJoin, FALSE);
        } else if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE) {
            while (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE) {
                while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_PROGRESSBAR,
                            CBaldurMessage::MSG_SUBTYPE_PROGRESSBAR_STATUS,
                            nMsgFrom,
                            dwSize))
                    != NULL) {
                    pMessage->OnProgressBarStatus(nMsgFrom, pData, dwSize);
                    delete pData;
                }

                while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_CMESSAGE,
                            CBaldurMessage::MSG_SUBTYPE_CMESSAGE_SPRITE_UPDATE,
                            nMsgFrom,
                            dwSize))
                    != NULL) {
                    CMessageSpriteUpdate* pUpdate = new CMessageSpriteUpdate(NULL, 0, 0);
                    if (pUpdate != NULL && pUpdate->UnmarshalMessage(pData, dwSize) == TRUE) {
                        if (pUpdate->sub_50D660(sTriggerArea) == TRUE) {
                            pUpdate->Run();
                        }
                    }

                    delete pData;
                    delete pUpdate;
                }

                g_pChitin->m_bDisplayStale = TRUE;
                SleepEx(60, FALSE);

                BOOLEAN bPending = FALSE;
                for (BYTE nSlot = 0; nSlot < CMultiplayerSettings::MAX_CHARACTERS; nSlot++) {
                    LONG nId;
                    if (nSlot < 6) {
                        nId = pGame->m_characters[nSlot];
                    } else {
                        nId = CGameObjectArray::INVALID_INDEX;
                    }

                    CGameSprite* pSprite;
                    do {
                        rc = pGame->GetObjectArray()->GetShare(nId,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        // NOTE: `Animate` is the function at 0x728560 which tests
                        // the "not dead" state bit.
                        if (pSprite->Animate()
                            && pSprite->m_pArea != NULL
                            && pSprite->m_pArea != pArea) {
                            bPending = TRUE;
                        }

                        pGame->GetObjectArray()->ReleaseShare(nId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }

                if (bPending) {
                    continue;
                }

                if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE) {
                    pMessage->SendSignal(CBaldurMessage::SIGNAL_SERVER, 51);

                    if (!pMessage->NonBlockingWaitForSignal(CBaldurMessage::SIGNAL_ALL_CLIENTS, 52)) {
                        do {
                            while ((pData = pMessage->PollSpecificMessageType(CBaldurMessage::MSG_TYPE_PROGRESSBAR,
                                        CBaldurMessage::MSG_SUBTYPE_PROGRESSBAR_STATUS,
                                        nMsgFrom,
                                        dwSize))
                                != NULL) {
                                pMessage->OnProgressBarStatus(nMsgFrom, pData, dwSize);
                                delete pData;
                            }

                            g_pChitin->m_bDisplayStale = TRUE;
                            SleepEx(60, FALSE);
                        } while (!pMessage->NonBlockingWaitForSignal(CBaldurMessage::SIGNAL_ALL_CLIENTS, 52));
                    }
                }

                break;
            }
        }

        g_pChitin->cProgressBar.m_bTimeoutVisible = FALSE;
        g_pChitin->cProgressBar.m_nActionProgress += g_pChitin->cProgressBar.m_nActionTarget - g_pChitin->cProgressBar.m_nActionProgress;
        g_pChitin->m_bDisplayStale = TRUE;
        sub_59FA00(TRUE);

        g_pChitin->SetProgressBar(FALSE, 0, 0, 0, FALSE, 0, FALSE, 0, FALSE, FALSE, 0xFF);

        g_pBaldurChitin->cSoundMixer.m_nActiveArea = reinterpret_cast<DWORD_PTR>(pArea);
        g_pBaldurChitin->cSoundMixer.UpdateSoundList();
        g_pBaldurChitin->m_pEngineWorld->m_weather.OnAreaChange(FALSE);

        if (g_pChitin->cNetwork.m_bConnectionEstablished == TRUE
            && g_pChitin->cNetwork.m_bIsHost == TRUE) {
            // NOTE: Shadows the outer `pGame`.
            CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 21370
            UTIL_ASSERT(pGame != NULL);

            for (INT nSlot = 0; nSlot < CMultiplayerSettings::MAX_CHARACTERS; nSlot++) {
                LONG nId;
                if (nSlot < 6) {
                    nId = pGame->m_characters[nSlot];
                } else {
                    nId = CGameObjectArray::INVALID_INDEX;
                }

                BOOLEAN bMoved = FALSE;
                BOOLEAN bInControl = FALSE;

                CGameSprite* pSprite;
                do {
                    rc = pGame->GetObjectArray()->GetShare(nId,
                        CGameObjectArray::THREAD_ASYNCH,
                        reinterpret_cast<CGameObject**>(&pSprite),
                        INFINITE);
                } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                if (rc == CGameObjectArray::SUCCESS) {
                    // NOTE: `Animate` is the function at 0x728560 which tests
                    // the "not dead" state bit.
                    if (pSprite->Animate()
                        && pSprite->m_pArea != NULL
                        && pSprite->m_pArea != pArea) {
                        bMoved = TRUE;

                        // NOTE: Uninline.
                        bInControl = pSprite->InControl();
                    }

                    pGame->GetObjectArray()->ReleaseShare(nId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }

                if (bMoved == TRUE) {
                    if (!bInControl) {
                        g_pBaldurChitin->GetObjectGame()->m_multiplayerSettings.SetCharacterControlledByPlayer(nSlot,
                            static_cast<BYTE>(g_pChitin->cNetwork.m_nLocalPlayer),
                            TRUE,
                            TRUE);
                    }

                    do {
                        rc = pGame->GetObjectArray()->GetDeny(nId,
                            CGameObjectArray::THREAD_ASYNCH,
                            reinterpret_cast<CGameObject**>(&pSprite),
                            INFINITE);
                    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

                    if (rc == CGameObjectArray::SUCCESS) {
                        pSprite->m_cLastSpriteUpdate.Initialize(TRUE);

                        CAIAction action(CAIAction::LEAVEAREALUA, sTriggerArea, ptEnter, 0, 1);

                        pSprite->ClearActions(FALSE);
                        pSprite->m_targetPoint.x = -1;
                        pSprite->m_targetPoint.y = -1;
                        pSprite->m_interrupt = TRUE;
                        pSprite->SetCurrAction(action);
                        pSprite->ExecuteAction();

                        if (g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_nCharacters == 0) {
                            g_pBaldurChitin->GetObjectGame()->UnselectAll();
                            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_bPicked = FALSE;
                            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_iPicked = -1;
                            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->m_nToolTip = 0;
                            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->OnDeactivation();
                            g_pBaldurChitin->GetObjectGame()->m_visibleArea = pSprite->m_pArea->m_id;
                            g_pBaldurChitin->GetObjectGame()->GetVisibleArea()->OnActivation();
                        }

                        pGame->GetObjectArray()->ReleaseDeny(nId,
                            CGameObjectArray::THREAD_ASYNCH,
                            INFINITE);
                    }
                }
            }
        }
    }
}

// 0x5C72D0
BOOLEAN CInfGame::GetGameSpyCharacterInformation(INT nCharacterSlot, CString& sName, CString& sRace, CString& sClass, CString& sLevel)
{
    const CRuleTables& ruleTables = g_pBaldurChitin->GetObjectGame()->GetRuleTables();
    CString sTemp;

    LONG nCharacterId = GetFixedOrderCharacterId(nCharacterSlot);

    if (nCharacterId == CGameObjectArray::INVALID_INDEX) {
        return FALSE;
    }

    CGameSprite* pSprite;

    BYTE rc;
    rc = m_cObjectArray.GetDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        reinterpret_cast<CGameObject**>(&pSprite),
        INFINITE);

    if (rc != CGameObjectArray::SUCCESS) {
        return FALSE;
    }

    BYTE nClass = pSprite->m_derivedStats.GetBestClass();

    sName = pSprite->GetName();
    ruleTables.GetRaceStringMixed(pSprite->m_liveTypeAI.m_nRace, sClass, 0);
    ruleTables.GetClassStringGameSpy(nClass, pSprite->m_baseStats.m_specialization, sClass);

    sTemp.Format("/%d", pSprite->m_derivedStats.m_nLevel);
    sLevel += sTemp;

    m_cObjectArray.ReleaseDeny(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);

    return TRUE;
}

// 0x5C7440
void CInfGame::PanicRescueItem(CItem* pItem, LONG nCharacterId)
{
    INT nEmptySlot = -1;
    INT nReplaceableSlot = -1;

    // Only critical items are rescued, everything else is simply destroyed.
    if ((pItem->GetFlagsFile() & 0x1) == 0) {
        // NOTE: Uninline.
        AddDisposableItem(pItem);
        return;
    }

    CGameSprite* pSprite;
    BYTE rc;
    do {
        rc = m_cObjectArray.GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
    } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

    if (rc != CGameObjectArray::SUCCESS) {
        return;
    }

    for (INT nSlot = 18; nSlot <= 33; nSlot++) {
        // NOTE: Always true.
        if (nSlot != 10) {
            CItem* pSlotItem = pSprite->m_equipment.m_items[nSlot];
            if (pSlotItem == NULL) {
                nEmptySlot = nSlot;
            } else if ((pSlotItem->GetFlagsFile() & 0x1) == 0) {
                nReplaceableSlot = nSlot;
            }
        }
    }

    if (nEmptySlot >= 0) {
        pSprite->m_equipment.m_items[nEmptySlot] = pItem;

        CMessage* pMessage = new CMessageSpriteEquipment(pSprite, nCharacterId, nCharacterId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    } else if (nReplaceableSlot >= 0) {
        // NOTE: Uninline.
        AddDisposableItem(pSprite->m_equipment.m_items[nReplaceableSlot]);

        pSprite->m_equipment.m_items[nReplaceableSlot] = pItem;

        CMessage* pMessage = new CMessageSpriteEquipment(pSprite, nCharacterId, nCharacterId);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(pMessage, FALSE);
    } else {
        // NOTE: Uninline.
        AddDisposableItem(pItem);
    }

    m_cObjectArray.ReleaseShare(nCharacterId,
        CGameObjectArray::THREAD_ASYNCH,
        INFINITE);
}

// 0x5C76B0
void CInfGame::MultiplayerSetCharacterCreationLocation()
{
    CResRef areaResRef;
    CString sAreaString;
    BOOLEAN bValueSet = FALSE;

    for (BYTE nIndex = 0; nIndex < 6; nIndex++) {
        if (bValueSet) {
            break;
        }

        // NOTE: Uninline `GetFixedOrderCharacterId`.
        LONG nCharacterId = g_pBaldurChitin->GetObjectGame()->GetFixedOrderCharacterId(nIndex);

        if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
            CGameSprite* pSprite;

            BYTE rc;
            do {
                rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                    CGameObjectArray::THREAD_ASYNCH,
                    reinterpret_cast<CGameObject**>(&pSprite),
                    INFINITE);
            } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

            if (rc == CGameObjectArray::SUCCESS) {
                CGameArea* pArea = pSprite->GetArea();
                if (pArea != NULL) {
                    areaResRef = pArea->m_resRef;
                    areaResRef.CopyToString(sAreaString);

                    CPoint ptStart = pSprite->GetPos();
                    g_pBaldurChitin->GetObjectGame()->GetMultiplayerSettings()->SetCharacterCreationLocation(sAreaString,
                        ptStart);

                    bValueSet = TRUE;

                    // TODO: Probably wrong, object released only when area is
                    // not null.
                    g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                        CGameObjectArray::THREAD_ASYNCH,
                        INFINITE);
                }
            }
        }
    }
}

// 0x5C7830
BOOL CInfGame::IsItemExclusive(CGameSprite* pSprite, INT slotNum, CItem* pItem, STRREF& strError)
{
    const CRuleTables& cRule = g_pBaldurChitin->GetObjectGame()->GetRuleTables();

    strError = -1;

    INT slots[] = {
        7,
        8,
        1,
        4,
        0,
    };

    if (pItem == NULL || !cRule.IsItemExclusive(pItem)) {
        return FALSE;
    }

    for (int index = 0; index < 5; index++) {
        if (slotNum != slots[index]
            && pSprite->GetEquipment()->m_items[slots[index]] != NULL
            && cRule.IsItemExclusive(pSprite->GetEquipment()->m_items[slots[index]])) {
            strError = 20685;
            return TRUE;
        }
    }

    return FALSE;
}

// 0x5C78F0
CWorldMap* CInfGame::GetWorldMap(CString sArea)
{
    CString v1;

    // NOTE: Not sure what first condition is for.
    if (sArea
        && sArea[0] != '\0'
        && sArea[1] != '\0'
        && sArea[2] != '\0') {
        v1 = sArea.Mid(2);
        if (atoi(v1) >= 9000) {
            return &m_cOtherWorldMap;
        }
    }

    return &m_cWorldMap;
}

// 0x5C79C0
BOOL CInfGame::sub_5C79C0(CString sArea)
{
    CString v1;

    // NOTE: Not sure what first condition is for.
    if (sArea
        && sArea[0] != '\0'
        && sArea[1] != '\0'
        && sArea[2] != '\0') {
        v1 = sArea.Mid(2);
        if (atoi(v1) >= 9000) {
            return TRUE;
        }
    }

    return FALSE;
}

// 0x5C7B10
DWORD CInfGame::AddItemToStore(const CResRef& cResStore, CItem* pItem, STRREF& strError)
{
    CMessage* message;

    strError = -1;

    DWORD nAdded = 0;
    WORD nOldUsageCount = 0;

    if (g_pChitin->cNetwork.GetSessionOpen()) {
        BOOL bStoreDemanded = FALSE;
        CStore cStore;

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            DemandServerStore(cResStore, TRUE);
            cStore.SetResRef(cResStore);
        } else {
            cStore.SetResRef(cResStore);
            if (!cStore.m_bLocalCopy || memcmp(cStore.m_pVersion, "STORV9.0", 8) != 0) {
                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(cResStore.GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return 0;
                }

                cStore.SetResRef(cResStore);
                bStoreDemanded = TRUE;
            }
        }

        if (cStore.m_header.m_nStoreType == 4) {
            if (cStore.IsItemAllowed(*pItem)) {
                if (cStore.m_header.m_nCapacity != 0
                    && cStore.m_header.m_nCapacity != cStore.GetNumItems()) {
                    if (pItem->GetMaxStackable() > 1) {
                        nAdded = pItem->GetUsageCount(0);
                        nOldUsageCount = pItem->GetUsageCount(0);

                        if (pItem->GetMaxStackable() > 1) {
                            if (static_cast<INT>(nAdded) / pItem->GetMaxStackable() + cStore.GetNumItems() > cStore.m_header.m_nCapacity) {
                                nAdded = cStore.m_header.m_nCapacity - cStore.GetNumItems();
                            }
                        } else {
                            // NOTE: Unreachable.
                            if (cStore.GetNumItems() + static_cast<INT>(nAdded) > cStore.m_header.m_nCapacity) {
                                nAdded = cStore.m_header.m_nCapacity - cStore.GetNumItems();
                            }
                        }

                        pItem->SetUsageCount(0, static_cast<WORD>(nAdded));
                    } else {
                        nAdded = 1;
                    }

                    if (!g_pChitin->cNetwork.GetSessionHosting() && !bStoreDemanded) {
                        bStoreDemanded = TRUE;
                        message = new CMessageStoreDemand(cResStore, -1, -1);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    }

                    message = new CMessageStoreAddItem(cStore.m_resRef, pItem->GetItemFile(), -1, -1);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                    if (g_pChitin->cNetwork.GetSessionHosting()) {
                        CStore* pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(cStore.m_resRef);

                        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                        // __LINE__: 22140
                        UTIL_ASSERT(pStore != NULL);

                        pStore->AddItemExt(*pItem, 1);
                    } else {
                        CStore::InvalidateStore(cStore.m_resRef);
                    }

                    if (pItem->GetMaxStackable() > 1) {
                        pItem->SetUsageCount(0, nOldUsageCount);
                    }
                } else {
                    // "The container is full."
                    strError = 24889;
                }
            } else {
                strError = 26285;
            }
        } else {
            strError = 26285;
        }

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(cStore.m_resRef);
        } else {
            if (bStoreDemanded) {
                message = new CMessageStoreRelease(cStore.m_resRef, -1, -1);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    } else {
        CStore cStore(cResStore);

        if (cStore.m_header.m_nStoreType == 4 && cStore.IsItemAllowed(*pItem)) {
            if (cStore.m_header.m_nCapacity != 0
                && cStore.m_header.m_nCapacity != cStore.GetNumItems()) {
                nAdded = 1;
                if (pItem->GetMaxStackable() > 1) {
                    nAdded = pItem->GetUsageCount(0);
                }

                if (cStore.GetNumItems() + static_cast<INT>(nAdded) > cStore.m_header.m_nCapacity) {
                    nAdded = cStore.m_header.m_nCapacity - cStore.GetNumItems();
                }

                pItem->SetUsageCount(0, static_cast<WORD>(nAdded));
                cStore.AddItemExt(*pItem, 1);
            } else {
                // "The container is full."
                strError = 24889;
            }
        } else {
            strError = 26285;
        }

        if (strError == -1) {
            cStore.Marshal(m_sTempDir);
        }
    }

    return nAdded;
}

// 0x5C8190
DWORD CInfGame::FindItemInStore(const CResRef& cResStore, const CResRef& cResItem, BOOL checkForIdentified)
{
    if (g_pChitin->cNetwork.GetSessionOpen()) {
        BOOL bStoreDemanded = FALSE;
        CStore cStore;
        if (g_pChitin->cNetwork.GetSessionHosting()) {
            DemandServerStore(cResStore, TRUE);
            cStore.SetResRef(cResStore);
        } else {
            cStore.SetResRef(cResStore);
            if (!cStore.m_bLocalCopy || memcmp(cStore.m_pVersion, "STORV9.0", 8) != 0) {
                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(cResStore.GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return 0;
                }
                cStore.SetResRef(cResStore);
                bStoreDemanded = TRUE;
            }
        }

        CResRef curItemRef;
        DWORD inStock = 0;
        CItem tempItem;

        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curItemRef = cStore.GetItemId(nIndex);
            if (curItemRef == cResItem) {
                if (checkForIdentified) {
                    cStore.GetItem(nIndex, tempItem);
                    if ((tempItem.m_flags & 0x1) != 0) {
                        inStock += cStore.GetItemNumInStock(nIndex);
                    }
                } else {
                    inStock += cStore.GetItemNumInStock(nIndex);
                }
            }
        }

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            ReleaseServerStore(cResStore);
        } else {
            if (bStoreDemanded) {
                CMessage* message = new CMessageStoreRelease(cResStore, -1, -1);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }

        return inStock;
    } else {
        CStore cStore(cResStore);
        CResRef curItemRef;
        DWORD inStock = 0;

        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curItemRef = cStore.GetItemId(nIndex);
            if (curItemRef == cResItem) {
                inStock += cStore.GetItemNumInStock(nIndex);
            }
        }

        return inStock;
    }
}

// 0x5C8510
SHORT CInfGame::TakeItemFromStore(const CResRef& cResStore, const CResRef& cResItem, LONG number)
{
    CItem tempItem;
    CMessage* message;

    if (g_pChitin->cNetwork.GetSessionOpen()) {
        BOOL bStoreDemanded = FALSE;
        CStore cStore;

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            DemandServerStore(cResStore, TRUE);
            cStore.SetResRef(cResStore);
        } else {
            cStore.SetResRef(cResStore);
            if (!cStore.m_bLocalCopy || memcmp(cStore.m_pVersion, "STORV9.0", 8) != 0) {
                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(cResStore.GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return 0;
                }

                cStore.SetResRef(cResStore);
                bStoreDemanded = TRUE;
            }
        }

        CResRef curResItem;
        CStore* pStore;
        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curResItem = cStore.GetItemId(nIndex);
            if (curResItem == cResItem) {
                DWORD inStock = cStore.GetItemNumInStock(nIndex);
                cStore.GetItem(nIndex, tempItem);

                if (!g_pChitin->cNetwork.GetSessionHosting() && !bStoreDemanded) {
                    bStoreDemanded = TRUE;
                    message = new CMessageStoreDemand(cResStore, -1, -1);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }

                if (g_pChitin->cNetwork.GetSessionHosting()) {
                    pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(cResStore);

                    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                    // __LINE__: 22412
                    UTIL_ASSERT(pStore != NULL);
                }

                // FIXME: Looks wrong, `removed` is reset on every iteration,
                // but probably should be accumulated.
                DWORD removed = 0;
                // NOTE: Unsigned compare.
                while (removed < inStock && removed < static_cast<DWORD>(number)) {
                    message = new CMessageStoreRemoveItem(cResStore,
                        cResItem,
                        tempItem.m_flags,
                        -1,
                        1,
                        -1,
                        -1);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                    if (g_pChitin->cNetwork.GetSessionHosting()) {
                        pStore->RemoveItemExt(cResItem, tempItem.m_flags, -1, 1, NULL);
                    }

                    removed++;
                }

                if (!g_pChitin->cNetwork.GetSessionHosting()) {
                    CStore::InvalidateStore(cResStore);
                }

                // NOTE: Unsigned compare.
                if (removed >= static_cast<DWORD>(number)) {
                    if (g_pChitin->cNetwork.GetSessionHosting()) {
                        g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(cResStore);
                    } else {
                        if (bStoreDemanded) {
                            message = new CMessageStoreRelease(cResStore, -1, -1);
                            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                        }
                    }

                    return static_cast<SHORT>(removed);
                }
            }
        }

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(cResStore);
        } else {
            if (bStoreDemanded) {
                message = new CMessageStoreRelease(cResStore, -1, -1);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
        return 0;
    } else {
        CStore cStore(cResStore);
        CResRef curItemRef;
        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curItemRef = cStore.GetItemId(nIndex);
            cStore.GetItem(nIndex, tempItem);
            if (curItemRef == cResItem) {
                DWORD inStock = cStore.GetItemNumInStock(nIndex);
                // FIXME: Looks wrong, `removed` is reset on every iteration,
                // but probably should be accumulated.
                DWORD removed = 0;
                // NOTE: Unsigned compare.
                while (removed < static_cast<DWORD>(number) && removed < inStock) {
                    cStore.RemoveItemExt(cResItem, tempItem.m_flags, -1, 1, NULL);
                    removed++;
                }

                // NOTE: Unsigned compare.
                if (removed >= static_cast<DWORD>(number)) {
                    cStore.Marshal(m_sTempDir);
                    return static_cast<SHORT>(removed);
                }
            }
        }
        cStore.Marshal(m_sTempDir);
        return 0;
    }
}

// 0x5C8C30
CItem* CInfGame::RemoveItemFromBag(const CResRef& cResStore, const CResRef& cResItem)
{
    CItem tempItem;
    CItem* pItem;
    CMessage* message;

    if (g_pChitin->cNetwork.GetSessionOpen()) {
        BOOL bStoreDemanded = FALSE;
        CStore cStore;

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            DemandServerStore(cResStore, TRUE);
            cStore.SetResRef(cResStore);
        } else {
            cStore.SetResRef(cResStore);
            if (!cStore.m_bLocalCopy || memcmp(cStore.m_pVersion, "STORV9.0", 8) != 0) {
                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(cResStore.GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return NULL;
                }

                cStore.SetResRef(cResStore);
                bStoreDemanded = TRUE;
            }
        }

        CResRef curItemRef;
        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curItemRef = cStore.GetItemId(nIndex);
            cStore.GetItem(nIndex, tempItem);
            if (curItemRef == cResItem) {
                pItem = new CItem();
                cStore.GetItem(nIndex, *pItem);

                if (!g_pChitin->cNetwork.GetSessionHosting() && !bStoreDemanded) {
                    bStoreDemanded = TRUE;
                    message = new CMessageStoreDemand(cResStore, -1, -1);
                    g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                }

                message = new CMessageStoreRemoveItem(cResStore,
                    cResItem,
                    pItem->m_flags,
                    -1,
                    1,
                    -1,
                    -1);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

                if (g_pChitin->cNetwork.GetSessionHosting()) {
                    CStore* pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(cStore.m_resRef);

                    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                    // __LINE__: 22577
                    UTIL_ASSERT(pStore != NULL);

                    pStore->RemoveItemExt(cResItem, pItem->m_flags, -1, 1, NULL);
                } else {
                    CStore::InvalidateStore(cResStore);
                }

                if (g_pChitin->cNetwork.GetSessionHosting()) {
                    g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(cStore.m_resRef);
                } else {
                    if (bStoreDemanded) {
                        message = new CMessageStoreRelease(cResStore, -1, -1);
                        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
                    }
                }

                return pItem;
            }
        }

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(cResStore);
        } else {
            if (bStoreDemanded) {
                message = new CMessageStoreRelease(cResStore, -1, -1);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    } else {
        CStore cStore(cResStore);
        CResRef curItemRef;
        for (INT nIndex = 0; nIndex < cStore.GetNumItems(); nIndex++) {
            curItemRef = cStore.GetItemId(nIndex);
            cStore.GetItem(nIndex, tempItem);
            if (curItemRef == cResItem) {
                pItem = new CItem();
                cStore.GetItem(nIndex, *pItem);
                cStore.RemoveItemExt(cResItem, pItem->m_flags, -1, 1, NULL);
                cStore.Marshal(m_sTempDir);
                return pItem;
            }
        }
    }

    return NULL;
}

// 0x5C93E0
INT CInfGame::sub_5C93E0()
{
    INT nCount = 0;

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        LONG nCharacterId = GetCharacterId(nPortrait);

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = g_pBaldurChitin->GetObjectGame()->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->field_70F2 == 1) {
                nCount++;
            }

            g_pBaldurChitin->GetObjectGame()->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    return nCount;
}

// NOTE: Somewhat broken version of `LoadMultiPlayerPermissions`.
//
// 0x5C9490
void CInfGame::ResetMultiPlayerPermissions()
{
    if (!g_pBaldurChitin->cNetwork.GetSessionOpen()
        || !g_pBaldurChitin->cNetwork.GetSessionHosting()) {
        return;
    }

    CMultiplayerSettings* pSettings = GetMultiplayerSettings();

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 22952
    UTIL_ASSERT(pSettings != NULL);

    CString sPermissions;
    CString sDefault("");

    GetPrivateProfileStringA("Multiplayer",
        "Default Permissions",
        sDefault,
        sPermissions.GetBuffer(10),
        10,
        g_pBaldurChitin->GetIniFileName());
    sPermissions.ReleaseBuffer(-1);

    if (sPermissions.GetLength() == 8) {
        BOOL bPurchasing = sPermissions[0] == '1';
        BOOL bAreaTransition = sPermissions[1] == '1';
        BOOL bDialog = sPermissions[2] == '1';
        BOOL bCharRecords = sPermissions[3] == '1';
        BOOL bPausing = sPermissions[4] == '1';
        BOOL bGroupPool = sPermissions[5] == '1';
        BOOL bLeader = sPermissions[6] == '1';
        BOOL bModifyChars = sPermissions[7] == '1';

        // FIXME: Individual permission values are used as player slots. So
        // depending on the value appropriate permission is set either on
        // player 0 or 1.

        pSettings->SetPermission(bPurchasing,
            CGamePermission::PURCHASING,
            bPurchasing,
            TRUE);
        pSettings->SetPermission(bAreaTransition,
            CGamePermission::AREA_TRANSITION,
            bAreaTransition,
            TRUE);
        pSettings->SetPermission(bDialog,
            CGamePermission::DIALOG,
            bDialog,
            TRUE);
        pSettings->SetPermission(bCharRecords,
            CGamePermission::CHAR_RECORDS,
            bCharRecords,
            TRUE);
        pSettings->SetPermission(bPausing,
            CGamePermission::PAUSING,
            bPausing,
            TRUE);
        pSettings->SetPermission(bGroupPool,
            CGamePermission::GROUP_POOL,
            bGroupPool,
            TRUE);
        pSettings->SetPermission(bLeader,
            CGamePermission::LEADER,
            bLeader,
            TRUE);
        pSettings->SetPermission(bModifyChars,
            CGamePermission::MODIFY_CHARS,
            bModifyChars,
            TRUE);
    }

    INT nRestrictStore = GetPrivateProfileIntA("Multiplayer",
        "Restrict Stores",
        0,
        g_pBaldurChitin->GetIniFileName());
    pSettings->SetRestrictStoreOption(nRestrictStore != 0);

    BYTE nImportingBitField = static_cast<BYTE>(GetPrivateProfileIntA("Multiplayer",
        "Import Character",
        CMultiplayerSettings::IMPORT_ALL,
        g_pBaldurChitin->GetIniFileName()));
    pSettings->SetImportingCharacterOption(nImportingBitField);
}

// 0x5C9720
INT CInfGame::GetNumberOfItemsInBag(const CResRef& storeResRef, CString a2)
{
    CMessage* message;
    INT nNumberOfItemsInBag = 0;

    if (g_pChitin->cNetwork.GetSessionOpen()) {
        BOOL demanded = FALSE;
        CStore store;
        if (g_pChitin->cNetwork.GetSessionHosting()) {
            DemandServerStore(storeResRef, TRUE);
            store.SetResRef(storeResRef);
        } else {
            store.SetResRef(storeResRef);
            if (!store.m_bLocalCopy || memcmp(store.m_pVersion, "STORV9.0", 8) != 0) {
                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(storeResRef.GetResRefStr(),
                        1014,
                        TRUE,
                        TRUE,
                        TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return FALSE;
                }

                store.SetResRef(storeResRef);
                demanded = TRUE;
            }
        }

        CResRef itemResRef;

        if (g_pChitin->cNetwork.GetSessionHosting() || demanded) {
            store.InvalidateStore(storeResRef);
        } else {
            demanded = TRUE;
            message = new CMessageStoreDemand(storeResRef, -1, -1);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        for (INT nIndex = 0; nIndex < store.GetNumItems(); nIndex++) {
            itemResRef = store.GetItemId(nIndex);

            // FIXME: Temporary instance, should be on stack.
            CItem* pItem = new CItem();
            store.GetItem(nIndex, *pItem);
            if (pItem->GetResRef() != a2) {
                nNumberOfItemsInBag += store.GetItemNumInStock(nIndex);
            }

            // FIXME: Probably leaking `pItem`.
        }

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            g_pBaldurChitin->GetObjectGame()->ReleaseServerStore(storeResRef);
        } else {
            if (demanded) {
                message = new CMessageStoreRelease(storeResRef, -1, -1);
                g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
            }
        }
    } else {
        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 23072
        UTIL_ASSERT_MSG(FALSE, "Very bad things are about to happen in GetNumberOfItemsInBag");
    }

    return nNumberOfItemsInBag;
}

// 0x5C9AC0
LONG CInfGame::GetAveragePartyLevel()
{
    LONG nAverageLevel = 0;

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        // NOTE: Uninline.
        LONG nCharacterId = GetCharacterId(nPortrait);

        if (nCharacterId != CGameObjectArray::INVALID_INDEX) {
            CGameSprite* pSprite;
            m_cObjectArray.GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);

            nAverageLevel += pSprite->GetLevel();

            m_cObjectArray.ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    if (nAverageLevel == 0 || nAverageLevel / m_nCharacters < 1) {
        return 1;
    }

    return nAverageLevel / m_nCharacters;
}

// 0x5C9C90
BOOL CInfGame::ChangeStoreMarkup(const CResRef& resRef, LONG nBuyMarkup, LONG nSellMarkup, LONG nAdditionalMarkup)
{
    CMessage* message;

    if (g_pChitin->cNetwork.GetSessionOpen()) {
        BOOL bStoreDemanded = FALSE;
        CStore cStore;

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            DemandServerStore(resRef, TRUE);
            cStore.SetResRef(resRef);
        } else {
            cStore.SetResRef(resRef);
            if (!cStore.m_bLocalCopy || memcmp(cStore.m_pVersion, "STORV9.0", 8) != 0) {
                if (!g_pBaldurChitin->GetBaldurMessage()->DemandResourceFromServer(resRef.GetResRefStr(), 1014, TRUE, TRUE, TRUE)) {
                    g_pChitin->cNetwork.CloseSession(TRUE);
                    return FALSE;
                }

                cStore.SetResRef(resRef);
                bStoreDemanded = TRUE;
            }
        }

        if (!g_pChitin->cNetwork.GetSessionHosting() && !bStoreDemanded) {
            message = new CMessageStoreDemand(resRef, -1, -1);
            g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);
        }

        message = new CMessage108(cStore.m_resRef,
            nBuyMarkup,
            nSellMarkup,
            nAdditionalMarkup,
            -1,
            -1);
        g_pBaldurChitin->GetMessageHandler()->AddMessage(message, FALSE);

        if (g_pChitin->cNetwork.GetSessionHosting()) {
            CStore* pStore = g_pBaldurChitin->GetObjectGame()->GetServerStore(cStore.m_resRef);

            // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
            // __LINE__: 23329
            UTIL_ASSERT(pStore != NULL);

            pStore->m_header.m_nBuyMarkUp = nBuyMarkup;
            pStore->m_header.m_nSellMarkDown = nSellMarkup;
            pStore->m_header.m_nAdditionalMarkDown = nAdditionalMarkup;
        } else {
            CStore::InvalidateStore(cStore.m_resRef);
        }

        // NOTE: Original code neither releases server store, nor sends
        // `CMessageStoreRelease`.
    } else {
        // NOTE: Original code does not apply new values in single player.
        CStore cStore(resRef);
        cStore.Marshal(m_sTempDir);
    }

    return TRUE;
}

// NOTE: Unclear why `nClass` is passed as pointer.
// TODO: Move to `CRuleTables`.
//
// 0x5CA5D0
UINT CInfGame::GetSpellcasterIndex(const BYTE& nClass)
{
    switch (nClass) {
    case CAIOBJECTTYPE_C_BARD:
        return 0;
    case CAIOBJECTTYPE_C_CLERIC:
        return 1;
    case CAIOBJECTTYPE_C_DRUID:
        return 2;
    case CAIOBJECTTYPE_C_PALADIN:
        return 3;
    case CAIOBJECTTYPE_C_RANGER:
        return 4;
    case CAIOBJECTTYPE_C_SORCERER:
        return 5;
    case CAIOBJECTTYPE_C_WIZARD:
        return 6;
    }

    CString sError;
    sError.Format("Invalid spell caster index: %d", nClass);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23583
    UTIL_ASSERT_MSG(FALSE, sError);
}

// NOTE: Unclear why `nList` is passed as pointer.
// TODO: Move to `CRuleTables`.
//
// 0x5CA730
BYTE CInfGame::GetSpellcasterClass(const UINT& nIndex)
{
    switch (nIndex) {
    case 0:
        return CAIOBJECTTYPE_C_BARD;
    case 1:
        return CAIOBJECTTYPE_C_CLERIC;
    case 2:
        return CAIOBJECTTYPE_C_DRUID;
    case 3:
        return CAIOBJECTTYPE_C_PALADIN;
    case 4:
        return CAIOBJECTTYPE_C_RANGER;
    case 5:
        return CAIOBJECTTYPE_C_SORCERER;
    case 6:
        return CAIOBJECTTYPE_C_WIZARD;
    }

    CString sError;
    sError.Format("Invalid spell caster class: %d", nIndex);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23624
    UTIL_ASSERT_MSG(FALSE, sError);
}

// NOTE: Unclear why `nIndex` is passed as pointer.
// TODO: Move to `CRuleTables`.
//
// 0x5CA870
DWORD CInfGame::GetSpellcasterClassMask(const UINT& nIndex)
{
    switch (nIndex) {
    case 0:
        return CLASSMASK_BARD;
    case 1:
        return CLASSMASK_CLERIC;
    case 2:
        return CLASSMASK_DRUID;
    case 3:
        return CLASSMASK_PALADIN;
    case 4:
        return CLASSMASK_RANGER;
    case 5:
        return CLASSMASK_SORCERER;
    case 6:
        return CLASSMASK_WIZARD;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23663
    UTIL_ASSERT_MSG(FALSE, "Invalid spell caster class mask");
}

// NOTE: Unclear why `a1` is passed as pointer.
// TODO: Move to `CRuleTables`.
//
// 0x5CA910
UINT CInfGame::GetSpellcasterDomainIndex(const DWORD& nSpecialization)
{
    if ((nSpecialization & SPECMASK_CLERIC_ILMATER) != 0) {
        return 0;
    } else if ((nSpecialization & SPECMASK_CLERIC_LATHANDER) != 0) {
        return 1;
    } else if ((nSpecialization & SPECMASK_CLERIC_SELUNE) != 0) {
        return 2;
    } else if ((nSpecialization & SPECMASK_CLERIC_HELM) != 0) {
        return 3;
    } else if ((nSpecialization & SPECMASK_CLERIC_OGHMA) != 0) {
        return 4;
    } else if ((nSpecialization & SPECMASK_CLERIC_TEMPUS) != 0) {
        return 5;
    } else if ((nSpecialization & SPECMASK_CLERIC_BANE) != 0) {
        return 6;
    } else if ((nSpecialization & SPECMASK_CLERIC_MASK) != 0) {
        return 7;
    } else if ((nSpecialization & SPECMASK_CLERIC_TALOS) != 0) {
        return 8;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23714
    UTIL_ASSERT_MSG(FALSE, "Invalid spell caster domain index");
}

// 0x5CA9E0
DWORD CInfGame::GetClassMask(const BYTE& nClass)
{
    switch (nClass) {
    case CAIOBJECTTYPE_C_BARBARIAN:
        return CLASSMASK_BARBARIAN;
    case CAIOBJECTTYPE_C_BARD:
        return CLASSMASK_BARD;
    case CAIOBJECTTYPE_C_CLERIC:
        return CLASSMASK_CLERIC;
    case CAIOBJECTTYPE_C_DRUID:
        return CLASSMASK_DRUID;
    case CAIOBJECTTYPE_C_FIGHTER:
        return CLASSMASK_FIGHTER;
    case CAIOBJECTTYPE_C_MONK:
        return CLASSMASK_MONK;
    case CAIOBJECTTYPE_C_PALADIN:
        return CLASSMASK_PALADIN;
    case CAIOBJECTTYPE_C_RANGER:
        return CLASSMASK_RANGER;
    case CAIOBJECTTYPE_C_ROGUE:
        return CLASSMASK_ROGUE;
    case CAIOBJECTTYPE_C_SORCERER:
        return CLASSMASK_SORCERER;
    case CAIOBJECTTYPE_C_WIZARD:
        return CLASSMASK_WIZARD;
    }

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23834
    UTIL_ASSERT_MSG(FALSE, "CInfGame::GetClassMask() - Invalid class mask!");
}

// 0x5CAAD0
BYTE CInfGame::GetSpellType(const CResRef& resRef)
{
    // NOTE: The exe uses the global game (`g_pBaldurChitin->GetObjectGame()`)
    // here, not `this`.
    UINT nID;

    // NOTE: Uninline.
    if (m_songs.Find(resRef, nID)) {
        return 3;
    }

    // NOTE: Uninline.
    if (m_shapeshifts.Find(resRef, nID)) {
        return 4;
    }

    if (m_innateSpells.Find(resRef, nID)) {
        return 2;
    }

    if (m_spells.Find(resRef, nID)) {
        return 1;
    }

    CString sError(" ");
    sError.Format("*** Invalid spell: %s\n", (LPCSTR)resRef.GetResRefStr());

    // TODO: Report error.

    return 0;
}

// 0x5CAC30
INT CInfGame::GetSpellLevel(const CResRef& resRef, BYTE nClass, DWORD nSpecialization)
{
    // NOTE: The exe uses the global game (`g_pBaldurChitin->GetObjectGame()`)
    // here, not `this`.
    UINT nID;

    // NOTE: Uninline.
    if (!m_spells.Find(resRef, nID)) {
        CString sError;
        sError.Format("Spell not in master list - Invalid spell: %s", (LPCSTR)resRef.GetResRefStr());

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 23940
        UTIL_ASSERT_MSG(FALSE, (LPCSTR)sError);
        return CSPELLLIST_MAX_LEVELS;
    }

    if (nSpecialization != 0) {
        UINT nDomainIndex = GetSpellcasterDomainIndex(nSpecialization);

        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            CSpellList* pSpells = &(m_spellsByDomain[nDomainIndex].m_lists[nLevel]);
            for (UINT nIndex = 0; nIndex < pSpells->m_nCount; nIndex++) {
                if (pSpells->m_pList[nIndex].m_nID == nID) {
                    return nLevel;
                }
            }
        }
    } else {
        UINT nClassIndex = GetSpellcasterIndex(nClass);

        for (UINT nLevel = 0; nLevel < CSPELLLIST_MAX_LEVELS; nLevel++) {
            CSpellList* pSpells = &(m_spellsByClass[nClassIndex].m_lists[nLevel]);
            for (UINT nIndex = 0; nIndex < pSpells->m_nCount; nIndex++) {
                if (pSpells->m_pList[nIndex].m_nID == nID) {
                    return nLevel;
                }
            }
        }
    }

    return CSPELLLIST_MAX_LEVELS;
}

// 0x5CADF0
void CInfGame::sub_5CADF0()
{
    // FIXME: What for?
    CInfGame* pGame = g_pBaldurChitin->GetObjectGame();

    for (SHORT nPortrait = 0; nPortrait < m_nCharacters; nPortrait++) {
        LONG nCharacterId = pGame->GetCharacterId(nPortrait);

        CGameSprite* pSprite;

        BYTE rc;
        do {
            rc = pGame->GetObjectArray()->GetShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                reinterpret_cast<CGameObject**>(&pSprite),
                INFINITE);
        } while (rc == CGameObjectArray::SHARED || rc == CGameObjectArray::DENIED);

        if (rc == CGameObjectArray::SUCCESS) {
            if ((pSprite->m_baseStats.m_generalState & STATE_DEAD) == 0
                && (pSprite->m_derivedStats.m_generalState & STATE_DEAD) == 0) {
                pSprite->field_9D15 = 1;
            }

            pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }
}

// NOTE: Odd location.
//
// 0x428620
INT CInfGame::GetCurrentChapter()
{
    CVariable* pVar = GetVariables()->FindKey(CHAPTER_GLOBAL);
    if (pVar != NULL) {
        return pVar->m_intValue;
    }
    return 0;
}

// NOTE: Odd location
//
// 0x435110
void CInfGame::SetCurrentChapter(INT nChapter)
{
    CVariable* pVar = GetVariables()->FindKey(CHAPTER_GLOBAL);
    if (pVar != NULL) {
        pVar->m_intValue = nChapter;
    }
}

// 0x452F50
CInfGameUnknownEntry::CInfGameUnknownEntry(const CString& sName)
    : m_sName(sName)
{
    field_4 = 0;
    field_8 = 0;
    memset(field_9, 0, sizeof(field_9));
}

// 0x452F80
//
// NOTE: The original is a `__thiscall` on `&field_4BDC` (a list helper).
CInfGameUnknownEntry* CInfGame::FindUnknownEntry(const CString& sName)
{
    POSITION pos = field_4BDC.GetHeadPosition();
    while (pos != NULL) {
        CInfGameUnknownEntry* pEntry = reinterpret_cast<CInfGameUnknownEntry*>(field_4BDC.GetNext(pos));
        if (pEntry != NULL && pEntry->m_sName.CompareNoCase(sName) == 0) {
            return pEntry;
        }
    }
    return NULL;
}

// 0x452FD0
CInfButtonArray* CInfGame::GetButtonArray()
{
    return &m_cButtonArray;
}

// 0x452FE0
LONG CInfGame::GetCharacterId(SHORT nPortrait)
{
    if (nPortrait < m_nCharacters) {
        return m_characterPortraits[nPortrait];
    }

    return CGameObjectArray::INVALID_INDEX;
}

// 0x453010
CTimerWorld* CInfGame::GetWorldTimer()
{
    return &m_worldTime;
}

// 0x453020
SHORT CInfGame::GetNumCharacters()
{
    return m_nCharacters;
}

// 0x453030
CGameObjectArray* CInfGame::GetObjectArray()
{
    return &m_cObjectArray;
}

// 0x453040
CGameArea* CInfGame::GetVisibleArea()
{
    return m_gameAreas[m_visibleArea];
}

// 0x453050
CGameSave* CInfGame::GetGameSave()
{
    return &m_gameSave;
}

// 0x453060
CMultiplayerSettings* CInfGame::GetMultiplayerSettings()
{
    return &m_multiplayerSettings;
}

// 0x439A70
CGameJournal* CInfGame::GetJournal()
{
    return &m_cJournal;
}

// 0x475DB0
CAIGroup* CInfGame::GetGroup()
{
    return &m_group;
}

// 0x4286A0
CVariableHash* CInfGame::GetVariables()
{
    return &m_variables;
}

// 0x6E4E80
CVidBitmap* CInfGame::GetMasterBitmap()
{
    return &m_rgbMasterBitmap;
}

// 0x45B610
LONG CInfGame::GetFixedOrderCharacterId(SHORT nSlot)
{
    if (nSlot < 6) {
        return m_characters[nSlot];
    }

    return CGameObjectArray::INVALID_INDEX;
}

// 0x40CAE0
CNamedCreatureVariableHash* CInfGame::GetNamedCreatures()
{
    return &m_namedCreatures;
}

// 0x68C010
void CInfGame::SetTempCursor(BYTE tempCursor)
{
    m_tempCursor = tempCursor;
}

// 0x45B630
CGameOptions* CInfGame::GetOptions()
{
    return &m_cOptions;
}

// 0x45B640
SHORT CInfGame::GetReputation()
{
    return m_nReputation / REPUTATION_MULTIPLIER;
}

// 0x45B660
CTypedPtrArray<CPtrArray, int*>& CInfGame::GetCharacterOverflow()
{
    return m_characterOverflow;
}

// 0x594070
SHORT CInfGame::GetState()
{
    return m_nState;
}

// 0x594080
void CInfGame::SetState(SHORT nState)
{
    m_nState = nState;
}

// 0x594090
BYTE CInfGame::GetIconIndex()
{
    return m_iconIndex;
}

// 0x5940A0
void CInfGame::SetIconIndex(BYTE iconIndex)
{
    m_iconIndex = iconIndex;
    m_iconResRef = "";
}

// 0x58FEC0
CSpellResRefList& CInfGame::GetMasterSongLookup()
{
    return m_songs;
}

// 0x5F67F0
CSpellResRefList* CInfGame::GetInnateSpells()
{
    return &m_innateSpells;
}

// 0x5F6800
CSpellResRefList* CInfGame::GetShapeshifts()
{
    return &m_shapeshifts;
}

// 0x64D660
CString CInfGame::GetSaveGame()
{
    return m_sSaveGame;
}

// 0x71AB80
void CInfGame::SetLastTarget(LONG id)
{
    m_lastClick.x = -1;
    m_lastClick.y = -1;
    m_lastTarget = id;
}

// NOTE: Inlined.
void CInfGame::SetLastClick(const CPoint& pt)
{
    m_lastClick = pt;
    m_lastTarget = CGameObjectArray::INVALID_INDEX;
}

// NOTE: Inlined.
BYTE CInfGame::GetKeymap(SHORT index)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1486
    UTIL_ASSERT(index >= 0 && index < CINFGAME_KEYMAP_SIZE);

    return m_pKeymap[index];
}

// NOTE: Inlined.
BYTE CInfGame::GetKeymapFlag(SHORT index)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1487
    UTIL_ASSERT(index >= 0 && index < CINFGAME_KEYMAP_SIZE);

    return m_pKeymapFlags[index];
}

// NOTE: Inlined
void CInfGame::SetQuickFormation(SHORT buttonNum, SHORT formation)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1499
    UTIL_ASSERT(buttonNum < CGAMESAVE_NUM_QUICK_FORMATIONS);

    m_gameSave.m_quickFormations[buttonNum] = formation;
}

// NOTE: Inlined.
void CInfGame::SetFormationToQuickFormation(SHORT buttonNum)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 1500
    UTIL_ASSERT(buttonNum < CGAMESAVE_NUM_QUICK_FORMATIONS);

    m_gameSave.m_curFormation = m_gameSave.m_quickFormations[buttonNum];
}

// NOTE: Inlined.
void CInfGame::SetFormation(SHORT formation)
{
    m_gameSave.m_curFormation = formation;
}

// NOTE: Inlined.
BOOL CInfGame::IsFamiliar(LONG id)
{
    return m_familiars.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(id))) != NULL;
}

// NOTE: Inlined.
BOOL CInfGame::IsAlly(LONG id)
{
    return m_allies.Find(reinterpret_cast<int*>(static_cast<INT_PTR>(id))) != NULL;
}

// -----------------------------------------------------------------------------

// NOTE: Inlined.
CSpellResRefList::CSpellResRefList()
{
    m_pList = NULL;
    m_nCount = 0;
}

// NOTE: Inlined in most places; an out-of-line copy exists at 0x59EB40.
//
// 0x59EB40
CSpellResRefList::~CSpellResRefList()
{
    if (m_pList != NULL) {
        delete m_pList;
        m_pList = NULL;
    }
    m_nCount = 0;
}

// FIXME: `nResRefColumn` should not be reference.
//
// 0x5C9FD0
void CSpellResRefList::Load(const C2DArray& Table, const INT& nResRefColumn)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23354
    UTIL_ASSERT(Table.GetWidth() > 0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23355
    UTIL_ASSERT(Table.GetHeight() > 0);

    // FIXME: Uses unsigned compare.
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23356
    UTIL_ASSERT(nResRefColumn < Table.GetWidth());

    if (m_pList != NULL) {
        delete m_pList;
        m_nCount = 0;
    }

    m_pList = new CResRef[Table.GetHeight()];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23369
    UTIL_ASSERT(m_pList != NULL);

    for (int index = 0; index < Table.GetHeight(); index++) {
        m_pList[index] = Table.GetAt(CPoint(nResRefColumn, index));
    }

    m_nCount = Table.GetHeight();
}

// 0x5CA160
bool CSpellResRefList::Find(const CResRef& resRef, UINT& nID)
{
    for (UINT nIndex = 0; nIndex < m_nCount; nIndex++) {
        if (m_pList[nIndex] == resRef) {
            nID = nIndex;
            return true;
        }
    }

    nID = 0;
    return false;
}

// NOTE: Inlined. Unclear why it's different from the one below.
const CResRef& CSpellResRefList::Get636(UINT nIndex)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 636
    UTIL_ASSERT(nIndex < m_nCount);

    // __FILE__: .\Include\InfGame.h
    // __LINE__: 637
    UTIL_ASSERT(m_pList != NULL);

    return m_pList[nIndex];
}

// FIXME: `nIndex` should not be reference.
// TODO: Is it `operator[]`?
//
// 0x5F6790
const CResRef& CSpellResRefList::Get(UINT& nIndex)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 652
    UTIL_ASSERT(nIndex < m_nCount);

    // __FILE__: .\Include\InfGame.h
    // __LINE__: 653
    UTIL_ASSERT(m_pList != NULL);

    return m_pList[nIndex];
}

// -----------------------------------------------------------------------------

// 0x59EB90
CSpellList::CSpellList()
{
    m_pList = NULL;
    m_nCount = 0;
}

// 0x59EBA0
CSpellList::~CSpellList()
{
    if (m_pList != NULL) {
        delete m_pList;
    }
    m_nCount = 0;
}

// 0x5CA1B0
void CSpellList::Load(const std::vector<CSpellListEntry>& spells)
{
    if (spells.empty()) {
        return;
    }

    size_t size = spells.size();
    if (size == 0) {
        return;
    }

    m_pList = new CSpellListEntry[size];

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23430
    UTIL_ASSERT(m_pList != NULL);

    for (size_t index = 0; index < size; index++) {
        m_pList[index] = spells[index];
    }

    m_nCount = size;
}

// 0x5CA250
bool CSpellList::GetSpellLevel(const UINT& nID, UINT& nIndex)
{
    // NOTE: The exe never reads `m_nLevel` here. The out value is an index
    // into the list (the caller uses it as `m_pList[nIndex]`), but it is only
    // stored for entries that do not match, so on a match at position `k` it
    // is `k - 1` (or 0 for `k == 0`).
    nIndex = 0;

    for (UINT nCurrent = 0; nCurrent < m_nCount; nCurrent++) {
        if (m_pList[nCurrent].m_nID == nID) {
            return true;
        }

        nIndex = nCurrent;
    }

    nIndex = 0;
    return false;
}

// TODO: Is it `operator[]`?
//
// NOTE: Inlined.
UINT CSpellList::Get(UINT nIndex)
{
    // __FILE__: .\Include\InfGame.h
    // __LINE__: 724
    UTIL_ASSERT(nIndex < m_nCount);

    // __FILE__: .\Include\InfGame.h
    // __LINE__: 725
    UTIL_ASSERT(m_pList != NULL);

    return m_pList[nIndex].m_nID;
}

// -----------------------------------------------------------------------------

// 0x59EBD0
CInfGameUnknownEntryList::~CInfGameUnknownEntryList()
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        POSITION posOld = pos;
        CString* node = GetNext(pos);
        if (node != NULL) {
            delete node;
        }
        RemoveAt(posOld);
    }
}

// -----------------------------------------------------------------------------

// 0x59EB60
CGroupedSpellList::CGroupedSpellList()
{
    m_nHighestLevel = 0;
}

// 0x59ECA0
CGroupedSpellList::~CGroupedSpellList()
{
}

// FIXME: `nClassColumn` should not be reference.
// FIXME: `nResRefColumn` should not be reference.
//
// 0x5CA2A0
void CGroupedSpellList::Load(const C2DArray& Table, const INT& nClassColumn, const INT& nResRefColumn, CSpellResRefList* pSpells)
{
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23495
    UTIL_ASSERT(Table.GetWidth() > 0);

    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23496
    UTIL_ASSERT(Table.GetHeight() > 0);

    // FIXME: Uses unsigned compare.
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23497
    UTIL_ASSERT(nClassColumn < Table.GetWidth());

    // FIXME: Uses unsigned compare.
    // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
    // __LINE__: 23498
    UTIL_ASSERT(nResRefColumn < Table.GetWidth());

    std::vector<CSpellListEntry> lists[9];

    for (int index = 0; index < Table.GetHeight(); index++) {
        int nLevel = atol(Table.GetAt(CPoint(nClassColumn, index)));
        UINT nID = index;

        // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
        // __LINE__: 23509
        UTIL_ASSERT(nLevel <= CSPELLLIST_MAX_LEVELS);

        CString sName = Table.GetAt(CPoint(nResRefColumn, index));
        CResRef resRef(sName);

        sName = sName.Left(strlen("**"));
        if (nLevel > 0 && sName != "**") {
            if (pSpells != NULL) {
                bool result = pSpells->Find(resRef, nID);

                // __FILE__: C:\Projects\Icewind2\src\Baldur\InfGame.cpp
                // __LINE__: 23524
                UTIL_ASSERT(result == true);
            }

            CSpellListEntry entry;
            entry.m_nID = nID;
            entry.m_nLevel = nLevel;
            lists[nLevel - 1].push_back(entry);
        }
    }

    // NOTE: Unsigned compare, implying `m_nHighestLevel` is also unsigned.
    for (UINT nLevel = 1; nLevel <= CSPELLLIST_MAX_LEVELS; nLevel++) {
        if (lists[nLevel - 1].size() != 0) {
            m_lists[nLevel - 1].Load(lists[nLevel - 1]);

            if (nLevel > m_nHighestLevel) {
                m_nHighestLevel = nLevel;
            }
        }
    }
}

// -----------------------------------------------------------------------------

// 0x5C7AC0
BYTE* CGameFile::GetData()
{
    if (pRes != NULL) {
        return reinterpret_cast<BYTE*>(pRes->Demand());
    } else {
        return NULL;
    }
}

// 0x5C7AD0
DWORD CGameFile::GetDataSize()
{
    if (pRes == NULL) {
        return 0;
    }

    if (pRes->Demand() == NULL) {
        return 0;
    }

    DWORD nSize = pRes->m_nResSizeActual;

    pRes->Release();

    return nSize;
}

// 0x5C7B00
void CGameFile::ReleaseData()
{
    if (pRes != NULL) {
        pRes->Release();
    }
}

// NOTE: This is a `CGameArea` method; it lives here only to keep clear of
// concurrent edits to `CGameArea.cpp`. It should move there.
//
// 0x46FEA0
BOOLEAN CGameArea::CheckRestEncounter()
{
    if (m_headerRestEncounter.m_activated == 0) {
        return FALSE;
    }

    BYTE nTry = 0;
    ULONG nOffset = 0;

    do {
        ULONG nTimeOfDay = (m_pGame->m_worldTime.m_gameTime + nOffset) % CTimerWorld::TIMESCALE_MSEC_PER_DAY;
        if (nTimeOfDay >= CTimerWorld::TIME_NIGHT || nTimeOfDay < CTimerWorld::TIME_DAWN) {
            if (rand() % 100 <= m_headerRestEncounter.m_probNight) {
                break;
            }
        } else {
            if (rand() % 100 <= m_headerRestEncounter.m_probDay) {
                break;
            }
        }

        nOffset += CTimerWorld::TIMESCALE_MSEC_PER_HOUR;
        nTry++;
    } while (nTry < 1);

    if (nTry == 1) {
        return FALSE;
    }

    if (m_headerRestEncounter.m_randomCreatureNum == 0) {
        return FALSE;
    }

    if (m_headerRestEncounter.m_maxTypeNum == 0) {
        return FALSE;
    }

    // NOTE: The original tests the divisor against zero.
    LONG nHour = CTimerWorld::TIMESCALE_MSEC_PER_HOUR;
    nOffset += (nHour != 0) ? rand() % nHour : 0;

    m_pGame->m_worldTime.AddCurrentTime(nOffset);
    m_pGame->FeedBack(CInfGame::FEEDBACK_TIMEPASSEDREST, nOffset, TRUE);

    CCreatureFile creatureData;
    SHORT nBestStep = 0;
    BYTE nTotalLevel = 0;
    CPoint ptProtagonist;
    LONG nCharacterId;

    LONG nProtagonist = g_pBaldurChitin->GetObjectGame()->GetProtagonist();

    for (SHORT nPortrait = 0; nPortrait < m_pGame->m_nCharacters; nPortrait++) {
        CGameSprite* pSprite;

        nCharacterId = m_pGame->GetCharacterId(nPortrait);

        BYTE rc = m_pGame->GetObjectArray()->GetShare(nCharacterId,
            CGameObjectArray::THREAD_ASYNCH,
            reinterpret_cast<CGameObject**>(&pSprite),
            INFINITE);
        if (rc == CGameObjectArray::SUCCESS) {
            if (pSprite->GetId() == nProtagonist) {
                ptProtagonist = pSprite->GetPos();
            }

            nTotalLevel += pSprite->GetDerivedStats()->m_nLevel;

            m_pGame->GetObjectArray()->ReleaseShare(nCharacterId,
                CGameObjectArray::THREAD_ASYNCH,
                INFINITE);
        }
    }

    LONG nDropped = 0;
    SHORT nRange = (m_visibility.field_E * CVisibilityMap::SQUARE_SIZEX / CPathSearch::GRID_SQUARE_SIZEX) / 2;

    CPoint ptGrid;
    ptGrid.x = ptProtagonist.x / CPathSearch::GRID_SQUARE_SIZEX;
    ptGrid.y = ptProtagonist.y / CPathSearch::GRID_SQUARE_SIZEY;

    CPoint ptBest = ptGrid;
    BYTE nBestDir = 0;

    // Look for the direction with the longest visible walkable line.
    for (BYTE nDir = 0; nDir < 9; nDir++) {
        if (nDir == 4) {
            continue;
        }

        CPoint pt = ptGrid;
        SHORT nStep = 0;

        if (nRange > 0) {
            for (;;) {
                pt.x += nDir % 3 - 1;
                pt.y += nDir / 3 - 1;

                if (pt.x >= 0
                    && pt.x < m_search.m_GridSquareDimensions.cx
                    && pt.y >= 0
                    && pt.y < m_search.m_GridSquareDimensions.cy) {
                    SHORT nTableIndex;
                    if (m_search.GetLOSCost(pt,
                            CGameObject::DEFAULT_VISIBLE_TERRAIN_TABLE,
                            nTableIndex,
                            FALSE)
                        != CPathSearch::COST_IMPASSABLE) {
                        // NOTE: Inlined tile lookup (like
                        // `CVisibilityMap::PointToTile` but with a 16-bit
                        // row multiply).
                        SHORT nRow = static_cast<SHORT>((pt.y * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2) / CVisibilityMap::SQUARE_SIZEY * m_visibility.m_nWidth);
                        SHORT nTile = static_cast<SHORT>(nRow + (pt.x * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2) / CVisibilityMap::SQUARE_SIZEX);

                        if (m_visibility.IsTileVisible(nTile)) {
                            nStep++;
                            if (nStep < nRange) {
                                continue;
                            }
                            break;
                        }
                    }

                    if (nStep > nBestStep
                        || (nStep == nBestStep && rand() % 7 == 0)) {
                        nBestStep = nStep;
                        ptBest = pt;
                        nBestDir = nDir;
                    }
                } else {
                    pt.x += 1 - nDir % 3;
                    pt.y += 1 - nDir / 3;
                    nStep--;

                    if (nStep > nBestStep
                        || (nStep == nBestStep && rand() % 7 == 0)) {
                        nBestStep = nStep;
                        ptBest = pt;
                        nBestDir = nDir;
                    }
                }
                break;
            }
        }

        if (nStep == nRange) {
            if (nStep > nBestStep || rand() % 7 == 0) {
                nBestStep = nStep;
                ptBest = pt;
                nBestDir = nDir;
            }
        }
    }

    BYTE nRoll = rand() % 10;

    INT nCreatureIndex;
    if (m_headerRestEncounter.m_randomCreatureNum == 0) {
        nCreatureIndex = 0;
    } else {
        nCreatureIndex = rand() % m_headerRestEncounter.m_randomCreatureNum;
    }

    CPoint* pOffsets = NULL;

    INT nCount = m_headerRestEncounter.m_maxTypeNum + (rand() % 5 - 2);
    if (nCount < 1) {
        nCount = 1;
    }

    if (g_pBaldurChitin->GetObjectGame()->m_cOptions.m_nNightmareMode == 1) {
        nCount++;
    }

    if (nCount > 0) {
        BYTE nIndex = 0;
        do {
            RESREF* pResRef = &m_headerRestEncounter.m_randomCreature[nCreatureIndex];

            // NOTE: Inlined `CResHelper::SetResRef`.
            creatureData.SetResRef(CResRef(reinterpret_cast<BYTE*>(pResRef)), TRUE, TRUE);

            CGameSprite* pSprite = new CGameSprite(creatureData.GetData(),
                creatureData.GetDataSize(),
                0,
                1,
                m_headerRestEncounter.m_lifeSpan * CTimerWorld::TIMESCALE_MSEC_PER_SEC + m_pGame->m_worldTime.m_gameTime,
                m_headerRestEncounter.m_huntingRange,
                m_headerRestEncounter.m_followRange,
                0x7FFFFFFF,
                ptProtagonist,
                0);

            pSprite->SetResRef(creatureData.GetResRef().GetResRefStr());

            creatureData.ReleaseData();

            if (pSprite != NULL) {
                CString sResRef;
                sResRef = reinterpret_cast<char*>(pResRef);
                pSprite->SetResRef(sResRef);

                INT nAttempt;
                for (nAttempt = 0; nAttempt < 5; nAttempt++) {
                    CPoint ptTarget;

                    if (nRoll != 0) {
                        if (nIndex == 0) {
                            SHORT nFormation = rand() % 9 + 1;
                            pOffsets = m_pGame->m_group.GetOffsets(nFormation,
                                (pSprite->GetDirection(ptProtagonist) * 360 / 16) % 360,
                                TRUE);
                        }

                        CPoint* pOffset = &pOffsets[nIndex % CAIGroup::FORMATION_MAX_SIZE];
                        ptTarget.x = pOffset->x / CAIGroup::OFFSET_MULTIPLIER + ptBest.x;
                        ptTarget.y = pOffset->y / CAIGroup::OFFSET_MULTIPLIER + ptBest.y;

                        ptTarget = m_search.GetNearestOpenSquare(ptTarget,
                            pSprite->GetTerrainTable(),
                            pSprite->m_animation.GetPersonalSpace(),
                            -1);
                    } else {
                        ptTarget = m_search.GetNearestOpenSquare(ptBest,
                            pSprite->GetTerrainTable(),
                            pSprite->m_animation.GetPersonalSpace(),
                            -1);

                        if (rand() % 2 == 0) {
                            ptBest.x += 1 - nBestDir % 3;
                            ptBest.y += 1 - nBestDir / 3;
                        }
                    }

                    ptTarget.x = ptTarget.x * CPathSearch::GRID_SQUARE_SIZEX + CPathSearch::GRID_SQUARE_SIZEX / 2;
                    ptTarget.y = ptTarget.y * CPathSearch::GRID_SQUARE_SIZEY + CPathSearch::GRID_SQUARE_SIZEY / 2;

                    SHORT nRow = static_cast<SHORT>(ptTarget.y / CVisibilityMap::SQUARE_SIZEY * m_visibility.m_nWidth);
                    SHORT nTile = static_cast<SHORT>(nRow + ptTarget.x / CVisibilityMap::SQUARE_SIZEX);

                    if (nTile < m_visibility.m_nWidth * m_visibility.m_nHeight) {
                        pSprite->AddToArea(this, ptTarget, 0, pSprite->m_animation.GetListType());
                        pSprite->SetFacing(pSprite->GetDirection(ptProtagonist));
                        break;
                    }
                }

                if (nAttempt >= 5) {
                    nDropped++;
                }
            } else {
                // __FILE__: C:\Projects\Icewind2\src\Baldur\CGameArea.cpp
                // __LINE__: 3611
                UTIL_ASSERT(FALSE);
            }

            nIndex++;
        } while (nIndex < nCount);

        if (nDropped > 0) {
            char* szMessage = new char[200];
            if (szMessage != NULL) {
                sprintf(szMessage, "Spawned creatures dropped due to invalid positions: %d of %d\n", nDropped, nCount);
                delete[] szMessage;
            }
        }

        if (pOffsets != NULL) {
            delete[] pOffsets;
        }
    }

    g_pBaldurChitin->GetBaldurMessage()->DisplayTextRef(-1,
        m_headerRestEncounter.m_randomCreatureString[nCreatureIndex],
        RGB(0, 0, 0),
        RGB(165, 144, 5),
        -1,
        nCharacterId,
        nCharacterId);

    return TRUE;
}
