/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Instance_Naxxramas
SD%Complete: 
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "scriptPCH.h"
#include "naxxramas.h"

enum NaxxEvents
{
    EVENT_BIGGLESWORTH_DIED_YELL = 1,
    EVENT_THADDIUS_SCREAM,
    EVENT_WINGBOSS_DEAD,
    
    EVENT_KT_LK_DIALOGUE_1,
    EVENT_KT_LK_DIALOGUE_2,
    EVENT_KT_LK_DIALOGUE_3,
    EVENT_KT_LK_DIALOGUE_4,
    EVENT_KT_LK_DIALOGUE_5,
    EVENT_KT_LK_DIALOGUE_GATE_OPEN,

    EVENT_SUMMON_FROGGER_WAVE,

    EVENT_4HM_DIALOGUE_1, // Sir Zeliek yells: Invaders! Cease this foolish venture at once! Turn away while you still can!
    EVENT_4HM_DIALOGUE_2, // Lady Blaumeux yells: Come, Zeliek, do not drive them out. Not until we've had our fun!
    EVENT_4HM_DIALOGUE_3, // Highlord Mograine yells: Enough prattling. Let them come. We shall grind their bones to dust.
    EVENT_4HM_DIALOGUE_4, // Lady Blaumeux yells: I do hope they stay long enough for me to... introduce myself.
    EVENT_4HM_DIALOGUE_5, // Sir Zeliek yells: Perhaps they will come to their senses... and run away as fast as they can.
    EVENT_4HM_DIALOGUE_6, // Thane Korth'azz yells: I've heard about enough a' yer snivelin'!Shut your flytrap before I shut it for ye'!
    EVENT_4HM_DIALOGUE_7, // Highlord Mograine yells: Conserve your anger. Harness your rage. You will all have outlets for your frustrations soon enough.

};


instance_naxxramas::instance_naxxramas(Map* pMap) : ScriptedInstance(pMap),
    m_faerlinaHaveGreeted(false),
    m_thaddiusHaveGreeted(false),
    m_horsemenDeathCounter(0),
    m_fChamberCenterX(0.0f),
    m_fChamberCenterY(0.0f),
    m_fChamberCenterZ(0.0f)
{
    Initialize();
}

void instance_naxxramas::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
    m_events.Reset();
    // 2-5min, no idea if it's correct
    m_events.ScheduleEvent(EVENT_THADDIUS_SCREAM, urand(1000 * 60 * 2, 1000 * 60 * 5)); 

    m_events.ScheduleEvent(EVENT_SUMMON_FROGGER_WAVE, Seconds(6));
}

void instance_naxxramas::SetTeleporterVisualState(GameObject* pGO, uint32 uiData)
{
    if (uiData == DONE)
        pGO->SetGoState(GO_STATE_ACTIVE);
    else
        pGO->SetGoState(GO_STATE_READY);
}

void instance_naxxramas::SetTeleporterState(GameObject* pGO, uint32 uiData)
{
    SetTeleporterVisualState(pGO, uiData);
    if (uiData == DONE)
        pGO->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
    else
        pGO->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
}

uint8 instance_naxxramas::GetNumEndbossDead()
{
    uint8 ret = 0;
    if (GetData(TYPE_MAEXXNA) == DONE)
        ++ret;
    if (GetData(TYPE_THADDIUS) == DONE)
        ++ret;
    if (GetData(TYPE_FOUR_HORSEMEN) == DONE)
        ++ret;
    if (GetData(TYPE_LOATHEB) == DONE)
        ++ret;
    return ret;
}

void instance_naxxramas::UpdateBossEntranceDoor(NaxxGOs which, uint32 uiData)
{
    if (GameObject* pGo = GetSingleGameObjectFromStorage(which))
    {
        UpdateBossEntranceDoor(pGo, uiData);
    }
}

void instance_naxxramas::UpdateBossEntranceDoor(GameObject* pGO, uint32 uiData)
{
    if (!pGO)
    {
        sLog.outError("instance_naxxramas::UpdateBossEntranceDoor called with nullptr GO");
        return;
    }
    if (uiData == IN_PROGRESS || uiData == SPECIAL)
        pGO->SetGoState(GO_STATE_READY);
    else
        pGO->SetGoState(GO_STATE_ACTIVE);
}

void instance_naxxramas::UpdateBossGate(NaxxGOs which, uint32 uiData)
{
    if (GameObject* pGo = GetSingleGameObjectFromStorage(which))
    {
        UpdateBossGate(pGo, uiData);
    }
}

void instance_naxxramas::UpdateBossGate(GameObject* pGO, uint32 uiData)
{
    if (!pGO)
    {
        sLog.outError("instance_naxxramas::UpdateBossGate called with nullptr GO");
        return;
    }
    if (uiData == DONE)
        pGO->SetGoState(GO_STATE_ACTIVE);
    else
        pGO->SetGoState(GO_STATE_READY);
}

void instance_naxxramas::UpdateTeleporters(uint32 uiType, uint32 uiData)
{
    // todo: what was the reason behind these? Should they despawn after 30 minutes?
    // DoRespawnGameObject(GO_<WING>_PORTAL, 30 * MINUTE);
    switch (uiType)
    {
    case TYPE_MAEXXNA:
        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_ARAC_EYE_BOSS))
            SetTeleporterVisualState(pGO, uiData);

        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_ARAC_EYE_RAMP))
            SetTeleporterVisualState(pGO, uiData);

        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_ARAC_PORTAL))
            SetTeleporterState(pGO, uiData);
        break;
    case TYPE_THADDIUS:
        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_CONS_EYE_BOSS))
            SetTeleporterVisualState(pGO, uiData);

        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_CONS_EYE_RAMP))
            SetTeleporterVisualState(pGO, uiData);

        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_CONS_PORTAL))
            SetTeleporterState(pGO, uiData);
        break;
    case TYPE_LOATHEB:
        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_PLAG_EYE_BOSS))
            SetTeleporterVisualState(pGO, uiData);

        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_PLAG_EYE_RAMP))
            SetTeleporterVisualState(pGO, uiData);

        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_PLAG_PORTAL))
            SetTeleporterState(pGO, uiData);
        break;
    case TYPE_FOUR_HORSEMEN:
        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_MILI_EYE_BOSS))
            SetTeleporterVisualState(pGO, uiData);

        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_MILI_EYE_RAMP))
            SetTeleporterVisualState(pGO, uiData);

        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_MILI_PORTAL))
            SetTeleporterState(pGO, uiData);
        break;
    default:
        sLog.outError("instance_naxxramas::UpdateTeleporters called with unsupported type %d", uiType);
    }

    if (   m_auiEncounter[TYPE_THADDIUS] == DONE
        && m_auiEncounter[TYPE_LOATHEB] == DONE
        && m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE
        && m_auiEncounter[TYPE_MAEXXNA] == DONE
       )
    {
        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_HUB_PORTAL))
        {
            pGO->SetGoState(GO_STATE_ACTIVE);
        }
    }
    else 
    {
        if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_HUB_PORTAL))
        {
            pGO->SetGoState(GO_STATE_READY);
        }
    }
}

void instance_naxxramas::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_ANUB_REKHAN:
        case NPC_FAERLINA:
        case NPC_MAEXXNA:
        case NPC_PATCHWERK:
        case NPC_GROBBULUS:
        case NPC_GLUTH:
        case NPC_THADDIUS:
        //case NPC_STALAGG:
        //case NPC_FEUGEN:
        case NPC_NOTH:
        case NPC_HEIGAN:
        case NPC_LOATHEB:
        case NPC_RAZUVIOUS:
        case NPC_GOTHIK:
        case NPC_ZELIEK:
        case NPC_THANE:
        case NPC_BLAUMEUX:
        case NPC_MOGRAINE:
        case NPC_SAPPHIRON:
        case NPC_KELTHUZAD:
        case NPC_MR_BIGGLESWORTH:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        

        case NPC_GUARDIAN:
        case NPC_SOLDIER_FROZEN:
        case NPC_SOUL_WEAVER:
        case NPC_UNSTOPPABLE_ABOM:
        case NPC_UNREL_TRAINEE:
        case NPC_UNREL_DEATH_KNIGHT:
        case NPC_UNREL_RIDER:
        case NPC_SPECT_TRAINEE:
        case NPC_SPECT_DEATH_KNIGTH:
        case NPC_SPECT_RIDER:
        case NPC_SPECT_HORSE:
            break;

        case NPC_SUB_BOSS_TRIGGER:
            if (m_auiEncounter[TYPE_GOTHIK] != IN_PROGRESS)
                m_lGothTriggerList.push_back(pCreature->GetGUID());
            break;
        
        // naxxramas acolyte and cultist
        case 15980:
        case 15981:
            pCreature->SetStandState(UNIT_STAND_STATE_KNEEL);
            break;
    }
}

void instance_naxxramas::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_ARAC_ANUB_DOOR:
        case GO_ARAC_ANUB_GATE:
        case GO_ARAC_FAER_WEB:
        case GO_ARAC_FAER_DOOR:
        case GO_ARAC_MAEX_INNER_DOOR:
        case GO_ARAC_MAEX_OUTER_DOOR:
        case GO_PLAG_SLIME01_DOOR:
        case GO_PLAG_SLIME02_DOOR:
        case GO_PLAG_NOTH_ENTRY_DOOR:
        case GO_PLAG_NOTH_EXIT_DOOR:
        case GO_PLAG_HEIG_ENTRY_DOOR:
        case GO_PLAG_HEIG_EXIT_DOOR:
        case GO_PLAG_HEIG_OLD_EXIT_DOOR:
        case GO_PLAG_LOAT_DOOR:
        case GO_MILI_GOTH_ENTRY_GATE:
        case GO_MILI_GOTH_EXIT_GATE:
        case GO_MILI_GOTH_COMBAT_GATE:
        case GO_MILI_HORSEMEN_DOOR:
        case GO_CHEST_HORSEMEN_NORM:
        case GO_CHEST_HORSEMEN_HERO:
        case GO_CONS_PATH_EXIT_DOOR:
        case GO_CONS_GLUT_EXIT_DOOR:
        case GO_CONS_THAD_DOOR:
        case GO_KELTHUZAD_WATERFALL_DOOR:
        case GO_KELTHUZAD_DOOR:
        case GO_ARAC_EYE_RAMP:
        case GO_PLAG_EYE_RAMP:
        case GO_MILI_EYE_RAMP:
        case GO_CONS_EYE_RAMP:
        case GO_ARAC_PORTAL:
        case GO_PLAG_PORTAL:
        case GO_MILI_PORTAL:
        case GO_CONS_PORTAL:
        case GO_ARAC_EYE_BOSS:
        case GO_PLAG_EYE_BOSS:
        case GO_MILI_EYE_BOSS:
        case GO_CONS_EYE_BOSS:
        case GO_KT_WINDOW_1:
        case GO_KT_WINDOW_2:
        case GO_KT_WINDOW_3:
        case GO_KT_WINDOW_4:
        case GO_CONS_NOX_TESLA_FEUGEN:
        case GO_CONS_NOX_TESLA_STALAGG:
        case GO_HUB_PORTAL:
        case GO_SAPPHIRON_SPAWN:
            m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
            break;
    }

    if (pGo->GetGoType() == GAMEOBJECT_TYPE_TRAP)
    {
        uint32 uiGoEntry = pGo->GetEntry();

        if ((uiGoEntry >= 181517 && uiGoEntry <= 181524) || uiGoEntry == 181678)
            m_alHeiganTrapGuids[0].push_back(pGo->GetObjectGuid());
        else if ((uiGoEntry >= 181510 && uiGoEntry <= 181516) || (uiGoEntry >= 181525 && uiGoEntry <= 181531) || uiGoEntry == 181533 || uiGoEntry == 181676)
            m_alHeiganTrapGuids[1].push_back(pGo->GetObjectGuid());
        else if ((uiGoEntry >= 181534 && uiGoEntry <= 181544) || uiGoEntry == 181532 || uiGoEntry == 181677)
            m_alHeiganTrapGuids[2].push_back(pGo->GetObjectGuid());
        else if ((uiGoEntry >= 181545 && uiGoEntry <= 181552) || uiGoEntry == 181695)
            m_alHeiganTrapGuids[3].push_back(pGo->GetObjectGuid());
    }

    switch (pGo->GetEntry())
    {
        // Arac wing
        case GO_ARAC_ANUB_DOOR:
            // opening probably linked to trash
            UpdateBossEntranceDoor(pGo, m_auiEncounter[TYPE_ANUB_REKHAN]);
            break;
        case GO_ARAC_ANUB_GATE:
            UpdateBossGate(pGo, m_auiEncounter[TYPE_ANUB_REKHAN]);
            break;
        case GO_ARAC_FAER_WEB:
            pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ARAC_FAER_DOOR:
        case GO_ARAC_MAEX_OUTER_DOOR:
            UpdateBossGate(pGo, m_auiEncounter[TYPE_FAERLINA]);
            break;
        case GO_ARAC_MAEX_INNER_DOOR:
            pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        
            
        // Plague wing
        case GO_PLAG_NOTH_ENTRY_DOOR:
            UpdateBossEntranceDoor(pGo, m_auiEncounter[TYPE_NOTH]);
            break;
        case GO_PLAG_NOTH_EXIT_DOOR:
            UpdateBossGate(pGo, m_auiEncounter[TYPE_NOTH]);
            break;
        case GO_PLAG_HEIG_ENTRY_DOOR:
            // todo: this gate does not close instantly on pull, but rather a while later
            // https://www.youtube.com/watch?v=kCnk6lwi2ug
            // This might be the case for more than this boss-entrance door.
            UpdateBossEntranceDoor(pGo, m_auiEncounter[TYPE_HEIGAN]);
            break;
        case GO_PLAG_HEIG_EXIT_DOOR:
        case GO_PLAG_HEIG_OLD_EXIT_DOOR:
        case GO_PLAG_LOAT_DOOR:
            UpdateBossGate(pGo, m_auiEncounter[TYPE_HEIGAN]);
            break;
        
        // -- Millitary wing
        case GO_MILI_GOTH_ENTRY_GATE:
            UpdateBossEntranceDoor(pGo, m_auiEncounter[TYPE_GOTHIK]);
            break;
        case GO_MILI_GOTH_EXIT_GATE:
        case GO_MILI_HORSEMEN_DOOR:
            UpdateBossGate(pGo, m_auiEncounter[TYPE_GOTHIK]);
            break;
        case GO_MILI_GOTH_COMBAT_GATE:
            pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CHEST_HORSEMEN_NORM:
            //todo: anything to be done?
            break;

        
        // -- Cons wing doors 
        case GO_CONS_PATH_EXIT_DOOR:
            UpdateBossGate(pGo, m_auiEncounter[TYPE_PATCHWERK]);
            break;
        case GO_CONS_GLUT_EXIT_DOOR:
        case GO_CONS_THAD_DOOR:
            UpdateBossGate(pGo, m_auiEncounter[TYPE_GLUTH]);
            break;


        // -- Frostwyrm lair 
        case GO_KELTHUZAD_WATERFALL_DOOR:
            UpdateBossGate(pGo, m_auiEncounter[TYPE_SAPPHIRON]);
            break;
        case GO_KELTHUZAD_DOOR:
            // todo: At least in wotlk an RP event started when players came close
            // to this door, and only after the RP the door opened (aka not when sapphiron dies).
            // unknown if this is the case in vanilla.
            UpdateBossGate(pGo, m_auiEncounter[TYPE_SAPPHIRON]);
            break;


        // --- Teleporters visual thing
        case GO_ARAC_EYE_RAMP:
        case GO_ARAC_EYE_BOSS:
            SetTeleporterVisualState(pGo, m_auiEncounter[TYPE_MAEXXNA]);
            break;
        case GO_PLAG_EYE_RAMP:
        case GO_PLAG_EYE_BOSS:
            SetTeleporterVisualState(pGo, m_auiEncounter[TYPE_LOATHEB]);
            break;
        case GO_MILI_EYE_RAMP:
        case GO_MILI_EYE_BOSS:
            SetTeleporterVisualState(pGo, m_auiEncounter[TYPE_FOUR_HORSEMEN]);
            break;
        case GO_CONS_EYE_RAMP:
        case GO_CONS_EYE_BOSS:
            SetTeleporterVisualState(pGo, m_auiEncounter[TYPE_THADDIUS]);
            break;

        // --- Actual teleporters
        case GO_ARAC_PORTAL:
            SetTeleporterState(pGo, m_auiEncounter[TYPE_MAEXXNA]);
            break;
        case GO_PLAG_PORTAL:
            SetTeleporterState(pGo, m_auiEncounter[TYPE_LOATHEB]);
            break;
        case GO_MILI_PORTAL:
            SetTeleporterState(pGo, m_auiEncounter[TYPE_FOUR_HORSEMEN]);
            break;
        case GO_CONS_PORTAL:
            SetTeleporterState(pGo, m_auiEncounter[TYPE_THADDIUS]);
            break;

        case GO_KT_WINDOW_1:
        case GO_KT_WINDOW_2:
        case GO_KT_WINDOW_3:
        case GO_KT_WINDOW_4:
            if (m_auiEncounter[TYPE_KELTHUZAD] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            else
                pGo->SetGoState(GO_STATE_READY);
            break;

        case GO_CONS_NOX_TESLA_FEUGEN:
        case GO_CONS_NOX_TESLA_STALAGG:
            if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                pGo->SetGoState(GO_STATE_READY);
            else
                pGo->SetGoState(GO_STATE_ACTIVE);

    }
}

bool instance_naxxramas::IsEncounterInProgress()
{
    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;

    return false;
}

void instance_naxxramas::SetData(uint32 uiType, uint32 uiData)
{
    ASSERT(this)

    switch (uiType)
    {
        case TYPE_ANUB_REKHAN:
            m_auiEncounter[uiType] = uiData;
            UpdateBossGate(GO_ARAC_ANUB_GATE, uiData);
            // opening probably linked to trash
            UpdateBossEntranceDoor(GO_ARAC_ANUB_DOOR, uiData);

            break;
        case TYPE_FAERLINA:
            m_auiEncounter[uiType] = uiData;
            UpdateBossEntranceDoor(GO_ARAC_FAER_WEB, uiData);
            UpdateBossGate(GO_ARAC_FAER_DOOR, uiData);
            
            // This one could potentially be linked to trash rather than Faerlina?
            // Though probably not, because you can get to maexxna without killing faerlina
            UpdateBossGate(GO_ARAC_MAEX_OUTER_DOOR, uiData);
            break;
        case TYPE_MAEXXNA:
            if (uiData == DONE)
                m_events.ScheduleEvent(EVENT_WINGBOSS_DEAD, 10000);
            m_auiEncounter[uiType] = uiData;
            UpdateBossEntranceDoor(GO_ARAC_MAEX_INNER_DOOR, uiData);
            UpdateTeleporters(uiType, uiData);
            break;
        case TYPE_NOTH:
            m_auiEncounter[uiType] = uiData;
            UpdateBossEntranceDoor(GO_PLAG_NOTH_ENTRY_DOOR, uiData);
            UpdateBossGate(GO_PLAG_NOTH_EXIT_DOOR, uiData);
            
            // Potentially open when some trash in room before is killed instead?
            UpdateBossGate(GO_PLAG_HEIG_ENTRY_DOOR, uiData);
            break;
        case TYPE_HEIGAN:
            m_auiEncounter[uiType] = uiData;
            // entry door is controlled by boss script
            UpdateBossGate(GO_PLAG_LOAT_DOOR, uiData);
            break;
        case TYPE_LOATHEB:
            if (uiData == DONE)
                m_events.ScheduleEvent(EVENT_WINGBOSS_DEAD, 10000);
            m_auiEncounter[uiType] = uiData;
            UpdateBossEntranceDoor(GO_PLAG_LOAT_DOOR, uiData);
            UpdateTeleporters(uiType, uiData);
            break;
        case TYPE_RAZUVIOUS:
            m_auiEncounter[uiType] = uiData;
            UpdateBossGate(GO_MILI_GOTH_ENTRY_GATE, uiData);
            break;
        case TYPE_GOTHIK:
            m_auiEncounter[uiType] = uiData;
            UpdateBossEntranceDoor(GO_MILI_GOTH_ENTRY_GATE, uiData);
            UpdateBossGate(GO_MILI_GOTH_EXIT_GATE, uiData);
            UpdateBossGate(GO_MILI_HORSEMEN_DOOR, uiData);
            if (GameObject* pGO = GetSingleGameObjectFromStorage(GO_MILI_GOTH_COMBAT_GATE))
            {
                switch (uiData)
                {
                case IN_PROGRESS:
                    pGO->SetGoState(GO_STATE_READY);
                    break;
                case SPECIAL:
                    pGO->SetGoState(GO_STATE_ACTIVE);
                    break;
                case FAIL:
                    //if (m_auiEncounter[TYPE_GOTHIK] == IN_PROGRESS)
                    pGO->SetGoState(GO_STATE_ACTIVE);
                    break;
                case DONE:
                    pGO->SetGoState(GO_STATE_ACTIVE);
                    m_events.ScheduleEvent(EVENT_4HM_DIALOGUE_1, Seconds(10)); // todo: don't know if it should trigger here or when opening 4hm door
                    break;
                }
            }
            break;
        case TYPE_FOUR_HORSEMEN:
            if(uiData == DONE)
                m_events.ScheduleEvent(EVENT_WINGBOSS_DEAD, 10000);
            m_auiEncounter[uiType] = uiData;
            UpdateBossEntranceDoor(GO_MILI_HORSEMEN_DOOR, uiData);
            UpdateTeleporters(uiType, uiData);
            if (uiData == SPECIAL)
            {
                ++m_horsemenDeathCounter;
                if (m_horsemenDeathCounter >= 4)
                {
                    SetData(TYPE_FOUR_HORSEMEN, DONE);
                }
            }
            else if(uiData == FAIL)
            {
                m_horsemenDeathCounter = 0;
            }
            else if (uiData == DONE)
            {
                // todo: (gemt) i commented this out. Is it to despawn chest after 30 minutes?
                // DoRespawnGameObject(m_uiHorsemenChestGUID, 30 * MINUTE);
            }
            
                
            break;
        case TYPE_PATCHWERK:
            m_auiEncounter[uiType] = uiData;
            UpdateBossGate(GO_CONS_PATH_EXIT_DOOR, uiData);
            break;
        case TYPE_GROBBULUS:
            // no doors here?
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_GLUTH:
            m_auiEncounter[uiType] = uiData;
            UpdateBossGate(GO_CONS_GLUT_EXIT_DOOR, uiData);
            
            // Should this open another way perhaps? Same issue as maexxna outer door.
            UpdateBossGate(GO_CONS_THAD_DOOR, uiData);
            break;
        case TYPE_THADDIUS:
            // Only set the same state once
            if (uiData == m_auiEncounter[uiType])
                break;

            if (uiData == DONE)
                m_events.ScheduleEvent(EVENT_WINGBOSS_DEAD, 10000);
            m_auiEncounter[uiType] = uiData;
            UpdateBossEntranceDoor(GO_CONS_THAD_DOOR, uiData);
            
            UpdateTeleporters(uiType, uiData);
            break;
        case TYPE_SAPPHIRON:
            if(uiData == DONE)
                m_events.ScheduleEvent(EVENT_KT_LK_DIALOGUE_1, 12000);

            m_auiEncounter[uiType] = uiData;
            UpdateBossGate(GO_KELTHUZAD_WATERFALL_DOOR, uiData);
            // GO_KELTHUZAD_DOOR is opened at the end of EVENT_KT_LK_DIALOGUE
            break;
        case TYPE_KELTHUZAD:
            UpdateBossEntranceDoor(GO_KELTHUZAD_DOOR, uiData);
            switch (uiData) 
            {
                case SPECIAL:
                {
                    Map::PlayerList const& lPlayers = instance->GetPlayers();

                    if (lPlayers.isEmpty())
                        return;

                    bool bCanBegin = true;

                    for (Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                    {
                        if (Player* pPlayer = itr->getSource())
                        {
                            if (!pPlayer->IsWithinDist2d(m_fChamberCenterX, m_fChamberCenterY, 15.0f))
                                bCanBegin = false;
                        }
                    }

                    if (bCanBegin)
                        m_auiEncounter[uiType] = IN_PROGRESS;

                    break;
                }
                case FAIL:
                    m_auiEncounter[uiType] = NOT_STARTED;
                    break;
                default:
                    m_auiEncounter[uiType] = uiData;
                    break;
            }
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        for (int i = 0; i < MAX_ENCOUNTER; ++i)
            saveStream << m_auiEncounter[i] << " ";

        strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_naxxramas::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    for (int i = 0; i < MAX_ENCOUNTER; ++i)
    {
        loadStream >> m_auiEncounter[i];
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }
    if (m_auiEncounter[TYPE_THADDIUS] == SPECIAL)
        m_auiEncounter[TYPE_THADDIUS] = FAIL;

    //todo: at least 4hm might need to be changed from SPECIAL to FAIL/NOT_STARTED as well
    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_naxxramas::GetData(uint32 uiType)
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    sLog.outError("instance_naxxramas::GetData() called with %d as param. %d is MAX_ENCOUNTERS", uiType, MAX_ENCOUNTER);
    return 0;
}

uint64 instance_naxxramas::GetData64(uint32 uiData)
{
    sLog.outBasic("instance_naxxramas::GetData64 called. Not implemented");
    return 0;
}

uint64 instance_naxxramas::GetGOUuid(NaxxGOs which)
{
    auto it = m_mNpcEntryGuidStore.find(which);
    if (it == m_mNpcEntryGuidStore.end())
    {
        sLog.outError("instance_naxxramas::GetGOUuid called with param %d, not found", which);
        return 0;
    }
    return it->second;
}

void instance_naxxramas::SetGothTriggers()
{
    Creature* pGoth = GetSingleCreatureFromStorage(NPC_GOTHIK);

    if (!pGoth)
        return;

    for (std::list<uint64>::iterator itr = m_lGothTriggerList.begin(); itr != m_lGothTriggerList.end(); ++itr)
    {
        if (Creature* pTrigger = instance->GetCreature(*itr))
        {
            GothTrigger pGt;
            pGt.bIsAnchorHigh = (pTrigger->GetPositionZ() >= (pGoth->GetPositionZ() - 5.0f));
            pGt.bIsRightSide = IsInRightSideGothArea(pTrigger);

            m_mGothTriggerMap[pTrigger->GetGUID()] = pGt;
        }
    }
}

Creature* instance_naxxramas::GetClosestAnchorForGoth(Creature* pSource, bool bRightSide)
{
    std::list<Creature* > lList;

    for (auto itr = m_mGothTriggerMap.begin(); itr != m_mGothTriggerMap.end(); ++itr)
    {
        if (!itr->second.bIsAnchorHigh)
            continue;

        if (itr->second.bIsRightSide != bRightSide)
            continue;

        if (Creature* pCreature = instance->GetCreature(itr->first))
            lList.push_back(pCreature);
    }

    if (!lList.empty())
    {
        lList.sort(ObjectDistanceOrder(pSource));
        return lList.front();
    }

    return NULL;
}

void instance_naxxramas::GetGothSummonPointCreatures(std::list<Creature*> &lList, bool bRightSide)
{
    for (UNORDERED_MAP<uint64, GothTrigger>::iterator itr = m_mGothTriggerMap.begin(); itr != m_mGothTriggerMap.end(); ++itr)
    {
        if (itr->second.bIsAnchorHigh)
            continue;

        if (itr->second.bIsRightSide != bRightSide)
            continue;

        if (Creature* pCreature = instance->GetCreature(itr->first))
            lList.push_back(pCreature);
    }
}

bool instance_naxxramas::IsInRightSideGothArea(Unit* pUnit)
{
    if (GameObject* pCombatGate = GetSingleGameObjectFromStorage(GO_MILI_GOTH_COMBAT_GATE))
        return (pCombatGate->GetPositionY() >= pUnit->GetPositionY());

    sLog.outError("left/right side check, Gothik combat area failed.");
    return true;
}

void instance_naxxramas::SetChamberCenterCoords(float fX, float fY, float fZ)
{
    m_fChamberCenterX = fX;
    m_fChamberCenterY = fY;
    m_fChamberCenterZ = fZ;
}

void instance_naxxramas::ToggleKelThuzadWindows(bool setOpen)
{
    for (int i = GO_KT_WINDOW_1; i <= GO_KT_WINDOW_4; i++)
    {
        if (GameObject* pGo = GetSingleGameObjectFromStorage(i))
            pGo->SetGoState(setOpen ? GO_STATE_ACTIVE : GO_STATE_READY);
    }
}

void instance_naxxramas::OnPlayerDeath(Player* p)
{
    if (m_auiEncounter[TYPE_ANUB_REKHAN] == IN_PROGRESS)
    {
        // On player death we spawn 5 scarabs under the player. Since the player
        // can die from falldmg or other sources, anubs script impl of KilledUnit may not
        // be called, thus we need to do it here.
        if (Creature* pAnub = GetSingleCreatureFromStorage(NPC_ANUB_REKHAN))
        {
            //pAnub->AI()->DoCast(p, 29105, true);
            pAnub->SendSpellGo(p, 28864);
            for (int i = 0; i < 5; i++)
            {
                if (Creature* cs = pAnub->SummonCreature(16698, p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), 0,
                    TEMPSUMMON_CORPSE_DESPAWN))
                {
                    cs->SetInCombatWithZone();
                    if (Unit* csTarget = pAnub->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        cs->AI()->AttackStart(csTarget);
                        cs->AddThreat(csTarget, 5000);
                    }
                }
            }
        }
    }
}

void instance_naxxramas::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
    case NPC_MR_BIGGLESWORTH:
        m_events.ScheduleEvent(EVENT_BIGGLESWORTH_DIED_YELL, 1000);
        break;
    }
}

void instance_naxxramas::Update(uint32 diff)
{
    m_events.Update(diff);
    while (auto l_EventId = m_events.ExecuteEvent())
    {
        switch (l_EventId)
        {
        case EVENT_BIGGLESWORTH_DIED_YELL:
            DoOrSimulateScriptTextForThisInstance(KELTHUZAD_SAY_CAT_DIED, NPC_KELTHUZAD);
            break;
        case EVENT_THADDIUS_SCREAM:
            if (m_auiEncounter[TYPE_THADDIUS] != DONE)
            {
                // Still not 100% confirmed that there should be no text in chatlog, but its most likely only the sound
                if (m_auiEncounter[TYPE_THADDIUS] != IN_PROGRESS && m_auiEncounter[TYPE_THADDIUS] != SPECIAL)
                    GetMap()->PlayDirectSoundToMap(urand(8873, 8876));
                    //DoOrSimulateScriptTextForThisInstance(THADDIUS_SAY_SCREAM4 + urand(0, 3), NPC_THADDIUS); 

                m_events.ScheduleEvent(EVENT_THADDIUS_SCREAM, urand(1000 * 60 * 2, 1000 * 60 * 5)); // 2-5min, no idea if it's correct
            }
            break;
        case EVENT_WINGBOSS_DEAD:
            DoOrSimulateScriptTextForThisInstance(KELTHUZAD_SAY_TAUNT1 - GetNumEndbossDead()+1, NPC_KELTHUZAD);
            break;
        case EVENT_KT_LK_DIALOGUE_1:
            DoOrSimulateScriptTextForThisInstance(SAY_SAPP_DIALOG1, NPC_KELTHUZAD);
            m_events.ScheduleEvent(EVENT_KT_LK_DIALOGUE_2, 6000);
            break;
        case EVENT_KT_LK_DIALOGUE_2:
            DoOrSimulateScriptTextForThisInstance(SAY_SAPP_DIALOG2_LICH, NPC_LICH_KING);
            m_events.ScheduleEvent(EVENT_KT_LK_DIALOGUE_3, 18000);
            break;
        case EVENT_KT_LK_DIALOGUE_3:
            DoOrSimulateScriptTextForThisInstance(SAY_SAPP_DIALOG3, NPC_KELTHUZAD);
            m_events.ScheduleEvent(EVENT_KT_LK_DIALOGUE_4, 7000);
            break;
        case EVENT_KT_LK_DIALOGUE_4:
            DoOrSimulateScriptTextForThisInstance(SAY_SAPP_DIALOG4_LICH, NPC_LICH_KING);
            m_events.ScheduleEvent(EVENT_KT_LK_DIALOGUE_5, 9000);
            break;
        case EVENT_KT_LK_DIALOGUE_5:
            DoOrSimulateScriptTextForThisInstance(SAY_SAPP_DIALOG5, NPC_KELTHUZAD);
            m_events.ScheduleEvent(EVENT_KT_LK_DIALOGUE_GATE_OPEN, 7000);
            break;
        case EVENT_KT_LK_DIALOGUE_GATE_OPEN:
            UpdateBossGate(GO_KELTHUZAD_DOOR, DONE);
            break;
        case EVENT_SUMMON_FROGGER_WAVE:
        {

            static constexpr float pos[6][4] = {
            {3128.66f, -3121.27f, 293.341f, 4.73893f},
            {3154.58f, -3126.18f, 293.591f, 4.43020f},
            {3175.28f, -3134.76f, 293.437f, 4.24492f},
            {3129.630f, -3157.652f, 293.32f, 4.73893f},
            {3144.894f, -3159.587f, 293.32f, 4.43020f},
            {3159.510f, -3166.001f, 293.27f, 4.24492f} };
            
            for (int i = 0; i < 3; i++)
            {
                if (Creature* frogger = instance->SummonCreature(16027, pos[i][0], pos[i][1], pos[i][2], pos[i][3], TEMPSUMMON_TIMED_DESPAWN, 13000))
                {
                    frogger->GetMotionMaster()->MovePoint(0, pos[i+3][0], pos[i + 3][1], pos[i + 3][2], pos[i + 3][3]);
                }
            }
            m_events.Repeat(Seconds(6));
            break;
        }
        case EVENT_4HM_DIALOGUE_1:
            DoOrSimulateScriptTextForMap(-1533059, NPC_ZELIEK, GetMap(), GetSingleCreatureFromStorage(16063));
            m_events.ScheduleEvent(EVENT_4HM_DIALOGUE_2, Seconds(7));
            break;
        case EVENT_4HM_DIALOGUE_2:
            DoOrSimulateScriptTextForMap(-1533045, NPC_BLAUMEUX, GetMap(), GetSingleCreatureFromStorage(16065));
            m_events.ScheduleEvent(EVENT_4HM_DIALOGUE_3, Seconds(7));
            break;
        case EVENT_4HM_DIALOGUE_3:
            DoOrSimulateScriptTextForMap(-1533071, NPC_MOGRAINE, GetMap(), GetSingleCreatureFromStorage(16062));
            m_events.ScheduleEvent(EVENT_4HM_DIALOGUE_4, Seconds(7));
            break;
        case EVENT_4HM_DIALOGUE_4:
            DoOrSimulateScriptTextForMap(-1533046, NPC_BLAUMEUX, GetMap(), GetSingleCreatureFromStorage(16065));
            m_events.ScheduleEvent(EVENT_4HM_DIALOGUE_5, Seconds(7));
            break;
        case EVENT_4HM_DIALOGUE_5:
            DoOrSimulateScriptTextForMap(-1533060, NPC_ZELIEK, GetMap(), GetSingleCreatureFromStorage(16063));
            m_events.ScheduleEvent(EVENT_4HM_DIALOGUE_6, Seconds(6));
            break;
        case EVENT_4HM_DIALOGUE_6:
            DoOrSimulateScriptTextForMap(-1533053, NPC_THANE, GetMap(), GetSingleCreatureFromStorage(NPC_THANE));
            m_events.ScheduleEvent(EVENT_4HM_DIALOGUE_7, Seconds(7));
            break;
        case EVENT_4HM_DIALOGUE_7:
            DoOrSimulateScriptTextForMap(-1533072, NPC_MOGRAINE, GetMap(), GetSingleCreatureFromStorage(16062));
            break;
        }
    }
}

InstanceData* GetInstanceData_instance_naxxramas(Map* pMap)
{
    return new instance_naxxramas(pMap);
}

void instance_naxxramas::onNaxxramasAreaTrigger(Player* pPlayer, const AreaTriggerEntry* pAt)
{
    switch (pAt->id)
    {
    case AREATRIGGER_HUB_TO_FROSTWYRM:
        if (   GetData(TYPE_MAEXXNA) == DONE
            && GetData(TYPE_THADDIUS) == DONE
            && GetData(TYPE_LOATHEB) == DONE
            && GetData(TYPE_FOUR_HORSEMEN) == DONE
           )
        {
            pPlayer->TeleportTo(toFrostwyrmTPPos);
        }
        break;
    case AREATRIGGER_KELTHUZAD:
        OnKTAreaTrigger(pAt);
        /*
        if (instance_naxxramas* pInstance = (instance_naxxramas*)pPlayer->GetInstanceData())
        {
            if (pInstance->GetData(TYPE_KELTHUZAD) == NOT_STARTED)
            {
                pInstance->SetData(TYPE_KELTHUZAD, SPECIAL);
                pInstance->SetChamberCenterCoords(pAt->x, pAt->y, pAt->z);
            }
        }
        */
        break;
    case AREATRIGGER_FAERLINA:
        if (!m_faerlinaHaveGreeted)
        {
            m_faerlinaHaveGreeted = true;
            if (Creature* pFaerlina = GetSingleCreatureFromStorage(NPC_FAERLINA))
            {
                if(pFaerlina->isAlive())
                    DoScriptText(SAY_FAERLINA_GREET, pFaerlina);
            }
        }
        break;
    case AREATRIGGER_THADDIUS_ENTRANCE:
        if (!m_thaddiusHaveGreeted)
        {
            m_thaddiusHaveGreeted = true;
            if (Creature* pThaddius = GetSingleCreatureFromStorage(NPC_THADDIUS))
            {
                if (pThaddius->isAlive())
                    DoScriptText(SAY_THADDIUS_GREET, pThaddius);
            }
        }
        break;
    }
}

bool AreaTrigger_at_naxxramas(Player* pPlayer, const AreaTriggerEntry* pAt)
{
    if (pPlayer->isGameMaster() || !pPlayer->isAlive())
        return false;

    if (instance_naxxramas* pInstance = (instance_naxxramas*)pPlayer->GetInstanceData())
    {
        pInstance->onNaxxramasAreaTrigger(pPlayer, pAt);
    }
    return false;
}


struct mob_spiritOfNaxxramasAI : public ScriptedAI
{
    mob_spiritOfNaxxramasAI(Creature* pCreature) 
        : ScriptedAI(pCreature)
    {
        Reset();
    }
    
    ObjectGuid portal;
    uint32 portalTimer;
    uint32 shadowboltVolleyTimer;

    void Reset() override
    {
        portalTimer = 5000;
        shadowboltVolleyTimer = 10000;
    }

    void JustDied(Unit* pKiller) override
    {
        if (Creature* pPortal = m_creature->GetMap()->GetCreature(portal))
        {
            pPortal->ForcedDespawn();
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (portalTimer)
        {
            if (portalTimer < diff)
            {
                // summon portal of shadows
                if (Creature* pCreature = m_creature->SummonCreature(16420, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0,
                    TEMPSUMMON_TIMED_DESPAWN, 120000))
                {
                    m_creature->SendSpellGo(m_creature, 28383); // since we're manually summoning, we also send the visual that we're not using
                    portal = pCreature->GetObjectGuid();
                    pCreature->CastSpell(pCreature, 28384, true); // pCreature casts portal of shadow spell on self
                    portalTimer = 0;
                }
            }
            else
                portalTimer -= diff;
        }

        // casting shadowbolt volley every 10 sec
        if (shadowboltVolleyTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, 28599) == CAST_OK)
            {
                shadowboltVolleyTimer = 10000;
            }
        }
        else
            shadowboltVolleyTimer -= diff;

        DoMeleeAttackIfReady();
    }

};

struct mob_naxxramasGarboyleAI : public ScriptedAI
{
    mob_naxxramasGarboyleAI(Creature* pCreature)
        : ScriptedAI(pCreature)
    {
        Reset();
        goStoneform();
    }
    
    void goStoneform()
    {
        if (m_creature->GetDefaultMovementType() == IDLE_MOTION_TYPE && m_creature->GetEntry() == 16168)
        {
            m_creature->CastSpell(m_creature, 29154, true);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }
    }

    uint32 acidVolleyTimer;
    void Reset() override
    {
        acidVolleyTimer = 4000;
    }
    
    void JustReachedHome() override
    {
        goStoneform();
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE))
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER
                && !m_creature->isInCombat()
                && m_creature->IsWithinDistInMap(pWho, 10.0f)
                && !pWho->HasAuraType(SPELL_AURA_FEIGN_DEATH)
                && m_creature->IsWithinLOSInMap(pWho))
            {
                AttackStart(pWho);
            }
        }
        else
        {
            ScriptedAI::MoveInLineOfSight(pWho);
        }
    }

    void Aggro(Unit*)
    {
        if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE))
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_creature->GetHealthPercent() < 30.0f && !m_creature->IsNonMeleeSpellCasted() && !m_creature->HasAura(28995))
        {
            if (DoCastSpellIfCan(m_creature, 28995) == CAST_OK)
            {
                m_creature->CastSpell(m_creature, 28995, true); // Stoneskin
                DoScriptText(-1531100, m_creature); // %s emits a strange noise.
            }
        }

        if (acidVolleyTimer < diff && !m_creature->IsNonMeleeSpellCasted())
        {
            // supposedly the first gargoyle in plague wing did not do the acid volley, so
            // hackfix here to skip him
            if (m_creature->GetDBTableGUIDLow() != 88095)
            {
                if (DoCastSpellIfCan(m_creature, 29325) == CAST_OK) // acid volley
                    acidVolleyTimer = 8000;
            }
        }
        else
            acidVolleyTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct mob_naxxramasPlagueSlimeAI : public ScriptedAI
{
    mob_naxxramasPlagueSlimeAI(Creature* pCreature)
        : ScriptedAI(pCreature)
    {
        Reset();
        prev_spell = 0;
    }
    uint32 colorChangeTimer;
    uint32 prev_spell;
    void ChangeColor()
    {
        uint32 spell = urand(28987, 28990);
        if(const DBCSpellEntry* entry = sSpellStore.LookupEntry(spell))
            m_creature->UpdateEntry(entry->EffectMiscValue[0]);
        if (prev_spell)
            m_creature->RemoveAurasDueToSpell(prev_spell);
        DoCastSpellIfCan(m_creature, spell, CAST_TRIGGERED);
        m_creature->SetObjectScale(2.0f); // updateentry and the actual spells screws up the scale...
        prev_spell = spell;
    }

    void Reset() override
    {
        colorChangeTimer = 0;
        ChangeColor();
    }

    void Aggro(Unit*)
    {
        m_creature->CallForHelp(10.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (colorChangeTimer < diff)
        {
            colorChangeTimer = urand(9000, 12000); // todo: no idea if timer is correct
            ChangeColor();
        }
        else
            colorChangeTimer -= diff;

        DoMeleeAttackIfReady();
    }
};
struct mob_toxic_tunnelAI : public ScriptedAI
{
    mob_toxic_tunnelAI(Creature* pCreature)
        : ScriptedAI(pCreature)
    {
        Reset();
    }
    uint32 checktime;
    void Reset() override
    {
        checktime = 0;
    }

    void AttackStart(Unit*) { }
    void MoveInLineOfSight(Unit*) { }

    void UpdateAI(const uint32 diff) override 
    {
        if (checktime < diff)
        {
            checktime = 5000;
            if (!m_creature->HasAura(28370))
                m_creature->CastSpell(m_creature, 28370, true);
        }
        else
            checktime -= diff;
    }
};

CreatureAI* GetAI_mob_spiritOfNaxxramas(Creature* pCreature)
{
    return new mob_spiritOfNaxxramasAI(pCreature);
}

CreatureAI* GetAI_mob_naxxramasGargoyle(Creature* pCreature)
{
    return new mob_naxxramasGarboyleAI(pCreature);
}

CreatureAI* GetAI_mob_plagueSlimeAI(Creature* pCreature)
{
    return new mob_naxxramasPlagueSlimeAI(pCreature);
}

CreatureAI* GetAI_toxic_tunnel(Creature* pCreature)
{
    return new mob_toxic_tunnelAI(pCreature);
}

void AddSC_instance_naxxramas()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_naxxramas";
    pNewScript->GetInstanceData = &GetInstanceData_instance_naxxramas;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_naxxramas";
    pNewScript->pAreaTrigger = &AreaTrigger_at_naxxramas;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "spirit_of_naxxramas_ai";
    pNewScript->GetAI = &GetAI_mob_spiritOfNaxxramas;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "naxxramas_gargoyle_ai";
    pNewScript->GetAI = &GetAI_mob_naxxramasGargoyle;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "naxxramas_plague_slime_ai";
    pNewScript->GetAI = &GetAI_mob_plagueSlimeAI;
    pNewScript->RegisterSelf();


    pNewScript = new Script;
    pNewScript->Name = "toxic_tunnel_ai";
    pNewScript->GetAI = &GetAI_toxic_tunnel;
    pNewScript->RegisterSelf();
}
