#include "test.h"

#include "CAICondition.h"
#include "CAIConditionResponse.h"
#include "CAIResponse.h"
#include "CAIScript.h"
#include "CAITrigger.h"
#include "CAIUtil.h"
#include "CGameAIBase.h"

// Compiled BCS text in the layout of AR1000.BCS's first block. The second
// trigger is `True()` (with made-up parameters) instead of `Global(...)` so
// it can be evaluated without a running game:
//
//   IF
//     OnCreation()
//     True()
//   THEN
//     RESPONSE #100
//       action 254 with actee "Reig" and parameters 3, [5.6], 7, 8, "str1", "str2"
//       action 30
//   END
#define OB_EMPTY "0 0 0 0 0 0 0 0 0 0 0 0 0 [-1.-1.-1.-1] \"\" 0 0 OB\n"
#define OB_REIG "0 0 0 0 0 0 0 0 0 0 0 0 0 [-1.-1.-1.-1] \"Reig\" 0 0 OB\n"

static const char* const kConditionResponse =
    "CR\n"
    "CO\n"
    "TR\n"
    "54 0 0 0 0 \"\" \"\" OB\n" OB_EMPTY
    "TR\n"
    "TR\n"
    "16419 7 0 11 12 \"MYAREAHEALED_REIG\" \"two\" OB\n" OB_EMPTY
    "TR\n"
    "CO\n"
    "RS\n"
    "RE\n"
    "100AC\n"
    "254OB\n" OB_EMPTY
    "OB\n" OB_REIG
    "OB\n" OB_EMPTY
    "3 5 6 7 8\"str1\" \"str2\" AC\n"
    "AC\n"
    "30OB\n" OB_EMPTY
    "OB\n" OB_EMPTY
    "OB\n" OB_EMPTY
    "0 0 0 0 0\"\" \"\" AC\n"
    "RE\n"
    "RS\n"
    "CR\n";

static CString Body(const char* text)
{
    // `CAIScript::Read` hands each `CAIConditionResponse` the text between two
    // "CR\n" markers.
    CString s(text);
    return CAIUtil::ReadBetween(s, CString("CR\n"));
}

TEST_CASE("CAIUtil::ReadBetween and ReadTo")
{
    CString line("abcXYdefXYghi");
    CHECK(CAIUtil::ReadBetween(line, CString("XY")) == "def");
    CHECK(line == "ghi");

    CString line2("12OB\nrest");
    CHECK(CAIUtil::ReadTo(line2, CString("OB\n"), FALSE) == "12");
    CHECK(line2 == "12OB\nrest");
    CHECK(CAIUtil::ReadTo(line2, CString("OB\n"), TRUE) == "12");
    CHECK(line2 == "rest");

    // Not found: the original returns an empty string, not the line.
    CString line3("no marker here");
    CHECK(CAIUtil::ReadTo(line3, CString("OB\n"), FALSE) == "");
}

TEST_CASE("BCS condition-response parses triggers, responses and actions")
{
    CAIConditionResponse cr;
    cr.Read(Body(kConditionResponse));

    REQUIRE(cr.m_condition.m_triggerList.GetCount() == 2);

    POSITION pos = cr.m_condition.m_triggerList.GetHeadPosition();
    CAITrigger* t1 = cr.m_condition.m_triggerList.GetNext(pos);
    CAITrigger* t2 = cr.m_condition.m_triggerList.GetNext(pos);

    CHECK(t1->m_triggerID == CAITRIGGER_ONCREATION);
    CHECK(t1->m_specificID == 0);
    CHECK(t1->m_flags == 0);
    CHECK(t1->m_triggerCause.m_nInstance == -1);
    CHECK(t1->m_triggerCause.m_nLocationType == CAIOBJECTTYPE_LOCATION_TYPE_NONE);

    CHECK(t2->m_triggerID == CAITRIGGER_TRUE);
    CHECK(t2->m_specificID == 7);
    CHECK(t2->m_flags == 0);
    CHECK(t2->m_specific2 == 11);
    CHECK(t2->m_specific3 == 12);
    CHECK(t2->m_string1 == "MYAREAHEALED_REIG");
    CHECK(t2->m_string2 == "two");

    REQUIRE(cr.m_responseSet.m_responseList.GetCount() == 1);
    CHECK(cr.m_responseSet.m_weightTotal == 100);

    CAIResponse* response = cr.m_responseSet.m_responseList.GetHead();
    CHECK(response->m_weight == 100);
    REQUIRE(response->m_actionList.GetCount() == 2);

    POSITION apos = response->m_actionList.GetHeadPosition();
    CAIAction* a1 = response->m_actionList.GetNext(apos);
    CAIAction* a2 = response->m_actionList.GetNext(apos);

    CHECK(a1->m_actionID == 254);
    CHECK(a1->m_acteeID.m_sName == "Reig");
    CHECK(a1->m_actorID.m_sName == "");
    CHECK(a1->m_acteeID2.m_sName == "");
    CHECK(a1->m_specificID == 3);
    CHECK(a1->m_dest.x == 5);
    CHECK(a1->m_dest.y == 6);
    CHECK(a1->m_specificID2 == 7);
    CHECK(a1->m_specificID3 == 8);
    CHECK(a1->m_string1 == "str1");
    CHECK(a1->m_string2 == "str2");

    CHECK(a2->m_actionID == 30);
    CHECK(a2->m_specificID == 0);
}

TEST_CASE("CAIResponse::InListEnd only checks the last action")
{
    CAIResponse response;
    response.Add(CAIAction(CAIAction::CONTINUE, 0, 0, 0));
    response.Add(CAIAction(30, 0, 0, 0));
    CHECK(response.InList(CAIAction::CONTINUE));
    CHECK_FALSE(response.InListEnd(CAIAction::CONTINUE));

    response.Add(CAIAction(CAIAction::CONTINUE, 0, 0, 0));
    CHECK(response.InListEnd(CAIAction::CONTINUE));
}

TEST_CASE("CAIObjectType::Read parses the object specifier")
{
    CAIObjectType type;
    type.Read(CString("2 3 4 5 6 7 8 9 21 1 0 0 0 [10.20.-1.-1] \"Name\" 0 0 "));

    CHECK(type.m_nEnemyAlly == 2);
    CHECK(type.m_nGeneral == 3);
    CHECK(type.m_nRace == 4);
    CHECK(type.m_nClass == 5);
    CHECK(type.m_nSpecific == 6);
    CHECK(type.m_nGender == 7);
    CHECK(type.m_nAlignment == 8);
    CHECK(type.m_nSubRace == 9);
    CHECK(type.m_SpecialCase[0] == 21);
    CHECK(type.m_SpecialCase[1] == 1);
    CHECK(type.m_SpecialCase[2] == 0);
    CHECK(type.m_sName == "Name");
    CHECK(type.m_nInstance == -1);
    CHECK(type.m_nLocationType == CAIOBJECTTYPE_LOCATION_TYPE_POINT);
    CHECK(type.m_ptCenter.x == 10);
    CHECK(type.m_ptCenter.y == 4 * 20 / 3);
}

// The base destructor removes the object from the game, which needs a running
// game, so test objects are deliberately leaked.
static CGameAIBase* MakeCaller()
{
    return new CGameAIBase();
}

TEST_CASE("CAICondition::Hold matches event and status triggers")
{
    CGameAIBase* caller = MakeCaller();

    CAIConditionResponse cr;
    cr.Read(Body(kConditionResponse));

    CTypedPtrList<CPtrList, CAITrigger*> pending;
    CHECK_FALSE(cr.m_condition.Hold(pending, caller));

    CAITrigger onCreation(CAITrigger::ONCREATION, 0);
    pending.AddTail(&onCreation);
    CHECK(cr.m_condition.Hold(pending, caller));

    // A negated `!True()` fails the condition.
    cr.m_condition.m_triggerList.GetTail()->m_flags = 1;
    CHECK_FALSE(cr.m_condition.Hold(pending, caller));

    pending.RemoveAll();
}

TEST_CASE("CAIScript::Find returns the chosen response")
{
    CGameAIBase* caller = MakeCaller();

    CAIConditionResponse cr;
    cr.Read(Body(kConditionResponse));

    CAIScript script;
    script.Add(cr.m_condition, cr.m_responseSet);

    CTypedPtrList<CPtrList, CAITrigger*> pending;
    CAITrigger onCreation(CAITrigger::ONCREATION, 0);
    pending.AddTail(&onCreation);

    CAIResponse* response = script.Find(pending, caller);
    REQUIRE(response != NULL);
    CHECK(response->m_responseSetNum == 1);
    CHECK(response->m_responseNum == 1);
    REQUIRE(response->m_actionList.GetCount() == 2);
    CHECK(response->m_actionList.GetHead()->m_actionID == 254);
    delete response;

    pending.RemoveAll();

    // Without the OnCreation event nothing fires.
    response = script.Find(pending, caller);
    REQUIRE(response != NULL);
    CHECK(response->m_actionList.GetCount() == 0);
    delete response;
}
