#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int isId(const char *characterID, const char *targetId) {
    if (characterID == NULL || targetId == NULL) return 1; 

    if (strstr(characterID, targetId) != NULL) {
        return 0; // หาเจอ (ID ยังคงเดิมแม้ชื่อจะพัง)
    }
    return 1;
}

#define MAX_SKILLS 20

int TurnCount = 1;

int SpeedState = 0;

int ClashPity = 1;

typedef struct { // skill
  const char *name;
  int BasePower;
  int CoinPower;
  int Coins;
  int Offense;
  int Defense;
  float DmgMutiplier;
  int active;      // For some certain effect
  int Unbreakable; // 1 = unbreakable, 0 = breakable
  int Copies;      // number of copies for weighted selection
  int Clashable;
  int skillType; // <--- เพิ่มตรงนี้: 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter

// Buff

// [0] = This turn, [1] = Next turn
int CoinPowerBoost[2];
int FinalPowerBoost[2];
int AttackPowerBoost[2];
int AttackSkillPowerBoost[2]; // [0] = this turn, [1] = Next turn
int DefensePowerBoost[2];
int DefenseSkillPowerBoost[2]; // [0] = this turn, [1] = Next turn
int BasePowerBoost[2];
int ClashPower[2];
float DamageUp[2];
float CriticalDamageUp[2];
float Protection[2];
float DmgMutiplierBoost[2];
int OffenseBoost[2];
int DefenseBoost[2];
int UnbreakableUp[2];
} SkillStats;

typedef struct { // Character
  const char *name;
  const char *ID;
  SkillStats skills[MAX_SKILLS];
  SkillStats defenseSkill[MAX_SKILLS];
  int numSkills;
  int numDefenseSkills;
  float HP;
  float MAX_HP;
  float Shield;
  float TempShield; // Temporary Shield, lose at turn end
  int MinSpeed;
  int MaxSpeed;
  int Speed;
  int Passive;
  int Sanity;         // Add this: -45 to 45
  int hasSanity;      // Add this: 1 = has sanity system, 0 = immune to sanity
  int sanityGainBase; // Base sanity gain on clash win (default 10)
  int sanityLossBase; // Base sanity loss on clash loss (default 8)
  int immuneToPanicSkip; // Add this: 1 = still acts when panicked, 0 = normal
                         // panic
 int SanityFreezeTurns; // 0 = no lock, >0 = locked for this many turns, -1 Can't Snap out of Panic
  int Stagger; // 0 = no Stagger, >0 = Staggered for this many turns

  // Buff — [0] = This turn, [1] = Next turn
  // Up
  int PlusCoinPowerBoost[2]; // [0] = this turn, [1] = Next turn
  int MinusCoinPowerBoost[2]; // [0] = this turn, [1] = Next turn
  int MultiplyCoinPowerBoost[2]; // [0] = this turn, [1] = Next turn
  int FinalPowerUp[2]; // [0] = this turn, [1] = Next turn
  int AttackPowerUp[2]; // [0] = this turn, [1] = Next turn
  int AttackSkillPowerUp[2]; // [0] = this turn, [1] = Next turn
  int DefensePowerUp[2]; // [0] = this turn, [1] = Next turn
  int DefenseSkillPowerUp[2]; // [0] = this turn, [1] = Next turn
  int BasePowerUp[2]; // [0] = this turn, [1] = Next turn
  int ClashPowerUp[2]; // [0] = this turn, [1] = Next turn
  float DamageUp[2]; // [0] = this turn, [1] = Next turn
  float CriticalDamageUp[2]; // [0] = this turn, [1] = Next turn
  float ProtectionUp[2]; // [0] = this turn, [1] = Next turn
  float DmgMutiplierBoost[2]; // [0] = this turn, [1] = Next turn
  int OffenseLevelUp[2]; // [0] = this turn, [1] = Next turn
  int DefenseLevelUp[2]; // [0] = this turn, [1] = Next turn
  // Down
  int PlusCoinPowerDrop[2]; // [0] = this turn, [1] = Next turn
  int MinusCoinPowerDrop[2]; // [0] = this turn, [1] = Next turn
  int MultiplyCoinPowerDrop[2]; // [0] = this turn, [1] = Next turn
  int FinalPowerDown[2]; // [0] = this turn, [1] = Next turn
  int AttackPowerDown[2]; // [0] = this turn, [1] = Next turn
  int AttackSkillPowerDown[2]; // [0] = this turn, [1] = Next turn
  int DefenseSkillPowerDown[2]; // [0] = this turn, [1] = Next turn
  int DefensePowerDown[2]; // [0] = this turn, [1] = Next turn
  int BasePowerDown[2]; // [0] = this turn, [1] = Next turn
  int ClashPowerDown[2]; // [0] = this turn, [1] = Next turn
  float DamageDown[2]; // [0] = this turn, [1] = Next turn
  float CriticalDamageDown[2]; // [0] = this turn, [1] = Next turn
  float ProtectionDown[2]; // [0] = this turn, [1] = Next turn
  int OffenseLevelDown[2]; // [0] = this turn, [1] = Next turn
  int DefenseLevelDown[2]; // [0] = this turn, [1] = Next turn

  int Paralyze[2]; // [0] = this turn, [1] = Next turn

  int Bind[2]; // [0] = this turn, [1] = Next turn
  int Haste[2]; // [0] = this turn, [1] = Next turn

  // Status
  int Burn[4]; // [0] = Burn Stack, [1] = Burn Count, [2] = Burn Stack next turn, [3] = Burn Count next turn
  int Bleed[4]; // [0] = Bleed Stack, [1] = Bleed Count, [2] = Bleed Stack next turn, [3] = Bleed Count next turn
  int Tremor[5]; // [0] = Tremor Stack, [1] = Tremor Count, [2] = Tremor Stack next turn, [3] = Tremor Count next turn, [4] = Tremor Burst Stack
const char * TremorType;
  int Rupture[4]; // [0] = Rupture Stack, [1] = Rupture Count, [2] = Rupture Stack next turn, [3] = Rupture Count next turn
  int Sinking[4]; // [0] = Sinking Stack, [1] = Sinking Count, [2] = Sinking Stack next turn, [3] = Sinking Count next turn
  int Poise[4]; // [0] = Poise Stack, [1] = Poise Count, [2] = Poise Stack next turn, [3] = Poise Count next turn
  int Charge[4]; // [0] = Charge Stack, [1] = Charge Count, [2] = Charge Stack next turn, [3] = Charge Count next turn

} Character;

typedef struct { // Clash result
  int winner;
  SkillStats *playerskillUsed;
  SkillStats *enemyskillUsed;

 // Clash
int playerCoins;
int enemyCoins;
int playerUnbreakableLost;
int enemyUnbreakableLost;

  // Buff
  int playerTempOffense;
  int enemyTempOffense;
  int playerTempDefense;
  int enemyTempDefense;
  int ClashCount;

  int playerFinalPower; 
  int enemyFinalPower;
} ClashResult;

// --- เพิ่มบรรทัดเหล่านี้ไว้ด้านบน เพื่อให้คอมไพเลอร์รู้จักฟังก์ชันล่วงหน้า for clash / clashable counter ---
ClashResult clashPhase(Character *p1, SkillStats *s1, int playerTempOffense, int playerTempDefense, Character *p2, SkillStats *s2, int enemyTempOffense, int enemyTempDefense, Character *fullPlayer, int PContinueUnbreakCoin, int EContinueUnbreakCoin);

ClashResult ClashableCounter(Character *p1, SkillStats *s1, int playerTempOffense, int playerTempDefense, int playerCoins, Character *p2, SkillStats *s2, int enemyTempOffense, int enemyTempDefense, int enemyCoins, Character *fullPlayer, int PContinueUnbreakCoin, int EContinueUnbreakCoin);

SkillStats *getEffectiveSkill(Character *c, Character *c2, SkillStats *chosenSkill, int *tempOffense, int *tempDefense);

//---------------------Buff system-----------------
void initializeCharacterBuffs(Character *c) {

    c->TremorType = "Normal";

    c->PlusCoinPowerBoost[0]     = 0; c->PlusCoinPowerBoost[1]     = 0;
    c->MinusCoinPowerBoost[0]    = 0; c->MinusCoinPowerBoost[1]    = 0;
    c->MultiplyCoinPowerBoost[0] = 0; c->MultiplyCoinPowerBoost[1] = 0;
    c->FinalPowerUp[0]           = 0; c->FinalPowerUp[1]           = 0;
    c->AttackPowerUp[0]          = 0; c->AttackPowerUp[1]          = 0;
    c->AttackSkillPowerUp[0]     = 0; c->AttackSkillPowerUp[1]     = 0;
    c->DefensePowerUp[0]         = 0; c->DefensePowerUp[1]         = 0;
    c->DefenseSkillPowerUp[0]         = 0; c->DefenseSkillPowerUp[1]         = 0;
    c->BasePowerUp[0]            = 0; c->BasePowerUp[1]            = 0;
    c->ClashPowerUp[0]           = 0; c->ClashPowerUp[1]           = 0;
    c->DamageUp[0]               = 0; c->DamageUp[1]               = 0;
    c->CriticalDamageUp[0]       = 0; c->CriticalDamageUp[1]       = 0;
    c->ProtectionUp[0]           = 0; c->ProtectionUp[1]           = 0;
    c->DmgMutiplierBoost[0]      = 0; c->DmgMutiplierBoost[1]      = 0;
    c->OffenseLevelUp[0]         = 0; c->OffenseLevelUp[1]         = 0;
    c->DefenseLevelUp[0]         = 0; c->DefenseLevelUp[1]         = 0;
    c->PlusCoinPowerDrop[0]      = 0; c->PlusCoinPowerDrop[1]      = 0;
    c->MinusCoinPowerDrop[0]     = 0; c->MinusCoinPowerDrop[1]     = 0;
    c->MultiplyCoinPowerDrop[0]  = 0; c->MultiplyCoinPowerDrop[1]  = 0;
    c->FinalPowerDown[0]         = 0; c->FinalPowerDown[1]         = 0;
    c->AttackPowerDown[0]        = 0; c->AttackPowerDown[1]        = 0;
    c->AttackSkillPowerDown[0]   = 0; c->AttackSkillPowerDown[1]   = 0;
    c->DefensePowerDown[0]       = 0; c->DefensePowerDown[1]       = 0;
    c->BasePowerDown[0]          = 0; c->BasePowerDown[1]          = 0;
    c->ClashPowerDown[0]         = 0; c->ClashPowerDown[1]         = 0;
    c->DamageDown[0]             = 0; c->DamageDown[1]             = 0;
    c->CriticalDamageDown[0]     = 0; c->CriticalDamageDown[1]     = 0;
    c->ProtectionDown[0]         = 0; c->ProtectionDown[1]         = 0;
    c->OffenseLevelDown[0]       = 0; c->OffenseLevelDown[1]       = 0;
    c->DefenseLevelDown[0]       = 0; c->DefenseLevelDown[1]       = 0;

    c->Paralyze[0] = 0; c->Paralyze[1] = 0;


    c->SanityFreezeTurns = 0;
    c->Stagger = 0;

    c->Bind[0] = c->Bind[1] = 0;
    c->Haste[0] = c->Haste[1] = 0;
  }


  void clearTurnEffects(Character *c) {
    c->TempShield = 0;

    // Rotate [1] (next turn) -> [0] (this turn), then clear [1]
    c->PlusCoinPowerBoost[0]     = c->PlusCoinPowerBoost[1];     c->PlusCoinPowerBoost[1]     = 0;
    c->MinusCoinPowerBoost[0]    = c->MinusCoinPowerBoost[1];    c->MinusCoinPowerBoost[1]    = 0;
    c->MultiplyCoinPowerBoost[0] = c->MultiplyCoinPowerBoost[1]; c->MultiplyCoinPowerBoost[1] = 0;
    c->FinalPowerUp[0]           = c->FinalPowerUp[1];           c->FinalPowerUp[1]           = 0;
    c->AttackPowerUp[0]          = c->AttackPowerUp[1];          c->AttackPowerUp[1]          = 0;
    c->AttackSkillPowerUp[0]     = c->AttackSkillPowerUp[1];     c->AttackSkillPowerUp[1]     = 0;
    c->DefensePowerUp[0]         = c->DefensePowerUp[1];         c->DefensePowerUp[1]         = 0;
    c->DefenseSkillPowerUp[0]    = c->DefenseSkillPowerUp[1];    c->DefenseSkillPowerUp[1]    = 0;
    c->BasePowerUp[0]            = c->BasePowerUp[1];            c->BasePowerUp[1]            = 0;
    c->ClashPowerUp[0]           = c->ClashPowerUp[1];           c->ClashPowerUp[1]           = 0;
    c->DamageUp[0]               = c->DamageUp[1];               c->DamageUp[1]               = 0;
    c->CriticalDamageUp[0]       = c->CriticalDamageUp[1];       c->CriticalDamageUp[1]       = 0;
    c->ProtectionUp[0]           = c->ProtectionUp[1];           c->ProtectionUp[1]           = 0;
    c->DmgMutiplierBoost[0]      = c->DmgMutiplierBoost[1];      c->DmgMutiplierBoost[1]      = 0;
    c->OffenseLevelUp[0]         = c->OffenseLevelUp[1];         c->OffenseLevelUp[1]         = 0;
    c->DefenseLevelUp[0]         = c->DefenseLevelUp[1];         c->DefenseLevelUp[1]         = 0;
    c->PlusCoinPowerDrop[0]      = c->PlusCoinPowerDrop[1];      c->PlusCoinPowerDrop[1]      = 0;
    c->MinusCoinPowerDrop[0]     = c->MinusCoinPowerDrop[1];     c->MinusCoinPowerDrop[1]     = 0;
    c->MultiplyCoinPowerDrop[0]  = c->MultiplyCoinPowerDrop[1];  c->MultiplyCoinPowerDrop[1]  = 0;
    c->FinalPowerDown[0]         = c->FinalPowerDown[1];         c->FinalPowerDown[1]         = 0;
    c->AttackPowerDown[0]        = c->AttackPowerDown[1];        c->AttackPowerDown[1]        = 0;
    c->AttackSkillPowerDown[0]   = c->AttackSkillPowerDown[1];   c->AttackSkillPowerDown[1]   = 0;
    c->DefensePowerDown[0]       = c->DefensePowerDown[1];       c->DefensePowerDown[1]       = 0;
    c->DefenseSkillPowerDown[0]   = c->DefenseSkillPowerDown[1];   c->DefenseSkillPowerDown[1]   = 0;
    c->BasePowerDown[0]          = c->BasePowerDown[1];          c->BasePowerDown[1]          = 0;
    c->ClashPowerDown[0]         = c->ClashPowerDown[1];         c->ClashPowerDown[1]         = 0;
    c->DamageDown[0]             = c->DamageDown[1];             c->DamageDown[1]             = 0;
    c->CriticalDamageDown[0]     = c->CriticalDamageDown[1];     c->CriticalDamageDown[1]     = 0;
    c->ProtectionDown[0]         = c->ProtectionDown[1];         c->ProtectionDown[1]         = 0;
    c->OffenseLevelDown[0]       = c->OffenseLevelDown[1];       c->OffenseLevelDown[1]       = 0;
    c->DefenseLevelDown[0]       = c->DefenseLevelDown[1];       c->DefenseLevelDown[1]       = 0;

    c->Paralyze[0] = c->Paralyze[1];
    c->Paralyze[1] = 0;

    c->Bind[0] = c->Bind[1];   c->Bind[1] = 0;
    c->Haste[0] = c->Haste[1]; c->Haste[1] = 0;

    // Apply next turn status stacks/counts -> current
    if (c->Burn[2] > 0 || c->Burn[3] > 0) {
      c->Burn[0] += c->Burn[2];
      c->Burn[1] += c->Burn[3];
      c->Burn[2] = 0;
      c->Burn[3] = 0;
    }

    if (c->Bleed[2] > 0 || c->Bleed[3] > 0) {
      c->Bleed[0] += c->Bleed[2];
      c->Bleed[1] += c->Bleed[3];
      c->Bleed[2] = 0;
      c->Bleed[3] = 0;
    }

    if (c->Tremor[2] > 0 || c->Tremor[3] > 0) {
      c->Tremor[0] += c->Tremor[2];
      c->Tremor[1] += c->Tremor[3];
      c->Tremor[2] = 0;
      c->Tremor[3] = 0;
    }

    if (c->Rupture[2] > 0 || c->Rupture[3] > 0) {
      c->Rupture[0] += c->Rupture[2];
      c->Rupture[1] += c->Rupture[3];
      c->Rupture[2] = 0;
      c->Rupture[3] = 0;
    }

    if (c->Sinking[2] > 0 || c->Sinking[3] > 0) {
      c->Sinking[0] += c->Sinking[2];
      c->Sinking[1] += c->Sinking[3];
      c->Sinking[2] = 0;
      c->Sinking[3] = 0;
    }

    if (c->Poise[2] > 0 || c->Poise[3] > 0) {
      c->Poise[0] += c->Poise[2];
      c->Poise[1] += c->Poise[3];
      c->Poise[2] = 0;
      c->Poise[3] = 0;
    }

    if (c->Charge[2] > 0 || c->Charge[3] > 0) {
      c->Charge[0] += c->Charge[2];
      c->Charge[1] += c->Charge[3];
      c->Charge[2] = 0;
      c->Charge[3] = 0;
    }
  }

void rotateSkillBuffs(SkillStats *s) {
    if (s == NULL) return;

    // --- ส่วนที่ 1: ย้ายค่าจาก Next Turn [1] มาเป็น This Turn [0] ---
    s->CoinPowerBoost[0]         = s->CoinPowerBoost[1];
    s->FinalPowerBoost[0]        = s->FinalPowerBoost[1];
    s->AttackPowerBoost[0]       = s->AttackPowerBoost[1];
  s->AttackSkillPowerBoost[0]       = s->AttackSkillPowerBoost[1];
  s->DefensePowerBoost[0]       = s->DefensePowerBoost[1];
  s->DefenseSkillPowerBoost[0]       = s->DefenseSkillPowerBoost[1];
    s->BasePowerBoost[0]         = s->BasePowerBoost[1];
    s->ClashPower[0]             = s->ClashPower[1];
    s->DamageUp[0]               = s->DamageUp[1];
    s->CriticalDamageUp[0]       = s->CriticalDamageUp[1];
    s->Protection[0]             = s->Protection[1];
    s->DmgMutiplierBoost[0]      = s->DmgMutiplierBoost[1];
    s->OffenseBoost[0]           = s->OffenseBoost[1];
    s->DefenseBoost[0]           = s->DefenseBoost[1];
  s->UnbreakableUp[0]           = s->UnbreakableUp[1];

    // --- ส่วนที่ 2: ล้างค่า Next Turn [1] ให้เป็น 0 เพื่อรอรับบัฟใหม่ในเทิร์นนี้ ---
    s->CoinPowerBoost[1]         = 0;
    s->FinalPowerBoost[1]        = 0;
    s->AttackPowerBoost[1]       = 0;
    s->AttackSkillPowerBoost[1]  = 0;
    s->DefensePowerBoost[1]      = 0;
    s->DefenseSkillPowerBoost[1] = 0;
    s->BasePowerBoost[1]         = 0;
    s->ClashPower[1]             = 0;
    s->DamageUp[1]               = 0.0f;
    s->CriticalDamageUp[1]       = 0.0f;
    s->Protection[1]             = 0.0f;
    s->DmgMutiplierBoost[1]      = 0.0f;
    s->OffenseBoost[1]           = 0;
    s->DefenseBoost[1]           = 0;
  s->UnbreakableUp[1]           = 0;
}

void resetCharacterSkillsBuffs(Character *c) {
    // ล้างบัฟในสกิลโจมตีทั้งหมด
    for (int i = 0; i < c->numSkills; i++) {
          rotateSkillBuffs(&c->skills[i]);
    }
    // ล้างบัฟในสกิลป้องกันทั้งหมด
    for (int i = 0; i < c->numDefenseSkills; i++) {
          rotateSkillBuffs(&c->defenseSkill[i]);
    }
}

void clearTurnSkillBuffs(Character *c) {
    if (c == NULL) return;
    for (int i = 0; i < c->numSkills; i++) {
        SkillStats *s = &c->skills[i];
        s->CoinPowerBoost[0]    = 0;
        s->FinalPowerBoost[0]   = 0;
        s->AttackPowerBoost[0]  = 0;
        s->AttackSkillPowerBoost[0]  = 0;
        s->DefensePowerBoost[0]      = 0;
        s->DefenseSkillPowerBoost[0] = 0;
        s->BasePowerBoost[0]    = 0;
        s->ClashPower[0]        = 0;
        s->DamageUp[0]          = 0.0f;
        s->CriticalDamageUp[0]  = 0.0f;
        s->Protection[0]        = 0.0f;
        s->DmgMutiplierBoost[0] = 0.0f;
        s->OffenseBoost[0]      = 0;
        s->DefenseBoost[0]      = 0;
    }
    for (int i = 0; i < c->numDefenseSkills; i++) {
        SkillStats *s = &c->defenseSkill[i];
        s->CoinPowerBoost[0]    = 0;
        s->FinalPowerBoost[0]   = 0;
        s->AttackPowerBoost[0]  = 0;
        s->AttackSkillPowerBoost[0]  = 0;
        s->DefensePowerBoost[0]      = 0;
        s->DefenseSkillPowerBoost[0] = 0;
        s->BasePowerBoost[0]    = 0;
        s->ClashPower[0]        = 0;
        s->DamageUp[0]          = 0.0f;
        s->CriticalDamageUp[0]  = 0.0f;
        s->Protection[0]        = 0.0f;
        s->DmgMutiplierBoost[0] = 0.0f;
        s->OffenseBoost[0]      = 0;
        s->DefenseBoost[0]      = 0;
    }
}

const char* getSkillTypeName(int type) {
    switch(type) {
        case 1: return "Guard Skill";
        case 2: return "Evade Skill";
        case 3: return "Counter Skill";
        case 4: return "Clashable Guard Skill";
        case 5: return "Clashable Counter Skill";
        default: return "Attack Skill";
    }
}




void inflictStatus(int status[2], int potencyAdd, int countAdd, int MinStack, int MaxStack, int MinCount, int MaxCount) {

    // 1. บวกค่าเข้าไปก่อน
    status[0] += potencyAdd;
    status[1] += countAdd;

    // Cap 0-99
  if (status[0] < MinStack) status[0] = MinStack;
  if (status[0] > MaxStack) status[0] = MaxStack;
  if (status[1] < MinCount) status[1] = MinCount;
  if (status[1] > MaxCount) status[1] = MaxCount;
}

// ฟังก์ชันกลางสำหรับหักดาเมจเข้าเกราะและเลือด // true damage = 1, normal damage = 0
void applyDamage(Character *attacker, Character *defender, int damage, int trueDamage, const char *Type) {
    if (damage <= 0 && defender != NULL) return;

  // -------------- Effect --------------

  // ------------------------------------------



  // -------------- Damage --------------

  if (!trueDamage) {

    // 1. TempShield
    if (defender->TempShield > 0) {
        if (defender->TempShield >= damage) { defender->TempShield -= damage; return; }
        else { damage -= defender->TempShield; defender->TempShield = 0; }
    }
    // 2. Shield
    if (defender->Shield > 0) {
        if (defender->Shield >= damage) { defender->Shield -= damage; return; }
        else { damage -= defender->Shield; defender->Shield = 0; }
    }
    // 3. HP
    defender->HP -= damage;
    if (defender->HP < 0) defender->HP = 0;

    // Heishou Pack - You Branch Adept Heathcliff save for lost HP
    if (isId(defender->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0) {
        defender->skills[6].active += (int)(damage);
    }

    // Heishou Pack - You Branch Adept Heathcliff - Burn
      if (isId(defender->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && Type != NULL && strcmp(Type, "Burn") == 0) {

        if (defender->HP <= 0) defender->HP = 1;

        int gain = 1;
        if (defender->HP < defender->MAX_HP * 0.5) gain += 1;

              defender->Passive += gain;
        if (defender->Passive > 20) defender->Passive = 20;

         printf("\n%s gains +%d Battleblood Instinct (%d)\n", defender->name, gain, defender->Passive);

        sleep(1);
    }

} else {

    // HP
    defender->HP -= damage;
    
    if (defender->HP < 0) defender->HP = 0;

    // Heishou Pack - You Branch Adept Heathcliff save for lost HP
    if (isId(defender->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0) {
        defender->skills[6].active += (int)(damage); 
    }

}

  // ---------------------- Anti death effect ----------------------

  // Erlking Heathcliff Faded promise for wild hunt
  if (attacker != NULL && isId(attacker->ID, "Erlking Heathcliff") == 0 && isId(defender->ID, "Heathcliff:Wild Hunt") == 0 && (attacker->skills[7].active == 1 || attacker->skills[7].active == 0) && defender->HP <= 0)  {

    if (defender->skills[7].active == 1) {

    printf("\n%s's 'Faded Promise' activated! In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn (Once per Encounter)\n", defender->name);

    sleep(1);

    }

      attacker->skills[7].active = 0;

      if (defender->HP < 1) defender->HP = 1;

  }

  // Hong lu:The Lord of Hongyuan - Passive
  if ((isId(defender->ID, "Hong lu:The Lord of Hongyuan")) == 0 &&
           defender->HP <= 0 && (defender->skills[5].active == 1 || defender->skills[5].active == 0)) {

    if (defender->skills[5].active == 1) {

    printf("\n%s's '%s' activeted! Nullity all damage; then apply 'Lordsguard' to all left Heishou Pack and bring %s's HP to 1 (Once per Encounter)\n",
      defender->name, defender->skills[5].name, defender->name);

    sleep(1);

    printf("\n%s: \"The Lord will not die.\"\n", defender->name);

    sleep(1);

    }

      defender->skills[5].active = 0;

    if (defender->HP < 1) defender->HP = 1;

  }

  // Heishou Pack - You Branch Adept Heathcliff - Burn
    if (isId(defender->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && Type != NULL && strcmp(Type, "Burn") == 0) {

      if (defender->HP <= 0) defender->HP = 1;

      int gain = 1;
      if (defender->HP < defender->MAX_HP * 0.5) gain += 1;

            defender->Passive += gain;
      if (defender->Passive > 20) defender->Passive = 20;

       printf("\n%s gains +%d Battleblood Instinct (%d)\n", defender->name, gain, defender->Passive);

      sleep(1);
  }

    // Heishou Pack - You Branch Adept Heathcliff - Anti death Passive
    if ((isId(defender->ID, "Heishou Pack - You Branch Adept Heathcliff")) == 0 &&
        defender->HP <= 0 && (defender->skills[3].active == 0 || defender->skills[5].active == -1)) {

      if (defender->skills[3].active == 0) {

      printf("\n%s's 'Flame Rooster's Death Defiance [炎鳥不死戦]' activated! Nullity all damage; then bring %s's HP to 1 (Once per Encounter)\n",
        defender->name, defender->name);

      sleep(1);

      printf("\n%s: \"Flame Rooster's Death Defiance [炎鳥不死戦]... Heh! You really thought I was gonna kick it... Huh?!\"\n",
        defender->name);

      sleep(1);

      }

        defender->skills[3].active = -1;

      if (defender->HP < 1) defender->HP = 1;

    }

    // Meursault:Blade Lineage Mentor - Passive
    if (isId(defender->ID, "Meursault:Blade Lineage Mentor") == 0 &&
        defender->HP <= 0 && (defender->Passive == 0 || defender->Passive == -1)) {

      if (defender->Passive == 0) {

      printf("\n%s's 'Swordplay of the Homeland' activated! Nullity all damage; then bring %s's HP to 1 (Once per Encounter)\n",
        defender->name, defender->name);

      sleep(1);

      }

        defender->defenseSkill[2].active = 1;

        defender->Passive = -1; 

      if (defender->HP < 1) defender->HP = 1;

    }

  // ------------------------------------------------------------------

}



void calculateSpeed(Character *c) {
    if (c->HP <= 0) {
        c->Speed = 0;
        return;
    }
    // สุ่มค่าจาก Min ถึง Max
    int baseSpeed = c->MinSpeed + (rand() % (c->MaxSpeed - c->MinSpeed + 1));
    // คำนวณ Speed สุทธิ (Haste เพิ่ม, Bind ลด)
    c->Speed = baseSpeed + c->Haste[0] - c->Bind[0];
    if (c->Speed < 1) c->Speed = 1; // Speed ต่ำสุดคือ 1

    printf("[%s] Speed: %d %-5s(Haste +%d, Bind -%d)\n", 
            c->name, c->Speed, "", c->Haste[0], c->Bind[0]);
}


// ------------------------ Stagger functions ---------------------
int isStaggered(Character *c) {
    if (c->Stagger > 0) {

        return 1;
    }
    return 0;
}

void MoveStagger(Character *attacker, Character *defender, int StaggerBar, int StaggerMoveAmount, int PrintType) {

  defender->Tremor[4] += StaggerMoveAmount;

  if (defender->Tremor[4] >= StaggerBar && defender->Stagger <= 0) {

    defender->Stagger += 2;

    if (PrintType == 1) {

      printf("\t Target 'Stagger' for one turn");

    } else {

      printf("\n%s Staggered for one turn\n", defender->name);

    }

      defender->Tremor[4] = 0;

  }
}

void TremorBurst(Character *attacker, Character *defender, int TremorBurstStagger, int *totalDamage, int PrintType) {

  if (defender->Tremor[0] <= 0 && defender->Tremor[1] > 0) defender->Tremor[0]++;
  if (defender->Tremor[1] <= 0 && defender->Tremor[0] > 0) defender->Tremor[1]++;

  int deal =  defender->Tremor[0];

    defender->Tremor[1] -= 1;
  if (defender->Tremor[1] <= 0) defender->Tremor[1] = 0;

  if (PrintType == 1) {

      printf(" Trigger 'Tremor Burst' on target (Stack %d Count %d)", defender->Tremor[0], defender->Tremor[1]);

  } else if (PrintType == 2) {

    printf("\n%s triggers 'Tremor Burst' on target (Stack %d Count %d)\n", attacker->name, defender->Tremor[0], defender->Tremor[1]);

  }

  if (defender->Tremor[1] <= 0) {
        defender->Tremor[0] = 0;
  }

  MoveStagger(attacker, defender, TremorBurstStagger, deal, PrintType);




  // Tremor - Scorch

  if (strcmp(defender->TremorType, "Scorch") == 0) {

    int damage = (deal + defender->Burn[0])/2;

    applyDamage(attacker, defender, damage, 0, NULL);

    if (totalDamage != NULL) {
      *totalDamage += damage;
    }

    defender->Burn[1]--;

    if (defender->Burn[1] <= 0) defender->Burn[1] = 0;

  if (PrintType == 1) {

      printf("\t [Tremor - Scorch] Take %d damage, and lose 1 Burn Count (%d)", damage, defender->Burn[1]);

  } else if (PrintType == 2) {

    printf("\n[Tremor - Scorch] Take %d damage, and lose 1 Burn Count (%d)\n", damage, defender->Burn[1]);

  }

    if (defender->Burn[1] <= 0) defender->Burn[0] = 0;

    if (defender->Tremor[1] <= 0) {
          defender->Tremor[0] = 0;
          defender->TremorType = "Normal"; 
    }

  }

}


//--------------------------Sanity functions----------------------------

// Helper to handle Sanity changes, locking, and clamping
void updateSanity(Character *c, int delta) {

  if (isId(c->ID, "Muga Ryōshū") == 0 && (c->Sinking[0] > 0 || c->Sinking[1] > 0)) {
      if (delta < 0) {
          int hpDmg = abs(delta) * 3;
          applyDamage(NULL, c, hpDmg, 1, NULL); // True damage
      }
      c->Sanity = -44; // ล็อคไว้ที่ -44 เสมอ
      return;
  }

  if (isId(c->ID, "Muga Ryōshū") == 0 && delta != 0) {
      return;
  }

  if (!c->hasSanity) return;

  // 1. Handle Healing Lock
  if (delta > 0 && c->SanityFreezeTurns > 0) return; 

  // เก็บค่า Sanity เดิมไว้ก่อนคำนวณ
  int oldSP = c->Sanity;

  // 2. Apply Change
  c->Sanity += delta;

  // 3. Clamp Sanity (-45 to 45)
  if (c->Sanity > 45) c->Sanity = 45;
  if (c->Sanity < -45) c->Sanity = -45;

  // 4. แก้ไขจุดนี้: ติด Lock เฉพาะตอนที่ตกลงมาจากค่าที่สูงกว่า -45 เท่านั้น
  if (c->Sanity <= -45 && oldSP > -45) {
    if (c->SanityFreezeTurns == 0) {
        c->SanityFreezeTurns = 2; 
    }
  }
}

// Check if character should skip turn due to panic
int isPanicked(Character *c) {
  if (c->hasSanity == 0)
    return 0; // No sanity = no panic
  if (c->immuneToPanicSkip)
    return 0; // Immune to panic skip
  return (c->Sanity <= -45); // Normal panic check
}

// Get Sanity status message
const char *getSanityStatus(Character *c) {
  if (c->Sanity <= -45) {
    if (isId(c->ID, "Lei heng") == 0) return "Beastly Instinct"; 
  if (isId(c->ID, "Erlking Heathcliff") == 0) return "Revenge"; 
     if (isId(c->ID, "Sukuna:King of Curse") == 0) return "King"; 
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0) return "Reawakening Joy Of Carnage"; 
   if (isId(c->ID, "Fixer grade 9?") == 0) return "Black Heart"; 
    if (isId(c->ID, "Jia Qiu") == 0) return "Jia Qiu"; 
    if (isId(c->ID, "The Middle Nursefather - Matthias") == 0) return "Mad Rampage"; 

    return "PANIC";
  }
  if (c->Sanity <= -30)
  return "Low Morale";
  return "Normal";
}

// Apply Low Morale/Panic debuff
void applySanityDebuff(Character *c) {

  // NORMAL ((-30)+ Sanity) - Reset to base values
  if (c->Sanity > -30) {
    if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0) {
      c->sanityGainBase = 6;

    } else if (isId(c->ID, "Erlking Heathcliff") == 0) {

      updateSanity(c, 15);
      printf("\n%s heals 15 Sanity (%d)\n", c->name, c->Sanity);
      sleep(1);
    } else if (isId(c->ID, "Fixer grade 9?") == 0) {
      c->sanityLossBase = -7;

    }

  }

  // LOW MORALE (-30 to -44 Sanity)
  if (c->Sanity <= -30 && c->Sanity > -45) {
    if (isId(c->ID, "Lei heng") == 0) {

      c->DamageUp[0] += 10;
      c->FinalPowerUp[0] += 1;
      c->ProtectionDown[0] += 10;

      printf("\nWhile 'Low Morale', %s gains Final Power +1, 10%% Damage Up, take 10%% more damage.\n",
             c->name);
      sleep(1);

    } else if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0) {
      c->OffenseLevelUp[0] += 2;
      c->sanityGainBase = 10;

      printf(
          "\nWhile 'Low Morale', %s gains Offense +2, Sanity Heal Efficiency +4.\n",
          c->name);
      sleep(1);

    } else if (isId(c->ID, "Erlking Heathcliff") == 0) {
      c->DamageUp[0] += 30;
        c->DefenseLevelUp[0] -= 3;
      updateSanity(c, 15);

      printf("\n%s heals 15 Sanity\n", c->name);

      sleep(1);

      printf("\nWhile 'Low Morale', %s gains 30%% Damage Up, Defense -3.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Fixer grade 9?") == 0) {
      c->AttackPowerUp[0] += 2;
        c->DefenseLevelUp[0] -= 5;
      c->sanityGainBase = -12; // Negative gain = loss on win

      printf("\nWhile 'Low Morale', %s gains 1 Attack Power Up, Defense -5, Sanity Loss Efficiency +6.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Jia Qiu") == 0) {

      c->DamageUp[0] += 10;

      printf(
          "\nWhile 'Low Morale', %s deals 10%% more damange.\n",
          c->name);
      sleep(1);

    } else if (isId(c->ID, "Sukuna:King of Curse") == 0) {

      c->DamageUp[0] += 30;

      printf(
          "\nWhile 'Low Morale', %s deals 30%% more damange.\n",
          c->name);
      sleep(1);

    } else if (isId(c->ID, "The Middle Nursefather - Matthias") == 0) {

      int bonusAtk = (c->Sanity <= -45) ? 3 : 2;
      int defDown = (c->Sanity <= -45) ? 5 : 4;
      int threshold = 30;

      c->AttackSkillPowerUp[0] += bonusAtk;
          c->DefenseLevelUp[0] -= defDown;

      // Damage Up จากความเจ็บปวด
      int dmgTakenLastTurn = c->skills[3].active;
      int extraDmgUp = dmgTakenLastTurn / threshold;
      if (extraDmgUp > 5) extraDmgUp = 5;
      c->DamageUp[0] += (extraDmgUp * 10); // สมมติ 1 Stack = 10%

      printf("\nWhile 'Low Morale', %s gains %d Attack Skill Power Up and %d Defense Level Down; gain +10%% Damage Up for every 30 HP damage taken last turn (%d%% - Max 50%%)\n",
         c->name, bonusAtk, defDown, extraDmgUp * 10);

      sleep(1);

    }

  }

  // PANIC (-45 Sanity)
  if (c->Sanity <= -45) {
    if (isId(c->ID, "Lei heng") == 0) {

      c->DamageUp[0] += 20;
      c->PlusCoinPowerBoost[0] += 2;
      c->ProtectionDown[0] += 20;

      printf("\nWhile 'Beastly Instinct', %s gains Plus Coin Boost +2, deal 20%% more damage, "
             "take 20%% more damage.\n",
             c->name);

      sleep(1);

    } else if (isId(c->ID,
                          "Sancho:The Second Kindred of Don Quixote") == 0) {
        c->OffenseLevelUp[0] += 3;
        c->DefenseLevelUp[0] += 6;

      printf("\nWhile 'Reawakening Joy Of Carnage', %s gains Offense +3 and Defense +6.\n", c->name);
      sleep(1);

    } else if (isId(c->ID, "Erlking Heathcliff") == 0) {

      c->DamageUp[0] += 50;
        c->DefenseLevelUp[0] -= 6;

      printf("\nWhile 'Revenge', %s gains 50%% Damage Up and Defense -6.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Fixer grade 9?") == 0) {

      c->DamageUp[0] += 30;
        c->DefenseLevelUp[0] -= 10;
      c->AttackPowerUp[0] += 3;

      printf("\nWhile 'Black Heart', %s gains 30%% Damage Up, 3 Attack Power Up, Defense -10.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Jia Qiu") == 0) {

      c->DamageUp[0] += 20;

      printf("\nWhile 'Jia Qiu', %s deals 20%% more damage.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Sukuna:King of Curse") == 0) {

      c->DamageUp[0] += 100;
      c->ProtectionDown[0] += 100;

      printf("\nWhile 'King', %s gains 100%% Damage Up and take 100%% more damage.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "The Middle Nursefather - Matthias") == 0) {

      int bonusAtk = (c->Sanity <= -45) ? 3 : 2;
      int defDown = (c->Sanity <= -45) ? 5 : 4;
      int threshold = 30;

      c->AttackSkillPowerUp[0] += bonusAtk;
          c->DefenseLevelUp[0] -= defDown;

      // Damage Up จากความเจ็บปวด
      int dmgTakenLastTurn = c->skills[3].active;
      int extraDmgUp = dmgTakenLastTurn / threshold;
      if (extraDmgUp > 5) extraDmgUp = 5;
      c->DamageUp[0] += (extraDmgUp * 10); // สมมติ 1 Stack = 10%

      printf("\nWhile 'Mad Rampage', %s gains %d Attack Skill Power Up and %d Defense Level Down; gain +10%% Damage Up for every 30 HP damage taken last turn (%d%% - Max 50%%)\n",
             c->name, bonusAtk, defDown, extraDmgUp * 10);

      sleep(1);
    }

  }

}

// Modified coin toss - check if character has sanity
int tossCoinWithSanity(Character *c) {
  // If character has no sanity system, use normal 50/50
  if (c->hasSanity == 0) {
    return rand() % 2;
  }

  // Otherwise use sanity-modified coin flip
  int headsChance = 50 + c->Sanity;







  // ----------- Tossing effect -----------

    // Heathcliff:Wild Hunt – Impending Ruin
  if (c->skills[4].name != NULL && strcmp(c->skills[4].name, "Impending Ruin") == 0 && c->skills[4].active > 0) {
          headsChance -= 10; // ลด 10%
  }

  // --------------------------------------------









  // Clamp between 5% and 95%
  if (headsChance < 5)
    headsChance = 5;
  if (headsChance > 95)
    headsChance = 95;

  int roll = rand() % 100;
  return (roll < headsChance) ? 1 : 0;
}

// Calculate sanity gain based on clash count
int calculateSanityGain(Character *c, int clashCount) {
  if (c->hasSanity == 0)
    return 0;

  // Base value (default 10, but can be character-specific)
  int baseGain = c->sanityGainBase;

  // Multiply by (1 + 0.2 * (clashCount - 1))
  // Example: clash 1 = 10, clash 2 = 12, clash 3 = 14.4, etc.
  float multiplier = 1.0f + (0.2f * (clashCount - 1));

  int gain = (int)(baseGain * multiplier);

  return gain;
}

// Calculate sanity loss based on clash count
int calculateSanityLoss(Character *c, int clashCount) {
  if (c->hasSanity == 0)
    return 0;

  // Base value (default 8, but can be character-specific)
  int baseLoss = c->sanityLossBase;

  // Multiply by (1 + 0.1 * (clashCount - 1))
  // Example: clash 1 = 8, clash 2 = 8.8, clash 3 = 9.6, etc.
  float multiplier = 1.0f + (0.1f * (clashCount - 1));

  int loss = (int)(baseLoss * multiplier);

  return loss;
}

// ฟังก์ชันสุ่มเลือกใบที่จะใช้ โดยคำนวณจาก Copies ของ Atk1, Atk2 และ Defense Skills ทั้งหมด
int pickEnemyActionWeighted(Character *c, int s1_idx, int s2_idx) {
    int totalWeight = 0;

    // 1. รวมน้ำหนักท่าโจมตีในมือ 2 ใบ
    if (s1_idx >= 0) totalWeight += c->skills[s1_idx].Copies;
    if (s2_idx >= 0) totalWeight += c->skills[s2_idx].Copies;

    // 2. รวมน้ำหนักท่าป้องกันที่มีทั้งหมด
    for (int i = 0; i < c->numDefenseSkills; i++) {
        totalWeight += c->defenseSkill[i].Copies;
    }

  if (totalWeight <= 0) {
      if (s1_idx >= 0 && c->skills[s1_idx].Copies > 0) return 1;
      if (s2_idx >= 0 && c->skills[s2_idx].Copies > 0) return 2;
      return 1; // สุดท้ายค่อย fallback
  }

    int r = rand() % totalWeight;
    int cum = 0;

  if (s1_idx >= 0 && c->skills[s1_idx].Copies > 0) {
    cum += c->skills[s1_idx].Copies;
    if (r < cum) return 1; 
  }

  if (s2_idx >= 0 && c->skills[s2_idx].Copies > 0) {
    cum += c->skills[s2_idx].Copies;
    if (r < cum) return 2;
  }

    // ตรวจสอบ Defense Skills (ส่งค่า 100 + index กลับไปเพื่อแยกแยะ)
    for (int i = 0; i < c->numDefenseSkills; i++) {
        cum += c->defenseSkill[i].Copies;
        if (r < cum) return 100 + i; 
    }

    return 1;
}

// Weighted pick helper for skills 0..2
int pickSkill(int *pool, int count, Character *c) {

  if (count <= 0) return -1; // Prevent crash

  int totalWeight = 0;
  for (int i = 0; i < count; i++)
    totalWeight += c->skills[pool[i]].Copies;

  int r = rand() % totalWeight;
  int cum = 0;
  for (int i = 0; i < count; i++) {
    cum += c->skills[pool[i]].Copies;
    if (r < cum)
      return pool[i];
  }
  return pool[0]; // fallback
}

void getSkills(Character *c, int *s1, int *s2, int *s3, int lastUnused,
                  int maxSkills) {
  // Build pool of skills with Copies > 0
  int pool[MAX_SKILLS]; // max skills
  int count = 0;
  for (int i = 0; i < maxSkills; i++) {
    if (c->skills[i].Copies > 0 && c->skills[i].Coins > 0)
      pool[count++] = i;
  }

  if (count == 0) {
      *s1 = *s2 = *s3 = -1; // fallback
    return;
  }

  // Copies = 0 keep but no more in skill pool, Copies > 0 skills normal, Copies < 0 delected skills

    if (lastUnused >= 0 && lastUnused < maxSkills && c->skills[lastUnused].Copies >= 0) {

        // เก็บ lastUnused ไว้ใน Slot 1
        *s1 = lastUnused;

        // ดึงสกิลจากช่อง Next (s3 เดิม) มาไว้ใน Slot 2 
        // แต่ต้องเช็คด้วยว่าสกิล s3 นั้นไม่ได้ถูกสั่งลบ (Copies < 0) ระหว่างเทิร์น
        if (*s3 >= 0 && *s3 < maxSkills && c->skills[*s3].Copies >= 0) {
            *s2 = *s3;
        } else {
            // ถ้า s3 เดิมถูกลบ ให้สุ่มใหม่จาก Pool (>0)
            *s2 = pickSkill(pool, count, c);
        }

        // 3. สล็อต Next (s3) จะต้องสุ่มใหม่เสมอจาก Pool (>0)
        *s3 = pickSkill(pool, count, c);

    } else {
        // กรณี lastUnused < 0 (ถูกลบ) หรือไม่มีค่า: ให้สุ่มใหม่หมดทั้ง 3 ช่องจาก Pool (>0)
        *s1 = pickSkill(pool, count, c);
        *s2 = pickSkill(pool, count, c);
        *s3 = pickSkill(pool, count, c);
    }
}










// Modified when gained new pattern
void GainNewPattern(Character *c, Character *c2) {

  if (isId(c->ID, "Lei heng") == 0) {

    if (c->Stagger > 0) {
    c->Stagger = 0;

      printf("\n%s recovers from 'Stagger'\n",
        c->name);

      sleep(1);
    }

  c2->ProtectionDown[0] += 50;

  printf("\n%s gains 1 Severing Slash [切斬] (Target takes +50%% damage) for one turn\n",
    c->name);

  sleep(1);

  int leftHP = (c->MAX_HP - c->HP)/(c->MAX_HP * 0.1);

  int healsp = 5*leftHP;
  if (healsp > 20) healsp = 20;

  if (healsp > 0) {
    updateSanity(c, healsp);

    printf("\n%s heals 5 Sanity for every 10%% missing HP on self (%d - Max 20) (Sanity %d)\n",
      c->name, healsp, c->Sanity);

    sleep(1);

  }

  }

}




// --------------------- The House of Spiders: The Ring Nursefather Hong Lu ---------------------

void CorpusTheater(Character *c, Character *c2) {

    if (c->skills[4].active <= 0) return;

  if (c->skills[7].active == 1) return; 

    // 3. ลด Stack ของ Corpus Theater
    c->skills[4].active--;
  c->skills[7].active = 1;

    // 4. สุ่มผลลัพธ์ 1 ใน 3 อย่าง (Next Turn)
    int r = rand() % 3;

    switch(r) {
        case 0:
            // ผลที่ 1: รับ Bleed Stack +3 (เพิ่มเข้าไปในเทิร์นหน้าหรือปัจจุบันตามความเหมาะสม)
              c2->Bleed[2] += 3;
            printf("\t [Corpus Theater] +3 Bleed Stack (%d) next turn (%d)", c2->Bleed[2], c->skills[4].active);
            break;
        case 1:
            // ผลที่ 2: รับ 1 Bind (ความเร็วลดลงในเทิร์นหน้า)
              c2->Bind[1] += 1;
            printf("\t [Corpus Theater] +1 Bind next turn (%d)", c->skills[4].active);
            break;
        case 2:
            // ผลที่ 3: รับ 1 Defense Power Down
              c2->DefenseSkillPowerDown[0] += 1;
            printf("\t [Corpus Theater] +1 Defense Skill Power Down next turn (%d)", c->skills[4].active);
            break;
    }
    sleep(1);
}

int countNegativeEffectTypes(Character *c) {
    int count = 0;

  // The House of Spiders: The Ring Nursefather Hong Lu - Inflict on attack Skill 3-2
  if (isId(c->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0) {

    if (c->skills[4].active > 0) count++;

    }

    // 1. นับสถานะพื้นฐาน (Potency หรือ Count ต้องมากกว่า 0)
    if (c->Burn[0] > 0 || c->Burn[1] > 0) count++;
    if (c->Bleed[0] > 0 || c->Bleed[1] > 0) count++;
    if (c->Tremor[0] > 0 || c->Tremor[1] > 0) count++;
    if (c->Rupture[0] > 0 || c->Rupture[1] > 0) count++;
    if (c->Sinking[0] > 0 || c->Sinking[1] > 0) count++;

    // 2. นับสถานะผิดปกติอื่นๆ
    if (c->Paralyze[0] > 0) count++;
    if (c->Bind[0] > 0) count++;

    // 3. นับค่าพลังที่ติดลบ (Debuff จาก Level/Power)
    if (c->OffenseLevelDown[0] > 0) count++;
    if (c->DefenseLevelDown[0] > 0) count++;
    if (c->PlusCoinPowerDrop[0] > 0) count++;   // debuff: drop plus coin power
    if (c->MinusCoinPowerDrop[0] > 0) count++;  // debuff: drop minus coin power
    if (c->AttackPowerDown[0] > 0) count++;
    if (c->FinalPowerDown[0] > 0) count++;
    if (c->BasePowerDown[0] > 0) count++;
    if (c->ClashPowerDown[0] > 0) count++;
    if (c->DamageDown[0] > 0) count++;
    if (c->CriticalDamageDown[0] > 0) count++;
    if (c->ProtectionDown[0] > 0) count++;      // Fragile (รับดาเมจแรงขึ้น)

    return count;
}

// ------------------------------------------------------------------------------------





// --------------------- Muga Ryoshu ---------------------

const char* glitchText(const char* original, int percent) {
    if (!original) return NULL;
    char* newStr = strdup(original);
    int len = strlen(newStr);
    if (len <= 0) return original;
    int toDelete = (len * percent) / 100;
    if (toDelete < 1) toDelete = 1;

    for (int i = 0; i < toDelete; i++) {
        int r = rand() % len;
        if (newStr[r] != ' ')
            newStr[r] = ' ';
    }

    return (const char*)newStr;
}

void severCoin(Character *attacker, Character *defender, SkillStats *atkSkill, SkillStats *defSkill) {
    if (!defSkill || defSkill->Coins <= 0) return;

    defSkill->Coins--;

  if (defSkill->Unbreakable > defSkill->Coins) {
      defSkill->Unbreakable = defSkill->Coins;
  }

    printf("\n\x1b[1;35m[SEVERED]\x1b[0m %s used '%s' to delete a coin!\n", attacker->name, atkSkill->name);

    // 3. ถ้าสกิลศัตรูพัง (เหรียญหมด)
    if (defSkill->Coins <= 0) {
        defSkill->Copies = -1; 

        printf("\n\x1b[1;31m[ERASED]\x1b[0m %s's Skill is collapsing...\n", defender->name);
    }
}

// ------------------------------------------------------------------------------------






// ----------------------------------------------------------------
// Combat Event structs
// ----------------------------------------------------------------

#define MAX_EVENT_OPTIONS 8

typedef struct {
    const char *label;        // ชื่อตัวเลือก
    const char *description;  // คำอธิบาย (NULL = ไม่แสดง)
} EventOption;

typedef struct {
    const char *intro;        // ข้อความเปิด        (NULL = ข้าม)
    const char *midText;      // ข้อความกลาง event  (NULL = ข้าม)
    const char *outro;        // ข้อความหลัง pass    (NULL = ข้าม)
    const char *failText;     // ข้อความหลัง fail    (NULL = ข้าม)

    EventOption options[MAX_EVENT_OPTIONS];
    int numOptions;

    int requirePowerCheck;
    int requiredPower;

    void (*onSelect)(Character *player, Character *enemy, int idx);
    void (*onFail)  (Character *player, Character *enemy, int idx);
} CombatEvent;

// ----------------------------------------------------------------
  // Power check — raw toss เท่านั้น ไม่รับบัฟใดๆ
  // ----------------------------------------------------------------
static int doPowerCheckRaw(Character *player, int *pSkill1, int *pSkill2, int *pSkill3) {

  printf("\n%s's Sanity: %d (%s)\n", player->name, player->Sanity, getSanityStatus(player));

printf("\nDashboard Skills:\n");

// สร้าง array เก็บ Index ของสกิลที่จะแสดง (มักจะมี 2 สกิล)
int skillsToShow[2] = {*pSkill1, *pSkill2};

int s_index;
for (int i = 0; i < 2; i++) {
    s_index = skillsToShow[i];
    SkillStats *s = &player->skills[s_index];

    // แก้ s->Name -> s->name
    if (s->Unbreakable > 0)
        printf("%d. %s (BasePower %d CoinPower %d Coins %d "
               "Offense %d Defense %d Unbreakable %d)\n",
               i + 1, s->name, s->BasePower, s->CoinPower,
               s->Coins, s->Offense, s->Defense, s->Unbreakable);
    else
        printf("%d. %s (BasePower %d CoinPower %d Coins %d "
               "Offense %d Defense %d Breakable)\n",
               i + 1, s->name, s->BasePower, s->CoinPower,
               s->Coins, s->Offense, s->Defense);

}

// แก้ให้ใช้ pSkill3 แทนการเขียน [2] ตรง
SkillStats *nextSkill = &player->skills[*pSkill3];

if (nextSkill->Unbreakable > 0)
  printf("Next Skill: '%s' (BasePower %d CoinPower %d Coins %d "
         "Offense %d Defense %d Unbreakable %d)\n",
         nextSkill->name, nextSkill->BasePower, nextSkill->CoinPower,
         nextSkill->Coins, nextSkill->Offense, nextSkill->Defense, nextSkill->Unbreakable);
else
  printf("Next Skill: '%s' (BasePower %d CoinPower %d Coins %d "
         "Offense %d Defense %d Breakable)\n",
        nextSkill->name, nextSkill->BasePower, nextSkill->CoinPower,
       nextSkill->Coins, nextSkill->Offense, nextSkill->Defense);

int sc;
printf("Choose skill (1-2): ");
while (1) {
    // รับ input แล้วแปลงให้ถูกต้อง
    if (scanf("%d", &sc) == 1 && sc >= 1 && sc <= 2) break;
    while (getchar() != '\n');
    printf("Invalid. Choose 1-2): ");
}

// แปลงการเลือก Input (1-based) เป็น Index (0-based) ใน array
int chosenIndex = skillsToShow[sc - 1];
SkillStats *chosen = &player->skills[chosenIndex];

    // Toss raw — BasePower + CoinPower เท่านั้น ไม่มีบัฟใดๆ
    printf("\nTossing Coins :\nPlayer:\n");
    int total = chosen->BasePower;
    printf("%-10d\n", total);
    usleep(500000);

    for (int i = 0; i < chosen->Coins; i++) {
        if (tossCoinWithSanity(player)) {
            total += chosen->CoinPower;
        }
        printf("%-10d\n", total);
        usleep(500000);
    }

    return total;
}

// ----------------------------------------------------------------
// Core runner — ใช้ได้กับทุก boss
// ----------------------------------------------------------------
// return: index ที่เลือก (0-based), -1 = fail power check
int runCombatEvent(Character *player, Character *enemy, CombatEvent *ev, int *pSkill1, int *pSkill2, int *pSkill3) {
    printf("\n\n--------------- Combat Event ---------------\n\n");

    if (ev->intro)   { printf("%s\n\n", ev->intro);   sleep(1); }

    for (int i = 0; i < ev->numOptions; i++) {
        printf("%d. %s\n", i+1, ev->options[i].label);
        if (ev->options[i].description)
            printf("Select to %s\n", ev->options[i].description);
    }

    if (ev->midText) { printf("\n%s\n\n", ev->midText); sleep(1); }

    int choice;
    printf("(Select Number 1-%d) : ", ev->numOptions);
    while (1) {
        if (scanf("%d", &choice) == 1 &&
            choice >= 1 && choice <= ev->numOptions) break;
        while (getchar() != '\n');
        printf("Invalid. Choose 1-%d: ", ev->numOptions);
    }
    int idx = choice - 1;
    printf("\nYou selected '%s'\n", ev->options[idx].label);
    sleep(1);

    if (ev->requirePowerCheck) {
        printf("\nRequired ≥ %d Power\n", ev->requiredPower);
        int total = doPowerCheckRaw(player, pSkill1, pSkill2, pSkill3);

        if (total >= ev->requiredPower) {
            printf("Check Passed!\n");
           sleep(1);
            if (ev->outro)    { printf("\n%s\n", ev->outro);    sleep(1); }
            if (ev->onSelect)   ev->onSelect(player, enemy, idx);
        } else {
            printf("Check Failed!\n");
          sleep(1);
            if (ev->failText) { printf("\n%s\n", ev->failText); sleep(1); }
            if (ev->onFail)     ev->onFail(player, enemy, idx);
            return -1;
        }
    } else {
        if (ev->outro)    { printf("\n%s\n", ev->outro);    sleep(1); }
        if (ev->onSelect)   ev->onSelect(player, enemy, idx);
    }

    return idx;
}

// ================================================================
// King in Binds — event callbacks
// ================================================================

static float KingDmgBonus = 0.0f;  // สะสม Damage Multiplier bonus
static int KingClashBonus = 0;  // สะสม Clash Power bonus

static void kingFirstEffect(Character *player, Character *enemy, int idx) {
    switch (idx) {
        case 0: { // joy — gain shield HP + Sanity

          static const char* TextChoice = 
          "You place the 'mask of joy' over your face.\n\n"
          "Confidence fills you, and nothing seems out of the realm of possibilities.\n\n"
          "The Sinners march into the banquet hall, their feet unbound by uncertainty.\n\n"
          "To the throne that could never be a respite.\n\n"
          "Let this battle bring a fleeting moment of joy for the lone-king who shall reign eternal, reign unaccompanied.\n\n"
          "Thus, the Sinners, wearing expressions of boundless joy, open the prelude to this grand banquet.";

          if (TextChoice) { printf("\n%s\n", TextChoice); sleep(3); }

          updateSanity(player, 20);

          int sh = (int)(player->MAX_HP * 0.4f);
          player->Shield += sh;
          printf("\n%s gains %d Shield HP (40%% of Max HP) (Shield %.2f)\n",
                 player->name, sh, player->Shield);

          printf("\n%s heals 20 Sanity (%d)\n",
                 player->name, player->Sanity);

           sleep(1);

            break;
        }
        case 1: { // sorrow — Clash Power + Sanity

          static const char* TextChoice = 
          "You place the 'mask of sorrow' over your face.\n\n"
            "Confidence fills you, and nothing seems out of the realm of possibilities.\n\n"
            "The Sinners march into the banquet hall, their feet unbound by uncertainty.\n\n"
            "To the throne that could never be a respite.\n\n"
            "Let this battle bring a fleeting moment of joy for the lone-king who shall reign eternal, reign unaccompanied.\n\n"
            "Thus, the Sinners, wearing expressions of boundless joy, open the prelude to this grand banquet.";

          if (TextChoice) { printf("\n%s\n", TextChoice); sleep(3); }

          KingClashBonus += 1;
              updateSanity(player, 20);
              printf("\n%s gains Clash Power +1 for this Encounter\n", player->name);
            printf("\n%s heals 20 Sanity (%d)\n", player->name, player->Sanity);

           sleep(1);

              break;
          }
        case 2: { // wrath — Damage Multiplier + Sanity

          static const char* TextChoice = 
          "You place the 'mask of wrath' over your face.\n\n"
            "Confidence fills you, and nothing seems out of the realm of possibilities.\n\n"
            "The Sinners march into the banquet hall, their feet unbound by uncertainty.\n\n"
            "To the throne that could never be a respite.\n\n"
            "Let this battle bring a fleeting moment of joy for the lone-king who shall reign eternal, reign unaccompanied.\n\n"
            "Thus, the Sinners, wearing expressions of boundless joy, open the prelude to this grand banquet.";

          if (TextChoice) { printf("\n%s\n", TextChoice); sleep(3); }

            KingDmgBonus += 0.1f;
            updateSanity(player, 20);
            printf("\n%s all skills gains +0.1 Damage Multiplier for this Encounter\n", player->name);
            printf("\n%s heals 20 Sanity (%d)\n", player->name, player->Sanity);

           sleep(1);


            break;
        }
    }
}

static void kingMidEffect(Character *player, Character *enemy, int idx) {
    switch (idx) {
        case 0: { // joy — heal Sanity

          static const char* TextChoice = 
          "You wear the 'mask of joy' over your face.\n\n"
            "Your thoughts become refreshingly clear.";

          if (TextChoice) { printf("\n%s\n", TextChoice); sleep(2); }

            updateSanity(player, 15);
            printf("\n%s heals 15 Sanity (%d)\n", player->name, player->Sanity);

           sleep(1);

            break;
        }
        case 1: { // sorrow — Heal HP

          static const char* TextChoice = 
          "You wear the 'mask of sorrow' over your face.\n\n"
            "The Sinners' hearts absorb its strength and determination like sponges.";

          if (TextChoice) { printf("\n%s\n", TextChoice); sleep(2); }

          int heal = (int)(player->MAX_HP * 0.3f);
          player->HP += heal;
          if (player->HP > player->MAX_HP) player->HP = player->MAX_HP;
              printf("\n%s heal 30%% of Max HP (%d) (HP %.2f)\n", player->name, heal, player->HP);

           sleep(1);


              break;
          }
        case 2: { // wrath — Damage Up

          static const char* TextChoice = 
          "You wear the 'mask of wrath' over your face.\n\n"
            "Passion compels the Sinners' hands to fiercely grasp their weapons.";

          if (TextChoice) { printf("\n%s\n", TextChoice); sleep(2); }

            KingDmgBonus += 0.1f;
          printf("\n%s all skills gains +0.1 Damage Multiplier for this Encounter\n", player->name);

           sleep(1);

            break;
        }
    }
}

static void kingMidFail(Character *player, Character *enemy, int idx) {
    printf("\n'Nothing happened'\n");

  sleep(1);
}

// ----------------------------------------------------------------
// King in Binds event definitions
// ----------------------------------------------------------------
static CombatEvent kingFirstEvent = {
    .intro =
        "An empty banquet hall, wreathed in ambient crimson light.\n\n"
        "The solitary king, bound to his throne, raises his arm in an unhurried motion.\n\n"
        "Your feet grow drenched in the cold mist of a vague tune.\n\n"
        "The lone king gently points to a platform near you with an inviting cadence.\n\n"
        "On it are masks, each depicting different spectrum of emotions.\n\n"
        "There is a mask of joy. A mask of sorrow. A mask of wrath.\n\n"
        "What face shall be worn in this audience with the king?",
    .midText  = NULL,
    .outro    = NULL,
    .failText = NULL,
    .options  = {
        {"Wear the mask of joy.",
         "gain Sheild HP, Sanity heal for this Encounter."},
        {"Wear the mask of sorrow.",
         "gain increased Clash Power, Sanity heal for this Encounter."},
        {"Wear the mask of wrath.",
         "deal more damage, gain Sanity heal for this Encounter."},
    },
    .numOptions        = 3,
    .requirePowerCheck = 0,
    .onSelect          = kingFirstEffect,
    .onFail            = NULL,
};

static CombatEvent kingMidEvent = {
    .intro =
        "Passion grows hotter as the banquet slowly reaches a crescendo.\n\n"
        "The King in Binds motions for the battle to pause.\n\n"
        "This gives us enough time to leave our broken masks and don new ones for ourselves.\n\n"
        "What new face shall we wear in audience with the king?",
    .midText =
        "Donning the mask once again summons the ardent emotions that slowly simmer into your heart.\n\n"
        "The sorrow rises through the fog of heated battle; its dense weight pierces deep, deep into your emotions.",
    .outro =
        "The sorrowful pressure slowly relents.\n\n"
        "The banquet is far from over.\n\n"
        "The Sinners each rise and shake off the gloom; the void in their hearts filled with determination.",
    .failText =
        "The deluge of sorrow sweeps the Sinners off their feet into an even deeper chasm.\n\n"
        "Yet, the banquet continues still.\n\n"
        "The Sinners repress their sorrows and step forward.",
    .options = {
        {"Wear the mask of joy.",    "heal Sanity."},
        {"Wear the mask of sorrow.", "heal HP."},
        {"Wear the mask of wrath.",  "deal more damage for this Encounter."},
    },
    .numOptions        = 3,
    .requirePowerCheck = 1,
    .requiredPower     = 12,
    .onSelect          = kingMidEffect,
    .onFail            = kingMidFail,
};














void clearDebuffsOnDeath(Character *defender, Character *attacker) {
    if (defender == NULL) return;

  clearTurnEffects(defender); // for this turn effect
  clearTurnEffects(defender); // for next turn effect

  defender->Shield = 0; 

  // Status
  defender->Burn[0] = 0; // [0] = Stack, [1] = Count
  defender->Burn[1] = 0; // [0] = Stack, [1] = Count
  defender->Bleed[0] = 0; // [0] = Stack, [1] = Count
  defender->Bleed[1] = 0; // [0] = Stack, [1] = Count
  defender->Tremor[0] = 0; // [0] = Stack, [1] = Count
  defender->Tremor[1] = 0; // [0] = Stack, [1] = Count
  defender->Tremor[4] = 0; // Tremor store / Stagger Threshold
  defender->Rupture[0] = 0; // [0] = Stack, [1] = Count
  defender->Rupture[1] = 0; // [0] = Stack, [1] = Count
  defender->Sinking[0] = 0; // [0] = Stack, [1] = Count
  defender->Sinking[1] = 0; // [0] = Stack, [1] = Count
  defender->Poise[0] = 0; // [0] = Stack, [1] = Count
  defender->Poise[1] = 0; // [0] = Stack, [1] = Count
  defender->Charge[0] = 0; // [0] = Stack, [1] = Count
  defender->Charge[1] = 0; // [0] = Stack, [1] = Count

  // ------------ Player ------------

  // The House of Spiders: The Index Nursefather Yi Sang - Tremor Burst
  if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {
      if (attacker->skills[12].active > 0) {
          attacker->skills[12].active = 0;
      }
  }

    // Binah - Fairy
    if (isId(attacker->ID, "Binah") == 0) {
        if (attacker->skills[0].active > 0) {
            attacker->skills[0].active = 0;
        }
    }

    // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly
    if (isId(attacker->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0) {
        if (attacker->skills[0].active > 0) {
            attacker->skills[0].active = 0;
        }
    }

  // The Middle Little Brother Sinclair - Marks
  if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0) {
      attacker->skills[0].active = 0; // Vendetta Mark
  }


  // The One Who Grips Faust
  if (isId(attacker->ID, "The One Who Grips Faust") == 0) {
      attacker->skills[0].active = 0; // Bleed Stack
    attacker->skills[1].active = 0; // Bleed Count
    attacker->skills[2].active = 0; // Nail
    attacker->skills[4].active = 0; // Gaze
  }


  // Muga Ryōshū
  if (isId(attacker->ID, "Muga Ryōshū") == 0) {
      attacker->skills[0].active = 0; // Sever the thread
  }

  // The House of Spiders: The Ring Nursefather Hong Lu
  if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0) {
      attacker->skills[4].active = 0;  // Corpus Theater Stacks (เริ่มที่ 3)
  }


  // ------------ Boss ------------

}











void defensePhase(Character *c, SkillStats *ds, Character *c2, SkillStats *ds2) {
    if (c->HP <= 0 || isStaggered(c) || isPanicked(c)) return;

  int powerUp = 0;

    printf("\n--- Defense Phase ---\n");
    printf("%s prepares defense with '%s'\n", c->name, ds->name);

    // แสดงรายละเอียดเหรียญ
    printf("Tossing %d Coins for Shield:\n", ds->Coins);
    printf("%-10s %-10s %-10s", "Coin", "Power", "Shield");

    int totalPower = ds->BasePower + ds->BasePowerBoost[0] + ds->FinalPowerBoost[0] + ds->DefensePowerBoost[0] + ds->DefenseSkillPowerBoost[0] + ((c->BasePowerUp[0] - c->BasePowerDown[0]) + (c->FinalPowerUp[0] - c->FinalPowerDown[0]) + (c->DefensePowerUp[0] - c->DefensePowerDown[0]) + (c->DefenseSkillPowerUp[0] - c->DefenseSkillPowerDown[0]));

    for (int i = 0; i < ds->Coins; i++) {
        int isHead = tossCoinWithSanity(c);
        int coinPowerResult = 0;

      int defTempDefense = (c->DefenseLevelUp[0] - c->DefenseLevelDown[0]) + ds->Defense;
      int atkTempOffense = (c2->OffenseLevelUp[0] - c2->OffenseLevelDown[0]) + ds2->Offense;

      int defenseDiff = defTempDefense - atkTempOffense;
      if (defenseDiff > 0) {
          powerUp += (defenseDiff / 3);
      }

        if (isHead) {
            // เช็ค Paralyze (อัมพาต) เหมือนตอนโจมตี
            if (c->Paralyze[0] > 0) {
                coinPowerResult = 0;
                c->Paralyze[0]--;
            } else {
                {
                  int charCoinBoostG = 0;
                  if (ds->CoinPower >= 0) {
                    charCoinBoostG += c->PlusCoinPowerBoost[0] - c->PlusCoinPowerDrop[0];
                  } else {
                    charCoinBoostG +=  c->MinusCoinPowerDrop[0] - c->MinusCoinPowerBoost[0];
                  }
                  coinPowerResult = ds->CoinPower + ds->CoinPowerBoost[0] + ds->FinalPowerBoost[0] + powerUp + (c->FinalPowerUp[0] - c->FinalPowerDown[0]) + (c->FinalPowerUp[0] - c->FinalPowerDown[0]) + charCoinBoostG;
                }
            }
        }

        totalPower += coinPowerResult;
        if (totalPower < 0) totalPower = 0;

        // ใน Limbus Company ค่าพลังที่ทอยได้ของ Guard คือค่า Shield ที่ได้รับ
        printf("\n%-10d %-10d %-10d", i + 1, totalPower, totalPower);

        usleep(500000); // หน่วงเวลาเพื่อให้ดูเหมือนทอยจริง
    }

    c->TempShield += totalPower;
    printf("\n(%d bonus) %s gained %d Shield! (Current Total Shield: %.2f)\n", 
           powerUp, c->name, totalPower, c->Shield + c->TempShield);

    sleep(1);
}











// ----------------------Attack phase-------------------------------
void attackPhase(Character *attacker, SkillStats *atk, int atkTempOffense,
                 int atkTempDefense, Character *defender, SkillStats *defSkill,
                 int defTempOffense, int defTempDefense, int remainingCoins,
                 int Unbreakable, int clashCount) {
  // printf("\n%s attacks %s with %s\n", attacker->name, defender->name,
  // atk->name);

  int IsunableAttackertoact = isStaggered(attacker);
  int IsunableDefensetoact = isStaggered(attacker);

  int randomVar = 0; // For some var taunt etc. Rodion thumb

  int nodefense = 0;

  if (attacker->HP > 0 && !IsunableAttackertoact) {

  if (remainingCoins <= 0) {
    printf("\nNo coins left to attack.\n");
    return;
  }

    int ClashLostAttack = 0; // ← Character's Cracking Coins in after attack
    int initialCrackedCount = Unbreakable; // save for use in some character's effect (This value isn't same as 'Unbreakable' in this function cause 'Unbreakable' is change in this function by -1 every loop but this doesn't change)

    if (atk->Unbreakable > 0 && Unbreakable == atk->Unbreakable) {
      ClashLostAttack = 1;
    }

    // The House of Spiders: The Thumb Nursefather Rodion - Precognition attack
    if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && atk == &attacker->skills[1]) {

      nodefense = 1;

    }

    // Clashable Guard

    int powerReduction = 0;
    
    int DefenseBonus = 0;
    
    if (defSkill != NULL && defSkill->skillType == 4 && !IsunableDefensetoact && !ClashLostAttack && defSkill->active == 1 && !nodefense) {
      
      int powerReduction = defSkill->BasePower + defSkill->BasePowerBoost[0];

      defSkill->active = 0;
      
        printf("\n--- Defense Phase ---\n");
        printf("%s prepare to mitigate damage with %s: %s (Cracking Coin fixed Coin Power to 1)\n", defender->name, getSkillTypeName(defSkill->skillType), defSkill->name);

        // 1. คำนวณพลังพื้นฐาน (Base + Level Bonus + Buff)
            DefenseBonus = defSkill->FinalPowerBoost[0] + defSkill->DefensePowerBoost[0] + defSkill->DefenseSkillPowerBoost[0] + ((defender->BasePowerUp[0] - defender->BasePowerDown[0]) + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + (defender->DefensePowerUp[0] - defender->DefensePowerDown[0]) + (defender->DefenseSkillPowerUp[0] - defender->DefenseSkillPowerDown[0]));

        int defenseDiff = defTempDefense - atkTempOffense;
        if (defenseDiff > 0) {
                    DefenseBonus += (defenseDiff / 3);
        }

        powerReduction += DefenseBonus;

        // 2. ช่วง Tossing Coins (Visual Part)
        printf("Tossing %d Coins (Fixed Coin Power: 1):\n", defSkill->Coins);
        printf("%-10s %-10s %-10s", "Coin", "Result", "Power");

        for (int j = 0; j < defSkill->Coins; j++) {
            int isHead = tossCoinWithSanity(defender);
            if (isHead) {
                if (attacker->Paralyze[0] > 0) { // ← Character's paralyze
                    powerReduction += 0;
                    defender->Paralyze[0]--;
                  }
                } else {
                    powerReduction += 1;
                  if (powerReduction <= 0) powerReduction = 0;
                }

            printf("\n%-10d %-10s %-10d", j + 1, (isHead ? "Heads" : "Tails"), powerReduction);
            usleep(400000); 
        }

        printf("\n(%d bonus) %s total Defense power: %d\n", DefenseBonus, defender->name, powerReduction);
        sleep(1);
    }





  //--------------------------- Before Attack Buff ----------------------------

    // Lei heng – Prey active: apply Clash Power -5, -50% atk dmg, -75% dmg taken
    if (isId(defender->ID, "Lei heng") == 0 && defender->skills[6].active == 1 && atk->skillType == 0) {
          atk->DamageUp[0] -= 50;
        printf("\n%s equipped Attack Skills, deal -50%% Damage with Attack Skill\n", attacker->name);
        sleep(1);
    }
    else if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[6].active == 1 && defSkill->skillType != 0) {
        defender->ProtectionUp[0] += 75;
      printf("\n%s equipped Defense Skills, take -75%% Damage\n", defender->name);
      sleep(1);
      }

    // Lei heng - Unopposed Attacks deal more damage
    if (isId(attacker->ID, "Lei heng") == 0 && clashCount <= 0 && defSkill->skillType == 0) {
      printf("\n%s doing 'Unopposed Attacks', deal +30%% damage On Hit\n", attacker->name);

      sleep(1);
    }

    // The House of Spiders: The Thumb Nursefather Rodion - Skill 3-1 lose
    if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (atk == &attacker->skills[4]) && attacker->skills[11].active == 0) {

      int lost = 10;

      int canLose = 10 - attacker->skills[6].active; // โควต้าที่เหลือในเทิร์นนี้

      int actualLoss = (lost > canLose) ? canLose : lost;
      if (actualLoss > attacker->Passive) actualLoss = attacker->Passive;

        attacker->Passive -= actualLoss;
        attacker->skills[6].active += actualLoss; // บันทึกว่าเสียไปเท่าไหร่แล้วในเทิร์นนี้

        printf("\n%s loses up to 10 Eye of Precognition Stack on self (%d)\n", attacker->name, attacker->Passive);

      sleep(1);

      if (attacker->Passive <= 0 && attacker->skills[11].active == 0) {
          attacker->skills[11].active = 1; // ติด Overheat

        printf("\n%s converts 'Eye of Precognition' into 'Eye of Precognition - Overheat'\n", attacker->name);

        sleep(1);
      }
      }

    // The House of Spiders: The Thumb Nursefather Rodion - Skill 3-2 damage Buff
    if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && atk == &attacker->skills[5]) {

      printf("\nThis %s's Skill while attacking, if target has 50%%- current HP, deal +50%% damage\n", attacker->name);

    }

    // ------------------- The Middle Nursefather - Matthias -------------------

    if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk->skillType == 3) {
        int grudge = attacker->skills[0].active;
        float counterBonus = grudge * 5.0f;

      if (counterBonus > 0) {
        atk->DamageUp[0] += counterBonus;
        printf("\n%s's Counter Skill deals +%.0f%% damage\n", attacker->name, counterBonus);

      sleep(1);
      }
    }

    if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->defenseSkill[0] || atk == &attacker->skills[3] || atk == &attacker->skills[8]) && (attacker->skills[9].active >= 1) && attacker->HP <= attacker->MAX_HP*0.5) {

        atk->DamageUp[0] += 40;
        printf("\n%s at 50%% or less HP, deal +40%% damage\n", attacker->name);

      sleep(1);

    }

    if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[9]) && attacker->HP <= attacker->MAX_HP*0.5) {

        atk->DamageUp[0] += 60;
        printf("\n%s at 50%% or less HP, deal +60%% damage\n", attacker->name);

      sleep(1);

    }

    if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->defenseSkill[0] || atk == &attacker->skills[7] || atk == &attacker->skills[8] || atk == &attacker->skills[9]) && (attacker->skills[9].active == 3)) {

        inflictStatus(attacker->Burn, atk->Coins * 5, 0, 0, 99, 0, 99);
        printf("\n%s gains (# of Coins in this Skill x 5) Burn Stack (%d)\n", attacker->name, attacker->Burn[0]);

      sleep(1);

    }

    // ----------------------------------------------------------------------------

    // Heishou Pack - You Branch Adept Heathcliff - Battleblood Instinct buff
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && attacker->Passive > 0) {

    float gain = attacker->Passive * 0.75f;

      atk->DamageUp[0] += gain;

      printf("\n%s deals +0.75%% damage(%.2f%%) for every Battleblood Instinct Stack (%d)\n", attacker->name, gain, attacker->Passive);

    sleep(1);
    }

    // The Middle Little Brother Sinclair - Passive Buff Mark
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && attacker->skills[0].active > 0) {

      int takevalue = attacker->skills[0].active * 2;
      if (takevalue > 20) takevalue = 20;

       defender->ProtectionDown[0] += takevalue;

      printf("\n%s takes 2%% more damage for every 'Vendtta Mark' on self (%d%% - Max 20%%)\n", defender->name, takevalue);

      sleep(1);
    }

    // Sukuna:King of Curse Chanting
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && atk == &attacker->skills[4]) {

      attacker->skills[4].active++; // Chant Count

    }

    // Sukuna:King of Curse - Skill 3
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[2])) {

      printf("\n%s's last coin count as 'Cursed Technique'\n", attacker->name);

      sleep(1);
    }

    // Sukuna:King of Curse - Skill 7
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[6])) {

      printf("\n%s's first and second coins count as 'Cursed Technique'\n", attacker->name);

      sleep(1);
    }

  // Binah lost
  if (isId(defender->ID, "Binah") == 0 && ClashLostAttack && defender->Passive) {

      defender->ProtectionUp[0] += 80;

    int shieldGain = 50 + ((defender->MAX_HP - defender->HP)/3);
    if (shieldGain > 100) shieldGain = 100;

    defender->TempShield += shieldGain;

    printf("\n%s getting attack by Full Cracking Coins, take -80%% damage and gain (50 + Missing HP/3) (%d - Max 100) Shield (%.2f)\n", defender->name, shieldGain, defender->Shield + defender->TempShield);
  }
    else if (isId(defender->ID, "Binah") == 0 && defender->Passive && defender->Sanity >= 0) {

    updateSanity(defender, -10);

    int shieldGain = 100 + ((defender->MAX_HP - defender->HP)/2);
      if (shieldGain > 300) shieldGain = 300;

      defender->TempShield += shieldGain;

    printf("\n%s at 0+ Sanity, consumes 10 Sanity(%d) to gain (100 + Missing HP/2) (%d - Max 300) Shield (%.2f)\n", defender->name, defender->Sanity, shieldGain, defender->Shield + defender->TempShield);
  }

    // ---------------------------------- Dawn Office Fixer Sinclair ----------------------------------------

  // Dawn Office Fixer Sinclair - Skill Buff base form
  if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && !attacker->skills[3].active && attacker->Sanity >= 45) {

    attacker->FinalPowerUp[0] += 3;

    printf("\n%s at 45 Sanity, gains 3 Final Power\n",
           attacker->name);

    sleep(1);

  }

    if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && attacker->Sanity >= 45) {

      atk->CoinPowerBoost[0] += 3;

      printf("\n%s at 45 Sanity, gains 3 Coin Power\n",
             attacker->name);

      sleep(1);

    }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S2 45 sp
  if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && attacker->Sanity >= 45 && (atk == &attacker->skills[1])) {

    attacker->AttackPowerUp[0] += 2;

    printf("\n%s at 45 Sanity, gains 2 Attack Power\n",
           attacker->name);

    sleep(1);

  }

    // Dawn Office Fixer Sinclair - Skill Buff ego form S3 45 sp
  if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && attacker->Sanity >= 45 && (atk == &attacker->skills[3] || atk == &attacker->skills[2])) {

    attacker->AttackPowerUp[0] += 5;

    printf("\n%s at 45 Sanity, gains 5 Attack Power\n",
           attacker->name);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S4
  if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && (atk == &attacker->skills[3] || atk == &attacker->skills[2]) && attacker->skills[3].active) {

      int boost = (abs(attacker->Sanity)) * 5;

      atk->DamageUp[0] += boost;

      printf("\n%s gains +5%% Damage (%d%%) for every Sanity further from 0 (%d Sanity)\n",
             attacker->name, boost, attacker->Sanity);

      sleep(1);

  }

    // ------------------------------------------------------------------------

  // Meursault:The Thumb Unbreakable BUff
  if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (attacker->skills[2].active >= (attacker->defenseSkill[3].active + (attacker->defenseSkill[3].active/2))) && attacker->Passive <= 0 && attacker->skills[3].active && ClashLostAttack) {

    float missing = (attacker->MAX_HP - attacker->HP) / attacker->MAX_HP; // fraction of HP missing (0.0 - 1.0)
    int SkillUp = ((int)(missing * 100.0f)) + 75;  // 75% + missing
    if (SkillUp > 150) SkillUp = 150;      // cap at 50%

      atk->DamageUp[0] += SkillUp;

       printf("\n%s on Clash Lost, Unbreakable Coins of this unit's Attack Skills deal +(75 + missing HP percentage on self)%% damage(%d%% - Max 150%%)\n", attacker->name, SkillUp);

    sleep(1);

  }

    // Meursault:The Thumb 20+ BUff
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (attacker->skills[2].active >= (attacker->defenseSkill[3].active + attacker->defenseSkill[3].active)) && attacker->Passive <= 0 && attacker->skills[3].active) {
    float TargetHP = (defender->HP / defender->MAX_HP) * 100; // fraction of HP missing (0.0 - 1.0)
      float UnitHP = (attacker->HP / attacker->MAX_HP) * 100; // fraction of HP missing (0.0 - 1.0)

      if (attacker->skills[2].active >= 20 && TargetHP > UnitHP) {

        float missingdifferent = TargetHP - UnitHP;

          int SkillUp = (int)(missingdifferent * 100);
          if (SkillUp > 50) SkillUp = 50;      // cap at 50%

          atk->DamageUp[0] += SkillUp;

        printf("\n%s deals +(HP percentage difference)%% damage against targets with higher remaining HP percentage than this unit (%d%% - Max 50%%)\n", attacker->name, SkillUp);

      sleep(1);

      }

    }

  // Jia Qiu - S3 and S6 S10
  if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[10] || atk == &attacker->skills[3] || atk == &attacker->skills[6])) {

    int boost = rand() % 30 + 1;

      atk->DamageUp[0] += boost;

    printf("\n%s deals %d%% more damage\n", attacker->name, boost);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
     (atk == &attacker->skills[2])) {

    int gain = 12 * attacker->Passive;

    atk->DamageUp[0] += gain;

    printf("\n%s deals 12%% more damage(%d%%) for every Coffin (%d)\n",
       attacker->name, gain, attacker->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
     (atk == &attacker->skills[3])) {

    int gain = 10 * attacker->Passive;

    atk->DamageUp[0] += gain;

    printf("\n%s deals 10%% more damage(%d%%) for every Coffin (%d)\n",
       attacker->name, gain, attacker->Passive);

      sleep(1);
  }

    // Heathcliff:Wild Hunt – buff Dullahan
    if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
       (atk == &attacker->skills[3])) {

      int gain = 20 * attacker->skills[0].active;

      atk->DamageUp[0] += gain;

      printf("\n%s deals 20%% more damage(%d%%) for every Dullahan (%d)\n",
         attacker->name, gain, attacker->skills[0].active);

        sleep(1);
    }

  // FireFist – Attack buff s1
  if (isId(attacker->ID, "Gregor:Firefist") == 0 && atk == &attacker->skills[0]) {

    int consumed = 15;

    attacker->skills[3].active += attacker->Passive >= consumed ? consumed : attacker->Passive; // store for passive

      int prevPassive = attacker->Passive;  // store before change
      attacker->Passive -= attacker->Passive >= consumed ? consumed : attacker->Passive;
    if (attacker->Passive < 0)
      attacker->Passive = 0;

    if (prevPassive > 50 && attacker->Passive > 50) {
      atk->DamageUp[0] += 10;
      printf("\n%s consumes %d District 12 Fuel(%d) to deal more 10%% damage\n", attacker->name, consumed, attacker->Passive);

      } else if (prevPassive > 50 && attacker->Passive <= 50) {
      atk->DamageUp[0] += 10;
       printf("\n%s consumes %d District 12 Fuel(%d) to deal more 10%% damage\n", attacker->name, consumed, attacker->Passive);
      sleep(1);
          printf("\nDistrict 12 Fuel becomes Overheated Fuel\n");
      } else {
      atk->DamageUp[0] += 20;
        printf("\n%s consumes %d Overheated Fuel(%d) to deal more 20%% damage\n", attacker->name, consumed, attacker->Passive);
      }

      sleep(1);
  }

// FireFist – Attack buff s2
if (isId(attacker->ID, "Gregor:Firefist") == 0 && atk == &attacker->skills[1]) {

  int consumed = 20;

  attacker->skills[3].active += attacker->Passive >= consumed ? consumed : attacker->Passive; // store for passive

    int prevPassive = attacker->Passive;  // store before change
    attacker->Passive -= attacker->Passive >= consumed ? consumed : attacker->Passive;
  if (attacker->Passive < 0)
    attacker->Passive = 0;

  if (prevPassive > 50 && attacker->Passive > 50) {
    atk->DamageUp[0] += 15;
    printf("\n%s consumes %d District 12 Fuel(%d) to deal more 15%% damage\n", attacker->name, consumed, attacker->Passive);

    } else if (prevPassive > 50 && attacker->Passive <= 50) {
    atk->DamageUp[0] += 15;
     printf("\n%s consumes %d District 12 Fuel(%d) to deal more 15%% damage\n", attacker->name, consumed, attacker->Passive);

    sleep(1);

        printf("\nDistrict 12 Fuel becomes Overheated Fuel\n");
    } else {
    atk->DamageUp[0] += 30;
      printf("\n%s consumes %d Overheated Fuel(%d) to deal more 30%% damage\n", attacker->name, consumed, attacker->Passive);
    }

    sleep(1);
}

// FireFist – Attack buff s3
if (isId(attacker->ID, "Gregor:Firefist") == 0 && (atk == &attacker->skills[2])) {

    int prevPassive = attacker->Passive;  // store before change
    int consumed = ((attacker->Passive - 25) > 0) ? 25 : attacker->Passive ;

  attacker->skills[3].active += consumed; // store for passive

  int boost = (int)((0.02 * consumed) * 100);  // 2% per missing fuel


  if (prevPassive > 50 && (attacker->Passive - consumed) > 50) {

    if (boost > 50) boost = 50;                    // cap at 50%
    atk->DamageUp[0] += boost;

     printf("\n%s consumes up to 25 District 12 Fuel(%d) to deal +2%% damage for every District 12 Fuel consumed (%d%% - Max 50%%)\n", attacker->name, ((attacker->Passive - consumed) < 0) ? 0 : (attacker->Passive - consumed), boost);

    } else if (prevPassive > 50 && (attacker->Passive - consumed) <= 50) {

    if (boost > 50) boost = 50;                    // cap at 50%
    atk->DamageUp[0] += boost;

     printf("\n%s consumes up to 25 District 12 Fuel(%d) to deal +2%% damage for every District 12 Fuel consumed (%d%% - Max 50%%)\n", attacker->name, ((attacker->Passive - consumed) < 0) ? 0 : (attacker->Passive - consumed), boost);

    sleep(1);

        printf("\nDistrict 12 Fuel becomes Overheated Fuel\n");
    } else {

    boost = (int)((0.04 * consumed) * 100);  // 4% per missing fuel
    if (boost > 100) boost = 100;                    // cap at 100%
    atk->DamageUp[0] += boost;

      printf("\n%s consumes up to 25 Overheated Fuel(%d) to deal +4%% damage for every District 12 Fuel consumed (%d%% - Max 100%%)\n", attacker->name, ((attacker->Passive - consumed) < 0) ? 0 : (attacker->Passive - consumed), boost);

    }

  sleep(1);

  if (consumed > 0) {

    attacker->DamageUp[1] += 20;

    printf("\n%s consumed Fuel, +20%% damage next turn\n", attacker->name);

  }
}

  // Heathcliff:Wild Hunt – skill 3
  if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
      (atk == &attacker->skills[3])) {

    int missingSanity = 45 - attacker->Sanity;
    if (missingSanity < 0) missingSanity = 0;        // Sanity above 45 gives no bonus

    double boost = 0.003 * missingSanity;           // 0.3% per missing Sanity
    if (boost > 0.21) boost = 0.21;                // cap at 21%

    // Apply boost to DamageUp (or a separate variable)
    atk->DamageUp[0] += (int)(boost * 100);

    printf("\n%s deals more damage the further this unit's Sanity value is from 45 (+0.3%% damage for every missing Sanity) (%d%% - Max 21%%)\n",
       attacker->name, (int)(boost * 100));

    sleep(1);
  }

    // ------------------ Meursault:Blade Lineage Mentor ------------------

    // Meursault:Blade Lineage Mentor - Shadow-vested Bladesinger [着影揮刀]
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && atk == &attacker->defenseSkill[1] && attacker->Passive == -1 && attacker->defenseSkill[2].active == 1) {

        printf("\n%s: \"Awash in the yearning moonlight,\"\n", attacker->name);

      sleep(1);

        // Effect A: Gain +20 Poise Potency and +4 Poise Count
        inflictStatus(attacker->Poise, 20, 4, 0, 99, 0, 99);
        printf("\n%s gains +20 Poise Stack (%d) and +4 Poise Count (%d)\n", attacker->name, attacker->Poise[0], attacker->Poise[1]);

      sleep(1);

      int bonusCritMult = (attacker->Poise[0] * 2);
      if (bonusCritMult > 100) bonusCritMult = 100;
        atk->CriticalDamageUp[0] += bonusCritMult;
      printf("\n%s deals +%d%% damage on Critical Hit (Max 100%%)\n", attacker->name, bonusCritMult);

      sleep(1);

        // Effect C: To Claim Their Bones gains +(Poise Potency / 5) Final Power
        int finalPowerBonus = attacker->Poise[0] / 5;
        if (finalPowerBonus > 10) finalPowerBonus = 10;
          atk->FinalPowerBoost[0] += finalPowerBonus;
        printf("\n%s gains +%d Final Power (Max 10)\n", attacker->name, finalPowerBonus);

        sleep(1);

      printf("\n%s's this Skill is not affected by Paralyze\n", attacker->name);

      sleep(1);

      printf("\n%s: \"I hew your throat and claim the breath that remains!\"\n", attacker->name);

      sleep(1);
    }

  // Meursault:Blade Lineage Mentor - Yield my flesh
  if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 &&
      (atk == &attacker->skills[2])) {

      int DamageBuff = 3 * attacker->Poise[0];
    if (DamageBuff > 75) DamageBuff = 75;

    atk->DamageUp[0] += DamageBuff;

      printf("\n%s deals +3%% damage for Poise Stack (%d) on self (%d%% - Max 75%%)\n", attacker->name, attacker->Poise[0], DamageBuff);

      sleep(1);

  }

    // Meursault:Blade Lineage Mentor - To claim thier bones
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 &&
        (atk == &attacker->defenseSkill[1])) {

        int DamageBuff = 2 * attacker->Poise[0];
      if (DamageBuff > 50) DamageBuff = 50;

      atk->DamageUp[0] += DamageBuff;

        printf("\n%s deals +2%% damage for Poise Stack (%d) on self (%d%% - Max 50%%)\n", attacker->name, attacker->Poise[0], DamageBuff);

        sleep(1);

    }

  // Meursault:Blade Lineage Mentor - Deal more on missing
  if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 &&
      (atk == &attacker->defenseSkill[0] || atk == &attacker->defenseSkill[1] || atk == &attacker->skills[2])) {

    float missingHPPercent = ((float)(attacker->MAX_HP - attacker->HP) / attacker->MAX_HP) * 100.0f;

    int fullMissingPercent = (int)missingHPPercent;

    float dmgBonus = (float)fullMissingPercent * 0.5f;

    // 4. Apply the maximum limit
    if (dmgBonus > 25.0f) {
        dmgBonus = 25.0f;
    }

    atk->DamageUp[0] += dmgBonus;

    printf("\n%s deals +0.5%% damage for every 1%% missing HP on self(%.1f%% - Max 25%%)\n",
        attacker->name, dmgBonus);
  }

    // Meursault:Blade Lineage Mentor - Overthrow skill
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 &&
        (atk == &attacker->defenseSkill[0])) {

     attacker->FinalPowerUp[1] += 1;

      printf("\n%s gains 1 Final Power Up next turn\n",
          attacker->name);
    }

    // ------------------------------------------------------------------------

  // Yi sang:Fell Bullet - Torn Memory
  if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 &&
      (atk == &attacker->skills[0] ||
       atk == &attacker->skills[1])) {

    printf("\n%s deals more +15%% damage(%d%%) for every Torn Memory(%d)\n",
           attacker->name, attacker->Passive * 15, attacker->Passive);

    sleep(1);
  }

    // Hong lu:The Lord of Hongyuan - Skill 3 deal more damage on HP
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[4] || atk == &attacker->skills[3])) {

      float missingSelf  = (float)(attacker->MAX_HP  - attacker->HP)  / attacker->MAX_HP * 100.0f;
      float missingEnemy = (float)(defender->MAX_HP - defender->HP) / defender->MAX_HP * 100.0f;

      float damageboost = (missingSelf + missingEnemy) / 1.0f;
      if (damageboost > 50.0f) damageboost = 50.0f;

      atk->DamageUp[0] += damageboost;

        printf("\n%s deals +1%% damage for every 1%% (missing HP percentage on target + missing HP percentage on self) (%.0f%% - Max 50%%)\n", attacker->name, damageboost);

      sleep(1);
    }

  // Hong lu:The Lord of Hongyuan - S3 Buff
  if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      (atk == &attacker->skills[3] || atk == &attacker->skills[4])) {

    int buff = 10;

    if (atk == &attacker->skills[3]) buff = 20;

    atk->DamageUp[0] += attacker->Passive*buff;

    printf("\n%s deals more +%d%% damage (%d%%) for every Heishou Bolus Contamination "
           "[黑獸丸染] (%d)\n",
           attacker->name, buff, attacker->Passive*buff, attacker->Passive);

    sleep(1);
  }

    // Hong lu:The Lord of Hongyuan - Skill 3-1/3-2 deal more damage on HP
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[3] || atk == &attacker->skills[4])) {

      int Boost = (defender->Rupture[0] + attacker->Poise[0]) / 6;
      if (Boost > 3) Boost = 3;

      if (Boost > 0) {

      atk->CoinPowerBoost[0] += Boost;

        printf("\n%s gains +1 Coin Power (%d - Max 3) for every 6 (Rupture Stack on target + Poise Stack on self) (%d)\n", attacker->name, Boost, (defender->Rupture[0] + attacker->Poise[0]));

      sleep(1);

      }
    }

    // Sancho:The Second Kindred of Don Quixote - Heal mechnics
   if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && Unbreakable <= 0) {

    int healvalue = 40;

     int missingHP = (int)(((attacker->MAX_HP - attacker->HP) / attacker->MAX_HP) * 100);
      if (missingHP > 20) missingHP = 20;

     if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
      (atk == &attacker->skills[10] || atk == &attacker->skills[11] || atk == &attacker->skills[12] || atk == &attacker->skills[13])) {
      healvalue += 100;
     }

     healvalue += missingHP;

     printf("\nOn Hit with this Skill: heal %d%% of the HP damage dealt\n", healvalue);

     sleep(1);

   }  // Sancho:The Second Kindred of Don Quixote - certain heal skill
     else if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
             0 &&
         (atk == &attacker->skills[10] || atk == &attacker->skills[11] || atk == &attacker->skills[12] || atk == &attacker->skills[13])) {

       printf("\nOn Hit with this Skill: heal 100%% of the HP damage dealt\n");

       sleep(1);
     }

  // Don Quixote:The Manager of La Manchaland and Sancho - Heal mechnics
  if ((isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
           0 &&
       (atk == &attacker->skills[0] || atk == &attacker->skills[1] ||
        atk == &attacker->skills[4]))) {

    printf("\nOn Hit with this Skill: heal 30%% of the HP damage dealt (Max "
           "10)\n");

    sleep(1);
  } else if ((isId(attacker->ID,
                         "Don Quixote:The Manager of La Manchaland") == 0 &&
              (atk == &attacker->skills[2]))) {

    printf("\nOn Hit with this Skill: heal 50%% of the HP damage dealt (Max 10)\n");

    sleep(1);
  } else if (isId(attacker->ID,
                        "Don Quixote:The Manager of La Manchaland") == 0 &&
             (atk == &attacker->skills[3] || atk == &attacker->skills[5])) {

    printf("\nOn Hit with this Skill: heal 50%% of the HP damage dealt (Max "
           "20)\n");

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Ultimate
  if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
      atk == &attacker->skills[13]) {

    if (Unbreakable == attacker->skills[13].Coins) {
      printf("\n%s Clash lost, Reduce 50%% damage\n", attacker->name);
      atk->DamageUp[0] -= 50;
    }

    sleep(1);
  }


  // Hong lu:The Lord of Hongyuan - Lordsguard
  const char *HeshinPacks = NULL;
  if (isId(defender->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      defender->skills[5].active == -1 &&
      (defender->skills[5].BasePower == 1 ||
       defender->skills[5].CoinPower == 1 || defender->skills[5].Coins == 1 ||
       defender->skills[5].Offense == 1)) {

    SkillStats *skill = &defender->skills[5];
    int *fields[] = {&skill->BasePower, &skill->CoinPower, &skill->Coins,
                     &skill->Offense};
    const char *names[] = {"Mao", "Si", "Wu", "You"};
    int fieldCount = 4;

    while (HeshinPacks == NULL) {
      int availableIndices[4];
      int availableCount = 0;

      // Keep stats that are still 1
      for (int i = 0; i < fieldCount; i++) {
        if (*fields[i] == 1) {
          availableIndices[availableCount++] = i;
        }
      }

      if (availableCount == 0) {
        break; // no more stats to assign
      }

      // randomly select one of the available stats
      int randIndex = availableIndices[rand() % availableCount];
      HeshinPacks = names[randIndex];
    }

    if (HeshinPacks != NULL) {
      printf("\n%s: \"Protect Hongyuan as ordered.\" (Heshin Packs - %s uses Lordsguard)\n",
             defender->name, HeshinPacks);

      sleep(1);

      printf("\n%s attacks 'Heshin Packs - %s' instead!\n", attacker->name,
             HeshinPacks);
    }
  }

    // Roland – Mang (心)
    if (isId(attacker->ID, "Fixer grade 9?") == 0 && isId(defender->ID, "Binah") == 0 && attacker->Sanity >= 0 && defender->Passive == 1) {

      int Mang = rand() % 5 + 1;

      printf("\nIf %s has 'Shin (心) - The Black Silence' and at 0+ Sanity, Generate 1-5 Mang (望) (%d)\n",
        attacker->name, Mang);

        attacker->skills[6].active += Mang;
      if (!ClashLostAttack) {

        atk->DamageUp[0] += Mang*(100/atk->Coins);

      } else {

        atk->DamageUp[0] += Mang*(1000/atk->Coins);

      }

      sleep(1);
    }

  // Roland - Ultimate
  if (isId(attacker->ID, "Fixer grade 9?") ==
          0 &&
      atk == &attacker->skills[9]) {

    // lose Black Silence
    if (Unbreakable > 0) {

      float lost = Unbreakable;

        atk->BasePowerBoost[0] -= lost;
       atk->DamageUp[0] -= lost*3;

      printf("\n%s loses 1 Base Power and deal -3%% damage for every Cracking Unbreakable Coins (%.0f)\n", attacker->name, lost);

       sleep(1);
    }

    printf("\n%s consumes all Black Silence and gain +5%% Damage (%d%%) for every 3 Black Silence(%d)\n", attacker->name, 5*(attacker->Passive / 3), attacker->Passive);

    attacker->Passive = 0;
    atk->DamageUp[0] += 5*(attacker->Passive / 3);

    sleep(1);

     printf("\n%s: \"Gone Angle...\"\n", attacker->name);

    sleep(1);
  }

    // ------------------- The House of Spiders: The Index Nursefather Yi Sang -------------------

    // --- [เพิ่ม] การลดดาเมจเมื่อ Yi Sang เป็นฝ่ายรับ (เหรียญแตก) ---
    if (isId(defender->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {
        if (Unbreakable > 0) { // ถ้าโดนโจมตีด้วยเหรียญที่แพ้ Clash มา (Cracking Coins)
            if (defender->skills[3].active == 1) { printf("\n%s takes -10%% damage from Cracking Unbreakable Coins\n", defender->name); } // ลดดาเมจ 10%
            if (defender->skills[3].active == 2) { printf("\n%s takes -25%% damage from Cracking Unbreakable Coins\n", defender->name); } // ลดดาเมจ 25%
        }
    }

    // The House of Spiders: The Ring Nursefather Hong Lu - Skill 3
    if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && atk == &attacker->skills[2]) {

      inflictStatus(attacker->Charge, 0, -4, 0, 99, 0, 20);

        attacker->skills[6].active += 4; // Count Consumed
        
          printf("\n%s consumes 4 Corpus Ingredient Count (%d)\n", attacker->name, attacker->Charge[1]);

        sleep(1);

      if (attacker->skills[6].active >= 10) {
       int Gain = attacker->skills[6].active / 10;
        attacker->skills[6].active -= Gain * 10; // Count Consumed
        inflictStatus(attacker->Charge, Gain, 0, 0, 99, 0, 20);
        printf("\n%s gains +%d Corpus Ingredient Stack (%d)\n", attacker->name, Gain, attacker->Charge[0]);

        sleep(1);

        if (attacker->Charge[0] >= 2 && !attacker->skills[14].active) {
          attacker->skills[14].active = 1; // Flagged
          printf("\n%s gains Artwork: Tibia\n", attacker->name);

          sleep(1);

          printf("\n%s: \"Ah...! Marvelous art...!\"\n", attacker->name);

          sleep(1);
        }
      }

    }

  //----------------------------------------------------------------

    // -------------------------- Evade Skill --------------------------------
    int Evaded = 0;
    int IsStillEvaded = 0;
    int evadePower = 0;

    int fanaticUsed = 0;

    if (!nodefense) {

    // Evade Skill
    if (defSkill != NULL && defSkill->skillType == 2 && defSkill->active == 1) {
        Evaded = 1;
      IsStillEvaded = 1;

      printf("\n%s uses 'Evade Skill' (%s)\n", defender->name, defSkill->name);

      sleep(1);

    }

    // The House of Spiders: The Thumb Nursefather Rodion - Precognition evade
    if (isId(defender->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && defender->Passive > 0 && defender->skills[11].active == 0 && Evaded == 0 && !isStaggered(defender) && !isPanicked(defender)) {
        Evaded = 1;
      IsStillEvaded = 1;

        defender->defenseSkill[1].active = 1;

      int lost = 0;

      if (clashCount > 0 && !ClashLostAttack) {
            lost = 5;
      } else if ((clashCount > 0 && ClashLostAttack) || clashCount <= 0) {
              lost = 3;
        }

      int canLose = 10 - defender->skills[6].active; // โควต้าที่เหลือในเทิร์นนี้

      int actualLoss = (lost > canLose) ? canLose : lost;
      if (actualLoss > defender->Passive) actualLoss = defender->Passive;

        defender->Passive -= actualLoss;
        defender->skills[6].active += actualLoss; // บันทึกว่าเสียไปเท่าไหร่แล้วในเทิร์นนี้

      printf("\n%s loses %d Eye of Precognition to uses 'Precognition' (%d Left)\n", defender->name, actualLoss, defender->Passive);

      sleep(1);

      if (defender->Passive <= 0 && defender->skills[11].active == 0) {
          defender->skills[11].active = 1; // ติด Overheat

        printf("\n%s converts 'Eye of Precognition' into 'Eye of Precognition - Overheat'\n", defender->name);
      }

      int gain = 0;

      gain = defender->Passive / 2;
      if (gain > 4) gain = 4;

        defender->defenseSkill[1].CoinPowerBoost[0] += gain;

      printf("\n%s gains +1 Coin Power for every 2 Eye of Precognition Stacks (%d) on self (%d - Max 4)\n", defender->name, defender->Passive, gain);

      sleep(1);

      gain = defender->Passive > 10 ? (defender->Passive - 10)/2 : 0;
      if (gain > 4) gain = 4;

        defender->defenseSkill[1].BasePowerBoost[0] += gain;
      
        printf("\n%s gains +1 Base Power for every 2 excess Eye of Precognition (%d) Stacks on self past 10 Stacks (%d - Max 4)\n", defender->name, defender->Passive, gain);

      sleep(1);

      if (defender->defenseSkill[4].active == 0) {
        defender->defenseSkill[4].active = 1;

        inflictStatus(defender->Poise, 0, 4, 0, 99, 0, 99);

        printf("\n%s gains +4 Poise Count (%d) (Once per turn)\n", defender->name, defender->Poise[1]);

        sleep(1);
      }

    }

    // --- [The One Who Grips Faust - Such Filth Setup] ---
  

    // เช็คว่า Faust เป็นฝ่ายรับ และมี Fanatic (Passive) หรือไม่
    if (isId(defender->ID, "The One Who Grips Faust") == 0 && defender->Passive > 0 && Evaded == 0) {
        Evaded = 1;
      IsStillEvaded = 1;
        fanaticUsed = defender->Passive;

      printf("\n%s consumes all Fanatic (%d) to use 'Such Filth'\n", defender->name, defender->Passive);

      defender->Passive = 0; // จ่าย Fanatic ทั้งหมดทันที

      sleep(1);

    }

    // The House of Spiders: The Ring Nursefather Hong Lu - EVade
    if (isId(defender->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && defender->skills[5].active == 1 && defSkill == &defender->defenseSkill[0] && Evaded == 0) {
        Evaded = 1;
      IsStillEvaded = 1;

      printf("\n%s consumeed Corpus Ingredient, use '%s'\n", defender->name, defender->defenseSkill[1].name);

      sleep(1);

      printf("\n%s: \"Vandalism is prohibited!\"\n", defender->name);

      sleep(1);

    }

    }




  printf("\n--- Attack Phase ---\n");

  int totalPower = atk->BasePower + atk->BasePowerBoost[0] + (attacker->BasePowerUp[0] - attacker->BasePowerDown[0]);
  int totalDamage = 0;

  printf("%s starts attack with %s: '%s'\n", attacker->name, getSkillTypeName(atk->skillType), atk->name);

  if (atk->Unbreakable > 0 && atk->Coins != atk->Unbreakable) {
    // Condition 1: Some breakable and some unbreakable coins remaining
    if (Unbreakable > 0) {
        // Output with Cracking Coins details
        printf("Tossing %d Coins for attack (from remaining clash %d Breakable "
               "Coins and %d Unbreakable Coins (%d Cracking Coins, Halve those Coins's Damage)):\n",
               remainingCoins, remainingCoins - atk->Unbreakable, atk->Unbreakable, Unbreakable);
    } else {
        // Output without Cracking Coins details
        printf("Tossing %d Coins for attack (from remaining clash %d Breakable "
               "Coins and %d Unbreakable Coins):\n",
               remainingCoins, remainingCoins - atk->Unbreakable, atk->Unbreakable);
    }
  } else if (atk->Unbreakable > 0 && atk->Coins == atk->Unbreakable) {
    // Condition 2: Only unbreakable coins remaining
    if (Unbreakable > 0) {
        // Output with Cracking Coins details
        printf("Tossing %d Coins for attack (from remaining clash %d Unbreakable "
               "Coins (%d Cracking Coins, Halve those Coins's Damage)):\n",
               remainingCoins, atk->Unbreakable, Unbreakable);
    } else {
        // Output without Cracking Coins details
        printf("Tossing %d Coins for attack (from remaining clash %d Unbreakable "
               "Coins):\n",
               remainingCoins, atk->Unbreakable);
    }
  } else {
    // Condition 3: Only breakable coins remaining (atk->Unbreakable is 0 or less)
    printf("Tossing %d Coins for attack (from remaining clash %d Breakable "
           "Coins):\n",
           remainingCoins, remainingCoins);
  }

  printf("%-10s %-10s %-10s", "Coin", "Power", "Damage");

  int AmmoLeft = attacker->Passive; // Calculate remaining ammo for Meursault: The Thumb's buff

    int bonus = 0;

  for (int i = 0; i < remainingCoins; i++) {

    if (attacker->HP > 0) {

    // --------------------------- Coin Buff Section ------------------------------------------

    int CoinBuff = 0;

       // ---------------------------------------- The House of Spiders: The Thumb Nursefather Rodion --------------------------------

          // The House of Spiders: The Thumb Nursefather Rodion – Buff at 1+ ammo
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && attacker->skills[12].active >= 5 
      && (atk == &attacker->skills[4] || atk == &attacker->skills[5])) {

        if (attacker->skills[10].active > 0) { // If there's still ammo left for this coin

        CoinBuff += 1;

        }

      }

      // ------------------------------------------------------------

       // ---------------------------------------- Meursault:The Thumb Coin Buff --------------------------------

          // Meursault: The Thumb – Buff spend tigermark
      if (isId(attacker->ID, "Meursault:The Thumb") == 0 && !attacker->skills[3].active && attacker->Passive > 0 
      && ((atk == &attacker->skills[0] && i == remainingCoins - 1) 
      || (atk == &attacker->skills[1] && (i == remainingCoins - 2 || i == remainingCoins - 1))
      || (atk == &attacker->skills[2] && (i == remainingCoins - 3 || i == remainingCoins - 2 || i == remainingCoins - 1)))) {

        if (AmmoLeft > 0) { // If there's still ammo left for this coin

          AmmoLeft--; // Consume 1 ammo for this coin
        CoinBuff += 1;

        }

      }

      // Meursault: The Thumb – Buff spend Savage tigermark
      if (isId(attacker->ID, "Meursault:The Thumb") == 0 && attacker->skills[3].active && attacker->Passive > 0
      && ((atk == &attacker->skills[0] && i == remainingCoins - 1) 
      || (atk == &attacker->skills[1] && (i == remainingCoins - 2 || i == remainingCoins - 1))
      || (atk == &attacker->skills[2] && (i == remainingCoins - 3 || i == remainingCoins - 2 || i == remainingCoins - 1))
      || (atk == &attacker->skills[3] && (i == remainingCoins - 3 || i == remainingCoins - 2 || i == remainingCoins - 1)))) {

        if (AmmoLeft > 0) { // If there's still ammo left for this coin

          AmmoLeft--; // Consume 1 ammo for this coin
        CoinBuff += 2;

        }

      }

    // ------------------------------------------------------------

    // ----------------------------------------------------------------------------------------

    int IsHeadHit = tossCoinWithSanity(attacker);

    if (IsHeadHit) {
      // Check paralyze
      if (attacker->Paralyze[0] > 0) { // ← Character's paralyze
        totalPower += 0;
        if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && atk == &attacker->skills[3]
          || isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && atk == &attacker->defenseSkill[1] && attacker->defenseSkill[2].active == 1) {
          totalPower += atk->CoinPower; // เพิ่มพลังตามปกติแม้ติดอัมพาต
        } else {
          totalPower += 0; // สกิลอื่นโดน Paralyze ปกติ
          attacker->Paralyze[0]--;
        }
      } else {
        {
          int charCoinBoost = 0;
          if (atk->CoinPower >= 0) {
            charCoinBoost += attacker->PlusCoinPowerBoost[0] - attacker->PlusCoinPowerDrop[0];
          } else {
            charCoinBoost += attacker->MinusCoinPowerDrop[0] - attacker->MinusCoinPowerBoost[0];
          }
          totalPower += CoinBuff + atk->CoinPower + atk->CoinPowerBoost[0] + charCoinBoost;
        }
        if (totalPower <= 0) totalPower = 0;
      }

      // ------------------ On Head hit ------------------------

      // Sukuna - Black Flash
      if (isId(attacker->ID, "Sukuna:King of Curse") == 0 &&
          atk == &attacker->skills[5]) {

        int heal = attacker->MAX_HP*0.1;

        attacker->HP += (attacker->HP + heal > attacker->MAX_HP)
                            ? attacker->MAX_HP - attacker->HP
                            : heal;

        updateSanity(attacker, 10);
        if (attacker->Sanity > 45) attacker->Sanity = 45;

        printf("\n%s Coins On Head Hit, Trigger 'Black Flash', Deal 2.5x more damage , HP +%d (%.2f), Sanity +10 (%d)", attacker->name, heal, attacker->HP, attacker->Sanity);

        sleep(1);
      }

      // Heishou Pack - You Branch Adept Heathcliff Skill 3 burn
      if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i != remainingCoins - 1 && attacker->HP >= attacker->MAX_HP*0.5) {

        applyDamage(NULL, attacker, attacker->Burn[0], 0, NULL);

        if (attacker->HP < 1)
            attacker->HP = 1;

           printf("\n%s Coins On Head Hit, at 50%%+ HP, take Burn damage by Burn Stack on self (%d)", attacker->name,attacker->Burn[0]);

      }

      // Heathcliff:Wild Hunt - Tanut Skill 3-1
      if (isId(attacker->ID, "Heathcliff:Wild Hunt") ==
                     0 &&
                 (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

        printf("\n\n%s: \"Remain buried in your regrets!\"\n", attacker->name);


        sleep(1);
      }

      // Heathcliff:Wild Hunt - Tanut Skill 3-2
      if (isId(attacker->ID, "Heathcliff:Wild Hunt") ==
                     0 &&
                 (atk == &attacker->skills[3]) && i == remainingCoins - 1) {

        printf("\n\n%s: \"Disappear with the storm...\"\n", attacker->name);


        sleep(1);
      }

      // ----------------------------------------------------------------

    } else {

       // ------------------ On Tail Hit ------------------------

      // Heathcliff:Wild Hunt - Tanut Skill 3-1
      if (isId(attacker->ID, "Heathcliff:Wild Hunt") ==
                     0 &&
                 (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

        printf("\n\n%s: \"Remain buried in your regrets...\"\n", attacker->name);


        sleep(1);
      }

      // Heathcliff:Wild Hunt - Tanut Skill 3-2
      if (isId(attacker->ID, "Heathcliff:Wild Hunt") ==
                     0 &&
                 (atk == &attacker->skills[3]) && i == remainingCoins - 1) {

        printf("\n\n%s: \"Disappear with the storm!\"\n", attacker->name);


        sleep(1);
      }

      // ----------------------------------------------------------------

    }






















    int currentPower = totalPower;

    bonus = 0;

    // Last coin power bonus (from character buffs only)
    if (i == remainingCoins - 1) {
      if (atk->skillType == 0) {
        bonus = (atk->FinalPowerBoost[0] + atk->AttackPowerBoost[0] + atk->AttackSkillPowerBoost[0]) + ((attacker->FinalPowerUp[0] - attacker->FinalPowerDown[0]) + (attacker->AttackPowerUp[0] - attacker->AttackPowerDown[0]) + (attacker->AttackSkillPowerUp[0] - attacker->AttackSkillPowerDown[0]));
      } else {
        bonus = (atk->FinalPowerBoost[0] + atk->DefensePowerBoost[0] + atk->DefenseSkillPowerBoost[0]) + ((attacker->FinalPowerUp[0] - attacker->FinalPowerDown[0]) + (attacker->DefensePowerUp[0] - attacker->DefensePowerDown[0]) + (attacker->DefenseSkillPowerUp[0] - attacker->DefenseSkillPowerDown[0]));
      }
        currentPower += bonus;
      if (currentPower <= 0) currentPower = 0;
    }

    // Calculate offense difference modifier: (Off - Def) / (|Off - Def| + 25) × 100
    int offenseDiff = atkTempOffense - defTempDefense;
    int absOffenseDiff = abs(offenseDiff);

    float damageModifier = 0;
        damageModifier = (int)(((float)offenseDiff / (float)(absOffenseDiff + 25)) * 100.0f);

    // Apply offense modifier to current power (as percentage)
    float modifiedPower = currentPower + ((currentPower * damageModifier) / 100.0f);

    if (defSkill != NULL && defSkill->skillType == 4 && powerReduction > 0) {
              modifiedPower -= powerReduction;
    }

if (modifiedPower < 0.0f) modifiedPower = 0.0f;

    // Adjust multiplier if defender is Unbreakable
    float adjustedMultiplier = atk->DmgMutiplier / ((Unbreakable > 0) ? 2.0f : 1.0f);

    // Add clash count bonus: +1% damage per clash (up to the current clash count)
    float clashMultiplier = 1.0f;  // 1% per clash
    if (Unbreakable <= 0) {
      clashMultiplier = 1.0f + (clashCount * 0.01f);  // 1% per clash
    }

    int Damage = (int)(modifiedPower * adjustedMultiplier * clashMultiplier);



    // ---------------- Keyword Status ----------------

    int IsCritical = 0;

    int PoiseChance = attacker->Poise[0] * 5;
    if (PoiseChance > 100) PoiseChance = 100;

    // Poise // 0 Stack 1 Count
    if ((attacker->Poise[0] > 0 || attacker->Poise[1] > 0) && !Evaded) {

      if (attacker->Poise[0] <= 0 && attacker->Poise[1] > 0) attacker->Poise[1]++;

      // สุ่มเลข 0-99 มาเทียบกับ Chance
      if ((rand() % 100) < PoiseChance) {

        IsCritical = 1;

              attacker->Poise[1] -= 1;

      }

        }






















    // Sukuna: King of Curse - Black Flash
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 &&
        (atk == &attacker->skills[5]) && IsHeadHit) {

        Damage *= 2.5;

    }

    // Yi sang:Fell Bullet - Torn Memory
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 &&
        (atk == &attacker->skills[0] ||
         atk == &attacker->skills[1])) {

        Damage += (int)(Damage * (0.15 * attacker->Passive));
    }

    // The House of Spiders: The Index Nursefather Yi Sang Skill 4 Last coins
    if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && (atk == &attacker->skills[3]) && i == remainingCoins - 1 && attacker->skills[3].active == 2) {

      printf("\n\n%s: \"Furiously, throughout it all.\"\n", attacker->name);

      sleep(1);

    }

    // ------------- Sukuna:King of Curse -------------

    // Sukuna:King of Curse - Skill 3
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: \"Come on... Keep trying.\"\n", attacker->name);

      sleep(1);
    }

    // Sukuna:King of Curse - Skill 7
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[6])) {

      if (i == 0) {

      printf("\n\n%s: \"Know your place...\"\n", attacker->name);

      sleep(1);

      }

      if (i == remainingCoins - 1) {

      printf("\n\n%s: \"Fool.\"\n", attacker->name);

      sleep(1);

      }

    }

    // Sukuna:King of Curse - Skill 4
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[3]) && i == 0) {

      printf("\n\n%s: \x1b[1;31m■ (Open)\x1b[0m\n", attacker->name);
      sleep(1);
      printf("\n%s: \"Fuga (鐚)\"\n", attacker->name);

      sleep(1);

      printf("\n%s: \"Arm yourself.\"\n", attacker->name);

      sleep(1);

      printf("\n%s: \"Let's have a contest of firepower.\"\n", attacker->name);

      sleep(1);

    }

    // Sukuna:King of Curse - Skill 8
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[7]) && i == 0) {

      printf("\n\n%s: \"It's over\"\n", attacker->name);

      sleep(1);

    }

    // ---------------------------------------

    // Dawn Office Fixer Sinclair - Tanut
    if (isId(attacker->ID, "Dawn Office Fixer Sinclair") ==
                   0 &&
               (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && !attacker->skills[3].active && i == 0) {

      printf("\n\n%s: \"I'll carve this stigma... into you!\"\n", attacker->name);

      sleep(1);

    }

    // Dawn Office Fixer Sinclair - Tanut
    if (isId(attacker->ID, "Dawn Office Fixer Sinclair") ==
                   0 &&
               (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && attacker->skills[3].active && i == 0) {

      printf("\n\n%s: \"Burn in...\"\n", attacker->name);

      sleep(1);

      printf("\n%s: \"This passion.\"\n", attacker->name);

       sleep(1);

    }

    // ---------------- Jia Qiu ----------------

    // Jia Qiu - Taunt S4
    if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[4] || atk == &attacker->skills[11]) && i == 0) {

    printf("\n\n%s: \"Cut them Down, Mao.\"\n", attacker->name);

    sleep(1);
    }

    // Jia Qiu - Taunt S12
    if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[12]) && i == 0) {

    printf("\n\n%s: \"Heed me, Zilu.\"\n", attacker->name);

    sleep(1);
    }

    // Jia Qiu - Taunt S15
    if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[15]) && i == 0) {

    printf("\n\n%s: \"Is it your companions who hold your tongue? Then... perhaps they must be shaken afore you are to speak your truth.\"\n", attacker->name);

    sleep(1);
    }

    // ------------------------------------------------

      // -------------------------- The House of Spiders: The Ring Nursefather Hong Lu ----------------------------------

      // The House of Spiders: The Ring Nursefather Hong Lu Skill Tanut Skill 2-2
      if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[2])) {

        if (i == remainingCoins - 2) {
        printf("\n\n%s \"From there...\"\n", attacker->name);

        sleep(1);
        }

        if (i == remainingCoins - 1) {
          printf("\n\n%s \"Please observe...!\"\n", attacker->name);

          sleep(1);
          }

      }

      // The House of Spiders: The Ring Nursefather Hong Lu Skill Tanut Skill 3-1
      if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[3])) {

        if (i == 0) {
        printf("\n\n%s \"Do you hear it?\"\n", attacker->name);

        sleep(1);
        }

        if (i == 1) {
          printf("\n\n%s \"The euphonious harmony of the humerus pair and 24 ribs...!\"\n", attacker->name);

          sleep(1);
          }

        if (i == 3) {
          printf("\n\n%s \"Tibia's Melody!!\"\n", attacker->name);

          sleep(1);
          }

      }

      // The House of Spiders: The Ring Nursefather Hong Lu Skill 2-2 Boost
      if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

        int count = countNegativeEffectTypes(defender);
        int boost = count * 15;
        if (boost > 50) boost = 50;

        atk->DamageUp[0] += boost;

        printf("\n%s deals +15%% damage for every type of negative effect (%d) on target (%d%% - Max 50%%)", attacker->name, count, boost);
      }

      // The House of Spiders: The Ring Nursefather Hong Lu Skill 3 Boost
      if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[3] || atk == &attacker->skills[4]) && i == remainingCoins - 1) {

        int count = countNegativeEffectTypes(defender);
        int boost = count * 10;
        if (boost > 30) boost = 30;

        atk->DamageUp[0] += boost;

        printf("\n%s deals +10%% damage for every type of negative effect (%d) on target (%d%% - Max 30%%)", attacker->name, count, boost);
      }

        // The House of Spiders: The Ring Nursefather Hong Lu Artwork: Tibia Damage Buff
        if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && attacker->skills[14].active && attacker->Charge[0] >= 2) {

        if ((defender->Bleed[0] > 0 || defender->Bleed[1] > 0)) {
          Damage *= 1.10;
        }

        }

      // --------------------------------------------------------------------------------------------------------

    // --------------------------------- Don Quixote:The Manager of La Manchaland ---------------------------------------

    // ------------------------ Don Quixote:The Manager of La Manchaland --------------

    // Don Quixote:The Manager of La Manchaland - Tanut Skill 2
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[1]) && i == remainingCoins - 1) {

      printf("\n\n%s: \"Until you fall\"\n", attacker->name);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Tanut Skill 3
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: \"Impale through!\"\n", attacker->name);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Skill 2 in both Buff dmg
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[1] || atk == &attacker->skills[4]) && i == remainingCoins - 1) {

      float P_HPDifferent = (attacker->MAX_HP - attacker->HP) / attacker->MAX_HP; // 0.0 - 1.0
       float E_HPDifferent = (defender->MAX_HP - defender->HP) / defender->MAX_HP; // 0.0 - 1.0

      int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

      float boost = (abs(HPDifferent) * 2.5);
      if (boost > 25) boost = 25;

       if (boost > 0) {

      atk->DamageUp[0] += boost;

      printf("\n%s's last coin deals +2.5%% damage for every 1%% HP different (%.1f%% - Max 25%%)", attacker->name, boost);

      sleep(1);
       }
    }

    // Don Quixote:The Manager of La Manchaland - Tanut Skill 3-2
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[5]) && i == remainingCoins - 1) {

      printf("\n\n%s: \"Time to end this.\"\n", attacker->name);

      sleep(1);
    }

    // --------------------------------------------------------------------------

    // Don Quixote:The Manager of La Manchaland - Skill 2-2 dmg
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[1]) && i == remainingCoins - 1 && attacker->Passive >= 2) {

      int boost = (attacker->Passive/3) * 15;
      if (boost > 75) boost = 75;

      atk->DamageUp[0] += boost;

      printf("\n%s's last coin deals +15%% damage(%d%% - Max 75%%) for every 3 HardBlood (%d)", attacker->name, boost, attacker->Passive);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Skill 2-2 dmg
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[4]) && i == remainingCoins - 1 && attacker->Passive >= 2) {

      int boost = (attacker->Passive/3) * 20;
      if (boost > 150) boost = 150;

      atk->DamageUp[0] += boost;

      printf("\n%s's last coin deals +20%% damage(%d%% - Max 150%%) for every 3 HardBlood (%d)", attacker->name, boost, attacker->Passive);

      sleep(1);
    }


      // FireFist – Attack buff s3
      if (isId(attacker->ID, "Gregor:Firefist") == 0 && (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

        printf("\n\n%s: \"RAAHHHHHHH!!! You vermin-like bastards!\"\n", attacker->name);

        sleep(1);

          int prevPassive = attacker->Passive;  // store before change
          int consumed = ((attacker->Passive - 25) > 0) ? 25 : attacker->Passive ;
          attacker->Passive -= consumed;

        int boost = (int)((0.02 * consumed) * 100);  // 2% per missing fuel

        if (prevPassive > 50 && attacker->Passive > 50) {

          if (boost > 50) boost = 50;                    // cap at 50%
          atk->DamageUp[0] += boost;

           printf("\n%s's last coin deal +2%% more damage for every District 12 Fuel consumed by this Skill (%d%% - Max 50%%)", attacker->name, boost);

          } else if (prevPassive > 50 && attacker->Passive <= 50) {

          if (boost > 50) boost = 50;                    // cap at 50%
          atk->DamageUp[0] += boost;

           printf("\n%s's last coin deal +2%% more damage for every District 12 Fuel consumed by this Skill (%d%% - Max 50%%)", attacker->name, boost);

          } else {

          boost = (int)((0.04 * consumed) * 100);  // 4% per missing fuel
          if (boost > 100) boost = 100;                    // cap at 100%
          atk->DamageUp[0] += boost;

           printf("\n%s's last coin deal +4%% more damage for every Overheated Fuel consumed by this Skill (%d%% - Max 100%%)", attacker->name, boost);
          }

        sleep(1);
      }

    // Jia Qiu – buff s5 and s13 S15
      if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[5] || atk == &attacker->skills[13] || atk == &attacker->skills[15]) && i == remainingCoins - 1) {

          int boost = abs((int)(5 * attacker->Sanity));  // 4% per missing fuel
          if (boost > 200) boost = 200;                    // cap at 100%
          atk->DamageUp[0] += boost;

           printf("\n%s's last coin deal +5%% damage for the further this unit's Sanity from 0 (%d%% - Max 200%%)", attacker->name, boost);

        sleep(1);
      }

    // Jia Qiu - S11
    if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[11] && i == remainingCoins - 1)) {

      int boost = abs(defender->Sanity);
      if (boost > 10) boost = 10;

          Damage += boost;

      printf("\n%s deal additional fixed damage equal to the further enemy's Sanity from 0 (%d - Max 10)", attacker->name, boost);

      sleep(1);
    }

    // Sukuna:King of Curse - Tanut Skill 5
    if (isId(attacker->ID, "Sukuna:King of Curse") ==
                   0 &&
               (atk == &attacker->skills[4]) && i == remainingCoins - 1) {

      if (attacker->skills[4].active == 1) {
      printf("\n\n%s: \"Scale of dragon\"\n", attacker->name);
      } else if (attacker->skills[4].active == 2) {
          printf("\n\n%s: \"Recoil\"\n", attacker->name);
          } else if (attacker->skills[4].active == 3) {
        printf("\n\n%s: \"Twin meteor\"\n", attacker->name);
        }

      sleep(1);
    }

      // --------------- The Middle Nursefather - Matthias ---------------

      // The Middle Nursefather - Matthias - Counter
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->defenseSkill[0]) && i == 1) {

      printf("\n\n%s: \"C'mere!\"\n", attacker->name);

        usleep(5000);
      }

      // The Middle Nursefather - Matthias - Skill 3
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[2])) {

        if (i == 0) {

        printf("\n\n%s: \"These toys...\"\n", attacker->name);

          usleep(5000);

        }

        if (i == 2) {

        printf("\n\n%s: \"...ain't fun...\"\n", attacker->name);

        usleep(5000);

        }

        if (i == 3) {

        printf("\n\n%s: \"...no more!\"\n", attacker->name);

        usleep(1000);

        }
      }

      // The Middle Nursefather - Matthias - Phase 2
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[4]) && i == 0) {

      printf("\n\n%s: \"Oh yeah, I'm down to unbox a layer of packaging for this fight!\"\n", attacker->name);

      sleep(1);
      }

      // The Middle Nursefather - Matthias - Charge Talk
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[5] || atk == &attacker->skills[6])) {

        if (i == remainingCoins - 1) {

      printf("\n\n%s: \"Ha...\"\n", attacker->name);

      sleep(2);

      }

      }

      // The Middle Nursefather - Matthias - Phase 3
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[5]) && i == 0) {

      printf("\n\n%s: \"Interesting... So this is what it feels like to meet someone strong in battle!\"\n", attacker->name);

      sleep(1);
      }

      // The Middle Nursefather - Matthias - Phase 4
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[6]) && i == 0) {

      printf("\n\n%s: \"You're pushin' me to go balls out... Hah!\"\n", attacker->name);

      sleep(1);
      }

      // The Middle Nursefather - Matthias - Using Gut Stab [Lævateinn]
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[8])) {

        if (i == remainingCoins - 4) {

        printf("\n\n%s: \"Be warned, it's gonna be hot.\"\n", attacker->name);

        sleep(1);

        }

        if (i == remainingCoins - 1) {

        printf("\n\n%s: \"Hot as hell!\"\n", attacker->name);

        sleep(1);

        }

      }

      // The Middle Nursefather - Matthias - Using '... Complete and Total Extermination [Lævateinn]'
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[9])) {

        if (i == remainingCoins - 2) {
          printf("\n\n%s: \"Hah!\"\n", attacker->name);

          sleep(1);
            }

        if (i == remainingCoins - 1) {
      printf("\n\n%s: \"Bullseye!\"\n", attacker->name);

      sleep(2);
        }

      }

      // ------------------------------------------------------------

    // ----------------- Sancho:The Second Kindred of Don Quixote -----------------

    // Sancho:The Second Kindred of Don Quixote - Ult skill 12
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        atk == &attacker->skills[12] && i == 0) {

      printf("\n\n%s: \"I'll pierce you!\"\n", attacker->name);

      sleep(1);

    }

      // Sancho:The Second Kindred of Don Quixote - Ultimate
      if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
              0 &&
          atk == &attacker->skills[13] && i == 0) {

        if (isId(defender->ID, "Don Quixote:The Manager of La Manchaland") == 0) { 
          printf("\n\n%s: \"No matter what... No matter how many times... I'll still go for our dream!!!\"\n", attacker->name); 
        } else { 
          printf("\n\n%s: \"You dream too! Will end...\"\n", attacker->name); 
        }

        sleep(1);
      }

    // Sancho:The Second Kindred of Don Quixote - Skill 1
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        atk == &attacker->skills[3] && i == 0) {

      printf("\n\n%s: \"There's blood...\"\n", attacker->name);

      sleep(1);

    }

    // Sancho:The Second Kindred of Don Quixote - Skill 4
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        atk == &attacker->skills[6]) {

      if (i == 0) {

      printf("\n\n%s: \"Annoying...\"\n", attacker->name);

      sleep(1);

      }

      if (i == 1) {

      printf("\n\n%s: \"Just die already...!\"\n", attacker->name);

      sleep(1);

      }

      if (i == 2) {

      printf("\n\n%s: \"Red Arwe.\"\n", attacker->name);

      sleep(1);

      }

    }

    // Sancho:The Second Kindred of Don Quixote - Skill 5
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        atk == &attacker->skills[7]) {

      if (i == 0) {

      printf("\n\n%s: \"Crescent Moon.\"\n", attacker->name);

      sleep(1);

      }

      if (i == 1) {

      printf("\n\n%s: \"Half Moon.\"\n", attacker->name);

      sleep(1);

      }

    }

    // Sancho:The Second Kindred of Don Quixote - Skill 6
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        atk == &attacker->skills[8]) {

      if (i == 0) {

      printf("\n\n%s: \"Full Moon.\"\n", attacker->name);

      sleep(1);

      }

      if (i == 1) {

      printf("\n\n%s: \"Spring Dragon, Autumn Lotus.\"\n", attacker->name);

      sleep(1);

      }

    }

    // Sancho:The Second Kindred of Don Quixote - Skill 7
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        atk == &attacker->skills[9]) {

      if (i == 0) {

      printf("\n\n%s: \"It's all in vain...\"\n", attacker->name);

      sleep(1);

      }

    }

    // Sancho:The Second Kindred of Don Quixote - Skill 8
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        atk == &attacker->skills[10]) {

      if (i == 0) {

      printf("\n\n%s: \"Unfurling the Thousand Pound Bow.\"\n", attacker->name);

      sleep(1);

      }

      if (i == 1) {

      printf("\n\n%s: \"Blak Arwe.\"\n", attacker->name);

      sleep(1);

      }

    }

    // Sancho:The Second Kindred of Don Quixote - Skill 9
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        atk == &attacker->skills[11]) {

      if (i == 0) {

      printf("\n\n%s: \"Rise up...\"\n", attacker->name);

      sleep(1);

      }

      if (i == 1) {

      printf("\n\n%s: \"With an intangible sword!\"\n", attacker->name);

      sleep(1);

      }

    }

    // --------------------------------------------------------------------

    // ------------------- Heathcliff:Wild Hunt ---------------------

    // Heathcliff:Wild Hunt - Tanut Skill 2
    if (isId(attacker->ID, "Heathcliff:Wild Hunt") ==
                   0 &&
               (atk == &attacker->skills[1]) && i == 2) {

      if (attacker->skills[0].active <= 0) {
      printf("\n\n%s: \"I'll rip you apart.\"\n", attacker->name);
      } else {
        printf("\n\n%s: \"Tear them all to shreds.\"\n", attacker->name);

        sleep(1);

        printf("\n%s: \"Dullahan!\"\n", attacker->name);
      }

      sleep(1);
    }

    // ------------------------------------------------------------------

    // Hong lu:The Lord of Hongyuan - Tanut Skill 2
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") ==
                   0 &&
               (atk == &attacker->skills[1]) && i == 0) {

      printf("\n\n%s: \"I'll personally sever your neck.\"\n", attacker->name);

      sleep(1);
    }

    // ---------------------------------------------------------------------

    // Lobotomy E.G.O::Solemn Lament Yi Sang - Tanut Skill 3
    if (isId(attacker->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
                   0 &&
               (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: \"Rest in peace.\"\n", attacker->name);

      sleep(1);
    }

    // --------------------------------- Yi sang:Fell Bullet ---------------------------------------

    // Yi sang:Fell Bullet - Tanut Skill 3
    if (isId(attacker->ID, "Yi sang:Fell Bullet") ==
                   0 &&
               (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: \"I shall fire and pierce you as you are.\"\n", attacker->name);

      sleep(1);
    }


    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 &&
        atk == &attacker->skills[2]) {

      int totalBonusPercent = 0; // ตัวแปรเก็บรวม % โบนัส กันทบต้นทุน

      if (i == remainingCoins - 2) {

      int gain = (7 * attacker->Passive);
        if (gain > 49) gain = 49;

        if (gain > 0) {

        totalBonusPercent += gain;

      printf("\n%s deals +7%% damage (%d%% - Max 49%%) for every Torn Memory on self (%d) in this Coin",
             attacker->name, gain, attacker->Passive);
        }

      sleep(1);

      if (IsCritical) {

      gain = (3 * attacker->Poise[0]);
        if (gain > 60) gain = 60;

        if (gain > 0) {

            totalBonusPercent += gain;

      printf("\n%s deals +3%% damage on Critical Hit (%d%% - Max 60%%) for every Poise Stack on self (%d) in this Coin",
             attacker->name, gain, attacker->Poise[0]);

      sleep(1);

        }

      }

      }

      if (i == remainingCoins - 1) {

      int gain = 20 * attacker->Passive;
        if (gain > 140) gain = 140;

        if (gain > 0) {

            totalBonusPercent += gain;

      printf("\n%s deals +20%% damage (%d%% - 140%%) for every Torn Memory on self (%d) in this Coin",
             attacker->name, gain, attacker->Passive);

      sleep(1);

        }

        gain = attacker->skills[5].active * 3;
        if (gain > 100) gain = 100;

        if (gain > 0) {

            totalBonusPercent += gain;

        }

        printf("\n%s deals +(HP percentage the first Coin removed from the main target x 3)%% damage (%d%% - Max 100%%)",
               attacker->name, gain);

        sleep(1);

      if (IsCritical) {

      gain = 5 * attacker->Poise[0];
        if (gain > 50) gain = 50;

        if (gain > 0) {

            totalBonusPercent += gain;

      printf("\n%s deals +5%% damage on Critical Hit (%d%% - Max 50%%) for every Poise Stack on self (%d) in this Coin",
             attacker->name, gain, attacker->Poise[0]);

      sleep(1);

        }

        gain = 10 * attacker->Passive;
        if (gain > 70) gain = 70;

        if (gain > 0) {

            totalBonusPercent += gain;

        printf("\n%s deals +10%% damage on Critical Hit (%d%% - Max 70%%) for every Torn Memory on self (%d) in this Coin",
               attacker->name, gain, attacker->Passive);

        sleep(1);

        }

      }

      }

      // --- สรุปคำนวณดาเมจครั้งเดียว ---
      if (totalBonusPercent > 0) {
          Damage += (Damage * totalBonusPercent / 100); 
      }

    }

    // ------------------------------------------------------------------------------

      // ------------- The Middle Nursefather - Matthias -------------------

      // The Middle Nursefather - Matthias Count Hit
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && !Evaded) {
           attacker->skills[6].active++; // นับจำนวนเหรียญที่ Matthias ตีโดน
      }

    // ------------- Don Quixote:The Manager of La Manchaland -------------------

      // Don Quixote:The Manager of La Manchaland - Tanut
      if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                     0 &&
                 (atk == &attacker->skills[3])) {

        if (i == 0) {

        printf("\n\n%s: \"Variant Sancho Hardblood Arts 6th...\"\n", attacker->name);

        sleep(1);

        }

        if (i == remainingCoins - 1 || (i == 0 && remainingCoins == 1)) {

        printf("\n\n%s: \"Tear Apart!\"\n", attacker->name);

        sleep(1);

        }

      }

      // Don Quixote:The Manager of La Manchaland - Tanut
      if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                     0 &&
                 (atk == &attacker->skills[4])) {

        if (i == 0) {

        printf("\n\n%s: \"Variant Sancho Hardblood Arts 8th... \"\n", attacker->name);

        sleep(1);

        }

        if (i == remainingCoins - 1 || (i == 0 && remainingCoins == 1)) {

        printf("\n\n%s: \"Split apart!\"\n", attacker->name);

        sleep(1);

        }

      }

      // Don Quixote:The Manager of La Manchaland - Tanut
      if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                     0 &&
                 (atk == &attacker->defenseSkill[1])) {

        if (i == 0) {

        printf("\n\n%s: \"Hardblood Arts 15th...\"\n", attacker->name);

        sleep(1);

        }

        if (i == remainingCoins - 1 || (i == 0 && remainingCoins == 1)) {

        printf("\n\n%s: \"Building up to the finale!\"\n", attacker->name);

        sleep(1);

        }

      }

      // Don Quixote:The Manager of La Manchaland - Tanut
      if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                     0 &&
                 (atk == &attacker->skills[5] && i == 0)) {

        printf("\n\n%s: \"Ascendant Sancho Hardblood Arts...\"\n", attacker->name);

        sleep(1);

        printf("\n%s: \"La Sangre.\"\n", attacker->name);

        sleep(1);

      }

      // --------------------------------------------------------------

      // Heishou Pack - You Branch Adept Heathcliff - Taunt Skill 3
      if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: \"Bloodtalons... conflagrate!\"\n", attacker->name);

      sleep(1);
      }

      // Heishou Pack - You Branch Adept Heathcliff - Taunt Skill 4
      else if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[3])) {

        if (i == 0) {

      printf("\n\n%s: \"Bloodtalons... conflagrate!\"\n", attacker->name);

      sleep(1);

        }

          if (remainingCoins > attacker->skills[3].Coins) {

      printf("\n\n%s: \"Hahahahahahaha!! Rooster’s Rampaging Blades Under the Ensanguined Heaven [血天下雞舞亂刀]!!\"\n", attacker->name);

      sleep(1);
          }

      }

     //----------------------- Lei heng ---------------------
      
    // Lei heng – skill 4
    if (isId(attacker->ID, "Lei heng") == 0 &&
        (atk == &attacker->skills[3])) {

      if (i == 0 && (attacker->defenseSkill[1].active > 0 || attacker->defenseSkill[2].active > 0)) {
      printf("\n\n%s: \"Eyes up here, boys! Don'tcha go losin' yer heads now!\"\n",
             attacker->name);

      sleep(1);
      } else if (i == 0 && (attacker->defenseSkill[1].active <= 0 || attacker->defenseSkill[2].active <= 0)) {
        printf("\n\n%s: \"Hwell, what's this? Manageable so far?\"\n",
               attacker->name);

        sleep(1);
      }

      if (i == remainingCoins - 1) {
        
        printf("\n\n%s: \"Hah!\"\n",
               attacker->name);
        
        }
      
    }

      // Lei heng – HAAA
      if (isId(attacker->ID, "Lei heng") == 0 &&
          (atk == &attacker->skills[4] || atk == &attacker->skills[2])) {
        
        if (i == remainingCoins - 1) {

          printf("\n\n%s: \"ha...\"\n",
             attacker->name);

          sleep(2);

          printf("\n%s: \"HAAAA!\"\n",
                 attacker->name);

          }

      }

      

    // Lei heng – skill 6
    if (isId(attacker->ID, "Lei heng") == 0 &&
        (atk == &attacker->skills[4]) && i == 0) {

      printf("\n\n%s: \"Y'all don't go on huntin' tigers without preparin' yerselves to get chomped 'tween one of them jaws!\"\n",
             attacker->name);

      sleep(1);
    }

    //-----------------------------------------





      // ---------------------------------------- The House of Spiders: The Thumb Nursefather Rodion --------------------------------

      // The House of Spiders: The Thumb Nursefather Rodion - Tanut Skill 1-2
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") ==
                     0 &&
                 (atk == &attacker->skills[1]) && i == 0) {

        printf("\n\n%s: \"Hah! My Turn, you little shit.\"\n", attacker->name);

        sleep(1);
      }

      // The House of Spiders: The Thumb Nursefather Rodion - Tanut Skill 2-1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") ==
                     0 &&
                 (atk == &attacker->skills[2]) && i == 2) {

        printf("\n\n%s: \"Distraction'll bury you six-feet under!\"\n", attacker->name);

        sleep(1);
      }

      // The House of Spiders: The Thumb Nursefather Rodion - Tanut Skill 3-1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") ==
                     0 &&
                 (atk == &attacker->skills[4])) {

        if (i == 0) {
        printf("\n\n%s: \"Hahahahaha!\"\n", attacker->name);

        sleep(1);
        }

        if (i == 3) {
          printf("\n\n%s: \"And the hunt comes to a close.\"\n", attacker->name);

          sleep(1);
          }
        
      }

      // The House of Spiders: The Thumb Nursefather Rodion - Tanut Skill 3-2
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") ==
                     0 &&
                 (atk == &attacker->skills[5])) {

        if (i == 0) {
        randomVar = rand() % 2 + 1;
      }

        if (randomVar == 1) {
        if (i == 0) {
        printf("\n\n%s: \"I'm sick and tired\"\n", attacker->name);

        sleep(2);
        }
        
        if (i == 1) {
            printf("\n\n%s: \"of Yoshihide,\"\n", attacker->name);

            sleep(1);
            }

          if (i == 2) {
            printf("\n\n%s: \"that fucking bitch—\"\n", attacker->name);

            }

          if (i == 3) {
            printf("\n\n%s: \"and you, too!\"\n", attacker->name);

            sleep(1);
            }

          if (i == 4) {
            printf("\n\n%s: \"To hell with you all!\"\n", attacker->name);

            sleep(2);
            }

        } else {
          if (i == 0) {
            printf("\n\n%s: \"Yeah, I hate you all\"\n", attacker->name);

            sleep(2);
            } 

              if (i == 2) {
                printf("\n\n%s: \"The damn Famiglia\"\n", attacker->name);

                }

              if (i == 3) {
                printf("\n\n%s: \"and every single one\"\n", attacker->name);

                sleep(1);
                }

              if (i == 4) {
                printf("\n\n%s: \"of you!\"\n", attacker->name);

                sleep(2);
                }

            }
        }

      

      // ------------------------------------------------------------------------------------------------------------------------


      

    // ---------------------------------------- Meursault:The Thumb --------------------------------

    // Meursault:The Thumb - Tanut Skill 3-2
    if (isId(attacker->ID, "Meursault:The Thumb") ==
                   0 &&
               (atk == &attacker->skills[3]) && i == 0) {

      printf("\n\n%s: \"I shall now face you with all my might\"\n", attacker->name);

      sleep(2);
    }

     // Meursault:The Thumb - Tanut Skill 3
      if (isId(attacker->ID, "Meursault:The Thumb") ==
                     0 &&
                 (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && i == remainingCoins - 1) {

        if (attacker->Passive > 0) {
        printf("\n\n%s: \"Firing all rounds!\"\n", attacker->name);
        } else {
          printf("\n\n%s: \"Firing all rounds...\"\n", attacker->name);
        }

        sleep(1);
      }

      // Meursault:The Thumb - Spent flag Check
      if (isId(attacker->ID, "Meursault:The Thumb") == 0) {

        if (attacker->Passive > 0) {
          attacker->skills[5].active = 1;
        }

      }

    // Meursault:The Thumb S1-1
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[0]) && i == remainingCoins - 1 && attacker->Passive > 0 && !attacker->skills[3].active) {

        attacker->Passive--;
        attacker->skills[2].active++;

        Damage += (Damage * 0.10f);

         printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->Passive);


    } // Meursault:The Thumb S1-2
    else if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[0]) && i == remainingCoins - 1 && attacker->Passive > 0 && attacker->skills[3].active) {

        attacker->Passive--;
        attacker->skills[2].active++;

        Damage += (Damage * 0.30f);

         printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->Passive);

    }

    // Meursault:The Thumb S2-1
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[1]) && attacker->Passive > 0 && !attacker->skills[3].active) {

      if (i == remainingCoins - 2) {

        attacker->Passive--;
        attacker->skills[2].active++;

          Damage += (Damage * 0.10f);

           printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->Passive);

      } else if (i == remainingCoins - 1) {
        attacker->Passive--;
        attacker->skills[2].active++;

        Damage += (Damage * 0.10f);

         printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->Passive);

      }

    } // Meursault:The Thumb S2-2
    else if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[1]) && attacker->Passive > 0 && attacker->skills[3].active) {

        if (i == remainingCoins - 2) {

          attacker->Passive--;
          attacker->skills[2].active++;

            Damage += (Damage * 0.30f);

             printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->Passive);

        } else if (i == remainingCoins - 1) {
          attacker->Passive--;
          attacker->skills[2].active++;

          Damage += (Damage * 0.30f);

           printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->Passive);

        }

      }

    // Meursault:The Thumb S3-1
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[2]) && !attacker->skills[3].active && attacker->Passive > 0) {

      if (i == remainingCoins - 1) {

        attacker->Passive--;
        attacker->skills[2].active++;
          atk->DamageUp[0] += 50;

        Damage += (Damage * 0.10f);

         printf("\n%s spends 1 Tigermark Round (%d) to deal +50%% damage and activate the Tremor Burst effect 2 more times", attacker->name, attacker->Passive);

      } else {

        attacker->Passive--;
        attacker->skills[2].active++;

          Damage += (Damage * 0.10f);

           printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->Passive);

      }

    }

    // Meursault:The Thumb S3-2
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[3]) && attacker->skills[3].active && attacker->Passive > 0) {

      if (i == remainingCoins - 1) {

        attacker->Passive--;
        attacker->skills[2].active++;

        Damage += (Damage * 0.30f);

         printf("\n%s spends 1 Savage Tigermark Round (%d) to activate the Tremor Burst effect 2 more times", attacker->name, attacker->Passive);

      } else if (i == remainingCoins - 3) {

        attacker->Passive--;
        attacker->skills[2].active++;

          Damage += (Damage * 0.30f);

           printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->Passive);

      } else if (i == remainingCoins - 2) {

        attacker->Passive--;
        attacker->skills[2].active++;

          Damage += (Damage * 0.30f);

           printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->Passive);


      }

    }

    // ------------------------------------------------------------

      int SpendAmmoLeiheng = 0;
      int SpendSavageAmmoLeiheng = 0;

      // Unopposed Attacks deal more damage
      if (isId(attacker->ID, "Lei heng") == 0 && clashCount <= 0 && defSkill->skillType == 0) {
        Damage *= 1.3;
      }
      
      // Lei heng Spend Ammo
      if (isId(attacker->ID, "Lei heng") == 0 && (attacker->defenseSkill[1].active > 0 || attacker->defenseSkill[2].active > 0)) {

        // Skill Defense Spend
        if (atk == &attacker->defenseSkill[0]) {

          if (i == remainingCoins - 1) {

            if (attacker->defenseSkill[1].active > 0) {
              SpendAmmoLeiheng++;
              attacker->defenseSkill[1].active--;

              printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->defenseSkill[1].active);
            } else if (attacker->defenseSkill[2].active > 0) {
                SpendSavageAmmoLeiheng++;
            attacker->defenseSkill[2].active--;

           printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->defenseSkill[2].active);

            }
        }
        }

        // Skill 1 Spend
        if (atk == &attacker->skills[0] && attacker->skills[0].active >= 1) {
        
          if (i == remainingCoins - 1) {

            if (attacker->defenseSkill[1].active > 0) {
              SpendAmmoLeiheng++;
              attacker->defenseSkill[1].active--;

              printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->defenseSkill[1].active);
            } else if (attacker->defenseSkill[2].active > 0) {
                SpendSavageAmmoLeiheng++;
            attacker->defenseSkill[2].active--;

           printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->defenseSkill[2].active);

            }
        }
        }

        // Skill 2 Spend
        if (atk == &attacker->skills[1] && attacker->skills[0].active >= 1) {

          if (i == remainingCoins - 2 && i == remainingCoins - 1) {

            if (attacker->defenseSkill[1].active > 0) {
              SpendAmmoLeiheng++;
              attacker->defenseSkill[1].active--;

              printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->defenseSkill[1].active);
            } else if (attacker->defenseSkill[2].active > 0) {
                SpendSavageAmmoLeiheng++;
            attacker->defenseSkill[2].active--;

           printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->defenseSkill[2].active);

            }
        }
        }

        // Skill 4 Spend
        if (atk == &attacker->skills[3]) {

            if (attacker->defenseSkill[1].active > 0) {
              SpendAmmoLeiheng++;
              attacker->defenseSkill[1].active--;

              Damage *= 1.2;

              printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->defenseSkill[1].active);
            } else if (attacker->defenseSkill[2].active > 0) {

                SpendSavageAmmoLeiheng++;
            attacker->defenseSkill[2].active--;

              Damage *= 1.5;

           printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->defenseSkill[2].active);

        }

          if (i == remainingCoins - 1 && (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0)) {

            Damage *= 1.5;

            if (attacker->defenseSkill[1].active > 0) {
              
              printf("\n%s spent Tigermark Round due to this Coin's effect, deal +50%% damage", attacker->name);
              
              } else if (attacker->defenseSkill[2].active > 0) {

              printf("\n%s spent Savage Tigermark Round due to this Coin's effect, deal +50%% damage", attacker->name);
              
            }
          }
        }

        // Skill 3 Spend
        if (atk == &attacker->skills[2]) {

            if (attacker->defenseSkill[1].active > 0) {

              SpendAmmoLeiheng = attacker->defenseSkill[1].active;
              attacker->defenseSkill[1].active = 0;

              printf("\n%s spends all Tigermark Round (Spend %d)", attacker->name, SpendAmmoLeiheng);
            } else if (attacker->defenseSkill[2].active > 0) {

              SpendSavageAmmoLeiheng = attacker->defenseSkill[2].active;
            attacker->defenseSkill[2].active = 0;

           printf("\n%s spends all Savage Tigermark Round (Spend %d)", attacker->name, SpendSavageAmmoLeiheng);

        }
        }

        // Skill 5 Spend
        if (atk == &attacker->skills[4]) {

          if (attacker->defenseSkill[1].active > 0) {
            SpendAmmoLeiheng++;
            attacker->defenseSkill[1].active--;

            printf("\n%s spends 1 Tigermark Round (%d)", attacker->name, attacker->defenseSkill[1].active);
          } else if (attacker->defenseSkill[2].active > 0) {

              SpendSavageAmmoLeiheng++;
          attacker->defenseSkill[2].active--;

          printf("\n%s spends 1 Savage Tigermark Round (%d)", attacker->name, attacker->defenseSkill[2].active);

          }
          }

      }






      
    // ----------------- The One Who Grips Faust -----------------

    // The One Who Grips Faust Skill 3 Last coins
    if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[2] && i == remainingCoins - 1) {

      printf("\n\n%s: \"Pierce through...!\"\n", attacker->name);

      sleep(1);

    }

    // The One Who Grips Faust Skill 4 after Last coins
    if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[3]) {

      if (i == 0) {

      printf("\n\n%s: \"...To Goodness.\"\n", attacker->name);

      sleep(1);

      }

      if (i == remainingCoins - 1) {

      printf("\n\n%s: \"Purge!\"\n", attacker->name);

      sleep(1);

      }

    }

    // The One Who Grips Faust Skill 5 after Last coins
    if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[4]) {

      if (i == 0) {

      printf("\n\n%s: \"Your life...\"\n", attacker->name);

      sleep(1);

      }

    }

    // -------------------------------------------------------

    // Meursault:Blade Lineage Mentor - Skill 3
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") ==
                   0 &&
               (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: \"Yield My Flesh!\"\n", attacker->name);

      sleep(1);
    }

      // Meursault:Blade Lineage Mentor - Skill 4
      if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") ==
                     0 &&
                 (atk == &attacker->defenseSkill[1])) {

        if (attacker->defenseSkill[2].active == 1) {

          
        if (i == 0) {

        printf("\n\n%s: \"Though stark be the lunar brilliance\"\n", attacker->name);

        sleep(1);
        }

        if (i == remainingCoins - 1) {

        printf("\n\n%s: \"Its lambent gleam shall wane with the rise of the dark moon!\"\n", attacker->name);

        sleep(1);
        }

          
        } else {
          
          if (i == 0) {

          printf("\n\n%s: \"Yield My Flesh...\"\n", attacker->name);

          sleep(1);
          }

          if (i == remainingCoins - 1) {

          printf("\n\n%s: \"To Claim Their Bones!\"\n", attacker->name);

          sleep(1);
          }
          
        }

      }

    // Hong lu:The Lord of Hongyuan S2 Last coins
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[4]) && i == remainingCoins - 1) {

      printf("\n\n%s: \"Clear the path.\"\n", attacker->name);

      sleep(1);
    }

    // Hong lu:The Lord of Hongyuan S2 Last coins
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[1] || atk == &attacker->skills[4]) && i == remainingCoins - 1 && IsCritical) {

        atk->DamageUp[0] += 50;

         printf("\n%s's last coin deal +50%% damage with Critical Hit", attacker->name);

    }


      // ---------------- Meursault:Blade Lineage Mentor ----------------
      
    // Meursault:Blade Lineage Mentor S2 Last coins
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[1]) && i == 2 && IsCritical) {

        atk->DamageUp[0] += 60;

         printf("\n%s's this coin deal +60%% damage on Critical Hit", attacker->name);

    }

      // Meursault:Blade Lineage Mentor S3/S4 Last coins
      if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[2] || atk == &attacker->defenseSkill[1]) && IsCritical) {

          Damage *= 1.3;

           printf("\n%s's this coin deal +30%% damage on Critical Hit", attacker->name);

      }

      // ส่วนเหรียญสุดท้าย (Effect D: Final coin deal +(Poise Potency + Poise Count)% damage)
      if (i == remainingCoins - 1 && isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && atk == &attacker->defenseSkill[1] && attacker->defenseSkill[2].active == 1) {
          float finalCoinBonus = (float)((attacker->Poise[0] + attacker->Poise[1]) * 2) / 100.0f;
          Damage += (int)(Damage * finalCoinBonus);
          printf("\n%s's last coin deal +%.0f%% damage", attacker->name, finalCoinBonus * 100);
      }


      // ------------------------------------------------------------------------------------------------

    // Fixer grade 9? S8 Last coins
    if (isId(attacker->ID, "Fixer grade 9?") == 0 && (atk == &attacker->skills[7]) && i == remainingCoins - 1) {

        atk->DamageUp[0] += 50;

         printf("\n%s's last coin deal +50%% damage", attacker->name);

    }


    // ---------------------------- The Middle Little Brother Sinclair ------------------------

    // The Middle Little Brother Sinclair Skill 2 Last coins
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && (atk == &attacker->skills[1] || (atk == &attacker->skills[3] && attacker->Passive >= 2 && attacker->Passive < 4)) && attacker->Passive >= 3 && i == remainingCoins - 1) {

        atk->DamageUp[0] += 20;

         printf("\n%s at 3+ Envy Resonance, last coin deal +20%% damage", attacker->name);

      sleep(1);
    }

    // The Middle Little Brother Sinclair Skill 3 Last coins
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && (atk == &attacker->skills[2] || (atk == &attacker->skills[3] && attacker->Passive >= 4)) && attacker->Passive > 0 && i == remainingCoins - 1) {

      int damageupValue = attacker->Passive * 5;
      if (damageupValue > 30) damageupValue = 30;

        atk->DamageUp[0] += damageupValue;

         printf("\n%s deals +5%% damage for every Envy Resonance (%d%% - Max 30%%)", attacker->name, damageupValue);

      sleep(1);

      if (attacker->skills[1].active >= 10) {

        int damageupValue = (attacker->skills[1].active/10) * 15;
        if (damageupValue > 45) damageupValue = 45;

        atk->DamageUp[0] += damageupValue;

      printf("\n%s deals +15%% damage for every 10 Book of Vengeance [Sinclair] (%d%% - Max 45%%)", attacker->name, damageupValue);

      sleep(1);

      }
    }


      

    // ------------------- The One Who Grips Faust --------------------------

    // The One Who Grips Faust Skill 3/4 Last coins
    if (isId(attacker->ID, "The One Who Grips Faust") == 0 && (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && attacker->skills[2].active >= 5 && i == remainingCoins - 1) {

        atk->DamageUp[0] += 50;

         printf("\n%s at 5+ Nail, %s's last coin deal +50%% damage", defender->name, attacker->name);

      sleep(1);
    }

    // The One Who Grips Faust - Whistles Count
    if (isId(attacker->ID, "The One Who Grips Faust") == 0) {
        if (i == 0) attacker->skills[3].active++; // นับครั้งการทำกิจกรรม (Whistles)
    }

      // The One Who Grips Faust - Whistles Count
      if (isId(defender->ID, "The One Who Grips Faust") == 0 && Evaded) {
          if (i == 0) defender->skills[3].active++; // นับครั้งการทำกิจกรรม (Whistles)
      }

    // ------------------------------------------------------------------------


// --------------------------------------------------------------------------------------

      // ----------------- The House of Spiders: The Thumb Nursefather Rodion -----------------

      // The House of Spiders: The Thumb Nursefather Rodion - Shin damage Buff
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (attacker->Poise[0] > 0 || attacker->Poise[1] > 0)) {

        float gain = attacker->Poise[0] * 3;
        if (gain > 15) gain = 15;

          Damage *= 1 + (gain/100);
      }

      // The House of Spiders: The Thumb Nursefather Rodion - Skill 3-2 damage Buff
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && defender->HP <= defender->MAX_HP*0.5 && atk == &attacker->skills[5]) {

          Damage *= 1.5;
        
      }

          // The House of Spiders: The Thumb Nursefather Rodion – Passive Buff La Spada di Palermo
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && attacker->skills[12].active >= 5 
      && (atk == &attacker->skills[4] || atk == &attacker->skills[5])) {

        if (attacker->skills[10].active > 0) { // If there's still ammo left for this coin

          Damage *= 1.25;

        }

        if (defender->Shield > 0 || defender->TempShield > 0) {
          float gain = 3*defender->Burn[0]/2;
          if (gain > 45) gain = 45;

          Damage *= 1.0f + (gain / 100);

        } else {
          float gain = defender->Burn[0]/2;
          if (gain > 15) gain = 15;

            Damage *= 1.0f + (gain / 100);
        }

      }

      int spendAccelerationRoundCount = attacker->defenseSkill[5].active;

      // The House of Spiders: The Thumb Nursefather Rodion - Ammo Poise gain Skill 2-1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && attacker->skills[10].active > 0 && atk == &attacker->skills[2]) {

        if (i != remainingCoins - 1) {

          attacker->skills[10].active--;
          spendAccelerationRoundCount++;
          attacker->defenseSkill[5].active++;
          inflictStatus(attacker->Poise, 2, 2, 0, 99, 0, 99);
          
          printf("\n%s spends 1 Acceleration Round (%d) (Gain +2 Poise Stack (%d) and +2 Poise Count (%d))", attacker->name, attacker->skills[10].active, attacker->Poise[0], attacker->Poise[1]);
        }

      }

      // The House of Spiders: The Thumb Nursefather Rodion - Ammo Poise gain Skill 3-1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && atk == &attacker->skills[4]) {

        if (i != remainingCoins - 1 && attacker->skills[10].active > 0) {

          attacker->skills[10].active--;
          spendAccelerationRoundCount++;
          attacker->defenseSkill[5].active++;
          inflictStatus(attacker->Poise, 2, 2, 0, 99, 0, 99);

          printf("\n%s spends 1 Acceleration Round (%d) (Gain +2 Poise Stack (%d) and +2 Poise Count (%d))", attacker->name, attacker->skills[10].active, attacker->Poise[0], attacker->Poise[1]);
        }

        if (i == remainingCoins - 1) {

          printf("\n%s triggers more Tremor Burst for every Acceleration Round spent by this Skill (%d - Max 2)", attacker->name, spendAccelerationRoundCount > 2 ? 2 : spendAccelerationRoundCount);
        }

      }

      // The House of Spiders: The Thumb Nursefather Rodion - Ammo Poise gain Skill 3-2
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && attacker->skills[10].active > 0 && atk == &attacker->skills[5]) {

        if (i == remainingCoins - 1) {

          int spend = attacker->skills[10].active; // คำนวณจำนวนที่ต้องจ่าย (สูงสุด 2)
          if (spend > 5) spend = 5;

          attacker->skills[10].active -= spend;
          spendAccelerationRoundCount += spend;
          attacker->defenseSkill[5].active += spend;
          inflictStatus(attacker->Poise, 2*spend, 2*spend, 0, 99, 0, 99);

          printf("\n%s spends up to 5 Acceleration Round (%d left) (Gain +%d Poise Stack (%d) and +%d Poise Count (%d))", attacker->name, attacker->skills[10].active, 2*spend, attacker->Poise[0], 2*spend, attacker->Poise[1]);

          printf("\n%s triggers more Tremor Burst for every Acceleration Round spent by this Skill (%d - Max 3)", attacker->name, spend > 3 ? 3 : spend);

          atk->DamageUp[0] += spendAccelerationRoundCount*10;
          
          printf("\n%s deals +10%% damage for every Acceleration Round that will be spent by this Coin effect (%d%% - Max 50%%)", attacker->name, spend*10);
        }

      }

      // ------------------------------------------------------------


    // --------------------- The Index Nursefather Yi Sang ----------------------------------

    // --- [เพิ่ม] โบนัสโจมตีสำหรับ Sizzling Wound ---
    if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {

      // คำนวณลำดับเหรียญที่แท้จริง: (เหรียญทั้งหมด - เหรียญที่เหลืออยู่ตอนนี้) + ลำดับเหรียญใน Loop ปัจจุบัน
      int realCoinIndex = (atk->Coins - remainingCoins) + i; 

      int currentUnbreakEffective = (Unbreakable > atk->Unbreakable) ? Unbreakable : atk->Unbreakable;
      int RedCoinStartIndex = atk->Coins - currentUnbreakEffective;
      int isRedCoin = (realCoinIndex >= RedCoinStartIndex);

        if (attacker->skills[3].active == 2 && isRedCoin) {
                Damage *= 1.15; // เพิ่มดาเมจ 15% เฉพาะเหรียญแดง
                //printf(" [Sizzling Boost!] "); // เปิดไว้เช็ค log ได้
            }
    }

    // --- Yi sang Mask reduce damage from cracking coins ---
    if (isId(defender->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {
           if (Unbreakable > 0) { 
            if (defender->skills[3].active == 1) { Damage *= 0.9; } // ลดดาเมจ 10%
            if (defender->skills[3].active == 2) { Damage *= 0.75; } // ลดดาเมจ 25%
        }
    }


    if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && atk == &attacker->skills[3] && i == remainingCoins - 1) {

          atk->DamageUp[0] += 90.0f;

        printf("\nWeapon assigned to this Coin is fixed as Scythe and Deal +90%% damage");

    }






    // --- The Index Nursefather Yi Sang: Oracle Device [Caduceus] Damage Count ---

    int weapon = -1; // ประกาศตัวแปรเก็บไว้ก่อน
    const char* wName[] = {"Hatchet", "Stiletto", "Bastard Sword", "Rapier", "Hammer", "Greatsword", "Lance", "Whip", "Scythe"};

    if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {

      if (atk == &attacker->skills[3] && i == remainingCoins - 1) {
          weapon = 8; // Fixed as Scythe
      } else {
          weapon = rand() % 9;
      }

        // --- 1. คำนวณความเสียหายตามชนิดอาวุธ ---
        if (weapon == 0 && atk == &attacker->skills[0]) Damage *= 1.15;
        if (weapon == 1 && atk == &attacker->skills[1]) Damage *= 1.15;
        if (weapon == 2 && atk == &attacker->skills[2]) Damage *= 1.25;
        if (weapon == 3 && atk == &attacker->skills[1]) Damage *= 1.15;
        if (weapon == 4 && atk == &attacker->skills[0]) Damage *= 1.15; // Hammer Bonus
        if (weapon == 5 && atk == &attacker->skills[2]) Damage *= 1.25;
        if (weapon == 6 && atk == &attacker->skills[1]) Damage *= 1.15;
        if (weapon == 7 && atk == &attacker->skills[0]) Damage *= 1.15;
        if (weapon == 8 && atk == &attacker->skills[2]) Damage *= 1.25;


      if (weapon == 2 || weapon == 3 || weapon == 4) { Damage *= 1.05; }
        if (weapon == 5 || weapon == 6 || weapon == 7) { Damage *= 1.15; }
      if (weapon == 8) { Damage *= 1.30; }

    }






      double totalBonusPercent = (attacker->DamageUp[0] + atk->DamageUp[0]) - attacker->DamageDown[0];

      // --- 2. ถ้าติด Critical ให้ "บวก" เปอร์เซ็นต์เพิ่มเข้าไปใน Pool แทนการคูณแยก ---
      if (IsCritical) {
          // โบนัสคริติคอลพื้นฐานคือ 20% บวกกับบัฟ CriticalDamageUp ที่มี
          double critBonus = 20.0 + attacker->CriticalDamageUp[0] - attacker->CriticalDamageDown[0] + atk->CriticalDamageUp[0];
          totalBonusPercent += critBonus; 

          // พิมพ์บอกเพื่อให้เช็คได้ว่ามันบวกเข้า pool แล้ว
          // printf("\t [Crit Additive: +%.0f%%]", critBonus); 
      }

      // --- 3. แปลงเปอร์เซ็นต์รวมเป็นตัวคูณ (เช่น 100% กลายเป็น 2.0) ---
      double finalTotalMultiplier = 1.0 + (totalBonusPercent / 100.0);
      if (finalTotalMultiplier < 0) finalTotalMultiplier = 0;

      // ดึงค่า Protection จากสกิลป้องกันมาเก็บไว้ก่อน ถ้าไม่มีสกิลให้เป็น 0
      float extraDefProt = (defSkill != NULL) ? defSkill->Protection[0] : 0.0f;
      
      // --- 4. คำนวณ Protection (ตัวลดดาเมจยังคงให้คูณแยกตามปกติเพื่อให้เห็นผลชัดเจน) ---
      double protectionMult = 1.0 - (((defender->ProtectionUp[0] + extraDefProt) - defender->ProtectionDown[0]) / 100.0);
      if (protectionMult < 0) protectionMult = 0;

      // --- 5. สรุปดาเมจสุดท้าย ---
      int finalDamage = (int)(Damage * finalTotalMultiplier * protectionMult);

      if (defender->Stagger > 0) {
          finalDamage *= 1.5; // เพิ่มดาเมจเป็น 2 เท่า (100%)
          // หรือ finalDamage *= 1.5; ถ้าอยากได้แค่ 50%
      }




    // Evaded
    if (Evaded && !IsunableDefensetoact && !nodefense) {

      int PowerUp = 0;

      int defenseDiff = defTempDefense - atkTempOffense;
      if (defenseDiff > 0) {
            PowerUp += (defenseDiff / 3);
      }

      if (isId(defender->ID, "The One Who Grips Faust") == 0 && fanaticUsed > 0) {

        // คำนวณพลังหลบตามสูตร: Base 4 + (โยนเหรียญที่มีพลัง 10 + Fanatic)
        evadePower = 4 + (defender->BasePowerUp[0] - defender->BasePowerDown[0]);
        if (tossCoinWithSanity(defender)) {
            {
              int fanaticCoinPower = 10 + fanaticUsed; // always positive
              int charEvadeBoost1 = defender->PlusCoinPowerBoost[0] - defender->PlusCoinPowerDrop[0] + defSkill->CoinPowerBoost[0];
              evadePower += fanaticCoinPower + charEvadeBoost1
                          + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + PowerUp;
            }
        }

      } else if (isId(defender->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0) {

      SkillStats EvadeSkill = defender->defenseSkill[1];

        evadePower += EvadeSkill.BasePower + EvadeSkill.BasePowerBoost[0] + (defender->BasePowerUp[0] - defender->BasePowerDown[0]);

        int IsHeadHit = tossCoinWithSanity(defender);

        if (IsHeadHit) {
          // Check paralyze
          if (attacker->Paralyze[0] > 0) { // ← Character's paralyze
                evadePower += 0; // สกิลอื่นโดน Paralyze ปกติ
                defender->Paralyze[0]--;
          } else {
              {
                int charEvadeBoost2 = EvadeSkill.CoinPowerBoost[0];
                if (EvadeSkill.CoinPower >= 0) {
                  charEvadeBoost2 += defender->PlusCoinPowerBoost[0] - defender->PlusCoinPowerDrop[0];
                } else {
                  charEvadeBoost2 += defender->MinusCoinPowerBoost[0] - defender->MinusCoinPowerDrop[0];
                }
                evadePower += EvadeSkill.CoinPower + charEvadeBoost2
                            + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + PowerUp;
              }
            if (evadePower <= 0) evadePower = 0;
          }
        }
      } else if (isId(defender->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0) {

      SkillStats EvadeSkill = defender->defenseSkill[1];

        evadePower = EvadeSkill.BasePower + EvadeSkill.BasePowerBoost[0] + (defender->BasePowerUp[0] - defender->BasePowerDown[0]);

        int IsHeadHit = tossCoinWithSanity(defender);

        if (IsHeadHit) {
          // Check paralyze
          if (attacker->Paralyze[0] > 0) { // ← Character's paralyze
                evadePower += 0; // สกิลอื่นโดน Paralyze ปกติ
                defender->Paralyze[0]--;
          } else {
              {
                int charEvadeBoost2 = EvadeSkill.CoinPowerBoost[0];
                if (EvadeSkill.CoinPower >= 0) {
                  charEvadeBoost2 += defender->PlusCoinPowerBoost[0] - defender->PlusCoinPowerDrop[0];
                } else {
                  charEvadeBoost2 += defender->MinusCoinPowerBoost[0] - defender->MinusCoinPowerDrop[0];
                }
                evadePower += EvadeSkill.CoinPower + charEvadeBoost2
                            + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + PowerUp;
              }
            if (evadePower <= 0) evadePower = 0;
          }
        }
      } else if (defSkill != NULL) {
        evadePower = defSkill->BasePower + (defender->BasePowerUp[0] - defender->BasePowerDown[0]);

        int IsHeadHit = tossCoinWithSanity(defender);

        if (IsHeadHit) {
          // Check paralyze
          if (attacker->Paralyze[0] > 0) { // ← Character's paralyze
                evadePower += 0; // สกิลอื่นโดน Paralyze ปกติ
                defender->Paralyze[0]--;
          } else {
              {
                int charEvadeBoost3 = defSkill->CoinPowerBoost[0];
                if (defSkill->CoinPower >= 0) {
                  charEvadeBoost3 += defender->PlusCoinPowerBoost[0] - defender->PlusCoinPowerDrop[0];
                } else {
                  charEvadeBoost3 += defender->MinusCoinPowerBoost[0] - defender->MinusCoinPowerDrop[0];
                }
                evadePower += defSkill->CoinPower + charEvadeBoost3
                            + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + (defender->FinalPowerUp[0] - defender->FinalPowerDown[0]) + PowerUp;
              }
            if (evadePower <= 0) evadePower = 0;
          }

      }

      }

      if (evadePower >= currentPower) { // ถ้าพลังหลบมากกว่าหรือเท่ากับพลังโจมตี
        finalDamage = 0;
      } else {
        Evaded = 0;       // หลบพลาด! หยุดการหลบในเหรียญที่เหลือ

        if (defSkill != NULL && defSkill->skillType == 2) {
            defSkill->active = 0; // เหรียญหลบพังแล้ว! (มีผลข้ามไปถึง Attack ต่อๆ ไปด้วย)
        }
      }

    }

    if (!Evaded) {

      applyDamage(attacker, defender, finalDamage, 0, NULL);

    totalDamage += finalDamage;

    }

    printf("\n%-10d %-10d %-10d", i + 1, currentPower, finalDamage);


    if (IsStillEvaded && Evaded && !IsunableDefensetoact && !nodefense) {
      printf(" %d (Evaded)", evadePower);


      // ---------------- On Evaded ----------------

      if (isId(defender->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && defender->skills[5].active) {
        defender->skills[5].active = 0;

        attacker->Bind[1] += 1;

        printf("\t [On Evade] Inflict 1 Bind next turn against the attacker (Once per turn)");

          defender->Haste[1] += 1;

        printf("\t [On Evade] Gain 1 Haste next turn (Once per turn)");
      }

      if (isId(defender->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && defender->defenseSkill[3].active < 2) {
        defender->defenseSkill[3].active++;

        inflictStatus(defender->Poise, 2, 0, 0, 99, 0, 99);

        printf("\t [On Evade] Poise Stack +2 (%d) (2 times per turn)", defender->Poise[0]);

       updateSanity(defender, 3);

        printf("\t [On Evade] Heal 3 Sanity (%d) (2 times per turn)", defender->Sanity);
        
      }

      // ------------------------------------------------

    } else if (IsStillEvaded && !Evaded && !IsunableDefensetoact && !nodefense) {
      printf(" %d (Cancel)", evadePower);
      IsStillEvaded = 0;


      // ---------------- On Hit Evaded ----------------
      
    }


// Yi sang:Fell Bullet First Coin damage Save
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 &&
        atk == &attacker->skills[2] && i == remainingCoins - 2) {

      int missingHP = (int)(((float)finalDamage / defender->MAX_HP) * 100.0f);
      if (missingHP > 100) missingHP = 100;

      attacker->skills[5].active = missingHP;

    }


// The Index Nursefather Yi Sang: Oracle Device [Caduceus] Weapon Name + Other
      if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {
            printf(" [%s] ", wName[weapon]); 

            // 1. ตรวจสอบสถานะเหรียญปัจจุบัน
            // isCracked = เหรียญนี้แพ้ Clash มา (แตก) จึงไม่แสดงผลพิเศษ
            // isRedCoin = เหรียญนี้เป็นเหรียญแดง (Unbreakable Coin ของ Yi Sang)
            int isCracked = (Unbreakable > 0);

            // 2. เอฟเฟกต์ On Hit (ทำงานเฉพาะเหรียญที่ไม่แตก)
            if (!isCracked) {
                // [Hammer: Tremor Burst System]
                if (weapon == 4 && !Evaded) {
                    attacker->skills[10].active += 3;
                    if (attacker->skills[10].active > 99) attacker->skills[10].active = 99;
                    int deal = attacker->skills[10].active;
                    attacker->skills[12].active += deal; // Store tremor burse damage

                  applyDamage(attacker, defender, deal, 0, NULL);

                    totalDamage += deal;
                    printf("\t Trigger 'Tremor Burst' (Stack %d Count 0) ", deal);

                    if (attacker->skills[12].active > 20 && defender->Stagger <= 0) {
                        defender->Stagger += 2;
                        printf("\t Target 'Stagger' for one turn");
                        attacker->skills[12].active = 0; // Reset
                    }
                    attacker->skills[10].active = 0;
                }

                // เอฟเฟกต์อาวุธอื่นๆ (จำกัด 2 ครั้งต่อเทิร์น)
                if (weapon == 0 && attacker->skills[15].active < 2 && !Evaded) { attacker->skills[15].active++; attacker->DamageUp[1] += 5; printf("(Gain +5%% damage next turn) "); }
                if (weapon == 1 && attacker->skills[16].active < 2 && !Evaded) { attacker->skills[16].active++; updateSanity(defender, -2); printf("(Target loses 2 Sanity (%d)) ", defender->Sanity); }
                if (weapon == 2 && attacker->skills[17].active < 2 && !Evaded) { attacker->skills[17].active++; attacker->OffenseLevelUp[1] += 1; printf("(Offense +1 next turn) "); }
                if (weapon == 3 && attacker->skills[18].active < 2 && !Evaded) { attacker->skills[18].active++; defender->DefenseLevelDown[1] += 1; printf("(Target gains 1 Defense Down Next Turn) "); }
                if (weapon >= 5 && weapon <= 7 && attacker->skills[6].active < 2 && !Evaded) { attacker->skills[6].active++; defender->ProtectionDown[1] += 5; printf("(Target takes +5%% damage Next Turn) "); }


            }


        // --- 3. กฎการรับแต้ม Hermes ---
        // เงื่อนไข: (ไม่ได้ใช้ Furioso)
              // คำนวณลำดับเหรียญที่แท้จริง: (เหรียญทั้งหมด - เหรียญที่เหลืออยู่ตอนนี้) + ลำดับเหรียญใน Loop ปัจจุบัน
              int realCoinIndex = (atk->Coins - remainingCoins) + i; 

        int currentUnbreakEffective = (Unbreakable > atk->Unbreakable) ? Unbreakable : atk->Unbreakable;
        int RedCoinStartIndex = atk->Coins - currentUnbreakEffective;
        int isRedCoin = (realCoinIndex >= RedCoinStartIndex);

              if (attacker->skills[8].active == 0 && isRedCoin) {
                int hermesHardCap = (attacker->Passive < 2) ? 8 : 9;
                int maxGainThisTurn = attacker->Passive + 2;

                if (attacker->skills[1].active < hermesHardCap && attacker->skills[13].active < maxGainThisTurn && !Evaded) {
                    attacker->skills[1].active++;
                    attacker->skills[13].active++;
                    printf("\t Procuration [Hermes] +1 (%d) ", attacker->skills[1].active);

                  // For prescript III Check
                  if (attacker->Passive == 2) {
                    attacker->skills[5].active = 1;
                    }

                }
            }

        }

    // -----------------------------------------------------------------------------------







    // -------------------------------- Heishou Pack - You Branch Adept Heathcliff --------------------------------

    // Heishou Pack - You Branch Adept Heathcliff - Gain on attack
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && !Evaded) {

      int gain = 1;
      if (attacker->HP < attacker->MAX_HP * 0.5) gain += 1;

        attacker->Passive += gain;
      if (attacker->Passive > 20) attacker->Passive = 20;

      printf("\t +%d Battleblood Instinct (%d)", gain, attacker->Passive);

    }

    // Heishou Pack - You Branch Adept Heathcliff - Bloodflame buff
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && attacker->skills[2].active > 0 && i < 3 && !Evaded) {

      int gain = 3;

      if (attacker->Sanity >= 45) {
        gain = 1;
        attacker->OffenseLevelUp[1]++;

        printf(" \t+%d Offense next turn", gain);
      } else {

      updateSanity(attacker, gain);
      if (attacker->Sanity > 45) attacker->Sanity = 45;

      printf(" \t+%d Sanity (%d)", gain, attacker->Sanity);

      }

    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 3 coins
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i != remainingCoins - 1 && !Evaded) {

          inflictStatus(attacker->Burn, 2, 0, 0, 99, 0, 99);

      printf("\n%s applies +2 Burn Stack(%d) on self", attacker->name, attacker->Burn[0]);

      sleep(1);
    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 2 Last coins
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[1]) && i == remainingCoins - 1 && !Evaded) {

          inflictStatus(attacker->Burn, 3, 1, 0, 99, 0, 99);

      printf("\n%s applies +3 Burn Stack(%d) and +1 Burn Count(%d) on self", attacker->name, attacker->Burn[0], attacker->Burn[1]);

      sleep(1);

      float damageboost = attacker->Burn[0] * 2.0f;
      if (damageboost > 20) damageboost = 20;
      int damage = finalDamage * (damageboost / 100);

      applyDamage(attacker, defender, damage, 0, NULL);

      totalDamage += damage;

         printf("\n%s deals %d ((Burn Stack on self x 2)%% of this Coin's final damage) addition damage (%.0f%% - Max 20%%)", attacker->name, damage, damageboost);

      sleep(1);
    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 3 Last coins
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i == remainingCoins - 1 && !Evaded) {

      float damageboost = (attacker->Burn[0] + attacker->Burn[1]) * 2.0f;
      if (damageboost > 20) damageboost = 20;
      int damage = finalDamage * (damageboost / 100);

      applyDamage(attacker, defender, damage, 0, NULL);

      totalDamage += damage;

         printf("\n%s deals %d ((Burn Stack and Count on self x 2)%% of this Coin's final damage) addition damage (%.0f%% - Max 20%%)", attacker->name, damage, damageboost);

      sleep(1);
    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 4 Last coins
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[3]) && i == remainingCoins - 1 && !Evaded) {

      float damageboost = 20.0f;
      int damage = finalDamage * (damageboost / 100);

      applyDamage(attacker, defender, damage, 0, NULL);

      totalDamage += damage;

         printf("\n%s deals %d (20%% of this Coin's final damage) addition damage", attacker->name, damage);

      sleep(1);

      float healvalue = finalDamage + damage;

      if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && remainingCoins > attacker->skills[3].Coins && (attacker->Burn[0] >= 20 || attacker->HP <= attacker->MAX_HP * 0.5)) {

        attacker->HP += healvalue;

        if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;

        printf("\n%s's final Coin, heal HP by the amount of damage the above effect dealt (%.0f)", attacker->name, healvalue);

        sleep(1);

      } else if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && remainingCoins <= attacker->skills[3].Coins && attacker->Burn[0] < 20 && attacker->HP > attacker->MAX_HP * 0.5) {

        attacker->HP += healvalue;

        if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;


        printf("\n%s's final Coin, heal HP by the amount of damage the above effect dealt (%0.f)", attacker->name, healvalue);

         sleep(1);
    }
    }

       if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {
      // Check on Hit for prescript II
      // หา Index ของสกิลที่กำลังใช้อยู่
      int atk_idx = -1;
      if (atk == &attacker->skills[0]) atk_idx = 0;
      else if (atk == &attacker->skills[1]) atk_idx = 1;
      else if (atk == &attacker->skills[2]) atk_idx = 2;
      else if (atk == &attacker->skills[3]) atk_idx = 3;

      // เงื่อนไขความสำเร็จ Prescript II: "Hit" a target with a marked skill
      if (attacker->Passive == 1 && atk_idx == attacker->skills[4].active) {
          if (attacker->skills[5].active == 0 && !Evaded) {
              attacker->skills[5].active = 1;
              //printf(" [Prescript II: Hit Success!] ");
          }
      }

       }
    // ----------------------------------------------------------------











  // ---------------- Keyword Status ----------------

    // Poise // 0 Stack 1 Count

    if (IsCritical) {

      if (attacker->Poise[1] <= 0) attacker->Poise[1] = 0;

    printf("\t Critical Hit! (Chance %d%% Count %d)", PoiseChance, attacker->Poise[1]);

    if (attacker->Poise[1] <= 0) attacker->Poise[0] = 0;

    }

    // Rupture // 0 Stack 1 Count
    if ((defender->Rupture[0] > 0 || defender->Rupture[1] > 0) && !Evaded) {

      if (defender->Rupture[0] <= 0 && defender->Rupture[1] > 0) defender->Rupture[0]++;

          int deal = defender->Rupture[0];

            defender->Rupture[1] -= 1;

           if (defender->Rupture[1] <= 0) defender->Rupture[1] = 0;

            printf("\t Deal %d Rupture damage on enemy (Count %d)", deal, defender->Rupture[1]);

          if (defender->Rupture[1] <= 0) defender->Rupture[0] = 0;

         applyDamage(attacker, defender, deal, 0, "Rupture");

            totalDamage += deal;

        }

    // Bleed // 0 Stack 1 Count
    if ((attacker->Bleed[0] > 0 || attacker->Bleed[1] > 0) && atk->skillType == 0) {

      int damage = attacker->Bleed[0] > 0 ? attacker->Bleed[0] : 1;

        attacker->Bleed[1]--;

      if (attacker->Bleed[1] <= 0) attacker->Bleed[1] = 0;

      printf("\t Take %d Bleed damage (Count %d)", damage, attacker->Bleed[1]);

      if (attacker->Bleed[1] <= 0) attacker->Bleed[0] = 0;

      applyDamage(NULL, attacker, damage, 0, "Bleed");

    }

      // Sinking // 0 Stack 1 Count
      if ((defender->Sinking[0] > 0 || defender->Sinking[1] > 0) && !Evaded) {

        if (defender->Sinking[0] <= 0 && defender->Sinking[1] > 0) defender->Sinking[0]++;

          if (defender->hasSanity == 1) { // Normal
          int deal = defender->Sinking[0];

            updateSanity(defender, -(deal));

            defender->Sinking[1] -= 1;

            if (defender->Sinking[1] <= 0) defender->Sinking[1] = 0;

          printf("\t Sanity -%d on enemy (%d) (Count %d)", deal, defender->Sanity, defender->Sinking[1]);

            if (defender->Sinking[1] <= 0) defender->Sinking[0] = 0;

          } else { // No Sanity enemy

            int deal = defender->Sinking[0];

            defender->Sinking[1] -= 1;

             if (defender->Sinking[1] <= 0) defender->Sinking[1] = 0;

              printf("\t Deal %d Sinking damage on enemy (Count %d)", deal, defender->Sinking[1]);

            if (defender->Sinking[1] <= 0) defender->Sinking[0] = 0;

            applyDamage(attacker, defender, deal, 0, "Sinking");

              totalDamage += deal;

          }
        }

      // -----------------------------------------

  // --------------------------------------------------------------------------------





  // ----------------------- Inflict Status -----------------------





      // --------------- Lei heng -----------------

            int TremorStack = 0;
            int TremorCount = 0;
            int BurnStack = 0;
            int BurnCount = 0;
            int PoiseStack = 0;
            int PoiseCount = 0;

            // Lei heng - Inflict on attack
            if (isId(attacker->ID, "Lei heng") == 0 && !Evaded) {

              // No Defense Skill equipped
              if (defSkill->skillType == 0) {
              // Unopposed Attacks
              if (clashCount <= 0) {

                defender->ProtectionDown[1] += 10;

                printf("\t [On Hit] Take Damage Up +10%% on enemy next turn");
              }
              
              // Ammo Spent
              if (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0) {

                defender->DefenseLevelDown[1] += 2;
                
                printf("\t [On Hit] Defense Level Down +2 on enemy next turn");
              }
              }

              // Defense Skill
                if (atk == &attacker->defenseSkill[0]) {
                if (i == remainingCoins - 2) {
                    TremorStack += 2;
                }

                if (i == remainingCoins - 1) {
                  if (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0) {
                    BurnStack += 3;
                    BurnCount += 3;
                    if (SpendSavageAmmoLeiheng > 0) {
                      BurnStack += 3;
                      BurnCount += 3;
                    }
                  }
              }

                }

              // Skill 1
              if (atk == &attacker->skills[0]) {
              if (i == remainingCoins - 2) {
                  TremorStack += 1;
                if (attacker->skills[0].active >= 1 && (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0)) {
                  TremorStack += 1;
                  if (SpendSavageAmmoLeiheng > 0) {
                    TremorStack *= 2;
                  }
                }
              }

              if (i == remainingCoins - 1) {
                TremorStack += 3;
                TremorCount += 2;
                if (attacker->skills[0].active >= 1 && (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0)) {
                  BurnStack += 3;
                  BurnCount += 2;
                  if (SpendSavageAmmoLeiheng > 0) {
                    TremorStack += 3;
                    TremorCount += 2;
                      BurnStack += 3;
                      BurnCount += 2;
                  }
                }
            }
                
              }

              // Skill 2
                if (atk == &attacker->skills[1]) {
                if (i == remainingCoins - 3) {
                    TremorCount += 2;
                }

                if (i == remainingCoins - 2) {
                  TremorStack += 1;
                  TremorCount += 2;
                  if (attacker->skills[0].active >= 1) {
                    TremorStack++;
                    if (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0) {
                    BurnStack += 3;
                    BurnCount += 2;
                      if (SpendSavageAmmoLeiheng > 0) {
                        TremorStack *= 2;
                        TremorCount *= 2;
                          BurnStack *= 2;
                          BurnCount *= 2;
                      }
                    }
                  }
              }

                  if (i == remainingCoins - 1) {
                    if (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0) {
                      int Stack = 3;
                      int Count = 2;

                      if (SpendSavageAmmoLeiheng > 0) {
                        TremorStack *= 2;
                        TremorCount *= 2;
                          BurnStack *= 2;
                          BurnCount *= 2;
                      }

                      if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[4].active == 1 && attacker->skills[0].active == 3) {
                        if (Stack > 0) Stack += 3;
                        if (Count > 0) Count++;
                          } else if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[4].active == 1) {
                        if (Stack > 0) Stack += 2;
                          }

                      inflictStatus(defender->Burn, Stack, 0, 0, 99, 0, 99);
                      printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, defender->Burn[0]);
                      
                      inflictStatus(defender->Burn, 0, Count, 0, 99, 0, 99);
                      printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, defender->Burn[1]);
                      }

                    if (attacker->skills[0].active >= 1) {
                      if (Unbreakable <= 0) {
                    for (int i = 0; i < 2; i++) {
                       printf("\t [On Hit without Cracking]");
                        attacker->skills[12].active++;
                      TremorBurst(attacker, defender, defender->MAX_HP, &totalDamage, 1);
                    }
                      }
                    } else {
                      printf("\t [On Hit]");
                        attacker->skills[12].active++;
                      TremorBurst(attacker, defender, defender->MAX_HP, &totalDamage, 1);
                    }
                  }
                  
                }

              
              // Skill 4
                if (atk == &attacker->skills[3]) {
                if (i == remainingCoins - 3) {
                    TremorStack += 3;

                  if (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0) {
                    BurnStack += 3;
                  }

                  if (SpendSavageAmmoLeiheng > 0) {
                    TremorStack *= 2;
                    TremorCount *= 2;
                      BurnStack *= 2;
                      BurnCount *= 2;
                  }

                    if (Unbreakable <= 0) {
                      int Stack = 4;
                      int Count = 4;

                      if (SpendSavageAmmoLeiheng > 0) {
                          Stack *= 2;
                          Count *= 2;
                      }

                      if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[4].active == 1 && attacker->skills[0].active == 3) {
                        if (Stack > 0) Stack += 3;
                        if (Count > 0) Count++;
                          } else if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[4].active == 1) {
                        if (Stack > 0) Stack += 2;
                          }

                      inflictStatus(defender->Tremor, Stack, 0, 0, 99, 0, 99);
                      printf("\t [On Hit without Cracking] Tremor Stack +%d on enemy (%d)", Stack, defender->Tremor[0]);
                      inflictStatus(defender->Tremor, 0, Count, 0, 99, 0, 99);
                      printf("\t [On Hit without Cracking] Tremor Count +%d on enemy (%d)", Stack, defender->Tremor[1]);
                    }
                }

                if (i == remainingCoins - 2) {
                  TremorCount += 3;

                  if (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0) {
                    BurnCount += 3;
                  }

                  if (SpendSavageAmmoLeiheng > 0) {
                    TremorStack *= 2;
                    TremorCount *= 2;
                      BurnStack *= 2;
                      BurnCount *= 2;
                  }

                   if (Unbreakable <= 0) {
                  defender->TremorType = "Scorch";

                  printf("\t [On Hit without Cracking] Trigger 'Amplitude Conversion' into 'Tremor - Scorch'");
                   }
              }

                  if (i == remainingCoins - 1) {

                    if (attacker->skills[0].active >= 1) {
                      if (Unbreakable <= 0) {
                    for (int i = 0; i < 2; i++) {
                       printf("\t [On Hit without Cracking]");
                        attacker->skills[12].active++;
                      TremorBurst(attacker, defender, defender->MAX_HP, &totalDamage, 1);
                    }
                      }
                    } else {
                      printf("\t [On Hit]");
                        attacker->skills[12].active++;
                      TremorBurst(attacker, defender, defender->MAX_HP, &totalDamage, 1);
                    }
                  }

                }

              // Skill 3
                if (atk == &attacker->skills[2]) {

                  int Stack = 0;
                  int Count = 0;
                  
                  if (SpendAmmoLeiheng > 0) {
                  Stack = SpendAmmoLeiheng;
                  Count = SpendAmmoLeiheng;
                  } else if (SpendSavageAmmoLeiheng > 0) {
                      Stack = SpendSavageAmmoLeiheng;
                      Count = SpendSavageAmmoLeiheng;
                      }

                  if (SpendSavageAmmoLeiheng > 0) {
                      Stack *= 2;
                      Count *= 2;
                  }

                  if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[4].active == 1 && attacker->skills[0].active == 3) {
                    if (Stack > 0) Stack += 3;
                    if (Count > 0) Count++;
                      } else if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[4].active == 1) {
                    if (Stack > 0) Stack += 2;
                      }

                  if (!ClashLostAttack) {
                  inflictStatus(defender->Tremor, Stack, Count, 0, 99, 0, 99);
                    if (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0) {
                  inflictStatus(defender->Burn, Stack, Count, 0, 99, 0, 99);
                    }
                  } else {
                    inflictStatus(defender->Tremor, Stack/2, Count/2, 0, 99, 0, 99);
                    if (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0) {
                    inflictStatus(defender->Burn, Stack/2, Count/2, 0, 99, 0, 99);
                    }
                  }

                    printf("\t [On Hit] Inflict Burn Stack (%d), Burn Count (%d), Tremor Stack (%d), and Tremor Count (%d) by Rounds spent", defender->Burn[0], defender->Burn[1], defender->Tremor[0], defender->Tremor[1]);
                  
                   if (Unbreakable <= 0) {
                  defender->TremorType = "Scorch";

                  printf("\t [On Hit without Cracking] Trigger 'Amplitude Conversion' into 'Tremor - Scorch'");

                    for (int i = 0; i < 3; i++) {
                       printf("\t [On Hit without Cracking]");
                        attacker->skills[12].active++;
                      TremorBurst(attacker, defender, defender->MAX_HP, &totalDamage, 1);
                    }
                }

            }


              // Skill 5
                  if (atk == &attacker->skills[4]) {

                    if (i == remainingCoins - 6 && (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0)) {
                    if (!ClashLostAttack) {
                    BurnStack += 6;
                    } else {
                      BurnStack += 6/2;
                    }
                    }

                    if (i == remainingCoins - 5 && (SpendAmmoLeiheng > 0 || SpendSavageAmmoLeiheng > 0)) {
                      if (!ClashLostAttack) {
                      BurnCount += 6;
                      } else {
                          BurnCount += 6/2;
                      }
                      }

                    if (i == remainingCoins - 4) {
                      if (!ClashLostAttack) {
                      TremorStack += 6;
                      } else {
                            TremorStack += 6/2;
                      }
                      }

                    if (i == remainingCoins - 3) {
                      if (!ClashLostAttack) {
                      TremorCount += 6;
                      } else {
                       TremorCount += 6/2;
                      }
                      }

                    if (SpendSavageAmmoLeiheng > 0) {
                      TremorStack *= 2;
                      TremorCount *= 2;
                        BurnStack *= 2;
                        BurnCount *= 2;
                    }

                    if (i == remainingCoins - 2) {
                      if (Unbreakable <= 0) {
                        defender->TremorType = "Scorch";

                        printf("\t [On Hit without Cracking] Trigger 'Amplitude Conversion' into 'Tremor - Scorch'");
                      }
                    }

                      if (i == remainingCoins - 1 && Unbreakable <= 0) {
                      for (int i = 0; i < 5; i++) {
                         printf("\t [On Hit without Cracking]");
                          attacker->skills[12].active++;
                        TremorBurst(attacker, defender, defender->MAX_HP, &totalDamage, 1);
                      }
                  }

              }

            }

      if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[4].active == 1 && attacker->skills[0].active == 3) {
        if (TremorStack > 0) TremorStack += 3;
        if (TremorCount > 0) TremorCount++;
          } else if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[4].active == 1) {
        if (TremorStack > 0) TremorStack += 2;
          }

              // ---------------------------------------------------------------------------

      // --------------- The House of Spiders: The Thumb Nursefather Rodion -----------------

      // The House of Spiders: The Thumb Nursefather Rodion - Inflict on attack Skill 1-1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (atk == &attacker->skills[0]) && !Evaded) {

        if (i == remainingCoins - 2) {
            TremorStack += 1;
              BurnStack += 1;
        }

        if (i == remainingCoins - 1) {
          TremorCount += 1;
            BurnCount += 1;
      }

      }

      // The House of Spiders: The Thumb Nursefather Rodion - Inflict on attack Skill 1-2
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (atk == &attacker->skills[1]) && !Evaded) {

        if (i == remainingCoins - 2) {
          TremorCount += 2;
          BurnCount += 1;
        }

        if (i == remainingCoins - 1) {
           printf("\t [On Hit]");
          attacker->skills[12].active++;
          TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);
      }

      }

      // The House of Spiders: The Thumb Nursefather Rodion - Inflict on attack Skill 2-1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (atk == &attacker->skills[2]) && !Evaded) {

        if (i == remainingCoins - 3) {
          TremorStack += 2;
          BurnStack += 2;
        }

        if (i == remainingCoins - 2) {
          TremorStack += 1;
            TremorCount += 1;
        }

        if (i == remainingCoins - 1) {
           printf("\t [On Hit]");
          attacker->skills[12].active++;
          TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);
      }

      }

      // The House of Spiders: The Thumb Nursefather Rodion - Inflict on attack Skill 2-2
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (atk == &attacker->skills[3]) && !Evaded) {

        if (i == remainingCoins - 3) {
          PoiseStack += 5;
          BurnStack += 2;
        }

        if (i == remainingCoins - 2) {
          TremorStack += 2;
            TremorCount += 2;

          defender->TremorType = "Scorch";

          printf("\t [On Hit] Trigger 'Amplitude Conversion' into 'Tremor - Scorch'");
        }

        if (i == remainingCoins - 1) {
           printf("\t [On Hit]");
          attacker->skills[12].active++;
          TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);
      }

      }

      // The House of Spiders: The Thumb Nursefather Rodion - Inflict on attack Skill 3-1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (atk == &attacker->skills[4]) && !Evaded) {

        if (i == remainingCoins - 4) {
          TremorStack += 2;
            BurnStack += 2;
        }

        if (i == remainingCoins - 3) {
            BurnCount += 2;
          TremorCount += 2;
        }

        if (i == remainingCoins - 2) {

          defender->TremorType = "Scorch";

          printf("\t [On Hit] Trigger 'Amplitude Conversion' into 'Tremor - Scorch'");
        }

        if (i == remainingCoins - 1) {
          if (spendAccelerationRoundCount > 2) spendAccelerationRoundCount = 2;
          for (int i = 0; i < spendAccelerationRoundCount + 1; i++) {
           printf("\t [On Hit]");
            attacker->skills[12].active++;
          TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);
          }
      }

      }

      // The House of Spiders: The Thumb Nursefather Rodion - Inflict on attack Skill 3-2
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (atk == &attacker->skills[5]) && !Evaded) {
      
        if (i == remainingCoins - 5 || i == remainingCoins - 4) {
          TremorStack += 2;
            BurnStack += 2;
        }
      
        if (i == remainingCoins - 3) {
            BurnCount += 3;
          TremorCount += 3;
        }
      
        if (i == remainingCoins - 2) {
      
          defender->TremorType = "Scorch";
      
          printf("\t [On Hit] Trigger 'Amplitude Conversion' into 'Tremor - Scorch'");
        }
      
        if (i == remainingCoins - 1) {
          if (spendAccelerationRoundCount > 3) spendAccelerationRoundCount = 3;
          for (int i = 0; i < spendAccelerationRoundCount + 1; i++) {
           printf("\t [On Hit]");
            attacker->skills[12].active++;
          TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);
          }
      }
      
      }

      // The House of Spiders: The Thumb Nursefather Rodion - Inflict on defense Skill 1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (atk == &attacker->defenseSkill[0]) && !Evaded) {

          TremorStack += 1;
            BurnStack += 1;

      }

if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && attacker->skills[13].active == 1) {
  if (TremorStack > 0) TremorStack++;
  if (TremorCount > 0) TremorCount++;
  if (BurnStack > 0) BurnStack++;
  if (BurnCount > 0) BurnCount++;
    }

      // ---------------------------------------------------------------------------







      
      // Inflict Print
      if (!Evaded) {

        // On self

        if (PoiseStack > 0) {
          inflictStatus(attacker->Poise, PoiseStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Poise Stack +%d on self (%d)", PoiseStack, attacker->Poise[0]);
        }

          if (PoiseCount > 0) {
            inflictStatus(attacker->Poise, 0, PoiseCount, 0, 99, 0, 99);
            printf("\t [On Hit] Poise Count +%d on self (%d)", PoiseCount, attacker->Poise[1]);
          }

        // On enemy

        if (TremorStack > 0) {
        inflictStatus(defender->Tremor, TremorStack, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", TremorStack, defender->Tremor[0]);
      }

        if (TremorCount > 0) {
          inflictStatus(defender->Tremor, 0, TremorCount, 0, 99, 0, 99);
          printf("\t [On Hit] Tremor Count +%d on enemy (%d)", TremorCount, defender->Tremor[1]);
        }

        if (BurnStack > 0) {
          inflictStatus(defender->Burn, BurnStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d on enemy (%d)", BurnStack, defender->Burn[0]);
        }

          if (BurnCount > 0) {
            inflictStatus(defender->Burn, 0, BurnCount, 0, 99, 0, 99);
            printf("\t [On Hit] Burn Count +%d on enemy (%d)", BurnCount, defender->Burn[1]);
          }

      }











      

// --------------- The House of Spiders: The Ring Nursefather Hong Lu -----------------

      int Stack = 0;
      int Count = 0;

      if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && attacker->skills[14].active && attacker->Charge[0] >= 2) {

        Stack++;
        Count++;
      }

// The House of Spiders: The Ring Nursefather Hong Lu - Inflict on attack Skill 1
if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[0]) && !Evaded) {

  if (i == remainingCoins - 2) {
    Stack += 4;
  inflictStatus(defender->Bleed, Stack, 0, 0, 99, 0, 99);
  printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", Stack, defender->Bleed[0]);
    CorpusTheater(attacker, defender);
}

  if (i == remainingCoins - 1) {
    attacker->DefenseLevelUp[1] += 2;
    printf("\t [On Hit] Gain Defense Level Up 2 next turn");
  defender->DefenseLevelDown[1] += 2;
  printf("\t [On Hit] Inflict Defense Level Down 2 on enemy next turn");
}

}

      // The House of Spiders: The Ring Nursefather Hong Lu - Inflict on attack Skill 2-1
      if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[1]) && !Evaded) {

        if (i == remainingCoins - 3) {
          Count += 2;
        inflictStatus(defender->Bleed, 0, Count, 0, 99, 0, 99);
        printf("\t [On Hit] Bleed Count +%d on enemy (%d)", Count, defender->Bleed[1]);
          CorpusTheater(attacker, defender);
      }

        if (i == remainingCoins - 2) {
          attacker->Haste[1] += 1;
          printf("\t [On Hit] Haste +1 on self next turn");
        defender->Bind[1] += 1;
        printf("\t [On Hit] Bind +1 on enemy next turn");
      }

        if (i == remainingCoins - 1) {
          inflictStatus(attacker->Charge, 0, 4, 0, 99, 0, 20);
            printf("\t [On Hit] Corpus Ingredient Count +4 (%d)", attacker->Charge[1]);
        }

      }

      // The House of Spiders: The Ring Nursefather Hong Lu - Inflict on attack Skill 2-2
      if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[2]) && !Evaded) {

        if (i == remainingCoins - 2) {
          Stack += 2;
        inflictStatus(defender->Bleed, Stack, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", Stack, defender->Bleed[0]);
          CorpusTheater(attacker, defender);
      }

        if (i == remainingCoins - 1) {
          updateSanity(attacker, 5);
          printf("\t [On Hit] Heal 5 Sanity on self (%d)", attacker->Sanity);
          updateSanity(defender, -5);
          printf("\t [On Hit] Deal 5 Sanity Damage (%d)", defender->Sanity);
          attacker->skills[4].active = 3;
          printf("\t [On Hit] Inflict Corpus Theater");
      }

      }

       // The House of Spiders: The Ring Nursefather Hong Lu - Inflict on attack Skill Defense
        if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->defenseSkill[0]) && !Evaded) {

          if (i == remainingCoins - 2) {
              Count += 1;
            inflictStatus(defender->Bleed, 0, Count, 0, 99, 0, 99);
            printf("\t [On Hit] Bleed Count +%d on enemy (%d)", Count, defender->Bleed[1]);
              CorpusTheater(attacker, defender);
          }

          if (i == remainingCoins - 1) {
            Stack += 2;
          inflictStatus(defender->Bleed, Stack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", Stack, defender->Bleed[0]);
            CorpusTheater(attacker, defender);
        }

        }

       // The House of Spiders: The Ring Nursefather Hong Lu - Inflict on attack Skill 3-1
        if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[3]) && !Evaded) {

          if (i == remainingCoins - 4) {
            Stack += 4;
          inflictStatus(defender->Bleed, Stack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", Stack, defender->Bleed[0]);
            CorpusTheater(attacker, defender);
          }

          if (i == remainingCoins - 3) {
            attacker->Haste[1] += 1;
            printf("\t [On Hit] Haste +1 on self next turn");
          defender->Bind[1] += 1;
          printf("\t [On Hit] Bind +1 on enemy next turn");
          }

          if (i == remainingCoins - 2) {
            attacker->DamageUp[1] += 10;
            printf("\t [On Hit] Damage Up +10%% on self next turn");
          defender->DamageDown[1] += 10;
          printf("\t [On Hit] Damage Down 10%% on enemy next turn");
          }

          if (i == remainingCoins - 1) {
            Count += 1;
          inflictStatus(defender->Bleed, 0, Count, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Count +%d on enemy (%d)", Count, defender->Bleed[1]);
            CorpusTheater(attacker, defender);
          }

        }

       // The House of Spiders: The Ring Nursefather Hong Lu - Inflict on attack Skill 3-2
        if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[4]) && !Evaded) {

          if (i == remainingCoins - 5) {
            Stack += 4;
          inflictStatus(defender->Bleed, Stack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", Stack, defender->Bleed[0]);
            CorpusTheater(attacker, defender);
          }

          if (i == remainingCoins - 4) {
            attacker->Haste[1] += 1;
            printf("\t [On Hit] Haste +1 on self next turn");
          defender->Bind[1] += 1;
          printf("\t [On Hit] Bind +1 on enemy next turn");
          }

          if (i == remainingCoins - 3) {
            attacker->DamageUp[1] += 10;
            printf("\t [On Hit] Damage Up +10%% on self next turn");
          defender->DamageDown[1] += 10;
          printf("\t [On Hit] Damage Down 10%% on enemy next turn");
          }

          if (i == remainingCoins - 2) {
            Count += 1;
          inflictStatus(defender->Bleed, 0, Count, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Count +%d on enemy (%d)", Count, defender->Bleed[1]);
            CorpusTheater(attacker, defender);
          }

          if (i == remainingCoins - 1) {
          inflictStatus(defender->Burn, 5, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +5 on enemy (%d)", defender->Burn[0]);
            attacker->skills[4].active = 3;
            printf("\t [On Hit] Inflict Corpus Theater");
          }

        }

// --------------------------------------------------------------------

      // --------------- Meursault:Blade Lineage Mentor -----------------

      // Meursault:Blade Lineage Mentor - Gain on attack Skill 1
      if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[0]) && !Evaded) {

        inflictStatus(attacker->Poise, 1, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Stack +1 on self (%d)", attacker->Poise[0]);

      }

      // Meursault:Blade Lineage Mentor - Gain on attack
      if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[1]) && i == 0 && !Evaded) {

        inflictStatus(attacker->Poise, 3, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Stack +3 on self (%d)", attacker->Poise[0]);

      }

      // Meursault:Blade Lineage Mentor - Gain on attack
      if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->defenseSkill[0]) && i == 0 && !Evaded) {

        inflictStatus(attacker->Poise, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Stack +2 on self (%d)", attacker->Poise[0]);

      }

      // --- เพิ่มหลังจากการเรียก applyDamage ---
      // Effect E: On Hit, heal HP by 10% of damage dealt
      if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && atk == &attacker->defenseSkill[1] && attacker->defenseSkill[2].active == 1 && !Evaded) {
          int healAmt = (int)(finalDamage * 0.1f);
          if (healAmt < 0) healAmt = 0;
          attacker->HP += healAmt;
          if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;
          printf("\t HP +%d", healAmt);
      }

      // ---------------------------------------------------------


       // ----------------- Meursault:The Thumb -----------------

      // Meursault:The Thumb - Skill Defense
      if (isId(attacker->ID, "Meursault:The Thumb") == 0 && atk == &attacker->defenseSkill[0] && !Evaded) {

        if (i == remainingCoins - 2) {
          int inflictStack = 1;
            int inflictCount = 0;
            if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack += 1; inflictCount += 1; }
          if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack += 2; inflictCount += 2; }

            if (inflictStack > 0) {
                inflictStatus(&defender->Tremor[0], inflictStack, 0, 0, 99, 0, 99); // Status, Stack, Count
            printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
            }

            if (inflictCount > 0) {
              inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
              printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
              }

          }

        if (i == remainingCoins - 1) {

      int inflictStack = 0;
        int inflictCount = 1;
        if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack > 0 ? inflictStack += 1 : 0; inflictCount > 0 ? inflictCount += 1 : 0; }
      if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }
        if (inflictStack > 0) {
        inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
        }

        if (inflictCount > 0) {
          inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
          printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
          }

        }

      }

      // Meursault:The Thumb - Skill 1
      if (isId(attacker->ID, "Meursault:The Thumb") == 0 && atk == &attacker->skills[0] && !Evaded) {

        if (i == remainingCoins - 2) {
          int inflictStack = 1;
            int inflictCount = 0;
          if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack > 0 ? inflictStack += 1 : 0; inflictCount > 0 ? inflictCount += 1 : 0; }
          if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

            if (inflictStack > 0) {
            inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
            printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
            }

            if (inflictCount > 0) {
              inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
              printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
              }

          }

        if (i == remainingCoins - 1) {

      int inflictStack = 2;
        int inflictCount = 0;
          if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack > 0 ? inflictStack += 1 : 0; inflictCount > 0 ? inflictCount += 1 : 0; }
          if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }
        if (inflictStack > 0) {
        inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
        }

        if (inflictCount > 0) {
          inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
          printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
          }

          if (attacker->skills[5].active == 1) { // Spent check

          int inflictStack = 2;
          int inflictCount = 0;
          if (attacker->skills[3].active) { inflictStack += 2; inflictCount += 1; }

          if (inflictStack > 0) {
          inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);
          }

          if (inflictCount > 0) {
            inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
            printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);
            }

        }

        }


          attacker->skills[5].active = 0; // Reset Spent flag Check

      }

      // Meursault:The Thumb - Skill 2
      if (isId(attacker->ID, "Meursault:The Thumb") == 0 && atk == &attacker->skills[1] && !Evaded) {

        if (i == remainingCoins - 3) {
          int inflictStack = 1;
            int inflictCount = 0;
          if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack > 0 ? inflictStack += 1 : 0; inflictCount > 0 ? inflictCount += 1 : 0; }
          if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

            if (inflictStack > 0) {
            inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
            printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
            }

            if (inflictCount > 0) {
              inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
              printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
              }

          }

        if (i == remainingCoins - 2) {

      int inflictStack = 2;
        int inflictCount = 0;
          if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack > 0 ? inflictStack += 1 : 0; inflictCount > 0 ? inflictCount += 1 : 0; }
          if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }
        if (inflictStack > 0) {
        inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
        }

        if (inflictCount > 0) {
          inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
          printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
          }

          if (attacker->skills[5].active == 1) { // Spent check

          int inflictStack = 2;
          int inflictCount = 0;
          if (attacker->skills[3].active) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

          if (inflictStack > 0) {
          inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);
          }

          if (inflictCount > 0) {
            inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
            printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);
            }

        }
        }

          if (i == remainingCoins - 1) {

        int inflictStack = 0;
          int inflictCount = 2;
            if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack > 0 ? inflictStack += 1 : 0; inflictCount > 0 ? inflictCount += 1 : 0; }
            if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

          if (inflictStack > 0) {
          inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
          }

          if (inflictCount > 0) {
            inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
            printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
            }

            if (attacker->skills[5].active == 1) { // Spent check

            int inflictStack = 0;
            int inflictCount = 2;
              if (attacker->skills[3].active) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

            if (inflictStack > 0) {
            inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
            printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);
            }

            if (inflictCount > 0) {
              inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
              printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);
              }

          }

            if (defender->Tremor[1] >= 3) {

            printf("\t [On Hit] Target has 3+ Tremor Count (%d),", defender->Tremor[1]);

              TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

            }

          }


          attacker->skills[5].active = 0; // Reset Spent flag Check

      }


      // Meursault:The Thumb - Skill 3-1
      if (isId(attacker->ID, "Meursault:The Thumb") == 0 && atk == &attacker->skills[2] && !Evaded) {

        if (i == remainingCoins - 3) {

          int inflictStack = 3;
            int inflictCount = 0;
          if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack > 0 ? inflictStack += 1 : 0; inflictCount > 0 ? inflictCount += 1 : 0; }
          if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }
            if (inflictStack > 0) {
            inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
            printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
            }

            if (inflictCount > 0) {
              inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
              printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
              }

              if (attacker->skills[5].active == 1) { // Spent check

              int inflictStack = 3;
              int inflictCount = 0;
                if (attacker->skills[3].active) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

              if (inflictStack > 0) {
              inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
              printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);
              }

              if (inflictCount > 0) {
                inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
                printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);
                }

            }

          }

        if (i == remainingCoins - 2) {

      int inflictStack = 0;
        int inflictCount = 3;
        if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack += 1; inflictCount += 1; }
      if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack += 2; inflictCount += 2; }
        if (inflictStack > 0) {
        inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
        }

        if (inflictCount > 0) {
          inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
          printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
          }

          if (attacker->skills[5].active == 1) { // Spent check

          int inflictStack = 0;
          int inflictCount = 3;
            if (attacker->skills[3].active) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

          if (inflictStack > 0) {
          inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);
          }

          if (inflictCount > 0) {
            inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
            printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);
            }

        }
        }

          if (i == remainingCoins - 1) {

            defender->TremorType = "Scorch";

            printf("\t [On Hit] Trigger 'Amplitude Conversion' into 'Tremor - Scorch'");

            printf("\t [On Hit]");

            TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

            if (attacker->skills[5].active == 1) { // Spent check

              printf("\t [On Hit]");

              TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

              printf("\t [On Hit]");

              TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

          }

          }


          attacker->skills[5].active = 0; // Reset Spent flag Check

      }


      // Meursault:The Thumb - Skill 3-2
      if (isId(attacker->ID, "Meursault:The Thumb") == 0 && atk == &attacker->skills[3] && !Evaded) {

        if (i == remainingCoins - 5) {

        int inflictStack = 3;
          int inflictCount = 0;
          if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack += 1; inflictCount += 1; }
        if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack += 2; inflictCount += 2; }
          if (inflictStack > 0) {
          inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
          }

          if (inflictCount > 0) {
            inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
            printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
            }

        }

        if (i == remainingCoins - 4) {

        int inflictStack = 0;
          int inflictCount = 3;
          if (attacker->skills[3].active && attacker->skills[2].active < 8) { inflictStack += 1; inflictCount += 1; }
        if (attacker->skills[3].active && attacker->skills[2].active >= 8) { inflictStack += 2; inflictCount += 2; }
          if (inflictStack > 0) {
          inflictStatus(defender->Tremor, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Tremor Stack +%d on enemy (%d)", inflictStack, defender->Tremor[0]);
          }

          if (inflictCount > 0) {
            inflictStatus(defender->Tremor, 0, inflictCount, 0, 99, 0, 99);
            printf("\t [On Hit] Tremor Count +%d on enemy (%d)", inflictCount, defender->Tremor[1]);
            }

        }

        if (i == remainingCoins - 3) {

              if (attacker->skills[5].active == 1) { // Spent check

              int inflictStack = 3;
              int inflictCount = 0;
                if (attacker->skills[3].active) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

              if (inflictStack > 0) {
              inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
              printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);
              }

              if (inflictCount > 0) {
                inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
                printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);
                }

            }

          }

        if (i == remainingCoins - 2) {

            if (attacker->skills[5].active == 1) { // Spent check

            int inflictStack = 0;
            int inflictCount = 3;
              if (attacker->skills[3].active) { inflictStack > 0 ? inflictStack += 2 : 0; inflictCount > 0 ? inflictCount += 2 : 0; }

            if (inflictStack > 0) {
            inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
            printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);
            }

            if (inflictCount > 0) {
              inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
              printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);
              }

          }

        }

        if (i == remainingCoins - 1) {

          defender->TremorType = "Scorch";

          printf("\t [On Hit] Trigger 'Amplitude Conversion' into 'Tremor - Scorch'");

          printf("\t [On Hit]");

          TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

            if (attacker->skills[5].active == 1) { // Spent check

              printf("\t [On Hit]");

              TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

              printf("\t [On Hit]");

              TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

          }

          }


          attacker->skills[5].active = 0; // Reset Spent flag Check

      }

    // ------------------------------------------------------------------------------------------------------



      // ----------------- The Middle Nursefather - Matthias -----------------

      // The Middle Nursefather - Matthias - Skill 1
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->skills[0] && !Evaded) {

        if (i == remainingCoins - 2) {
          int inflict = 3;
          if (attacker->skills[9].active == 1) inflict = 4; 
          if (attacker->skills[9].active == 2) inflict = 5; 
          if (attacker->skills[2].active > 0) inflict++;
          inflictStatus(defender->Bleed, inflict, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", inflict, defender->Bleed[0]);
        } 

        if (i == remainingCoins - 1) {
          int inflictStack = 0;
          if (attacker->skills[2].active > 0) inflictStack++;

          if (inflictStack > 0) {
          inflictStatus(defender->Bleed, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", inflictStack, defender->Bleed[0]);
          }

          int inflict = 2;
          if (attacker->skills[9].active == 2) inflict = 3; 
          inflictStatus(defender->Bleed, 0, inflict, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Count +%d on enemy (%d)", inflict, defender->Bleed[1]);
        }

      }

      // The Middle Nursefather - Matthias - Skill 2
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->skills[1] && !Evaded) {

        int inflict = 5;
        if (attacker->skills[9].active >= 1) inflict += 5; 
        if (attacker->skills[9].active >= 2) inflict += 5; 
        printf("\t [On Hit] Raise Stagger Threshold by %d", inflict);
        MoveStagger(attacker, defender, defender->MAX_HP/3, inflict, 1);

        int inflictBleed = 0;
        if (attacker->skills[2].active > 0) inflictBleed++;
        if (inflictBleed > 0) {
        inflictStatus(defender->Bleed, inflictBleed, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", inflictBleed, defender->Bleed[0]);
      }

      }

      // The Middle Nursefather - Matthias - Skill 3
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->skills[2] && !Evaded) {

        if (i == remainingCoins - 4) {
          int inflict = 3;
          if (attacker->skills[9].active == 1) inflict = 4; 
          if (attacker->skills[9].active == 2) inflict = 5; 
          if (attacker->skills[2].active > 0) inflict++;
          inflictStatus(defender->Bleed, inflict, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", inflict, defender->Bleed[0]);
        } 

        if (i == remainingCoins - 3) {
          int inflictStack = 0;
          if (attacker->skills[2].active > 0) inflictStack++;

          if (inflictStack > 0) {
          inflictStatus(defender->Bleed, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", inflictStack, defender->Bleed[0]);
          }

          int inflict = 2;
          if (attacker->skills[9].active == 2) inflict = 3; 
          inflictStatus(defender->Bleed, 0, inflict, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Count +%d on enemy (%d)", inflict, defender->Bleed[1]);
        } 

        if (i == remainingCoins - 2) {

          int inflictStack = 0;
          if (attacker->skills[2].active > 0) inflictStack++;

          if (inflictStack > 0) {
          inflictStatus(defender->Bleed, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", inflictStack, defender->Bleed[0]);
          }

          if (Unbreakable <= 0) {
            int inflict = 3;
            if (attacker->skills[9].active == 1) inflict = 4; 
            if (attacker->skills[9].active == 2) inflict = 5; 
            if (attacker->skills[2].active > 0) inflict++;
            inflictStatus(defender->Bleed, inflict, 0, 0, 99, 0, 99);
            printf("\t [On Hit without Cracking] Bleed Stack +%d on enemy (%d)", inflict, defender->Bleed[0]);
          }
        }

        if (i == remainingCoins - 1) {

          int inflictStack = 0;
          if (attacker->skills[2].active > 0) inflictStack++;

          if (inflictStack > 0) {
          inflictStatus(defender->Bleed, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", inflictStack, defender->Bleed[0]);
          }

          if (Unbreakable <= 0) {
            int inflict = 2;
            if (attacker->skills[9].active == 2) inflict = 3; 
            inflictStatus(defender->Bleed, 0, inflict, 0, 99, 0, 99);
            printf("\t [On Hit without Cracking] Bleed Count +%d on enemy (%d)", inflict, defender->Bleed[1]);
          }
        }

      }

      // The Middle Nursefather - Matthias - Skill 4
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->skills[3] && !Evaded) {

        if (Unbreakable <= 0 && attacker->skills[9].active == 0 && attacker->skills[2].active > 0) {
          int deal = 5;

          applyDamage(attacker, defender, deal, 0, NULL);

          totalDamage += deal;
          printf("\t Deal %d bonus damage", deal);
        } else if (Unbreakable <= 0 && attacker->skills[9].active > 0 && attacker->skills[2].active > 0) {

          int deal = 10;

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;
            printf("\t Deal %d bonus damage", deal);
        }

        if (attacker->skills[9].active >= 1) {
          int inflict = 2;
          if (attacker->skills[9].active >= 1) inflict += 2;
          if (attacker->skills[9].active >= 2) inflict += 2;
          inflictStatus(defender->Burn, inflict, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflict, defender->Burn[0]);
        }

        if (i == remainingCoins - 3) {
          int inflict = 2;
          inflictStatus(defender->Bind, 0, inflict, 0, 99, 0, 99);
          printf("\t [On Hit] Bind +%d on enemy next turn (Speed -(Stack) for one turn)", inflict);
        } 

        if (i == remainingCoins - 2 && attacker->skills[9].active < 1) {

          int deal = 10;

          applyDamage(attacker, defender, deal, 0, NULL);

          totalDamage += deal;
          printf("\t [On Hit] Deal %d bonus damage", deal);
        } else  if (i == remainingCoins - 2 && attacker->skills[9].active >= 1 && Unbreakable <= 0) {
          int deal = 15;

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;
            printf("\t [On Hit without Cracking] Deal %d bonus damage", deal);
          }

        if (i == remainingCoins - 1) {
          if (Unbreakable <= 0) {
            int inflict = 2;
            if (attacker->skills[9].active >= 1) inflict = 3; 
            defender->Paralyze[0] += inflict;
            printf("\t [On Hit without Cracking] Paralyze +%d on enemy (Fix the Power of %d Coins to 0 for one turn)", inflict, inflict);
          }
        }

      }

      // The Middle Nursefather - Matthias - Skill 5/6/7
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[4] || atk == &attacker->skills[5] || atk == &attacker->skills[6]) && !Evaded) {

        if (Unbreakable <= 0 && attacker->skills[9].active == 0 && attacker->skills[2].active > 0) {
          int deal = 5;

          applyDamage(attacker, defender, deal, 0, NULL);

          totalDamage += deal;
          printf("\t Deal %d bonus damage", deal);
        } else if (Unbreakable <= 0 && attacker->skills[9].active > 0 && attacker->skills[2].active > 0) {
          int deal = 10;

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;
            printf("\t Deal %d bonus damage", deal);
        }

        if (Unbreakable <= 0 && i != remainingCoins - 1) {
          int deal = 10;

          applyDamage(attacker, defender, deal, 0, NULL);

          totalDamage += deal;
          printf("\t [On Hit without Cracking] Deal %d bonus damage", deal);
        } else if (Unbreakable <= 0 && i == remainingCoins - 1) {

          int take = 50;
          if (attacker->skills[9].active >= 3) take = 100; 

          defender->ProtectionDown[1] += take;
          printf("\t [On Hit without Cracking] Target takes +%d%% damage next turn", take);
        }

      }

      // The Middle Nursefather - Matthias - Skill 8
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->skills[7] && !Evaded) {

        if (i == remainingCoins - 2 && Unbreakable <= 0) {

        int inflict = 10;

        printf("\t [On Hit without Cracking] Raise Stagger Threshold by %d", inflict);

          MoveStagger(attacker, defender, defender->MAX_HP/3, inflict, 1);

        }

        if (i == remainingCoins - 1) {

          int inflictStack = 10;
          int inflictCount = 2;
          inflictStatus(defender->Burn, inflictStack, inflictCount, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d (%d) and Burn Count +%d (%d) on enemy", inflictStack, defender->Burn[0], inflictCount, defender->Burn[1]);

        }

      }

      // The Middle Nursefather - Matthias - Skill 9
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->skills[8] && !Evaded) {

          if (Unbreakable <= 0 && ((i == remainingCoins - 5) || (i == remainingCoins - 4))) {

            int deal = 10;

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;
            printf("\t [On Hit without Cracking] Deal %d bonus damage", deal);

        }

        if ((i == remainingCoins - 3) || (i == remainingCoins - 1)) {

          int inflictStack = 10;
          inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);

        }

        if (i == remainingCoins - 2) {

          int inflictCount = 4;
          inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);

        }

      }

      // The Middle Nursefather - Matthias - Skill 10
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->skills[9] && !Evaded) {

          if (i == remainingCoins - 5) {

            int inflictStack = 5;
            inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
            printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);

        }

        if (i == remainingCoins - 4) {

          int inflictCount = 3;
          inflictStatus(defender->Burn, 0, inflictCount, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflictCount, defender->Burn[1]);

          if (Unbreakable <= 0) {
            int deal = 10;

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;
            printf("\t [On Hit without Cracking] Deal %d bonus damage", deal);

          }

        }

        if ((i == remainingCoins - 3) || (i == remainingCoins - 2)) {

          int inflictStack = 10;
          inflictStatus(defender->Burn, inflictStack, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflictStack, defender->Burn[0]);

          if (Unbreakable <= 0 && (i == remainingCoins - 3)) {

            int deal = 10;

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;
            printf("\t [On Hit without Cracking] Deal %d bonus damage", deal);

          }

        }

        if (Unbreakable <= 0 && (i == remainingCoins - 1)) {

          int deal = 20;

          applyDamage(attacker, defender, deal, 0, NULL);

          totalDamage += deal;
          printf("\t [On Hit without Cracking] Deal %d bonus damage", deal);

        }

      }


      // The Middle Nursefather - Matthias - Defense Skill
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->defenseSkill[0] && !Evaded) {

        if (i == remainingCoins - 2 && attacker->skills[9].active != 3) {
          int inflict = 3;
          if (attacker->skills[9].active == 1) inflict = 4; 
          if (attacker->skills[9].active == 2) inflict = 5; 
          inflictStatus(defender->Bleed, inflict, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Bleed Stack +%d on enemy (%d)", inflict, defender->Bleed[0]);
        } else if (i == remainingCoins - 2 && attacker->skills[9].active == 3) {
          int inflict = 10;
          inflictStatus(defender->Burn, inflict, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Stack +%d on enemy (%d)", inflict, defender->Burn[0]);
        }

        if (i == remainingCoins - 1 && attacker->skills[9].active != 3) {
          attacker->Tremor[4] -= 10;
          if (attacker->Tremor[4] < 0) attacker->Tremor[4] = 0;
          printf("\t [On Hit] Lower user's Stagger Threshold by 10");
        } else if (i == remainingCoins - 1 && attacker->skills[9].active == 3) {
          int inflict = 2;
          inflictStatus(defender->Burn, 0, inflict, 0, 99, 0, 99);
          printf("\t [On Hit] Burn Count +%d on enemy (%d)", inflict, defender->Burn[1]);
        }

      }

      // --------------------------------------------------------------------

    // ---------------------- Yi sang:Fell Bullet ----------------------

    // Yi sang:Fell Bullet - DefSkill 1 Gain / Inflict
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 && atk == &attacker->defenseSkill[0] && !Evaded) {

      if (i == remainingCoins - 1) {
          inflictStatus(attacker->Poise, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Stack +2 on self (%d)", attacker->Poise[0]);
        inflictStatus(attacker->Poise, 0, 2, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Count +2 on self (%d)", attacker->Poise[1]);
      }

    }

    // Yi sang:Fell Bullet - Skill 1 Gain / Inflict
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 && atk == &attacker->skills[0] && !Evaded) {

      if (i == remainingCoins - 1) {
          inflictStatus(attacker->Poise, 0, 1, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Count +1 on self (%d)", attacker->Poise[1]);

        if (IsCritical) {
          int inflict = 1;
          if (attacker->skills[2].active > 0) inflict += 1;
          inflictStatus(defender->Bleed, 0, inflict, 0, 99, 0, 99);
          printf("\t [On Crit] Bleed Count +%d on enemy (%d)", inflict, defender->Bleed[1]);

          attacker->Passive += 1;
          if (attacker->Passive > 7) attacker->Passive = 7;

          printf("\t [On Crit] Torn Memory +1 (%d/7)", attacker->Passive);
        }

      }

    }

    // Yi sang:Fell Bullet - Skill 2 Gain / Inflict
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 && atk == &attacker->skills[1] && !Evaded) {

      if (i == remainingCoins - 2) {
          inflictStatus(attacker->Poise, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Stack +2 on self (%d)", attacker->Poise[0]);

        if (IsCritical) {
          int inflict = 2;
          if (attacker->skills[2].active > 0) inflict += 1;
          inflictStatus(defender->Bleed, inflict, 0, 0, 99, 0, 99);
          printf("\t [On Crit] Bleed Stack +%d on enemy (%d)", inflict, defender->Bleed[0]);
        }

      }

      if (i == remainingCoins - 1) {

        inflictStatus(attacker->Poise, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Stack +2 (%d)", attacker->Poise[0]);

        attacker->Passive += 1;
        if (attacker->Passive > 7) attacker->Passive = 7;

        printf("\t [On Hit] Torn Memory +1 (%d/7)", attacker->Passive);

        if (IsCritical) {
          int inflict = 2;
          if (attacker->skills[2].active > 0) inflict += 1;
          inflictStatus(defender->Bleed, inflict, 0, 0, 99, 0, 99);
          printf("\t [On Crit] Bleed Stack +%d on enemy (%d)", inflict, defender->Bleed[0]);

          attacker->Passive += 2;
          if (attacker->Passive > 7) attacker->Passive = 7;

          printf("\t [On Crit] Torn Memory +2 (%d/7)", attacker->Passive);
        }

      }

    }

    // Yi sang:Fell Bullet - Skill 3 Gain / Inflict
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 && atk == &attacker->skills[2] && !Evaded) {

      if (i == remainingCoins - 1) {
            inflictStatus(defender->Bleed, 3, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Bleed Stack +3 (%d)", defender->Bleed[0]);

        int inflict = 1;
        if (attacker->skills[2].active > 0) inflict += 1;
        inflictStatus(defender->Bleed, 0, inflict, 0, 99, 0, 99);
        printf("\t [On Hit] Bleed Count +%d on enemy (%d)", inflict, defender->Bleed[1]);

        if (IsCritical) {
          int inflict = 5;
          if (attacker->skills[2].active > 0) inflict += 1;
          inflictStatus(defender->Bleed, inflict, 0, 0, 99, 0, 99);
          printf("\t [On Crit] Bleed Stack +%d (%d)", inflict, defender->Bleed[0]);

          inflict = 3;
          if (attacker->skills[2].active > 0) inflict += 1;
          inflictStatus(defender->Bleed, 0, inflict, 0, 99, 0, 99);
          printf("\t [On Crit] Bleed Count +%d on enemy (%d)", inflict, defender->Bleed[1]);
        }

      }

    }

    // ------------------------------------------------------------------

    // ---------------------- Heathcliff:Wild Hunt ----------------------

    // Heathcliff:Wild Hunt – Skill 2 Inflict
    if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
       (atk == &attacker->skills[1])) {

      if (i == remainingCoins - 1) {
        attacker->skills[4].active += 3;
        printf("\t [On Hit] Impending Ruin +3 on enemy (%d)", attacker->skills[4].active);
      }

    }

    // ------------------------------------------------------------------

    // ---------------------- Hong lu:The Lord of Hongyuan ----------------------

    // Hong lu:The Lord of Hongyuan - Skill 1 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[0] && !Evaded) {

      if (i == remainingCoins - 2) {
          inflictStatus(attacker->Poise, 0, 3, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Count +3 on self (%d)", attacker->Poise[1]);
      }

      if (i == remainingCoins - 1) {
        inflictStatus(defender->Rupture, 0, 2, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Count +2 on enemy (%d)", defender->Rupture[1]);
      }

    }

    // Hong lu:The Lord of Hongyuan - Skill 2 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[1] && !Evaded) {

      if (i == remainingCoins - 4) {
        inflictStatus(defender->Rupture, 0, 2, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Count +2 on enemy (%d)", defender->Rupture[1]);
      }

      if (i == remainingCoins - 3) {
          inflictStatus(attacker->Poise, 0, 2, 0, 99, 0, 99);
        printf("\t [On Hit] Poise Count +2 on self (%d)", attacker->Poise[1]);
      }

      if (i == remainingCoins - 2) {
        inflictStatus(defender->Rupture, 1, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Stack +1 on enemy (%d)", defender->Rupture[0]);
      }

      if (i == remainingCoins - 1) {
        inflictStatus(defender->Rupture, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Stack +2 on enemy (%d)", defender->Rupture[0]);
      }

    }

    // Hong lu:The Lord of Hongyuan - Skill 3 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[2] && !Evaded) {

      if (i == remainingCoins - 1) {
        inflictStatus(defender->Rupture, 0, 5, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Count +5 on enemy (%d)", defender->Rupture[1]);
      }

    }

    // Hong lu:The Lord of Hongyuan - Skill 4 Inflict
      if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[3] && !Evaded) {

          inflictStatus(defender->Rupture, 1, 0, 0, 99, 0, 99);
          printf("\t [On Hit] Rupture Stack +1 on enemy (%d)", defender->Rupture[0]);

        if (i == remainingCoins - 1) {
          attacker->FinalPowerUp[1] += 2;
          printf("\t [On Hit] Gain +2 Final Power next turn");
          defender->ProtectionDown[1] += 10;
          printf("\t [On Hit] Target takes +10%% damage next turn");
        }

      }

    // Hong lu:The Lord of Hongyuan - Skill 5 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[4] && !Evaded) {

      if (i == remainingCoins - 2) {
        inflictStatus(defender->Rupture, 0, 1, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Count +1 on enemy (%d)", defender->Rupture[1]);
      }

      if (i == remainingCoins - 1) {
        inflictStatus(defender->Rupture, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Stack +2 on enemy (%d)", defender->Rupture[0]);
        defender->ProtectionDown[1] += 10;
        printf("\t [On Hit] Target takes +10%% damage next turn");
      }

    }

    // Heshin Packs - Mao - Skill 7 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[6] && !Evaded) {

      if (i == remainingCoins - 3) {
        inflictStatus(defender->Rupture, 0, 2, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Count +2 on enemy (%d)", defender->Rupture[1]);
      }

      if (i == remainingCoins - 2) {
        inflictStatus(defender->Rupture, 1, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Stack +1 on enemy (%d)", defender->Rupture[0]);
      }

      if (i == remainingCoins - 1) {
        inflictStatus(defender->Rupture, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Stack +2 on enemy (%d)", defender->Rupture[0]);
      }

    }

    // Heshin Packs - Si - Skill 8 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[7] && !Evaded) {

      if (i == remainingCoins - 1) {
        inflictStatus(defender->Rupture, 0, 2, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Count +2 on enemy (%d)", defender->Rupture[1]);
      }

    }

    // Heshin Packs - Wu - Skill 9 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[8] && !Evaded) {

      if (i == remainingCoins - 3) {
        inflictStatus(defender->Rupture, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Stack +2 on enemy (%d)", defender->Rupture[0]);
        inflictStatus(defender->Tremor, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Tremor Stack +2 on enemy (%d)", defender->Tremor[0]);
      }

      if (i == remainingCoins - 2) {
        defender->Bind[1] += 1;
        printf("\t [On Hit] Bind +1 on enemy next turn (%d) (Speed -(Stack) for one turn)", defender->Bind[1]);
        inflictStatus(defender->Tremor, 0, 1, 0, 99, 0, 99);
        printf("\t [On Hit] Tremor Count +1 on enemy (%d)", defender->Tremor[1]);
      }

      if (i == remainingCoins - 1) {
        TremorBurst(attacker, defender, 20, &totalDamage, 1);
      }

    }

    // Heshin Packs - You - Skill 10 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[9] && !Evaded) {

        inflictStatus(defender->Burn, 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit] Burn Stack +2 on enemy (%d)", defender->Burn[0]);

      if (i == remainingCoins - 1) {
        inflictStatus(defender->Rupture, 0, 3, 0, 99, 0, 99);
        printf("\t [On Hit] Rupture Count +3 on enemy (%d)", defender->Rupture[1]);
      }

    }

// --------------------------------------------------------------------------------------------------------------



    // -------------------------------- The One Who Grips Faust --------------------------------

  // The One Who Grips Faust - SKill 1 Inflict
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[0] && !Evaded) {

    if (i == remainingCoins - 2) {
      attacker->skills[2].active += 1; // Nail
      printf("\t [On Hit] Nail +1 on enemy (%d)", attacker->skills[2].active);
          attacker->skills[6].active += 1; // Bleed Count
       printf("\t [On Hit] Fanatic +1 Next turn");
    }

    if (i == remainingCoins - 1) {
        inflictStatus(defender->Bleed, 2, 0, 0, 99, 0, 99);
       printf("\t [On Hit] Bleed Stack +2 on enemy (%d)", defender->Bleed[0]);
    }

  }

  // The One Who Grips Faust - SKill 2 Inflict
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[1] && !Evaded) {

    if (i == remainingCoins - 3) {
      attacker->skills[2].active += 2; // Nail
      printf("\t [On Hit] Nail +2 on enemy (%d)", attacker->skills[2].active);
        inflictStatus(defender->Bleed, 0, 2, 0, 99, 0, 99);
       printf("\t [On Hit] Bleed Count +2 on enemy (%d)", defender->Bleed[1]);
    }

    if (i == remainingCoins - 2) {
      attacker->skills[2].active += 3; // Nail
      printf("\t [On Hit] Nail +3 on enemy (%d)", attacker->skills[2].active);
        inflictStatus(defender->Bleed, 2, 0, 0, 99, 0, 99);
       printf("\t [On Hit] Bleed Stack +2 on enemy (%d)", defender->Bleed[0]);
    }

    if (i == remainingCoins - 1) {
      attacker->skills[7].active += 1; // Gaze Next turn
      printf("\t [On Hit] Inflict Gaze on enemy next turn");

      if (IsHeadHit) {
        defender->Paralyze[1] += 1;
        printf("\t [Head Hit] Paralyze +1 on enemy next turn (Fix the Power of 1 Coins to 0 for one turn)");
      }
    }

  }

  // The One Who Grips Faust - SKill 3 Inflict
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[2] && !Evaded) {

    if (i == remainingCoins - 3) {
      attacker->skills[2].active += 2; // Nail
      printf("\t [On Hit] Nail +2 on enemy (%d)", attacker->skills[2].active);
        inflictStatus(defender->Bleed, 0, 1, 0, 99, 0, 99);
       printf("\t [On Hit] Bleed Count +1 on enemy (%d)", defender->Bleed[1]);
    }

    if (i == remainingCoins - 2) {
      inflictStatus(defender->Bleed, 2, 0, 0, 99, 0, 99);
       printf("\t [On Hit] Bleed Stack +2 on enemy (%d)", defender->Bleed[0]);
      if (IsHeadHit) {
      attacker->skills[2].active += 2; // Nail
      printf("\t [Head Hit] Nail +2 on enemy (%d)", attacker->skills[2].active);
      }
    }

  }

  // The One Who Grips Faust - SKill 4 Inflict
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[3] && !Evaded) {

    if (i != remainingCoins - 1 && IsHeadHit) {
      attacker->skills[2].active += 1; // Nail
      printf("\t [Head Hit] Nail +1 on enemy (%d)", attacker->skills[2].active);
    } else if (i == remainingCoins - 1 && attacker->skills[2].active >= 5) {
      defender->Stagger += 2;
      printf("\t [On Hit] At 5+ Nail (%d), Stagger target for one turn and remove Nail", attacker->skills[2].active);
      attacker->skills[2].active = 0;
    }

    if (i != remainingCoins - 1) {
      inflictStatus(defender->Bleed, 0, 1, 0, 99, 0, 99);
       printf("\t [On Hit] Bleed Count +1 on enemy (%d)", defender->Bleed[1]);
    } else if (i == remainingCoins - 1) {
      inflictStatus(defender->Bleed, 3, 0, 0, 99, 0, 99);
       printf("\t [On Hit] Bleed Stack +3 on enemy (%d)", defender->Bleed[0]);
      }

  }

  // The One Who Grips Faust - SKill 5 Inflict
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[4] && !Evaded) {

    if (i == remainingCoins - 1) {
      defender->ProtectionDown[1] += 50;
      printf("\t [On Hit] Target takes +50%% damage next turn");
    }

  }

  // ---------------------------------------------------------------------------

    // -------------------------------- Gregor:Firefist --------------------------------

    // Gregor:Firefist - S1 Burn
    if (isId(attacker->ID, "Gregor:Firefist") == 0 &&
        atk == &attacker->skills[0] && !Evaded) {

      int Stack = 0;
      int Count = 0;

      if (i == remainingCoins - 1) { // First Coin
        Count = 2;
      } else if (i == remainingCoins - 2) { // Second Coin
          Stack = 2;
        if (attacker->Passive <= 50) {
            Stack += 1;
        }
        }

      inflictStatus(defender->Burn, Stack, Count, 0, 99, 0, 99);

      if (Count > 0 && Stack > 0) {
        printf("\t [On Hit] Burn Stack +%d on enemy (%d) \t [On Hit] Burn Count +%d on enemy (%d)", Stack, defender->Burn[0], Count, defender->Burn[1]);
      } else if (Stack > 0) {
           printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, defender->Burn[0]);
           } else if (Count > 0) {
             printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, defender->Burn[1]);
           }

      sleep(1);
    }

    // Gregor:Firefist - S2 Burn
    if (isId(attacker->ID, "Gregor:Firefist") == 0 &&
        atk == &attacker->skills[1] && !Evaded) {

      int Stack = 0;
      int Count = 0;

      if (i == remainingCoins - 1) { // First Coin
        Count = 2;
        if (attacker->Passive <= 50) {
          Count += 1;
        }
      } else if (i == remainingCoins - 2) { // Second Coin
          Stack = 2;
        if (attacker->Passive <= 50) {
          Stack += 1;
        }
      }

      inflictStatus(defender->Burn, Stack, Count, 0, 99, 0, 99);

        if (Count > 0 && Stack > 0) {
          printf("\t [On Hit] Burn Stack +%d on enemy (%d) \t [On Hit] Burn Count +%d on enemy (%d)", Stack, defender->Burn[0], Count, defender->Burn[1]);
        } else if (Stack > 0) {
             printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, defender->Burn[0]);
             } else if (Count > 0) {
               printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, defender->Burn[1]);
             }


      sleep(1);
    }

    // Gregor:Firefist - S3 Burn
      if (isId(attacker->ID, "Gregor:Firefist") == 0 &&
          atk == &attacker->skills[2] && !Evaded) {

        int Stack = 0;
        int Count = 0;

        if (i == remainingCoins - 2 || i == remainingCoins - 3) { // First Coin
          Stack = 2;
          Count = 1;
          if (attacker->Passive <= 50) {
            Stack += 1;
          }
        }

        inflictStatus(defender->Burn, Stack, Count, 0, 99, 0, 99);

        if (Count > 0 && Stack > 0) {
          printf("\t [On Hit] Burn Stack +%d on enemy (%d) \t [On Hit] Burn Count +%d on enemy (%d)", Stack, defender->Burn[0], Count, defender->Burn[1]);
        } else if (Stack > 0) {
             printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, defender->Burn[0]);
             } else if (Count > 0) {
               printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, defender->Burn[1]);
             }


        sleep(1);
      }

    // Gregor:Firefist - S4 Burn
    if (isId(attacker->ID, "Gregor:Firefist") == 0 &&
        atk == &attacker->defenseSkill[0] && !Evaded) {

      int Stack = 0;
      int Count = 0;

      if (i == remainingCoins - 1) { // First Coin
        Stack = 1;
      }

        inflictStatus(defender->Burn, Stack, Count, 0, 99, 0, 99);

      if (Count > 0 && Stack > 0) {
        printf("\t [On Hit] Burn Stack +%d on enemy (%d) \t [On Hit] Burn Count +%d on enemy (%d)", Stack, defender->Burn[0], Count, defender->Burn[1]);
      } else if (Stack > 0) {
           printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, defender->Burn[0]);
           } else if (Count > 0) {
             printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, defender->Burn[1]);
           }

      sleep(1);
    }

    // ------------------------------------------------------------------------


    // ------------------------------------ Binah -----------------------------------

    // Binah - Fairy Skill 1
    if (isId(attacker->ID, "Binah") == 0 && (atk == &attacker->skills[0]) && !Evaded) {

      int inflictvalue = 1;

      if (attacker->Passive) inflictvalue = 2;

            attacker->skills[0].active += inflictvalue;

      printf("\t [On Hit] Fairy +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

      sleep(1);
    }

    // Binah - Fairy Skill 3
    if (isId(attacker->ID, "Binah") == 0 && (atk == &attacker->skills[2]) && i == 0 && !Evaded) {

      int inflictvalue = 3;

      if (attacker->Passive) inflictvalue = 5;

       attacker->skills[0].active += inflictvalue;

      printf("\t [On Hit] Fairy +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

    }

    // Binah - Fairy
    if (isId(attacker->ID, "Binah") == 0 && attacker->skills[0].active > 0 && !Evaded) {

      if (!attacker->Passive) {

        applyDamage(attacker, defender, attacker->skills[0].active, 0, NULL);

      totalDamage += attacker->skills[0].active;

      printf("\t [On Hit] Fairy deals %d damage", attacker->skills[0].active);

      } else {

        int Fairydamage = 0.5*((defender->MAX_HP/100)*attacker->skills[0].active);

        applyDamage(attacker, defender, Fairydamage, 1, NULL);

          totalDamage += Fairydamage;

          printf("\t [On Hit] Fairy deals %d true damage", Fairydamage);

          }

    }

    // -----------------------------------------------------------------------------------------


    // ----------------------------- Lobotomy E.G.O::Solemn Lament Yi Sang ------------------------


    // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly s1
    if (isId(attacker->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
            0 && (attacker->Passive > 0) && (atk == &attacker->skills[0]) && !Evaded) {

      int inflictvalue = 1;

      attacker->Passive -= inflictvalue;
      if (attacker->Passive < 0) attacker->Passive = 0;
        attacker->skills[0].active += inflictvalue;

      attacker->skills[3].active += inflictvalue; // Consumed Living & depart Count

      printf("\t [On Hit] Butterfly +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

    } // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly s2
    else if (isId(attacker->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
            0 && (attacker->Passive > 0) && (atk == &attacker->skills[1]) && !Evaded) {

       int inflictvalue;

      if (i == remainingCoins - 2) {
        inflictvalue = 5;
        if (attacker->Passive < 5) inflictvalue = attacker->Passive;
      } else {
        inflictvalue = 1;
      }

      attacker->Passive -= inflictvalue;
      if (attacker->Passive < 0) attacker->Passive = 0;
        attacker->skills[0].active += inflictvalue;

      attacker->skills[3].active += inflictvalue; // Consumed Living & depart Count

      printf("\t [On Hit] Butterfly +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

    } // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly s3
    else if (isId(attacker->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
            0 && (attacker->Passive >= 0) && (atk == &attacker->skills[2]) && !Evaded) {

       int inflictvalue;

      if (i == remainingCoins - 4) {
        inflictvalue = 1;
      } else if (i == remainingCoins - 3) {
        inflictvalue = 6;
        if (attacker->Passive < 6) inflictvalue = attacker->Passive;
      } else if (i == remainingCoins - 2) {
        inflictvalue = attacker->Passive;
      }


      if (i != remainingCoins - 1) {
      attacker->Passive -= inflictvalue;
        if (attacker->Passive < 0) attacker->Passive = 0;
        attacker->skills[0].active += inflictvalue;

        attacker->skills[3].active += inflictvalue; // Consumed Living & depart Count

      printf("\t [On Hit] Butterfly +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

      } 

      if (i == remainingCoins - 1) {

        applyDamage(attacker, defender, attacker->skills[0].active, 0, NULL);

        totalDamage += attacker->skills[0].active;
        printf("\t [On Hit] Deal more damage equal to Butterfly on enemy(%d)", attacker->skills[0].active);

    } 

    }



    // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly heal lose at 0-
    if (isId(attacker->ID,
                   "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
        (attacker->skills[0].active > 0) && attacker->Sanity < 0 && !Evaded) {

      int random = rand() % 100 + 1;

      if (random <= 70) {

        int min = attacker->skills[0].active/3;
        if (min < 1) min = 1;

        updateSanity(attacker, min);
        if (attacker->Sanity > 45) attacker->Sanity = 45;

        printf(" \tSanity +%d on self (%d)", min, attacker->Sanity);

      } else {

        if (defender->hasSanity == 1 && defender->sanityGainBase >= 0) { // Normal
        int min = attacker->skills[0].active/3;
        if (min < 1) min = 1;

          updateSanity(defender, -(min));
          if (defender->Sanity < -45) defender->Sanity = -45;

        printf(" \tSanity -%d on enemy (%d)", min, defender->Sanity);

        } else if (defender->hasSanity == 1 && defender->sanityGainBase < 0) { // Negative Sanity enemy
          int min = attacker->skills[0].active/3;
          if (min < 1) min = 1;

          updateSanity(defender, min);
          if (defender->Sanity > 45) defender->Sanity = 45;

          printf(" \tSanity +%d on enemy(%d)", min, defender->Sanity);

          } else { // No Sanity enemy

            int min = attacker->skills[0].active/3;
            if (min < 1) min = 1;

            printf(" \tDeal %d more damage on enemy", min);

          applyDamage(attacker, defender, min, 0, NULL);

            totalDamage += min;

        }
      }

    } // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly heal lose at 0+
      else if (isId(attacker->ID,
                     "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
          (attacker->skills[0].active > 0) && attacker->Sanity >= 0 && !Evaded) {

        int random = rand() % 100 + 1;

        if (random < 30) {

          int min = attacker->skills[0].active/3;
            if (min < 1) min = 1;

          updateSanity(attacker, min);
          if (attacker->Sanity > 45) attacker->Sanity = 45;

            printf(" \tSanity +%d on self (%d)", min, attacker->Sanity);

        } else {

          if (defender->hasSanity == 1 && defender->sanityGainBase >= 0) { // Normal
            int min = attacker->skills[0].active/3;
            if (min < 1) min = 1;

            updateSanity(defender, -(min));
             if (defender->Sanity < -45) defender->Sanity = -45;

            printf(" \tSanity -%d on enemy (%d)", min,  defender->Sanity);

            } else if (defender->hasSanity == 1 && defender->sanityGainBase < 0) { // Negative Sanity enemy
              int min = attacker->skills[0].active/3;
              if (min < 1) min = 1;

            updateSanity(defender, min);
            if (defender->Sanity > 45) defender->Sanity = 45;

              printf(" \tSanity +%d on enemy(%d)", min, defender->Sanity);

              } else { // No Sanity enemy

                int min = attacker->skills[0].active/3;
                if (min < 1) min = 1;

                printf(" \tDeal %d more damage on enemy", min);

            applyDamage(attacker, defender, min, 0, NULL);

            totalDamage += min;

            }

        }

        if (defender->hasSanity == 1 && defender->Sanity < 0 && defender->sanityGainBase >= 0) { // Normal

          int deal = attacker->skills[0].active/2 - (defender->Sanity/5);

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;

          }

              } else if (defender->hasSanity == 1 && defender->Sanity > 0 && defender->sanityGainBase < 0) { // Negative Sanity enemy

          int deal = attacker->skills[0].active/2 + (defender->Sanity/5);

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;

          }

              } else if (defender->hasSanity == 0) { // No Sanity enemy

          int deal = attacker->skills[0].active/2;

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;

          }

        }



      sleep(1);
    }

    //Lobotomy E.G.O::Solemn Lament Yi Sang while attack Reload
    if (isId(attacker->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
      0 && attacker->Passive <= 0 && i != remainingCoins - 1) {

      if (atk != &attacker->skills[2]) {

        i = remainingCoins;

            updateSanity(attacker, -(15));
          if (attacker->Sanity < -45) attacker->Sanity = -45;

         int ShieldGain = ((attacker->skills[0].active * 2) > 40 ? 40 : (attacker->skills[0].active * 2));
        int Shield = (ShieldGain/100.0f) * attacker->MAX_HP;

        attacker->Shield += Shield;

        printf("\n%s rans out of The Living & The Departed, stop the attack and use 'Reload' instead, Spends 15 Sanity(%d) to Gain 20 The Living & The Departed and gain Shield HP equal to (Butterfly on the target x 2)%% of Max HP. (%d%% - Max 40%%) (%d - Shield %.2f)", attacker->name, attacker->Sanity, ShieldGain, Shield, attacker->Shield + attacker->TempShield);

        attacker->Passive = 20;

        sleep(1);

      } else if (atk == &attacker->skills[2] && i != remainingCoins - 2) {

         i = remainingCoins;

        updateSanity(attacker, -(15));
        if (attacker->Sanity < -45) attacker->Sanity = -45;

        int ShieldGain = ((attacker->skills[0].active * 2) > 40 ? 40 : (attacker->skills[0].active * 2));
        int Shield = (ShieldGain/100.0f) * attacker->MAX_HP;

        attacker->Shield += Shield;

        printf("\n%s rans out of The Living & The Departed, stop the attack and use 'Reload' instead, Spends 15 Sanity(%d) to Gain 20 The Living & The Departed and gain Shield HP equal to (Butterfly on the target x 2)%% of Max HP. (%d%% - Max 40%%) (%d - Shield %.2f)", attacker->name, attacker->Sanity, ShieldGain, Shield, attacker->Shield + attacker->TempShield);

        attacker->Passive = 20;

        sleep(1);

      }


          sleep(1);


      }


    // -----------------------------------------------------

      // The Middle Nursefather - Matthias - The Book of Vengeance
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && !Evaded && attacker->skills[10].active == 0) {
        attacker->skills[10].active = 1; // Once per skill
        updateSanity(attacker, 7);
        printf("\t heal 7 Sanity (%d)", attacker->Sanity);
      }

    // ------------------- Muga Ryōshū -------------------

    // Muga Ryōshū on attack
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && attacker->skills[0].active/5 > 0) {
        applyDamage(attacker, defender, attacker->skills[0].active/5, 0, NULL); // deal sever the thread/5 damage
      printf("\t Enemy takes %d damage", attacker->skills[0].active/5);

      totalDamage += attacker->skills[0].active/5;
    }

    // Muga Ryōshū gains on attack
    if (isId(attacker->ID, "Muga Ryōshū") == 0) {
        attacker->Passive += 4; // Gain 1 Muga for every coin use
        if (attacker->Passive > 100) attacker->Passive = 100;
      printf("\t Muga [無我] +4 (%d)", attacker->Passive);

      if (!Evaded) {
      attacker->skills[0].active += attacker->skills[10].active; // Inflicted sever
      if (attacker->skills[0].active > 100) attacker->skills[0].active = 100;
      printf("\t Sever the Thread [切絲] +%d on enemy (%d)", attacker->skills[10].active, attacker->skills[0].active);
      }
    }

    // Muga Ryōshū gains on get hit
    if (isId(defender->ID, "Muga Ryōshū") == 0) {
          defender->skills[11].active += 1; // Gain 1 Muga for every coin use
        if (defender->skills[11].active > 2) defender->skills[11].active = 2;
    }

    // Muga Ryōshū on attack skill 1
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[0] && Unbreakable <= 0 && !Evaded) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 2) {

      inflictStatus(defender->Bleed, bleedstackinf + 1, 0, 0, 99, 0, 99);
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 1, defender->Bleed[0]);
        inflictStatus(defender->Bleed, 0, 2, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 1) {

        inflictStatus(defender->Bleed, bleedstackinf + 3, 0, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf +3, defender->Bleed[0]);
          inflictStatus(defender->Bleed, 0, 1, 0, 99, 0, 99);
            printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

        }

    }

    // Muga Ryōshū on attack skill 2
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[1] && Unbreakable <= 0 && !Evaded) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 2) {

      inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
        inflictStatus(defender->Bleed, 0, 1, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 1) {

        inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
          inflictStatus(defender->Bleed, 0, 2, 0, 99, 0, 99);
            printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

        }

    }

    // Muga Ryōshū on attack skill 3
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[2] && Unbreakable <= 0 && !Evaded) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 3) {

      inflictStatus(defender->Bleed, bleedstackinf + 1, 0, 0, 99, 0, 99);
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 1, defender->Bleed[0]);
        inflictStatus(defender->Bleed, 0, 1, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 2) {

      inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
        inflictStatus(defender->Bleed, 0, 1, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 1) {

        inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
          inflictStatus(defender->Bleed, 0, 3, 0, 99, 0, 99);
            printf("\t [On Hit without Cracking] Bleed Count +3 on target (%d)", defender->Bleed[1]);

        }

    }

    // Muga Ryōshū on attack skill 4
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[3] && Unbreakable <= 0 && !Evaded) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 3) {

      inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
        inflictStatus(defender->Bleed, 0, 1, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 2) {

      inflictStatus(defender->Bleed, bleedstackinf + 1, 0, 0, 99, 0, 99);
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 1, defender->Bleed[0]);
        inflictStatus(defender->Bleed, 0, 2, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 1) {

        inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
          inflictStatus(defender->Bleed, 0, 2, 0, 99, 0, 99);
            printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

        }

    }

    // Muga Ryōshū on attack skill 5
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[4] && Unbreakable <= 0 && !Evaded) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 5) {

        inflictStatus(defender->Bleed, 0, 2, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 4) {

        inflictStatus(defender->Bleed, 0, 2, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 3) {

      inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);

      }

      if (i == remainingCoins - 2) {

      inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);

      }

      if (i == remainingCoins - 1) {

        inflictStatus(defender->Bleed, bleedstackinf + 2, 0, 0, 99, 0, 99);
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
          inflictStatus(defender->Bleed, 0, 2, 0, 99, 0, 99);
            printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

        }

    }

    // ---------------------------------------------------------

    // Sukuna:King of Curse gains Binding vow
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[0] || atk == &attacker->skills[1] || (atk == &attacker->skills[2] && (i == remainingCoins - 1)) || (atk == &attacker->skills[6] && (i == remainingCoins - 4 || i == remainingCoins - 5)) || atk == &attacker->skills[7])) {

      int gainsvalue = finalDamage/4;

        attacker->skills[3].active += gainsvalue;

        printf(" \t 'Binding Vow - Open' +%d (%d)", gainsvalue, attacker->skills[3].active);

    }


    // ------------------------- King in Binds ------------------------------


    // King in Binds skill 1, 3 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[0] || atk == &attacker->skills[2]) && !Evaded) {

      inflictStatus(defender->Sinking, 2, 0, 0, 99, 0, 99);

        printf("\t [On Hit] Sinking Stack +2 on target (%d)",  defender->Sinking[0]);

    }

    // King in Binds skill 2 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[1]) && !Evaded) {

      if (i == remainingCoins - 3) {

          inflictStatus(defender->Sinking, 0, 2, 0, 99, 0, 99);

        printf("\t [On Hit] Sinking Count +2 on target (%d)", defender->Sinking[1]);

      } else {

            inflictStatus(defender->Sinking, 3, 0, 0, 99, 0, 99);

          printf("\t [On Hit] Sinking Stack +3 on target (%d)", defender->Sinking[0]);

        }

    }

    // King in Binds skill 4 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[3]) && !Evaded) {

      if (i == remainingCoins - 2) {

        inflictStatus(defender->Tremor, 0, 3, 0, 99, 0, 99);

        printf("\t [On Hit] Tremor Count +3 on target (%d)", defender->Tremor[1]);

      } else if (i == remainingCoins - 1) {

        TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

      }

    }

    // King in Binds skill 6 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[5]) && !Evaded) {

          inflictStatus(defender->Tremor, defender->Sinking[0], 0, 0, 99, 0, 99);

        printf("\t [On Hit] Inflict Tremor Stack equal to Sinking Stack on target (%d)", defender->Tremor[0]);

       TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

    }

    // King in Binds Moment of Audience
    if (isId(attacker->ID, "King in Binds") == 0 && attacker->skills[6].active > 0 && !Evaded) {

      // For make it work 'next turn'
        if (attacker->skills[6].active == 1) {
          // จังหวะที่ 1: เพิ่งใช้ท่า Present Thyself มาหมาดๆ (เพิ่งเซ็ตเป็น 1 ใน getEffectiveSkill)
          // เราจะเปลี่ยนเป็น 2 เพื่อบอกว่า "ตาหน้าพร้อมทำงานนะ"
          attacker->skills[6].active = 2;

        } else if (attacker->skills[6].active >= 2) {
          // จังหวะที่ 2: ค่าเป็น 2 แล้ว (แปลว่าชาร์จมาจากตาที่แล้ว) -> ปล่อยเอฟเฟกต์!

          // --- ใส่ Code เอฟเฟกต์ Tremor Burst / Fragile 10% ตรงนี้ ---
          // (พวก printf และการลด HP/Shield ของ target)

          // จังหวะที่ 3: จัดการคิว (Stacking)
          if (attacker->skills[6].Unbreakable > 0) {
              // ถ้ามีการใช้ท่าซ้ำซ้อนจน Unbreakable เป็น 1
              attacker->skills[6].Unbreakable--; 
              attacker->skills[6].active = 2; // ให้ชาร์จพร้อมทำงาน
          } else {
              attacker->skills[6].active = 0; // จบงาน ไม่มีคิวต่อ
          }

          TremorBurst(attacker, defender, defender->MAX_HP/4, &totalDamage, 1);

        inflictStatus(defender->Tremor, 5, 5, 0, 99, 0, 99);

        printf(" \tTremor Stack +5 (%d) and Tremor Count +5 (%d) on target", defender->Tremor[0], defender->Tremor[1]);

      defender->ProtectionDown[1] += 10;

      printf("\n\n%s take +10%% damage next turn\n", defender->name);

    }

    }

    // King in Binds skill 5 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[4]) && !Evaded) {

      if (defender->hasSanity == 1) { // Normal
        int deal = defender->Sinking[0] + defender->Sinking[1];

        if (defender->Sanity - deal <= -45) {

          int finalSP = defender->Sanity - deal;
          int excessdamage = (-45 - finalSP); // เช่น -45 - (-50) = 5

          updateSanity(defender, -(deal));

          applyDamage(attacker, defender, excessdamage, 0, NULL);

            totalDamage += excessdamage;

           printf("\nCause 'Sinking Deluge', target loses (Sinking Stack + Count) Sanity (%d) (Sanity %d). Target at -45 Sanity, deal damage equal to excess that cause from 'Sinking Deluge' (%d)", deal, defender->Sanity, excessdamage);

        } else {

          updateSanity(defender, -(deal));

         printf("\nCause 'Sinking Deluge', target loses (Sinking Stack + Count) Sanity (%d) (Sanity %d); then lose all Sinking", deal, defender->Sanity);

          attacker->skills[2].active = 0;
          attacker->skills[1].active = 0;

        }

        } else { // No Sanity enemy

        int deal = attacker->skills[1].active + attacker->skills[2].active;

        applyDamage(attacker, defender, deal, 0, NULL);

            totalDamage += deal;

        printf("\nCause 'Sinking Deluge', target takes (Sinking Stack + Count) damage (%d); then lose all Sinking", defender->Sanity);

        attacker->skills[2].active = 0;
        attacker->skills[1].active = 0;

        }


    }



    // ----------------------------------------------------------

    //Dawn Office Fixer Sinclair S1 EGO FORM REUSE
    if (isId(attacker->ID, "Dawn Office Fixer Sinclair") ==
      0 && attacker->skills[3].active && i == 0 && atk == &attacker->skills[0]) {

        remainingCoins++;

       printf("\n%s in a Volatile E.G.O State, Reuse Coin (once per Skill)", attacker->name);

          sleep(1);

      }

    // Fixer grade 9? S6 Reuse
    if (isId(attacker->ID, "Fixer grade 9?") ==
      0 && attacker->Passive >= 10 && i == 0 && atk == &attacker->skills[5] && !ClashLostAttack) {

        remainingCoins++;

      attacker->Passive -= 5;
       if (attacker->Passive < 0) attacker->Passive = 0;

       printf("\n%s at 10+ Black Silence and without Clash Lost, consumes 5 Black Silence (%d) to Reuse Coin (once per Skill)", attacker->name, attacker->Passive);

          sleep(1);

      }

    // Fixer grade 9? S3 Reuse
    if (isId(attacker->ID, "Fixer grade 9?") == 0 && attacker->Passive >= 15 && atk == &attacker->skills[2] && i == remainingCoins - 1 && remainingCoins <= atk->Coins && !ClashLostAttack) {

      attacker->skills[2].Coins = remainingCoins;

      remainingCoins++;

       attacker->Passive -= 5;
       if (attacker->Passive < 0) attacker->Passive = 0;

       printf("\n%s at 15+ Black Silence and without Clash Lost, consumes 5 Black Silence (%d) to Reuse Coin (once per Skill)", attacker->name, attacker->Passive);

          sleep(1);

    } else if (isId(attacker->ID, "Fixer grade 9?") == 0 && atk == &attacker->skills[3]) {
      attacker->skills[2].Coins = 2;
    }

    // -------------------- Heishou Pack - You Branch Adept Heathcliff ---------------------

    // Heishou Pack - You Branch Adept Heathcliff - Skill 2 reuse
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (attacker->skills[2].active > 0 || attacker->HP < attacker->MAX_HP * 0.5) && atk == &attacker->skills[1] && i == remainingCoins - 1 && remainingCoins <= attacker->skills[1].Coins && !Evaded) {

      attacker->skills[1].Coins = remainingCoins;

      remainingCoins++;

       printf("\n%s has Bloodflame [血炎] or less than 50%% HP, Reuse Coin (Once per Skill)", attacker->name);

          sleep(1);

    } else if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && atk == &attacker->skills[1]) {
      attacker->skills[1].Coins = 3;
    }

    // Heishou Pack - You Branch Adept Heathcliff - Skill 3-2 reuse
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (attacker->Burn[0] >= 20 || attacker->HP <= attacker->MAX_HP * 0.5) && atk == &attacker->skills[3] && i == remainingCoins - 1 && remainingCoins <= attacker->skills[3].Coins && !Evaded) {

      attacker->skills[3].Coins = remainingCoins;

      remainingCoins++;

       printf("\n%s has 20+ Burn or less than 50%% HP, Reuse Coin (Once per Skill)", attacker->name);

          sleep(1);

    } else if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && atk == &attacker->skills[3]) {
      attacker->skills[3].Coins = 4;
    }

    // -------------------------------------------------------------

    // Heathcliff:Wild Hunt – Skill 4 addition damage
    if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
       (atk == &attacker->skills[3]) && i == remainingCoins - 1 && !Evaded) {

      int damage = abs(attacker->Sanity - defender->Sanity);
      if (damage > 30) damage = 30;

      applyDamage(attacker, defender, damage, 0, NULL);

      totalDamage += damage;

      printf("\n%s deals more damage base on Sanity difference (%d - Max 30)",
         attacker->name, damage);

        sleep(1);
    }


    // Sancho:The Second Kindred of Don Quixote - Heal mechnics
     if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && ClashLostAttack == 0 && !Evaded) {

      int healvalue = 40;

       int missingHP = (int)(((float)(attacker->MAX_HP - attacker->HP) / attacker->MAX_HP) * 100);
        if (missingHP > 20) missingHP = 20;

       healvalue += missingHP;

    if (isId(attacker->ID,
                   "Sancho:The Second Kindred of Don Quixote") == 0 &&
        (atk == &attacker->skills[10] || atk == &attacker->skills[11] || atk == &attacker->skills[12] || atk == &attacker->skills[13])) {

      healvalue += 100; // +100% heal

    }

       int finalheal = (finalDamage * (healvalue / 100.0f));

      printf(" HP +%d", (int)(attacker->HP + finalheal) > attacker->MAX_HP
                              ? (int)(attacker->MAX_HP - attacker->HP)
                              : finalheal);

      attacker->HP += (int)(attacker->HP + finalheal) > attacker->MAX_HP
                          ? (int)(attacker->MAX_HP - attacker->HP)
                          : finalheal;

     } // Sancho:The Second Kindred of Don Quixote - Heal mechnics for certain skil when clash lost
        else if (isId(attacker->ID,"Sancho:The Second Kindred of Don Quixote") == 0 &&
           (atk == &attacker->skills[10] || atk == &attacker->skills[11] || atk == &attacker->skills[12] || atk == &attacker->skills[13]) && ClashLostAttack == 1 && !Evaded) {

         printf(" HP +%d", (int)(attacker->HP + finalDamage) > attacker->MAX_HP
                                 ? (int)(attacker->MAX_HP - attacker->HP)
                                 : finalDamage);

         attacker->HP += (int)(attacker->HP + finalDamage) > attacker->MAX_HP
                             ? (int)(attacker->MAX_HP - attacker->HP)
                             : finalDamage;

        }

    // Don Quixote:The Manager of La Manchaland - Heal mechnics
    if ((isId(attacker->ID,
                    "Don Quixote:The Manager of La Manchaland") == 0 &&
         (atk == &attacker->skills[0] || atk == &attacker->skills[1] ||
          atk == &attacker->skills[4])) && !Evaded) {
      // Max 10 with heal 30% of the HP damage dealt
      printf(" HP +%d",
        (int)(attacker->HP + (finalDamage * 0.3 > 10 ? 10 : finalDamage * 0.3) >
              attacker->MAX_HP)
                 ? (int)(attacker->MAX_HP - attacker->HP)
                 : (int)(finalDamage * 0.3 > 10 ? 10 : finalDamage * 0.3));

      attacker->HP +=
        (int)(attacker->HP + (finalDamage * 0.3 > 10 ? 10 : finalDamage * 0.3) >
           attacker->MAX_HP)
              ? (int)(attacker->MAX_HP - attacker->HP)
              : (int)(finalDamage * 0.3 > 10 ? 10 : finalDamage * 0.3);

    } else if ((isId(attacker->ID,
                           "Don Quixote:The Manager of La Manchaland") == 0 &&
                (atk == &attacker->skills[2])) && !Evaded) {
      // Max 10 with heal 50% of the HP damage dealt
      printf(" HP +%d",
        (int)(attacker->HP + (finalDamage * 0.5 > 10 ? 10 : finalDamage * 0.5) >
              attacker->MAX_HP)
                 ? (int)(attacker->MAX_HP - attacker->HP)
                 : (int)(finalDamage * 0.5 > 10 ? 10 : finalDamage * 0.5));

      attacker->HP +=
        (int)(attacker->HP + (finalDamage * 0.5 > 10 ? 10 : finalDamage * 0.5) >
           attacker->MAX_HP)
              ? (int)(attacker->MAX_HP - attacker->HP)
              : (int)(finalDamage * 0.5 > 10 ? 10 : finalDamage * 0.5);

    } else if (isId(attacker->ID,
                          "Don Quixote:The Manager of La Manchaland") == 0 &&
               (atk == &attacker->skills[5] || atk == &attacker->skills[3]) && !Evaded) {
      // Max 20 with heal 50% of the HP damage dealt
      printf(" HP +%d",
             (int)(attacker->HP + (finalDamage * 0.5 > 20 ? 20 : finalDamage * 0.5) >
              attacker->MAX_HP)
                 ? (int)(attacker->MAX_HP - attacker->HP)
                 : (int)(finalDamage * 0.5 > 20 ? 20 : finalDamage * 0.5));

      attacker->HP +=
        (int)(attacker->HP + (finalDamage * 0.5 > 20 ? 20 : finalDamage * 0.5) >
           attacker->MAX_HP)
              ? (int)(attacker->MAX_HP - attacker->HP)
              : (int)(finalDamage * 0.5 > 20 ? 20 : finalDamage * 0.5);
    }


    // Jia Qiu enemy heal
    if (isId(attacker->ID, "Jia Qiu") == 0 && (attacker->skills[15].active > 0) && defender->HP <= 0) {

      if (isId(defender->ID, "Hong lu:The Lord of Hongyuan") == 0) {

        attacker->skills[15].active -= 1;

      defender->HP = defender->MAX_HP;
        defender->FinalPowerUp[1] += 1;
        printf("\n%s's Uncompromising Imposition activated! Heal up to max HP and gain 1 Final Power, lose 1 stack(%d)", defender->name, attacker->skills[15].active);
        sleep(1);
    } else {

        attacker->skills[15].active -= 1;

        defender->HP = defender->MAX_HP;
        printf("\n%s's Dialogues activated! Heal up to max HP, lose 1 stack(%d)", defender->name, attacker->skills[15].active);
        sleep(1);
    }

      }

      // Jia Qiu enemy heal
      if (isId(defender->ID, "Jia Qiu") == 0 && (defender->skills[15].active > 0) && attacker->HP <= 0) {

        if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0) {

            defender->skills[15].active -= 1;

          attacker->HP = attacker->MAX_HP;
            attacker->FinalPowerUp[1] += 1;
          printf("\n%s's Uncompromising Imposition activated! Heal up to max HP and gain 1 Final Power, lose 1 stack(%d)", attacker->name, attacker->skills[15].active);
          sleep(1);
      } else {

            defender->skills[15].active -= 1;

            attacker->HP = attacker->MAX_HP;
          printf("\n%s's Dialogues activated! Heal up to max HP, lose 1 stack(%d)", attacker->name, attacker->skills[15].active);
          sleep(1);
      }


    }

  // The One Who Grips Faust Skill 2 after Last coins
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[1] && i == remainingCoins - 1) {

    printf("\n\n%s: \"Heeheh.\"\n", attacker->name);

    sleep(1);

  }

    // The One Who Grips Faust Skill 5 after Last coins
      if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[4]) {

        if (i == remainingCoins - 1) {

        printf("\n\n%s: \"... Is mine now!\"\n", attacker->name);

        sleep(1);

        }

      }

      // ------------- The Middle Nursefather - Matthias -------------

      // The Middle Nursefather - Matthias - Using '... Complete and Total Extermination [Lævateinn]' or certain slash attack
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[3] || atk == &attacker->skills[4] || atk == &attacker->skills[9] || atk == &attacker->skills[5] || atk == &attacker->skills[6])) {

        if (i == remainingCoins - 1) {

          printf("\n\n%s: \"Haaahh!!!\"\n", attacker->name);

            }

      }

      // The Middle Nursefather - Matthias - Skill 1 2 / 10
      if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[0] || atk == &attacker->skills[2] || atk == &attacker->skills[9]) && !Evaded) {

        if (i == remainingCoins - 1) {
          if (Unbreakable <= 0 && attacker->skills[2].active > 0) {
            int gain = 2;
            if (atk == &attacker->skills[9]) gain += 3;
            defender->Paralyze[0] += gain;
            printf("\n[On Hit without Cracking] If this unit has 'The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]', inflict %d Paralyze (Fix the Power of %d Coins to 0 for one turn)", gain, gain);
          }
        }

      }

       // The Middle Nursefather - Matthias - Skill 8
        if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && (atk == &attacker->skills[7]) && !Evaded) {

          if (i == remainingCoins - 2) {
            if (Unbreakable <= 0 && attacker->skills[2].active > 0) {
              int gain = 2;
              defender->Paralyze[0] += gain;
              printf("\n[On Hit without Cracking] If this unit has 'The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]', inflict %d Paralyze (Fix the Power of %d Coins to 0 for one turn)", gain, gain);
            }
          }

        }

      // ------------------------------------------------------------------------------

      // The House of Spiders: The Ring Nursefather Hong Lu Skill Tanut Skill 3-2
      if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[4])) {

        if (i == remainingCoins - 1) {
          printf("\n\n%s: \"The gallery's closing hour is here\"\n", attacker->name);

          sleep(2);

          printf("\n%s: \"Everyone. Good-bye!\"\n", attacker->name);

          sleep(2);

          attacker->Passive = -1; // Reset
          }

      }

      if (isId(defender->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && Evaded && defender->defenseSkill[0].active == 0 && defender->defenseSkill[1].active > 0) {

        defender->defenseSkill[0].active = 1;

        int random = rand() % 5 + 1;

        if (random == 1) {

        printf("\n\n%s: \"What, having a hard time landing a hit?\"\n", defender->name);

        } else if (random == 2) {

            printf("\n\n%s: \"Told ya already. You're too damn slow!\"\n", defender->name);

            } else if (random == 3) {

        printf("\n\n%s: \"Saw that shit coming a mile away~\"\n", defender->name);


        } else if (random == 4) {

        printf("\n\n%s: \"I'm readin' ya like an open book.\"\n", defender->name);


        } else {

          printf("\n\n%s: \"You're slow as shit.\"\n", defender->name);


          }

      } else if (isId(defender->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && !Evaded && !nodefense && defender->defenseSkill[0].active <= 1 && defender->defenseSkill[1].active > 0) {

        defender->defenseSkill[0].active = 2;

        int random = rand() % 3 + 1;

        if (random == 1) {
        
        printf("\n\n%s: \"Shit!\"\n", defender->name);


        } else if (random == 2) {

            printf("\n\n%s: \"What the-\"\n", defender->name);


            } else {

          printf("\n\n%s: \"You've gotta be fucking kidding me...\"\n", defender->name);


          }
        
      }

      // The House of Spiders: The Thumb Nursefather Rodion - Tanut Skill 2-2
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") ==
                     0 &&
                 (atk == &attacker->skills[3]) && i == 2) {

        printf("\n\n%s: \"Textbook. Am I asking for the impossible?\"\n", attacker->name);

        sleep(1);
      }

      // The House of Spiders: The Thumb Nursefather Rodion - Tanut Skill 3-1
      if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") ==
                     0 &&
                 (atk == &attacker->skills[4]) && i == 3) {

        printf("\n\n%s: \"I'll make mincemeat out of you all!\"\n", attacker->name);

        sleep(1);
      }

    // The Middle Little Brother Sinclair Skill 3 after Last coins
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && (atk == &attacker->skills[2] || (atk == &attacker->skills[3] && attacker->Passive >= 4)) && attacker->Passive > 0 && i == remainingCoins - 1) {

      printf("\n\n%s: \"Does that sting?\"\n", attacker->name);

      sleep(1);

    }

    // The House of Spiders: The Index Nursefather Yi Sang Skill 4 after Last coins
    if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && (atk == &attacker->skills[3]) && i == remainingCoins - 1 && attacker->skills[3].active == 2) {

      printf("\n\n%s: \"*beep* Hah... the waves are rolling in.\"\n", attacker->name);

      sleep(1);

    } else if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && (atk == &attacker->skills[3]) && i == remainingCoins - 1) {

        printf("\n\n%s: \"Replication complete.\"\n", attacker->name);

        sleep(1);

      }


    if (Unbreakable > 0) Unbreakable--; // ← Character's Cracking Coins

    sleep(1);
  }

  }


    printf("\n(%d bonus) Damage Multiplier: %.2f", bonus,
       atk->DmgMutiplier);

  printf("\n%s total damage dealt (Opponent's defense: %d): %d\n",
         attacker->name, defTempDefense, totalDamage);





  //---------------- After Attack Buff ----------------------------

    // ----------------- The House of Spiders: The Thumb Nursefather Rodion -----------------

    // Accelerating Future
    if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && attacker->skills[4].active > 0) {

       attacker->defenseSkill[5].active = 0; // Reset Spend Count

    attacker->skills[4].active = 0; // Accelerating Future
      
      attacker->skills[17].active = 0; // at max Accelerating Future flag

    printf("\n%s's 'Accelerating Future' expires\n", attacker->name);

    sleep(1);

    } 
    
    if (isId(defender->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && defender->skills[4].active > 0) {

       attacker->defenseSkill[5].active = 0; // Reset Spend Count

        defender->skills[4].active = 0; // Accelerating Future

          defender->skills[17].active = 0; // at max Accelerating Future flag

      printf("\n%s's 'Accelerating Future' expires\n", defender->name);

      sleep(1);

      }

 // The House of Spiders: The Thumb Nursefather Rodion - Skill 3-2
  if (isId(attacker->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && atk == &attacker->skills[5] && defender->HP <= 0) {

       attacker->skills[10].active = 10;

       printf("\n%s uses 'Reload' (Lose all currently owned Ammo, and reload back to full)\n", attacker->name);

        sleep(1);

    printf("\n%s: \"Who's next? C'mon... We all know how this ends anyway.\"\n", attacker->name);

    sleep(1);

  }

      // -------------------------------------------------------------------------------------

    // ----------------- The House of Spiders: The Ring Nursefather Hong Lu -----------------
    if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0) {

        attacker->skills[7].active = 0; // Once per Skill Reset

      if (atk == &attacker->skills[4]) {
        attacker->skills[8].active = 1; // Somatic Frisson-inspiring Melody
        attacker->MinSpeed++;
        attacker->MaxSpeed++;
        updateSanity(attacker, 10);
        printf("\n%s heals 10 Sanity (%d) and apply 'Somatic Frisson-inspiring Melody' on self\n", attacker->name, attacker->Sanity);
      }

        // เก็บสถานะว่ามีการทำดาเมจ HP/Shield หรือยัง (Passive 1)
        if (totalDamage > 0 && !Evaded) {
          inflictStatus(attacker->Charge, 0, 5, 0, 99, 0, 20);
          printf("\n%s gains +5 Corpus Ingredient Count (%d)\n", attacker->name, attacker->Charge[1]);

            // 5. A Maestro's Critique (ฮีล 4 SP เมื่อลงสถานะลบสำเร็จ)
            // (เช็คว่าสกิล Anatomize หรือ Gather Ingredient ทำงาน)
            if (!(atk == &attacker->skills[1] && atk->Coins <= 1) && attacker->skills[2].active < 3) {
                attacker->skills[2].active++;

                if (attacker->Sanity >= 45) {
                    attacker->DamageUp[1] += 10;
                    printf("\n%s gains +10%% Damage Up next turn\n", attacker->name);
                } else {
                  updateSanity(attacker, 4);
                  printf("\n%s heals 4 Sanity (%d)\n", attacker->name, attacker->Sanity);
                }
            }
        }

        // Kill Bonus
        if (defender->HP <= 0) {
          inflictStatus(attacker->Charge, 0, 3, 0, 99, 0, 20);
            printf("\n%s gains +3 Corpus Ingredient Count (%d)\n", attacker->name, attacker->Charge[1]);
        }
    }

    // The House of Spiders: The Ring Nursefather Hong Lu - Inflict on attack Skill 2
    if (isId(attacker->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (atk == &attacker->skills[1]) && !Evaded) {

      int count = countNegativeEffectTypes(defender);

      if (count >= 3) {

      printf("\n%s has 3+ types of negative effects (%d), use '%s' Skill against the target (Once per turn)\n", defender->name, count, attacker->skills[2].name);

      sleep(1);

        // โจมตีซ้ำ
        // แก้ไขจุดนี้: ส่ง SkillEffective ที่เป็น pointer เข้าไปเลย
        attackPhase(attacker, &attacker->skills[2], atkTempOffense, atkTempDefense, 
                    defender, defSkill, defTempOffense, defTempDefense, 
          attacker->skills[2].Coins, 0, clashCount);

      }

    }

    // -------------------------------------------------------------------------------------

    // Lei heng Count Hit
    if (isId(defender->ID, "Lei heng") == 0 && !Evaded) {

        // Lei heng damage 
        defender->skills[7].active += totalDamage;

    }

    // The Middle Nursefather - Matthias Count Hit
    if (isId(defender->ID, "The Middle Nursefather - Matthias") == 0 && !Evaded) {
        defender->skills[4].active++; // นับจำนวน Hit

        // สะสมดาเมจไว้ใช้ Mad Rampage เทิร์นหน้า
        defender->skills[11].active += totalDamage;

    }

    // The Middle Nursefather - Matthias Unsealed
    if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0) {
      attacker->skills[10].active = 0; // Once per skill Reset

      if (atk == &attacker->skills[4]) {
          attacker->skills[9].active = 1;

          // Buff Skill
          attacker->skills[0].BasePower += 1;
          attacker->skills[0].Offense += 1;

          attacker->skills[1].BasePower += 1;
          attacker->skills[1].Offense += 1;

          attacker->skills[2].BasePower += 1;
          attacker->skills[2].Offense += 1;

          attacker->skills[3].name = "Are Ya Feelin' the Heat?!";
          attacker->skills[3].Offense += 1;

          attacker->defenseSkill[0].Offense += 1;

        printf("\n%s's 'Sealed Sword' becomes 'First Seal Removed' (Unlocks access to certain Skills based on the state of the sword.)\n", attacker->name);

        sleep(1);
      } else if (atk == &attacker->skills[5]) {
        attacker->skills[9].active = 2;

        attacker->skills[0].name = "Stomping!";
        attacker->skills[0].BasePower += 1;
        attacker->skills[0].Offense += 1;

        attacker->skills[1].name = "Punting!";
        attacker->skills[1].BasePower += 1;
        attacker->skills[1].Offense += 1;

        attacker->skills[2].name = "Stamp of Complete and Total Extermination!";
        attacker->skills[2].Offense += 1;

        attacker->skills[3].name = "Time to Melt Away and Die";
        attacker->skills[3].Offense += 1;

        attacker->defenseSkill[0].name = "Rule Violation!";
        attacker->defenseSkill[0].Offense += 1;

          printf("\n%s's 'First Seal Removed' becomes 'Second Seal Removed' (Unlocks access to certain Skills based on the state of the sword.)\n", attacker->name);

        sleep(1);
        } else if (atk == &attacker->skills[6]) {
        attacker->skills[9].active = 3;

        for (int i = 0; i <= 3; i++) {
          attacker->skills[i].Copies = -1; // Delete
          attacker->skills[i].Coins = 0; // ทำให้สกิลที่ค้างอยู่ใน Dashboard ใช้งานไม่ได้
        }

        attacker->skills[7].Copies = 3; // Gain
        attacker->skills[8].Copies = 2; // Gain
        attacker->skills[9].Copies = 1; // Gain

        attacker->defenseSkill[0].name = "Rule Violation!!";
        attacker->defenseSkill[0].BasePower -= 1;
        attacker->defenseSkill[0].Offense += 1;
        attacker->defenseSkill[0].Copies = 2;

        printf("\n%s's 'Second Seal Removed' becomes 'Lævateinn' (Unlocks access to certain Skills based on the state of the sword. Turn Start: Inflict +5 Burn Stack and +2 Burn Count against all units)\n", attacker->name);

        sleep(1);

        printf("\n%s gains 'Ridiculous Grit' next turn\n", attacker->name);

        for (int i = 0; i <= 3; i++) {
          attacker->skills[i].Copies = -1; // Delete
        }

        sleep(1);
      }
    }

    // The Middle Nursefather - Matthias Lose when use skill 3
    if (isId(attacker->ID, "The Middle Nursefather - Matthias") == 0 && atk == &attacker->skills[2]) {

        attacker->skills[0].active = 0;
        attacker->skills[1].active++; // gain Kiddo
      if (attacker->skills[1].active > 10) attacker->skills[1].active = 10;

          printf("\n%s Consume all 'The Middle - Grudge' on self and gain 1 'Check This Out, Kiddo!' (%d)\n", attacker->name, attacker->skills[1].active);

        sleep(1);
      }

    // The Middle Nursefather - Matthias Lose when hit
    if (isId(defender->ID, "The Middle Nursefather - Matthias") == 0) {
      if (defender->skills[4].active >= 2) {
          defender->skills[4].active = 0; // Reset

          // Resentment Logic
          if (defender->skills[1].active > 0) defender->skills[1].active -= 1; // Lose Kiddo
        if (defender->skills[1].active < 0) defender->skills[1].active = 0;
          defender->skills[0].active++; // Gain 1 Grudge
          if (defender->skills[0].active > 10) defender->skills[0].active = 10;

          printf("\n%s gains 1 'The Middle - Grudge' (%d) and loses 1 'Check This Out, Kiddo!' (%d)\n", defender->name, defender->skills[0].active, defender->skills[1].active);

        sleep(1);
      }
    }

    // Muga Ryōshū - Execution
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && attacker->skills[0].active >= 100 && atk != &attacker->skills[5]) {

          printf("\n\x1b[1;31m%s: \"However much time slips by, I hope it feels like only a fleeting moment to her.\"\x1b[0m\n", attacker->name);

          sleep(1);

      // โจมตีซ้ำด้วยสกิลที่ 5
      attackPhase(attacker, &attacker->skills[5], atkTempOffense, atkTempDefense, 
                  defender, defSkill, defTempOffense, defTempDefense, 
                  attacker->skills[5].Coins, 0, clashCount);

        }

    // Muga Ryōshū - Execution
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[5]) {

          defender->HP = 0;

      defender->Passive = -99;

      for (int i = 0; i < MAX_SKILLS; i++) {
          defender->skills[i].active = -99;
      }

      defender->Stagger = 999;            // ทำให้ติด Stagger ค้างไว้ (กันเหนียว)

        }

    // The One Who Grips Faust - Bliss of Execution
    if (isId(attacker->ID, "The One Who Grips Faust") == 0) {
        // 8. Bliss of Execution (Once per Turn)
        if (attacker->Passive > 0 && attacker->skills[2].active >= 6 && 
            attacker->skills[5].active == 0 && atk != &attacker->skills[4]) {

            attacker->skills[5].active = 1; // ปักป้ายว่าใช้ไปแล้ว

            printf("\n%s has 'Fanatic' and enemy has 6+ Nail (%d), use 'I Shall Claim Your Life!' (Once per turn)\n", attacker->name, attacker->skills[2].active);

            sleep(1);

            // โจมตีซ้ำด้วยสกิลที่ 4 (I Shall Claim Your Life!)
            attackPhase(attacker, &attacker->skills[4], atkTempOffense, atkTempDefense, 
                        defender, defSkill, defTempOffense, defTempDefense, 
                        attacker->skills[4].Coins, 0, clashCount);
        }

        // Gaze Reward: ฟื้น Sanity ให้ Faust หลังจบการตี
        if (attacker->skills[4].active > 0 && !Evaded) {
            updateSanity(attacker, 5);
          attacker->skills[4].active = 0;
            printf("\n%s heals 5 Sanity from Gaze (%d); then loses Gaze\n", attacker->name, attacker->Sanity);
        }
    }

    // Sukuna:King of Curse - Lost of Cursed Energy
    if (isId(attacker->ID, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[7])) {

      printf("\n%s gains 'Lost of Cursed Energy'\n", attacker->name);

      attacker->FinalPowerDown[1] += 5;
      attacker->DamageDown[1] += 50;
      attacker->ProtectionDown[0] += 30;

      sleep(1);
    }

// ------------------------ The House of Spiders: The Index Nursefather Yi Sang ---------------------------

    if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {

      int uncrackedUnbreakable = atk->Unbreakable - initialCrackedCount;
          if (uncrackedUnbreakable < 0) uncrackedUnbreakable = 0;

      // --- ส่วนคำนวณขีดจำกัด (Caps) คงไว้ตามเดิมเพื่อสมดุลเกม ---
      int maxGainThisTurn = attacker->Passive + 2;
      int hermesHardCap = (attacker->Passive < 2) ? 8 : 9;

      int quotaLeftThisTurn = maxGainThisTurn - attacker->skills[13].active;
      int spaceLeftInStack = hermesHardCap - attacker->skills[1].active;

      if (quotaLeftThisTurn < 0) quotaLeftThisTurn = 0;
      if (spaceLeftInStack < 0) spaceLeftInStack = 0;

      // หาค่าที่จะได้รับจริง (ไม่เกินโควต้า และไม่เกินช่องว่างในคลัง)
      int actualGain = uncrackedUnbreakable; 
      if (actualGain > quotaLeftThisTurn) actualGain = quotaLeftThisTurn;
      if (actualGain > spaceLeftInStack) actualGain = spaceLeftInStack;

      if (actualGain > 0 && !Evaded) {
          attacker->skills[1].active += actualGain;
          attacker->skills[13].active += actualGain;

        // For prescript III Check
        if (attacker->Passive == 2) {
          attacker->skills[5].active = 1;
          }

          printf("\n%s gains +%d 'Procuration [Hermes]' (%d)\n", 
                 attacker->name, actualGain, 
                 attacker->skills[1].active);
          sleep(1);
      }

        // 2. ถ้าเป็น Furioso-Replica: ได้ Hermes ในเทิร์นหน้า (ฝากไว้ใน skills[7].active)
        if (atk == &attacker->skills[3] && uncrackedUnbreakable > 0) {
            attacker->skills[7].active = (uncrackedUnbreakable / 2);
            printf("\n%s gains +%d 'Procuration [Hermes]' next turn\n", attacker->name, attacker->skills[7].active);

          sleep(1);

            // เปลี่ยนร่างเป็น Sizzling Wound ทันทีที่ใช้ Furioso ครั้งแรก
            if (attacker->skills[3].active == 1) {
                attacker->skills[3].active = 2;
                printf("\n%s converts 'Wound-casing Mask' to 'Sizzling Wound'\n", attacker->name);

              sleep(1);

              printf("\n%s: \"Haha... I wonder if my darling daughter would remember this wound...\"\n", attacker->name);

              sleep(1);
            }
        }

        if (attacker->skills[1].active > 9) attacker->skills[1].active = 9;
        printf("\n%s's 'Procuration [Hermes]' Stacks: %d/9\n", attacker->name, attacker->skills[1].active);

      if (attacker->skills[1].active == 9) printf("\n%s: \"Ninth. Executed.\"\n", attacker->name);
      if (attacker->skills[1].active == 8) printf("\n%s: \"Eight. One left.\"\n", attacker->name);
      if (attacker->skills[1].active == 7) printf("\n%s: \"Seven. Two to go.\"\n", attacker->name);
      if (attacker->skills[1].active == 6) printf("\n%s: \"Sixth\"\n", attacker->name);
      if (attacker->skills[1].active == 5) printf("\n%s: \"Five\"\n", attacker->name);
      if (attacker->skills[1].active == 4) printf("\n%s: \"Four\"\n", attacker->name);
      if (attacker->skills[1].active == 3) printf("\n%s: \"Third\"\n", attacker->name);
      if (attacker->skills[1].active == 2) printf("\n%s: \"Two\"\n", attacker->name);
      if (attacker->skills[1].active == 1) printf("\n%s: \"One\"\n", attacker->name);

      sleep(1);
    }

    if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && atk == &attacker->skills[3]) {
        // [Attack End] Consume all Procuration [Hermes]
        attacker->skills[1].active = 0; 
        printf("\n%s consumes all Procuration [Hermes] Stacks", attacker->name);

      // [Attack End]
      defender->Burn[0] += 5;
        defender->Bind[1] += 3;
        defender->ProtectionDown[1] += 30; //Fragile: รับดาเมจแรงขึ้น 30%

        printf("\n%s inflicts 5 Burn Stack (%d), 3 Bind next turn and target takes +30%% damage next turn.\n", attacker->name, defender->Burn[0]);
    }

    // -------------------------------------------------------------------------

    // ------------------- The Middle Little Brother Sinclair ------------------------

    // The Middle Little Brother Sinclair Passive never forget
    if (isId(defender->ID, "The Middle Little Brother Sinclair") == 0 && defender->skills[3].active == 0) {

      defender->skills[3].active = 1; // Check for Once per Turn

      defender->skills[0].active += 5;
      if (defender->skills[0].active > 10) defender->skills[0].active = 10;

      printf("\n%s inflicts 5 'Vendetta Mark' against the attacker (%d - Max 10)\n",
        defender->name, defender->skills[0].active);

      sleep(1);
    }

     // The Middle Little Brother Sinclair Passive never forget gain book
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && attacker->skills[0].active > 0) {

        attacker->skills[1].active += attacker->skills[0].active;
      if (attacker->skills[1].active > 30) attacker->skills[1].active = 30;

      printf("\n%s consumes 'Vendetta Mark', %s gains 'Book of Vengeance [Sinclair]' equal to the amount consumed (%d) (%d - Max 30)\n",
        defender->name, attacker->name, attacker->skills[0].active, attacker->skills[1].active);

      attacker->skills[0].active = 0;

      sleep(1);
    }

    // The Middle Little Brother Sinclair loses Envy Resonance
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && attacker->Passive > 0 && attacker->skills[2].active == 1) {

      attacker->skills[2].active = 0;

      attacker->Passive = 0;

      printf("\n%s loses all Envy Resonance\n", attacker->name);

      atk->Clashable = 1;
      atk->skillType = 0;

      attacker->skills[3] = attacker->skills[4];

      sleep(1);

    }

    // -----------------------------------------------

    // Don Quixote:The Manager of La Manchaland - S3-1 Buff S3-2
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") == 0 &&
        atk == &attacker->skills[5]) {

      int Hardblood = (int)(attacker->Passive/10) * 10;

        attacker->Passive -= Hardblood;
      if (attacker->Passive < 1) attacker->Passive = 1;

      printf("\n%s consumes 10 Hardblood for every 10 Hardblood (%d left)\n", attacker->name, attacker->Passive);

      sleep(1);

      if (ClashLostAttack == 0) {

         attacker->Passive += Hardblood/2;
        if (attacker->Passive > 30) attacker->Passive = 30;

        printf("\n%s attacked without Clash Lost, gains half of Hardblood consumes (%d) (%d left)\n", attacker->name, Hardblood/2, attacker->Passive);

        sleep(1);
      }

    }

    // Don Quixote:The Manager of La Manchaland - Empower Skill
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") == 0 &&
        (atk == &attacker->skills[3] || atk == &attacker->skills[4])) {

      int Hardblood = 10;

      attacker->Passive -= Hardblood;
      if (attacker->Passive < 1) attacker->Passive = 1;

      printf("\n%s consumes %d Hardblood (%d left)\n", attacker->name,
             Hardblood, attacker->Passive);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Hardblood gains 2
    if (isId(defender->ID, "Don Quixote:The Manager of La Manchaland") == 0) {

      int Hardblood = 2;

      defender->Passive += Hardblood;
      if (defender->Passive > 30) defender->Passive = 30;

      printf("\n%s gains %d Hardblood (%d)\n", defender->name, Hardblood,
             defender->Passive);

      sleep(1);
    }

  // Heishou Pack - You Branch Adept Heathcliff Skill 3 after attack
  if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2])) {

        inflictStatus(attacker->Burn, 0, 2, 0, 99, 0, 99);

       printf("\n%s gains 2 Burn Count (%d)\n", attacker->name,attacker->Burn[1]);

    sleep(1);

  }

  // Heishou Pack - You Branch Adept Heathcliff Skill 4 after attack consumes
  if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[3])) {

      attacker->Passive = 0;

    attacker->skills[2].Copies = 1;

       printf("\n%s consumes all Battleblood Instinct\n", attacker->name);

    sleep(1);

    if (attacker->Burn[0] > 20) {

      int consumes = (attacker->Burn[0] - 20) > 25 ? 25 : (attacker->Burn[0] - 20);

      attacker->Burn[0] -= consumes;

      int healvalue = consumes * 2;

      attacker->HP += healvalue;
      if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;

    printf("\n%s at more than 20 Burn Stack, consume up to 25 excess Burn (Consumed %d Burn Stack %d) and heal (%d%% - Burn consumed x 2)%% HP\n", attacker->name, consumes, attacker->Burn[0], healvalue);

    sleep(1);

    }

  }

  // --------------------------------------- Binah  -----------------------------------------

  // Binah - Skill 2 debuff
  if (isId(attacker->ID, "Binah") == 0 && (atk == &attacker->skills[1]) && !Evaded) {

    int inflictvalue = 1;

        defender->Paralyze[1] += inflictvalue;

    printf("\n%s inflicts %d Paralyze next turn (Fix the Power of 1 Coins to 0 for one turn)\n", attacker->name,inflictvalue);

    sleep(1);
  }

  // Binah - Skill 4 debuff
  if (isId(attacker->ID, "Binah") == 0 && (atk == &attacker->skills[3]) && !Evaded) {

    int inflictvalue = 50;

    defender->DamageDown[0] += inflictvalue;
        defender->DamageDown[1] += inflictvalue;

    printf("\n%s deals -%d%% damage for this turn and next turn\n", defender->name, inflictvalue);

    sleep(1);
  }

  // Binah - Skill 5 debuff serious
  if (isId(attacker->ID, "Binah") == 0 && (atk == &attacker->skills[4]) && !Evaded) {

    int inflictvalue = 1;

    if (attacker->Passive) {
      inflictvalue = 2;
    }

        defender->FinalPowerDown[0] += inflictvalue;
    defender->FinalPowerDown[1] += inflictvalue;

    printf("\n%s gains %d Final Power Down this turn and next turn by %s's Skill\n", defender->name, inflictvalue, attacker->name);

    sleep(1);
  }

  // Binah - Skill 3 4 debuff serious
  if (isId(attacker->ID, "Binah") == 0 && attacker->Passive && (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && !Evaded) {

    int inflictvalue = 5;

    if (atk == &attacker->skills[3]) {
      inflictvalue = 10;
    }

    if (defender->sanityGainBase >= 0) {

      updateSanity(defender, -(inflictvalue));
      if (defender->Sanity < -45) defender->Sanity = -45;

    printf("\n%s loses %d Sanity by %s's Skill (%d)\n", defender->name, inflictvalue, attacker->name, defender->Sanity);
    } else {

      updateSanity(defender, inflictvalue);
        if (defender->Sanity > 45) defender->Sanity = 45;

      printf("\n%s gains %d Sanity by %s's Skill (%d)\n", defender->name, inflictvalue, attacker->name, defender->Sanity);
    }

    sleep(1);
  }

  // -----------------------------------------------------------------------------------------

   // Hong lu:The Lord of Hongyuan S3-1 Debuff
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[2]) && attacker->Passive > 0 && !Evaded) {

        defTempDefense -= attacker->Passive;
        defTempOffense -= attacker->Passive;

         printf("\n%s's last coin, Inflict 1 Offense Down and 1 Defense Down for every Heishou Bolus Contamination [黑獸丸染] (%d)\n", attacker->name, attacker->Passive);

      sleep(1);
    }

    // -------------------------------- Roland ------------------------------

    // Fixer grade 9? S8 Heal
    if (isId(attacker->ID, "Fixer grade 9?") == 0 && (atk == &attacker->skills[8]) && attacker->Passive >= 10) {

      int totalheal = totalDamage;

      if (totalheal > 10) totalheal = 10;

          updateSanity(attacker, -(totalDamage));
      if (attacker->Sanity < -45) attacker->Sanity = -45;

         printf("\n%s at 10+ Black Silence, loses Sanity equal to damage dealt (%d - Max 10)\n", attacker->name, totalheal);

      sleep(1);
    }

    if (isId(attacker->ID, "Fixer grade 9?") == 0 && atk == &attacker->skills[9]) {

        // [Attack End]
      defender->Bleed[0] += 5;
        defender->Bind[1] += 3;
        defender->ProtectionDown[1] += 30; //Fragile: รับดาเมจแรงขึ้น 30%

        printf("\n%s inflicts 5 Bleed Stack (%d), 3 Bind next turn and target takes +30%% damage next turn.\n", attacker->name, defender->Bleed[0]);
    }

    // Roland – Mang (心)
    if (isId(attacker->ID, "Fixer grade 9?") == 0 && isId(defender->ID, "Binah") == 0 && attacker->skills[6].active > 0) {

      int Mang = attacker->skills[6].active * 5;

      printf("\nIf %s generated Mang (望), lose 5 Sanity for each Mang (望) generated (%d)\n",
        attacker->name, Mang);

        updateSanity(attacker, -(Mang));

      attacker->skills[6].active = 0;

      sleep(1);
    }

    // ----------------------------------------------------------------

  // Dawn Office Fixer Sinclair - Skill EGO form S4 lose sanity
  if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && (atk == &attacker->skills[3] || atk == &attacker->skills[2])) {

      updateSanity(attacker, -(15));
      if (attacker->Sanity < -45) attacker->Sanity = -45;

      printf("\n%s loses 15 Sanity (%d Sanity)\n",
             attacker->name, attacker->Sanity);

      sleep(1);

  }

  // Roland – Buff
  if (isId(attacker->ID, "Fixer grade 9?") == 0 && atk == &attacker->skills[9]) {

    int Losetotal = totalDamage;
    if (Losetotal > 20) Losetotal = 20;

    updateSanity(attacker, -(Losetotal));
    if (attacker->Sanity < -45) attacker->Sanity = -45;

    printf("\n%s loses Sanity equal to dealt damage (%d - Max 20)\n",
      attacker->name, Losetotal);

    sleep(1);
  }

  //--------------------------------Lei heng---------------------

    // Lei heng – Reload at attack end
    if (isId(attacker->ID, "Lei heng") == 0 && (atk == &attacker->defenseSkill[0] || atk == &attacker->skills[2] || atk == &attacker->skills[4])) {

      if (attacker->defenseSkill[2].active > 0 || attacker->defenseSkill[3].active > 0) {

        attacker->defenseSkill[1].active = 0; // Tigermark rounds

        int current_clip = attacker->defenseSkill[2].active;
        int reserve = attacker->defenseSkill[3].active;
        int needed = 6 - current_clip; // ขาดอีกเท่าไหร่จะเต็ม 6

            int take = (reserve >= needed) ? needed : reserve;
            attacker->defenseSkill[2].active += take;
            attacker->defenseSkill[3].active -= take;

        printf("\n%s has Savage Tigermark Round, uses 'Reload - Tactical' (Reload this unit's Ammo to its maximum capacity (does not discard any remaining Ammo))"
          "\n Savage Tigermark Round:\n"
          " - Stack (Loaded Ammo) : %d\n"
           " - Count (Remaining Ammo) : %d\n",
          attacker->name, attacker->defenseSkill[2].active, attacker->defenseSkill[3].active);

        sleep(1);

      } else {

        attacker->defenseSkill[1].active = 6; // Tigermark rounds

      printf("\n%s uses 'Reload' (Lose all currently owned Ammo, and reload back to full) (Tigermark Round: 6)\n",
        attacker->name);

      sleep(1);

      }

    } else // Lei heng – Reload at attack end
      if (isId(defender->ID, "Lei heng") == 0 && (defSkill == &defender->defenseSkill[0])) {

        if (defender->defenseSkill[2].active > 0 || defender->defenseSkill[3].active > 0) {

            defender->defenseSkill[1].active = 0; // Tigermark rounds

          int current_clip = defender->defenseSkill[2].active;
          int reserve = defender->defenseSkill[3].active;
          int needed = 6 - current_clip; // ขาดอีกเท่าไหร่จะเต็ม 6

              int take = (reserve >= needed) ? needed : reserve;
                defender->defenseSkill[2].active += take;
                defender->defenseSkill[3].active -= take;

          printf("\n%s has Savage Tigermark Round, uses 'Reload - Tactical' (Reload this unit's Ammo to its maximum capacity (does not discard any remaining Ammo))"
            "\n Savage Tigermark Round:\n"
            " - Stack (Loaded Ammo) : %d\n"
             " - Count (Remaining Ammo) : %d\n",
            defender->name, defender->defenseSkill[2].active, defender->defenseSkill[3].active);

          sleep(1);

        } else {

            defender->defenseSkill[1].active = 6; // Tigermark rounds

        printf("\n%s uses 'Reload' (Lose all currently owned Ammo, and reload back to full) (Tigermark Round: 6)\n",
          defender->name);

        sleep(1);

        }

      }

    // Lei heng – Chosen Prey after attack stagger
    if (isId(attacker->ID, "Lei heng") == 0 && attacker->skills[6].active == 1 && isStaggered(defender)) {

      updateSanity(defender, -10);

      defender->ClashPowerDown[0]++;
      defender->ClashPowerDown[1]++;

      printf("\n%s loses 10 Sanity (%d) and gains 1 Clash Power Down this turn and next turn\n",
        defender->name, defender->Sanity);
      sleep(1);
    }

  // Lei heng – skill 3 or 6
  if (isId(attacker->ID, "Lei heng") == 0 &&
      (atk == &attacker->skills[2] || atk == &attacker->skills[4])) {

    int clashpowerdebuff = attacker->skills[1].active * 1;
    if (clashpowerdebuff > 5) clashpowerdebuff = 5;
    int takemoredamage = attacker->skills[1].active * 10;
    if (takemoredamage > 50) takemoredamage = 50;

    attacker->ClashPowerDown[1] += clashpowerdebuff;
      attacker->ProtectionDown[1] += takemoredamage;

        printf("\n%s gains %d Overheat (Clash Power -%d, Take %d%% more damage) next turn\n", attacker->name, attacker->skills[1].active, clashpowerdebuff, takemoredamage);

  }

  // Lei heng – inner strength gain
  if (isId(attacker->ID, "Lei heng") == 0 &&
    attacker->skills[0].active == 2) {

    attacker->Passive += remainingCoins * 2;
    if (attacker->Passive >= 25)
      attacker->Passive = 25;

    printf("\n%s gains +%d Inner Strength [底力](%d - Max 25)\n",
           attacker->name, remainingCoins * 2, attacker->Passive);

  } else if (isId(attacker->ID, "Lei heng") == 0 &&
    attacker->skills[0].active == 3) {

    attacker->Passive += remainingCoins * 3;
    if (attacker->Passive >= 50)
      attacker->Passive = 50;

    printf("\n%s gains +%d Extreme Strength [極力](%d - Max 50)\n",
           attacker->name, remainingCoins * 3, attacker->Passive);
  }

  // --------------------- Heathcliff:Wild Hunt -----------------------
  // Heathcliff:Wild Hunt – skill 3 heal sanity
  if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
      atk == &attacker->skills[3]) {

    if (attacker->Sanity < 0) {
      printf("\n%s at less than 0 Sanity, heals 10 Sanity. When at less than 0 Sanity, heal more Sanity the further this unit's Sanity is from 0 (heal 2 additionalal Sanity for every missing Sanity; Max 50)\n",
         attacker->name);

        int missingSP = -attacker->Sanity;       // how far below 0
        int extraHeal = 2 * missingSP;           // 2 Sanity per missing SP
        if (extraHeal > 50) extraHeal = 50;      // cap at 50

        int totalHeal = 10 + extraHeal;          // base 10 + extra
        updateSanity(attacker, totalHeal);

      printf("\n%s heals %d Sanity (%d)\n",
         attacker->name, totalHeal, attacker->Sanity);
    }
  }

    // --------------------- Heathcliff:Wild Hunt -----------------------
    // Heathcliff:Wild Hunt – skill 3 loses Dullahan on attack side
    if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
        atk == &attacker->skills[3] && attacker->skills[0].active > 0) {

        atk->skillType = 0;

         attacker->skills[0].active = 0;
      attacker->skills[2].Copies = 1;

        attacker->MinSpeed -= 1;
        attacker->MaxSpeed -= 1;

        printf("\n%s loses all 'Dullahan'\n", attacker->name);
    } 

    // Heathcliff:Wild Hunt – skill 3 loses Dullahan on defender side
    if (isId(defender->ID, "Heathcliff:Wild Hunt") == 0 &&
      defSkill == &defender->skills[3] && defender->skills[0].active > 0) {

      atk->skillType = 0;

           defender->skills[0].active = 0;
        defender->skills[2].Copies = 1;

        defender->MinSpeed -= 1;
        defender->MaxSpeed -= 1;

        printf("\n%s loses all 'Dullahan'\n", defender->name);
    }

  // Heathcliff:Wild Hunt – gain coffin
  if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
     (atk == &attacker->skills[2] || atk == &attacker->skills[3])) {

    int gain;

    if (atk == &attacker->skills[2]) {
      gain = 1;
    } else {
      gain = 2;
    }
    attacker->Passive += gain;
    if (attacker->Passive > 10) attacker->Passive = 10;

    printf("\n%s gains %d Coffin (%d - Max 10)\n",
       attacker->name, gain, attacker->Passive);

      sleep(1);
  }

    // Heathcliff:Wild Hunt – Skill 4 heal sanity
    if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
       (atk == &attacker->skills[3])) {

      updateSanity(attacker, 10);
      if (attacker->Sanity > 45) attacker->Sanity = 45;

      printf("\n%s heals 10 Sanity (%d)\n",
         attacker->name, attacker->Sanity);

        sleep(1);
    }

     // Heathcliff:Wild Hunt – Skill 2 loses sanity
      if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
         (atk == &attacker->skills[1]) && attacker->skills[0].active > 0) {

        updateSanity(attacker, -(10));
        if (attacker->Sanity < -45) attacker->Sanity = -45;

        printf("\n%s mounted 'Dullahan', loses 10 Sanity (%d)\n",
           attacker->name, attacker->Sanity);

          sleep(1);
      }

  // --------------------------- Hong lu:The Lord of Hongyuan -------------------------- 
  // Hong lu:The Lord of Hongyuan - S3-1 to S3-2
  if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      atk == &attacker->skills[2] &&
      (attacker->skills[5].BasePower == 1 ||
       attacker->skills[5].CoinPower == 1 || attacker->skills[5].Coins == 1 ||
       attacker->skills[5].Offense == 1)) {

    SkillStats *skill = &attacker->skills[5];
    int *fields[] = {&skill->BasePower, &skill->CoinPower, &skill->Coins,
                     &skill->Offense};
    const char *names[] = {"Mao", "Si", "Wu", "You"};
    int fieldCount = 4;

    const char *HeshinPacks = NULL;

    while (HeshinPacks == NULL) {
      int availableIndices[4];
      int availableCount = 0;

      // Keep stats that are still 1
      for (int i = 0; i < fieldCount; i++) {
        if (*fields[i] == 1) {
          availableIndices[availableCount++] = i;
        }
      }

      if (availableCount == 0) {
        break; // no more stats to assign
      }

      // randomly select one of the available stats
      int randIndex = availableIndices[rand() % availableCount];
      HeshinPacks = names[randIndex];
      *fields[randIndex] = 0;
    }

    if (isId(HeshinPacks, "Mao") == 0) {

      printf("\n%s: \"Send the Hare.\"\n", attacker->name);

      sleep(1);

      printf("\n%s: \"Carve those pests out!\"\n", attacker->name);

      const char *savename = attacker->name;

      attacker->name = "Heshin Packs - Mao";

      attackPhase(attacker, &attacker->skills[6], attacker->skills[6].Offense,
        attacker->skills[6].Defense, defender, defSkill,
        defTempOffense, defTempDefense, attacker->skills[6].Coins, 0, clashCount);

      attacker->name = savename;

       sleep(1);

      printf("\n%s: \"Mao...\"\n", attacker->name);

      sleep(1);

      attackPhase(attacker, &attacker->skills[4], attacker->skills[4].Offense,
                  attacker->skills[4].Defense, defender, defSkill,
                  defTempOffense, defTempDefense, attacker->skills[4].Coins, 0, clashCount);

      sleep(1);

      attacker->skills[5].BasePower = 0;

      attacker->Passive += 1;

      // Increase Offense by 3 for all skills except skill 5
      for (int i = 0; i < attacker->numSkills; i++) {
        if (i != 5) {
          attacker->skills[i].Offense += 3;
        }
      }

      atkTempOffense += 3;

      printf("\n%s gains 1 Heishou Bolus Contamination [黑獸丸染](%d) - "
             "Mao (Offense +3)\n",
             attacker->name, attacker->Passive);

    } else if (isId(HeshinPacks, "Si") == 0) {

      printf("\n%s: \"Pierce with the Serpent.\"\n", attacker->name);

      sleep(1);

      printf("\n%s: \"Carve those pests out!\"\n", attacker->name);

      const char *savename = attacker->name;

      attacker->name = "Heshin Packs - Si";

      attackPhase(attacker, &attacker->skills[7], attacker->skills[7].Offense,
        attacker->skills[7].Defense, defender, defSkill,
        defTempOffense, defTempDefense, attacker->skills[7].Coins, 0, clashCount);

      attacker->name = savename;

       sleep(1);

      printf("\n%s: \"Si...\"\n", attacker->name);

      sleep(1);

      attackPhase(attacker, &attacker->skills[4], attacker->skills[4].Offense,
                  attacker->skills[4].Defense, defender, defSkill,
                  defTempOffense, defTempDefense, attacker->skills[4].Coins, 0, clashCount);

      sleep(1);

      attacker->skills[5].CoinPower = 0;

      attacker->Passive += 1;

      // Increase Offense by 3 for all skills except skill 5
      for (int i = 0; i < attacker->numSkills; i++) {
        if (i != 5) {
          attacker->skills[i].BasePower += 1;
        }
      }

      printf("\n%s gains 1 Heishou Bolus Contamination [黑獸丸染](%d) - "
             "Si (Base Power +1)\n",
             attacker->name, attacker->Passive);

    } else if (isId(HeshinPacks, "Wu") == 0) {

      printf("\n%s: \"Intercept with the Horse.\"\n", attacker->name);

      sleep(1);

      printf("\n%s: \"Carve those pests out!\"\n", attacker->name);

      const char *savename = attacker->name;

      attacker->name = "Heshin Packs - Wu";

      attackPhase(attacker, &attacker->skills[8], attacker->skills[8].Offense,
        attacker->skills[8].Defense, defender, defSkill,
        defTempOffense, defTempDefense, attacker->skills[8].Coins, 0, clashCount);

      attacker->name = savename;

       sleep(1);

      printf("\n%s: \"Wu...\"\n", attacker->name);

      sleep(1);

      attackPhase(attacker, &attacker->skills[4], attacker->skills[4].Offense,
                  attacker->skills[4].Defense, defender, defSkill,
                  defTempOffense, defTempDefense, attacker->skills[4].Coins, 0, clashCount);

      sleep(1);

      attacker->skills[5].Coins = 0;

      attacker->Passive += 1;

      // Increase Offense by 3 for all skills except skill 5
      for (int i = 0; i < attacker->numSkills; i++) {
        if (i != 5) {
          attacker->skills[i].Defense += 5;
        }
      }

     atkTempDefense += 5;

      printf("\n%s gains 1 Heishou Bolus Contamination [黑獸丸染](%d) - "
             "Wu (Defense +5)\n",
             attacker->name, attacker->Passive);

    } else if (isId(HeshinPacks, "You") == 0) {

      printf("\n%s: \"Charge forth, Rooster.\"\n", attacker->name);

      sleep(1);

      printf("\n%s: \"Carve those pests out!\"\n", attacker->name);

      const char *savename = attacker->name;

      attacker->name = "Heshin Packs - You";

      attackPhase(attacker, &attacker->skills[9], attacker->skills[9].Offense,
        attacker->skills[9].Defense, defender, defSkill,
        defTempOffense, defTempDefense, attacker->skills[9].Coins, 0, clashCount);

      attacker->name = savename;

      updateSanity(attacker, 15);
      if (attacker->Sanity > 45) attacker->Sanity = 45;

      printf("\n%s heals 15 Sanity (%d)\n", attacker->name, attacker->Sanity);

       sleep(1);

      printf("\n%s: \"You...\"\n", attacker->name);

      sleep(1);

      attackPhase(attacker, &attacker->skills[4], attacker->skills[4].Offense,
                  attacker->skills[4].Defense, defender, defSkill,
                  defTempOffense, defTempDefense, attacker->skills[4].Coins, 0, clashCount);

      sleep(1);

      attacker->skills[5].Offense = 0;

      attacker->Passive += 1;

      // Increase for all skills except skill 5
      for (int i = 0; i < attacker->numSkills; i++) {
        if (i != 5) {
          attacker->skills[i].DmgMutiplier += 0.1;
        }
      }

      printf("\n%s gains 1 Heishou Bolus Contamination [黑獸丸染](%d) - "
             "You (Damage Multiplier +0.1)\n",
             attacker->name, attacker->Passive);
    }

    sleep(1);

    printf("\n%s: \"Retreat. Your life still has a use to fulfil.\"\n",
      attacker->name);

    sleep(1);

  } else if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
             atk == &attacker->skills[2]) {

    printf("\n%s: \"I'll show you myself...\"\n", attacker->name);

    sleep(1);

    attackPhase(attacker, &attacker->skills[3], attacker->skills[3].Offense,
                attacker->skills[3].Defense, defender, defSkill, defTempOffense,
                defTempDefense, attacker->skills[3].Coins, 0, clashCount);
  }

  // Hong lu:The Lord of Hongyuan - S3-1 to S3-2
  if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      atk == &attacker->skills[3]) {

    printf("\n%s: \"How Daguanyuan has been purged.\"\n", attacker->name);

    sleep(1);
  }

  // Hong lu:The Lord of Hongyuan - Lordsguard
  if (isId(defender->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      defender->skills[5].active == -1 && (HeshinPacks != NULL)) {

    if (defender->HP <= 0) {

    defender->HP = 1;

    printf("\n%s: \"Retreat. Your life still has a use to fulfil.\"\n",
           defender->name);

       SkillStats *skill = &defender->skills[5];
      int *fields[] = {&skill->BasePower, &skill->CoinPower, &skill->Coins,
                       &skill->Offense};

      const char *names[] = {"Mao", "Si", "Wu", "You"};
      int fieldCount = 4;

      if (HeshinPacks != NULL) {
          int targetIndex = -1;

          // วนลูปหาว่า HeshinPacks ตรงกับชื่อไหนใน names
          for (int i = 0; i < fieldCount; i++) {
              if (strcmp(HeshinPacks, names[i]) == 0) {
                  targetIndex = i;
                  break;
              }
          }

          // ถ้าเจอ Index ที่ตรงกัน
          if (targetIndex != -1) {
              // ตอนนี้สามารถเข้าถึงหรือแก้ไขค่าใน *fields[targetIndex] ได้แล้ว
              *fields[targetIndex] = 0;
          }
      }

    } else {

      printf("\n%s: \"I do not allow retreat until the enemy has been slain.\"\n",
         defender->name);

    }

    sleep(1);

  }

  // Hong lu:The Lord of Hongyuan - Mao
  if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      atk == &attacker->skills[6] && !Evaded) {

    defender->ClashPowerDown[1] += 1;

    printf("\n%s gains 1 Clash Power Down next turn by %s's Skill\n", defender->name, attacker->name);

    sleep(1);

  } // Hong lu:The Lord of Hongyuan - Si
  else if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      atk == &attacker->skills[7] && !Evaded) {

    int randomlost = rand() % 2 + 1;

    if (randomlost) {

    defender->OffenseLevelDown[1] += 1;

    printf("\n%s gains 1 Offense Down next turn by %s's Skill\n", defender->name, attacker->name);

    } else if (!randomlost) {

        defender->DefenseLevelDown[1] += 1;

        printf("\n%s gains 1 Defense Down next turn by %s's Skill\n", defender->name, attacker->name);

        }

    sleep(1);

  } // Hong lu:The Lord of Hongyuan - Wu
  else if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      atk == &attacker->skills[8] && !Evaded) {

    defender->ProtectionDown[1] += 20;

    printf("\n%s takes +20%% damage next turn by %s's Skill\n", defender->name, attacker->name);

    sleep(1);

  }

  // --------------------------------------------------------------

    // Meursault:Blade Lineage Mentor - Passive
    if (isId(defender->ID, "Meursault:Blade Lineage Mentor") == 0) {

      defender->FinalPowerUp[1] += 1;

      printf("\n%s gain +1 Final Power next turn (Once per enemy's skill)\n",
             defender->name);

      sleep(1);
    }

     // Yi sang:Fell Bullet - Torn Memory S3 lost
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 &&
        atk == &attacker->skills[2]) {

      updateSanity(attacker, -(attacker->Passive*2));
      if (attacker->Sanity < -45) attacker->Sanity = -45;

      printf("\n%s loses (Torn Memory x 2) Sanity (%d - Max 14) (Sanity %d)\n",
             attacker->name, attacker->Passive*2, attacker->Sanity);

      sleep(1);
    }

  // Yi sang:Fell Bullet - Torn Memory S3 lost
  if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 &&
      atk == &attacker->skills[2] &&
      attacker->Passive >= 7) {

    updateSanity(attacker, 20);
    if (attacker->Sanity > 45) attacker->Sanity = 45;

    attacker->skills[2].active++;

    printf("\n%s loses all Torn Memory to gain 'Fell Bullet' (%d), All skills' Damage Multiplier +0.2 and Clash Power +2 then heal 20 Sanity on self (%d)\n",
           attacker->name, attacker->skills[2].active, attacker->Sanity);

    attacker->skills[0].DmgMutiplier += 0.2;
    attacker->skills[1].DmgMutiplier += 0.2;
    attacker->skills[2].DmgMutiplier += 0.2;
     attacker->defenseSkill[0].DmgMutiplier += 0.2;
    attacker->skills[2].Copies = 1;

    attacker->Passive = 0;

    sleep(1);

    printf("\n%s: \"Thus, you sink into a bottomless slumber.\"\n",
           attacker->name);

           sleep(1);
  }

    // -------------------------------- Don Quixote:The Manager of La Manchaland --------------------------------

  // Don Quixote:The Manager of La Manchaland - Hardblood gains 5 for counter
  // won
  if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
          0 &&
      (atk == &attacker->skills[6])) {

    attacker->Passive += 5;
    if (attacker->Passive > 30) attacker->Passive = 30;

    printf("\n%s gains 5 Hardblood(%d)\n", attacker->name, attacker->Passive);

    sleep(1);
  }

  //------------------------------------Sancho:The Second Kindred of Don Quixote-------------------

  // Sancho - Ultimate After attack
  if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
      (atk == &attacker->skills[12]) && attacker->Passive >= 1 && !Evaded) {

     attacker->Passive -= 1;
    if (attacker->Passive < 1) attacker->Passive = 1;

    defender->Paralyze[1] += 1;

      printf("\n%s consumes 1 Hardblood(%d) to inflict 1 Paralyze next turn (Fix the Power of 1 Coins to 0 for one turn)\n", attacker->name, attacker->Passive); 

    sleep(1);
  }

    // Sancho - Skill 11
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        (atk == &attacker->skills[10]) && !Evaded) {

      updateSanity(defender, -(2));
      if (defender->Sanity < -45) defender->Sanity = -45;

        printf("\n%s consumes 2 Sanity of target (%d)\n", attacker->name, defender->Sanity); 

      sleep(1);

      if (attacker->Passive >= 3) {
       attacker->Passive -= 3;
      if (attacker->Passive < 1) attacker->Passive = 1;

      updateSanity(defender, -(5));
        if (defender->Sanity < -45) defender->Sanity = -45;

        printf("\n%s consumes 3 Hardblood (%d) to consumes 5 Sanity of target (%d)\n", attacker->name, attacker->Passive, defender->Sanity); 

      sleep(1);
    }
    }

    // Sancho - Skill 12
    if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        (atk == &attacker->skills[11]) && !Evaded) {

      defender->Paralyze[1] += 1;

      printf("\n%s inflict 1 Paralyze next turn (Fix the Power of 1 Coins to 0 for one turn)\n", attacker->name);

      sleep(1);

      if (attacker->Passive >= 3) {
       attacker->Passive -= 3;
      if (attacker->Passive < 1) attacker->Passive = 1;

      defender->Paralyze[1] += 2;

        printf("\n%s consumes 3 Hardblood (%d) to inflict 2 Paralyze next turn (Fix the Power of 1 Coins to 0 for one turn)\n", attacker->name, attacker->Passive); 

      sleep(1);
    }

    }

    // Sancho:The Second Kindred of Don Quixote - Gain Hardblood on Hit without Clash Lose
     if (isId(attacker->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && ClashLostAttack == 0) {

        attacker->Passive += 3;
       if (attacker->Passive > 30) attacker->Passive = 30;

       printf("\n%s gains 3 Hardblood (%d)\n", attacker->name, attacker->Passive); 

       sleep(1);
     }

    // --------------------------------------------------

    // -------------------- Erlking Heathcliff ----------------------------
    // Erlking Heathcliff Skill 2 buff
    if (isId(attacker->ID, "Erlking Heathcliff") == 0 && atk == &attacker->skills[2] && !Evaded) {

       attacker->DamageUp[1] += 30;
       attacker->FinalPowerUp[1] += 3;

      printf("\n%s gains 3 Final Power Up and +30%% damage next turn\n", attacker->name);

      sleep(1);
    }

    // Erlking Heathcliff Skill 3 debuff
    if (isId(attacker->ID, "Erlking Heathcliff") == 0 && (atk == &attacker->skills[3]) && !Evaded) {

         defender->Paralyze[1] += 3;

      printf("\n%s inflict 3 Paralyze next turn (Fix the Power of 3 Coins to 0 for one turn)\n", attacker->name);

      sleep(1);
    }

// --------------------------------------------------------------

//------------------------------------------------------------------

  // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly
  if (isId(attacker->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0) {

      printf("\n%s consumed %d The Living & The Departed(%d)\n", attacker->name, attacker->skills[3].active, attacker->Passive); 

    sleep(1);
  }

  // Lobotomy E.G.O::Solemn Lament Yi Sang - SKill 2 gain
  if (isId(attacker->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 && atk == &attacker->skills[1]) {

    if (attacker->Passive >= 1 && attacker->Passive < 20) {

    int gainback = clashCount;
    if (gainback > 6) gainback = 6;

    attacker->Passive += gainback;
      if (attacker->Passive > 20) attacker->Passive = 20;

      printf("\n%s at 1+ The Living & The Departed, gains The Living & The Departed equal to Clash Count (%d - Max 6) (%d)\n", attacker->name, gainback, attacker->Passive); 

    sleep(1);

    }

  }

  // ------------------------- Jia Qiu -------------------------------

  // Jia Qiu - S4 and S9
  if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[4] || atk == &attacker->skills[9]) && !Evaded) {

      defender->ClashPowerDown[1] += 1;

    printf("\n%s gains 1 Clash Power Down next turn by %s's Skill\n", defender->name, attacker->name);

    sleep(1);
  }

  // Jia Qiu - S2 and S8
  if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[2] || atk == &attacker->skills[8]) && !Evaded) {

        updateSanity(defender, -(5 * remainingCoins));
    if (defender->Sanity < -45) defender->Sanity = -45;

    printf("\n%s loses 5 Sanity for every one remaining coins of %s's Skill (%d) (Sanity %d)\n", defender->name, attacker->name, 5 * remainingCoins, defender->Sanity);

    sleep(1);
  }

  // Jia Qiu - S12
  if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[12])) {

    int boost = 5*(abs(defender->Sanity));

        attacker->DamageUp[1] += boost;

    printf("\n%s deal +5%% damage for every Sanity enemy further from 0 next turn (%d%%)\n", attacker->name, defender->Sanity);

    sleep(1);
  }

  // Jia Qiu - Passive
  if (isId(attacker->ID, "Jia Qiu") == 0) {
      updateSanity(attacker, 5);
    if (attacker->Sanity > 45) attacker->Sanity = 45;

    printf("\n%s heals 5 Sanity (%d)\n", attacker->name, attacker->Sanity);
  }

  // Jia Qiu Phase 1.5
  if (isId(defender->ID, "Jia Qiu") == 0 && defender->HP <= defender->MAX_HP * 0.85 &&
      defender->Passive == 1) {

    printf("\n%s: \"I am not yet to hear 'your' answer.\"\n", defender->name);

    sleep(1);

    printf("\n%s: \"That answer, hiding in the shadows of your hesitation.\"\n",
           defender->name);

    sleep(1);

    printf("\n%s: \"That answer that you buried deep within.\"\n",
           defender->name);

    defender->Passive = 2;

    // Disable the old
    for (int i = 0; i < 2; i++) {
      if (defender->skills[i].Copies > 0) {
        defender->skills[i] = defender->skills[3];
        defender->skills[i].Copies = -1;
      }
    }
    defender->defenseSkill[1] = defender->skills[3];
    defender->defenseSkill[1].Copies = -1;

    // Set copies for the newly mapped primary skills
     defender->skills[4].Copies = 4;
    defender->skills[2].Copies = 3;
    defender->skills[5].active = 0;
  }

  // Jia Qiu LAST Ult
  if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[16])) {

    defender->HP = 1;

    printf("\n%s's HP drop to 1\n", defender->name);


    sleep(1);
  }


  // -------------------------------------------------------------------------------------------------------------


    // The House of Spiders: The Thumb Nursefather Rodion - Precognition evade
    if (isId(defender->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (defender->defenseSkill[1].active == 1 || defender->defenseSkill[1].active == 2)) {

      defender->defenseSkill[1].active = 2;

      int lost = 2;

      int canLose = 10 - defender->skills[6].active; // โควต้าที่เหลือในเทิร์นนี้
      if (canLose > 0) {

      int actualLoss = (lost > canLose) ? canLose : lost;
      if (actualLoss > defender->Passive) actualLoss = defender->Passive;

        defender->Passive -= actualLoss;
        defender->skills[6].active += actualLoss; // บันทึกว่าเสียไปเท่าไหร่แล้วในเทิร์นนี้

      printf("\n%s loses %d Eye of Precognition on self (%d)\n", defender->name, actualLoss, defender->Passive);

      sleep(1);

        if (defender->Passive <= 0 && defender->skills[11].active == 0) {
            defender->skills[11].active = 1; // ติด Overheat

          printf("\n%s converts 'Eye of Precognition' into 'Eye of Precognition - Overheat'\n", defender->name);

          sleep(1);
        }

      }

      if (Evaded) {

      printf("\nThis %s's Skills does not trigger Defense Skills\n", defender->name);

      sleep(1);

      attackPhase(defender, &defender->skills[1], defender->skills[1].Offense,
      defender->skills[1].Defense, attacker, atk, atkTempOffense,
        atkTempDefense, defender->skills[1].Coins, 0, clashCount);

    }
    }

  } // closes if (attacker->HP > 0 && !isStaggered(attacker)) (depth 2→1)

} // closes attackPhase (depth 1→0)



























// Returns effective skill and also temporary offense/defense for this turn
SkillStats *getEffectiveSkill(Character *c, Character *c2,
                              SkillStats *chosenSkill, int *tempOffense,
                              int *tempDefense) {

   if (chosenSkill == NULL) return NULL;  // No skill selected


  // ----------------------- Combat Start --------------------------

  *tempOffense += chosenSkill->Offense;
  *tempDefense += chosenSkill->Defense;







  // --------------- The House of Spiders: The Ring Nursefather Hong Lu ----------------

  // The House of Spiders: The Ring Nursefather Hong Lu - Consumed and loses / Buff Passive
  if (isId(c->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && !isStaggered(c) && !isPanicked(c)) {

    if (c->Passive > 0) {
    c->Passive -= 1;
    if (c->Passive < 0) c->Passive = 0;
    printf("\n%s loses 1 Viewing the Tableau (%d)\n", c->name, c->Passive);

    sleep(1);
    }

    if (c->skills[8].active == 1) {

      if (chosenSkill->skillType == 0) {
      chosenSkill->ClashPower[0] += 1;
      chosenSkill->DamageUp[0] += 10;
      }

      printf("\n%s has Somatic Frisson-inspiring Melody gains following effect:"
        "\n - Min & Max Speed +1 "
        "\n - Base Attack Skills gain Clash Power +1 and deal +10%% damage\n", c->name);
    }

    int Consumed = 0;
    int Gain = 0;

    if (c->Charge[1] >= 10) {

      if (chosenSkill == &c->skills[1]) {
         c->skills[5].active = 1;
        Consumed += 5;
        inflictStatus(c->Charge, 0, -5, 0, 99, 0, 20);

        printf("\n%s at 10+ Corpus Ingredient Count, consume 5 Corpus Ingredient Count (%d) to gain the following effects:", c->name, c->Charge[1]);

        chosenSkill->ClashPower[0] += 1;
        chosenSkill->Unbreakable = 1;

        printf("\n - Clash Power +1"
          "\n - Convert the final Coin on this Skill to Unbreakable Coin\n");

      } else if (chosenSkill == &c->skills[3]) {
         c->skills[5].active = 1;
        Consumed += (c->Charge[1] > 20 ? 20 : c->Charge[1]);

        printf("\n%s at 10+ Corpus Ingredient Count (%d), consume up to 20 Corpus Ingredient Count (%d) to gain the following effects:", c->name, c->Charge[1], Consumed);

        inflictStatus(c->Charge, 0, -Consumed, 0, 99, 0, 20);

        chosenSkill->FinalPowerBoost[0] += 2;
        chosenSkill->DamageUp[0] += 1 * Consumed;
        chosenSkill->Unbreakable = chosenSkill->Coins;

        if (c->Charge[0] >= 5) {
          chosenSkill->DamageUp[0] += 60;
        } else if (c->Charge[0] >= 3) {
            chosenSkill->DamageUp[0] += 30;
          }

        printf("\n - Final Power +2"
         "\n - Deal +1%% damage for every Corpus Ingredient Count consumed"
          "\n - At 3+/5+ Corpus Ingredient Stack, gain +30%%/+60%% Damage (%d)"
          "\n - Convert all Coins on this Skill to Unbreakable Coins\n", c->Charge[0]);

        } else if (chosenSkill == &c->skills[4]) {
         c->skills[5].active = 1;
        Consumed += (c->Charge[1] > 20 ? 20 : c->Charge[1]);

        printf("\n%s at 10+ Corpus Ingredient Count (%d), consume up to 20 Corpus Ingredient Count (%d) to gain the following effects:", c->name, c->Charge[1], Consumed);

          inflictStatus(c->Charge, 0, -Consumed, 0, 99, 0, 20);

        chosenSkill->FinalPowerBoost[0] += 2;
        chosenSkill->DamageUp[0] += 1 * Consumed;

          if (c->Charge[0] >= 5) {
            chosenSkill->DamageUp[0] += 60;
          } else if (c->Charge[0] >= 3) {
              chosenSkill->DamageUp[0] += 30;
            }

          printf("\n - Final Power +2"
           "\n - Deal +1%% damage for every Corpus Ingredient Count consumed"
            "\n - At 3+/5+ Corpus Ingredient Stack, gain +30%%/+60%% Damage (%d)\n", c->Charge[0]);

      } else if (chosenSkill == &c->defenseSkill[0]) {
         c->skills[5].active = 1;
        Consumed += 5;
        inflictStatus(c->Charge, 0, -5, 0, 99, 0, 20);

        printf("\n%s at 10+ Corpus Ingredient Count, consume 5 Corpus Ingredient Count (%d) to gain the following effects:", c->name, c->Charge[1]);

        chosenSkill->ClashPower[0] += 1;
        chosenSkill->Unbreakable = 1;

        printf("\n - Clash Power +1"
          "\n - Convert the final Coin on this Skill to Unbreakable Coin\n");

      }

      sleep(1);

    }

    c->skills[6].active += Consumed; // Count Consumed

    if (c->skills[6].active >= 10) {
    Gain = c->skills[6].active / 10;
      c->skills[6].active -= Gain * 10; // Count Consumed
      inflictStatus(c->Charge, Gain, 0, 0, 99, 0, 20);
      printf("\n%s gains +%d Corpus Ingredient Stack (%d)\n", c->name, Gain, c->Charge[0]);

      sleep(1);

      if (c->Charge[0] >= 2 && !c->skills[14].active) {
        c->skills[14].active = 1; // Flagged
        printf("\n%s gains Artwork: Tibia\n", c->name);

        sleep(1);

        printf("\n%s: \"Ah...! Marvelous art...!\"\n", c->name);

        sleep(1);
      }
    }

  }

  // The House of Spiders: The Ring Nursefather Hong Lu - Artwork: Tibia
  if (isId(c->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0) {

    int tibStack = c->Charge[0];
    if (tibStack >= 2) {
        int baseBoost = 1;
        if (tibStack >= 4) baseBoost++;
        if (tibStack >= 5) baseBoost++;
        c->BasePowerUp[0] += baseBoost;
        printf("\n%s gains +%d Base Power from Artwork: Tibia (Corpus Ingredient Stack: %d)\n", c->name, baseBoost, tibStack);

      sleep(1);
    }

  }

  // The House of Spiders: The Ring Nursefather Hong Lu - Skill 1/4
  if (isId(c->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[3])) {

    if (chosenSkill == &c->skills[0]) {

    int count = countNegativeEffectTypes(c2);

    if (count > 0) {
      int gain = count*10;
      if (gain > 30) gain = 30;

      if (gain > 0) {

        chosenSkill->DamageUp[0] += gain;

      printf("\n%s deals +10%% damage (%d%% - Max 30%%) for every type of negative effects on target (%d)\n", c->name, gain, count);

      sleep(1);
      }
    }

    }

    if (c2->Bleed[0] >= 4) {

    int gain = c2->Bleed[0]/4;
    if (gain > 3) gain = 3;

    if (gain > 0) {

      chosenSkill->FinalPowerBoost[0] += gain;

     printf("\n%s gains +1 Final Power (%d - Max 3) for every 4 Bleed Stack on target (%d)\n", c->name, gain, c2->Bleed[0]);

      sleep(1);

    }

  }

  }

  // The House of Spiders: The Ring Nursefather Hong Lu - Skill 2
  if (isId(c->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0 && chosenSkill == &c->skills[1]) {

    int count = countNegativeEffectTypes(c2);

    if (count > 0) {
      int gain = count*10;
      if (gain > 30) gain = 30;

      if (gain > 0) {

        chosenSkill->DamageUp[0] += gain;

      printf("\n%s deals +10%% damage (%d%% - Max 30%%) for every type of negative effects on target (%d)\n", c->name, gain, count);

      sleep(1);
      }
    }

    if (c2->Bleed[0] >= 4) {

    int gain = c2->Bleed[0]/4;
    if (gain > 4) gain = 4;

    if (gain > 0) {

      chosenSkill->FinalPowerBoost[0] += gain;

     printf("\n%s gains +1 Final Power (%d - Max 4) for every 4 Bleed Stack on target (%d)\n", c->name, gain, c2->Bleed[0]);

      sleep(1);

    }

  }

  }

    // ----------------------------------------------------------------

  // --------------- The Middle Nursefather - Matthias ----------------

  // The Middle Nursefather - Matthias - Passive
  if (isId(c->ID, "The Middle Nursefather - Matthias") == 0) {
      // 1. Check This Out, Kiddo! บัฟ
      int kiddo = c->skills[1].active;
      c->BasePowerUp[0] += (kiddo / 2);
      c->DamageUp[0] += (kiddo * 5);
      if (kiddo > 0) 
          printf("\n%s gains Base Power +%d and Damage +%d%% from 'Check This Out, Kiddo!' (%d)\n", 
                 c->name, kiddo/2, kiddo * 5, kiddo);

    sleep(1);

      // 2. The Middle - Grudge บัฟ
      int grudge = c->skills[0].active;
    if (grudge > 0) {
      *tempOffense += grudge;
      *tempDefense -= (grudge * 2);
    printf("\n%s gains Offense +%d, Defense -%d from 'The Middle - Grudge' (%d)\n", c->name, grudge, (grudge * 2), c->skills[0].active);

       sleep(1);

      if (grudge >= 10) {
          chosenSkill->Protection[0] -= 20; // Take +20% damage
          printf("\n%s is at Max 'The Middle - Grudge' Stack! Take +20%% damage\n", c->name);

        sleep(1);
      }
    }

    if (c->skills[9].active == 3 && chosenSkill != &c->skills[6]) {

      int take = (((c->MAX_HP - c->HP) / c->MAX_HP) * 100);
      if (take > 90) take = 90;

      c->ProtectionUp[0] += take;

      printf("\n%s - 'Ridiculous Grit' gains following effect:\n"
       " - Take -(Missing HP percentage)%% damage from Skill (%d%% - Max 90%%; Rounded down)\n"
        " - On Clash Win, heal (Clash count x 5) more Sanity (Max 15)\n"
        " - Turn End: If this unit lost Sanity this turn due to its Skill effects, gain +30%% Damage Up next turn\n", c->name, take);

      sleep(1);

      printf("\n%s: \"I don't need some stupid crutch like that 'Shin (心)' crap. I just gotta pull as much power as I can from these tattoos and grit my teeth, and I'll overpower your fancy little tricks no problem.\"\n", c->name);

      sleep(1);
    }
  }

  // The Middle Nursefather - Matthias - Skill 1/2
  if (isId(c->ID, "The Middle Nursefather - Matthias") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

    if (c2->Bleed[0] >= 4) {

    chosenSkill->CoinPowerBoost[0] += 1;

     printf("\n%s at 4+ Bleed Stack (%d), %s gains +1 Coin Power\n", c2->name, c2->Bleed[0], c->name);

    sleep(1);

    }

    if (c->skills[9].active >= 1 && c->skills[2].active > 0) {

      printf("\n%s has 'The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]', inflict 1 Bleed Stack On Coin Hit\n", c->name);

      sleep(1);

    }

  }

  // The Middle Nursefather - Matthias - Skill 3
  if (isId(c->ID, "The Middle Nursefather - Matthias") == 0 && (chosenSkill == &c->skills[2])) {

    if (c2->Bleed[0] >= 3) {

    int gain = c2->Bleed[0]/3;
    if (gain > 3) gain = 3;

    if (gain > 0) {

      chosenSkill->CoinPowerBoost[0] += gain;

     printf("\n%s gains +1 Base Power (%d - Max 3) for every 3 Bleed Stack on target (%d)\n", c->name, gain, c2->Bleed[0]);

      sleep(1);

    }

      if ((c->skills[9].active >= 1) && c->skills[2].active > 0) {

        printf("\n%s has 'The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]', inflict 1 Bleed Stack On Coin Hit\n", c->name);

        sleep(1);

      }

    }

  }

  // The Middle Nursefather - Matthias - Skill 4/5/6/7
  if (isId(c->ID, "The Middle Nursefather - Matthias") == 0 && (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[4] || chosenSkill == &c->skills[5] || chosenSkill == &c->skills[6])) {

      if ((c->skills[9].active >= 1) && c->skills[2].active > 0) {

        printf("\n%s has 'The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]', deal 10 bonus damage On Unbroken Coin Hit\n", c->name);

        sleep(1);

      } else if ((c->skills[9].active < 1) && c->skills[2].active > 0) {
        printf("\n%s has 'The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]', deal 5 bonus damage On Unbroken Coin Hit\n", c->name);

        sleep(1);
      }

  }

  // The Middle Nursefather - Matthias - Skill 7
  if (isId(c->ID, "The Middle Nursefather - Matthias") == 0 && (chosenSkill == &c->skills[7] || chosenSkill == &c->skills[8] || chosenSkill == &c->skills[9] || (chosenSkill == &c->defenseSkill[0] && (c->skills[9].active == 3)))) {

    updateSanity(c, -10);

        printf("\n%s loses 10 Sanity (%d)\n", c->name, c->Sanity);

        sleep(1);

  }

  // ------------------------------------------------------------


  // --------------- Muga Ryōshū ----------------

  // Muga Ryōshū - Passive
  if (isId(c->ID, "Muga Ryōshū") == 0) {

    *tempOffense += 6;

    printf("\n%s gains +6 Offense from 'Tiansha Star's Blade - Arayashiki [天殺星刀阿賴耶識]'\n", c->name);

    sleep(1);

      // Passive 2 & 4: Offense/Defense +6 (Base) + (Muga/10)
      int levelBoost = 6 + (c->Passive / 10);
      *tempOffense += levelBoost;
      *tempDefense += levelBoost;

      // Passive 5: Damage Reduction (Muga self + Sever enemy)%
      float reduction = (float)(c->Passive + c2->skills[0].active);
      if (reduction > 90.0f) reduction = 90.0f;
      chosenSkill->Protection[0] += reduction;

      printf("\n%s gains +%d Offense and Defense, Damage Reduction +%.0f%%\n", c->name, levelBoost, reduction);

    sleep(1);
  }

  // Muga Ryōshū - Final Power Skill 1/2
  if (isId(c->ID, "Muga Ryōshū") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

    int gain = c->Passive/10;
    if (gain > 3) gain = 3;

    if (gain > 0) {

    chosenSkill->FinalPowerBoost[0] += gain;

    printf("\n%s gains +1 Final Power (%d - Max 3) for every 10 Muga [無我] on self (%d)\n", c->name, gain, c->Passive);

    sleep(1);

    }

    gain = c2->Bleed[0]/6;
    if (gain > 3) gain = 3;

    if (gain > 0) {

    chosenSkill->FinalPowerBoost[0] += gain;

    printf("\n%s gains +1 Final Power (%d - Max 3) for every 6 Bleed Stack on target (%d)\n", c->name, gain, c2->Bleed[0]);

    sleep(1);

    }

  }

  // Muga Ryōshū - Final Power Skill 3/4
  if (isId(c->ID, "Muga Ryōshū") == 0 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3] || chosenSkill == &c->skills[4] || chosenSkill == &c->skills[5])) {

    printf("\n%s's Skill: Random chance to delete the earliest Coin on the Skill this unit Clashed with\n", c->name);

    sleep(1);

    }

  // Muga Ryōshū - Final Power Skill 3/4
  if (isId(c->ID, "Muga Ryōshū") == 0 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int gain = c->Passive/10;
    if (gain > 5) gain = 5;

    if (gain > 0) {

    chosenSkill->FinalPowerBoost[0] += gain;

    printf("\n%s gains +1 Final Power (%d - Max 5) for every 10 Muga [無我] on self (%d)\n", c->name, gain, c->Passive);

    sleep(1);

    }

    gain = c2->Bleed[0]/6;
    if (gain > 4) gain = 4;

    if (gain > 0) {

    chosenSkill->FinalPowerBoost[0] += gain;

    printf("\n%s gains +1 Final Power (%d - Max 4) for every 6 Bleed Stack on target (%d)\n", c->name, gain, c2->Bleed[0]);

    sleep(1);

    }

  }

  // Muga Ryōshū - Final Power Skill 5
  if (isId(c->ID, "Muga Ryōshū") == 0 && (chosenSkill == &c->skills[4])) {

    int gain = c->Passive/4;
    if (gain > 12) gain = 12;

    if (gain > 0) {

    chosenSkill->FinalPowerBoost[0] += gain;

    printf("\n%s gains +1 Final Power (%d - Max 12) for every 4 Muga [無我] on self (%d)\n", c->name, gain, c->Passive);

    sleep(1);

    }

  }

  // -------------------- The One Who Grips Faust --------------------

  // The One Who Grips Faust - Passive
  if (isId(c->ID, "The One Who Grips Faust") == 0) {

      // 7. You Must Accept the Pain!: เปลี่ยนสกิลเมื่อตะปูเยอะ
      if (chosenSkill == &c->skills[2] && c->skills[2].active >= 3) {
          chosenSkill = &c->skills[3]; // ใช้ Purify แทน Execution
          printf("\n%s has 3+ Nails, using 'Purify' instead\n", c2->name);

        sleep(1);
      }

      // 5. Gaze: เป้าหมายรับดาเมจแรงขึ้น 20%
      if (c->skills[4].active > 0) {
          c2->ProtectionDown[0] += 20; 
          printf("\n%s takes +20%% damage from Gaze.\n", c2->name);

        sleep(1);
      }
  }

  // The One Who Grips Faust - Skill 1
  if (isId(c->ID, "The One Who Grips Faust") == 0 && chosenSkill == &c->skills[0]) {

    int gain = c2->Bleed[0]/3;
    if (gain > 1) gain = 1;

    if (gain > 0) {

    chosenSkill->CoinPowerBoost[0] += gain;

    printf("\n%s at 3+ Bleed Stack (%d), %s gains +1 Coin Power\n",
       c2->name, c2->Bleed[0], c->name);

    sleep(1);
    }

    gain = c2->Bleed[1]/3;
    if (gain > 1) gain = 1;

    if (gain > 0) {

    chosenSkill->ClashPower[0] += 2;

    printf("\n%s at 3+ Bleed Count (%d), %s gains +2 Clash Power\n",
       c2->name, c2->Bleed[1], c->name);

    sleep(1);
    }
  }

  // The One Who Grips Faust - Skill 2
  if (isId(c->ID, "The One Who Grips Faust") == 0 && chosenSkill == &c->skills[1]) {

    c->skills[6].active += 1;

    printf("\n%s gains 1 Fanatic next turn\n",
       c->name);

    sleep(1);

    int gain = c2->Bleed[0]/6;
    if (gain > 1) gain = 1;

    if (gain > 0) {

    chosenSkill->CoinPowerBoost[0] += gain;

    printf("\n%s at 6+ Bleed Stack (%d), %s gains +1 Coin Power\n",
       c2->name, c2->Bleed[0], c->name);

    sleep(1);
    }

     gain = c2->Bleed[1]/3;
      if (gain > 1) gain = 1;

      if (gain > 0) {

      chosenSkill->ClashPower[0] += 1;

      printf("\n%s at 3+ Bleed Count (%d), %s gains +1 Clash Power\n",
         c2->name, c2->Bleed[1], c->name);

      sleep(1);
      }
  }

  // The One Who Grips Faust - Skill 3
  if (isId(c->ID, "The One Who Grips Faust") == 0 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    c->skills[6].active += 2;
    c->Passive += 2;

    printf("\n%s gains 2 Fanatic this turn and next turn\n",
       c->name);

    sleep(1);

    if (chosenSkill == &c->skills[2]) {

    int gain = (c2->Bleed[0] + c->skills[2].active)/8;
    if (gain > 1) gain = 1;

    if (gain > 0) {

    chosenSkill->CoinPowerBoost[0] += 2;

    printf("\n%s at 8+ (Bleed Stack + Nail Stack) (%d), %s gains +2 Coin Power\n",
       c2->name, (c2->Bleed[0] + c->skills[2].active), c->name);

    sleep(1);
    }

     gain = c2->Bleed[1]/3;
      if (gain > 1) gain = 1;

      if (gain > 0) {

      chosenSkill->ClashPower[0] += 2;

      printf("\n%s at 3+ Bleed Count (%d), %s gains +2 Clash Power\n",
         c2->name, c2->Bleed[1], c->name);

      sleep(1);
      }
    }
  }

  // The One Who Grips Faust - Skill 4
  if (isId(c->ID, "The One Who Grips Faust") == 0 && chosenSkill == &c->skills[3]) {

    int gain = (c2->Bleed[0] + c->skills[2].active)/6;
    if (gain > 2) gain = 2;

    if (gain > 0) {

    chosenSkill->CoinPowerBoost[0] += gain;

    printf("\n%s gains +1 Coin Power (%d - Max 2) for every 6 (Bleed Stack + Nail Stack) (%d)\n",
       c->name, gain, (c2->Bleed[0] + c->skills[2].active));

    sleep(1);
    }

    gain = 2 * (c2->Bleed[0] + c2->Bleed[1]);
    if (gain > 50) gain = 50;

    if (gain > 0) {

    chosenSkill->DamageUp[0] += gain;

    printf("\n%s deals +2%% damage (%d%% - Max 50%%) for every (Bleed Stack + Bleed Count) Stack (%d)\n",
       c->name, gain, (c2->Bleed[0] + c2->Bleed[1]));

    sleep(1);
    }

  }

  // The One Who Grips Faust - Passive
  if (isId(c->ID, "The One Who Grips Faust") == 0) {
      // 2. Fanatic Logic: เพิ่มพลังถ้ามี Fanatic และศัตรูติดตะปู
      if (c->Passive > 0 && c->skills[2].active > 0) {
          chosenSkill->FinalPowerBoost[0] += c->Passive;
          printf("\n%s gains Final Power equal to Fanatic (%d)\n", c->name, c->Passive);

        sleep(1);
      }
  }

  // ------------------------------------------------------------


// -------------------- The House of Spiders: The Index Nursefather Yi Sang --------------------
// The House of Spiders: The Index Nursefather Yi Sang - Imitation of a Life
  if (isId(c->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {

    if (c->skills[3].active == 1) { // Wound-casing Mask
      *tempOffense += 2;
      *tempDefense -= 2;
      printf("\n%s has 'Wound-casing Mask' Offense +2, Defense -2\n", c->name);

      sleep(1);
    } 
    else if (c->skills[3].active == 2) { // Sizzling Wound
      *tempOffense += 3;
      *tempDefense -= 3;
      printf("\n%s has 'Sizzling Wound' Offense +3, Defense -3\n", c->name);

      sleep(1);
    }

    // [Unlock Stage Defense Buffs]
    if (c->Passive == 1) {*tempDefense += 1; printf("\n%s has 'Unlock - I', gains +1 Defense\n", c->name); sleep(1);} // Unlock I
    if (c->Passive == 2) {*tempDefense += 2; printf("\n%s has 'Unlock - II', gains +2 Defense\n", c->name); sleep(1);} // Unlock II
    if (c->Passive >= 3) {
        *tempDefense += 3; // Unlock III

      printf("\n%s has 'Unlock - III', gains +3 Defense\n", c->name);

      sleep(1);

        // --- [Shin (心) - Fate] Effects ---
        // 1. Gain +1 Offense and +1 Defense
        *tempOffense += 1;
        *tempDefense += 1;

      printf("\n%s has 'Shin (心) - Fate'\n - Gains +1 Offense and +1 Defense", c->name);

        // 2. Final Power +1 for every 20% Missing HP (Self + Target) (Max 3)
      // 1. คำนวณเปอร์เซ็นต์ที่หายไปเป็น decimal (0.0 - 1.0) 
      // ต้องคูณ 1.0f เพื่อให้เป็น float ป้องกัน Integer Division
      float selfMissing = (float)(c->MAX_HP - c->HP) / c->MAX_HP;
      float targetMissing = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP;

      // 2. รวมกันแล้วหารด้วย 0.2 (หรือคูณ 5) เพื่อหาว่ามีกี่ "20%"
      // เช่น 0.2 + 0.2 = 0.4 | 0.4 / 0.2 = 2.0
      float missP = (selfMissing + targetMissing) / 0.20f;

      // 3. ปรับค่า Boost
      int Boost = (int)missP; 
      if (Boost > 3) Boost = 3;

      *tempOffense += Boost;
        if (Boost > 0) {printf("\n - Gains +1 Offense for every 20%% (missing HP percentage on target + missing HP percentage on self; rounded down) (%d - Max 3)", Boost);}

        // 3. Deal +1% damage for every 3 Sanity higher than target (Max 15%)
        if (c->Sanity > c2->Sanity) {
            int spDiff = (c->Sanity - c2->Sanity) / 3;
            if (spDiff > 15) spDiff = 15;
            chosenSkill->DamageUp[0] += spDiff;
            printf("\n - If this unit's Sanity higher than the target's, deals +1%% damage for every 3 Sanity difference (%d%% - Max 15%%)",  spDiff);

        }

      printf("\n");

      sleep(1);
    }

    int chosenIdx = -1;
    for(int i=0; i<3; i++) { if(chosenSkill == &c->skills[i]) chosenIdx = i; }

    if (chosenSkill == &c->skills[3]) chosenIdx = 3;

    if (chosenIdx == c->skills[4].active) { // ทำตามคำสั่ง

      // ถ้าเป็น Stage 0 (Prescript I): แค่ "ใช้" (Execute) ก็ถือว่าสำเร็จเลย
      if (c->Passive == 0) {
          c->skills[5].active = 1; // Mark ว่าสำเร็จ
      }

      // --- ส่วนคำนวณขีดจำกัด (Caps) คงไว้ตามเดิมเพื่อสมดุลเกม ---
      int maxGainThisTurn = c->Passive + 2;
      int hermesHardCap = (c->Passive < 2) ? 8 : 9;

      int quotaLeftThisTurn = maxGainThisTurn - c->skills[13].active;
      int spaceLeftInStack = hermesHardCap - c->skills[1].active;

      if (quotaLeftThisTurn < 0) quotaLeftThisTurn = 0;
      if (spaceLeftInStack < 0) spaceLeftInStack = 0;

      // หาค่าที่จะได้รับจริง (ไม่เกินโควต้า และไม่เกินช่องว่างในคลัง)
      int actualGain = 1; 
      if (actualGain > quotaLeftThisTurn) actualGain = quotaLeftThisTurn;
      if (actualGain > spaceLeftInStack) actualGain = spaceLeftInStack;

      if (actualGain > 0) {
          c->skills[1].active += actualGain;
          c->skills[13].active += actualGain;

        // For prescript III Check
        if (c->Passive == 2) {
          c->skills[5].active = 1;
          }

          printf("\n%s gains +%d 'Procuration [Hermes]' (%d)\n", 
                 c->name, actualGain, 
                 c->skills[1].active);
          sleep(1);
      }

        int grace = c->skills[0].active;
        int dmgBoost = (grace >= 9) ? 16 : (grace * 2);
      if (dmgBoost > 0) {
        chosenSkill->DamageUp[0] += dmgBoost; // +2% ต่อ Grace หรือ 20% ถ้าเต็ม 9
        printf("\n%s +2%% damage with Skills marked with 'Mark of the Prescript' for every 'Grace of the Prescript' on self (%d%% - Max 16%%)\n", c->name, dmgBoost);

      sleep(1);
      }
    }

    *tempOffense += (c->skills[0].active / 3); // Offense +1 ทุก 3 Grace

    if ((c->skills[0].active / 3) > 0) { printf("\n%s gains +1 Offense (%d) for every 3 'Grace of the Prescript' (%d)\n", c->name, (c->skills[0].active / 3), c->skills[0].active); sleep(1);}
  }

  // The House of Spiders: The Index Nursefather Yi Sang - Skill Defense Buff
  if (isId(c->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && chosenSkill == &c->defenseSkill[0] && c->Passive >= 2) {

      chosenSkill->CoinPowerBoost[0] += 2;

      printf("\n%s at Unlock - II, gain +2 Coin Power\n", c->name);

      sleep(1);

    }


  // The House of Spiders: The Index Nursefather Yi Sang - Skill 1 Buff
  if (isId(c->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && chosenSkill == &c->skills[0]) {

    if (c->skills[4].active == 0) {

      chosenSkill->ClashPower[0] += 1;
      chosenSkill->DamageUp[0] += 20;

      printf("\n%s's Skill is marked with 'Mark of the Prescript', Clash Power +1 and deal +20%% more damage\n", c->name);

      sleep(1);

    }

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
      float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

      int gain = (missingSelf + missingEnemy) / 10;
      if (gain > 2) gain = 2;

    if (gain > 0) {

      chosenSkill->CoinPowerBoost[0] += gain;

        printf("\n%s gains +1 Coin Power (%d - Max 2) for every 10%% (missing HP percentage on target + missing HP percentage on self)\n", c->name, gain);

      sleep(1);

    }

    int convertvalue = c->Passive - 1;

    if (convertvalue > 0 && c->skills[0].Unbreakable <= c->skills[0].Coins) {

       c->skills[0].Unbreakable = convertvalue;

    printf("\n%s converts Coins equal to (%d - [Unlock stage - 1]) into Unbreakable Coins, beginning with the final Coin\n", c->name, convertvalue);

      sleep(1);

    }

    }

  // The House of Spiders: The Index Nursefather Yi Sang - Skill 2 Buff
  if (isId(c->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && chosenSkill == &c->skills[1]) {

    if (c->skills[4].active == 1) {

      chosenSkill->FinalPowerBoost[0] += 1;

      printf("\n%s's Skill is marked with 'Mark of the Prescript', Final Power +1\n", c->name);

      sleep(1);

    }

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
      float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

      int gain = (missingSelf + missingEnemy) / 10;
      if (gain > 2) gain = 2;

    if (gain > 0) {

      chosenSkill->CoinPowerBoost[0] += gain;

        printf("\n%s gains +1 Coin Power (%d - Max 2) for every 10%% (missing HP percentage on target + missing HP percentage on self)\n", c->name, gain);

      sleep(1);

    }

    int convertvalue = c->Passive;

    if (convertvalue > 0 && c->skills[1].Unbreakable <= c->skills[1].Coins) {

       c->skills[1].Unbreakable = convertvalue;

    printf("\n%s converts Coins equal to (%d - [Unlock stage]) into Unbreakable Coins, beginning with the final Coin\n", c->name, convertvalue);

      sleep(1);

    }

    }

  // The House of Spiders: The Index Nursefather Yi Sang - Skill 3 Buff
  if (isId(c->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && chosenSkill == &c->skills[2]) {

    if (c->skills[4].active == 2) {

      chosenSkill->DamageUp[0] += 20;

      printf("\n%s's Skill is marked with 'Mark of the Prescript', deal +20%% more damage\n", c->name);

      sleep(1);

    }

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
      float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

      int gain = (missingSelf + missingEnemy) / 10;
      if (gain > 2) gain = 2;

    if (gain > 0) {

      chosenSkill->CoinPowerBoost[0] += gain;

        printf("\n%s gains +1 Coin Power (%d - Max 2) for every 10%% (missing HP percentage on target + missing HP percentage on self)\n", c->name, gain);

      sleep(1);

    }

    int convertvalue = c->Passive + 1;

    if (convertvalue > 0 && c->skills[2].Unbreakable <= c->skills[2].Coins) {

       c->skills[2].Unbreakable = convertvalue;

    printf("\n%s converts Coins equal to (%d - [Unlock stage + 1]) into Unbreakable Coins, beginning with the final Coin\n", c->name, convertvalue);

      sleep(1);

    }

    }

  // The House of Spiders: The Index Nursefather Yi Sang - Skill 4 Buff
  if (isId(c->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && chosenSkill == &c->skills[3]) {

    // [Combat Start] สั่งห้ามรับ Hermes ในเทิร์นนี้ (ฝากค่าไว้ใน skills[8].active)
    c->skills[8].active = 1;

    // Logic: Coin Power does not change; instead...
    // ทุกๆ +1 Coin Power เปลี่ยนเป็น +5 Final Power และ +25% Damage
    int netCoinPower = chosenSkill->CoinPowerBoost[0];
    if (netCoinPower > 0) {
        chosenSkill->FinalPowerBoost[0] += (netCoinPower * 5);
        chosenSkill->DamageUp[0] += (netCoinPower * 25.0f);

        // ยกเลิกค่า Coin Power Boost เดิม เพื่อไม่ให้เหรียญมีค่าพลังเพิ่มขึ้นจริงๆ
        chosenSkill->CoinPowerBoost[0] = 0;
        printf("\n%s Converting %d Coin Power to +%d Final Power, +%d%% Damage", c->name, netCoinPower, netCoinPower * 5, netCoinPower * 25);

      sleep(1);
    }

    printf("\n%s's this Skill is not affected by Paralyze\n", c->name);

      sleep(1);

      chosenSkill->ClashPower[0] += 3;

    printf("\n%s gains +3 Clash Power\n", c->name);

      sleep(1);

    if (c->skills[4].active == 2) {

      chosenSkill->DamageUp[0] += 10;

      printf("\n%s's Skill is marked with 'Mark of the Prescript', deal +10%% more damage\n", c->name);

      sleep(1);

    }

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
      float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

      int gain = (missingSelf + missingEnemy) / 15;
      if (gain > 2) gain = 2;

      chosenSkill->FinalPowerBoost[0] += gain;

        printf("\n%s gains +1 Final Power (%d - Max 2) for every 15%% (missing HP percentage on target + missing HP percentage on self)\n", c->name, gain);

    sleep(1);

    }

  // --------------------------------------------------------------------------------




  // ---------------------------- The Middle Little Brother Sinclair ----------------------------

  // The Middle Little Brother Sinclair - Passive Buff Tattoo
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill != &c->skills[0] && c->skills[2].active == 0) {

    c->Passive++;

    printf("\n%s gains 1 Envy Resonance (%d)\n", c->name, c->Passive);

    sleep(1);
  }

  // The Middle Little Brother Sinclair - Envy Resonance offense buff for skill 2 and 3
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && c->Passive >= 2 && (chosenSkill == &c->skills[1] || chosenSkill == &c->skills[2])) {

    int gain = c->Passive / 2;
    if (gain > 3) gain = 3;

    *tempOffense += gain;

      printf("\n%s at 2+ Envy Resonance, gains (Envy Resonance / 2) Offense Level (%d - Max 3, Rounded down)\n", c->name, gain);

    sleep(1);
  }

  // The Middle Little Brother Sinclair - Passive Buff Book
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && c->skills[1].active > 0 && c->skills[2].active == 0) {

    int dmgvalue = c->skills[1].active;
    if (dmgvalue > 30) dmgvalue = 30;

     chosenSkill->DamageUp[0] += dmgvalue;

    printf("\n%s gains %d%% more damage from 'Book of Vengeance [Sinclair]' (Max 30%%)\n", c->name, c->skills[1].active);

    sleep(1);

    if (c->skills[1].active >= 10) {

       chosenSkill->DamageUp[0] += 30;

      printf("\n%s at 10+ 'Book of Vengeance [Sinclair]' Stack, gain 30%% Damage Up\n", c->name);

      sleep(1);
    }

    if (c->skills[1].active >= 20) {

       c->ClashPowerUp[0] += 1;
      c->BasePowerUp[0] += 1;

      printf("\n%s at 20+ 'Book of Vengeance [Sinclair]' Stack, gain 1 Clash Power Up and 1 Base Power Up\n", c->name);

      sleep(1);
    }

    if (c->skills[1].active >= 30) {

       chosenSkill->DamageUp[0] += 50;

      printf("\n%s at 30 'Book of Vengeance [Sinclair]' Stack, gains 50%% Damage Up\n", c->name);

      sleep(1);
    }

  }

  // The Middle Little Brother Sinclair - Skill 1 Buff
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill == &c->skills[0]) {

    int MissingHP = ((c->MAX_HP - c->HP) / c->MAX_HP) * 100;

    int boost = (int)(abs(MissingHP) / 10);
    if (boost > 0) {

      chosenSkill->CoinPowerBoost[0] += 1;

       printf("\n%s at 10%% missing HP, Coin Power +1\n", c->name);

    }

    if (c->Sanity < 45) {

    updateSanity(c, 5);

    printf("\n%s heals 5 Sanity (%d)\n", c->name, c->Sanity);

    } else {

      chosenSkill->ClashPower[0]++;

      printf("\n%s at max Sanity, gain +1 Clash Power instead\n", c->name);

    }

    sleep(1);
  }

  // The Middle Little Brother Sinclair - Skill 2 Buff
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill == &c->skills[1]) {

    int MissingHP = ((c->MAX_HP - c->HP) / c->MAX_HP) * 100;

    int boost = (int)(abs(MissingHP) / 10);
    if (boost > 0) {

      chosenSkill->CoinPowerBoost[0] += 1;

       printf("\n%s at 10%% missing HP, Coin Power +1\n", c->name);

       sleep(1);
    }
  }

  // The Middle Little Brother Sinclair - Skill 3 Buff
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill == &c->skills[2]) {

    int MissingHP = ((c->MAX_HP - c->HP) / c->MAX_HP) * 100;

    int boost = (int)(abs(MissingHP) / 10);
    if (boost > 2) boost = 2;

    if (boost > 0) {

      chosenSkill->CoinPowerBoost[0] += boost;

       printf("\n%s gains +1 Coin Power for every 10%% missing HP (%d - Max 2)\n", c->name, boost);

       sleep(1);
    }
  }

  // The Middle Little Brother Sinclair - Skill 3 buff Book
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill == &c->skills[2]) {

    int dmgvalue = c->skills[1].active;
    if (dmgvalue > 30) dmgvalue = 30;

     chosenSkill->DamageUp[0] += dmgvalue;

    printf("\n%s deals +1%% damage for 'Book of Vengeance [Sinclair]' (%d%% - Max 30%%)\n", c->name, c->skills[1].active);

    sleep(1);
  }

  // The Middle Little Brother Sinclair - Skill 4 Buff
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill == &c->defenseSkill[0]) {

    c->skills[4] = *chosenSkill;
    chosenSkill->skillType = 3;

    c->skills[2].active = 1; // Tell game that using this skill for lose resonance

    int gainvalue = (int)((c->MAX_HP - c->HP) * 0.50f); // 30%% as shield

    c->TempShield += gainvalue;

    printf("\n%s gains 50%% of missing HP as Shield (%d, Rounded down) (Shield %.2f) (Once per Turn)\n", c->name, gainvalue, c->Shield + c->TempShield);

    sleep(1);

       printf("\nThis %s's Skill behavior changes based on the sum of Envy Resonance.:"
        "\n - At 2+ sum of Envy Resonance., use 'Payback with Interest' as Counter Skill"
        "\n - At 4+ sum of Envy Resonance., use 'Write 'em all down' as Counter Skill"
        "\n - At 6+ sum of Envy Resonance., use 'Write 'em all down' as Counter Skill and gain (100%% + 10%% for every excess Envy Resonance) Damage Up"
         "\nEnvy Resonance: %d\n", c->name, c->Passive);

       sleep(1);

    if (c->Passive >= 6) {

      *tempOffense -= chosenSkill->Offense;
      *tempDefense -= chosenSkill->Defense;

      chosenSkill = &c->skills[2];

      chosenSkill->Clashable = 0;
      chosenSkill->skillType = 3;

      chosenSkill->DamageUp[0] += 100 + 10*(c->Passive - 6);

      getEffectiveSkill( c, c2, &c->skills[2], tempOffense, tempDefense);

     } else if (c->Passive >= 4) {

      *tempOffense -= chosenSkill->Offense;
      *tempDefense -= chosenSkill->Defense;

      chosenSkill = &c->skills[2];

      chosenSkill->Clashable = 0;
      chosenSkill->skillType = 3;

      getEffectiveSkill( c, c2, &c->skills[2], tempOffense, tempDefense);

     } else if (c->Passive >= 2) {

      *tempOffense -= chosenSkill->Offense;
      *tempDefense -= chosenSkill->Defense;

      chosenSkill = &c->skills[1];

      chosenSkill->Clashable = 0;
      chosenSkill->skillType = 3;

      getEffectiveSkill( c, c2, &c->skills[1], tempOffense, tempDefense);

     } 
  }

  // ------------------------------------------------------------------------

  // ---------------------------- Binah -----------------------------

  // Binah - Arbiter
  if (isId(c->ID, "Binah") == 0 && !c->Passive) {

    c->DamageUp[0] -= 20;
    c->FinalPowerUp[0] -= 1;

    printf("\n%s's 'Incomplete Arbiter' activated, deals -20%% damage, Final Power -1\n", c->name);

    sleep(1);
  } else if (isId(c->ID, "Binah") == 0 && c->Passive) {

      c->DamageUp[0] += 50;
      c->FinalPowerUp[0] += 2;

      printf("\n%s's 'An Arbiter' activated, gains +50%% damage, Final Power +2\n", c->name);

      sleep(1);
    }

  // Binah - heal Sanity
  if (isId(c->ID, "Binah") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[3])) {

    updateSanity(c, 10);


    printf("\n%s heals 10 Sanity (%d)\n", c->name, c->Sanity);

    sleep(1);
  }

  // Binah - Skill 5 buff
  if (isId(c->ID, "Binah") == 0 && (chosenSkill == &c->skills[4])) {

    chosenSkill->Protection[0] += 30;
    chosenSkill->Protection[1] += 30;

    printf("\n%s takes -30%% damage for this turn and next turn\n", c->name);

    sleep(1);
  }

  // Binah - Fairy buff
  if (isId(c->ID, "Binah") == 0 && c->skills[0].active > 0 && c->Passive) {

    int boost = c->skills[0].active * 20;

    chosenSkill->DamageUp[0] += boost;
    chosenSkill->BasePowerBoost[0] += c->skills[0].active*10;

    printf("\n%s deals +20%% damage(%d%%) and +10 Base Power(%d) for every Fairy on enemy (%d)\n", c->name, boost, c->skills[0].active*10, c->skills[0].active);

    sleep(1);
  }

  // -----------------------------------------------

  // -------------------------- Jia Qiu -----------------------------

 // Jia Qiu - heal Sanity on mao or zilu
  if (isId(c->ID, "Jia Qiu") == 0 &&
      (chosenSkill == &c->skills[12] || chosenSkill == &c->skills[4] || chosenSkill == &c->skills[11])) {

    updateSanity(c, 10);


    printf("\n%s heals 10 Sanity (%d)\n", c->name, c->Sanity);

    sleep(1);
  }

  // Jia Qiu S15 Nerf
  if (isId(c->ID, "Jia Qiu") == 0 &&
      (chosenSkill == &c->skills[15])) {

    if (isId(c2->ID, "Hong lu:The Lord of Hongyuan") == 0) {

    printf("\n%s lost 2 Clash Power(%d) for every Uncompromising Imposition on target(%d)\n", c->name, 2 * c->skills[15].active, c->skills[15].active);
    } else {
      printf("\n%s lost 2 Clash Power(%d) for every Dialogues on target (%d)\n", c->name, 2 * c->skills[15].active, c->skills[15].active);
    }

    chosenSkill->ClashPower[0] -= 2 * c->skills[15].active;

    sleep(1);
  }

// ------------------------------------------------------------------------

  // ----------------------- Wild hunt ------------------------------
  // Heathcliff: Wild Hunt – skill 3 -> skill 4 if HP ≤ half
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
      chosenSkill == &c->skills[2] && c->skills[0].active > 0) {

    printf("\nWhen activated 'Dullahan', %s switching '%s' to Skill '%s'\n", c->name,
           chosenSkill->name, c->skills[3].name);

    chosenSkill = &c->skills[3];

    sleep(1);
  }

  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
      chosenSkill == &c->defenseSkill[0] && c->Sanity >= 15 && c->skills[0].active > 0) {

    printf("\n%s activated 'Dullahan', and equipped '%s', using '%s' as Clashable Counter instead\n", c->name,
           chosenSkill->name, c->skills[3].name);

    chosenSkill = &c->skills[3];
    chosenSkill->skillType = 5;

    sleep(1);
  }

  // Wild hunt – Buff
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 && c->skills[0].active > 0) {

    *tempOffense += 3;
    *tempDefense -= 3;

    printf("\n%s mounted 'Dullahan', Offense +3, Defense -3\n",
           c->name);

    sleep(1);

    printf("\n%s: \"Dullahan! Time to ride for death.\"\n", c->name);

    sleep(1);

  }

  // Heathcliff:Wild Hunt – skill 4 lose sanity
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
      (chosenSkill == &c->skills[3])) {

      updateSanity(c, -(15));

    if (c->Sanity <= -45) c->SanityFreezeTurns = 0;


    printf("\n%s loses 15 Sanity (%d)\n",
       c->name, c->Sanity);

    sleep(1);
  }


  // Heathcliff:Wild Hunt – gain coffin
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
      (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[2])) {

    int gain;

    if (chosenSkill == &c->skills[2]) {
      gain = 1;
    } else {
      gain = 2;
    }

    c->Passive += gain;
    if (c->Passive > 10) c->Passive = 10;

    printf("\n%s gains %d Coffin (%d - Max 10)\n",
       c->name, gain, c->Passive);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – Damage coffin
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 && c->Passive >= 3) {

    int gain;

    gain = ((c->Passive/3) * 20);
    c->DamageUp[0] += gain;

    printf("\n%s deals 20%% Damage Up (%d%%) for every 3 Coffin (%d)\n",
       c->name, gain, c->Passive);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – Clash power coffin
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 && c->Passive >= 5) {

    int gain;

    gain = c->Passive/5;
    chosenSkill->ClashPower[0] += gain;

    printf("\n%s gains 1 Clash Power(%d) for every 5 Coffin (%d)\n",
       c->name, gain, c->Passive);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[1]) && c->Passive >= 4) {

    int gain = c->Passive/4;

    chosenSkill->CoinPowerBoost[0] += gain;

    printf("\n%s gains 1 Coin Power(%d) for every 4 Coffin (%d)\n",
       c->name, gain, c->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0] || chosenSkill == &c->defenseSkill[0]) && abs(c->Sanity - c2->Sanity) >= 10) {

    int gain = abs(c->Sanity - c2->Sanity)/10;
    if (gain > 2) gain = 2;

    chosenSkill->CoinPowerBoost[0] += gain;

    printf("\n%s gains 1 Coin Power for every 10 Sanity difference (%d - Max 2)\n",
       c->name, gain);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0]) && c->Passive >= 3) {

    chosenSkill->ClashPower[0] += 1;

    printf("\n%s at 3+ Coffin(%d), Clash Power +1\n",
       c->name, c->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1/2
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1] || chosenSkill == &c->defenseSkill[0]) && abs(c->Sanity - c2->Sanity) >= 10) {

    chosenSkill->ClashPower[0] += 1;

    printf("\n%s at 10+ Sanity difference, Clash Power +1\n",
       c->name);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 3
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[2]) && abs(c->Sanity - c2->Sanity) >= 15) {

    int gain = abs(c->Sanity - c2->Sanity)/15;
    if (gain > 4) gain = 4;

    chosenSkill->CoinPowerBoost[0] += gain;

    printf("\n%s gains 1 Coin Power for every 15 Sanity difference (%d - Max 4)\n",
       c->name, gain);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 4
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[3]) && abs(45 - c->Sanity) >= 20) {

    int gain = abs(45 - c->Sanity)/20;
    if (gain > 4) gain = 4;

    chosenSkill->BasePowerBoost[0] += gain;

    printf("\n%s gains 1 Base Power for every 20 Sanity further from 45 (%d - Max 4)\n",
       c->name, gain);

      sleep(1);
  }

  //-------------------------------------------------

// ---------------------- Meursault:Blade Lineage Mentor ---------------------

  //        Meursault:Blade Lineage Mentor - Remembrance
  if ((isId(c->ID, "Meursault:Blade Lineage Mentor") == 0 &&
       c->HP <= c->MAX_HP * 0.6)) {

      printf("\n%s HP at 60%% or less HP, Apply 'Remembrance' buff on self, Gains buff at 5+ Poise Stack or 7+ Poise Stack on self (%d)\n",c->name, c->Poise[0]);

    int PowerBuff;
    int ProtectionBuff;
      int DamageBuff;

      if (c->Poise[0] >= 7 && chosenSkill != &c->skills[2]) {

        PowerBuff = (4/chosenSkill->Coins) < 1 ? 1 : (4/chosenSkill->Coins);
        DamageBuff = (50/chosenSkill->Coins);

        chosenSkill->CoinPowerBoost[0] += PowerBuff;
        chosenSkill->CriticalDamageUp[0] += DamageBuff;

        printf("At 7+ Poise Stack (%d), gain +%d Coin Power and gain +%d%% damage on Critical Hit\n", c->Poise[0], PowerBuff, DamageBuff);

        sleep(1);
      }

    else if (c->Poise[0] >= 5 && chosenSkill != &c->skills[2]) {

      PowerBuff = (3/chosenSkill->Coins) < 1 ? 1 : (3/chosenSkill->Coins);
      DamageBuff = (30/chosenSkill->Coins);

      chosenSkill->CoinPowerBoost[0] += PowerBuff;
      chosenSkill->CriticalDamageUp[0] += DamageBuff;

      printf("At 5+ Poise Stack (%d), gain +%d Coin Power and gain +%d%% damage on Critical Hit\n", c->Poise[0], PowerBuff, DamageBuff);

      sleep(1);
    } else if (c->Poise[0] >= 7 && chosenSkill == &c->skills[2]) {

      ProtectionBuff = 50;
          DamageBuff = 100;

          c->ProtectionUp[0] += ProtectionBuff;
          chosenSkill->CriticalDamageUp[0] += DamageBuff;

          printf("At 7+ Poise Stack (%d), take -%d%% damage and gain +%d%% damage on Critical Hit\n", c->Poise[0], ProtectionBuff, DamageBuff);

          sleep(1);
        }

      else if (c->Poise[0] >= 5 && chosenSkill == &c->skills[2]) {

          ProtectionBuff = 50;
        DamageBuff = 50;

          c->ProtectionUp[0] += ProtectionBuff;
        chosenSkill->CriticalDamageUp[0] += DamageBuff;

        printf("At 5+ Poise Stack (%d), take -%d%% damage and gain +%d%% damage on Critical Hit\n", c->Poise[0], ProtectionBuff, DamageBuff);

        sleep(1);
      } 

      printf("\n%s: \"If you will cut... then wager your life on it.\"\n", c->name);

    }

  //        Meursault:Blade Lineage Mentor - Skill 1 buff
  if ((isId(c->ID, "Meursault:Blade Lineage Mentor") == 0 && chosenSkill == &c->skills[0])) {

    int boost = c->Poise[0] / 5;
    if (boost > 0) {

    chosenSkill->CoinPowerBoost[0] += 1;

      printf("\n%s at 5+ Poise Stack on self (%d), Coin Power +1\n", c->name, c->Poise[0]);

      sleep(1);
    }

  }

  //        Meursault:Blade Lineage Mentor - Skill 2 buff
  if ((isId(c->ID, "Meursault:Blade Lineage Mentor") == 0 && chosenSkill == &c->skills[1])) {

    int boost = c->Poise[0] / 7;
    if (boost > 0) {

    chosenSkill->CoinPowerBoost[0] += 1;

      printf("\n%s at 7+ Poise Stack on self (%d), Coin Power +1\n", c->name, c->Poise[0]);

      sleep(1);
    }

  }

  //        Meursault:Blade Lineage Mentor - Skill 1 Gain
  if ((isId(c->ID, "Meursault:Blade Lineage Mentor") == 0 && chosenSkill == &c->skills[0])) {

    int boost = 2;
    if (boost > 0) {

    c->Poise[1] += boost;

      printf("\n%s gains +%d Poise Count (%d)\n", c->name, boost, c->Poise[1]);

      sleep(1);
    }

  }

  //        Meursault:Blade Lineage Mentor - Skill 2 Gain
  if ((isId(c->ID, "Meursault:Blade Lineage Mentor") == 0 && chosenSkill == &c->skills[1])) {

    int boost = 3;
    if (boost > 0) {

      c->Poise[1] += boost;

      printf("\n%s gains +%d Poise Count (%d)\n", c->name, boost, c->Poise[1]);

      sleep(1);
    }

  }

  // --------------------------------------


  // ----------------------- Meursault:The Thumb ---------------------

  // Meursault: The Thumb – skill 3 -> skill 4 if HP ≤ 65
  if (isId(c->ID, "Meursault:The Thumb") == 0 &&
      chosenSkill == &c->skills[2] && c->skills[3].active && c->Passive > 0) {

    printf("\n%s at 1+ Savage Tigermark Round, switching '%s' to Skill '%s'\n", c->name,
           chosenSkill->name, c->skills[3].name);

    chosenSkill = &c->skills[3];

    sleep(1);
  }

  // Meursault: The Thumb – S3-1 Unbreakable
  if (isId(c->ID, "Meursault:The Thumb") == 0 && chosenSkill == &c->skills[2] && c->Passive >= 1 && !c->skills[3].active && c->skills[2].active >= (c->defenseSkill[2].active/4)) {

    printf("\n%s at 1+ Tigermark Round and %d+ Tigermark Round spent, convert all Coins of 'Tanglecleaver' into Unbreakable Coins\n", c->name, c->defenseSkill[2].active/4);

    chosenSkill->Unbreakable = chosenSkill->Coins;

    sleep(1);
  }

  // Meursault: The Thumb – all Unbreakable
  if (isId(c->ID, "Meursault:The Thumb") == 0 && c->Passive >= 1 && c->skills[3].active && chosenSkill->skillType == 0) {

    printf("\n%s at 1+ Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins\n", c->name);

     chosenSkill->Unbreakable = chosenSkill->Coins;

    sleep(1);
  }

  // Meursault: The Thumb – Overheat
  if (isId(c->ID, "Meursault:The Thumb") == 0 && c->Passive <= 0 && c->skills[3].active) {

    if (chosenSkill->skillType == 0) {

    printf("\n%s at 0 Savage Tigermark Round, convert all Coins of this unit's Attack Skills that spend 'Unique Ammo' to Unbreakable Coins and Gain 'Overheat'\n", c->name);

      if (chosenSkill == &c->skills[0]) {
     chosenSkill->Unbreakable = 1;
    }

    if (chosenSkill == &c->skills[1]) {
     chosenSkill->Unbreakable = 2;
    }

  if (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3]) {
   chosenSkill->Unbreakable = 3;
  }

    sleep(1);

    }

    int loseClashpower = c->skills[2].active / ((c->defenseSkill[2].active + c->defenseSkill[3].active) / 5);
    if (loseClashpower > 5) loseClashpower = 5;

    chosenSkill->ClashPower[0] -= loseClashpower;

    printf("\nOverheat: Min & Max Speed +2, Attack Skills Lose (cumulative number of Tigermark Rounds & Savage Tigermark Rounds spent / (cumulative number of Tigermark Rounds & Savage Tigermark Rounds Gained/5 (%d))) Clash Power (%d - Max 5); however, gain the following effects (cumulative):\n", (c->defenseSkill[2].active + c->defenseSkill[3].active) / 5, loseClashpower);

    printf(" - Cumulative Rounds spent: %d\n", c->skills[2].active);

       float missing = (c->MAX_HP - c->HP) / c->MAX_HP; // fraction of HP missing (0.0 - 1.0)
        int SkillUp = (int)(missing / 0.10f) * 10;  // 10% for every 10%
        if (SkillUp > 50) SkillUp = 50;      // cap at 50%

      chosenSkill->Protection[0] += SkillUp;

      printf(" - %d+ Rounds spent: Take 10%% less damage for every 10%% missing HP on self at Turn Start (%d%% - Max 50%%)\n", c->defenseSkill[3].active, SkillUp);

      printf(" - %d+ Rounds spent: On Clash Lose, Unbreakable Coins of this unit's Attack Skills deal +(75 + missing HP percentage on self)%% damage (Max 150%%)\n", (c->defenseSkill[3].active + (c->defenseSkill[3].active/2)));
      
      printf(" - %d+ Rounds spent: Deal +(HP percentage difference)%% damage against targets with higher remaining HP percentage than this unit (Max 50%%)\n", (c->defenseSkill[3].active + c->defenseSkill[3].active));

    sleep(1);

  }

  // Meursault:The Thumb - Skill 1 and 2 or defskill Final Power
  if (isId(c->ID, "Meursault:The Thumb") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1] || chosenSkill == &c->defenseSkill[0])) {

    int Buff = c2->Burn[0] + c2->Tremor[0];
    int FinalPowerbuff = Buff/4;
    int Max = 3;

    if (chosenSkill == &c->skills[1]) Max = 4;
    if (chosenSkill == &c->defenseSkill[0]) Max = 2;

     if (FinalPowerbuff > Max) FinalPowerbuff = Max;

    if (FinalPowerbuff > 0) {

     chosenSkill->FinalPowerBoost[0] += FinalPowerbuff;

      printf("\n%s gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max %d)\n", c->name, c2->Burn[0], c2->Tremor[0], FinalPowerbuff, Max);

    sleep(1);
    }
  }

  // Meursault:The Thumb - Skill def clash power
  if (isId(c->ID, "Meursault:The Thumb") == 0 && c->Passive <= 0 && (chosenSkill == &c->defenseSkill[0])) {

     chosenSkill->ClashPower[0] += 2;

      printf("\n%s at 0 'Tigermark Round' or 'Savage Tigermark Round', gains +2 Clash Power\n", c->name);

    sleep(1);
  }

  // Meursault:The Thumb - Skill 3 and 4 coin power
  if (isId(c->ID, "Meursault:The Thumb") == 0 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int Buff = c2->Burn[0] + c2->Tremor[0];
    int CoinPowerbuff = Buff/8;
     if (CoinPowerbuff > 2) CoinPowerbuff = 2;

    if (CoinPowerbuff > 0) {

     chosenSkill->CoinPowerBoost[0] += CoinPowerbuff;

      printf("\n%s gains +1 Coin Power for every 8 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 2)\n", c->name, c2->Burn[0], c2->Tremor[0], CoinPowerbuff);

    sleep(1);
    }
  }

  // --------------------------------------------

  // Shin buffs (temporary, print once per skill selection)
  if ((isId(c->ID, "Meursault:The Thumb") == 0 &&
       c->skills[3].active) ||
      (isId(c->ID, "Lei heng") == 0 && c->skills[4].active == 1 && c->skills[0].active != 3)) {

    if (isId(c->ID, "Meursault:The Thumb") == 0 && c->skills[2].active < c->defenseSkill[3].active) {

      printf("\n%s 'Tiantui Star [天退星]' activated!\n" 
          " - Max & Min Speed +1\n", c->name);

      if (c->Speed - c2->Speed >= 3) {
        float DamageBuff = (2.5*((int)(abs(c2->Speed - c->Speed)))) > 20 ? 20 : (2.5*(int)(abs(c2->Speed - c->Speed)));
        c->DamageUp[0] += DamageBuff;
        printf(" - If this unit's Speed is faster than the target's by 3 or more, deal +(Speed difference x 2.5)%% damage (%.1f%% - Max 20%%)\n", DamageBuff);
      }

        printf(" - Inflict +1 more Tremor Stack and Tremor Count with this unit's Skills\n"
          " - At %d+ (sum of Tigermark Round and Savage Tigermark Round spent (%d)) activate 'Shin (心) - Tiantui Star [天退星]' instead\n", c->defenseSkill[3].active, c->skills[2].active);

    } else if (isId(c->ID, "Meursault:The Thumb") == 0 && c->skills[2].active >= c->defenseSkill[3].active) {


      printf("\n%s at %d+ (sum of Tigermark Round and Savage Tigermark Round spent), 'Shin (心) - Tiantui Star [天退星]' activated!\n"
          " - Max & Min Speed +3\n", c->name, c->defenseSkill[3].active);

        if (c->Speed - c2->Speed >= 3) {
        float DamageBuff = (5*((int)(abs(c2->Speed - c->Speed)))) > 40 ? 40 : (5*(int)(abs(c2->Speed - c->Speed)));
        c->DamageUp[0] += DamageBuff;
          printf(" - If this unit's Speed is faster than the target's by 3 or more, deal +(Speed difference x 5)%% damage (%.1f%% - Max 40%%)\n", DamageBuff);
        }

      printf(" - Inflict +2 more Tremor Stack and Tremor Count with this unit's Skills\n");

    } else {

      float missing = (c->MAX_HP - c->HP) / (c->MAX_HP * 0.2); // fraction of HP missing (0.0 - 1.0)
      int SkillDamageUp = (int)(missing) * 10;  // 10% for every 20%
      if (SkillDamageUp > 30) SkillDamageUp = 30;      // cap at 30%

      int SkillFinalPowerBoost = (int)(missing); // +1 for every 20%
      if (SkillFinalPowerBoost > 3) SkillFinalPowerBoost = 3; // cap at 3

      c->DamageUp[0] += SkillDamageUp;
      c->FinalPowerUp[0] += SkillFinalPowerBoost;

      float DamageBuff = 0;
      if (c->Speed - c2->Speed >= 3) {
        DamageBuff = (5*((int)(abs(c2->Speed - c->Speed)))) > 40 ? 40 : (5*(int)(abs(c2->Speed - c->Speed)));
        c->DamageUp[0] += DamageBuff;
      }

      printf(
        "\n%s's 'Tiantui Star [天退星]' activated!\n"
        " - Gain 10%% more damage (%d%%) and +1 Final Power (%d) for every 20%% HP missing (Max 3 each)\n"
        " - Inflict +2 more Tremor Stack with this unit's Skills\n"
          " - If this unit's Speed is faster than the target's by 3 or more, deal +(Speed difference x 5)%% damage (%.0f - Max 20%%)\n"
          " - Convert the final Coin of 'Triple Slash - Blast' to an Unbreakable Coin\n"
          " - This buff changes into 'Shin (心) - Tiantui Star [天退星]' when using a specific pattern\n", 
        c->name, SkillDamageUp, SkillFinalPowerBoost, DamageBuff);
      sleep(1);

      printf("\n%s: \"That's more like it. Y'all are firin' me up!\"\n", c->name);

    }

    sleep(1);
  }



  // ------------------ The House of Spiders: The Thumb Nursefather Rodion ------------------

  // The House of Spiders: The Thumb Nursefather Rodion - Shin
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && c->skills[13].active == 1) {

      printf("\n%s has 'Shin (心) - Disgrace'\n"
          " - Min & Max Speed +1\n"
          " - Base Skills inflict +1 more Tremor Stack and Burn Stack and Count\n"
        " - Deal +3%% damage for every 3 Poise Stack on self (Max 15%%)\n", c->name);

    sleep(1);
    
  }

  // The House of Spiders: The Thumb Nursefather Rodion - Skill 2-1 to 2-2
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (chosenSkill == &c->skills[2]) && c->skills[10].active <= 0) {

     chosenSkill = &c->skills[3];

      printf("\n%s does not have Acceleration Round, activate as 'Sezionatura di Cervo'\n", c->name);

    sleep(1);

    inflictStatus(c->Poise, 0, 4, 0, 99, 0, 99);
    printf("\n%s gains +4 Poise Count (%d)\n", c->name, c->Poise[1]);

    sleep(1);
  }

  // The House of Spiders: The Thumb Nursefather Rodion - Skill 2-2
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (chosenSkill == &c->skills[3]) && c->skills[11].active == 1) {

     chosenSkill->FinalPowerBoost[0] += 2;
    chosenSkill->UnbreakableUp[0] = chosenSkill->Coins;

      printf("\n%s in Eye of Precognition - Overheat state, gain Final Power +2 and convert all Coins into Unbreakable Coins\n", c->name);

    sleep(1);
  }

  // The House of Spiders: The Thumb Nursefather Rodion - Skill Defense 1 lose
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (chosenSkill == &c->defenseSkill[0]) && c->skills[11].active == 0) {

    int lost = 4;

    int canLose = 10 - c->skills[6].active; // โควต้าที่เหลือในเทิร์นนี้

    int actualLoss = (lost > canLose) ? canLose : lost;
    if (actualLoss > c->Passive) actualLoss = c->Passive;

      c->Passive -= actualLoss;
      c->skills[6].active += actualLoss; // บันทึกว่าเสียไปเท่าไหร่แล้วในเทิร์นนี้

      printf("\n%s loses 4 Eye of Precognition Stack on self (%d)\n", c->name, c->Passive);

    sleep(1);

    if (c->Passive <= 0 && c->skills[11].active == 0) {
        c->skills[11].active = 1; // ติด Overheat

      printf("\n%s converts 'Eye of Precognition' into 'Eye of Precognition - Overheat'\n", c->name);

      sleep(1);
    }
    }

  // The House of Spiders: The Thumb Nursefather Rodion - Skill 1-1/2-1/2-2/3-1 Buff
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3] || chosenSkill == &c->skills[4])) {

    int Buff = (c2->Burn[0] + c2->Tremor[0])/6;
     if (Buff > 2) Buff = 2;

    if (Buff > 0) {

     chosenSkill->CoinPowerBoost[0] += Buff;

      printf("\n%s gains +1 Coin Power for every 6 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 2)\n", c->name, c2->Burn[0], c2->Tremor[0], Buff);

    sleep(1);
    }
  }

  // The House of Spiders: The Thumb Nursefather Rodion - Skill 1-1/1-2/2-1/3-1 Buff
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1] || chosenSkill == &c->skills[2] || chosenSkill == &c->skills[4])) {

    int Buff = (c->Poise[0])/6;

    if (Buff > 0) {

     chosenSkill->FinalPowerBoost[0] += 1;

      printf("\n%s At 6+ Poise Stack (%d), Final Power +1\n", c->name, c->Poise[0]);

    sleep(1);
    }
  }

  // The House of Spiders: The Thumb Nursefather Rodion - Defense Skill 1 Buff
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (chosenSkill == &c->defenseSkill[0])) {

    int Buff = (c2->Burn[0] + c2->Tremor[0])/4;
     if (Buff > 2) Buff = 2;

    if (Buff > 0) {

     chosenSkill->FinalPowerBoost[0] += Buff;

      printf("\n%s gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 2)\n", c->name, c2->Burn[0], c2->Tremor[0], Buff);

    sleep(1);
    }

  }

  // The House of Spiders: The Thumb Nursefather Rodion - Skill 3-2 Buff
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (chosenSkill == &c->skills[5])) {

     chosenSkill->CoinPowerBoost[0] += 1;

      printf("\n%s gains +1 Coin Power against enemy with Game Target\n", c->name);

    sleep(1);

    if (c->skills[10].active > 0) {

    chosenSkill->CoinPowerBoost[0] += 1;

      printf("\n%s has 1+ Acceleration Round, Coin Power +1\n", c->name);

    sleep(1);
    }

    int Buff = (c2->Burn[0] + c2->Tremor[0])/4;
     if (Buff > 5) Buff = 5;

    if (Buff > 0) {

     chosenSkill->FinalPowerBoost[0] += Buff;

      printf("\n%s gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 5)\n", c->name, c2->Burn[0], c2->Tremor[0], Buff);

    sleep(1);
    }
    
  }

  // The House of Spiders: The Thumb Nursefather Rodion - Overheated buff
  if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && c->skills[11].active == 1) {

    *tempDefense -= 3;
    *tempOffense += 3;

      printf("\n%s gains +3 Offense Level and -3 Defense Level from 'Eye of Precognition - Overheat' on self\n", c->name);

    sleep(1);
    }

   // The House of Spiders: The Thumb Nursefather Rodion - Skill 3-2 Passive Buff
    if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (chosenSkill == &c->skills[4] || chosenSkill == &c->skills[5]) && c->skills[12].active >= 5) {

      printf("\nThis %s's Skill at sum of Tremor Burst in this Encounter equal 5 or more while using, if this unit has 1+ Acceleration Round, Skill 5 and 6 gains Coin Power +1 and deals +25%% damage\n", c->name);

    }

  // ------------------------------------------------------



 // ---------------------------- Lei heng -----------------------------
    if ((isId(c->ID, "Lei heng") == 0 && c->skills[4].active == 1 && c->skills[0].active == 3) || (isId(c->ID, "Lei heng") == 0 && isId(c2->ID, "Muga Ryōshū") == 0)) {

    if (isId(c2->ID, "Muga Ryōshū") == 0) {
      c->skills[4].active = 1;
        c->skills[0].active = 3;
    }

    float missing = (c->MAX_HP - c->HP) / (c->MAX_HP * 0.15); // fraction of HP missing (0.0 - 1.0)
    int SkillDamageUp = (int)(missing) * 10;  // 10% for every 15%
    if (SkillDamageUp > 50) SkillDamageUp = 50;      // cap at 50%

    int SkillFinalPowerBoost = (int)(missing); // +1 for every 15%
    if (SkillFinalPowerBoost > 5) SkillFinalPowerBoost = 5; // cap at 3

    c->DamageUp[0] += SkillDamageUp;
    c->FinalPowerUp[0] += SkillFinalPowerBoost;

      float DamageBuff = 0;
      if (c->Speed - c2->Speed >= 3) {
        DamageBuff = (5*((int)(abs(c2->Speed - c->Speed)))) > 40 ? 40 : (5*(int)(abs(c2->Speed - c->Speed)));
        c->DamageUp[0] += DamageBuff;
      }

    printf("\n%s's 'Shin (心) - Tiantui Star [天退星]' activated!\n"
      " - Gain 10%% more damage (%d%%) and +1 Final Power (%d) for every 15%% HP missing (Max 5 each) \n"
      " - Inflict +3 more Tremor Stack and +1 more Tremor Count with this unit's Skills\n"
      " - If this unit's Speed is faster than the target's by 3 or more, deal +(Speed difference x 5)%% damage (%.0f - Max 20%%)\n"
      " - Convert the final Coins of 'Double Slash - Blast' and 'Triple Slash - Blast' to Unbreakable Coins\n",
     c->name, SkillDamageUp, SkillFinalPowerBoost, DamageBuff);

    sleep(1);

     printf("\n%s: \"That's more like it. Y'all are firin' me up!\"\n", c->name);

  }

  // Lei heng – heal Sanity Passive and Buff dmg Passive
  if (isId(c->ID, "Lei heng") == 0 && c->Sanity > -45) {

        int randombuff = (c->Speed / 3) * 10;
    if (randombuff > 50) randombuff = 50;

          if (randombuff > 0) {

        c->DamageUp[0] += randombuff;

        printf("\n%s deals +%d%% damage\n", c->name, randombuff);

          }

        // heal Sanity Passive

        if (c->Sanity <= 15) {

    int randomheal = c->Speed;

    if (c->Sanity < 0) randomheal *= 2;

    updateSanity(c, randomheal);


    printf("\n%s heals %d Sanity (%d)\n", c->name, randomheal, c->Sanity);

        }

    sleep(1);

  }

  // Lei heng – Prey active: apply Clash Power -5, -50% atk dmg, -75% dmg taken
  if (isId(c2->ID, "Lei heng") == 0 && c2->skills[6].active == 1) {
    chosenSkill->ClashPower[0] -= 3;
  }

  // Lei heng – skill 3 -> skill 6 if HP ≤ 40%
  if (isId(c->ID, "Lei heng") == 0 && c->HP < c->MAX_HP * 0.4 &&
      (chosenSkill == &c->skills[2])) {

    chosenSkill = &c->skills[4];

    sleep(1);

  }

  // Lei heng – skill 5 buff
  if (isId(c->ID, "Lei heng") == 0 && (chosenSkill == &c->defenseSkill[0])) {

    c->DamageUp[1] += 10;

    printf("\n%s gains 10%% more damage next turn\n", c->name);

    sleep(1);
  }

  // Lei heng – inner strength skill buff no blast skill 1
  if (isId(c->ID, "Lei heng") == 0 &&
    (chosenSkill == &c->skills[0]) && (strstr(c->skills[0].name, "Blast") == NULL)) {

    int Boost = (c2->Burn[0] + c2->Tremor[0])/4;
    if (Boost > 2) Boost = 2;
      chosenSkill->FinalPowerBoost[0] += Boost;
    if (Boost > 0) {
      printf("\n%s gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 2)\n", c->name, c2->Burn[0], c2->Tremor[0], Boost);

      sleep(1);

    }

  }  // Lei heng – inner strength skill buff blast skill 1
  else if (isId(c->ID, "Lei heng") == 0 &&
     (chosenSkill == &c->skills[0]) && (strstr(c->skills[0].name, "Blast") != NULL)) {

  int Boost = (c2->Burn[0] + c2->Tremor[0])/4;
  if (Boost > 4) Boost = 4;
    chosenSkill->FinalPowerBoost[0] += Boost;
  if (Boost > 0) {
    printf("\n%s gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 4)\n", c->name, c2->Burn[0], c2->Tremor[0], Boost);

    sleep(1);

  }

  }

    // Lei heng – inner strength skill buff no blast skill 2
    if (isId(c->ID, "Lei heng") == 0 &&
      (chosenSkill == &c->skills[1]) && (strstr(c->skills[1].name, "Blast") == NULL)) {

      int Boost = (c2->Burn[0] + c2->Tremor[0])/4;
      if (Boost > 2) Boost = 2;
        chosenSkill->FinalPowerBoost[0] += Boost;
      if (Boost > 0) {
        printf("\n%s gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 2)\n", c->name, c2->Burn[0], c2->Tremor[0], Boost);

        sleep(1);
      }

    }  // Lei heng – inner strength skill buff blast skill 2
    else if (isId(c->ID, "Lei heng") == 0 &&
       (chosenSkill == &c->skills[1]) && (strstr(c->skills[1].name, "Blast") != NULL)) {

    int Boost = (c2->Burn[0] + c2->Tremor[0])/4;
    if (Boost > 4) Boost = 4;
      chosenSkill->FinalPowerBoost[0] += Boost;
    if (Boost > 0) {
      printf("\n%s gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 4)\n", c->name, c2->Burn[0], c2->Tremor[0], Boost);

      sleep(1);

    }

    }

      // Lei heng – inner strength skill buff skill 4
      if (isId(c->ID, "Lei heng") == 0 &&
        (chosenSkill == &c->skills[3])) {

        int Boost = (c2->Burn[0] + c2->Tremor[0])/6;
        if (Boost > 2) Boost = 2;
          chosenSkill->CoinPowerBoost[0] += Boost;
        if (Boost > 0) {
          printf("\n%s gains +1 Coin Power for 6 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 2)\n", c->name, c2->Burn[0], c2->Tremor[0], Boost);

          sleep(1);

        }

      }

  // Lei heng – Consumes inner strength
  if (isId(c->ID, "Lei heng") == 0 &&
    (chosenSkill == &c->skills[2])) {

    printf("\n%s consumes all Inner Strength [底力] Stack on self (%d)\n",
      c->name, c->Passive);

    if (c->Passive >= 10) {

      int Boost = c->Passive / 10;
      if (Boost > 2) Boost = 2;
        chosenSkill->AttackPowerBoost[0] += Boost;

      printf(" - At 10 consumed, gains +1 Attack Power for every 10 Stack consumed (%d - Max 2)\n", Boost);
    }

    int Boost = (c2->Burn[0] + c2->Tremor[0])/4;
    if (Boost > 5) Boost = 5;
      chosenSkill->FinalPowerBoost[0] += Boost;
    if (Boost > 0 && c->Passive >= 25) {
      printf(" - At 25 consumed, gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 5)\n", c2->Burn[0], c2->Tremor[0], Boost);

    }

    sleep(1);
    
    c->skills[3].active = c->Passive;
    c->Passive = 0;

  }  // Lei heng – Consumes Extreme strength
  else if (isId(c->ID, "Lei heng") == 0 &&
      (chosenSkill == &c->skills[4])) {

      printf("\n%s consumes all Extreme Strength [底力] Stack on self (%d)\n",
        c->name, c->Passive);

    if (c->Passive >= 10) {

      int Boost = c->Passive / 10;
      if (Boost > 4) Boost = 4;
        chosenSkill->AttackPowerBoost[0] += Boost;

      printf(" - At 10 consumed, gains +1 Attack Power for every 10 Stack consumed (%d - Max 4)\n", Boost);
    }

    int Boost = (c2->Burn[0] + c2->Tremor[0])/4;
    if (Boost > 6) Boost = 6;
      chosenSkill->FinalPowerBoost[0] += Boost;
    if (Boost > 0 && c->Passive >= 25) {
      printf(" - At 25 consumed, gains +1 Final Power for every 4 (Burn Stack(%d) + Tremor Stack(%d)) on target (%d - Max 6)\n", c2->Burn[0], c2->Tremor[0], Boost);
    }
    if (Boost >= 50) {

      chosenSkill->CoinPowerBoost[0] +=1;
      chosenSkill->DamageUp[0] += 50;

      printf(" - At 50 consumed, gains +1 Coin Power and deal +50%% damage\n");
    }

    sleep(1);
    
    c->skills[3].active = c->Passive;
    c->Passive = 0;

    }

  // ------------------------------------------------------------






  // --------------------- Erlking Heathcliff ------------------------------

  // Erlking Heathcliff – Passive Buff
  if (isId(c->ID, "Erlking Heathcliff") == 0) {

    float enemyHP = (c2->HP / c2->MAX_HP) * 100; // fraction of HP missing (0.0 - 1.0)
    float myHP = (c->HP / c->MAX_HP) * 100; // fraction of HP missing (0.0 - 1.0)

    if (enemyHP > myHP) {

    chosenSkill->ClashPower[0] += 2;
    chosenSkill->DamageUp[0] += 20;

    printf("\n%s gains +2 Clash Power and +20%% damage\n", c->name);

    sleep(1);
    }
  }

  // Erlking Heathcliff – skill 5
  if (isId(c->ID, "Erlking Heathcliff") == 0 && chosenSkill == &c->skills[5] && abs(c2->Sanity - c->Sanity) > 0) {

    int buff = abs(c2->Sanity - c->Sanity);
    if (buff > 2) buff = 2;

    chosenSkill->ClashPower[0] += buff;

    printf("\n%s gains 1 Clash Power for every 5 Sanity difference (%d - Max 2)\n", c->name, buff);

    sleep(1);
  }

   // ------------------------------------------------------------

// ------------------------------ Gregor:Firefist ------------------------------

    // Gregor:Firefist – Reset passive
  if (isId(c->ID, "Gregor:Firefist") == 0 && c->Passive <= 0) {

    printf("\n%s runs out of fuel, use '%s' instead!\n", c->name, c->defenseSkill[0].name);

    sleep(1);

    printf("\n%s: \"I've prepped plenty of fuel.\"\n", c->name);

    chosenSkill = &c->defenseSkill[0];

    sleep(1);
  }

  // Gregor:Firefist – Reset passive
  if (isId(c->ID, "Gregor:Firefist") == 0 && chosenSkill == &c->defenseSkill[0]) {

    printf("\n%s regain District 12 Fuel to 100\n", c->name);

    c->Passive = 100;

    sleep(1);
  }

  // Gregor:Firefist – Buff Passive
  if (isId(c->ID, "Gregor:Firefist") == 0) {

    if (c2->HP <= (c2->MAX_HP *0.75) || c->HP <= (c->MAX_HP *0.75)) {

    if (c->skills[3].active > 0 && (c2->Burn[0] + c2->Burn[1]) < 30) {
      int boost = c->skills[3].active * 0.2;  // 0.2% per consumed fuel
      if (boost > 40) {
            boost = 40;
      }
      chosenSkill->DamageUp[0] += boost;
      printf("\n%s's HP or %s's HP at 75%% or less HP, Deal +0.2%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (%d%% - Max 40%%)\n",
         c2->name, c->name, boost);

       printf("\n%s: \"Let's see how much more of this you can take.\"\n", c->name);

    } else if (c->skills[3].active > 0 && (c2->Burn[0] + c2->Burn[1]) >= 30) {
      int boost = c->skills[3].active * 0.3;  // 0.3% per consumed fuel
      if (boost > 60) {
        boost = 60;
      }
      chosenSkill->DamageUp[0] += boost;
        printf("\n%s's HP or %s's HP at 75%% or less HP, and main target at 30+ (Burn Stack + Burn Count) (%d), Deal +0.3%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (%d%% - Max 60%%)\n",
           c2->name, c->name, (c2->Burn[0] + c2->Burn[1]), boost);

         printf("\n%s: \"Let's see how much more of this you can take.\"\n", c->name);
      }


    sleep(1);

    }
  }

  // Gregor:Firefist - S1 Coin power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[0]) {

    int gain = c2->Burn[0] / 3;

      if (gain > 0) {
        if (gain > 2) gain = 2;

    printf("\n%s gains +1 Coin Power(%d) for every 3 Burn Stack on target(%d) (Max 2)\n", c->name,
           gain, c2->Burn[0]);

         chosenSkill->CoinPowerBoost[0] += gain;

      }

    sleep(1);
    }

  // Gregor:Firefist - S2 Coin power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[1]) {

    int gain = c2->Burn[0] / 6;

      if (gain > 0) {
        if (gain > 2) gain = 2;

    printf("\n%s gains +1 Coin Power(%d) for every 3 Burn Stack on target(%d) (Max 2)\n", c->name,
           gain, c2->Burn[0]);

         chosenSkill->CoinPowerBoost[0] += gain;

      }

    sleep(1);
    }

  // Gregor:Firefist - S3 Base power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[2]) {

    int gain = c2->Burn[0] / 6;

      if (gain > 0) {
        if (gain > 3) gain = 3;

    printf("\n%s gains +1 Base Power(%d) for every 6 Burn Stack on target(%d) (Max 3)\n", c->name,
           gain, c2->Burn[0]);

         chosenSkill->BasePowerBoost[0] += gain;

      }

    sleep(1);
  }

  // Gregor:Firefist - S3 Coin power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[2]) {

    int gain = c2->Burn[1] / 3;

      if (gain > 0) {
        if (gain > 2) gain = 2;

    printf("\n%s gains +1 Coin Power(%d) for every 3 Burn Count on target(%d) (Max 2)\n", c->name,
           gain, c2->Burn[1]);

             chosenSkill->CoinPowerBoost[0] += gain;

      }

    sleep(1);
  }

  // Gregor:Firefist - S3 Final power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[2]) {

    int gain = (c2->Burn[0] + c2->Burn[1]) / 10;

      if (gain > 0) {
        if (gain > 3) gain = 3;

    printf("\n%s gains +1 Final Power(%d) for every 10 (Burn Stack + Count) on target(%d) (Max 3)\n", c->name,
           gain, c2->Burn[0] + c2->Burn[1]);

         chosenSkill->FinalPowerBoost[0] += gain;

      }

    sleep(1);
  }

    //------------------------------------------------------------------

  // --------------------- Roland --------------------------

  // Roland – Shin (心) - The Black Silence
  if (isId(c->ID, "Fixer grade 9?") == 0 && isId(c2->ID, "Binah") == 0 && c2->Passive == 1) {

    int BaseBuff = ((abs(c2->Sanity - c->Sanity))/10);
     int DmgBuff = 10*((abs(c2->Sanity - c->Sanity))/10);

    printf("\n%s has 'Shin (心) - The Black Silence', Defense +50, Offense +15, +1 Base Power (%d) and +10%% damage (%d%%) for every 10 different Sanity, All Skills become Unbreakable Coins\n",
       c->name, BaseBuff, DmgBuff);

    *tempOffense += 15;
    *tempDefense += 50;
    chosenSkill->BasePowerBoost[0] += BaseBuff;
    chosenSkill->DamageUp[0] += DmgBuff;

    chosenSkill->Unbreakable = chosenSkill->Coins;

    sleep(1);
  }

  // Roland – Buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && c->HP <= c->MAX_HP * 0.5) {
    printf("\n%s: \"That's that, this is this....\" , Offense +10, Defense -5\n",
           c->name);

    *tempOffense += 10;
    *tempDefense -= 5;

    sleep(1);
  }

  //Roland Furioso
  if (isId(c->ID, "Fixer grade 9?") == 0 && chosenSkill->Copies != 0 && chosenSkill != &c->skills[9]) {

    chosenSkill->Copies = -1;

    int randomtaunt = rand() % 3 + 1;

    if (randomtaunt == 1) {
      printf("\n%s: \"You shall feel like I did....\"\n", c->name);
    } else if (randomtaunt == 2) {
      printf("\n%s: \"O sorrow... When will I free from you?\"\n", c->name);
    } else
    printf("\n%s: \"I have nothing but sorrow... And I want nothing more.\"\n", c->name);

    sleep(1);
  }

  // Roland – Skill 1 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[0]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 8) Boost = 8;

    printf("\n%s gains +1 Base Power (%d - Max 8) for every 4 Black Silence (%d)\n",
           c->name, Boost, c->Passive);

    chosenSkill->BasePowerBoost[0] += Boost;

    sleep(1);
  }

  // Roland – Skill 2 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[1]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 5) Boost = 5;

    printf("\n%s gains +1 Base Power (%d - Max 5) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    chosenSkill->BasePowerBoost[0] += Boost;

    sleep(1);
  }

  // Roland – Skill 3 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[2]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 5) Boost = 5;

    printf("\n%s gains +1 Coin Power (%d - Max 5) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

        chosenSkill->CoinPowerBoost[0] += Boost;

    sleep(1);
  }

  // Roland – Skill 4 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[3]) && c->Passive >= 3) {

    int Boost = c->Passive/3;
    if (Boost > 4) Boost = 4;

    printf("\n%s gains +1 Base Power (%d - Max 4) for every 3 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    chosenSkill->BasePowerBoost[0] += Boost;

    sleep(1);
  }

  // Roland – Skill 5 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[4]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 4) Boost = 4;

    printf("\n%s gains +1 Base Power (%d - Max 4) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    chosenSkill->BasePowerBoost[0] += Boost;

    sleep(1);
  }

  // Roland – Skill 6 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[5]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 10) Boost = 10;

    printf("\n%s gains +1 Base Power (%d - Max 10) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    chosenSkill->BasePowerBoost[0] += Boost;

    sleep(1);

    if (c->Passive >= 5) {

    Boost = c->Passive/5;
    if (Boost > 5) Boost = 5;

    printf("\n%s gains +1 Coin Power (%d - Max 5) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

        chosenSkill->CoinPowerBoost[0] += Boost;

    sleep(1);

    }

  }

  // Roland – Skill 7 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[6]) && c->Passive >= 5) {

    int Boost = c->Passive/5;
    if (Boost > 3) Boost = 3;

    printf("\n%s gains +1 Base Power (%d - Max 3) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    chosenSkill->BasePowerBoost[0] += Boost;

    sleep(1);

  }

  // Roland – Skill 8 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[7]) && c->Passive >= 5) {

    int Boost = c->Passive/5;
    if (Boost > 3) Boost = 3;

    printf("\n%s gains +1 Coin Power (%d - Max 3) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

        chosenSkill->CoinPowerBoost[0] += Boost;

    sleep(1);

  }

  // Roland – Skill 9 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[8]) && c->Passive >= 10) {

    int Boost = c->Passive/5;
    if (Boost > 2) Boost = 2;

    printf("\n%s converts 1 Coin to Unbreakable Coin (%d - Max 2) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    if (chosenSkill->Unbreakable < chosenSkill->Coins) {
      chosenSkill->Unbreakable += Boost;
    }

    sleep(1);

  }

  // Roland – Skill 10 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[9]) && c->Passive >= 5) {

    int buff = c->Passive/5;
    if (buff > 5) buff = 5;

    printf("\n%s gain +1 Final Power for every 5 Black Silence (%d) (%d - Max 5)\n",
      c->name, c->Passive, buff);

      chosenSkill->FinalPowerBoost[0] += buff;

    sleep(1);
  }

  // Roland – Skill 10 buff for shin (心)
    if (isId(c->ID, "Fixer grade 9?") == 0 && isId(c2->ID, "Binah") == 0 && c2->Passive == 1 && (chosenSkill == &c->skills[9])) {

      int buff = c->Passive/2;
      if (buff > 10) buff = 10;

      printf("\nIf %s has 'Shin (心) - The Black Silence', gain +1 Base Power (%d - Max 10) for every 2 Black Silence (%d)\n",
        c->name, buff, c->Passive);

        chosenSkill->BasePowerBoost[0] += buff;

      sleep(1);
    }

  // ---------------------------------------------------------------------

  // --------------------------- Hong lu:The Lord of Hongyuan ---------------------------

  // Hong lu:The Lord of Hongyuan - Skill 1 deal more damage on HP
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[0])) {

    int Boost = (c2->Rupture[0] + c->Poise[0]) / 6;
    if (Boost > 1) Boost = 1;

     if (Boost > 0) {

        chosenSkill->CoinPowerBoost[0] += Boost;

      printf("\n%s at 6+ (Rupture Stack on target + Poise Stack on self) (%d), Coin Power +1\n", c->name, (c2->Rupture[0] + c->Poise[0]));

    sleep(1);

     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 2 deal more damage on HP
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[1])) {

    int Boost = (c2->Rupture[0] + c->Poise[0]) / 6;
    if (Boost > 2) Boost = 2;

     if (Boost > 0) {

        chosenSkill->CoinPowerBoost[0] += Boost;

      printf("\n%s gains +1 Coin Power (%d - Max 2) for every 6 (Rupture Stack on target + Poise Stack on self) (%d)\n", c->name, Boost, (c2->Rupture[0] + c->Poise[0]));

    sleep(1);

     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 3 deal more damage on HP
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[2])) {

    int Boost = (c2->Rupture[0] + c->Poise[0]) / 4;
    if (Boost > 3) Boost = 3;

     if (Boost > 0) {

        chosenSkill->CoinPowerBoost[0] += Boost;

      printf("\n%s gains +1 Coin Power (%d - Max 3) for every 4 (Rupture Stack on target + Poise Stack on self) (%d)\n", c->name, Boost, (c2->Rupture[0] + c->Poise[0]));

    sleep(1);
     }
  }

  // Hong lu:The Lord of Hongyuan - Skill Defense Gain
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->defenseSkill[0])) {


    c->AttackSkillPowerUp[1] += 2;

      printf("\n%s gains +2 Attack Skill Power Up next turn\n", c->name);

    sleep(1);

  }

  // Hong lu:The Lord of Hongyuan - Skill 2 Gain
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[1])) {

    int gain = 3;

     if (gain > 0) {

    inflictStatus(c->Poise, gain, 0, 0, 99, 0, 99);
       if (c->Poise[1] > 99) c->Poise[1] = 99;

      printf("\n%s gains +3 Poise Stack (%d)\n", c->name, c->Poise[0]);

    sleep(1);

     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 2 Gain
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[2])) {

    inflictStatus(c->Poise, 5, 3, 0, 99, 0, 99);

      printf("\n%s gains +5 Poise Stack (%d) and +3 Poise Count (%d)\n", c->name, c->Poise[0], c->Poise[1]);

    sleep(1);

  }

  // ---------------------------------------------------------------------------------


  // --------------------------- Yi sang:Fell Bullet -----------------

      // Yi sang:Fell Bullet - Fell Bullet Clash power buff
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      c->skills[2].active > 0) {

    inflictStatus(c->Poise, c->skills[2].active*2, 0, 0, 99, 0, 99);

      printf("\n%s gains +2 Poise Stack (%d) for every Fell Bullet (%d) (Poise Stack %d)\n", c->name, c->skills[2].active*2, c->skills[2].active, c->Poise[0]);

    sleep(1);

    chosenSkill->ClashPower[0] += c->skills[2].active * 2;

    printf("\n%s gains +2 Clash Power (%d) for every Fell Bullet (%d)\n", c->name, c->skills[2].active * 2, c->skills[2].active);

    sleep(1);
  }

  // Yi sang:Fell Bullet - Buff s3
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[2]) {

    int gain = c->Poise[0] / 5;
    if (gain > 2) gain = 2;

    if (gain > 0) {

      chosenSkill->CoinPowerBoost[0] += gain;

      printf("\n%s gains +1 Coin Power for every 5 Poise Stack (%d) on self (%d - Max 2)\n", c->name, c->Poise[0], gain);

    sleep(1);
    }

    gain = c->Poise[1] / 3;
    if (gain > 2) gain = 2;

    if (gain > 0) {

      chosenSkill->BasePowerBoost[0] += gain;

      printf("\n%s gains +1 Base Power for every 3 Poise Count (%d) on self (%d - Max 2)\n", c->name, c->Poise[1], gain);

    sleep(1);
    }

  }

  // Yi sang:Fell Bullet - Buff s1
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[0]) {

    int gain = c->Poise[0] / 5;
    if (gain > 2) gain = 2;

    if (gain > 0) {

      chosenSkill->CoinPowerBoost[0] += gain;

      printf("\n%s gains +1 Coin Power for every 5 Poise Stack (%d) on self (%d - Max 2)\n", c->name, c->Poise[0], gain);

    sleep(1);
    }

    gain = c->Poise[1] / 3;
    if (gain > 2) gain = 2;

    if (gain > 0) {

      chosenSkill->BasePowerBoost[0] += gain;

      printf("\n%s gains +1 Base Power for every 3 Poise Count (%d) on self (%d - Max 2)\n", c->name, c->Poise[1], gain);

    sleep(1);
    }

  }

    // Yi sang:Fell Bullet - Buff s2
    if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
        chosenSkill == &c->skills[1]) {

      int gain = c->Poise[0] / 5;
      if (gain > 2) gain = 2;

      if (gain > 0) {

        chosenSkill->CoinPowerBoost[0] += gain;

        printf("\n%s gains +1 Coin Power for every 5 Poise Stack (%d) on self (%d - Max 2)\n", c->name, c->Poise[0], gain);

      sleep(1);
      }

    }

  // Yi sang:Fell Bullet - Buff defenseskill 1
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->defenseSkill[0]) {

    int gain = c->Poise[0] / 2;
    if (gain > 3) gain = 3;

    if (gain > 0) {

      chosenSkill->CoinPowerBoost[0] += gain;

      printf("\n%s gains +1 Coin Power for every 2 Poise Stack (%d) on self (%d - Max 3)\n", c->name, c->Poise[0], gain);

    sleep(1);
    }

  }

  // Yi sang:Fell Bullet - Poise s1
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[0]) {

     c->Poise[1] += 2;

      printf("\n%s gains +2 Poise Count (%d)\n", c->name, c->Poise[1]);

    sleep(1);

    if (c->Passive < 7) {

      c->Passive += 1;
      if (c->Passive >= 7) {
        c->Passive = 7;
      }

      printf("\n%s gains 1 Torn Memory (%d/7)\n", c->name, c->Passive);
      sleep(1);
    }

  }

  // Yi sang:Fell Bullet - Poise s2
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[1]) {

     inflictStatus(c->Poise, 2, 0, 0, 99, 0, 99);
    if (c->Poise[1] > 99) c->Poise[1] = 99;

      printf("\n%s gains +2 Poise Stack (%d)\n", c->name, c->Poise[0]);

    sleep(1);

  }

  // Yi sang:Fell Bullet - Poise s3
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[2]) {

     inflictStatus(c->Poise, c->Passive, 0, 0, 99, 0, 99);
    if (c->Poise[1] > 99) c->Poise[1] = 99;

      printf("\n%s gains +1 Poise Stack (%d) for every Torn Memory on self (%d)\n", c->name, c->Poise[0], c->Passive);

    sleep(1);

  }

  // Yi sang:Fell Bullet - Poise s1
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->defenseSkill[0]) {

    if (c->Passive < 7) {

      c->Passive += 2;
      if (c->Passive >= 7) {
        c->Passive = 7;
      }

      printf("\n%s gains 2 Torn Memory (%d/7)\n", c->name, c->Passive);
      sleep(1);
    }

  }

      //---------------------------------------------

  // -------------------- Don Quixote:The Manager of La Manchaland ---------------------------

  // Don Quixote:The Manager of La Manchaland - S3-1 Buff S3-2
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") == 0 &&
      chosenSkill == &c->skills[2] && c->Passive >= 15) {

    chosenSkill = &c->skills[5];

    printf("\n%s has 15+ Hardblood(%d), The Manager of La Manchaland Don "
           "Quixote: Empower Skill 3\n",
           c->name, c->Passive);

    sleep(1);

  }

  // Don Quixote:The Manager of La Manchaland - Empower Skill
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") == 0 &&
      chosenSkill == &c->skills[0] && c->Passive >= 15) {

    int Hardblood = 10;

    chosenSkill = &c->skills[3];

    printf("\n%s has 15+ Hardblood(%d), The Priest of La Manchaland Gregor: "
           "Empower Skill 1\n",
           c->name, c->Passive);

    sleep(1);

  } else if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             chosenSkill == &c->skills[1] && c->Passive >= 15) {

    int Hardblood = 10;

    chosenSkill = &c->skills[4];

    printf("\n%s has 15+ Hardblood(%d), The Barber of La Manchaland Outis: "
           "Empower Skill 2\n",
           c->name, c->Passive);

    sleep(1);

  }

  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
    0 &&
  c->Passive >= 15 && chosenSkill == &c->defenseSkill[0]) {

  printf("\n%s has 15+ Hardblood(%d), The Princess of La Manchaland "
  "Rodion: Empower Defense Skill\n",
  c->name, c->Passive);

  c->Passive -= (c->Passive) / 2;

    chosenSkill = &c->defenseSkill[1];

  sleep(1);

  printf(
  "\n%s consumes half of Hardblood(%d left) on self to use %s\n",
  c->name, c->Passive, c->defenseSkill[1].name);

  }

  // Don Quixote:The Manager of La Manchaland - Hardblood gains 2-4
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
          0 &&
      (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1] ||
    chosenSkill == &c->skills[2] || chosenSkill == &c->defenseSkill[0])) {

    c->HP -= (int)(c->MAX_HP * 0.03);
    if (c->HP < 1) c->HP = 1;

    int Hardblood = rand() % 3 + 2;

    c->Passive += Hardblood;
    if (c->Passive > 30) c->Passive = 30;

    printf("\n%s consumes 3%% of Max HP(%d) to gain %d Hardblood (%d) (this damage does not lower the unit's HP below 1)\n", c->name, (int)(c->MAX_HP * 0.03), Hardblood,
           c->Passive);

    sleep(1);
  }

  // Don Quixote:The Manager of La Manchaland - Hardblood heal for counter-2 won
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
          0 &&
      (chosenSkill == &c->defenseSkill[1])) {

    c->Shield += 5 * (c->Passive);

    printf("\n%s gains 5 Shield HP (%.2f) for every Hardblood consumed (%d)\n", c->name, c->Shield + c->TempShield, c->Passive);

    sleep(1);
  }

  // Don Quixote:The Manager of La Manchaland if HP ≤ 30 gains buff
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") == 0 &&
      c->HP <= c->MAX_HP*0.5) {

    c->Passive += 3;

    printf("\n%s at 50%% or less HP, 'Responsibility' activated!, Clash Power +1, Deal +20%% damage, Take +20%% damage, "
           "and gains 3 Hardblood(%d)\n",
           c->name, c->Passive);

    chosenSkill->ClashPower[0] += 1;
    chosenSkill->DamageUp[0] += 20;
    chosenSkill->Protection[0] -= 20;

    sleep(1);
  }

  // Don Quixote:The Manager of La Manchaland - Skill 1/2 in both Buff Coin
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             (chosenSkill == &c->skills[0] ||  chosenSkill == &c->skills[1] || chosenSkill == &c->skills[3] ||  chosenSkill == &c->skills[4])) {

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    int boost = (int)(abs(HPDifferent) / 6);
    if (boost > 2) boost = 2;

     if (boost > 0) {

    chosenSkill->CoinPowerBoost[0] += boost;

    printf("\n%s gains 1 Coin Power for every 6%% HP different (%d - Max 2)\n", c->name, boost);

    sleep(1);
     }
  }

  // Don Quixote:The Manager of La Manchaland - Skill 2 in both Buff Coin
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             (chosenSkill == &c->skills[1] ||  chosenSkill == &c->skills[4])) {

    int boost = 3 * (c->Passive / 5);
    if (boost > 10) boost = 10;

     if (boost > 0) {

    chosenSkill->AttackPowerBoost[0] += boost;

    printf("\n%s gains +3 Attack Power for every 5 Hardblood (%d - Max 10)\n", c->name, boost);

    sleep(1);
     }
  }

  // Don Quixote:The Manager of La Manchaland - Skill 3 in both Buff Coin
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             (chosenSkill == &c->skills[2] ||  chosenSkill == &c->skills[5])) {

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    int boost = (int)(abs(HPDifferent) / 5);
    if (boost > 2) boost = 2;

     if (boost > 0) {

    chosenSkill->CoinPowerBoost[0] += boost;

    printf("\n%s gains 1 Coin Power for every 5%% HP different (%d - Max 2)\n", c->name, boost);

    sleep(1);
     }
  }

  // Don Quixote:The Manager of La Manchaland - Skill 1-2 Buff dmg
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             (chosenSkill == &c->skills[3])) {

    int HPmissing = ((c->MAX_HP - c->HP) / c->MAX_HP) * 100; // 0 - 100
    int damageUP = 2 * (HPmissing);
    if (damageUP > 50) damageUP = 50;

    if (damageUP > 0) {

    chosenSkill->DamageUp[0] += damageUP;

    printf("\n%s deals +2%% damage for every missing HP on self (%d%% - Max 50%%)\n", c->name, damageUP);

    sleep(1);
    }
  }

  // Don Quixote:The Manager of La Manchaland - Skill 3 both Buff dmg
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[5])) {

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    float boost = (abs((int)(HPDifferent/5)) * 5);
    if (boost > 50) boost = 50;
     if (boost > 0) {

    chosenSkill->DamageUp[0] += boost;

    printf("\n%s deals +5%% damage for every 10%% HP different (%.0f%% - Max 50%%)\n", c->name, boost);

    sleep(1);
     }

    if (c->Passive >= 5 && chosenSkill == &c->skills[2]) {

      int boost = (c->Passive/5) * 20;
      if (boost > 100) boost = 100;

      chosenSkill->DamageUp[0] += boost;

      printf("\n%s deals +20%% damage (%d%% - Max 100%%) for every 5 Hardblood (%d)\n", c->name, boost, c->Passive);

      sleep(1);\

    } else if (c->Passive >= 5 && chosenSkill == &c->skills[5]) {

    int boost = (c->Passive/5) * 25;
    if (boost > 100) boost = 100;

    chosenSkill->DamageUp[0] += boost;

    printf("\n%s deals +25%% damage (%d%% - Max 100%%) for every 5 Hardblood (%d)\n", c->name, boost, c->Passive);

    sleep(1);
    }
  }

  // Don Quixote:The Manager of La Manchaland - Skill 3-2 Buff base
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             (chosenSkill == &c->skills[5])) {

    int boost = c->Passive / 10;
     if (boost > 3) boost = 3;

    chosenSkill->DamageUp[0] += boost;

    printf("\n%s gains 1 Base Power (%d - Max 3) for every 10 Hardblood on self (%d)\n", c->name, boost, c->Passive);

    sleep(1);
  }

  // Don Quixote:The Manager of La Manchaland - Hardblood Buff
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") == 0 && c->Passive >= 5 && c->Passive < 10) {

    int boost = (int)(c->Passive / 5);

    *tempOffense += boost;

    printf("\n%s has 5+ Hardblood, gains +1 Offense (%d) for every 5 stacks (%d)\n", c->name, boost, c->Passive);

    sleep(1);
  } else if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") == 0 && c->Passive >= 10) {

    int boost = (int)(c->Passive / 5);

    *tempOffense += boost;
    *tempDefense += boost;

    printf("\n%s has 10+ Hardblood, gains +1 Offense and +1 Defense (%d) for every 5 stacks (%d)\n", c->name, boost, c->Passive);

    sleep(1);
  }

  // ----------------------------------------------------------



  //-----------------------Sancho:The Second Kindred of Don Quixote ---------------------

  // Sancho:The Second Kindred of Don Quixote - Hardblood gains 1
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1] ||
       chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3] ||
       chosenSkill == &c->skills[4] || chosenSkill == &c->skills[6] || 
    chosenSkill == &c->skills[10] ||  chosenSkill == &c->skills[11]) && c->HP > 1) {

    int Hardblood = 1;

        c->Passive += Hardblood;
    if (c->Passive > 30) c->Passive = 30;
        c->HP -= (int)(c->MAX_HP * 0.03);
        if (c->HP < 1) c->HP = 1;

      printf("\n%s consumes 3%% HP(%d) to gain +%d Hardblood(%d) (this damage does not lower the unit's HP below 1)\n", c->name, (int)(c->MAX_HP * 0.03), Hardblood, c->Passive);

      sleep(1);

    } else if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && (chosenSkill == &c->skills[5]) && c->HP > 1) {

    int Hardblood = 2;

      c->Passive += Hardblood;
    if (c->Passive > 30) c->Passive = 30;
      c->HP -= (int)(c->MAX_HP * 0.01);
      if (c->HP < 1) c->HP = 1;

      updateSanity(c, 10);


      printf("\n%s consumes 1%% HP(%d) to gain +%d Hardblood(%d) and heals 10 Sanity (Sanity %d) (this damage does not lower the unit's HP below 1)\n", c->name, (int)(c->MAX_HP * 0.01), Hardblood, c->Passive, c->Sanity);

      sleep(1);

  }

  // Sancho:The Second Kindred of Don Quixote - Hardblood gains 2 at start combat
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0) {

    int Hardblood = 1;

    c->Passive += Hardblood;
    if (c->Passive > 30) c->Passive = 30;

    printf("\n%s gains +%d Hardblood(%d)\n", c->name, Hardblood, c->Passive);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Hardblood gains 5 with skill 9
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      (chosenSkill == &c->skills[9])) {

    int Hardblood = 5;

    c->Passive += Hardblood;
    if (c->Passive > 30) c->Passive = 30;
    c->Shield += (int)(c->MAX_HP*0.1);

    printf("\n%s gains +%d Hardblood(%d) and %d Shield HP (%.2f)\n", c->name, Hardblood, c->Passive, (int)(c->MAX_HP*0.1), c->Shield + c->TempShield);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Hardblood Buff
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && c->Passive >= 10 && c->Passive < 20) {

    int boost = (int)(c->Passive / 6);

    *tempOffense += boost;
    *tempDefense += boost;

    printf("\n%s has 10+ Hardblood, gains +1 Offense and Defense(%d) for every 6 stacks(%d)\n", c->name, boost, c->Passive);

    sleep(1);
  } else if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && c->Passive >= 20) {

    int boost = 2 * (int)(c->Passive / 6);

    *tempOffense += boost;
    *tempDefense += boost;

    printf("\n%s has 20+ Hardblood, gains +2 Offense and Defense(%d) for every 6 stacks(%d)\n", c->name, boost, c->Passive);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Skill 4/5 Buff Coin
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") ==
                 0 &&
             (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[4])) {

    int boost = c->Passive / 10;
    if (boost > 2) boost = 2;
    if (boost > 0) {

        chosenSkill->CoinPowerBoost[0] += boost;

    printf("\n%s gains +1 Coin Power (%d - Max 2) for every 10 Hardblood on self (%d)\n", c->name, boost, c->Passive);

    sleep(1);
    }

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    boost = (int)(abs(HPDifferent) / 15);
    if (boost > 3) boost = 3;

     if (boost > 0) {

    chosenSkill->CoinPowerBoost[0] += boost;

       printf("\n%s gains +1 Coin Power for every 15%% HP different (%d - Max 3)\n", c->name, boost);

       sleep(1);
        }
  }

  // Sancho:The Second Kindred of Don Quixote - Skill 8 Buff Coin
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") ==
                 0 &&
             (chosenSkill == &c->skills[7])) {

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    int boost = (int)(abs(HPDifferent) / 6);
    if (boost > 1) boost = 1;

     if (boost > 0) {

    chosenSkill->CoinPowerBoost[0] += boost;

       printf("\n%s at 6%% HP different, Coin Power +1\n", c->name);

       sleep(1);

        }
  }

  // Sancho:The Second Kindred of Don Quixote - Skill 9 Buff Base
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") ==
                 0 &&
             (chosenSkill == &c->skills[7])) {

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    int boost = (int)(abs(HPDifferent) / 3);
    if (boost > 3) boost = 3;

     if (boost > 0) {

    chosenSkill->BasePowerBoost[0] += boost;

       printf("\n%s gains +1 Base Power for every 3%% HP different (%d - Max 3)\n", c->name, boost);

       sleep(1);

        }
  }

  // Sancho:The Second Kindred of Don Quixote - skill 11
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
    chosenSkill == &c->skills[10] && c->HP > 1) {

    int boost = 0;
    int HPvalue = 0;

    while (c->HP > 1 && boost < 2) {
    c->HP -= (int)(c->MAX_HP * 0.03);
      if (c->HP < 1) c->HP = 1;

      HPvalue += (int)(c->MAX_HP * 0.03);

    boost++;
    }

        chosenSkill->CoinPowerBoost[0] += boost;

      printf("\n%s gains +1 Coin Power (%d - Max 2) for every consumed 3%% of Max HP (%d) (this damage does not lower the unit's HP below 1)\n",
         c->name, boost, HPvalue);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - skill 12
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    chosenSkill == &c->skills[11] && c->HP > 1) {

  int boost = 0;
    int HPvalue = 0;

    while (c->HP > 1 && boost < 5) {
    c->HP -= (int)(c->MAX_HP * 0.02);
      if (c->HP < 1) c->HP = 1;

      HPvalue += (int)(c->MAX_HP * 0.02);

    boost++;
    }

    chosenSkill->BasePowerBoost[0] += boost;

      printf("\n%s gains +1 Base Power (%d - Max 5) for every consumed 2%% of Max HP (%d) (this damage does not lower the unit's HP below 1)\n",
         c->name, boost, HPvalue);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Ultimate skill 13
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
    chosenSkill == &c->skills[12] && c->HP > 1) {

    if (c->Passive >= 5) {
    c->Passive -= 5;
    if (c->Passive < 1) c->Passive = 1;

    printf("\n%s consumes 5 Hardblood(%d) to gain Clash Power +5\n",
           c->name, c->Passive);
    } else if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
    chosenSkill == &c->skills[12] && c->HP > 1) {

      printf("\n%s's Hardblood isn't enough consumes 5%% of Max HP(%d) to gain Clash Power +5 instead (this damage does not lower the unit's HP below 1)\n",
         c->name, (int)(c->MAX_HP * 0.05));

      c->HP -= (int)(c->MAX_HP * 0.05);
      if (c->HP < 1) c->HP = 1;
    }

    chosenSkill->ClashPower[0] += 5;

    sleep(1);
  }
  // Sancho:The Second Kindred of Don Quixote - Ultimate
  if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
    chosenSkill == &c->skills[13]) {

    updateSanity(c, c->Passive * 2);


    c->Passive = 1;

    printf("\n%s consumes all Hardblood to heal 2 Sanity per Hardblood and heal up to 45 Sanity\n",
           c->name);

    if (c->Sanity < 45 && c->HP > 1) {

      c->HP -= 45 - c->Sanity;
      if (c->HP < 1) c->HP = 1;

      printf("\n%s's Hardblood isn't enough consumes HP to heal 1 Sanity per HP(%d) and heal up to 45 Sanity instead (this damage does not lower the unit's HP below 1)\n",
         c->name, 45 - c->Sanity);

      c->Sanity = 45;
    }

      sleep(1);
    }

    // Sancho:The Second Kindred of Don Quixote - consumes Hardblood with s5
    if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") ==  0 &&
        (chosenSkill == &c->skills[5]) && c->Passive >= 4) {

      c->Passive -= 4;
      if (c->Passive < 1) c->Passive = 1;

      updateSanity(c, 10);


      printf("\n%s consumes 4 Hardblood(%d) to heal 10 Sanity (%d)\n",
             c->name, c->Passive, c->Sanity);

      sleep(1);
    }
    //-----------------------------------------------------------------------------------------

  // -------------------- Lobotomy E.G.O::Solemn Lament Yi Sang --------------------

  // Lobotomy E.G.O::Solemn Lament Yi Sang - Clash power
  if (isId(c->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
          0 && (chosenSkill == &c->defenseSkill[0])) {

    c->defenseSkill[0].active = 1;

    }


  // Lobotomy E.G.O::Solemn Lament Yi Sang - Clash power
  if (isId(c->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
          0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

    if (c->Passive >= 10) {
    chosenSkill->ClashPower[0] += 1;

    printf("\n%s at 10+ The Living & The Departed(%d), gains 1 Clash Power\n",
           c->name, c->Passive);

    sleep(1);
    }

    if (abs(c->Sanity - c2->Sanity) >= 10) {

      int buff = abs(c->Sanity - c2->Sanity)/10;
      if (buff > 2) buff = 2;

    chosenSkill->CoinPowerBoost[0] += buff;

    printf("\n%s gains 1 Coins Power for every 10 Sanity difference (%d - Max 2)\n",
           c->name, buff);

      sleep(1);
    }

  }  // Lobotomy E.G.O::Solemn Lament Yi Sang - Clash power
  else if (isId(c->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
            0 && (chosenSkill == &c->skills[2])) {

    if (c->Passive >= 5) {
      chosenSkill->BasePowerBoost[0] += c->Passive/5;

      printf("\n%s gains 1 Base Power(%d) for every 5 The Living & The Departed(%d)\n",
             c->name, c->Passive/5, c->Passive);

      sleep(1);
    }

    if (abs(c->Sanity - c2->Sanity) >= 10) {

      int buff = abs(c->Sanity - c2->Sanity)/10;
      if (buff > 2) buff = 2;

    chosenSkill->CoinPowerBoost[0] += buff;

    printf("\n%s gains 1 Coins Power for every 10 Sanity difference (%d - Max 2)\n",
           c->name, buff);

    sleep(1);
    }

    }

    // ------------------------------------------------------------

  // ------------------------ Dawn Office Fixer Sinclair ----------------------------

  // Dawn Office Fixer Sinclair - Skill Buff S2
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->Sanity >= 20 && (chosenSkill == &c->skills[1])) {

    chosenSkill->ClashPower[0] += 1;

    printf("\n%s at 20+ Sanity, gains 1 Clash Power\n",
           c->name);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff form S2
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->Sanity >= 10 && (chosenSkill == &c->skills[1])) {

    chosenSkill->CoinPowerBoost[0] += 1;

    printf("\n%s at 10+ Sanity, gains +1 Coin Power (%d Sanity)\n",
           c->name, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff base form S1
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->Sanity >= 10 && (chosenSkill == &c->skills[0])) {

    int boost = c->Sanity/5;
    if (boost > 4) boost = 4;

    chosenSkill->BasePowerBoost[0] += boost;

    printf("\n%s at 10+ Sanity, gains 1 Base Power for every 5 Sanity (%d - Max 4) (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff base form S3
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && !c->skills[3].active && c->Sanity >= 10 && (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[2])) {

    int boost = c->Sanity/10;
    if (boost > 2) boost = 2;

    chosenSkill->CoinPowerBoost[0] += boost;

    printf("\n%s at 10+ Sanity, gains 1 Coin Power for every 10 Sanity (%d - Max 2) (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S2
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && (chosenSkill == &c->skills[1])) {

    updateSanity(c, -(5));

    chosenSkill->AttackPowerBoost[0] += 5;

    printf("\n%s is in a Volatile E.G.O State consumes 5 Sanity (%d) to gain +5 Attack Power\n",
           c->name, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S4
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[2])) {

    if (c->Sanity >= 0) {
    int boost = 2*(c->Sanity/10);
      if (boost > 8) boost = 8;

        chosenSkill->CoinPowerBoost[0] += boost;

    printf("\n%s at 0+ Sanity gains 2 Coin Power (%d - Max 8) for every 10 Sanity (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

    }

  }

  // Dawn Office Fixer Sinclair - Skill Buff base form
 if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && !c->skills[3].active && c->Sanity >= 10) {

      chosenSkill->ClashPower[0] += c->Sanity/10;

      printf("\n%s at 10+ Sanity, gains 1 Clash Power (%d) for every 10 Sanity (%d Sanity)\n",
             c->name, c->Sanity/10, c->Sanity);

      sleep(1);

    }

  // Dawn Office Fixer Sinclair - Skill Buff Ego form
 if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && c->Sanity >= 20) {

          chosenSkill->CoinPowerBoost[0] += c->Sanity/20;

      printf("\n%s at 20+ Sanity, gains 1 Coin Power (%d) for every 20 Sanity (%d Sanity)\n",
             c->name, c->Sanity/20, c->Sanity);

      sleep(1);

    }

  // ------------------------------------------------------------------------------

  // --------------------------- Heishou Pack - You Branch Adept Heathcliff -----------------------

  // Heishou Pack - You Branch Adept Heathcliff - Bloody Storm of Blades
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->HP <= c->MAX_HP * 0.8) {

    int gain = ((c->MAX_HP - c->HP)/c->MAX_HP) / 0.2;
    if (gain > 3) gain = 3;

    *tempOffense += gain;
    *tempDefense += gain;

      printf("\n%s gains 1 Offense (%d) and 1 Defense (%d) for every 20%% missing HP (Max 3 each)\n", c->name, gain, gain);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 4
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->Passive >= 20 && chosenSkill == &c->skills[2]) {

    chosenSkill = &c->skills[3];

      printf("\n%s's Battleblood Instinct at 20+ Stack, activate '%s' instead\n", c->name, c->skills[3].name);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 1 and 2 Clash power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->Burn[0] >= 10 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

    int clashpower = c->Burn[0]/10;
     if (clashpower > 2) clashpower = 2;

     chosenSkill->ClashPower[0] += clashpower;

      printf("\n%s gains +1 Clash Power for every 10 Burn Stack (%d) on self (%d - Max 2)\n", c->name, c->Burn[0], clashpower);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 1 and 2 coin power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (c->Burn[0] + c->Burn[1]) >= 6 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

     chosenSkill->CoinPowerBoost[0] += 1;

      printf("\n%s at 6+ Burn (Stack(%d) + Count(%d)) on self, gains +1 Coin Power\n", c->name, c->Burn[0], c->Burn[1]);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill def base power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (c->Burn[0] + c->Burn[1]) >= 10 && (chosenSkill == &c->defenseSkill[0])) {

     chosenSkill->BasePowerBoost[0] += 1;

      printf("\n%s at 10+ Burn (Stack(%d) + Count(%d)) on self, gains +1 Base Power\n", c->name, c->Burn[0], c->Burn[1]);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill def coin power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->defenseSkill[0]) && c->skills[2].active > 0) {

      chosenSkill->CoinPowerBoost[0] += 1;

      printf("\n%s has Bloodflame [血炎], gains +1 Coin Power\n", c->name);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill def deal more damage on HP
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->defenseSkill[0])) {

    float damageboost = c->skills[6].active / 3;
    if (damageboost > 30.0f) damageboost = 30.0f;

    chosenSkill->DamageUp[0] += damageboost;

      printf("\n%s deals +1%% damage for every 3%% HP this unit cumulatively lost in this Encounter (%.0f%% - Max 30%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 and 4 coin power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (c->Burn[0] + c->Burn[1]) >= 6 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int Buff = (c->Burn[0] + c->Burn[1])/6;
     if (Buff > 2) Buff = 2;

     chosenSkill->CoinPowerBoost[0] += Buff;

      printf("\n%s gains +1 Coin Power for every 6 Burn (Stack(%d) + Count(%d)) on self (%d - Max 2)\n", c->name, c->Burn[0], c->Burn[1], Buff);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 and 4 Clash power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->HP <= c->MAX_HP * 0.8 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int clashpower = (c->MAX_HP - c->HP)/c->MAX_HP / 0.2;
     if (clashpower > 3) clashpower = 3;

     chosenSkill->ClashPower[0] += clashpower;

      printf("\n%s gains +1 Clash Power for every 20%% missing HP on self (%d - Max 3)\n", c->name, clashpower);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 deal more damage on burn
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[2]) && c->Burn[0] > 0) {

    float damageboost = c->Burn[0] * 1.5f;
    if (damageboost > 30.0f) damageboost = 30.0f;
    chosenSkill->DamageUp[0] += damageboost;

      printf("\n%s deals +1.5%% damage for every Burn Stack on self (%.1f%% - Max 30%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 4 deal more damage on burn
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[3]) && c->Burn[0] > 0) {

    int damageboost = c->Burn[0] * 3;
    if (damageboost > 30) damageboost = 30;
    chosenSkill->DamageUp[0] += damageboost;

      printf("\n%s deals +3%% damage for every Burn Stack on self (%d%% - Max 30%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 4 deal more damage on HP
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[3])) {

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
    float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

    float damageboost = (missingSelf + missingEnemy) / 2.0f;
    if (damageboost > 50.0f) damageboost = 50.0f;

    chosenSkill->DamageUp[0] += damageboost;

      printf("\n%s deals +1%% damage for every 2%% (missing HP percentage on target + missing HP percentage on self) (%.0f%% - Max 50%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 coins
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[2]) && (c->skills[2].active > 0 || c->HP < c->MAX_HP*0.5)) {

    c->skills[2].Unbreakable = c->skills[2].Coins;

      printf("\n%s has Bloodflame [血炎], or less than 50%% HP, convert all Coins to Unbreakable Coins\n", c->name);

    sleep(1);

  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 and 4 Gain 3 Bloodflame [血炎] 
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3] || chosenSkill == &c->defenseSkill[0])) {

    int gain = 3;

    if (chosenSkill == &c->defenseSkill[0]) gain = 2;

    c->skills[2].active += gain;
    if (c->skills[2].active > 3) c->skills[2].active = 3;

      printf("\n%s gains %d Bloodflame [血炎] (%d - Max 3)\n", c->name, gain, c->skills[2].active);

    sleep(1);
  }

  // ---------------------------------------------------------



// ------------------- King in Binds -------------------

  // King in Binds - Skill 1 Coin buff
  if (isId(c->ID, "King in Binds") == 0 && c2->Sinking[0] >= 6 && (chosenSkill == &c->skills[0])) {

        chosenSkill->CoinPowerBoost[0] += 1;

      printf("\nIf target has 6+ Sinking Stack, Coin Power +1\n");

    sleep(1);
  }

  // King in Binds - Skill 2 or 4 Coin buff
  if (isId(c->ID, "King in Binds") == 0 && c2->Sinking[0] >= 6 && (chosenSkill == &c->skills[1] || chosenSkill == &c->skills[3])) {

    int gain = c2->Sinking[0]/6;
    if (gain > 2) gain = 2;

        chosenSkill->CoinPowerBoost[0] += gain;

      printf("\n%s gains +1 Coin Power (%d - Max 2) for every 6 Sinking Stack on target (%d)\n", c->name, gain, c2->Sinking[0]);

    sleep(1);
  }

  // King in Binds - Skill 3 Clash Power buff
  if (isId(c->ID, "King in Binds") == 0 && c2->Tremor[0] >= 3 && (chosenSkill == &c->skills[2])) {

    chosenSkill->ClashPower[0] += 1;

    printf("\nIf target has 3+ Tremor Stack, Clash Power +1\n");

    sleep(1);
  }

  // King in Binds - Skill 5 Clash Power buff
  if (isId(c->ID, "King in Binds") == 0 && c2->Tremor[0] >= 3 && (chosenSkill == &c->skills[4])) {

    int gain = c2->Tremor[0]/3;
    if (gain > 3) gain = 3;

    chosenSkill->ClashPower[0] += gain;

    printf("\n%s gains +1 Clash Power (%d - Max 3) for every 3 Tremor Stack on target (%d)\n", c->name, gain, c2->Tremor[0]);

    sleep(1);
  }

  // King in Binds - Skill 6 buff
  if (isId(c->ID, "King in Binds") == 0 && (chosenSkill == &c->skills[5])) {

    c->DefenseLevelDown[1] += 5;

    chosenSkill->FinalPowerBoost[1] += 2;

    if (c->skills[6].active == 0) {
    c->skills[6].active++;
    } else {
      c->skills[6].Unbreakable++; // in case it stack
    }

    printf("\n%s gains 5 Defense Down, 2 Final Power Up, Moment of Audience (On Hit with skill: Trigger 'Tremor Burst', target gain +5 Tremor Stack, +5 Tremor Count and take 10%% more damage next turn; then this effect expire) next turn\n", c->name);

    sleep(1);

    if (c->Passive > 0) {

    c2->TempShield += 50.0f;

    printf("\nTarget with 'Bandages of the King in Binds', gains 50 Shield (%.2f)\n", c2->Shield + c2->TempShield);

    }
  }

  // ---------------------------------------------------------

  // ------------------ Sukuna:King of Curse ------------------

  // Sukuna:King of Curse fuga
  if (isId(c->ID, "Sukuna:King of Curse") ==
          0 && (chosenSkill == &c->skills[3]) && c->skills[3].active > 0) {

    printf("\n%s consumes all 'Binding Vow - Open' to gains 1%% damage for every consumed (%d)\n", c->name, c->skills[3].active);

    chosenSkill->DamageUp[0] += c->skills[3].active;

    int Coingain = c->skills[3].active/5;
    if (Coingain > 5) Coingain = 5;

    if (Coingain > 0) {

        chosenSkill->CoinPowerBoost[0] += Coingain;

    printf("\n%s gains +1 Coin Power (%d - Max 5) for every 5 'Binding Vow - Open' consumed (%d)\n",
           c->name, Coingain, c->skills[3].active);

    sleep(1);
    }

    int Basegain = c->skills[3].active/10;
    if (Basegain > 3) Basegain = 3;

    if (Basegain > 0) {

    chosenSkill->BasePowerBoost[0] += Basegain;

    printf("\n%s gains +1 Base Power (%d - Max 3) for every 10 'Binding Vow - Open' consumed (%d)\n",
           c->name, Basegain, c->skills[3].active);

    sleep(1);
    }

    c->skills[3].active = 0;

  }

    // ------------------------------------------------------



// Evade Skill
  if (chosenSkill->skillType == 2 || chosenSkill->skillType == 4) {
      chosenSkill->active = 1; // 1 หมายถึง เหรียญหลบยังอยู่ดี
  }













  return chosenSkill;
}

















void applyClashStartPassives(Character *p1, SkillStats *s1, Character *p2, SkillStats *s2) {

  // --------------------- Clash Phase ----------------------

  // Heishou Pack - You Branch Adept Heathcliff passive gain on clash
  if (isId(p1->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0) {

    p1->Passive += 3;
    if (p1->Passive > 20) p1->Passive = 20;

    printf("\n%s gains 3 Battleblood Instinct (%d)\n", p1->name, p1->Passive);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff Skill 1 and 2 gain on clash
  if (isId(p1->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (s1 == &p1->skills[0] || s1 == &p1->skills[1])) {

    p1->Burn[0]++;
    if (p1->Burn[0] > 99) p1->Burn[0] = 99;
    p1->Burn[1]++;
    if (p1->Burn[1] > 99) p1->Burn[1] = 99;

    printf("\n%s applies +1 Burn Stack(%d) and +1 Burn Count(%d) on self\n", p1->name, p1->Burn[0], p1->Burn[1]);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff Skill 1 and 2 gain on clash
  if (isId(p1->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (s1 == &p1->defenseSkill[0])) {

      inflictStatus(p1->Burn, 0, 3, 0, 99, 0, 99);

    printf("\n%s gains +3 Burn Count on self (%d)\n", p1->name, p1->Burn[1]);

    sleep(1);
  }

  // --------------------------------------------------------
}



















// p1 = winner p2 = loser
ClashResult ClashableCounter(Character *p1, SkillStats *s1, int playerTempOffense,
   int playerTempDefense, int WinnerCoin, Character *p2, SkillStats *s2,
   int enemyTempOffense, int enemyTempDefense, int LoserCoin,
   Character *fullPlayer, int PContinueUnbreakCoin, int EContinueUnbreakCoin) {


  int playerUnbreakableLost = 0;
  int enemyUnbreakableLost = 0;

  // ContinueUnbreakCoin
  if (PContinueUnbreakCoin > 0) {
    playerUnbreakableLost = PContinueUnbreakCoin;
  }
  if (EContinueUnbreakCoin > 0) {
      enemyUnbreakableLost = EContinueUnbreakCoin;
  }

  // ตรวจสอบว่าฝั่งไหน Coins หมด (คือผู้แพ้ที่ต้องการจะ Counter)
  Character *counterUnit = p2; 
  Character *AttackUnit  = p1; 

   // เช็คว่าคนที่แพ้คือฝั่ง Player หรือไม่
   int isCounterUnitPlayerSide = (counterUnit == fullPlayer); 

  // ------------------------- Counter -----------------------------------------

  // Don Quixote:The Manager of La Manchaland - Counterattack
  if (isId(counterUnit->ID, "Don Quixote:The Manager of La Manchaland") ==
          0 &&
      counterUnit->Passive >= 5 &&
      (s2 != &counterUnit->defenseSkill[0] && s2 != &counterUnit->defenseSkill[1] && s2 != &counterUnit->skills[3] && s2 != &counterUnit->skills[4] && s2 != &counterUnit->skills[5])) {

       clearTurnSkillBuffs(counterUnit);


    int cOff = (counterUnit->OffenseLevelUp[0] - counterUnit->OffenseLevelDown[0]), cDef = (counterUnit->DefenseLevelUp[0] - counterUnit->DefenseLevelDown[0]);

    SkillStats *chosenSkill = getEffectiveSkill(
        counterUnit, AttackUnit, &counterUnit->defenseSkill[0], &cOff, &cDef);

    counterUnit->Passive -= 5;
    if (counterUnit->Passive < 1) counterUnit->Passive = 1;
    printf("\n%s consumes 5 Hardblood (%d left) to continue clashing with %s (Once per Turn)\n",
        counterUnit->name, counterUnit->Passive, chosenSkill->name);

    sleep(1);

    counterUnit->HP -= (int)(counterUnit->MAX_HP * 0.03);
    if (counterUnit->HP < 1) counterUnit->HP = 1;

    int Hardblood = rand() % 3 + 2;

    counterUnit->Passive += Hardblood;
    if (counterUnit->Passive > 30) counterUnit->Passive = 30;

    printf("\n%s consumes 3%% of Max HP(%d) to gain %d Hardblood (%d) (this damage does not lower the unit's HP below 1)\n", counterUnit->name, (int)(counterUnit->MAX_HP * 0.03), Hardblood,
           counterUnit->Passive);

    sleep(1);

    if (isId(counterUnit->ID, "Don Quixote:The Manager of La Manchaland") ==
            0 &&
        counterUnit->Passive >= 10) {

      clearTurnSkillBuffs(counterUnit);

      printf("\n%s has 10+ Hardblood(%d), The Princess of La Manchaland "
         "Rodion: Empower Defense Skill\n",
         counterUnit->name, counterUnit->Passive);

      counterUnit->Passive -= (counterUnit->Passive) / 2;

      sleep(1);

      printf(
      "\n%s consumes half of Hardblood(%d left) on self to use %s\n",
      counterUnit->name, counterUnit->Passive, counterUnit->defenseSkill[1].name);

      int cOff = (counterUnit->OffenseLevelUp[0] - counterUnit->OffenseLevelDown[0]), cDef = (counterUnit->DefenseLevelUp[0] - counterUnit->DefenseLevelDown[0]);

      chosenSkill = getEffectiveSkill(counterUnit, AttackUnit, &counterUnit->defenseSkill[1], &cOff, &cDef);

      int savedEnemyCoins = s1->Coins;
      s1->Coins = WinnerCoin;

      ClashResult newClash;

      if (isCounterUnitPlayerSide) {
          // --- [กรณีที่ 1] เรา (Player) เป็นคนสวนกลับ ---
          // counterUnit (เรา) อยู่ซ้าย ใช้ chosenSkill (ท่าสวน)
          // AttackUnit (บอส) อยู่ขวา ใช้ s1 (ท่าเดิมที่เขาชนะเรามา)
          newClash = clashPhase(counterUnit, chosenSkill, cOff, cDef, 
                                           AttackUnit, s1, playerTempOffense, playerTempDefense, 
                                           fullPlayer, EContinueUnbreakCoin, PContinueUnbreakCoin);
          s1->Coins = savedEnemyCoins;

      } else {
          // --- [กรณีที่ 2] บอส (Enemy) เป็นคนสวนกลับ ---
          // AttackUnit (เรา) อยู่ซ้าย ใช้ s1 (ท่าเดิมที่เราชนะเขามา)
          // counterUnit (บอส) อยู่ขวา ใช้ chosenSkill (ท่าสวนของเขา)
          newClash = clashPhase(AttackUnit, s1, playerTempOffense, playerTempDefense, 
                                           counterUnit, chosenSkill, cOff, cDef, 
                                           fullPlayer, PContinueUnbreakCoin, EContinueUnbreakCoin);
          s1->Coins = savedEnemyCoins;

      }

      sleep(1); 

      return newClash;

    } else {

      int savedEnemyCoins = s1->Coins;
      s1->Coins = WinnerCoin;

      ClashResult newClash;

      if (isCounterUnitPlayerSide) {
          // --- [กรณีที่ 1] เรา (Player) เป็นคนสวนกลับ ---
          // counterUnit (เรา) อยู่ซ้าย ใช้ chosenSkill (ท่าสวน)
          // AttackUnit (บอส) อยู่ขวา ใช้ s1 (ท่าเดิมที่เขาชนะเรามา)
          newClash = clashPhase(counterUnit, chosenSkill, cOff, cDef, 
                                           AttackUnit, s1, playerTempOffense, playerTempDefense, 
                                           fullPlayer, EContinueUnbreakCoin, PContinueUnbreakCoin);
          s1->Coins = savedEnemyCoins;

      } else {
          // --- [กรณีที่ 2] บอส (Enemy) เป็นคนสวนกลับ ---
          // AttackUnit (เรา) อยู่ซ้าย ใช้ s1 (ท่าเดิมที่เราชนะเขามา)
          // counterUnit (บอส) อยู่ขวา ใช้ chosenSkill (ท่าสวนของเขา)
          newClash = clashPhase(AttackUnit, s1, playerTempOffense, playerTempDefense, 
                                           counterUnit, chosenSkill, cOff, cDef, 
                                           fullPlayer, PContinueUnbreakCoin, EContinueUnbreakCoin);
          s1->Coins = savedEnemyCoins;

      }

      sleep(1); 

      return newClash;

    }
  }



  // Counter Wild hunt
    if (isId(counterUnit->ID, "Heathcliff:Wild Hunt") == 0 && counterUnit->Sanity >= 15 && counterUnit->skills[0].active > 0 && !counterUnit->skills[2].active) {

       clearTurnSkillBuffs(counterUnit);

      counterUnit->skills[2].active = 1;

      printf("\n%s mounted 'Dullahan', at 15+ Sanity(%d), use '%s' to continue the Clash (Once per Turn)\n",
         counterUnit->name, counterUnit->Sanity, counterUnit->skills[3].name);

      sleep(1);

      int cOff = (counterUnit->OffenseLevelUp[0] - counterUnit->OffenseLevelDown[0]), cDef = (counterUnit->DefenseLevelUp[0] - counterUnit->DefenseLevelDown[0]);

      SkillStats *chosenSkill = getEffectiveSkill(
          counterUnit, AttackUnit, &counterUnit->skills[3], &cOff, &cDef);

      int savedEnemyCoins = s1->Coins;
      s1->Coins = WinnerCoin;

      ClashResult newClash;

      if (isCounterUnitPlayerSide) {
          // --- [กรณีที่ 1] เรา (Player) เป็นคนสวนกลับ ---
          // counterUnit (เรา) อยู่ซ้าย ใช้ chosenSkill (ท่าสวน)
          // AttackUnit (บอส) อยู่ขวา ใช้ s1 (ท่าเดิมที่เขาชนะเรามา)
          newClash = clashPhase(counterUnit, chosenSkill, cOff, cDef, 
                                           AttackUnit, s1, playerTempOffense, playerTempDefense, 
                                           fullPlayer, EContinueUnbreakCoin, PContinueUnbreakCoin);
          s1->Coins = savedEnemyCoins;

      } else {
          // --- [กรณีที่ 2] บอส (Enemy) เป็นคนสวนกลับ ---
          // AttackUnit (เรา) อยู่ซ้าย ใช้ s1 (ท่าเดิมที่เราชนะเขามา) 
          // counterUnit (บอส) อยู่ขวา ใช้ chosenSkill (ท่าสวนของเขา)
          newClash = clashPhase(AttackUnit, s1, playerTempOffense, playerTempDefense, 
                                           counterUnit, chosenSkill, cOff, cDef, 
                                           fullPlayer, PContinueUnbreakCoin, EContinueUnbreakCoin);
          s1->Coins = savedEnemyCoins;

      }

      sleep(1); 

      return newClash;

    }

  // -------------------------------------------------------------

// [สำคัญมาก] ถ้าไม่มี Counter ไหนทำงานเลย ให้ส่งค่า "ว่าง" กลับไป
ClashResult empty = {0}; 
empty.winner = 0; // ใช้ winner 0 เป็นตัวบอกว่าไม่มี Counter เกิดขึ้น
return empty;

}















// p1 = winner p2 = loser
void applyClashRoundResult(Character *p1, SkillStats *s1, Character *p2, SkillStats *s2, int playerCoins, int enemyCoins, int clashCount) {

    // --- ClashEffect Identity ---

   // ------------------------------ player win ----------------------------------

  // The House of Spiders: The Thumb Nursefather Rodion - Skill 1 Lose to buff
  if (isId(p1->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (s1 == &p1->skills[0]) && p1->skills[11].active == 0 && enemyCoins <= 0) {

    int lost = playerCoins;

    int canLose = 10 - p1->skills[6].active; // โควต้าที่เหลือในเทิร์นนี้

    int actualLoss = (lost > canLose) ? canLose : lost;
    if (actualLoss > p1->Passive) actualLoss = p1->Passive;

        p1->Passive -= actualLoss;
        p1->skills[6].active += actualLoss; // บันทึกว่าเสียไปเท่าไหร่แล้วในเทิร์นนี้

    int gain = 5 * lost;
    if (gain > 10) gain = 10;
    
    s1->DamageUp[0] += gain;

      printf("\n%s won the Clash, lose (# of remaining Coins) Eye of Precognition Stack on self (%d); deal +5%% damage for every Stack lost (%d%% - Max 10%%)\n", p1->name, p1->Passive, gain);

    sleep(1);

    if (p1->Passive <= 0 && p1->skills[11].active == 0) {
          p1->skills[11].active = 1; // ติด Overheat

      printf("\n%s converts 'Eye of Precognition' into 'Eye of Precognition - Overheat'\n", p1->name);
    }
    }

  // The House of Spiders: The Thumb Nursefather Rodion - Skill 2 Lose to buff
  if (isId(p1->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (s1 == &p1->skills[2] || s1 == &p1->skills[3]) && p1->skills[11].active == 0 && enemyCoins <= 0) {

    int lost = playerCoins;

    int canLose = 10 - p1->skills[6].active; // โควต้าที่เหลือในเทิร์นนี้

    int actualLoss = (lost > canLose) ? canLose : lost;
    if (actualLoss > p1->Passive) actualLoss = p1->Passive;

        p1->Passive -= actualLoss;
        p1->skills[6].active += actualLoss; // บันทึกว่าเสียไปเท่าไหร่แล้วในเทิร์นนี้

    int gain = 5 * lost;
    if (gain > 15) gain = 15;

    s1->DamageUp[0] += gain;

      printf("\n%s won the Clash, lose (# of remaining Coins) Eye of Precognition Stack on self (%d); deal +5%% damage for every Stack lost (%d%% - Max 15%%)\n", p1->name, p1->Passive, gain);

    sleep(1);

    if (p1->Passive <= 0 && p1->skills[11].active == 0) {
          p1->skills[11].active = 1; // ติด Overheat

      printf("\n%s converts 'Eye of Precognition' into 'Eye of Precognition - Overheat'\n", p1->name);
    }
    }

  // -------------- The One Who Grips Faust --------------

  // The One Who Grips Faust Skill 1 won
  if (isId(p1->ID, "The One Who Grips Faust") == 0 &&
      s1 == &p1->skills[0] && enemyCoins <= 0) {

    updateSanity(p2, -3);

    printf("\n%s won the Clash, %s loses 3 Sanity (%d)\n", p1->name, p2->name, p2->Sanity);

     sleep(1);
  }

  // The One Who Grips Faust Skill 2 won
  if (isId(p1->ID, "The One Who Grips Faust") == 0 &&
      s1 == &p1->skills[1] && enemyCoins <= 0) {

    p1->skills[6].active += 1;

    printf("\n%s won the Clash, %s gains 1 Fanatic next turn\n", p1->name, p1->name);

     sleep(1);
  }

  // The One Who Grips Faust Skill 2 won
  if (isId(p1->ID, "The One Who Grips Faust") == 0 &&
      (s1 == &p1->skills[2] || s1 == &p1->skills[3]) && enemyCoins <= 0) {

    p1->skills[6].active += 2;
    p1->Passive += 2;

    printf("\n%s won the Clash, %s gains 2 Fanatic this turn and next turn\n", p1->name, p1->name);

     sleep(1);
  }

  // --------------------------------------------------------

  // Heishou Pack - You Branch Adept Heathcliff Skill 4 won
  if (isId(p1->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 &&
      s1 == &p1->skills[3] && enemyCoins <= 0) {

      inflictStatus(p1->Burn, 10, 0, 0, 99, 0, 99);

    printf("\n%s won the Clash, %s gains 10 Burn Stack (%d)\n", p1->name, p1->name, p1->Burn[0]);

     sleep(1);
  }


  // ---------------------- Lobotomy E.G.O::Solemn Lament Yi Sang -------------------

  // Lobotomy E.G.O::Solemn Lament Yi Sang Skill 1 won
  if (isId(p1->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
      s1 == &p1->skills[0] && enemyCoins <= 0) {

    int value = 2;

    if (p2->hasSanity == 1 && p2->sanityGainBase >= 0) { // Normal

            updateSanity(p2, -(value));
        if (p2->Sanity < -45) p2->Sanity = -45;

      printf("\n%s won the Clash, %s loses %d Sanity (%d)\n", p1->name, p2->name, value, p2->Sanity);

      } else if (p2->hasSanity == 1 && p2->sanityGainBase < 0) { // Negative Sanity enemy

          updateSanity(p2, value);
        if (p2->Sanity > 45) p2->Sanity = 45;

      printf("\n%s won the Clash, %s gains %d Sanity (%d)\n", p1->name, p2->name, value, p2->Sanity);


     sleep(1);
  } else { // without Sanity
        p1->skills[0].active += value;

          printf("\n%s won the Clash, %s gains %d Butterfly (%d)\n", p1->name, p2->name, value, p1->skills[0].active);

         sleep(1);
    }

  }

  // Lobotomy E.G.O::Solemn Lament Yi Sang Skill 2 won
  if (isId(p1->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
      s1 == &p1->skills[1] && enemyCoins <= 0) {

    int value = 1;

      if (p2->hasSanity == 1 && p2->sanityGainBase >= 0) { // Normal

              updateSanity(p2, -(value));
          if (p2->Sanity < -45) p2->Sanity = -45;

        printf("\n%s won the Clash, %s loses %d Sanity (%d)\n", p1->name, p2->name, value, p2->Sanity);

        } else if (p2->hasSanity == 1 && p2->sanityGainBase < 0) { // Negative Sanity enemy

            updateSanity(p2, value);
          if (p2->Sanity > 45) p2->Sanity = 45;

        printf("\n%s won the Clash, %s gains %d Sanity (%d)\n", p1->name, p2->name, value, p2->Sanity);


       sleep(1);
    } else { // without Sanity
        p1->skills[0].active += value;

          printf("\n%s won the Clash, %s gains %d Butterfly (%d)\n", p1->name, p2->name, value, p1->skills[0].active);

         sleep(1);
    }

  }

  // Lobotomy E.G.O::Solemn Lament Yi Sang Skill 2 won
  if (isId(p1->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
      s1 == &p1->skills[2] && enemyCoins <= 0) {

    int value = 3;

      if (p2->hasSanity == 1 && p2->sanityGainBase >= 0) { // Normal

              updateSanity(p2, -(value));
          if (p2->Sanity < -45) p2->Sanity = -45;

        printf("\n%s won the Clash, %s loses %d Sanity (%d)\n", p1->name, p2->name, value, p2->Sanity);

        } else if (p2->hasSanity == 1 && p2->sanityGainBase < 0) { // Negative Sanity enemy

            updateSanity(p2, value);
          if (p2->Sanity > 45) p2->Sanity = 45;

        printf("\n%s won the Clash, %s gains %d Sanity (%d)\n", p1->name, p2->name, value, p2->Sanity);


       sleep(1);
    } else { // without Sanity
        p1->skills[0].active += value;

          printf("\n%s won the Clash, %s gains %d Butterfly (%d)\n", p1->name, p2->name, value, p1->skills[0].active);

         sleep(1);
    }

  }

  // Sukuna:King of Curse - skill 8 Clash lost
  if (isId(p2->ID, "Sukuna:King of Curse") == 0 && enemyCoins <= 0 && s2 == &p2->skills[7]) {

    p2->ProtectionDown[0] += 30;
    s2->DamageUp[0] -= 80;
    updateSanity(p2, -10);

    printf("\n%s lost the Clash, loses 10 Sanity (%d), deals -80%% damage and takes +30%% damage\n",
      p2->name, p2->Sanity);

    sleep(1);
  }

  // Sukuna:King of Curse - skill 6 Clash lost
  if (isId(p2->ID, "Sukuna:King of Curse") == 0 && enemyCoins <= 0 && (s2 == &p2->skills[5] || (s2 == &p2->skills[3] && p2->skills[3].Unbreakable > 0))) {

    p2->Paralyze[0] += 1;
    p2->FinalPowerDown[0] += 5;

    printf("\n%s lost the Clash, gains 5 Final Power Down and 1 Paralyze (Fix the Power of 1 Coins to 0 for one turn)\n",
      p2->name);

    sleep(1);
  }


  // -----------------------------------------------------------------

  // Jia Qiu S15 lost
  if (isId(p2->ID, "Jia Qiu") == 0 && enemyCoins <= 0 && s2 == &p2->skills[15]) {
    for (int i = 0; i < p2->numSkills; i++) {
      p2->skills[i].Defense -= 24;
    }
    s2->DamageUp[0] -= 50;

    printf("\n%s lost the Clash, Deal -50%% damage and Defense -24 next turn for this Encounter\n", p2->name);

     sleep(1);
  }

  // Meursault:Blade Lineage Mentor Skill 3 won
  if (isId(p1->ID, "Meursault:Blade Lineage Mentor") == 0 &&
      s1 == &p1->skills[2] && enemyCoins <= 0) {
    inflictStatus(p1->Poise, 5, 0, 0, 99, 0, 99);
    printf("\n%s won the Clash, gains 5 Poise Stack (%d)\n", p1->name, p1->Poise[0]);

     sleep(1);
  }

  // Meursault:Blade Lineage Mentor Skill 2 won
  if (isId(p1->ID, "Meursault:Blade Lineage Mentor") == 0 &&
      s1 == &p1->skills[1] && enemyCoins <= 0) {
    p1->FinalPowerUp[1] += 1;
    printf("\n%s won the Clash, gains 1 Final Power Up next turn\n", p1->name);

     sleep(1);
  }

  // Heathcliff:Wild Hunt – skill 3 heal sanity
  if (isId(p1->ID, "Heathcliff:Wild Hunt") == 0 &&
      s1 == &p1->skills[2] && enemyCoins <= 0) {
    updateSanity(p1, 10);
    if (p1->Sanity > 45) p1->Sanity = 45;
    printf("\n%s won the Clash, heals 10 Sanity (%d)\n", p1->name, p1->Sanity);

     sleep(1);
  }

  // Heathcliff:Wild Hunt – skill 3 heal sanity
  if (isId(p1->ID, "Heathcliff:Wild Hunt") == 0 &&
      s1 == &p1->defenseSkill[0] && enemyCoins <= 0) {
    p1->OffenseLevelUp[1] += clashCount/3;
    printf("\n%s won the Clash, Gain Offense Level Up next turn equal to (# of Clashes/3) (%d)\n", p1->name, clashCount/3);

     sleep(1);
  }

  //------------------------- Roland ---------------------------

  // gain Black Silence when lost
  if (isId(p2->ID, "Fixer grade 9?") == 0 && enemyCoins <= 0) {
    p2->Passive += 1;
    if (p2->Passive > 60) p2->Passive = 60;
    printf("\n%s lost the Clash, gains 1 Black Silence(%d - Max 60)\n", p2->name, p2->Passive);

     sleep(1);
  }

  // lose Black Silence
  if (isId(p2->ID, "Fixer grade 9?") == 0 &&
      (s2 == &p2->skills[8] || s2 == &p2->skills[1] || s2 == &p2->skills[2]) && enemyCoins <= 0) {
    p2->Passive -= 3;
    if (p2->Passive < 0) p2->Passive = 0;
    p2->ProtectionDown[0] += 20;
    printf("\n%s lost the Clash, loses 3 Black Silence(%d) and take 20%% more damage\n", p2->name, p2->Passive);

     sleep(1);
  }

  // lose Black Silence
  if (isId(p2->ID, "Fixer grade 9?") == 0 &&
      (s2 == &p2->skills[4]) && enemyCoins <= 0 && p2->Passive >= 3) {
    p2->Passive -= 3;
    if (p2->Passive < 0) p2->Passive = 0;
    p2->ProtectionUp[0] += 50;
    printf("\n%s lost the Clash, consumes 3 Black Silence(%d) and take 50%% less damage\n", p2->name, p2->Passive);

     sleep(1);
  }

  // Lost Black Silence Furioso
  if (isId(p2->ID, "Fixer grade 9?") == 0 &&
      (s2 == &p2->skills[9]) && enemyCoins <= 0) {
    p2->Passive -= 10;
    if (p2->Passive < 0) p2->Passive = 0;
    p2->ProtectionDown[0] += 40;
    printf("\n%s lost the Clash, loses 10 Black Silence (%d), take +40%% damage, and gains -5 Defense for this Encounter\n", p2->name, p2->Passive);

  for (int i = 0; i < p2->numSkills; i++) {
    if (s2 != &p2->skills[i]) {
    p2->skills[i].Defense -= 5;
    }
  }
    s2->Defense -= 5;

     sleep(1);
  }

  //-------------------------------------------------------------

  // ---------------------Sancho:The Second Kindred of Don Quixote ---------------------
  // Sancho:The Second Kindred of Don Quixote - Block
  if (isId(p2->ID, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
      p2->skills[12].active == 0 && p2->skills[13].active == 0 &&
      p2->Passive >= 5 && enemyCoins <= 0) {

    p2->Passive -= 5;
    if (p2->Passive < 1) p2->Passive = 1;

    p2->Shield += 25;
    p2->ProtectionUp[0] += 25;

    printf("\n%s consumes 5 Hardblood(%d left) to take -25%% damage and "
           "gain 25 Shield HP (%.2f)\n",
           p2->name, p2->Passive, p2->Shield + p2->TempShield);

     sleep(1);
  }
  // Sancho:The Second Kindred of Don Quixote - Skill 8 and 9 lost
  if (isId(p2->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      (s2 == &p2->skills[7] || s2 == &p2->skills[8]) && enemyCoins <= 0 && p2->Passive >= 4) {

    p2->Passive -= 4;
    if (p2->Passive < 1) p2->Passive = 1;
    p2->Shield += 20;

    printf("\n%s lost the Clash, consumes 4 Hardblood(%d) to gain 20 Shield HP (%.2f)\n", p2->name, p2->Passive, p2->Shield + p2->TempShield);

     sleep(1);
  }
  // Sancho:The Second Kindred of Don Quixote - Skill 10 lost
  if (isId(p2->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      (s2 == &p2->skills[9]) && enemyCoins <= 0) {
    s2->DamageUp[0] -= 50;
    p2->ProtectionDown[0] += 30;
    printf("\n%s lost the Clash, deals -50%% damage and takes 30%% more damage\n", p2->name);

     sleep(1);
  }
  // Sancho:The Second Kindred of Don Quixote - Skill 14 lost
  if (isId(p2->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      (s2 == &p2->skills[13]) && enemyCoins <= 0) {
    p2->Passive -= 5;
    if (p2->Passive < 1) p2->Passive = 1;
    printf("\n%s lost the Clash, loses 5 Hardblood(%d)\n", p2->name, p2->Passive);

     sleep(1);
  }
  //-----------------------------------------

  // Sukuna:King of Curse – Skill 4 lost
  if (isId(p2->ID, "Sukuna:King of Curse") == 0 && s2 == &p2->skills[3] &&
      enemyCoins <= 0) {

    updateSanity(p2, -(10));
    if (p2->Sanity < -45)
      p2->Sanity = -45;

    printf("\n%s loses 10 Sanity (%d)\n", p2->name, p2->Sanity);

    sleep(1);
  }


  // --------------------- Lei heng ---------------------
  // Lei heng – Skill 4 lost
  if (isId(p2->ID, "Lei heng") == 0 && s2 == &p2->skills[3] &&
      enemyCoins <= 0) {

    updateSanity(p2, -(10));
    if (p2->Sanity < -45)
      p2->Sanity = -45;

    printf("\n%s loses 10 Sanity (%d)\n", p2->name, p2->Sanity);

    sleep(1);
  } // Lei heng – Skill 3 lost
  else if (isId(p2->ID, "Lei heng") == 0 && s2 == &p2->skills[2] &&
      enemyCoins <= 0) {

    p2->FinalPowerDown[0] += 4;
    p2->Paralyze[0] += 1;

    printf("\n%s lost the Clash, gains 4 Final Power down and 1 Paralyze (Fix the Power of 1 Coins to 0 for one turn)\n", p2->name);

    sleep(1);
  } // Lei heng – Skill 6 lost
  else if (isId(p2->ID, "Lei heng") == 0 && s2 == &p2->skills[4] &&
      enemyCoins <= 0) {

    p2->FinalPowerDown[0] += 2;
    p2->Paralyze[0] += 6;

    printf("\n%s lost the Clash, gains 2 Final Power down and 6 Paralyze (Fix the Power of 6 Coins to 0 for one turn)\n", p2->name);

    sleep(1);

  }

  // Lei heng – Skill 3 and 6 lost
  if (isId(p2->ID, "Lei heng") == 0 && (s2 == &p2->skills[2] || s2 == &p2->skills[4]) &&
      enemyCoins <= 0) {

    p2->DamageUp[0] -= 80;

    printf("\n%s lost the Clash, deals -80%% damage; inflict only half the Stack and Count for both Burn and Tremor inflicted by this Skill (unless raised by other effects)\n", p2->name);

    sleep(1);
  }

  // Lei heng – Skill 1-2 2-2 and defense lost
  if (isId(p2->ID, "Lei heng") == 0 && ((s2 == &p2->skills[0] && p2->skills[0].active >= 1) || (s2 == &p2->skills[1] && p2->skills[0].active >= 1) || s2 == &p2->defenseSkill[0]) &&
      enemyCoins <= 0) {

    if (p2->defenseSkill[1].active > 0) {
      
    p2->defenseSkill[1].active -= 1;

    printf("\n%s lost the Clash, loses 1 Tigermark Round (%d)\n", p2->name, p2->defenseSkill[1].active);

    } else if (p2->defenseSkill[2].active > 0) {
      
      p2->defenseSkill[2].active -= 1;

      printf("\n%s lost the Clash, loses 1 Savage Tigermark Round (%d)\n", p2->name, p2->defenseSkill[2].active);
      
    }

    sleep(1);
  }

  // Lei heng (as enemy/p2) – Capo IIII: inflict Tremor Count
  if ((isId(p2->ID, "Lei heng") == 0 && enemyCoins <= 0)) {
    int tremorCount = 1 + (clashCount / 3);
    if (tremorCount > 3) tremorCount = 3;
    inflictStatus(p1->Tremor, 0, tremorCount, 0, 99, 0, 99);
    printf("\n%s inflicts +%d Tremor Count (%d)\n",
      p2->name, tremorCount, p1->Tremor[1]);
    sleep(1);
  }

  // Lei heng (as player/p1) – Capo IIII: inflict Tremor Count
  if ((isId(p1->ID, "Lei heng") == 0 && playerCoins <= 0)) {
    int tremorCount = 1 + (clashCount / 3);
    if (tremorCount > 3) tremorCount = 3;
    inflictStatus(p2->Tremor, 0, tremorCount, 0, 99, 0, 99);
    printf("\n%s inflicts +%d Tremor Count (%d)\n",
      p1->name, tremorCount, p2->Tremor[1]);
    sleep(1);
  }

  // Lei heng (as enemy/p2) – Prey: player (p1) wins clash → heal 20%% HP
  if (isId(p2->ID, "Lei heng") == 0 && p2->skills[6].active == 1 && enemyCoins <= 0) {
    int heal = (int)(p1->MAX_HP * 0.2f);
    if ((int)p1->HP + heal > (int)p1->MAX_HP) heal = (int)p1->MAX_HP - (int)p1->HP;
      p1->HP += heal;
      printf("\n%s won the Clash against %s, heals 20%% HP (%d - %.2f/%.2f)\n",
        p1->name, p2->name, (int)(p1->MAX_HP * 0.2f), p1->HP, p1->MAX_HP);
      sleep(1);
  }

  //-----------------------------------------

  // Dawn Office Fixer Sinclair base form S3 won
  if (isId(p1->ID, "Dawn Office Fixer Sinclair") == 0 &&
      (s1 == &p1->skills[3] || s1 == &p1->skills[2]) && enemyCoins <= 0) {

    updateSanity(p1, 10);
    if (p1->Sanity > 45) p1->Sanity = 45;

    printf("\n%s won the Clash, heals 10 Sanity (%d)\n", p1->name, p1->Sanity);

     sleep(1);
  }


  // Erlking Heathcliff – lost the Clash
  if (isId(p2->ID, "Erlking Heathcliff") == 0 && enemyCoins <= 0) {

    updateSanity(p2, -(5));
    if (p2->Sanity < -45) p2->Sanity = -45;
    p2->FinalPowerDown[1] += 1;

    printf("\n%s lost the Clash, loses 5 Sanity(%d) and gains 1 Final Power Down\n", p2->name, p2->Sanity);

     sleep(1);
  }



  // Jia Qiu – answer me lost the Clash
  if (isId(p2->ID, "Jia Qiu") == 0 && s2 == &p2->skills[14] && enemyCoins <= 0) {

    p2->skills[15].active += 1;

    if (isId(p1->ID, "Hong lu:The Lord of Hongyuan") == 0) {

      printf("\n%s lost the Clash, inflicts 1 Uncompromising Imposition (%d)\n", p2->name, p2->skills[15].active);

    } else {

       printf("\n%s lost the Clash, inflicts 1 Dialogues (%d)\n", p2->name, p2->skills[15].active);

    }

}


// --------------------- King in Binds --------------------------------

// King in Binds skill 2, 3, 4, 5 lose
if (isId(p2->ID, "King in Binds") == 0 && (s2 == &p2->skills[1] || s2 == &p2->skills[2] || s2 == &p2->skills[3] || s2 == &p2->skills[4]) && enemyCoins <= 0) {

  int ReduceValue = 2;
  if (s2 == &p2->skills[1]) ReduceValue = 1;
  if (s2 == &p2->skills[4]) ReduceValue = 4;

    p1->Sinking[1] -= ReduceValue;
  if (p1->Sinking[1] <= 0) { p1->Sinking[1] = 0; p1->Sinking[0] = 0; }

  printf("\n%s lost the Clash, reduce Sinking Count on target by %d (%d)\n", p2->name, ReduceValue, p1->Sinking[1]);

   sleep(1);

  if (s2 == &p2->skills[3]) {

    updateSanity(p1, -10);

    printf("\n%s lost the Clash, %s loses 10 Sanity (%d)\n", p2->name, p1->name, p1->Sanity);

     sleep(1);

  }
}

// ------------------------------------------------------------




// ------------------------------------------------------------------------------------------------------------------------------------













  // ------------------------------ enemy win -----------------------------------------------------------------------------------------

  // ------------------ The Middle Nursefather - Matthias ------------------

  // The Middle Nursefather - Matthias - Passive Win
  if (isId(p1->ID, "The Middle Nursefather - Matthias") == 0 && playerCoins <= 0) {
      p1->skills[1].active += 1; // Gain 1 Kiddo
      if (p1->skills[1].active > 10) p1->skills[1].active = 10;

    printf("\n%s gains 1 'Check This Out, Kiddo!' (%d)\n", p1->name, p1->skills[1].active);

    sleep(1);

      // ถ้าเป็นการชนะครั้งแรกของเทิร์น
      if (p1->skills[5].active == 0 && p1->skills[8].active == 0 && playerCoins <= 0) {
          p1->skills[5].active = 1;
         p1->skills[8].active = 3; // ตั้งค่า Cooldown ไว้ 3 เทิร์น (เทิร์นนี้และอีก 2 เทิร์นหน้า)
          p1->skills[1].active += 1; // Gain extra Kiddo
        if (p1->skills[1].active > 10) p1->skills[1].active = 10;
          s1->CoinPowerBoost[0] += 1; // Gain Coin Power +1 for THIS skill
          printf("\n%s gains 1 'Check This Out, Kiddo!' (%d) and +1 Coin Power for this skill\n", p1->name, p1->skills[1].active);

        sleep(1);
      }
  }

  // The Middle Nursefather - Matthias - Skill 1
  if (isId(p2->ID, "The Middle Nursefather - Matthias") == 0 && enemyCoins <= 0 && (s2 == &p2->skills[0] || s2 == &p2->skills[7])) {

    if (p2->skills[9].active == 1) {

      p2->ClashPowerUp[1] += 1;

      printf("\n%s lost the Clash, gain 1 Clash Power Up Next turn\n", p2->name);

      sleep(1);

    }

    if ((p2->skills[9].active >= 2)) {

      p2->BasePowerUp[1] += 1;

      printf("\n%s lost the Clash, gain 1 Base Power Up Next turn\n", p2->name);

      sleep(1);

    }

  }

  // The Middle Nursefather - Matthias - Skill 2
  if (isId(p2->ID, "The Middle Nursefather - Matthias") == 0 && enemyCoins <= 0 && s2 == &p2->skills[1]) {

    if ((p2->skills[9].active >= 1)) {

      p2->DamageUp[1] += 10;

      printf("\n%s lost the Clash, gain +10%% Damage Up Next turn\n", p2->name);

      sleep(1);

    }

  }

  // The Middle Nursefather - Matthias - skill 7 8 9 Clash lost
  if (isId(p2->ID, "The Middle Nursefather - Matthias") == 0 && enemyCoins <= 0 && (s2 == &p2->skills[7] || s2 == &p2->skills[8] || s2 == &p2->skills[9])) {

    p2->DamageDown[0] += 80;

    printf("\n%s lost the Clash, deals -80%% damage\n",
      p2->name);

    sleep(1);
  }

  // The Middle Nursefather - Matthias - Skill 7 8 9
  if (isId(p1->ID, "The Middle Nursefather - Matthias") == 0 && playerCoins <= 0 && (s1 == &p1->skills[7] || s1 == &p1->skills[8] || s1 == &p1->skills[9])) {

    int gain = clashCount * 5;
    if (gain > 15) gain = 15;

    updateSanity(p1, gain);

      printf("\n%s won the Clash, heal %d Sanity (%d)\n", p1->name, gain, p1->Sanity);

      sleep(1);

  }

  // ------------------------------------------------------------------------

  //---------------Sancho:The Second Kindred of Don Quixote ---------------------

  // Sancho:The Second Kindred of Don Quixote - Skill 7 won
  if (isId(p1->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    s1 == &p1->skills[6] && playerCoins <= 0 && p1->HP > 1) {

    printf("\n%s won the Clash, consumes 4%% of Max HP(%d) to gain 1 Hardblood(%d) and deal 25%% more damage (this damage does not lower the unit's HP below 1)\n",
      p1->name, (int)(p1->MAX_HP * 0.04), p1->Passive);

        p1->HP -= (int)(p1->MAX_HP * 0.04);
    if (p1->HP < 1) p1->HP = 1;

        s1->DamageUp[0] += 25;
      p1->Passive += 1;

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Skill 8 and 9 won
  if (isId(p1->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    (s1 == &p1->skills[7] || s1 == &p1->skills[8]) && playerCoins <= 0) {

    printf("\n%s won the Clash, deals 20%% more damage and more 20%% damage for every 10 Hardblood(%d)\n",
      p1->name, p1->Passive);

      s1->DamageUp[0] += 20 * ((p2->Passive / 10) + 1);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Skill 10 won
  if (isId(p1->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    (s1 == &p1->skills[9]) && playerCoins <= 0) {

    int ShieldGain = (int)(p1->MAX_HP * 0.15);

      p1->Shield += ShieldGain;

    printf("\n%s won the Clash, gains %d Shield HP (%.2f)\n",
      p1->name, ShieldGain, p1->Shield + p1->TempShield);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Skill 14 won
  if (isId(p1->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      (s1 == &p1->skills[13]) && playerCoins <= 0) {
      s1->DamageUp[0] += 10;
    if (p1->Passive < 1) p1->Passive = 1;
    printf("\n%s won the Clash, deal +10%% damage\n", p1->name);

     sleep(1);
  }




  // Sukuna:King of Curse - Passive Clash win
  if (isId(p1->ID, "Sukuna:King of Curse") == 0 && playerCoins <= 0) {

      p1->AttackPowerUp[1] += 5;

    printf("\n%s won the Clash, gains 5 Attack Power Up next turn\n",
      p1->name);

    sleep(1);
  }

  // Sukuna:King of Curse - skill 8 Clash win
  if (isId(p1->ID, "Sukuna:King of Curse") == 0 && playerCoins <= 0 && s1 == &p1->skills[7]) {

    updateSanity(p1, 10);
    if (p1->Sanity > 45) p1->Sanity = 45;

    printf("\n%s won the Clash, heals 10 Sanity (%d)\n",
      p1->name, p1->Sanity);

    sleep(1);
  }



  //------------------------------------------------------------------

  //-------------------------- Lei heng ------------------------------
  // Lei heng – skill 3
  if (isId(p1->ID, "Lei heng") == 0 && (s1 == &p1->skills[2]) &&
    playerCoins <= 0) {

    printf(
        "\n%s: Aw heck, don't tell me ny'all are tuckered out already!\n",
        p2->name);

    sleep(1);
  }

  // Lei heng – inner strength gain
  if (isId(p1->ID, "Lei heng") == 0 && p1->skills[0].active == 2 &&
      playerCoins <= 0) {

    p1->Passive += clashCount;
    if (p1->Passive >= 25)
      p1->Passive = 25;

    printf("\n%s gains +%d Inner Strength [底力](%d - Max 25)\n", p1->name,
           clashCount, p1->Passive);

     sleep(1);

  } else if (isId(p1->ID, "Lei heng") == 0 &&
    p1->skills[0].active == 3 && playerCoins <= 0) {

    p1->Passive += clashCount * 2;
    if (p1->Passive >= 50)
      p1->Passive = 50;

    printf("\n%s gains +%d Extreme Strength [極力](%d - Max 50)\n",
           p1->name, clashCount * 2, p1->Passive);

     sleep(1);
  }

  //-------------------------------------------------------------

  //-------------------------- Erlking Heathcliff ------------------------------

  // Erlking Heathcliff – won the Clash
  if (isId(p1->ID, "Erlking Heathcliff") == 0 && playerCoins <= 0) {

    updateSanity(p1, 5);
    if (p1->Sanity > 45) p1->Sanity = 45;

    printf("\n%s won the Clash, heals 5 Sanity (%d)\n", p1->name, p1->Sanity);

     sleep(1);
  }

  // Erlking Heathcliff – skill 5
  if (isId(p1->ID, "Erlking Heathcliff") == 0 && (s1 == &p1->skills[4]) &&
    playerCoins <= 0) {

    p1->DamageUp[1] += 10;
    p1->FinalPowerUp[1] += 1;

    printf(
        "\n%s won the Clash, gains 1 Final Power Up and deal 10%% more damage next turn\n",
        p1->name);

    sleep(1);
  }

  // Erlking Heathcliff – Skill 6
  if (isId(p1->ID, "Erlking Heathcliff") == 0 && s1 == &p1->skills[5] &&
      playerCoins <= 0) {

    updateSanity(p1, -(5));

    printf(
        "\n%s loses 5 Sanity from %s's Skill (%d)\n",
        p2->name, p1->name, p2->Sanity);

    sleep(1);
  }

  //-------------------------------------------------------------






  //------------------------- Roland ---------------------------

  // gain Black Silence when won
  if (isId(p1->ID, "Fixer grade 9?") == 0 && playerCoins <= 0) {
    p1->Passive += 3;
    if (p1->Passive > 60) p1->Passive = 60;
    printf("\n%s won the Clash, gains 3 Black Silence(%d - Max 60)\n", p1->name, p1->Passive);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && p1->Passive >= 3 && playerCoins <= 0 && s1 == &p1->skills[1]) {
    p1->Passive -= 3;
    s1->DamageUp[0] += 50;
    if (p1->Passive < 0) p1->Passive = 0;
    printf("\n%s won the Clash, consumes 3 Black Silence(%d) to deal 50%% more damage\n", p1->name, p1->Passive);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && p1->Passive >= 5 && playerCoins <= 0 && s1 == &p1->skills[3]) {
    p1->Passive -= 5;
    if (p1->Passive < 0) p1->Passive = 0;
    p2->Paralyze[1] += 2;
    printf("\n%s won the Clash, consumes 5 Black Silence(%d) to inflicts 2 Paralyze next turn (Fix the Power of 2 Coins to 0 for one turn)\n", p1->name, p1->Passive);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && p1->Passive >= 2 && playerCoins <= 0 && s1 == &p1->skills[5]) {
    p1->Passive -= 3;
    if (p1->Passive < 0) p1->Passive = 0;
    p2->ProtectionDown[0] += 20;
    printf("\n%s won the Clash, consumes 3 Black Silence(%d), %s take 20%% more damage next turn\n", p1->name, p1->Passive, p2->name);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && p1->Passive >= 2 && playerCoins <= 0 && (s1 == &p1->skills[0] || s1 == &p1->skills[4])) {
    p1->Passive -= 2;
    if (p1->Passive < 0) p1->Passive = 0;
    p1->DamageUp[1] += 15;
    printf("\n%s won the Clash, consumes 2 Black Silence(%d) to gain 15%% Damage Up next turn\n", p1->name, p1->Passive);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && playerCoins <= 0 && (s1 == &p1->skills[8])) {
    p1->Passive += 3;
    if (p1->Passive > 60) p1->Passive = 60;
    p1->ClashPowerUp[1] += 1;
    printf("\n%s won the Clash, gains 3 Black Silence(%d - Max 60) and Clash Power +1 next turn\n", p1->name, p1->Passive);

     sleep(1);
  }

  //------------------------------------------------------------

  // Dawn Office Fixer Sinclair base form S3 lost
  if (isId(p2->ID, "Dawn Office Fixer Sinclair") == 0 && !p2->skills[3].active &&
     (s2 == &p2->skills[3] || s2 == &p2->skills[2]) && playerCoins <= 0) {

    updateSanity(p2, -(10));
    if (p2->Sanity < -45) p2->Sanity = -45;

    printf("\n%s lost the Clash, loses 10 Sanity (%d)\n", p2->name, p2->Sanity);

     sleep(1);
  }

  // Dawn Office Fixer Sinclair EGO form S3 lost
  if (isId(p2->ID, "Dawn Office Fixer Sinclair") == 0 && p2->skills[3].active &&
      (s2 == &p2->skills[3] || s2 == &p2->skills[2]) && playerCoins <= 0) {

    updateSanity(p2, -(30));
    if (p2->Sanity < -45) p2->Sanity = -45;

    printf("\n%s lost the Clash, loses 30 Sanity (%d)\n", p2->name, p2->Sanity);

     sleep(1);
  }




// --------------------- King in Binds --------------------------------

// King in Binds skill 1, 2, 3, 5 win
if (isId(p1->ID, "King in Binds") == 0 && (s1 == &p1->skills[0] || s1 == &p1->skills[1] || s1 == &p1->skills[2] || s1 == &p1->skills[4]) && playerCoins <= 0) {

  int InfilctValue = 3;
  if (s1 == &p1->skills[2]) InfilctValue = 4;
  if (s1 == &p1->skills[4]) InfilctValue = 2;

  inflictStatus(p2->Sinking, 0, InfilctValue, 0, 99, 0, 99);

  printf("\n%s won the Clash, infilct +%d Sinking Count (%d)\n", p1->name, InfilctValue, p2->Sinking[1]);

   sleep(1);
}

// ------------------------------------------------------------


}












//-----------------------Clashing phase-------------------------------
ClashResult clashPhase(Character *p1, SkillStats *s1, int playerTempOffense,
                       int playerTempDefense, Character *p2, SkillStats *s2,
                       int enemyTempOffense, int enemyTempDefense,
                       Character *fullPlayer, int PContinueUnbreakCoin, int EContinueUnbreakCoin) {

  printf("\n--- Clash Phase ---\n");

  // --------------------- Clash Phase ----------------------

  applyClashStartPassives(p1, s1, p2, s2); // รันให้ฝั่งซ้าย
  applyClashStartPassives(p2, s2, p1, s1); // รันให้ฝั่งขวา

  if (PContinueUnbreakCoin <= 0) {
    s1->Unbreakable += s1->UnbreakableUp[0];
    if (s1->Unbreakable > s1->Coins) s1->Unbreakable = s1->Coins;
  }
  if (EContinueUnbreakCoin <= 0) {
    s2->Unbreakable += s2->UnbreakableUp[0];
    if (s2->Unbreakable > s2->Coins) s2->Unbreakable = s2->Coins;
  }

  int PCoinBoost = 0;
  if (s1->CoinPower >= 0) {
      PCoinBoost = p1->PlusCoinPowerBoost[0] - p1->PlusCoinPowerDrop[0];
  } else {
      PCoinBoost = p1->MinusCoinPowerDrop[0] - p1->PlusCoinPowerBoost[0];
  }
  
  if (s1->Unbreakable > 0) {
    printf("\nPlayer: '%s' | BasePower %d | CoinPower %d | Coins %d | Offense %d | "
           "Defense %d | Unbreakable %d\n",
           s1->name, s1->BasePower + s1->BasePowerBoost[0] + (p1->BasePowerUp[0] - p1->BasePowerDown[0]),
           s1->CoinPower + s1->CoinPowerBoost[0] + PCoinBoost, s1->Coins, playerTempOffense,
           playerTempDefense, s1->Unbreakable);
  } else {
    printf("\nPlayer: '%s' | BasePower %d | CoinPower %d | Coins %d | Offense %d | "
           "Defense %d | Breakable\n",
      s1->name, s1->BasePower + s1->BasePowerBoost[0] + (p1->BasePowerUp[0] - p1->BasePowerDown[0]),
         s1->CoinPower + s1->CoinPowerBoost[0] + PCoinBoost, s1->Coins, playerTempOffense,
           playerTempDefense);
  }

  int ECoinBoost = 0;
  if (s1->CoinPower >= 0) {
      ECoinBoost = p1->PlusCoinPowerBoost[0] - p1->PlusCoinPowerDrop[0];
  } else {
      ECoinBoost = p1->MinusCoinPowerDrop[0] - p1->PlusCoinPowerBoost[0];
  }
  
  if (s2->Unbreakable > 0) {
    printf("Enemy: '%s' | BasePower %d | CoinPower %d | Coins %d | Offense %d | "
           "Defense %d | Unbreakable %d\n",
      s2->name, s2->BasePower + s2->BasePowerBoost[0] + (p2->BasePowerUp[0] - p2->BasePowerDown[0]),
         s2->CoinPower + s2->CoinPowerBoost[0] + ECoinBoost, s2->Coins, enemyTempOffense,
           enemyTempDefense, s2->Unbreakable);
  } else {
    printf("Enemy: '%s' | BasePower %d | CoinPower %d | Coins %d | Offense %d | "
           "Defense %d | Breakable\n",
      s2->name, s2->BasePower + s2->BasePowerBoost[0] + (p2->BasePowerUp[0] - p2->BasePowerDown[0]),
         s2->CoinPower + s2->CoinPowerBoost[0] + ECoinBoost, s2->Coins, enemyTempOffense,
           enemyTempDefense);
  }

  int playerCoins = s1->Coins;
  int enemyCoins = s2->Coins;
  int round = 1;
  int clashCount = 0; // Add this: track total clash rounds
  int playerLostWithSkill3 = 0;

  int playerUnbreakableLost = 0;
  int enemyUnbreakableLost = 0;

  // ContinueUnbreakCoin
  if (PContinueUnbreakCoin > 0) {
    playerUnbreakableLost = PContinueUnbreakCoin;
  }
  if (EContinueUnbreakCoin > 0) {
      enemyUnbreakableLost = EContinueUnbreakCoin;
  }



  // --- Counter Yield My Flesh ---
  // เช็คว่า "ผู้แพ้" คือ Meursault และใช้ Skill 3 หรือไม่
  Character *Winner = NULL;
  SkillStats *WinnerSkill = NULL;
  Character *Loser = NULL;
  SkillStats *LoserSkill = NULL;


  int playerTotal = 0;
  int enemyTotal = 0;

  while (playerCoins > 0 && enemyCoins > 0 && p1->HP > 0 && p2->HP > 0) {

    // ----------------- On Clashing buff --------------------

    if (isId(p2->ID, "King in Binds") == 0) {
        int Clashpowerbuff = (clashCount + 1)/2;

      if (Clashpowerbuff > 0) {

          printf("\n%s gains +%d Clash Power\n", p2->name, Clashpowerbuff);

      }

    }

    // --------------------------------------------------

    printf("\nRound %d:\nPlayer:\t\tEnemy:\n", round);

    clashCount++; // Increment clash count each round

    int maxToss = (playerCoins > enemyCoins) ? playerCoins : enemyCoins;

    playerTotal = s1->BasePower + s1->BasePowerBoost[0] + (p1->BasePowerUp[0] - p1->BasePowerDown[0]);
    enemyTotal = s2->BasePower + s2->BasePowerBoost[0] + (p2->BasePowerUp[0] - p2->BasePowerDown[0]);

    // ใช้การคูณลดทอน
    double roundDelay = 0.15;
    for (int r = 1; r < round; r++) {
        roundDelay *= 0.8; // ยิ่ง Round สูง ค่า Delay ยิ่งน้อยลงแบบทวีคูณ
    }

    if (roundDelay < 0.02) 
        roundDelay = 0.02;

    printf("%-10d %-10d\n", playerTotal, enemyTotal);
    usleep((int)(roundDelay * 500000));

    // ----------------------- In coin toss section -----------------------
    for (int i = 0; i < maxToss; i++) {

      // ----------------------- Before tossing -----------------------
      int PlayerCoinBuff = 0;
      int EnemyCoinBuff = 0;

      int *targetBuffVars[2] = {&PlayerCoinBuff, &EnemyCoinBuff};
      Character *sideChars[2] = {p1, p2};
      SkillStats *sideSkills[2] = {s1, s2};
      int sideRemCoins[2] = {playerCoins, enemyCoins}; // จำนวนเหรียญที่เหลือใน Round นี้

      for (int side = 0; side < 2; side++) {
          Character *c = sideChars[side];
          SkillStats *s = sideSkills[side];
          int rem = sideRemCoins[side]; // จำนวนเหรียญที่เหลืออยู่ตอนนี้

        if (isId(c->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && (s == &c->skills[4] || s == &c->skills[5]) && c->skills[10].active > 0 && c->skills[12].active >= 5 && i < s->Coins) {
            int pwr = 1;
            *(targetBuffVars[side]) = pwr;
            printf("%s: this Coin Power +%d\n", c->name, pwr);
        }

          if (i < rem && isId(c->ID, "Meursault:The Thumb") == 0) {

              // 1. ระบุจำนวนนัดที่สกิลนี้ "ต้องการ" (S1=1, S2=2, S3=3, S4=3)
              int ammoNeeded = (s == &c->skills[0]) ? 1 : (s == &c->skills[1] ? 2 : (s == &c->skills[2] ? 3 : 0));
              if (s == &c->skills[3]) ammoNeeded = 3;

              // 2. คำนวณจุดเริ่มใช้กระสุน โดยอิงจาก "เหรียญที่เหลืออยู่จริง (rem)"
              // สูตร: เหรียญสุดท้ายตอนนี้ - (จำนวนที่ต้องการ - 1)
              int startConsumingIdx = rem - ammoNeeded; 
              if (startConsumingIdx < 0) startConsumingIdx = 0; // ถ้าเหรียญเหลือน้อยกว่าจำนวนที่ใช้ ให้เริ่มที่เหรียญแรก

              // 3. ตรวจสอบว่าเหรียญปัจจุบัน (i) อยู่ในช่วงที่ใช้กระสุนไหม
              if (i >= startConsumingIdx) {
                  // ลำดับนัดที่ i (นับจากจุดเริ่มใช้กระสุน)
                  int ammoOrder = i - startConsumingIdx; 

                  // 4. Priority Logic: ถ้ากระสุน (Passive) มีพอสำหรับลำดับนี้ ให้บัฟ
                  if (ammoOrder < c->Passive) {
                      int pwr = c->skills[3].active ? 2 : 1;
                      *(targetBuffVars[side]) = pwr; 
                      printf("%s: this Coin Power +%d\n", c->name, pwr);
                  }
              }
          }
      }
      // -----------------------------------------------------------------------

      // ------------------------------------------------------------------------------------------


      if (i < playerCoins && tossCoinWithSanity(p1)) { // Pass character
        // Check paralyze
        if (p1->Paralyze[0] > 0) { // ← Character's paralyze
          playerTotal += 0;
          p1->Paralyze[0]--; // ← Character's paralyze
        } else {
          {
            int charClashBoost1 = 0;
            if (s1->CoinPower >= 0) {
              charClashBoost1 += p1->PlusCoinPowerBoost[0] - p1->PlusCoinPowerDrop[0];
            } else {
              charClashBoost1 += p1->MinusCoinPowerBoost[0] - p1->MinusCoinPowerDrop[0];
            }
            playerTotal += PlayerCoinBuff + s1->CoinPower + s1->CoinPowerBoost[0] + charClashBoost1;
          }
          if (playerTotal <= 0) playerTotal = 0;
        }
      }
      if (i < enemyCoins && tossCoinWithSanity(p2)) { // Pass character
        // Check paralyze
        if (p2->Paralyze[0] > 0) { // ← Character's paralyze
          enemyTotal += 0;
          p2->Paralyze[0]--; // ← Character's paralyze
        } else {
          {
            int charClashBoost2 = 0;
            if (s2->CoinPower >= 0) {
              charClashBoost2 += p2->PlusCoinPowerBoost[0] - p2->PlusCoinPowerDrop[0];
            } else {
              charClashBoost2 += p2->MinusCoinPowerBoost[0] - p2->MinusCoinPowerDrop[0];
            }
            enemyTotal += EnemyCoinBuff + s2->CoinPower + s2->CoinPowerBoost[0] + charClashBoost2;
          }
          if (enemyTotal <= 0) enemyTotal = 0;
        }
      }

      // Meursault:Blade Lineage Mentor's Passive
      if (!(strcmp(s1->name, "Yield My Flesh") == 0)) {

      // Last coin offense bonus
      int bonus = 0;

      // Check if this is player's last coin
      if (i == playerCoins - 1) {

        if (s1->skillType == 4) { // เราใช้ท่าป้องกันที่ Clash ได้
          bonus += (playerTempDefense - enemyTempOffense) / 3;
        } else if (s2->skillType == 4) { // เราใช้ท่าโจมตีปกติ แต่ศัตรูใช้ท่าป้องกันที่ Clash ได้
          bonus += (playerTempOffense - enemyTempDefense) / 3;
        } else { // โจมตีปกติ vs โจมตีปกติ
          bonus += (playerTempOffense - enemyTempOffense) / 3;
        }

        if (bonus < 0) bonus = 0; // ต้องห้ามติดลบตามกฎเกม

        // Calculate player's clash bonus
        if (s1->skillType != 0) { // ถ้าเป็น Clashable Guard or Counter

          // บวกโบนัสจาก DefensePowerBoost และ ClashPower/FinalPower ตามปกติ
          bonus += (s1->ClashPower[0] + s1->FinalPowerBoost[0]) + ((p1->FinalPowerUp[0] - p1->FinalPowerDown[0]) + (p1->ClashPowerUp[0] - p1->ClashPowerDown[0])) + (p1->DefenseSkillPowerUp[0] - p1->DefenseSkillPowerDown[0]);
        } else { // ถ้าเป็น Attack ปกติ (โค้ดเดิมของคุณ)
          bonus += (s1->ClashPower[0] + s1->FinalPowerBoost[0]) + ((p1->ClashPowerUp[0] - p1->ClashPowerDown[0]) + (p1->FinalPowerUp[0] - p1->FinalPowerDown[0]) + (p1->AttackSkillPowerUp[0] - p1->AttackSkillPowerDown[0])) ;
        }

        if (bonus != 0) {
          playerTotal += bonus;
          if (playerTotal <= 0) playerTotal = 0;
          printf("Player Clash Power bonus applied: %d\n", bonus);
        }
      }

    }

      // Meursault:Blade Lineage Mentor's Passive
      if (!(strcmp(s2->name, "Yield My Flesh") == 0)) {

        // Last coin offense bonus
        int bonus = 0;

        // King in Binds Passive
        int kingPassiveP2 = (isId(p2->ID, "King in Binds") == 0) ? (clashCount / 2) : 0;

      // Check if this is enemy's last coin
      if (i == enemyCoins - 1) {

        if (s2->skillType == 4) {
          // ศัตรูใช้ท่าป้องกันที่ Clash ได้ (เขาป้องกัน vs เราโจมตี)
            bonus += (enemyTempDefense - playerTempOffense) / 3;
        } else if (s1->skillType == 4) { 
          // ศัตรูใช้ท่าโจมตีปกติ แต่เราใช้ท่าป้องกันที่ Clash ได้ (เขาโจมตี vs เราป้องกัน)
            bonus += (enemyTempOffense - playerTempDefense) / 3;
        } else { 
          // โจมตีปกติ vs โจมตีปกติ
            bonus += (enemyTempOffense - playerTempOffense) / 3;
        }

        if (bonus < 0) bonus = 0; // ต้องห้ามติดลบตามกฎเกม

        // Calculate enemy's clash bonus
        if (s2->skillType != 0) { // ถ้าเป็น Clashable Guard
          bonus += (s2->ClashPower[0] + s2->FinalPowerBoost[0]) + ((p2->FinalPowerUp[0] - p2->FinalPowerDown[0]) + (p2->ClashPowerUp[0] - p2->ClashPowerDown[0])) + (p2->DefenseSkillPowerUp[0] - p2->DefenseSkillPowerDown[0]);
        } else { // ถ้าเป็น Attack ปกติ (โค้ดเดิมของคุณ)
          bonus += (s2->ClashPower[0] + s2->FinalPowerBoost[0]) + ((p2->ClashPowerUp[0] - p2->ClashPowerDown[0]) + (p2->FinalPowerUp[0] - p2->FinalPowerDown[0]) + (p2->AttackSkillPowerUp[0] - p2->AttackSkillPowerDown[0])) + kingPassiveP2;
        }

        if (bonus != 0) {
          enemyTotal += bonus;
          if (enemyTotal <= 0) enemyTotal = 0;
          printf("Enemy Clash Power bonus applied: %d\n", bonus);
        }

      }

      }

      usleep((int)(roundDelay * 500000));

      if (i < playerCoins && i < enemyCoins) {
        printf("%-10d %-10d\n", playerTotal, enemyTotal);
      } else if (i < playerCoins && i >= enemyCoins) {
          printf("%-10d %-10s\n", playerTotal, "");
        } else if (i >= playerCoins && i < enemyCoins) {
        printf("%-10s %-10d\n", "", enemyTotal);
      } 

      usleep((int)(roundDelay * 1000000));

      // ----------------------- After tossing -----------------------

      // ----------------------------------------------

    }




    // Determine clash result
    if (playerTotal > enemyTotal && enemyCoins > 0) {
      printf("Player wins this clash! Enemy loses 1 coin.\n");
      enemyCoins--;


      if (enemyCoins > 0) {
        usleep((int)(roundDelay * 5000000));
      } else {
      sleep(1);
      }


      if (isId(p1->ID, "Muga Ryōshū") == 0) {
        int chance = 0;
        if (s1 == &p1->skills[2] || s1 == &p1->skills[3]) chance = 25 + (p1->Passive/3) + (p1->skills[0].active/3);
        else if (s1 == &p1->skills[4]) chance = 50 + (p1->Passive/3) + (p1->skills[0].active/3);

        if ((rand() % 100) < chance) {
            severCoin(p1, p2, s1, s2); // p1 ชนะ, ตัดเหรียญ s2, และทำลายชื่อ s1 ของตัวเอง
          sleep(1);
        }
      }


      // -------------- Counter --------------

      if (enemyCoins <= 0) {

        ClashResult counterResult = ClashableCounter(p1, s1, playerTempOffense, playerTempDefense, playerCoins, 
         p2, s2, enemyTempOffense, enemyTempDefense, enemyCoins, 
         fullPlayer, playerUnbreakableLost, enemyUnbreakableLost);

          // ถ้า winner ไม่เป็น 0 แสดงว่ามีการ Counter เกิดขึ้นจริงๆ
          if (counterResult.winner != 0) {
              return counterResult; // จบการ Clash ปัจจุบัน แล้วส่งผลจากการ Counter กลับไปเลย
          }
      }

      // ---------------------------------------

      // Update Sanity based on clash count
      if (p1->hasSanity && enemyCoins <= 0) {
        int gain = calculateSanityGain(p1, clashCount);
        updateSanity(p1, gain);

        if (ClashPity) ClashPity = 0;

        if (gain < 0) {
          printf("\n%s loses %d Sanity (Sanity %d Clash %d)", p1->name, -gain, p1->Sanity, clashCount);
        } else {
          printf("\n%s gains %d Sanity (Sanity %d Clash %d)", p1->name, gain, p1->Sanity, clashCount);
        }
      }

      if (p2->hasSanity && enemyCoins <= 0) {
        int loss = calculateSanityLoss(p2, clashCount);
        updateSanity(p2, -(loss));

        if (loss >= 0) {
          printf("\n%s loses %d Sanity (Sanity %d Clash %d)\n", p2->name, loss, p2->Sanity, clashCount);
        } else {
          printf("\n%s gains %d Sanity (Sanity %d Clash %d)\n", p2->name, -loss, p2->Sanity, clashCount);
        }
      } else if (!p2->hasSanity && enemyCoins <= 0) {
        printf("\n");
      }





       //Unbreakable lost
      if (s2->Unbreakable > 0 && enemyUnbreakableLost < s2->Unbreakable && s2->Unbreakable > enemyCoins) {
        enemyUnbreakableLost++;
      }





      // เรียกใช้ฟังก์ชันเดียวจบ (ส่ง Player เป็นคนชนะ, Enemy เป็นคนแพ้)
      applyClashRoundResult(p1, s1, p2, s2, playerCoins, enemyCoins, clashCount); // Coin need to stay same array, i am too lazy to fix it






      if (isId(p2->ID, "Meursault:Blade Lineage Mentor") == 0 && s2 == &p2->skills[2] && enemyCoins <= 0) {
        Loser = p2; LoserSkill = s2;
      }





    } else if (enemyTotal > playerTotal && playerCoins > 0) {
      printf("Enemy wins this clash! Player loses 1 coin.\n");
      playerCoins--;




        if (playerCoins > 0) {
          usleep((int)(roundDelay * 5000000));
        } else {
       sleep(1);
        }





      if (isId(p2->ID, "Muga Ryōshū") == 0) {
        int chance = 0;
        if (s2 == &p2->skills[2] || s2 == &p2->skills[3]) chance = 25 + (p2->Passive/3) + (p2->skills[0].active/3);
        else if (s2 == &p2->skills[4]) chance = 50 + (p2->Passive/3) + (p2->skills[0].active/3);

        if ((rand() % 100) < chance) {
            severCoin(p2, p1, s2, s1); // บอส Ryoshu ชนะ, ตัดเหรียญ s1, และทำลายชื่อ s2
          sleep(1);
        }
      }



      // -------------- Counter --------------

      if (playerCoins <= 0) {

        ClashResult counterResult = ClashableCounter(p2, s2, enemyTempOffense, enemyTempDefense, enemyCoins, 
          p1, s1, playerTempOffense, playerTempDefense, playerCoins, 
         fullPlayer, enemyUnbreakableLost, playerUnbreakableLost);

          // ถ้า winner ไม่เป็น 0 แสดงว่ามีการ Counter เกิดขึ้นจริงๆ
          if (counterResult.winner != 0) {
              return counterResult; // จบการ Clash ปัจจุบัน แล้วส่งผลจากการ Counter กลับไปเลย
          }
      }

      // ---------------------------------------



      // Update Sanity based on clash count
      if (p2->hasSanity && playerCoins <= 0) {
        int gain = calculateSanityGain(p2, clashCount);
        updateSanity(p2, gain);

        if (gain < 0) {
          printf("\n%s loses %d Sanity (Sanity %d Clash %d)", p2->name, -gain, p2->Sanity, clashCount);
        } else {
          printf("\n%s gains %d Sanity (Sanity %d Clash %d)", p2->name, gain, p2->Sanity, clashCount);
        }
      }

      if (p1->hasSanity && playerCoins <= 0) {

         int loss = calculateSanityLoss(p1, clashCount);

         if (ClashPity) {
            loss = 0;
          }

        updateSanity(p1, -(loss));

        if (loss >= 0) {
          printf("\n%s loses %d Sanity (Sanity %d Clash %d)\n", p1->name, loss, p1->Sanity, clashCount);
        } else if (loss < 0) {
           printf("\n%s gains %d Sanity (Sanity %d Clash %d)\n", p1->name, -loss, p1->Sanity, clashCount);
        }
      } else if (!p1->hasSanity && playerCoins <= 0) {
        printf("\n");
      } 


      //Unbreakable lost
      if (s1->Unbreakable > 0 && playerUnbreakableLost < s1->Unbreakable && s1->Unbreakable > playerCoins) {
        playerUnbreakableLost++;
      }






      // เรียกใช้ฟังก์ชันเดียวจบ (ส่ง Player เป็นคนแพ้, Enemy เป็นคนชนะ)
      applyClashRoundResult(p2, s2, p1, s1, playerCoins, enemyCoins, clashCount); // Coin need to stay same array, i am too lazy to fix it





      if (isId(p1->ID, "Meursault:Blade Lineage Mentor") == 0 && s1 == &p1->skills[2] && playerCoins <= 0) {
          Loser = p1; LoserSkill = s1;
      }






    } else {
      printf("Clash is a draw! Tossing agains...\n");

      usleep((int)(roundDelay * 5000000));
    }

    // The House of Spiders: The Thumb Nursefather Rodion Passive on clash
    if (isId(p1->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && p1->Passive > 0 && p1->skills[11].active == 0) {

      s1->ClashPower[0] -= p1->skills[4].active/2;

      if (p1->skills[4].active == 5 && !p1->skills[17].active) {
        p1->skills[17].active = 1;
        s1->CoinPowerBoost[0] += 1;
      }
      
      if (p1->skills[4].active < 5) p1->skills[4].active++; // Accelerating Future

      printf("\n%s gains 1 Accelerating Future (%d)\n", p1->name, p1->skills[4].active);

      int gain = 0;

      gain = p1->skills[4].active/2;

      s1->ClashPower[0] += gain;

      if (playerCoins <= 0 || enemyCoins <= 0) {

        gain = 3*p1->skills[4].active;
        if (gain > 15) gain = 15;
        
        s1->DamageUp[0] += gain;
      }

      if (p1->skills[6].active < 10) { // Lose eye of pro on clash
          p1->Passive--;
          p1->skills[6].active++;

        printf("\n%s loses 1 Eye of Precognition (%d)\n", p1->name, p1->Passive);
      }

      if (p1->Passive <= 0 && p1->skills[11].active == 0) {
          p1->skills[11].active = 1; // ติด Overheat

      printf("\n%s converts 'Eye of Precognition' into 'Eye of Precognition - Overheat'\n", p1->name);

      sleep(1);
      }
      
      }

    // The House of Spiders: The Thumb Nursefather Rodion Passive on clash - enemy
    if (isId(p2->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && p2->Passive > 0 && p2->skills[11].active == 0) {

      s2->ClashPower[0] -= p2->skills[4].active/2;

      if (p2->skills[4].active == 5 && !p2->skills[17].active) {
        p2->skills[17].active = 1;
        s2->CoinPowerBoost[0] += 1;
      }

      if (p2->skills[4].active < 5) p2->skills[4].active++; // Accelerating Future

      printf("\n%s gains 1 Accelerating Future (%d)\n", p2->name, p2->skills[4].active);

      int gain = 0;

      gain = p2->skills[4].active/2;

      s2->ClashPower[0] += gain;

      if (playerCoins <= 0 || enemyCoins <= 0) {

        gain = 3*p2->skills[4].active;
        if (gain > 15) gain = 15;

        s2->DamageUp[0] += gain;
      }

      if (p2->skills[6].active < 10) { // Lose eye of pro on clash
          p2->Passive--;
          p2->skills[6].active++;

        printf("\n%s loses 1 Eye of Precognition (%d)\n", p2->name, p2->Passive);
      }

      if (p2->Passive <= 0 && p2->skills[11].active == 0) {
          p2->skills[11].active = 1; // ติด Overheat

      printf("\n%s converts 'Eye of Precognition' into 'Eye of Precognition - Overheat'\n", p2->name);

      sleep(1);
      }
    
    }

    // Bleed p1 // 0 Stack 1 Count
    if ((p1->Bleed[0] > 0 || p1->Bleed[1] > 0) && s1->skillType == 0) {

      int damage = p1->Bleed[0] > 0 ? p1->Bleed[0] : 1;

        p1->Bleed[1]--;

      if (p1->Bleed[1] <= 0) p1->Bleed[1] = 0;

      printf("\n%s takes %d Bleed damage (Count %d)\n", p1->name, damage, p1->Bleed[1]);

      if (p1->Bleed[1] <= 0) p1->Bleed[0] = 0;

      applyDamage(NULL, p1, damage, 0, "Bleed");

    }

    // Bleed p2 // 0 Stack 1 Count
    if ((p2->Bleed[0] > 0 || p2->Bleed[1] > 0) && s2->skillType == 0) {

      int damage = p2->Bleed[0] > 0 ? p2->Bleed[0] : 1;

          p2->Bleed[1]--;

      if (p2->Bleed[1] <= 0) p2->Bleed[1] = 0;

      printf("\n%s takes %d Bleed damage (Count %d)\n", p2->name, damage, p2->Bleed[1]);

      if (p2->Bleed[1] <= 0) p2->Bleed[0] = 0;

      applyDamage(NULL, p2, damage, 0, "Bleed");

    }





    round++;
  }

  ClashResult result;
  result.winner = (playerCoins > 0) ? 1 : 2;
  result.playerCoins = playerCoins;
  result.enemyCoins = enemyCoins;
  result.playerskillUsed = s1;
  result.enemyskillUsed = s2;
  result.playerUnbreakableLost = playerUnbreakableLost;
  result.enemyUnbreakableLost = enemyUnbreakableLost;
  result.playerTempOffense = playerTempOffense;
  result.enemyTempOffense = enemyTempOffense;
  result.playerTempDefense = playerTempDefense;
  result.enemyTempDefense = enemyTempDefense;
  result.ClashCount = clashCount;
  result.playerFinalPower = playerTotal; 
  result.enemyFinalPower = enemyTotal;

  // printf("Clash result: winner %d Player %d coins, Enemy %d coins\n",
  // result.winner, playerCoins, enemyCoins);

  int IsplayerStagger = isStaggered(p1);
  int IsenemyStagger  = isStaggered(p2);

  if (playerUnbreakableLost > 0 && playerCoins <= 0 && !IsenemyStagger) {
    usleep(500000);

    // ถ้าเป็นสกิลโจมตีปกติ ให้ศัตรูตีเราก่อน แล้วเราค่อยสวนด้วยเหรียญที่เหลือ (Cracking)
    if (s2->skillType != 4) {
        attackPhase(p2, result.enemyskillUsed, result.enemyTempOffense,
                    result.enemyTempDefense, p1, result.playerskillUsed,
                    result.playerTempOffense, result.playerTempDefense,
                    (result.enemyCoins > result.enemyskillUsed->Unbreakable)
                        ? result.enemyCoins
                        : result.enemyskillUsed->Unbreakable,
          result.enemyUnbreakableLost
          , clashCount);

      } else {
        // --- [Clashable Guard Win Effect] ---
        p1->Tremor[4] += result.enemyFinalPower;
        printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                p2->name, p2->name, p1->name, result.enemyFinalPower);
        sleep(1);
        if (p1->Tremor[4] > 50 && p1->Stagger <= 0) {
              p1->Stagger += 2;
          printf("\n%s Staggered for one turn\n", p1->name);
          sleep(1);
              p1->Tremor[4] = 0;
        }
       }

        usleep(500000);
        if ((p1->HP > 0 && p2->HP > 0) && (s1->skillType == 0 || s1->skillType == 5)) {
          printf("\n%s lost the Clash with Cracking Unbreakable Coins (Halve the Damage)\n", p1->name);
          attackPhase(p1, result.playerskillUsed, result.playerTempOffense,
                      result.playerTempDefense, p2, result.enemyskillUsed,
                      result.enemyTempOffense, result.enemyTempDefense,
                      (result.playerCoins > result.playerskillUsed->Unbreakable)
                          ? result.playerCoins
                          : result.playerskillUsed->Unbreakable,
                      result.playerskillUsed->Unbreakable, clashCount);
        }

    result.winner = 99;

    }





  if (enemyUnbreakableLost > 0 && enemyCoins <= 0 && !IsplayerStagger) {
    usleep(500000);

     if (s1->skillType != 4) {

    attackPhase(p1, result.playerskillUsed, result.playerTempOffense,
                result.playerTempDefense, p2, result.enemyskillUsed,
                result.enemyTempOffense, result.enemyTempDefense,
                (result.playerCoins > result.playerskillUsed->Unbreakable)
                    ? result.playerCoins
                    : result.playerskillUsed->Unbreakable,
      result.playerUnbreakableLost
      , clashCount);

     } else {
      // --- [Clashable Guard Win Effect] ---
      p2->Tremor[4] += result.playerFinalPower;
      printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
              p1->name, p1->name, p2->name, result.playerFinalPower);
      sleep(1);
      if (p2->Tremor[4] > 50 && p2->Stagger <= 0) {
          p2->Stagger += 2;
        printf("\n%s Staggered for one turn\n", p2->name);
        sleep(1);
          p2->Tremor[4] = 0;
      }

     }

    usleep(500000);
    if ((p1->HP > 0 && p2->HP > 0) && (s2->skillType == 0 || s2->skillType == 5)) {
      printf("\n%s lost the Clash with Cracking Unbreakable Coins (Halve the Damage)\n", p2->name);
      usleep(500000);
      attackPhase(p2, result.enemyskillUsed, result.enemyTempOffense,
                  result.enemyTempDefense, p1, result.playerskillUsed,
                  result.playerTempOffense, result.playerTempDefense,
                  (result.enemyCoins > result.enemyskillUsed->Unbreakable)
                      ? result.enemyCoins
                      : result.enemyskillUsed->Unbreakable,
                  s2->Unbreakable, clashCount);
    }

    result.winner = 99;

  }

  // Yield my flesh mechanic
    if (Loser != NULL) {
       SkillStats *LoserSkill = &Loser->defenseSkill[1];
      // กำหนดเป้าหมายที่จะโดนสวน (คนชนะ)
      Character *Winner = (Loser == p1) ? p2 : p1;
      SkillStats *WinnerSkill = (Loser == p1) ? s2 : s1;
      int winOff = (Winner == p1) ? playerTempOffense : enemyTempOffense;
      int winDef = (Winner == p1) ? playerTempDefense : enemyTempDefense;
      int loseOff = (Loser == p1) ? playerTempOffense : enemyTempOffense;
      int loseDef = (Loser == p1) ? playerTempDefense : enemyTempDefense;
      int winCoins = (Winner == p1) ? playerCoins : enemyCoins;

    usleep(500000);

      if (WinnerSkill->skillType != 4) {

      attackPhase(Winner, WinnerSkill, winOff, winDef, Loser, LoserSkill, loseOff, loseDef, 
      // ตรรกะคำนวณเหรียญ: ถ้าเป็น Unbreakable ให้ใช้จำนวนเหรียญสูงสุด (15) 
      (WinnerSkill->Unbreakable > 0) ? 
          ((winCoins > WinnerSkill->Unbreakable) ? winCoins : WinnerSkill->Unbreakable) 
          : winCoins, 
      // ส่งจำนวนเหรียญที่แตกไปจริงๆ (เช่น 1) แทนที่จะส่ง 0
      (Winner == p1) ? playerUnbreakableLost : enemyUnbreakableLost, 
      clashCount);

    } else {
      // --- [Clashable Guard Win Effect] ---
      Winner->Tremor[4] += result.playerFinalPower;
      printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
        Winner->name, Winner->name, Loser->name, result.playerFinalPower);
        sleep(1);
      if (Loser->Tremor[4] > 50 && p1->Stagger <= 0) {
            Loser->Stagger += 2;
        printf("\n%s Staggered for one turn\n", p2->name);
        sleep(1);
          Loser->Tremor[4] = 0;
      }

      // เมื่อ Guard ชนะ จะไม่เกิดการ attackPhase ปกติ (เพราะเป็นสกิลป้องกัน)
  }

    usleep(500000);
    if (Loser->HP > 0) {

      if (Loser->Stagger > 0) {
      Loser->Stagger = 0;
      Loser->Tremor[4] += Loser->MAX_HP/4;
      }

      if ((isId(Loser->ID, "Meursault:Blade Lineage Mentor") == 0 &&
         Loser->HP <= Loser->MAX_HP * 0.6)) {


        printf("\n%s HP at 60%% or less HP, Apply 'Remembrance' buff on self, Gains buff at 5+ Poise Stack or 7+ Poise Stack on self (%d)\n", Loser->name, Loser->Poise[0]);

        int PowerBuff = 0;
        int DamageBuff = 0;

        if (Loser->Poise[0] >= 7) {

          PowerBuff = 1;

          PowerBuff = 1;
          DamageBuff = 25;

            LoserSkill->CoinPowerBoost[0] += PowerBuff;
            LoserSkill->CriticalDamageUp[0] += DamageBuff;

          printf("At 7+ Poise Stack (%d), gain +%d Coin Power and gain +%d%% damage on Critical Hit\n", Loser->Poise[0], PowerBuff, DamageBuff);

          sleep(1);
        }
      else if (Loser->Poise[0] >= 5) {

        PowerBuff = 1;
        DamageBuff = 15;

          LoserSkill->CoinPowerBoost[0] += PowerBuff;
          LoserSkill->CriticalDamageUp[0] += DamageBuff;

        printf("At 5+ Poise Stack (%d), gain +%d Coin Power and gain +%d%% damage on Critical Hit\n", Loser->Poise[0], PowerBuff, DamageBuff);

        sleep(1);
      } 

        printf("\n%s: \"If you will cut... then wager your life on it.\"\n", Loser->name);

        sleep(1);

      }

      usleep(500000);

      attackPhase(Loser, &Loser->defenseSkill[1], Loser->defenseSkill[1].Offense, Loser->defenseSkill[1].Defense,
        Winner, WinnerSkill, winOff, winDef, Loser->defenseSkill[1].Coins, 0, 0);

      fullPlayer->defenseSkill[1].active = 0;

      if (Loser->defenseSkill[2].active == 1) {
        Winner->Paralyze[1] += 5;
          Winner->Bleed[0] += 5;
        Winner->Bleed[1] += 5;

        printf("\n%s inflicts +5 Bleed Stack (%d), +5 Bleed Count (%d) and 5 Paralyze next turn (Fix the Power of 5 Coins to 0 for one "
               "turn)\n",
               Loser->name, Winner->Bleed[0], Winner->Bleed[1]);
      } else {
         Winner->Paralyze[1] += 5;
        Winner->Bleed[0] += 3;
        
      printf("\n%s inflicts +3 Bleed Stack (%d) and 5 Paralyze next turn (Fix the Power of 5 Coins to 0 for one "
             "turn)\n",
             Loser->name, Winner->Bleed[0]);
      }

      updateSanity(Loser, 15);
      if (Loser->Sanity > 45) Loser->Sanity = 45;

      printf("\n%s heals 15 Sanity (%d)\n",
        Loser->name, Loser->Sanity);

      Loser->sanityLossBase = 5;
    }
    result.winner = 99;
  }

  sleep(1);

  return result;
}






// ----------------------------------Setup characters----------------------
void setupCharacters(Character *player, Character *enemy, int pIndex,
                     int eIndex) {

  // ล้างข้อมูลทุกอย่างในโครงสร้าง player และ enemy ให้เป็น 0 ทั้งหมดก่อน
  memset(player, 0, sizeof(Character)); 
  memset(enemy, 0, sizeof(Character));


  // Initialize buffs
  initializeCharacterBuffs(player);
  initializeCharacterBuffs(enemy);

  // When setting up characters:
  player->Sanity = 0; // Start at neutral
  enemy->Sanity = 0;  // Start at neutral
  player->hasSanity = 1;
  enemy->hasSanity = 1;
  player->sanityGainBase = 10;
  player->sanityLossBase = 5;
  enemy->sanityGainBase = 6;
  enemy->sanityLossBase = 4;
  player->immuneToPanicSkip = 0;
  enemy->immuneToPanicSkip = 0;

  player->Passive = 0;
  enemy->Passive = 0;

  player->Shield = 0;
  enemy->Shield = 0;

  clearTurnEffects(player);
  clearTurnEffects(enemy);

  if (pIndex == 0) {
    player->name = "Meursault:The Thumb";
    player->HP = 122;
    player->MAX_HP = 122;
    player->MinSpeed = 4;
    player->MaxSpeed = 6;
    player->skills[0] =
        (SkillStats){"Double Slash - Blast [爆]", 4, 4, 2, 3, 2, 1, 1, 0, 3, 1}; // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    player->skills[1] =
        (SkillStats){"Triple Slash - Blast [爆]", 4, 4, 3, 4, 2, 1, 1, 0, 2, 1};
    player->skills[2] =
        (SkillStats){"Tanglecleaver [快刀亂麻]", 5, 4, 3, 5, 2, 1, 0, 0, 1, 1};
    player->skills[3] = (SkillStats){
        "Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]",3,3,5,6,2,1,0,5,0, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"I'm Burning Up.", 5, 4, 2, 3, 2, 1, 0, 0, 0, 1, 5};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 4; // <-- important
  } else if (pIndex == 1) {
    player->name = "Meursault:Blade Lineage Mentor";
    player->HP = 122;
    player->MAX_HP = 122;
    player->MinSpeed = 4;
    player->MaxSpeed = 6;
    player->skills[0] =
        (SkillStats){"Draw of the Sword", 3, 4, 2, 3, 3, 1, 1, 0, 3, 1};
    player->skills[1] = (SkillStats){"Acupuncture", 3, 5, 3, 3, 3, 1, 1, 0, 2, 1};
    player->skills[2] =
        (SkillStats){"Yield My Flesh", 20, -8, 1, 0, 15, 1, 1, 0, 1, 1};
    // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable, Type
    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Overthrow", 8, 10, 1, 5, 3, 1, 0, 0, 0, 0, 3};
    player->defenseSkill[1] =
      (SkillStats){"To Claim Their Bones", 4, 4, 4, 5, 30, 2, 1, 0, 0, 0, 3};

    player->numDefenseSkills = 2; // <-- important

    player->numSkills = 3; // <-- important
  } else if (pIndex == 2) {
    player->name = "Heathcliff:Wild Hunt";
    player->HP = 151;
    player->MAX_HP = 151;
    player->MinSpeed = 3;
    player->MaxSpeed = 8;
    player->skills[0] = (SkillStats){"Beheading", 3, 4, 2, 2, -2, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Memorial Procession", 5, 3, 3, 2, -2, 1, 0, 0, 2, 1};
    player->skills[2] = (SkillStats){"Requiem", 6, 6, 2, 5, -2, 1, 0, 0, 1, 1};
    player->skills[3] = (SkillStats){
        "Lament, Mourn, and Despair", 31, -13, 2, 5, -2, 1, 1, 0, 0, 1};

    // Impending Ruin
      player->skills[4].name = "Impending Ruin"; 
    player->skills[4].active = 0;

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"O Dullahan...!", 5, 4, 2, 3, -2, 1, 0, 0, 0, 1, 5};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 4; // <-- important
  } else if (pIndex == 3) {
    player->name = "Hong lu:The Lord of Hongyuan";
    player->HP = 102;
    player->MAX_HP = 102;
    player->MinSpeed = 4;
    player->MaxSpeed = 8;
    player->skills[0] =
        (SkillStats){"I Wish to Open the Path", 4, 4, 2, 1, 0, 1, 1, 0, 3, 1};
    player->skills[1] = (SkillStats){
        "Tarnished Blood's Absolute Cleaver of Ambitions [汚血絕志竟成]", 3, 3, 4, 2, -2, 1, 1, 2, 2, 1};
    player->skills[2] =
        (SkillStats){"Answer Me, Heishou Packs", 10, 12, 1, 3, -2, 1, 1, 1, 1, 1};
    player->skills[3] = (SkillStats){
        "Lonesome Stand: Sacrifice to Claim The Garden [孑孑單身，捨生取园]", 6, 4, 3, 3, -2, 1, 1, 0, 0, 1};
    player->skills[4] =
        (SkillStats){"I Carve the Path of a Lord", 6, 4, 2, 3, -2, 1, 1, 0, 0, 1};
    player->skills[5] = (SkillStats){
        "Embrace the Tarnished Blood and Exsanguinate Others For the Cause.",1, 1, 1, 1, 0, 0, 1, 0, 0, 0};                // Mao, Si, Wu, You, 0, 0, active, 0, Copies, Clashable
  player->skills[6] = (SkillStats){
    "Traceless to Sight and Sound Alike.", 5, 4, 3, 3, 3, 1.2, 1, 0, 0, 1}; // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
  player->skills[7] = (SkillStats){
    "Serpentshear Puncturing Arm", 8, 14, 1, 4, 0, 2, 1, 0, 0, 1}; 
  player->skills[8] = (SkillStats){
    "Cavalry's Vanguard Charge",5, 4, 3, 3, 5, 1, 1, 0, 0, 1}; 
  player->skills[9] = (SkillStats){
    "Bloodflame Massacre", 4, 3, 4, 4, 1, 1, 1, 0, 0, 1}; 

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Heishou Keenclaw [黑獸利爪]", 6, 7, 1, 0, 0, 1, 0, 1, 0, 1, 5};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 5; // <-- important
  } else if (pIndex == 4) {
    player->name = "Yi sang:Fell Bullet";
    player->HP = 106;
    player->MAX_HP = 106;
    player->MinSpeed = 4;
    player->MaxSpeed = 7;
    player->skills[0] =
        (SkillStats){"See Through Defenses", 5, 6, 1, 1, 2, 1, 1, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Punch Through", 4, 6, 2, 3, 2, 1, 1, 1, 2, 1};
    player->skills[2] =
        (SkillStats){"Target Readjustment Fire", 4, 7, 2, 5, 2, 1, 0, 2, 1, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Front Line Support", 5, 7, 1, 2, 2, 1, 0, 0, 0, 1, 5};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 3; // <-- important
  } else if (pIndex == 5) {
    player->name = "Don Quixote:The Manager of La Manchaland";
    player->HP = 103;
    player->MAX_HP = 103;
    player->MinSpeed = 4;
    player->MaxSpeed = 7;
      player->Passive = 1;
    player->skills[0] =
        (SkillStats){"Enough is Enough", 3, 4, 2, 3, 0, 1, 1, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Let All Blossom Free", 4, 6, 2, 3, 0, 1, 1, 0, 2, 1};
    player->skills[2] =
        (SkillStats){"I Shall Impale", 5, 4, 3, 5, 0, 1, 1, 0, 1, 1};
    player->skills[3] = (SkillStats){
        "Variant Sancho Hardblood Arts 6 - Whip", 4, 3, 3, 3, 0, 1, 1, 2, 0, 1};
    player->skills[4] =
        (SkillStats){"Variant Sancho Hardblood Arts 8 - Split Apart", 6, 3, 3, 3, 0, 1, 1, 1, 0, 1};
    player->skills[5] =
        (SkillStats){"Ascendant Sancho Hardblood Arts - La Sangre", 5, 5, 4, 5, 0, 1, 1, 4, 0, 1};
    player->skills[6] =
        (SkillStats){"Laughters Will Subside", 5, 4, 2, 3, 5, 1, 1, 0, 0, 1};
    player->skills[7] =
        (SkillStats){"Variant Sancho Hardblood Arts 15 - Buildup to Finale",6,5,2,3,10,1,1,1,0, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Laughters Will Subside", 5, 4, 2, 3, 5, 1, 1, 0, 0, 1, 5};
    player->defenseSkill[1] = (SkillStats){"Variant Sancho Hardblood Arts 15 - Buildup to Finale",6,5,2,3,10,1,1,1,0, 1, 5};

    player->numDefenseSkills = 2; // <-- important

    player->numSkills = 8; // <-- important
  } else if (pIndex == 6) {
    player->name = "Lobotomy E.G.O::Solemn Lament Yi Sang";
    player->HP = 106;
    player->MAX_HP = 106;
    player->MinSpeed = 3;
    player->MaxSpeed = 8;
      player->Passive = 20;
    player->skills[0] =
        (SkillStats){"Celebration for the Departed", 4, 4, 2, 2, 2, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Solemn Lament for the Living", 4, 6, 2, 2, 2, 1, 1, 0, 2, 1};
    player->skills[2] =
        (SkillStats){"Goodbye Now, A Sorrow In You", 4, 3, 4, 5, 2, 1, 1, 0, 1, 1};
      player->skills[3].active = 0; // Count consumed Living and depart

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"FromTheCoffinAButterflyTakesFlight", 10, 4, 1, 0, 3, 1, 0, 0, 0, 0, 1};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 3; // <-- important
  } else if (pIndex == 7) {
    player->name = "Dawn Office Fixer Sinclair";
    player->HP = 106;
    player->MAX_HP = 106;
    player->MinSpeed = 3;
    player->MaxSpeed = 7;
    player->sanityGainBase = 15;
    player->sanityLossBase = 7;
    player->skills[0] =
        (SkillStats){"Fierce Charge", 3, 7, 1, 2, 0, 1, 1, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Sunset Blade", 5, 3, 3, 2, 0, 1, 1, 0, 2, 1};
    player->skills[2] =
        (SkillStats){"Stigmatize", 4, 7, 2, 3, 0, 1, 1, 0, 1, 1};
    player->skills[3] =
      (SkillStats){"Blazing Strike", 13, 15, 1, 5, 0, 1, 0, 0, 0, 1};
    player->skills[4] =
      (SkillStats){"Stigmatize", 4, 7, 2, 3, 0, 1, 1, 0, 1, 1};
    player->skills[5] =
    (SkillStats){"Blazing Strike", 13, 15, 1, 5, 0, 1, 0, 0, 0, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Rue", 15, 5, 1, 0, 5, 1, 0, 0, 0, 0, 1};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 4; // <-- important
  } else if (pIndex == 8) {
    player->name = "Gregor:Firefist";
    player->HP = 140;
    player->MAX_HP = 140;
    player->MinSpeed = 4;
    player->MaxSpeed = 6;
    player->Passive = 100;
    player->skills[0] = (SkillStats){"Flamethrow", 3, 4, 2, 2, 3, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"I'll burn away every last drop of your filthy blood",4, 6, 2, 3, 3, 1, 0, 0, 2, 1};
    player->skills[2] = (SkillStats){"Firefist", 5, 4, 3, 5, 3, 1, 1, 0, 1, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"I have to keep going for big sis", 9, 7, 1, 2, 3, 1, 0, 0, 0, 1, 5};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 4; // <-- important
  } else if (pIndex == 9) {
    player->name = "Heishou Pack - You Branch Adept Heathcliff";
    player->HP = 132;
    player->MAX_HP = 132;
    player->MinSpeed = 4;
    player->MaxSpeed = 7;
    player->skills[0] = (SkillStats){"Peck 'em", 2, 3, 3, 1, -1, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Mutilating Talons", 4, 4, 3, 2, -1, 1, 0, 0, 2, 1};
    player->skills[2] = (SkillStats){"Bloodflame Massacre [血炎亂舞]", 4, 3, 4, 3, -1, 1, 0, 0, 1, 1};
    player->skills[3] = (SkillStats){"Rooster's Rampaging Blades Under the Ensanguined Heaven [血天下雞舞亂刀]", 5, 3, 4, 5, -1, 1, 0, 4, 0, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"More Grub for Us", 5, 4, 2, 2, -1, 1, 0, 0, 0, 1, 5};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 4; // <-- important
  } else if (pIndex == 10) {
    player->name = "The Middle Little Brother Sinclair";
    player->HP = 165;
    player->MAX_HP = 165;
    player->MinSpeed = 4;
    player->MaxSpeed = 6;
    player->Passive = 0;
    player->skills[0] = (SkillStats){"Is it You?!", 3, 4, 2, 2, 5, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Payback with Interest", 4, 4, 3, 3, 5, 1, 0, 0, 2, 1};
    player->skills[2] = (SkillStats){"Write 'em all down", 5, 4, 3, 4, 5, 1, 0, 0, 1, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Warmup in the East", 5, 4, 2, 5, 5, 1, 0, 0, 0, 0, 3};

    player->numSkills = 3; // <-- important
    player->numDefenseSkills = 1; // <-- important
  } else if (pIndex == 11) {
    player->name = "The House of Spiders: The Index Nursefather Yi Sang";
      player->HP = 92; 
    player->MAX_HP = 92;
    player->MinSpeed = 3;
    player->MaxSpeed = 8;
      player->skills[0] = (SkillStats){"'Enwrap 330 times...'", 3, 4, 2, 2, 2, 1, 0, 0, 3, 1};
      player->skills[1] = (SkillStats){"'Revel with Soundless Applause...'", 4, 4, 3, 3, 2, 1, 0, 0, 2, 1};
      player->skills[2] = (SkillStats){"'Raise and Laugh the Blade...'", 4, 3, 4, 4, 2, 1, 0, 0, 1, 1};
      player->skills[3] = (SkillStats){"Furioso-Replica", 2, 2, 9, 5, 2, 1, 0, 9, 0, 1}; 

    player->Passive = 0;          // Stage
    player->skills[0].active = 0; // Grace
    player->skills[1].active = 0; // Hermes Stacks
    player->skills[2].active = 0; // Karma
    player->skills[3].active = 1; // เริ่มต้นด้วยหน้ากาก
    player->skills[4].active = 0; // Prescript Target Skill
    player->skills[5].active = 0; // Prescript Executed flag
    player->skills[8].active = 0; // <--- สำคัญ: ปลดล็อคการรับแต้ม
    player->skills[13].active = 0; // <--- สำคัญ: รีเซ็ตโควต้าเทิร์นแรก

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"By Unpredictable Whim", 4, 10, 1, 0, 2, 1, 0, 0, 0, 0, 2};

    player->numDefenseSkills = 1; // <-- important

      player->numSkills = 4;
    } else if (pIndex == 12) {
    player->name = "The One Who Grips Faust";
      player->HP = 110; 
    player->MAX_HP = 110;
    player->MinSpeed = 4;
    player->MaxSpeed = 7;
      player->skills[0] = (SkillStats){"Cackle", 4, 3, 2, 4, -1, 1, 0, 0, 3, 1};
      player->skills[1] = (SkillStats){"The Gripping", 4, 4, 3, 4, -1, 1, 0, 0, 2, 1};
      player->skills[2] = (SkillStats){"Execution", 6, 2, 3, 4, -1, 1, 0, 0, 1, 1};
      player->skills[3] = (SkillStats){"Purify", 6, 3, 4, 5, -1, 1, 0, 1, 0, 1}; 
      player->skills[4] = (SkillStats){"I Shall Claim Your Life!", 12, 5, 1, 0, -1, 1, 0, 0, 0, 0}; 

    player->Passive = 0;          // Fanatic
    player->skills[6].active = 0; // Fanatic Next turn
    player->skills[0].active = 0; // Bleed Stack
    player->skills[1].active = 0; // Bleed Count
    player->skills[2].active = 0; // Nail
    player->skills[3].active = 0; // Whistles Counter
    player->skills[4].active = 0; // Gaze
    player->skills[7].active = 0; // Gaze Next turn
    player->skills[5].active = 0; // Bliss Flag (for once per turn)

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Such Filth", 4, 10, 1, 0, 0, 1, 0, 0, 0, 0, 2};

    player->numDefenseSkills = 1; // <-- important

      player->numSkills = 5;
    } else if (pIndex == 13) {
    player->name = "The House of Spiders: The Ring Nursefather Hong Lu";
      player->HP = 101; 
    player->MAX_HP = 101;
    player->MinSpeed = 4;
    player->MaxSpeed = 7;
      player->skills[0] = (SkillStats){"Anatomize", 3, 4, 2, 2, 2, 1, 0, 0, 3, 1};
      player->skills[1] = (SkillStats){"Gather Ingredient - Blood-bathed Objet", 4, 4, 3, 3, 2, 1, 0, 0, 2, 1};
    player->skills[2] = (SkillStats){"Installation Art no. 3: Improvised Ribcage", 5, 4, 2, 3, 2, 1, 0, 0, 0, 0};
      player->skills[3] = (SkillStats){"Tibia's Melody - Anatomization of the Unanatomized by the Anatomized", 4, 3, 4, 4, 2, 1, 0, 0, 1, 1}; 
      player->skills[4] = (SkillStats){"Closing Time - Installation Art no. 1: Your Flesh and Bones as the Gallery's Seats", 3, 4, 5, 5, 2, 2, 0, 5, 0, 1}; 
    // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    player->Passive = 21;          // Viewing the Tableau (เริ่มที่ 21)
    player->skills[2].active = 0;  // Maestro's Critique Turn Counter (Max 3)
    player->skills[3].active = 0;  // First Stagger Recovery Flag (0=unused, 1=used)
    player->skills[4].active = 0;  // Corpus Theater Stacks (เริ่มที่ 3)
     player->skills[5].active = 0;  // Consumes for evade flag (0=unused, 1=used)
    player->skills[6].active = 0;  // Consumes Count

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[1] = (SkillStats){"Curating the Exhibition", 4, 10, 1, 0, 2, 1, 0, 0, 0, 0, 2};
    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Please. Vandalism is Prohibited.", 4, 4, 2, 2, 2, 1, 0, 0, 0, 1, 5};

    player->numDefenseSkills = 2; // <-- important

      player->numSkills = 5;
    } else if (pIndex == 14) {
    player->name = "The House of Spiders: The Thumb Nursefather Rodion";
      player->HP = 109; 
    player->MAX_HP = 109;
    player->MinSpeed = 4;
    player->MaxSpeed = 7;
      player->skills[0] = (SkillStats){"Colpi di Taglio", 3, 4, 2, 2, 0, 1, 0, 0, 3, 1};
      player->skills[1] = (SkillStats){"Sezionatura di Coniglio", 4, 4, 2, 2, 0, 1, 0, 0, 0, 0};
    player->skills[2] = (SkillStats){"I'll Gladly Blast a Hole Through Ya", 4, 4, 3, 3, 0, 1, 0, 0, 2, 1};
      player->skills[3] = (SkillStats){"Sezionatura di Cervo", 4, 4, 3, 3, 0, 1, 0, 0, 0, 1}; 
      player->skills[4] = (SkillStats){"Sezionatura di Elefante", 4, 3, 4, 4, 0, 1, 0, 0, 1, 1}; 
    player->skills[5] = (SkillStats){"Disposal", 5, 2, 5, 5, 0, 1, 0, 5, 0, 1}; 

    player->numSkills = 6;
    
    // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable

    player->Passive = -1;           // Eye of Precognition (30/30)
      player->skills[6].active = 0; // Eye count per turn
    player->skills[4].active = 0; // Accelerating Future
    player->skills[3].active = 0; // Stagger flag (0 = ยังไม่ใช้)
    player->skills[10].active = -1; // Acceleration Round (กระสุน 10/10)
    player->skills[11].active = 0;  // Overheat state (0 = ปกติ)
    player->skills[12].active = 0;  // (จำนวนครั้งที่ Tremor Burst)
    player->skills[13].active = 0;  // Shin - Disgrace flag
    player->skills[14].active = 0;  // Once per encounter

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Fuck Off!", 3, 4, 2, 2, 0, 1, 0, 0, 0, 1, 5};
    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[1] = (SkillStats){"Precognition", 3, 7, 1, 0, 2, 1, 0, 0, 0, 0, 2};

    player->numDefenseSkills = 2; // <-- important

    } else if (pIndex == 99) {
    player->name = "Muga Ryōshū";
      player->HP = 403; 
    player->MAX_HP = 403;
    player->MinSpeed = 3;
    player->MaxSpeed = 6;
    player->Sanity = -44;
    player->sanityGainBase = 0; // SP ถูกล็อค
    player->sanityLossBase = 0;
    player->immuneToPanicSkip = 1; // ไม่ติด Panic แม้ SP จะต่ำ
    player->SanityFreezeTurns = -1; // ล็อค SP ไว้ถาวร

      player->skills[0] = (SkillStats){"Sever", 12, -1, 2, 2, -4, 1, 1, 2, 3, 1};
    player->skills[1] = (SkillStats){"Paint", 12, -1, 2, 2, -4, 1, 1, 2, 3, 1};
      player->skills[2] = (SkillStats){"Splatter", 15, -4, 3, 4, -4, 1, 1, 3, 2, 1};
    player->skills[3] = (SkillStats){"Brushstroke", 15, -4, 3, 4, -4, 1, 1, 3, 2, 1};
      player->skills[4] = (SkillStats){"Slay the Heavens - Tiansha [天殺]", 24, -6, 5, 6, -4, 1, 1, 5, 1, 1};
      player->skills[5] = (SkillStats){"'Erasing Me, Erasing You.'", 36, -6, 5, 5, -4, 1, 1, 5, 0, 1}; 

    player->Passive = 0;          // Muga [無我]
    player->skills[0].active = 0; // Sever the Thread [切絲]
    player->skills[1].active = 0; // Tiansha Star's Blade - Arayashiki [天殺星刀阿賴耶識]

      player->numSkills = 6;
    } else if (pIndex == 88) { // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    player->name = "Binah";
    player->HP = 100;
    player->MAX_HP = 100;
    player->MinSpeed = 5;
    player->MaxSpeed = 10;
    player->skills[0] = (SkillStats){"Degraded Fairy", 4, 5, 2, 2, 5, 1, 0, 0, 3, 1};
    player->skills[1] = (SkillStats){"Degraded Chain", 8, 6, 1, 4, 5, 1, 1, 0, 2, 1};
    player->skills[2] = (SkillStats){"Degraded Pillar", 7, 4, 2, 4, 5, 1, 1, 0, 2, 1};
    player->skills[3] = (SkillStats){"Degraded Lock", 9, 11, 1, 6, 5, 1, 0, 0, 1, 1};
    player->skills[4] = (SkillStats){"Degraded Shockwave", 4, 5, 3, 6, 5, 1, 0, 0, 1, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Spatial Guard", 5, 10, 1, 0, 0, 1, 1, 1, 0, 1, 4};

    player->numDefenseSkills = 1; // <-- important

    player->numSkills = 5; // <-- important
  }
  // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active,
  // Unbreakable, Copies, Clashable
  if (eIndex == 0) {
    enemy->name = "Bandit";
    enemy->HP = 15000;
    enemy->MAX_HP = 15000;
    enemy->MinSpeed = 2;
    enemy->MaxSpeed = 6;
    enemy->SanityFreezeTurns = -1;
    enemy->skills[0] = (SkillStats){"Slash", 3, 5, 1, 1, 2, 1, 1, 0, 3, 1};
    enemy->skills[1] = (SkillStats){"Charge!", 2, 3, 3, 2, 1, 1, 1, 0, 2, 1};
    enemy->skills[2] =
        (SkillStats){"Path of Bandit!", 2, 2, 4, 4, 1, 1.5, 1, 0, 1, 1};
    enemy->numSkills = 3; // <-- important
  } else if (eIndex == 1) {
    enemy->name = "Lei heng";
    enemy->HP = 765;
    enemy->MAX_HP = 765;
      enemy->MinSpeed = 2;
      enemy->MaxSpeed = 4;
    enemy->immuneToPanicSkip = 1;
    enemy->sanityGainBase = 8;
    enemy->sanityLossBase = 10;
    enemy->skills[0] = (SkillStats){"Double Slash", 4, 2, 2, 2, 3, 1, 0, 0, 4, 1};
    enemy->skills[1] = (SkillStats){"Triple Slash", 3, 2, 3, 3, 3, 1, 0, 0, 2, 1};
    enemy->skills[2] =
        (SkillStats){"Tanglecleaver [快刀亂麻]", 8, 12, 1, 6, 3, 1, 0, 1, 0, 1};
    enemy->skills[3] = (SkillStats){
        "Blasting Shatterslash [爆碎斬]", 4, 3, 3, 4, 3, 1, 0, 3, 0, 1};
    enemy->skills[4] = (SkillStats){
      "Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]", 3, 3, 6, 6, 3, 1, 0, 6, 0, 1};

    enemy->Passive = 0;                 // Inner Strength [底力] / Extreme Strength [極力] (แต้มพลังสะสม)
    enemy->skills[0].active = 0;        // Boss Phase (0: เริ่มต้น, 1: เฟส 2, 2: เฟส 3, 3: เฟส 4/Shin)
    enemy->skills[1].active = 0;        // Overheat Level (ใช้คำนวณดีบัฟ Clash Power Down และ Fragile)
    enemy->skills[2].active = 0;        // Skill 3 [Tanglecleaver] Turn Counter (ตัวนับเทิร์นสำหรับใช้ท่าใหญ่ 1)
    enemy->skills[3].active = 0;        // Strength Consumed (จำนวนแต้มที่ใช้ไป เพื่อคำนวณฮีล SP ตอนจบเทิร์น)
    enemy->skills[4].active = 0;        // Tiantui Star [天退星] Flag (0: ยังไม่เปิด, 1: เปิดใช้งานบัฟดาวตก)
    enemy->skills[5].active = 0;        // Skill 4 [Savage Tigerslayer] Turn Counter (ตัวนับเทิร์นสำหรับใช้ท่าใหญ่ 2)
    enemy->defenseSkill[1].active = 0;  // Tigermark Round (จำนวนกระสุนสำหรับระบบอาวุธ Gunblade) 
     enemy->defenseSkill[2].active = 0;// Savage Tigermark Round Loaded Ammo (Stack)
    enemy->defenseSkill[3].active = 0;  // Savage Tigermark Round Remaining Ammo (Count)
    enemy->skills[7].active = 0;  // Damage taken last turn
    enemy->skills[8].active = 0;  // next 2 turn Count
    enemy->skills[6].active = 0;        // Chosen Prey flag (0 = no Prey on player, 1 = player has Prey)

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    enemy->defenseSkill[0] = (SkillStats){
        "Reloading Tiantui Star's Blade", 4, 4, 2, 1, 3, 1, 0, 0, 0, 1, 5};

    enemy->numDefenseSkills = 1;

    enemy->numSkills = 5; // <-- important
  } else if (eIndex == 2) {
    enemy->name = "Erlking Heathcliff";
    enemy->HP = 901;
    enemy->MAX_HP = 901;
    enemy->MinSpeed = 2;
    enemy->MaxSpeed = 4;
    enemy->sanityGainBase = 10;
    enemy->sanityLossBase = 11;
    enemy->immuneToPanicSkip = 1;
    enemy->skills[0] =
        (SkillStats){"Greatsword Rend", 4, 5, 1, 0, 4, 1, 1, 0, 4, 1};
    enemy->skills[1] = (SkillStats){
        "Heed My Call, Wuthering Heights", 3, 3, 1, 0, 4, 1, 1, 0, 2, 0};
    enemy->skills[2] = (SkillStats){
        "You'll Get Shoved In This Coffin Too", 3, 4, 1, 0, 4, 1, 1, 0, 2, 0};
    enemy->skills[3] =
        (SkillStats){"Behead Heathcliffs", 8, 5, 1, 0, 4, 1, 1, 0, 0, 1};
    enemy->skills[4] = (SkillStats){"Smackdown", 4, 3, 3, 0, 4, 1, 1, 0, 2, 1};
    enemy->skills[5] =
        (SkillStats){"Hollow Coffin Mace", 4, 4, 2, 0, 4, 1.5, 1, 0, 0, 1};
    enemy->skills[6] =
        (SkillStats){"Ride for Death, Dullahan", 12, 1, 3, 0, 4, 1, 1, 0, 0, 1};
    enemy->skills[7] = (SkillStats){"Sorrow and Lament In The Erlking's Wake.", 4, 4, 4, 0, 4, 1, 0, 0, 0, 1};
    enemy->skills[8] = (SkillStats){
        "Every Heathcliff Must Die...", 30, 10, 1, 1, -50, 100, 0, 1, 0, 0};
    enemy->numSkills = 9; // <-- important
  } else if (eIndex == 3) {
    enemy->name = "Sukuna:King of Curse";
    enemy->HP = 657;
    enemy->MAX_HP = 657;
    enemy->MinSpeed = 4;
    enemy->MaxSpeed = 6;
    enemy->sanityGainBase = 10;
    enemy->sanityLossBase = 7;
    enemy->immuneToPanicSkip = 1;
    enemy->skills[0] = (SkillStats){
        "Cursed Technique - Dismantle", 6, 2, 3, 3, 10, 1, 1, 0, 4, 1}; // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[1] = (SkillStats){"Cursed Technique - Cleave", 6, 3, 2, 3, 10, 1, 1, 0, 4, 1};
    enemy->skills[2] = (SkillStats){"Blitz speed", 4, 2, 5, 4, 10, 1, 1, 1, 3, 1};
    enemy->skills[3] = (SkillStats){"Cursed Technique - Fuga:Open [鐚]", 7, 15, 1, 6, 10, 1, 0, 0, 2, 1};
    enemy->skills[4] =
        (SkillStats){"Chanting", 5, 10, 1, 0, 10, 0, 0, 0, 0, 0};
    enemy->skills[5] = (SkillStats){"Black Flash", 10, 5, 1, 5, 10, 1, 1, 1, 3, 1};
    enemy->skills[6] = (SkillStats){"Know your place...", 2, 1, 5, 1, 10, 1, 1, 1, 2, 0};
    enemy->skills[7] =
      (SkillStats){"Cursed Technique - World Cutting Slash", 10, 15, 1, 7, 10, 1.5, 0, 1, 0, 1};
    enemy->numSkills = 8; // <-- important
  } else if (eIndex == 4) {
    enemy->name = "Don Quixote";
    enemy->HP = 200;
    enemy->MAX_HP = 200;
    enemy->MinSpeed = 1;
    enemy->MaxSpeed = 2;
    enemy->Passive = 0;
    enemy->skills[0] = (SkillStats){
        "Joust", 4, 4, 1, 2, 2, 1, 1, 0, 3, 1}; // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[1] =
        (SkillStats){"Galloping Tilt", 4, 6, 1, 2, 2, 1, 1, 0, 2, 1};
    enemy->skills[2] = (SkillStats){"For Justice!", 3, 3, 3, 2, 2, 1, 1, 0, 1, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    enemy->defenseSkill[0] = (SkillStats){"Evade", 2, 10, 1, 0, 2, 1, 1, 0, 2, 0, 2};

    enemy->numDefenseSkills = 1; // <-- important

    enemy->skills[3] = (SkillStats){"Topple", 3, 4, 2, 1, 0, 1, 1, 0, 0, 1};
    enemy->skills[4] = (SkillStats){"Wound", 2, 4, 2, 1, 0, 1, 1, 0, 0, 1};
    enemy->skills[5] =
        (SkillStats){"Sanguine Joy", 5, 9, 1, 2, 0, 1, 0, 0, 0, 1};
    enemy->skills[6] =
        (SkillStats){"Ecstasy of Blood", 3, 3, 3, 2, 0, 1, 1, 3, 0, 1};
    enemy->skills[7] = (SkillStats){
        "Don Quixote Hardblood Arts 4 - Sundering", 3, 5, 2, 2, 0, 1, 1, 0, 0, 1};
    enemy->skills[8] = (SkillStats){
        "Don Quixote Hardblood Arts 11 - Impaling", 3, 3, 3, 3, 0, 1, 1, 0, 0, 1};
    enemy->skills[9] = (SkillStats){
        "Don Quixote Hardblood Arts 2 - Hardblood", 4, 3, 4, 2, 0, 1, 1, 4, 0, 1};
    enemy->skills[10] =
        (SkillStats){"Variant Don Quixote Style: Sancho Arts 4 - Track", 3, 5, 2, 3, 0, 1, 1, 0, 0, 1};
    enemy->skills[11] =
        (SkillStats){"Variant Don Quixote Style: Sancho Arts 3 - Ambush", 2, 4, 3, 2, 0, 1, 1, 0, 0, 1};
    enemy->skills[12] =
        (SkillStats){"Variant Don Quixote Style: Sancho Arts 2 - La Sangre", 12, 14, 1, 2, 0, 1, 1, 1, 0, 1};
    enemy->skills[13] =
        (SkillStats){"La Aventura Ha Terminado", 12, 14, 1, 2, 0, 2, 1, 1, 0, 1};
    enemy->numSkills = 14; // <-- important
  } else if (eIndex == 5) {
    enemy->name = "Jia Qiu";
    enemy->HP = 2439;
    enemy->MAX_HP = 2439;
    enemy->MinSpeed = 3;
    enemy->MaxSpeed = 5;
      enemy->sanityGainBase = 7;
      enemy->sanityLossBase = 5;
    enemy->immuneToPanicSkip = 1;
    enemy->Passive = 0;
    enemy->skills[0] = (SkillStats){"Question (問)", 4, 7, 1, 37, 0, 1, 1, 0, 3, 1}; 
    // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[1] =
        (SkillStats){"Answer (答)", 3, 8, 1, 37, 0, 1, 1, 0, 3, 1};
    enemy->skills[2] = (SkillStats){"Act Not With Impropriety (非禮勿動)", 5, 4, 2, 37, 0, 1, 1, 1, 0, 1};
    enemy->skills[3] = (SkillStats){"Thunderstrike", 4, 8, 1, 37, 0, 1, 1, 0, 2, 1};
    enemy->skills[4] = (SkillStats){"Cut them Down, Mao", 4, 6, 2, 37, 0, 1, 1, 1, 2, 1};
    enemy->skills[5] =
        (SkillStats){"I Must Push You Further", 5, 3, 4, 37, 0, 1, 1, 4, 0, 1};

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    enemy->defenseSkill[0] = (SkillStats){"Evade", 4, 10, 1, 35, 37, 1, 1, 0, 2, 0, 2};
    enemy->defenseSkill[1] = (SkillStats){"Do Not Meddle", 3, 3, 2, 37, 0, 1, 1, 0, 3, 1, 5};

    enemy->numDefenseSkills = 2; // <-- important

    //Phase 2
    enemy->skills[7] = (SkillStats){
        "I Ask Again", 4, 7, 1, 37, 0, 1, 1, 1, 0, 1};
    enemy->skills[8] = (SkillStats){
        "Act Not With Impropriety (非禮勿動)", 4, 4, 2, 37, 0, 1, 1, 0, 0, 1};
    enemy->skills[9] = (SkillStats){
        "You Will Not Meddle", 4, 4, 2, 37, 0, 1, 1, 0, 0, 1};
    enemy->skills[10] =(SkillStats){
      "Thunderstrike - Ripple (波)", 7, 13, 1, 37, 0, 1, 1, 0, 0, 1};
    enemy->skills[11] =(SkillStats){
      "Heishou - Mao Linkstrike - Bladestorm", 4, 3, 4, 37, 0, 1, 1, 4, 0, 1};
    enemy->skills[12] =(SkillStats){
      "Heed Me, Zilu", 3, 3, 4, 37, 0, 1, 1, 4, 0, 1};
    enemy->skills[13] =(SkillStats){
      "Deathrite Deluge", 7, 13, 1, 37, 0, 1, 1, 1, 0, 1};

    char buffer[64];
    if (strstr(player->name, "Hong lu") != NULL) {
      sprintf(buffer, "Answer Me, Jia baoyu");
    } else {
    sprintf(buffer, "Answer Me, %s", player->name);
    }
    enemy->skills[14] =(SkillStats){
      strdup(buffer), 3, 2, 4, 37, 0, 1, 1, 4, 0, 1};

    enemy->skills[15] =(SkillStats){
      "Tiangang Star - Form (格)", 5, 8, 4, 37, 0, 1, 0, 4, 0, 1};
    enemy->skills[16] =(SkillStats){
      "Like a Roaring Storm", 5, 8, 1, 37, 0, 1, 1, 0, 0, 0};
    enemy->numSkills = 17; // <-- important
  } else if (eIndex == 6) {
    enemy->name = "King in Binds";
    enemy->HP = 3519/2;
    enemy->MAX_HP = 3519;
    enemy->MinSpeed = 2;
    enemy->MaxSpeed = 5;
      enemy->hasSanity = 0;
    enemy->skills[0] = (SkillStats){"Enveloping Resignation", 3, 4, 2, 0, 0, 1, 0, 0, 3, 1}; 
    // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[1] =
        (SkillStats){"Strangling Despair", 4, 4, 3, 0, 0, 1, 0, 0, 3, 1};
    enemy->skills[2] = (SkillStats){"Chainbreaker", 3, 4, 4, 0, 0, 1, 0, 0, 3, 1};
    enemy->skills[3] = (SkillStats){"Chains of Binding", 5, 5, 2, 0, 0, 1, 0, 0, 2, 1};
    enemy->skills[4] = (SkillStats){"Thou Wilt Sink", 8, 10, 1, 0, 0, 1, 0, 0, 2, 1};
    enemy->skills[5] =
        (SkillStats){"Present Thyself Before the King", 35, -15, 1, 0, 0, 1, 0, 0, 0, 0};
    enemy->skills[6] =
      (SkillStats){"Moment of Audience", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    enemy->numSkills = 6; // <-- important
  } else if (eIndex == 7) {
    enemy->name = "The Middle Nursefather - Matthias";
    enemy->HP = 2000;
    enemy->MAX_HP = 2000;
    enemy->MinSpeed = 1;
    enemy->MaxSpeed = 3;
    enemy->hasSanity = 1;
    enemy->sanityGainBase = 10;
    enemy->sanityLossBase = 8;
    enemy->immuneToPanicSkip = 1;

    enemy->skills[0] = (SkillStats){"Stomping", 9, 2, 2, 3, 15, 1, 1, 1, 3, 1}; 
    // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[1] = (SkillStats){"Punting", 9, 2, 2, 3, 15, 1, 1, 0, 3, 1}; 
    enemy->skills[2] = (SkillStats){"Stamp of Vengeance", 10, 1, 4, 2, 15, 1, 1, 4, 0, 1}; 
    enemy->skills[3] = (SkillStats){"Don't Let Somethin' Like This Break Ya!", 11, 1, 3, 2, 15, 1, 1, 3, 2, 1}; 
    enemy->skills[4] = (SkillStats){"Gonna Unbox a Layer of Packaging!", 12, 1, 3, 3, 15, 1, 1, 3, 0, 1}; 
    enemy->skills[5] = (SkillStats){"Looks Like I Gotta Unbox the Second Layer, too.", 12, 1, 4, 4, 15, 1, 1, 4, 0, 1}; 
    enemy->skills[6] = (SkillStats){"Lævateinn - Been a While Since I Had to Unbox the Whole Thing!", 13, 1, 4, 5, 15, 1, 1, 4, 0, 1};
    enemy->skills[7] = (SkillStats){"I'll Gut Ya Like a Fish", 11, 2, 2, 5, 15, 1, 1, 2, 0, 1}; 
    // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[8] = (SkillStats){"Gut Stab [Lævateinn]", 11, 1, 5, 5, 15, 1, 1, 5, 0, 1}; 
    enemy->skills[9] = (SkillStats){"... Complete and Total Extermination [Lævateinn]", 13, 1, 5, 8, 15, 1, 1, 5, 0, 1}; 

    for(int i=0; i<10; i++) enemy->skills[i].active = 0;

    /*skills[0].active: The Middle - Grudge (ความแค้น - Max 10)
    skills[1].active: Check This Out, Kiddo! (ดูนี่ไว้นะไอ้หนู - Max 10)
    skills[2].active: Vengeance Tattoo (รอยสักแห่งการล้างแค้น - Max 2)
    skills[3].active: ดาเมจที่ได้รับในเทิร์นที่แล้ว (สำหรับ Mad Rampage)
    skills[4].active: จำนวนครั้งที่โดนโจมตีในเทิร์นนี้ (เพื่อเช็ค "เมื่อโดนโจมตีครบ 2 ครั้ง")
    skills[5].active: Flag สำหรับ "ชนะ Clash ครั้งแรกในเทิร์น"
    skills[6].active: ตัวนับจำนวน Coin ที่ Matthias โจมตีโดนในเทิร์นนี้
    skills[8].active: ตัวนับ Cooldown 3 turn
    skills[9].active: Unsealed State*/

    enemy->numSkills = 10; // <-- important

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    enemy->defenseSkill[0] = (SkillStats){"Rule Violation", 6, 2, 2, 3, 15, 1, 1, 0, 2, 0, 3};

    enemy->numDefenseSkills = 1; // <-- important

  } else if (eIndex == 8) {
    enemy->name = "Fixer grade 9?";
    enemy->HP = 1897;
    enemy->MAX_HP = 1897;
    enemy->MinSpeed = 3;
    enemy->MaxSpeed = 7;
    enemy->sanityGainBase = -7;
    enemy->sanityLossBase = -5;
    enemy->immuneToPanicSkip = 1;
    enemy->SanityFreezeTurns = -1;
    enemy->skills[0] =
        (SkillStats){"Allas Workshop", 12, -5, 2, 3, -5, 1, 1, 0, 10, 1};
    enemy->skills[1] =
        (SkillStats){"Wheels Industry", 15, -11, 1, 5, -5, 1, 1, 0, 2, 1};
    enemy->skills[2] =
        (SkillStats){"Crystal Atelier", 16, -6, 2, 3, -5, 1, 1, 0, 1, 1};
    enemy->skills[3] =
        (SkillStats){"Zelkova Workshop", 11, -5, 2, 2, -5, 1, 1, 0, 10, 1};
    enemy->skills[4] =
        (SkillStats){"Old Boys Workshop", 10, -5, 1, 2, -5, 1, 1, 0, 15, 1};
    enemy->skills[5] =
        (SkillStats){"Mook Workshop", 10, -7, 1, 2, -5, 1, 1, 0, 15, 1};
    enemy->skills[6] =
        (SkillStats){"Ranga Workshop", 11, -4, 3, 2, -5, 1, 0, 0, 10, 1};
    enemy->skills[7] =
        (SkillStats){"Atelier Logic", 15, -4, 3, 3, -5, 1, 1, 0, 1, 1};
    enemy->skills[8] = (SkillStats){"Durandal", 15, -7, 2, 4, -5, 1, 1, 0, 2, 1};
    enemy->skills[9] =
        (SkillStats){"Furioso", 25, -3, 15, 6, -5, 1, 0, 15, 0, 1};
    enemy->numSkills = 10; // <-- important
  } else if (eIndex == 99) {
    enemy->name = "Evil Bandit";
    enemy->HP = 4;
    enemy->MAX_HP = 1000;
    enemy->MinSpeed = 5;
    enemy->MaxSpeed = 7;
    enemy->hasSanity = 1;
    
    enemy->numSkills = 0; // <-- important

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    enemy->defenseSkill[0] = (SkillStats){"Evade", 20, 2, 2, 3, 5, 1, 1, 0, 2, 0, 2};

    enemy->numDefenseSkills = 1; // <-- important

  }


  player->ID = player->name; // ID จะไม่โดน glitchText แตะต้อง
  enemy->ID = enemy->name; // ID จะไม่โดน glitchText แตะต้อง


}



















// ----------------------------------------------------------------
// handleTurnStart — Before everything after turn %d
// ----------------------------------------------------------------
// Function สำหรับจัดการ Effect ตอนเริ่ม Turn
// c = ตัวละครที่กำลังตาย (เช่น player)
// opponent = ตัวละครฝ่ายตรงข้าม (เช่น enemy หรือ boss)
// forcedSkillIndex = ตัวแปร pointer สำหรับบังคับให้ใช้สกิลที่กำหนด (สำหรับ AI/Enemy)
void handleTurnStart(Character *player, Character *enemy, SkillStats **enemySkillEffective, int *playerSkill1, int *playerSkill2, int *playerSkill3, int *enemySkill1, int *enemySkill2, int *enemySkill3) {

  //------------------- Turn Start ----------------------------




  




  
  if (isId(enemy->ID, "Your Best Friend") == 0 && enemy->numSkills > 0 && (*enemySkill1 == -1 || *enemySkill2 == -1)) {
    enemy->Passive = 1;
      
      getSkills(enemy, enemySkill1, enemySkill2, enemySkill3, -1, enemy->numSkills);

    // บังคับเลือกท่าโจมตีใหม่ทันที (เพื่อไม่ให้มันใช้ท่าป้องกันอันเดิมในเทิร์นนี้)
    if (enemySkillEffective != NULL) {
        *enemySkillEffective = &enemy->skills[*enemySkill1];
    }
    
  }

  

  if (isId(enemy->ID, "Your Best Friend") == 0 && enemy->Passive == 0) {
    enemy->Passive = 1;

    printf("\n%s: \"Back off that all I want...\"\n", enemy->name);

    sleep(1);

    printf("\n\x1b[1;30m\"He sounds familier, but you don't know why.\"\x1b[0m\n");

    sleep(1);
  }

  if (isId(enemy->ID, "Your Best Friend") == 0 && enemy->Passive == 1) {

    enemy->Passive = 2;

    for (int i = 0; i < enemy->numSkills; i++) {
      enemy->skills[i].Offense -= 50;
      enemy->skills[i].Defense -= 50;
    }

    for (int i = 0; i < enemy->numDefenseSkills; i++) {
        enemy->defenseSkill[i].Offense -= 50;
        enemy->defenseSkill[i].Defense -= 50;
    }

    enemy->Shield += enemy->MAX_HP;

    printf("\n%s gains 'Love' (Lose 50 Offense Level and 50 Defense Level, and gain Shield HP equal to (Max HP) (Activate once per encounter))\n", enemy->name);

    sleep(1);

    printf("\n\x1b[1;30m\"Sometimes I wish they just give up...\"\x1b[0m\n");

    sleep(1);

  }

  if (isId(enemy->ID, "Your Best Friend") == 0 && enemy->Passive == 2 && (enemy->Shield <= 0 || enemy->HP <= enemy->MAX_HP*0.7)) {

    enemy->Passive = 3;

    for (int i = 0; i < enemy->numSkills; i++) {
      enemy->skills[i].Offense += 50;
      enemy->skills[i].Defense += 50;
      enemy->skills[i].DmgMutiplier += 0.5;
    }

    for (int i = 0; i < enemy->numDefenseSkills; i++) {
        enemy->defenseSkill[i].Offense += 50;
        enemy->defenseSkill[i].Defense += 50;
      enemy->skills[i].DmgMutiplier += 0.5;
    }

    printf("\n%s: \"Why are you making things so difficult than it should be...\"\n", enemy->name);

    sleep(1);

    printf("\nIn that time, A hoodie that a man wore, slip off and fly away\n");

    sleep(3);

    printf("\n%s: \"What...the... No... I-It can't be.\"\n", player->name);

    sleep(3);

    printf("\n%s: \"Why?! I thought you are my best friend!\"\n", player->name);

    enemy->name = enemy->ID;

     sleep(2);

    printf("\n%s: \"...\"\n", enemy->name);

    sleep(3);

    printf("\n%s: \"Ha... Friend? Don't be freaking ridiculous with me!?\"\n", enemy->name);

    sleep(2);

    printf("\n%s: \"Had you even truly care about me? had you ask me if I'm being fine? NO!\"\n", enemy->name);

    sleep(2);

    printf("\n%s: \"I am. The One who tried. Keep it up. I think it's just a time the separate us...\"\n", enemy->name);

    sleep(2);

    printf("\n%s: \"But now I know I was wrong... All you care is YOURSELF!\"\n", enemy->name);

    sleep(2);

    printf("\n%s: \"Haha... What am I expecting from you...\"\n", enemy->name);

    sleep(3);

    printf("\n%s: \"I-I...\"\n", player->name);

    sleep(2);

    printf("\n%s: \"I don't want to hear anytime from you... I had enough of you. I can't blame you after all.\"\n", enemy->name);

    sleep(1);

    printf("\n%s converts 'Love' to 'Hate' (Gain +0.5%% Damage Multiplier); then heal up to 100%% HP\n", enemy->name);

    sleep(1);

    printf("\n%s: \"What should I do? am I the one in wrong way...?\"\n", player->name);

    sleep(1);

      updateSanity(player, -100);

    player->SanityFreezeTurns -= 1;

    printf("\n%s loses 100 Sanity (%d)\n", player->name, player->Sanity);

    sleep(1);

  }

  if (isId(enemy->ID, "Your Best Friend") == 0 && enemy->Passive == 3) {

  enemy->Passive = 4;

    printf("\n%s: \"No... This isn't you...\"\n", player->name);

    sleep(1);

    printf("\n%s: \"You must in control or something.\"\n", player->name);

    sleep(2);

    printf("\n%s: \"Face the truth '%s'... This is me\"\n", enemy->name, player->name);

    sleep(3);

  }















  // ------------- The House of Spiders: The Thumb Nursefather Rodion -------------

  // The House of Spiders: The Thumb Nursefather Rodion Passive
  if (isId(player->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0) {

      // Overheat gain 10
      if (player->skills[11].active == 1) {

        if (player->Passive <= 0) {
          if (*playerSkill1 != 5 || *playerSkill2 != 5) *playerSkill2 = 5;
          printf("\n%s Replacing Skill with '%s'\n", player->name, player->skills[5].name);

          sleep(1);

          printf("\n%s: \"... Tsk. Overheated already?\"\n", player->name);

          sleep(1);

        }

        player->Passive += 10;
        if (player->Passive > 30) player->Passive = 30;

        printf("\n%s gains 10 Eye of Precognition - Overheat (%d)\n", player->name, player->Passive);

        sleep(1);

        if (player->Passive >= 30) {

          player->skills[11].active = 0;
          
          printf("\n%s converts 'Eye of Precognition - Overheat' into 'Eye of Precognition'\n", player->name);

          sleep(1);
        }

        if (*playerSkill1 == 5 || *playerSkill2 == 5) {

          enemy->ProtectionDown[0] += 15;

        printf("\n%s inflicts 'Game Target' on the enemy\n", player->name);

        sleep(1);
        }
        
      }

    if (player->Passive == -1) {

      player->Passive = 30;

    printf("\n%s gains 30 Eye of Precognition\n", player->name);

    sleep(1);

    }

    if (player->skills[10].active == -1) {

      player->skills[10].active = 10;

    printf("\n%s gains 10 Acceleration Round\n", player->name);

    sleep(1);

    }

  }

  // --------------------------------------------------------------------------------------------------------
  

  // ------------- The House of Spiders: The Ring Nursefather Hong Lu -------------

  // The House of Spiders: The Ring Nursefather Hong Lu Passive
  if (isId(player->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0) {

      // Skill Replacement (เมื่อ Tableau หมด)
      if (player->Passive == 0) {
          // แทนที่สกิลแถวบน (สกิล 0) ด้วยสกิล 4 (Closing Time)
          if (*playerSkill1 != 4) *playerSkill1 = 4;
          printf("\n%s Replacing Skill with 'Closing Time - Installation Art no. 1: Your Flesh and Bones as the Gallery's Seats'\n", player->name);

        sleep(1);

        printf("\n%s: \"Preparations for the exhibition have covered all corners.\"\n", player->name);

        sleep(1);
      }

    if ((player->Passive == 21 || (player->Passive == -1 && player->skills[8].active == 1))) {

      player->Passive = 21;

    printf("\n%s gains 21 Viewing the Tableau\n", player->name);

    sleep(1);

    }

  }

  // --------------------------------------------------------------------------------------------------------

  // ------------- Hong lu:The Lord of Hongyuan -------------

  // Hong lu:The Lord of Hongyuan - Heishou Bolus Contamination [黑獸丸染] Buff
  if (isId(player->ID, "Hong lu:The Lord of Hongyuan") == 0) {

     player->MinSpeed = 4;
    player->MaxSpeed = 8;

    if (player->Passive >= 1) {

      player->MaxSpeed += player->Passive;
      player->DamageUp[0] += player->Passive*2;

      printf("\n%s at 1+ Heishou Bolus Contamination [黑獸丸染], gains +1 Max Speed and +2%% Damage Up for every Stack (%d)", player->name, player->Passive);

     }

    if (player->Passive >= 2) {

      player->MinSpeed += player->Passive;
      player->DefenseLevelUp[0] += player->Passive;

      printf("\n%s at 2+ Heishou Bolus Contamination [黑獸丸染], gains +1 Min Speed and +1 Defense Level Up for every Stack (%d)", player->name, player->Passive);

     }

    if (player->Passive >= 3) {

      player->OffenseLevelUp[0] += player->Passive;

      printf("\n%s at 3+ Heishou Bolus Contamination [黑獸丸染], gains +1 Offense Level Up for every Stack (%d)", player->name, player->Passive);

     }

    if (player->Passive >= 4) {

      player->OffenseLevelUp[0] += 4;
      player->DamageUp[0] += 10;

      printf("\n%s at 4+ Heishou Bolus Contamination [黑獸丸染], gains +4 more Offense Level Up and +10%% Damage Up (%d)", player->name, player->Passive);

     }

    if (player->Passive >= 1) {

      printf("\n");

    }

    sleep(1);
  }

   // ------------- Meursault:The Thumb -------------

  // Shin buffs
  if ((isId(player->ID, "Meursault:The Thumb") == 0 && player->skills[3].active)) {

    if (player->skills[2].active < 8) {

      player->MinSpeed = 5;
      player->MaxSpeed = 7;

      if (player->Passive == 0) {
        player->MinSpeed = 7;
        player->MaxSpeed = 9;
      }

    } else if (player->skills[2].active >= 8) {

        player->MinSpeed = 7;
      player->MaxSpeed = 9;

      if (player->Passive == 0) {
        player->MinSpeed = 9;
        player->MaxSpeed = 11;
      }

    }

  }

    // -----------------------------------------------------------------

  // ------------- The Middle Nursefather - Matthias -------------

  // The Middle Nursefather - Matthias - Reset skill
  if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0 && enemy->skills[9].active == 3 && enemy->skills[10].active == 0) {

    enemy->skills[10].active = 1;

    getSkills(enemy, enemySkill1, enemySkill2, enemySkill3, -1, enemy->numSkills);

    if (enemySkillEffective != NULL) {
      int pool[] = {7, 8, 9}; // รายชื่อสกิลใหม่ของร่าง Lævateinn
      int count = 3;
      int totalWeight = 0;

      // คำนวณน้ำหนักรวมจาก Copies (เหมือน pickSkill)
      for (int i = 0; i < count; i++) {
          totalWeight += enemy->skills[pool[i]].Copies;
      }

      if (totalWeight > 0) {
          int r = rand() % totalWeight;
          int cum = 0;
          for (int i = 0; i < count; i++) {
              cum += enemy->skills[pool[i]].Copies;
              if (r < cum) {
                  // เปลี่ยนสกิลที่ Matthias ถืออยู่ในมือตอนนี้ทันที!
                  *enemySkillEffective = &enemy->skills[pool[i]]; 
                  break;
              }
          }
      }

    }

  }

  // The Middle Nursefather - Matthias - Moses help
  if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0) {
    player->TempShield += 30;

    int gain = player->Sanity/10;

    if (gain < 0) gain = 0;

    player->OffenseLevelUp[0] += gain;
    player->DefenseLevelUp[0] += gain;

    printf("\n%s gains 30 Shield (%.2f) and gains (Sanity/10) Offense Level Up and Defense Level Up (%d; rounded down)\n", player->name, (player->TempShield + player->Shield), gain);
  }

  // The Middle Nursefather - Matthias - Change Phase
  if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0) {

    float hpRate = enemy->HP / enemy->MAX_HP;

    if (hpRate <= 0.7 && enemy->skills[9].active == 0 && enemy->skills[4].Coins > 0) {

      if(enemySkillEffective) *enemySkillEffective = &enemy->skills[4];

    } else if (hpRate <= 0.5 && enemy->skills[9].active == 1 && enemy->skills[5].Coins > 0) {

        if(enemySkillEffective) *enemySkillEffective = &enemy->skills[5];

      } else if (hpRate <= 0.2 && enemy->skills[9].active == 2 && enemy->skills[6].Coins > 0) {

      if(enemySkillEffective) *enemySkillEffective = &enemy->skills[6];

    } // The Middle Nursefather - Matthias - at 10 The Middle - Grudge
    else if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0 && enemy->skills[0].active == 10 && enemy->skills[2].Coins > 0 && (enemy->skills[9].active != 3)) {
      if(enemySkillEffective) *enemySkillEffective = &enemy->skills[2];
    }
  }

  if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0 && enemy->skills[9].active == 3 && *enemySkillEffective != &enemy->skills[6]) {

            inflictStatus(player->Burn, 5, 2, 0, 99, 0, 99);
    inflictStatus(enemy->Burn, 5, 2, 0, 99, 0, 99);

          printf("\n%s gains +5 Burn Stack (%d) and +2 Burn Count (%d)\n", player->name, player->Burn[0], player->Burn[1]);

        sleep(1);

    printf("\n%s gains +5 Burn Stack (%d) and +2 Burn Count (%d)\n", enemy->name, enemy->Burn[0], enemy->Burn[1]);

    sleep(1);
  }

  if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0) {
      int activeTattoo = enemy->skills[2].active; // ดึงค่าที่ฝากไว้จากเทิร์นที่แล้วมาใช้

      if (activeTattoo > 0) {
            enemy->DamageUp[0] += 10;
          printf("\n%s dealing +10%% damage from 'The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]' (%d)\n", enemy->name, enemy->skills[2].active);

        sleep(1);
      }
  }

  if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0 && enemy->Passive == 0) {

    enemy->Passive = 1;

    printf("\n%s gains 'The Book of Vengeance'\n", player->name);
      printf("\n%s: \"The Middle never forgets.\"\n", enemy->name);
      // ในที่นี้คือการทำให้ player มีสถานะพิเศษที่เมื่อตี Matthias แล้วจะได้ Sanity +7
    sleep(1);

    printf("\n%s gains 'Sealed Sword' (Unlocks access to certain Skills based on the state of the sword.)\n", enemy->name);

    sleep(1);
  }

  // The Middle Nursefather - Matthias - Passive Rest
  if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0) {

      // Reset ตัวนับเทิร์น
        enemy->skills[5].active = 0; // Reset First Clash Win flag
        enemy->skills[6].active = 0; // Reset Matthias hit counter
  }

  // -----------------------------------------------------------------

  // --------------- Muga Ryōshū ---------------

  // Muga Ryōshū – gains Speed
  if (isId(player->ID, "Muga Ryōshū") == 0 && player->skills[0].active/3 > 0) {

      applyDamage(player, enemy, player->skills[0].active/3, 0, NULL);

      printf("\n%s takes %d fixed damage\n", enemy->name, player->skills[0].active/3);

      sleep(1);

    inflictStatus(enemy->Bleed, 3, 2, 0, 99, 0, 99);

    printf("\n%s gains +3 Bleed Stack (%d) and +2 Bleed Count (%d)\n", enemy->name, enemy->Bleed[0], enemy->Bleed[1]);

    sleep(1);

  }

  // Muga Ryōshū – gains Speed
  if (isId(player->ID, "Muga Ryōshū") == 0) {

    if (player->skills[1].active == 0) {
      player->skills[1].active = 1;

        player->MinSpeed += 6;
      player->MaxSpeed += 6;

      printf("\n%s gains 'Tiansha Star's Blade - Arayashiki [天殺星刀阿賴耶識]'\n", player->name);

      sleep(1);
    }

  }

  if (isId(player->ID, "Muga Ryōshū") == 0) {
      // 1. Wading Through a Dream: รับ Muga ตามจำนวนเทิร์น

    // ยิ่ง Muga เยอะ ระดับการ Glitch ยิ่งสูง
    int intensity = player->Passive/10; 

    if (intensity > 0) {

      for (int i = 0; i < player->numSkills; i++) {
          // เงื่อนไข: ไม่ใช่ท่าไม้ตาย (index 5), สกิลมีชื่ออยู่จริง, และสกิลยังไม่พัง (Copies >= 0)
          if (i != 5 && player->skills[i].name != NULL && player->skills[i].Copies >= 0) {
             player->skills[i].name = glitchText(player->skills[i].name, intensity);
          }
      }

            player->name = glitchText(player->name, intensity/2); // ชื่อ
    }
    // ----------------------

      player->Passive += TurnCount;
      if (player->Passive > 100) player->Passive = 100;
    printf("\n%s gains +%d Muga [無我] (%d)\n", player->name, TurnCount, player->Passive);

    sleep(1);

      // 2. Severed and Torn: Inflict Sever the Thread more อัตโนมัติทุกต้นเทิร์น
      // สูตร: 1 (พื้นฐาน) + (Muga/10; Max 4)
    player->skills[10].active = 0;

      int moreInflict = 2 + (player->Passive / 10);
      if (moreInflict > 6) moreInflict = 6; 
      player->skills[10].active += moreInflict; // ค่าสูงสุด Inflict บนตัวศัตรู

      // 3. Offense Level จากการโดนตีในตาที่แล้ว (สะสมจาก applyDamage)
      if (player->skills[11].active > 0) {
          player->OffenseLevelUp[0] += player->skills[11].active * 3;
          printf("\n%s gains +%d Offense Level from hits taken (Max 6)\n", player->name, player->skills[11].active * 3);
          player->skills[11].active = 0; // reset counter
        sleep(1);
      }

  }

  // ------------------------------------------------------------------------------------------

  // Erlking Heathcliff – always use skill 9 if HP ≤ 50
  if (isId(player->ID, "Erlking Heathcliff") == 0 && player->HP <= 50) {

      player->Speed = 1;

    printf("\n%s Fixed Speed to 1\n", player->name);

  }

  // Sukuna:King of Curse Domain expansion
  if (isId(enemy->ID, "Sukuna:King of Curse") == 0 && enemy->skills[8].active <= 0 && (enemy->Passive == 1)) {

      enemy->skills[8].active = 1;

    enemy->skills[3].BasePower += 5;

    enemy->skills[3].Unbreakable += 1;

    enemy->skills[3].Copies = 3;

        printf("\n\x1b[1;31m[!] THE ATMOSPHERE CURDLES...\x1b[0m\n"); // ตัวหนังสือสีแดงเข้ม
        sleep(1);

        printf("\n%s: \"You might have defeated me in the state I was in back then...\"\n", enemy->name);
        sleep(2);

        printf("\n%s: \"But you have yet to witness... \x1b[1;31mTrue Jujutsu.\x1b[0m\"\n", enemy->name);
        sleep(2);

        // Effect ก่อนกางอาณาเขต
        printf("\n. . ."); sleep(1); printf(" . . ."); sleep(1); printf(" . . .\n");

        printf("\n\x1b[1;31mDOMAIN EXPANSION... (領域展開)\x1b[0m\n");
        sleep(2);


        printf("\n\x1b[1;31m"); // เริ่มใช้สีแดง
        printf("#########################################################\n");
        printf("##                                                     ##\n");
        printf("##           \x1b[1;37m伏 魔 御 厨 子 (MALEVOLENT SHRINE)\x1b[1;31m        ##\n");
        printf("##                                                     ##\n");
        printf("#########################################################\n");
        printf("\x1b[0m"); // ล้างสี

        printf("\n\x1b[3mThe environment transforms into a landscape of bones and blood.\x1b[0m\n");
        printf("\x1b[3mAn ornate, demonic Buddhist shrine rises from the visceral swamp.\x1b[0m\n");
        printf("\n\x1b[1;31mALL TARGETS WITHIN RANGE WILL BE CUT RELENTLESSLY.\x1b[0m\n");

        sleep(3);
    }











// The One Who Grips Faust - Turn Start
  if (isId(player->ID, "The One Who Grips Faust") == 0) {
      // 1. Whistles Logic
      if (player->skills[3].active >= 3) {
          updateSanity(player, 15);
          player->Passive += 2; 
          player->skills[3].active = 0; 
          printf("\n%s heals 15 Sanity (%d) and gains 2 Fanatic (%d)\n", player->name, player->Sanity, player->Passive);
      }

      // Reset Bliss Flag (สิทธิการตีซ้ำ)
      player->skills[5].active = 0;

    // 3. Nail Turn Start: ตะปูสร้างเลือดไหล (เก็บค่าไว้ที่ตัว Faust เอง)
    if (player->skills[2].active > 0) {
        int nailAmt = player->skills[2].active;
        enemy->Bleed[0] += 1;       // +1 Bleed Stack
          enemy->Bleed[1] += nailAmt; // +Nail Bleed Count
        printf("\n%s gains +1 Bleed Stack (%d) and +%d Bleed Count (%d)\n", enemy->name, enemy->Bleed[0], nailAmt, enemy->Bleed[1]);
    }
  }

  // The Middle Little Brother Sinclair - Passive Once per Turn reset
  if (isId(player->ID, "The Middle Little Brother Sinclair") == 0) {

    player->skills[3].active = 0;

  }

  //Roland Furioso
  if (isId(enemy->ID, "Fixer grade 9?") == 0) {

    int allInactive = 1; // assume all inactive

    // check skills 0–8
    for (int i = 0; i <= 8; i++) {
        if (enemy->skills[i].Copies > 0) {
            allInactive = 0; // found one active
            break;
        }
    }

    if (allInactive == 1) {

      if(enemySkillEffective) *enemySkillEffective = &enemy->skills[9];

      if (enemy->Stagger > 0) {
          enemy->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            enemy->name);

        sleep(1);
        }

      sleep(1);
    } else {
        // Normal random selection
        if(enemySkillEffective) *enemySkillEffective = &enemy->skills[(rand() % 2 == 0 ? *enemySkill1 : *enemySkill2)];
    }

  } 

  // ------------------ The House of Spiders: The Index Nursefather Yi Sang ------------------

  if (strcmp(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 &&
     strcmp(enemy->ID, "Fixer grade 9?") == 0) {

    player->ClashPowerDown[0] += 3;

  }

  // The House of Spiders: The Index Nursefather Yi Sang gains Wound-casing Mask at start
  if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && player->skills[3].active == 0) {

    player->skills[3].active = 1;

    printf("\n%s gains 'Wound-casing Mask'\n", player->name);

    sleep(1);

  }


  // The House of Spiders: The Index Nursefather Yi Sang - Index Target
  if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {

    // Sizzling Wound DOT
    if (player->skills[3].active == 2) {
      player->Burn[0]++; 
      player->Bleed[0]++;
      printf("\n%s gains 1 Burn Stack (%d) and 1 Bleed Stack (%d)\n", player->name, player->Burn[0], player->Bleed[0]);

      sleep(1);

    }

      // แปะสถานะ Target ไว้ที่ตัวศัตรู
        player->skills[11].active = 1; // Index Target
    enemy->ProtectionDown[0] += 10;
      printf("\n%s gains 'The Prescript's Target' (Takes +10%% damage from Index units)\n", 
             enemy->name);
  }

  // The House of Spiders: The Index Nursefather Yi Sang focred Furioso
  if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {
      if (player->skills[1].active >= 9 && *playerSkill1 != 3 && *playerSkill2 != 3) {
          *playerSkill1 = 3; // บังคับให้ช่องที่ 1 เป็น Furioso ทันที
      }
  }

  // --- The House of Spiders: The Index Nursefather Yi Sang - Turn Start ---
  if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {


    // ตรวจสอบสถานะการทำงาน: ถ้า Staggered หรือ Panicked จะไม่รัน Prescript ใดๆ ทั้งสิ้น
    if (player->Stagger > 0 || isPanicked(player)) {
        printf("\n%s is Staggered or in Panic. doesn't gain 'Prescript: [Device]' this turn.\n", player->name);

        // กำหนด Flag พิเศษ (เช่น skills[5].active = 2) เพื่อบอกว่าเทิร์นนี้ "งดเช็ค" 
        // เพื่อไม่ให้ไปโดนบวกแต้ม Karma (Karmic Consequence) ในตอนจบเทิร์น
        player->skills[5].active = 2; 
    } 
    else {

    if (player->skills[7].active > 0) {

      player->skills[1].active += player->skills[7].active;
      player->skills[13].active += player->skills[7].active; // Maximum per turn

      // For prescript III Check
      if (player->Passive == 2) {
        player->skills[5].active = 1;
        }

      printf("\n%s gains +%d 'Procuration [Hermes]'\n", player->name, player->skills[7].active);

      player->skills[7].active = 0;

    }

    // จัดการเรื่อง Device Level ตาม Unlock Stage (Passive)
    // Passive 0 = I, 1 = II, 2 = III, 3 = IV
    const char* deviceLevels[] = {"I", "II", "III", "IV"};
    int currentStage = player->Passive;
    if (currentStage > 3) currentStage = 3; // กันเหนียวถ้า Stage เกิน

      if (currentStage == 3) {
    printf("\n%s gains 'Prescript: [Device] %s' : Eliminate all enemies before next Prescript arrvies.\n", player->name, deviceLevels[currentStage]);
      } else if (currentStage == 2) {
          printf("\n%s gains 'Prescript: [Device] %s' : Gain Procuration [Hermes]. Repeat this Prescript until Procuration [Hermes] reaches 9 Stacks.\n", player->name, deviceLevels[currentStage]);
            } else if (currentStage == 1) {
        printf("\n%s gains 'Prescript: [Device] %s' : Hit a target with a Skill with 'Mark of the Prescript'.\n", player->name, deviceLevels[currentStage]);
          } else {
        printf("\n%s gains 'Prescript: [Device] %s' : Use a Skill with 'Mark of the Prescript'.\n", player->name, deviceLevels[currentStage]);
          }
    sleep(1);

    player->skills[5].active = 0; // Prescript Checked reset
    player->skills[6].active = 0; // Two time per time reset
    player->skills[15].active = 0; // Two time per time reset
    player->skills[16].active = 0; // Two time per time reset
    player->skills[17].active = 0; // Two time per time reset
    player->skills[18].active = 0; // Two time per time reset

    // (ตรรกะเดิมของคุณ) การสุ่มเลือกสกิลตาม Prescript
    if (player->Passive >= 2) { 
      if (*playerSkill1 == 2 || *playerSkill2 == 2 || *playerSkill1 == 3 || *playerSkill2 == 3) {
          if (*playerSkill1 == 2 || *playerSkill1 == 3) player->skills[4].active = *playerSkill1;
          else player->skills[4].active = *playerSkill2;
      } else {
          player->skills[4].active = (rand() % 2 == 0) ? *playerSkill1 : *playerSkill2;
      }
    } else {
      player->skills[4].active = (rand() % 2 == 0) ? *playerSkill1 : *playerSkill2;
    }

    // --- เปลี่ยนการแสดงผลตรงนี้ ---
    printf("\n%s's [Prescript [Device] %s] : Mark Skill '%s'!\n", 
           player->name, deviceLevels[currentStage], player->skills[player->skills[4].active].name);

    sleep(1);

    // 5. เช็คแต้ม Hermes เพื่อใช้ท่าไม้ตาย (ตรรกะเดิม)
    if (player->skills[1].active >= 9) {

        printf("\n%s Procuration [Hermes] at 9! 'Furioso-Replica' is ready.\n", player->name);

      sleep(1);

        // --- ส่วน Skill Conversion Logic ---
        // เช็คว่าในมือ (Dashboard) มี Furioso หรือยัง
        if (*playerSkill1 != 3 && *playerSkill2 != 3) {
            printf("\n%s [Imitation of a Life] No Furioso on Dashboard! Converting top slot next turn\n", player->name);
            // หมายเหตุ: การเปลี่ยนค่า playerSkill1 ตรงๆ ในนี้จะไม่มีผลต่อ main 
            // เพราะ C รับค่า playerSkill1 แบบ Value (สำเนา) ไม่ใช่ Pointer
            // วิธีแก้: คุณต้องไปเพิ่มบรรทัด conversion ใน main() ตามที่อธิบายด้านล่าง
           sleep(1);
        }
    }

    }

    // 5. บทลงโทษ Karma
    if (player->skills[2].active >= 10) {
        int penalty = player->skills[2].active / 10;
        player->DefenseLevelDown[0] += penalty;
        printf("\n%s's Karmic Consequence (%d): Defense Level -%d\n", player->name, player->skills[2].active, penalty);

      sleep(1);
    }
    if (player->skills[2].active >= 20) {
        int penalty = player->skills[2].active / 20;
        player->ProtectionDown[0] += penalty * 10;
        printf("%s's Karmic Consequence (%d): Take +%d%% damage\n", player->name, player->skills[2].active, penalty * 10);

      sleep(1);
    }

  }

  // ------------------------------------------------------------------------------------------

    // ---------------- Sancho:The Second Kindred of Don Quixote -----------------------

  // Sancho:The Second Kindred of Don Quixote – heal HP at start
  if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0) {

    int healvalue = (((enemy->MAX_HP - enemy->HP) / enemy->MAX_HP) * 100)/2;
    if (healvalue > 50) healvalue = 50;
    if (healvalue > 0) {

    enemy->HP += healvalue;
      if (enemy->HP > enemy->MAX_HP) enemy->HP = enemy->MAX_HP;

    printf("\n%s heals (percentage missing HP/2) HP (%d - Max 50)\n",
           enemy->name, healvalue);

    sleep(1);
    }
  }

    // Sancho:The Second Kindred of Don Quixote – heal sanity at -15 Sanity or less
    if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && enemy->Sanity <= -15 && enemy->HP > 1) {

      int healvalue = ((enemy->MAX_HP - enemy->HP) / enemy->MAX_HP * 100)/2;
      int consumed = 5 - (enemy->Sanity/5);
      if (consumed > enemy->HP) consumed = enemy->HP + 1;

      if (enemy->Passive >= consumed) {
      enemy->Passive -= consumed;
      if (enemy->Passive < 1) enemy->Passive = 1;
      updateSanity(enemy, healvalue);
      enemy->ClashPowerUp[0] += 2;

      printf("\n%s at -15 or less Sanity, consumes %d Hardblood (%d left) to gain %d Sanity and gain 2 Clash Power Up\n",
             enemy->name, consumed, enemy->Passive, healvalue);

      sleep(1);
      }
    }

  // Sancho:The Second Kindred of Don Quixote with Don Quixote:The Manager of La Manchaland - Power down
  if (isId(player->ID, "Don Quixote:The Manager of La Manchaland") == 0 &&
    isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && enemy->skills[5].active == 0) {

    enemy->skills[5].active = 1;

    enemy->Sanity = -45;
    enemy->SanityFreezeTurns = 1;

    printf("\n%s gains 'Call of Mother', Start Phase with -45 Sanity \n", enemy->name);

    sleep(1);

    printf("\n%s: \"Don't you hear it? A call...\"\n", player->name);

    sleep(1);
  }

    // -----------------------------------------------------------


  // -------------------------- Heathcliff:Wild Hunt -------------------------

  // Heathcliff:Wild Hunt - Call of erlking
    if (isId(player->ID, "Heathcliff:Wild Hunt") == 0 && (player->HP <= player->MAX_HP * 0.5 || player->Sanity <= -45) && player->skills[0].active == 0 && !player->skills[1].active) {

      player->skills[1].active = 1;
      player->skills[0].active += 1;
      if (player->skills[0].active > 3) player->skills[0].active = 3;

      if (player->Sanity <= -45) {player->SanityFreezeTurns = 0;}

      player->MinSpeed += 1;
      player->MaxSpeed += 1;

      printf("\n%s at 50%% or less HP, or at -45 Sanity, if this unit at -45, does not 'Panic' and if this unit does not have 'Dullahan', gain 'Dullahan' and snap out from 'Stagger' (Once per Encounter)\n", player->name);

      if (enemy->Stagger > 0) {
        enemy->Stagger = 0;

        printf("\n%s recovers from 'Stagger'\n",
          enemy->name);

        sleep(1);
      }

      if (player->Sanity <= 0) {
      int missingSP = -player->Sanity;       // how far below 0
        int extraHeal = 2 * missingSP;           // 2 Sanity per missing SP
        if (extraHeal > 50) extraHeal = 50;      // cap at 50

        int totalHeal = 10 + extraHeal;          // base 10 + extra
      updateSanity(player, totalHeal);

      printf("\n%s heals %d Sanity(%d)\n",
        player->name, totalHeal, player->Sanity);
      }

    }

  // ------------------------------------------------


  // Erlking Heathcliff – always use skill 9 if HP ≤ 50
  if (isId(enemy->ID, "Erlking Heathcliff") == 0 && enemy->HP <= 50 &&
    !enemy->skills[8].active && enemy->Passive == 2) {

    enemy->skills[8].active = 1;

    enemy->Speed = 1;

    if(enemySkillEffective) *enemySkillEffective = &enemy->skills[8];

  }



  // --------------------- Heishou Pack - You Branch Adept Heathcliff ------------------

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 Unbreakable coins reset
  if (isId(player->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0) {

    player->skills[2].Unbreakable = 0;

    if (player->Passive >= 20 && player->HP <= player->MAX_HP * 0.5) player->skills[2].Copies = 3; // S3 Pity
    else player->skills[2].Copies = 1;

  }

  // Heishou Pack - You Branch Adept Heathcliff - Heal from anti death
  if (isId(player->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && player->skills[3].active == -2) {

    player->skills[3].active--;

    int healHPpercentag = player->Burn[0] + 20;
    if (healHPpercentag > 49) healHPpercentag = 49;
    int healvalue = (player->MAX_HP * healHPpercentag/100);

    player->HP += (player->HP + healvalue) > player->MAX_HP ? player->MAX_HP - player->HP : healvalue;

    printf("\n%s heals %d%% HP (%.2f), and remove all Burn on self (Max 49%%; Once per Encounter)\n",
      player->name, healHPpercentag, player->HP);

      player->Burn[0] = 0;
        player->Burn[1] = 0;

  }

  // --------------------------------------------------------

  // Meursault:The Thumb - Skill 3 Unbreakable coins reset
  if (isId(player->ID, "Meursault:The Thumb") == 0) {

    player->skills[2].Unbreakable = 0;

  }

  if (isId(player->ID, "Meursault:The Thumb") == 0 && !player->skills[3].active && player->Passive == 0) {

    int amount = ((int)(12 * enemy->MAX_HP)) / 847;
    if (amount < 12) amount = 12;

    if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 || isId(enemy->ID, "Don Quixote") == 0) amount += 4; // pity for boss
    if (isId(enemy->ID, "Sukuna:King of Curse") == 0) amount += 6; // pity for boss
    if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0) amount += 10; // pity for boss

    player->Passive = amount;
    player->defenseSkill[2].active += amount; // Gain value

    printf("\n%s gains %d Tigermark Rounds\n", player->name, amount);

    sleep(1);

  }

  // ------------------- Dawn Office Fixer Sinclair -----------------------------

  // Dawn Office Fixer Sinclair - Ego
  if (isId(player->ID, "Dawn Office Fixer Sinclair") == 0 && !player->skills[3].active && player->HP <= player->MAX_HP * 0.3 && player->Sanity > -45 && player->skills[0].active) {

    player->skills[0].active = 0;
     player->Sanity = 45;
      player->skills[2] = player->skills[5];
      player->skills[3] = player->skills[5];
      player->skills[3].active = 1;
      player->skills[2].Copies = 0;
      player->skills[3].Copies = 1;

    printf("\n%s at 30%% or less HP and Sanity isn't -45, reset Sanity to 45; then enters the Volatile E.G.O::Waxen Pinion state (Once per Encounter)\n", player->name);

    sleep(1);

    printf("\n%s: \"If sorrow has become my indelible stigma, then... I choose to rise with it instead!\"\n", player->name);

    sleep(1);

  } else if (isId(player->ID, "Dawn Office Fixer Sinclair") == 0 && !player->skills[3].active && player->Sanity >= 40) {

     updateSanity(player, -(20));
    player->skills[2] = player->skills[5];
    player->skills[3] = player->skills[5];
    player->skills[3].active = 1;
    player->skills[2].Copies = 0;
    player->skills[3].Copies = 1;

    printf("\n%s consumes 20 Sanity(%d) to enter the Volatile E.G.O::Waxen Pinion state\n", player->name, player->Sanity);

    sleep(1);

    printf("\n%s: \"This time... I'll definitely put an end to this!\"\n", player->name);

    sleep(1);

  } else if (isId(player->ID, "Dawn Office Fixer Sinclair") == 0 && player->skills[3].active && player->Sanity <= 0) {

    int clashpowerbuff = 3 * player->Passive;
    if (clashpowerbuff > 20) clashpowerbuff = 20;

     player->Passive = 0;
    player->skills[3] = player->skills[4];
    player->skills[2] = player->skills[4];
    player->skills[3].active = 0;
    player->skills[2].Copies = 1;
    player->skills[3].Copies = 0;
    player->ClashPowerUp[0] = clashpowerbuff; player->ClashPowerDown[0] = 0;
    player->ClashPowerUp[1] = clashpowerbuff; player->ClashPowerDown[1] = 0;


      printf("\n%s exits the Volatile E.G.O::Waxen Pinion state and loses all 'Volatile Passion' to gain +(Volatile Passion x 3) (%d - Max 20) Clash Power for this turn and next turn\n", player->name, clashpowerbuff);

      sleep(1);

    printf("\n%s: \"... I have to be bold!\"\n", player->name);

    sleep(1);


    }

  // Dawn Office Fixer Sinclair - Volatile Passion
  if (isId(player->ID, "Dawn Office Fixer Sinclair") == 0 && player->skills[3].active) {

      player->Passive += 1;

    player->FinalPowerUp[0] += 1 * player->Passive;
      player->DamageUp[0] += 20 * player->Passive;
    player->ProtectionUp[0] += 10 * player->Passive;

    printf("\n%s gains 1 'Volatile Passion', gain 1 Final Power (%d), gain +20%% Damage (%d%%) and Take -10%% Damage (%d%%) for every stack (%d)\n", player->name, 1 * player->Passive, 20 * player->Passive, 10 * player->Passive, player->Passive);

    sleep(1);

  }

  // ------------------------------------------------------------------ 

  // ------------------- Jia Qiu -----------------------------
  // Jia Qiu - Power down
  if (isId(enemy->ID, "Jia Qiu") == 0) {

    if (enemy->Passive == 0) {

    enemy->Passive = 1;

    for (int i = 0; i < enemy->numSkills; i++) {
        enemy->skills[i].Offense -= 35;
        enemy->skills[i].Defense -= 35;
    }

      for (int i = 0; i < enemy->numDefenseSkills; i++) {
          enemy->defenseSkill[i].Offense -= 35;
          enemy->defenseSkill[i].Defense -= 35;
      }

    enemy->DamageUp[0] += 30;
      updateSanity(enemy, 30);

    printf("\n%s gains 'A Sliver of Anticipation', He's not giving it his all. Heal 30 Sanity, Lose 35 Offense, 35 Defense and Deal 30%% more damage\n",
           enemy->name);

    sleep(1);

    } else {
      enemy->DamageUp[0] += 30;
    }
  }

  // Jia Qiu - Last attack at 85% HP
    if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP < enemy->MAX_HP * 0.85 && enemy->Passive == 2 && enemy->skills[5].active == 0 && !isStaggered(enemy)) {

      if(enemySkillEffective) *enemySkillEffective = &enemy->skills[5];

      enemy->skills[5].active = 1;
    }

  // -------------------------------------------------------------

  //--------------------- Lei heng -----------------------------

  // Lei heng – Chosen Prey (Turn Start)
  if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[0].active >= 1 && enemy->skills[7].active > 0) {
    int preyChance = 50 - enemy->Sanity;
    if (preyChance < 0) preyChance = 0;
    if (preyChance > 0 && (rand() % 100) < preyChance) {
      enemy->skills[6].active = 1;
      printf("\n%s inflicts 'Prey' on %s\n", enemy->name, player->name);
      sleep(1);
    }

      enemy->skills[7].active = 0; // dmg taken reset
  }

  // Lei heng – if HP ≤ 60%
  if (isId(enemy->ID, "Lei heng") == 0 && (enemy->HP <= enemy->MAX_HP * 0.6 || (enemy->skills[1].active >= 1 && enemy->skills[2].active >= 5)) &&
    enemy->skills[0].active == 2 && enemy->skills[4].active == 0) {

    enemy->skills[4].active = 1; // Active 'Tiantui Star [天退星]'

    if (enemy->Stagger > 0) {
      enemy->Stagger = 0;

      printf("\n%s recovers from 'Stagger'\n",
        enemy->name);

      sleep(1);
    }

    enemy->skills[1].Unbreakable = 1;

    printf("\n%s activates 'Tiantui Star [天退星]'!\n", enemy->name);

    sleep(1);

      enemy->defenseSkill[1].active = 0; // Tigermark rounds

    enemy->defenseSkill[3].active += 18;

    int current_clip = enemy->defenseSkill[2].active;
    int reserve = enemy->defenseSkill[3].active;
    int needed = 6 - current_clip; // ขาดอีกเท่าไหร่จะเต็ม 6

        int take = (reserve >= needed) ? needed : reserve;
            enemy->defenseSkill[2].active += take;
            enemy->defenseSkill[3].active -= take;

    printf("\n%s loses all Tigermark Round; then converts the effect Tigermark Round to Savage Tigermark Round and gains 18 Savage Tigermark Round (Count) and uses 'Reload - Tactical' (Reload this unit's Ammo to its maximum capacity (does not discard any remaining Ammo))"
      "\n Savage Tigermark Round:\n"
      " - Stack (Loaded Ammo) : %d\n"
       " - Count (Remaining Ammo) : %d\n",
       enemy->name, enemy->defenseSkill[2].active, enemy->defenseSkill[3].active);

    sleep(1);

    printf("\n%s: \"Can't leave a dance unfinished. Ain't that right?\"\n", enemy->name);

     sleep(1);
  }

  // Lei heng – if HP ≤ 40%
  if (isId(enemy->ID, "Lei heng") == 0 && (enemy->HP <= enemy->MAX_HP * 0.4 || (enemy->skills[1].active >= 2 && enemy->skills[2].active >= 5)) &&
    enemy->skills[0].active == 2) {

    enemy->skills[0].active = 3; // Phase 4

    printf("\n%s converts 'Inner Strength [底力]' to 'Extreme Strength [極力]'\n",
      enemy->name);

    sleep(1);

    enemy->skills[0].Unbreakable = 1;

    printf("\n%s converts 'Tiantui Star [天退星]' to 'Shin (心) - Tiantui Star [天退星]'\n",
      enemy->name);

    sleep(1);

    printf("\n%s replaces the powerful attack '%s' with '%s'\n", enemy->name,
      enemy->skills[2].name, enemy->skills[4].name);

    sleep(1);

    GainNewPattern(enemy, player);

  }

  // Lei heng – skill 3 using first fight
  if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[2].active == 0 && enemy->skills[0].active == 2) {

    enemy->skills[2].active = 1; // Turn Count
    if(enemySkillEffective) *enemySkillEffective = &enemy->skills[2]; 
    enemy->skills[1].active++; // Overheat count
    if (enemy->skills[1].active > 5) enemy->skills[1].active = 5;
    enemy->MinSpeed += 10; // เพิ่มที่ฐาน
    enemy->MaxSpeed += 10; // เพิ่มที่เพดาน

  }

  // Lei heng – skill 3 every 5 turns
  else if (isId(enemy->ID, "Lei heng") == 0 && (enemy->skills[2].active > 0 && enemy->skills[2].active < 5) && enemy->skills[0].active == 2) {

    enemy->skills[2].active++; // Turn Count

  }
  else if (isId(enemy->ID, "Lei heng") == 0 && (enemy->skills[2].active >= 5) && enemy->skills[0].active == 2 && enemy->skills[2].Coins > 0) {

    if(enemySkillEffective) *enemySkillEffective = &enemy->skills[2]; 
    enemy->skills[1].active++; // Overheat count
    if (enemy->skills[1].active > 5) enemy->skills[1].active = 5;
    enemy->MinSpeed += 10; // เพิ่มที่ฐาน
    enemy->MaxSpeed += 10; // เพิ่มที่เพดาน

  }

  // Lei heng – skill 6 using first fight
  if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[5].active == 0 && enemy->skills[0].active == 3) {

    enemy->skills[5].active = 1;
    if(enemySkillEffective) *enemySkillEffective = &enemy->skills[4]; 
    enemy->skills[1].active++; // Overheat count
    if (enemy->skills[1].active > 5) enemy->skills[1].active = 5;
    enemy->MinSpeed += 15; // เพิ่มที่ฐาน
    enemy->MaxSpeed += 15; // เพิ่มที่เพดาน

  }

  // Lei heng – skill 6 every 5 turns
    else if (isId(enemy->ID, "Lei heng") == 0 && (enemy->skills[5].active > 0 && enemy->skills[5].active < 5) && enemy->skills[0].active == 3) {

      enemy->skills[5].active++; // Turn Count

    }
    else if (isId(enemy->ID, "Lei heng") == 0 && (enemy->skills[5].active >= 5) && enemy->skills[0].active == 3 && enemy->skills[4].Coins > 0) {

      if(enemySkillEffective) *enemySkillEffective = &enemy->skills[4]; 
      enemy->skills[1].active++; // Overheat count
      if (enemy->skills[1].active > 5) enemy->skills[1].active = 5;
      enemy->MinSpeed += 15; // เพิ่มที่ฐาน
      enemy->MaxSpeed += 15; // เพิ่มที่เพดาน

    }

  //-------------------------------------------------------------

  // ------------------------- Sancho --------------------------

  // Sancho – skill 13 using first fight
  if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    enemy->HP <= enemy->MAX_HP * 0.6 && enemy->skills[12].active && !isStaggered(enemy)) {

    enemy->skills[12].active = 0;
  if(enemySkillEffective) *enemySkillEffective = &enemy->skills[12];

  }
  // Sancho – skill 14 using first fight
  if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    enemy->HP <= enemy->MAX_HP * 0.3 && enemy->skills[13].active && enemy->skills[12].active == 0 && !isStaggered(enemy)) {

    enemy->skills[13].active = 0;
  if(enemySkillEffective) *enemySkillEffective = &enemy->skills[13];

  sleep(1);
  }

  //-------------------------------------------------------------------------

  // ------------------------- King in Binds --------------------------

  // King in Binds – skill 6 using first fight
  if (isId(enemy->ID, "King in Binds") == 0 && enemy->skills[5].active == 0 && enemy->skills[0].active == 1) {

    if (enemy->Stagger > 0) {
        enemy->Stagger = 0;

        printf("\n%s recovers from 'Stagger'\n",
          enemy->name);

        sleep(1);
      }

    enemy->skills[5].active = 1;
    if(enemySkillEffective) *enemySkillEffective = &enemy->skills[5]; 

  }

  // King in Binds – skill 6 every 4 turns
    else if (isId(enemy->ID, "King in Binds") == 0 && (enemy->skills[5].active > 0 && enemy->skills[5].active < 4) && enemy->skills[0].active == 1) {

      enemy->skills[5].active++; // Turn Count

    }
    else if (isId(enemy->ID, "King in Binds") == 0 && (enemy->skills[5].active >= 4) && enemy->skills[0].active == 1 && !isStaggered(enemy)) {

      if(enemySkillEffective) *enemySkillEffective = &enemy->skills[5]; 
      enemy->skills[5].active = 1;

    }

  //-------------------------------------------------------------------------

  // Jia Qiu enemy heal
  if (isId(player->ID, "Jia Qiu") == 0 && (player->skills[15].active > 0) && enemy->HP <= 0) {

    if (isId(enemy->ID, "Hong lu:The Lord of Hongyuan") == 0) {

      player->skills[15].active -= 1;

    enemy->HP = enemy->MAX_HP;
      enemy->FinalPowerUp[1] += 1;
      printf("\n%s's Uncompromising Imposition activated! Heal up to max HP and gain 1 Final Power, lose 1 stack(%d)", enemy->name, player->skills[15].active);
      sleep(1);
  } else {

      player->skills[15].active -= 1;

      enemy->HP = enemy->MAX_HP;
      printf("\n%s's Dialogues activated! Heal up to max HP, lose 1 stack(%d)", enemy->name, player->skills[15].active);
      sleep(1);
  }

  }















  applySanityDebuff(player);
  applySanityDebuff(enemy);

}




















// ----------------------------------------------------------------
// handleBeforeFight — After showing player hp and sanity
// ----------------------------------------------------------------
void handleBeforeFight(Character *player, Character *enemy, SkillStats **enemySkillEffective, int playerSkill1, int playerSkill2, int enemySkill1, int enemySkill2) {

  if (!SpeedState) {
    SpeedState = 1;
  calculateSpeed(player);
  calculateSpeed(enemy);
  }

    // ------------------ Before fight -----------------------

  // The Middle Nursefather - Matthias - at 10 The Middle - Grudge
  if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0 && enemy->skills[0].active == 10 && enemy->skills[9].active != 3 && !isStaggered(enemy)) {
    printf("\n%s: *sigh*  \"...Not even gonna write that down. That's a summary execution!\"\n", enemy->name);
  }

  // The One Who Grips Faust - Purify ready
  if (isId(player->ID, "The One Who Grips Faust") == 0 && player->skills[2].active >= 3 && (&player->skills[playerSkill1] == &player->skills[2] || &player->skills[playerSkill2] == &player->skills[2])) {

          printf("\n%s: \"Higher... Still higher! Let me advance... toward a purer body... Huhu!\"\n", player->name);

  }

    // --------------------------- Sukuna:King of Curse --------------------------------


    // Sukuna:King of Curse Chanting
    if (isId(enemy->ID, "Sukuna:King of Curse") == 0 && enemy->skills[4].active < 3 && TurnCount % 3 == 0) {

       if(enemySkillEffective) *enemySkillEffective = &enemy->skills[4];

    }

    // Sukuna:King of Curse World Cutting Slash
    else if (isId(enemy->ID, "Sukuna:King of Curse") == 0 && enemy->skills[4].active == 3 && !isStaggered(enemy)) {

     if(enemySkillEffective) *enemySkillEffective = &enemy->skills[7];

        enemy->skills[4].active = 0;

      printf("\n%s: \"No more playing around, brat.\"\n",
        enemy->name);

      sleep(1);

    }

    // ---------------------------------------------------------------------


     // ------------------ Jia Qiu -----------------------

    // Jia Qiu - Last attack at 10% HP
      if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP < enemy->MAX_HP * 0.2 && enemy->Passive == 6) {

        printf("\n%s: \"I expect you to fight to your deaths in this crucial struggle, reversible as they may be.\"\n",
          enemy->name);

          if(enemySkillEffective) *enemySkillEffective = &enemy->skills[15];

          enemy->Passive = 7;

      }

    // Jia Qiu - Taunt S14
    if (isId(enemy->ID, "Jia Qiu") == 0 && *enemySkillEffective == &enemy->skills[14]) {

      printf("\n%s: \"It must lie there still, shrouded it may be.\"\n",
        enemy->name);

    }

    // Jia Qiu - Last attack at 30% HP
    if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP < enemy->MAX_HP * 0.4 && enemy->Passive == 4) {

      printf("\n%s: \"Do not fear the futility. There will be time for that once you have spoken your mind.\"\n",
        enemy->name);

        if(enemySkillEffective) *enemySkillEffective = &enemy->skills[16];

        enemy->Passive = 5;

    }

    // Jia Qiu - Last attack at 85% HP
      if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP < enemy->MAX_HP * 0.85 && enemy->Passive == 2 && enemy->skills[5].active == 0 && !isStaggered(enemy)) {

          if(enemySkillEffective) *enemySkillEffective = &enemy->skills[5];

          enemy->skills[5].active = 1;
      }

    // -------------------------------------------------------------

  // The House of Spiders: The Index Nursefather Yi Sang – Before fight with Furioso-Replica with Sizzling Wound
  if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 &&
      (&player->skills[playerSkill1] == &player->skills[3] ||
       (&player->skills[playerSkill2] == &player->skills[3])) && player->skills[3].active == 2) {

    player->BasePowerUp[0] += 1;
    player->DamageUp[0] += 30;
    player->ClashPowerUp[0] += 2;

    printf("\n%s gains 'Indulgence in Prescripts'\n",
      player->name);

    sleep(1);

      printf("\n%s: *beep* \"The will of Hermes.\"\n",
        player->name);

    sleep(1);

  } else if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && (&player->skills[playerSkill1] == &player->skills[3] ||
       (&player->skills[playerSkill2] == &player->skills[3]))) { // The House of Spiders: The Index Nursefather Yi Sang – Before fight with Furioso-Replica without Sizzling Wound

        printf("\n%s: \"I shall replicate a furious heart.\"\n",
          player->name);

      sleep(1);

    }

    // Roland - Skill 9 Unbreakable coins reset
  if (isId(player->ID, "Fixer grade 9?") == 0 && player->skills[8].Unbreakable > 0) {

    player->skills[8].Unbreakable = 0;

  }

    //Roland Furioso
    if (isId(player->ID, "Fixer grade 9?") == 0) {

      int allInactive = 1; // assume all inactive

      // check skills 0–8
      for (int i = 0; i <= 8; i++) {
          if (player->skills[i].Copies > 0) {
              allInactive = 0; // found one active
              break;
          }
      }

      if (allInactive == 1) {

        printf("\n%s: \"I just wanted to stop the cycle. To make it end somewhere.\"\n",
        player->name);

        for (int i = 0; i <= 8; i++) {
          player->skills[i].Copies = 1;
        }

        sleep(1);
      }

    } 

    // Lei heng – skill 3 and skill 6 Before fight
      if (isId(enemy->ID, "Lei heng") == 0 &&
          (*enemySkillEffective == &enemy->skills[2] ||
           (*enemySkillEffective == &enemy->skills[4]))) {

        printf("\n%s: \"I'maboutta drop somethin' big on y'all! Don't let it kill "
               "y'all now and spoil the fun!\"\n",
          enemy->name);

        sleep(1);
      }

      // Sancho – Before fight
      if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
          (*enemySkillEffective == &enemy->skills[12] ||
           (*enemySkillEffective == &enemy->skills[13]))) {

        if (isId(enemy->ID, "Don Quixote:The Manager of La Manchaland") == 0) {

          printf("\n%s: \"My name is Sancho!\"\n",
            enemy->name);

          printf("\n%s: \"...\"\n",
             player->name);

          sleep(1);

          printf("\n%s: \"And I, Sancho, declare upon my honor; this lance shall end that festering, slothful dream!\"\n", enemy->name);

          printf("\n%s: \"You... Are like... Him... what's a juvenile dream!\"\n",
             player->name);

        } else {
          printf("\n%s: \"That's just attachment; my dream has already ended\"\n", enemy->name);
        }

        sleep(1);
      }

    // Don Quixote:The Manager of La Manchaland – Before fight
    if (isId(player->ID, "Don Quixote:The Manager of La Manchaland") == 0 &&
        (&player->skills[playerSkill1] == &player->skills[2] ||
         (&player->skills[playerSkill2] == &player->skills[2])) && player->Passive >= 15) {

        printf("\n%s: \"When it comes to making weapons, I had surpassed my father.\"\n",
          player->name);

      sleep(1);

    } else if (isId(player->ID, "Don Quixote:The Manager of La Manchaland") == 0 && player->Passive >= 15) {

      printf("\n%s: \"With your weapons... I will lead this bloody battle to victory.\"\n",
        player->name);

      sleep(1);

    }

    // -------------------------------------------------

}











// ----------------------------------------------------------------
  // handleTurnEnd — shared turn-end logic for all battle loops
  // ----------------------------------------------------------------
  void handleTurnEnd(Character *player, Character *enemy, SkillStats *playerSkillUsed, SkillStats *enemySkillUsed) {

    // ----------------------------- Turn End ------------------------------

    SpeedState = 0;


// -------------- Special Status Effects --------------

    // Tremor
    if (player->Tremor[0] > 0 || player->Tremor[1] > 0) {

            player->Tremor[1] -= 1;
      if (player->Tremor[1] <= 0) player->Tremor[1] = 0;

      printf("\n%s loses 1 Tremor Count (Stack %d Count %d)\n", player->name, player->Tremor[0], player->Tremor[1]);

      if (player->Tremor[1] <= 0) {
          player->Tremor[0] = 0;
          player->TremorType = "Normal"; 
      }

      sleep(1);
    }

    // Charge
    if (player->Charge[0] > 0 || player->Charge[1] > 0) {

            player->Charge[1] -= 1;
      if (player->Charge[1] <= 0) player->Charge[1] = 0;

      // The House of Spiders: The Ring Nursefather Hong Lu - Unqiue Charge
        if (isId(player->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0) {
          printf("\n%s loses 1 Corpus Ingredient Count (%d)\n", player->name, player->Charge[1]);
        }

      else {
      printf("\n%s loses 1 Charge Count (%d)\n", player->name, player->Charge[1]);
      }


      if (player->Charge[1] <= 0) player->Charge[0] = 0;

      sleep(1);
    }


      // Burn
        if ((player->Burn[0] > 0 || player->Burn[1] > 0) && player->HP > 0) {

          int damage = player->Burn[0] > 0 ? player->Burn[0] : 1;

          if (player->HP < 0) player->HP = 0;

            player->Burn[1]--; // Count Burn min 1
          if (player->Burn[1] <= 0) player->Burn[1] = 0;

          printf("\n%s takes %d Burn damage (Count %d)\n", player->name, damage, player->Burn[1]);

          sleep(1);

          applyDamage(NULL, player, damage, 0, "Burn");

          if (player->Burn[1] <= 0) player->Burn[0] = 0;

        }










    // Heathcliff:Wild Hunt - Impending Ruin
    if (player->skills[4].name != NULL && strcmp(player->skills[4].name, "Impending Ruin") == 0 && player->skills[4].active > 0) {
        player->skills[4].active--;
        printf("\n%s loses 1 Impending Ruin (%d)\n", enemy->name, player->skills[4].active);
    }


    // ----------------------- Heishou Pack - You Branch Adept Heathcliff ----------------

    // Heishou Pack - You Branch Adept Heathcliff - Bloodflame
    if (isId(player->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && player->skills[2].active > 0) {

      player->skills[2].active--;

       printf("\n%s loses 1 Bloodflame [血炎] (%d)\n", player->name, player->skills[2].active);
    }

    // -------------------------------------------------------------

    // Binah - Fairy
    if (isId(player->ID, "Binah") == 0 && player->skills[0].active > 0) {

      if (!player->Passive) {

      enemy->HP -= player->skills[0].active;
      if (enemy->HP < 0) enemy->HP = 0;

      printf("\n%s take true damage equal to Fairy on self (%d)\n", enemy->name, player->skills[0].active);

      sleep(1);

      } else {

        int fairydamage = 0.5*((enemy->MAX_HP/100) * player->skills[0].active);

        enemy->HP -= fairydamage;
        if (enemy->HP < 0) enemy->HP = 0;

        printf("\n%s take (0.5 x Fairy on self (%d))%% Max HP true damage (%d)\n", enemy->name, player->skills[0].active, fairydamage);

        sleep(1);

      }

      player->skills[0].active = player->skills[0].active/2;

       printf("\nhalve the Fairy stack on %s (%d)\n", enemy->name, player->skills[0].active);

      sleep(1);
    }

 // The One Who Grips Faust - Nail and Gaze
    if (isId(player->ID, "The One Who Grips Faust") == 0) {

      if (player->skills[2].active > 0) {
        // Nail Halving (ตะปูลดลงครึ่งหนึ่ง)
        player->skills[2].active /= 2;
        printf("\n%s's Nails are halved (%d)\n", enemy->name, player->skills[2].active);
      }

      // Clear Fanatic
      if (player->Passive > 0) {
          player->Passive = 0;
        printf("\n%s loses 'Fanatic'\n", player->name);
      } 

      if (player->skills[6].active > 0) { // Next turn Fanatic
            player->Passive += player->skills[6].active;
        printf("\n%s gains %d Fanatic this turn\n", player->name, player->skills[6].active);
        player->skills[6].active = 0;
        }

        // Clear gaze
      if (player->skills[4].active == 1) {
        player->skills[4].active = 0;
        printf("\n%s loses 'Gaze'\n", enemy->name);
      } 

      if (player->skills[7].active > 0) { // Next turn Gaze
          player->skills[4].active = 1;
        printf("\n%s gains 'Gaze' this turn\n", enemy->name);
        player->skills[7].active = 0;
        }

    }






    // Overheat
    if (isId(player->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0) {

      if (player->skills[11].active == 1 && player->Passive == 0 && player->skills[14].active == 0) { // Overheat

           player->skills[10].active = 10;
           player->skills[14].active = 1; // Once per encounter

       printf("\n%s uses 'Reload' (Lose all currently owned Ammo, and reload back to full) (Once per Encounter)\n", player->name);

        sleep(1);

       if (player->skills[13].active == 0) {
           player->skills[13].active = 1; // Shin
           player->MinSpeed++;
           player->MaxSpeed++;

      printf("\n%s gains 'Shin (心) - Disgrace' next turn\n", player->name);

       sleep(1);
       }

    } 
    }

    // The House of Spiders: The Thumb Nursefather Rodion - Turn End
    if (isId(player->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0) {

        player->defenseSkill[1].active = 0; // Reset Evade
        player->skills[6].active = 0; // Reset Eye count per turn
      player->defenseSkill[0].active = 0; // Taunt Reset
      player->defenseSkill[3].active = 0; // Evaded 2 time per turn rest
      player->defenseSkill[4].active = 0; // Evaded once time per turn rest

      if (player->Stagger > 0 && player->skills[3].active == 0) {
      player->Stagger = 0;
          player->skills[3].active = 1; // ใช้ได้ครั้งเดียวต่อ Encounter
          printf("\n%s recovers from Stagger!\n", player->name);

        sleep(1);

        printf("\n%s: \"Like hell I'm kickin' the bucket in this fucking dump.\"\n", player->name);

        sleep(1);

        if (player->skills[13].active == 0) {
             player->skills[13].active = 1; // Shin
            player->MinSpeed++;
            player->MaxSpeed++;

          printf("\n%s gains 'Shin (心) - Disgrace' next turn\n", player->name);

           sleep(1);
           }
      }
      }

    // The House of Spiders: The Thumb Nursefather Rodion - Turn End Defense 1
    if (isId(player->ID, "The House of Spiders: The Thumb Nursefather Rodion") == 0 && playerSkillUsed == &player->defenseSkill[0] && player->skills[10].active < 5) {

          player->skills[10].active = 5; // Acceleration Round (กระสุน 5/10);

         printf("\n%s at less than 5 Acceleration Round, reload Acceleration Round to 5\n", player->name);

          sleep(1);
      
      }

    // Meursault:Blade Lineage Mentor - End Shadow-vested Bladesinger [着影揮刀]
    if (isId(player->ID, "Meursault:Blade Lineage Mentor") == 0 && player->defenseSkill[2].active == 1) {

        player->defenseSkill[2].active = 2;

    }
    
    // Evil Bandit Lose base
    if (isId(player->ID, "Evil Bandit") == 0) {


      for (int i = 0; i < player->numDefenseSkills; i++) {
        player->defenseSkill[i].BasePower -= 1;
      }

      printf("\n%s's all skills lose 1 Base Power\n",
        player->name);
      
          sleep(1);
        }

    // Meursault:The Thumb Shin buffs (temporary, print once)
      if (isId(player->ID, "Meursault:The Thumb") == 0 &&
           (player->Passive <= 0 || player->HP <= player->MAX_HP*0.65 || player->Stagger > 0 || playerSkillUsed == &player->defenseSkill[0]) && !player->skills[3].active) {

        if (player->Stagger > 0) {
            player->Stagger = 0;

            printf("\n%s recovers from 'Stagger'\n",
              player->name);

            sleep(1);
          }

        int amount = ((int)(8 * enemy->MAX_HP)) / 847;
        if (amount < 8) amount = 8;

        if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 || isId(enemy->ID, "Don Quixote") == 0) amount += 2; // pity for boss
        if (isId(enemy->ID, "Sukuna:King of Curse") == 0) amount += 3; // pity for boss
        if (isId(enemy->ID, "The Middle Nursefather - Matthias") == 0) amount += 5; // pity for boss

          player->skills[3].active = 1;

            player->Passive = amount;
        player->defenseSkill[3].active += amount; // Gain value 2

          printf("\n%s activates 'Tiantui Star [天退星]' and reload %d Savage Tigermark Round\n",
            player->name, amount);

          sleep(1);

          printf("\n%s: \"I see that you are worth the cost of my ammunition.\"\n", player->name);

        }

// ------------------------------ The House of Spiders: The Ring Nursefather Hong Lu ------------------------------

    // Passive
    if (isId(player->ID, "The House of Spiders: The Ring Nursefather Hong Lu") == 0) {
      // 1. Turn End: Reset Unbreak
      if (player->skills[5].active == 1) {
        player->skills[5].active = 0;
      if (player->skills[2].Unbreakable > 0) player->skills[2].Unbreakable = 0; // Reset Unbreak
      if (player->defenseSkill[0].Unbreakable > 0) player->defenseSkill[0].Unbreakable = 0; // Reset Unbreak
      if (player->skills[3].Unbreakable > 0) player->skills[3].Unbreakable = 0; // Reset Unbreak
      }

        // 2. Turn End: Lose 2 Count
      if (player->Passive > 0) {
        if (player->Passive > 0) player->Passive -= 2;
      if (player->Passive < 0) player->Passive = 0;

      printf("\n%s loses 2 Viewing the Tableau (%d)\n", player->name, player->Passive);

        sleep(1);

      }

        // 4. Transcend the Corpus: ฟื้นจาก Stagger
        if (player->Stagger > 0 && player->Passive > 0 && player->skills[3].active == 0) {
            player->Stagger = 0;
            player->skills[3].active = 1; // ใช้ได้ครั้งเดียวต่อ Encounter
            printf("\n%s recovers from Stagger!\n", player->name);

          sleep(1);

          printf("\n%s: \"I cannot let the audience witness such a degrading sight.\"\n", player->name);

          sleep(1);
        }

        player->skills[2].active = 0; // รีเซ็ต Critique ต่อเทิร์น
    }

    // ------------------------------------------------------------------------------------------

    // Muga Ryōshū - Sever the thread delete
    if (isId(player->ID, "Muga Ryōshū") == 0) {

      // ระดับความพังพื้นฐานคือ 10% หรือตามค่า Muga (ถ้ามี)
      int intensity = player->skills[0].active/10;

      if (intensity > 0) {

          for (int i = 0; i < enemy->numSkills; i++) {
              // เงื่อนไข: ไม่ใช่ท่าไม้ตาย (index 5), สกิลมีชื่ออยู่จริง, และสกิลยังไม่พัง (Copies >= 0)
              if (enemy->skills[i].name != NULL && enemy->skills[i].Copies >= 0) {
                enemy->skills[i].name = glitchText(enemy->skills[i].name, intensity);
              }
          }
                  enemy->name = glitchText(enemy->name, intensity/2); // ชื่อ

        }
    }

// The Middle Little Brother Sinclair - Counter Reset
    if (isId(player->ID, "The Middle Little Brother Sinclair") == 0) {
        // คืนค่าให้ทุกสกิลกลับเป็นท่าโจมตีปกติที่ Clash ได้
        for (int i = 0; i < player->numSkills; i++) {
            player->skills[i].skillType = 0; // 0 = Atk
            player->skills[i].Clashable = 1; // 1 = Clashable
        }
      player->skills[2].active = 0; // รีเซ็ต Flag ทิ้งทุกจบเทิร์น
      player->skills[3].active = 0; // รีเซ็ต Once per turn อื่นๆ
    }

// The Middle Nursefather - Matthias - Skill 7 8 9
if (isId(player->ID, "The Middle Nursefather - Matthias") == 0 && (playerSkillUsed == &player->skills[7] || playerSkillUsed == &player->skills[8] || playerSkillUsed == &player->skills[9])) {

  player->DamageUp[1] += 30;

    printf("\n%s gains +30%% damage next turn\n", player->name);

    sleep(1);

}

// The Middle Nursefather - Matthias - Change Phase
    if (isId(player->ID, "The Middle Nursefather - Matthias") == 0) {

      player->skills[3].active = player->skills[11].active; // Save Damage taken last turn
      player->skills[11].active = 0; // Reset Damage taken

      float hpRate = player->HP / player->MAX_HP;
      int *thresholdState = &player->skills[7].active; // ใช้ดัชนี 7 เก็บ Bitmask ของ Threshold

      if (hpRate <= 0.40 && !(*thresholdState & 4)) {
          player->skills[0].active = 10;
          *thresholdState |= 4;
          printf("\n%s gains Max 'The Middle - Grudge' Stack\n", player->name);

        sleep(1);
      } else if (hpRate <= 0.60 && !(*thresholdState & 2)) {
          player->skills[0].active = 10;
          *thresholdState |= 2;
        printf("\n%s gains Max 'The Middle - Grudge' Stack\n", player->name);

        sleep(1);
      } else if (hpRate <= 0.90 && !(*thresholdState & 1)) {
          player->skills[0].active = 10;
          *thresholdState |= 1;
        printf("\n%s gains Max 'The Middle - Grudge' Stack\n", player->name);

        sleep(1);
      }
    }

    if (isId(player->ID, "The Middle Nursefather - Matthias") == 0) {
        int hitsThisTurn = player->skills[6].active;
        int tattooNextTurn = 0;

        if (hitsThisTurn > 3) {
            tattooNextTurn = hitsThisTurn - 3; // เกิน 3 มาเท่าไหร่ ได้รอยสักเท่านั้น
            if (tattooNextTurn > 2) tattooNextTurn = 2; // สูงสุดไม่เกิน 2 Stack
        }

        // ฝากค่ารอยสักไว้ใน skills[2].active เพื่อใช้ในเทิร์นหน้า
        player->skills[2].active = tattooNextTurn;

      if (player->skills[2].active > 0) {
        printf("\n%s gains %d 'The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]' next turn\n", player->name, tattooNextTurn);
      }

        // Reset ตัวนับเหรียญของเทิร์นนี้ทิ้ง
        player->skills[6].active = 0; 
    }

    // Sukuna:King of Curse Domain expansion
    if (isId(enemy->ID, "Sukuna:King of Curse") == 0 && enemy->skills[8].active > 0) {

      int dealvalue = player->MAX_HP*0.05;

      applyDamage(enemy, player, dealvalue, 0, NULL);

      printf("\n%s takes %d damage\n", player->name, dealvalue);

      sleep(1);

      enemy->skills[3].active += 5;

      printf("\n%s gains 5 'Binding Vow - Open' (%d)\n", enemy->name, enemy->skills[3].active);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Turn End passive
    if (isId(player->ID, "Don Quixote:The Manager of La Manchaland") == 0) {

     int gainmissing = ((player->MAX_HP - player->HP) / (player->MAX_HP * 0.15));
      if (gainmissing > 3) gainmissing = 3;

      if (gainmissing > 0) {

        int gain = gainmissing * 10;

        player->DamageUp[1] += gain;

      printf("\n%s gains 10%% Damage Up next turn for every 15%% missing HP at Turn End (%d%% - Max 30%%)\n", player->name, gain);

      }

      }


    // Yi Sang:Fell Bullet - Pity buff
    if (isId(player->ID, "Yi Sang:Fell Bullet") == 0) {

      if (player->Passive >= 7) {
        player->skills[2].Copies = 3;
      } else {
        player->skills[2].Copies = 1;
      }

      }


// The House of Spiders: The Index Nursefather Yi Sang Passive
    if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {

      // [Unlock Stage Sanity Heal]
      // stage 1 = 5, stage 2 = 10, stage 3 = 15
      if (player->Passive == 1) {
          updateSanity(player, 5);
          printf("\n%s has 'Unlock - I' Combat End: Heal 5 Sanity (%d)\n", player->name, player->Sanity);
      } else if (player->Passive == 2) {
          updateSanity(player, 10);
          printf("\n%s has 'Unlock - II' Combat End: Heal 10 Sanity (%d)\n", player->name, player->Sanity);
      } else if (player->Passive == 3) {
          updateSanity(player, 15);
          printf("\n%s has 'Unlock - III' Combat End: Heal 15 Sanity (%d)\n", player->name, player->Sanity);
      }

       player->skills[8].active = 0; // ปลดล็อกการรับ Hermes สำหรับเทิร์นถัดไป

      player->skills[13].active = 0; // Reset โควต้าการรับ Hermes สำหรับเทิร์นหน้า

      // 1. เช็คหน้ากากแตก (Nursefather Passive)
      if ((player->Stagger > 0 || player->HP <= player->MAX_HP * 0.65) && player->skills[3].active == 1) {

        if (enemy->Stagger > 0) {
          enemy->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            enemy->name);

          sleep(1);
        }

        player->skills[3].active = 2;

          printf("\n%s at 65%% or less HP, or 'Stagger', converts 'Wound-casing Mask' to 'Sizzling Wound' and recover from 'Stagger'\n", player->name);

        sleep(1);

        printf("\n%s: \"Haha... I wonder if my darling daughter would remember this wound...\"\n", player->name);

        sleep(1);
      }

// 2. สรุป Prescript (Nursefather Yi Sang)
if (player->skills[5].active == 1) { // ถ้าทำตาม Prescript สำเร็จ

  printf("\n%s's [Prescript: [Device]]: _Clear._\n", player->name);

  sleep(1);

    // --- ตรรกะการฮีล Sanity (SP) ---
    if (player->skills[1].active >= 9) {
        updateSanity(player, 8);
        printf("\n%s executed Prescript with Full Indulgence! Heal 8 Sanity (%d)\n", player->name, player->Sanity);
    } 
    else if (player->Passive == 2) {
        updateSanity(player, 4);
        printf("\n%s executed Prescript at 'Unlock - II'. Heal 4 Sanity (%d)\n", player->name, player->Sanity);
    } 
    else {
        updateSanity(player, 8);
        printf("\n%s executed Prescript. Heal 8 Sanity (%d)\n", player->name, player->Sanity);
    }

    // --- ตรรกะการรับ Grace of the Prescript ---
    // เงื่อนไข: จะได้รับ Grace ก็ต่อเมื่อ (ไม่ใช่ Stage 2) หรือ (เป็น Stage 2 แต่ต้องเป็น Full Indulgence)
    int isFullIndulgence = (player->skills[1].active >= 9);

    if (player->Passive != 2 || isFullIndulgence) {
        player->skills[0].active += 3;

        // คำนวณเพดานของ Grace (Max Grace)
        int maxGrace = 0;
        if (player->Passive == 0) maxGrace = 3;
        else if (player->Passive == 1) maxGrace = 6;
        else maxGrace = 9; // สำหรับ Stage 2 ที่เป็น Full Indulgence หรือ Stage 3

        if (player->skills[0].active > maxGrace) player->skills[0].active = maxGrace;

        printf("%s gains 3 'Grace of the Prescript' (%d)\n", player->name, player->skills[0].active);
    }

} 
else if (player->skills[5].active == 0 && player->Passive < 3) {
    // กรณีล้มเหลว (ติด Karma)
    player->skills[2].active += 5;
    if (player->skills[2].active > 100) player->skills[2].active = 100;
    printf("\n%s gains 5 'Karmic Consequence' (%d - Max 100)\n", player->name, player->skills[2].active);
    sleep(1);
}

      // 3. ปรับ Stage ตาม Grace
      int grace = player->skills[0].active;
      if (grace >= 9 && player->Passive < 3) {
        player->Passive = 3; 

        printf("\n%s gains 'Unlock - III'\n", player->name); 

        sleep(1);

        printf("\n%s: \"Severed and torn until even the form is undone. (支離滅裂)\"\n", player->name);

        sleep(1);
      }
      else if (grace >= 6 && player->Passive < 2) {
        player->Passive = 2; 

        printf("\n%s gains 'Unlock - II'\n", player->name);

        sleep(1);

        printf("\n%s: \"Like the Naraka of Avīci and Raurava. (阿鼻叫喚)\"\n", player->name);

        sleep(1);
      }
      else if (grace >= 3 && player->Passive < 1) {
        player->Passive = 1; 

        printf("\n%s gains 'Unlock - I'\n", player->name);

        sleep(1);

        printf("\n%s: \"Wading through a dream, the self nowhere to be found. (無我夢中)\"\n", player->name);

        sleep(1);
      }

    }

    // The House of Spiders: The Index Nursefather Yi Sang Evade
    if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && player->Passive < 2 && playerSkillUsed == &player->defenseSkill[0]) {

      player->Passive = 2; 

      printf("\n%s gains 'Unlock - II'\n", player->name);

      sleep(1);

      int gain = 6 - player->skills[0].active;

      player->skills[0].active += gain;

      printf("%s raises the 'Grace of the Prescript' Stack to 6\n", player->name);

      sleep(1);

      player->skills[2].active += 5 * gain;
      if (player->skills[2].active > 100) player->skills[2].active = 100;
      printf("%s gains 5 'Karmic Consequence' (%d) for every 'Grace of the Prescript' gained via the effect above (%d - Max 100)\n", player->name, 5 * gain, player->skills[2].active);

      sleep(1);

      printf("\n%s: \"Like the Naraka of Avīci and Raurava. (阿鼻叫喚)\"\n", player->name);

    }


    // Erlking Heathcliff Phase 2
    if (isId(enemy->ID, "Erlking Heathcliff") == 0 && (enemy->HP <= enemy->MAX_HP * 0.7 || TurnCount >= 6) &&
        enemy->Passive == 0) {

      enemy->Passive = 1;

      enemy->sanityLossBase = 10;

      enemy->skills[1] = enemy->skills[5];
      enemy->skills[2] = enemy->skills[4];
      enemy->skills[1].Copies = -1;
      enemy->skills[2].Copies = -1;

      enemy->skills[3].Copies = 4;
      enemy->skills[4].Copies = 3;
      enemy->skills[5].Copies = 3;
      enemy->skills[6].Copies = 2;
      enemy->skills[7].Copies = 2;

      printf("\n%s: \"May you wake in torment, my dear Catherine.\"\n", enemy->name);

      sleep(1);
    }

    // Wild hunt – Gain
    if (isId(player->ID, "Heathcliff:Wild Hunt") == 0 && playerSkillUsed == &player->defenseSkill[0] && player->skills[0].active <= 0) {

      player->skills[0].active++;
      if (player->skills[0].active > 3) player->skills[0].active = 3;

      player->MinSpeed += 1;
      player->MaxSpeed += 1;

      printf("\n%s gains 1 'Dullahan' (%d - Max 3)\n",
             player->name, player->skills[0].active);

      sleep(1);
    } else if // Wild hunt – Lose
       (isId(player->ID, "Heathcliff:Wild Hunt") == 0 && (playerSkillUsed == &player->defenseSkill[0] || playerSkillUsed == &player->skills[3]) && player->skills[0].active > 0) {

         player->skills[0].active = 0;
         player->skills[2].Copies = 1;

         player->MinSpeed -= 1;
         player->MaxSpeed -= 1;

        printf("\n%s loses all 'Dullahan'\n",
               player->name);

        sleep(1);
      }

     // Heathcliff:Wild Hunt - Dullahan
      if (isId(player->ID, "Heathcliff:Wild Hunt") == 0 && player->skills[0].active > 0) {

        player->skills[2].active = 0; // Reset Counter

        if (playerSkillUsed != &player->defenseSkill[0]) {

        player->skills[0].active += 1; // Dullahan
        if (player->skills[0].active > 3) player->skills[0].active = 3;

        printf("\n%s gains 1 Dullahan (%d - Max 3)\n",
          player->name, player->skills[0].active);

        }

        // A bit buff for unlucky player
        if (player->skills[0].active >= 3 || player->Sanity < 0) player->skills[2].Copies = 3; // A bit buff for unlucky player
        if (player->skills[0].active >= 3 && player->Sanity < 0) player->skills[2].Copies = 6;

        if (player->Sanity <= -25) {

          player->skills[0].active = 0;
          player->skills[2].Copies = 1;

            player->MinSpeed -= 1;
            player->MaxSpeed -= 1;

          printf("\n%s's Sanity at -25 or less, loses all 'Dullahan'\n", player->name);

        } else {

        updateSanity(player, -(5));

        printf("\n%s loses 5 Sanity (%d)\n",
          player->name, player->Sanity);

          if (player->Sanity <= -25) {

            player->skills[0].active = 0;
            player->skills[2].Copies = 1;

            player->MinSpeed -= 1;
            player->MaxSpeed -= 1;

            printf("\n%s's Sanity at -25 or less, loses all 'Dullahan'\n", player->name);

          } else {

        int losevalue = (15 - (player->Passive/2));
          if (losevalue < 10) losevalue = 10;

        updateSanity(player, -(losevalue));

        printf("\n%s loses %d Sanity (%d)\n",
          player->name, losevalue, player->Sanity);

          }

        }

      }

    // -------------------------------- Lei heng --------------------------------

    // Lei heng – Chosen Prey expires at Turn End
    if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[6].active == 1) {
      enemy->skills[6].active = 0;
      updateSanity(player, 10);
      if (player->Sanity > 45) player->Sanity = 45;
      printf("\n%s's 'Prey' expires. %s heals 10 Sanity (%d)\n",
        player->name, player->name, player->Sanity);
      sleep(1);
    }

    // Lei heng – skill 3 and skill 6 turn end
    if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[0].active == 3 && enemy->skills[3].active > 0) {

      if (enemy->MinSpeed > 15) { // เช็คว่ามีความเร็วส่วนเกินไหม (ปกติบอสมีแค่ 2-4)
          enemy->MinSpeed -= 15;
          enemy->MaxSpeed -= 15;
      }

      enemy->skills[5].active = 1;

      printf("\n%s heals Sanity equal to Extreme Strength [底力] consumed (%d)\n", enemy->name, enemy->skills[3].active);
      
      updateSanity(enemy, enemy->skills[3].active);

      sleep(1);

      int gain = enemy->skills[3].active/25;

       enemy->Haste[1] += gain;

       printf("\n%s gains 1 Haste (%d) for every 25 Extreme Strength [底力] consumed (%d) next turn\n", enemy->name, gain, enemy->skills[3].active);

      sleep(1);
      
        enemy->skills[3].active = 0;

    } else if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[0].active == 2 && enemy->skills[3].active > 0) {

      if (enemy->MinSpeed > 10) { // เช็คว่ามีความเร็วส่วนเกินไหม (ปกติบอสมีแค่ 2-4)
          enemy->MinSpeed -= 10;
          enemy->MaxSpeed -= 10;
      }

      enemy->skills[2].active = 1;

      printf("\n%s heals Sanity equal to Inner Strength [底力] consumed (%d)\n", enemy->name, enemy->skills[3].active);
      
      updateSanity(enemy, enemy->skills[3].active);

      sleep(1);

      int gain = enemy->skills[3].active/25;

       enemy->Haste[1] += gain;

       printf("\n%s gains 1 Haste (%d) for every 25 Inner Strength [底力] consumed (%d) next turn\n", enemy->name, gain, enemy->skills[3].active);

      sleep(1);
      
      enemy->skills[3].active = 0;

    } 

    // Lei heng – Gain Savage Tigermark Round every 2 turn 
    if (isId(enemy->ID, "Lei heng") == 0 && (enemy->defenseSkill[2].active <= 0 || enemy->defenseSkill[3].active <= 0) && enemy->skills[4].active == 1) {

      if (enemy->skills[8].active > 2) {

        enemy->defenseSkill[1].active = 0;

        enemy->defenseSkill[2].active += 6;
        enemy->defenseSkill[3].active += 6;
        
        printf("\n%s loses all Tigermark Round; then converts the effect Tigermark Round to Savage Tigermark Round and gain 12 Savage Tigermark Round and reload" 
          "\n Savage Tigermark Round:\n"
          " - Stack (Loaded Ammo) : 6\n"
           " - Count (Remaining Ammo) : 6\n",
           enemy->name);

        sleep(1);

        enemy->ProtectionDown[1] += 20;
        enemy->ClashPowerDown[1] += 2;

        printf("\n%s gains +20%% Take Damage Up next turn and 2 Clash Power Down next turn\n",
           enemy->name);

        sleep(1);

      } else {

        enemy->skills[8].active++;

      }

    }

    // Lei heng – HP < 90%
    if (isId(enemy->ID, "Lei heng") == 0 && (enemy->HP < enemy->MAX_HP * 0.9 || TurnCount >= 3) && enemy->skills[0].active == 0) {

      GainNewPattern(enemy, player);

      enemy->skills[0].active = 1; // Phase 2

      enemy->defenseSkill[1].active = 6; // Tigermark rounds

      printf("\n%s uses 'Reload' (Lose all currently owned Ammo, and reload back to full) (Tigermark Rounds: 6)\n",
         enemy->name);

      sleep(1);

      printf("\n%s: \"Huh... y'all ain't half bad! Keep makin' it worth my bullet fees!\"\n",
             enemy->name);

      enemy->skills[0].name = "Double Slash - Blast [爆]";
      enemy->skills[0].CoinPower += 1;
      enemy->skills[1].name = "Triple Slash - Blast [爆]";
      enemy->skills[1].CoinPower += 1;

      //enemy->skills[2].Copies = 1;
      enemy->skills[3].Copies = 4;
      enemy->defenseSkill[0].Copies = 2;

      sleep(1);
    }

    // Lei heng – HP < 80%
    if (isId(enemy->ID, "Lei heng") == 0 && (enemy->HP < enemy->MAX_HP * 0.8 || TurnCount >= 5) && enemy->skills[0].active == 1) {

      GainNewPattern(enemy, player);

       enemy->skills[0].active = 2; // Phase 3

      printf("\n%s gains 25 Inner Strength [底力]\n", enemy->name);

      enemy->Passive += 25;
      if (enemy->Passive > 25) enemy->Passive = 25;

      sleep(1);

      printf("\n%s: \"That's more like it. Y'all are firin' me up!\"\n",
         enemy->name);

      sleep(1);

    }

     // -------------------------------------------------------------

    // Dawn Office Fixer Sinclair - Volatile Passion
    if (isId(player->ID, "Dawn Office Fixer Sinclair") == 0 && player->skills[3].active) {

        updateSanity(player, -((5 * player->Passive) > 40 ? 40 : (5 * player->Passive)));

      printf("\n%s lose 5 Sanity(%d - Max 40) for every stack (%d) (Sanity %d)\n", player->name, 5 * player->Passive, player->Passive, player->Sanity);

      sleep(1);

    }

    // ------------------------------ Roland -----------------------

    // Fixer grade 9? – heals every 3 turns on 0+ Sanity
    if (isId(enemy->ID, "Fixer grade 9?") == 0 && enemy->Sanity >= 0 && (TurnCount % 3) == 0) {

      float missingHPPercent = ((float)(enemy->MAX_HP - enemy->HP) / enemy->MAX_HP) * 100.0f;
      int gainpermissing = missingHPPercent/20;

      int losevalue = (enemy->Sanity/2 + (5 * (gainpermissing)));

      updateSanity(enemy, -losevalue);

       printf("\n%s loses %d Sanity (%d)\n", enemy->name, losevalue, enemy->Sanity);

      sleep(1);

      int gainvalue = 3 + (missingHPPercent/10);

      enemy->Passive += gainvalue;

      printf("\n%s gains %d Black Silence (%d)\n", enemy->name, gainvalue, enemy->Passive);

      sleep(1);

    } else if (isId(enemy->ID, "Fixer grade 9?") == 0 && enemy->Sanity < 0 && (TurnCount % 3) == 0) {

      float missingHPPercent = ((float)(enemy->MAX_HP - enemy->HP) / enemy->MAX_HP) * 100.0f;
      int gainpermissing = missingHPPercent/30;

      int losevalue = 5 + (5 * gainpermissing);

        updateSanity(enemy, -(losevalue));

      printf("\n%s loses %d Sanity (%d)\n", enemy->name, losevalue, enemy->Sanity);

      sleep(1);

        enemy->DamageUp[1] += enemy->Passive * 2;

      printf("\n%s gains +2%% damage (%d%%) for every Black Silence (%d) next turn\n", enemy->name, enemy->Passive * 2, enemy->Passive);

      sleep(1);

      }

    // Roland - Ultimate
    if (isId(enemy->ID, "Fixer grade 9?") == 0 && enemy->skills[9].active > 0) {

      enemy->skills[9].active -= 1;
      enemy->ClashPowerDown[1] += 2 * enemy->skills[9].active;

      printf("\n%s loses 1 Sorrow (%d)\n", enemy->name, enemy->skills[9].active);

      sleep(1);

    }

    // Roland - Ultimate
    if (isId(enemy->ID, "Fixer grade 9?") ==
            0 &&
      enemySkillUsed == &enemy->skills[9]) {

      printf("\n%s gains 4 Sorrow, gain 2 Clash Power Down for every 1 Stack, lose 1 Stacks at Turn End\n", enemy->name);

      enemy->skills[9].active = 4;
      enemy->ClashPowerDown[1] += 2 * enemy->skills[9].active;

      sleep(1);

    }
    // -------------------------------------------------------------

      //------------------------ Lobotomy E.G.O::Solemn Lament Yi Sang ---------------------------

    // Lobotomy E.G.O::Solemn Lament Yi Sang - Reload
    if (isId(player->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 && (player->Passive <= 0 || player->defenseSkill[0].active == 1)) {

      int Spend = (30 - player->Passive)/2;

      player->defenseSkill[0].active = 0;

      updateSanity(player, -(Spend));

       int ShieldGain = (player->skills[0].active * 2) > 40 ? 40 : (player->skills[0].active * 2);
      int Shield = (ShieldGain/100.0f) * player->MAX_HP;

      player->Shield += Shield; 

      printf("\n%s uses 'Reload', Spends %d Sanity (%d) to gain 20 The Living & The Departed and gain Shield HP equal to (Butterfly on the target x 2)%% of Max HP. (%d%% - Max 40%%) (%d - Shield %.2f)\n", player->name, Spend, player->Sanity, ShieldGain, Shield, player->Shield);

        player->Passive = 20;

      sleep(1);
    }

    // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly
    if (isId(player->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 && player->skills[0].active > 0) {

      player->skills[0].active = 0;

      player->skills[3].active = 0; // Consumed Living & depart Count

      printf("\n%s loses all Butterfly on self\n", enemy->name);

      sleep(1);
    }

  //--------------------------------------------------------------

    // Jia Qiu enemy heal sanity
    if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->skills[15].active > 0) {

      updateSanity(player, 5);

        printf("\n%s heals 5 Sanity (%d)\n", player->name, player->Sanity);
        sleep(1);

    }

    // Jia Qiu enemy heal
    if (isId(player->ID, "Jia Qiu") == 0 && (player->skills[15].active > 0) && enemy->HP <= 0) {

      if (isId(enemy->ID, "Hong lu:The Lord of Hongyuan") == 0) {

        player->skills[15].active -= 1;

      enemy->HP = enemy->MAX_HP;
        enemy->FinalPowerUp[1] += 1;
        printf("\n%s's Uncompromising Imposition activated! Heal up to max HP and gain 1 Final Power, lose 1 stack(%d)", enemy->name, player->skills[15].active);
        sleep(1);
    } else {

        player->skills[15].active -= 1;

        enemy->HP = enemy->MAX_HP;
        printf("\n%s's Dialogues activated! Heal up to max HP, lose 1 stack(%d)\n", enemy->name, player->skills[15].active);
        sleep(1);
    }

    }

    // Jia Qiu Phase 2
    if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP <= enemy->MAX_HP * 0.6 &&
        enemy->Passive == 2 && enemy->skills[15].active == 0) {

      printf("\n%s: \"Reflect upon yourself! Exhume your humanity, your righteousness from deep within!\"\n", enemy->name);

      if (isId(player->ID, "Hong lu:The Lord of Hongyuan") == 0) {

        printf("\n%s: \"Phew... okay, fine. Then... As the Lord of Hongyuan, I hereby grant you audience.\"\n", player->name);

        sleep(1);

         printf("\n%s gains 3 Uncompromising Imposition, Turn End heal 5 Sanity, When HP drop to 0 heal up to max HP and gain 1 Final Power; then lose 1 stack\n", player->name);
      } else {
        sleep(1);

         printf("\n%s gains 3 Dialogues, Turn End heal 5 Sanity, When HP drop to 0 heal up to max HP; then lose 1 stack\n", player->name);
      }

      enemy->skills[15].active = 3;

      enemy->Passive = 3;

      // Disable the old
      for (int i = 0; i < enemy->numSkills; i++) {
        if (enemy->skills[i].Copies > 0) {
          enemy->skills[i] = enemy->skills[3];
          enemy->skills[i].Copies = -1;
        }
      }

      // Set copies for the newly mapped primary skills
       enemy->skills[7].Copies = 3;
      enemy->skills[8].Copies = 3;
      enemy->skills[9].Copies = 3;
       enemy->skills[10].Copies = 2;
       enemy->skills[11].Copies = 2;
       enemy->skills[12].Copies = 2;
       enemy->skills[13].Copies = 2;
       enemy->skills[14].Copies = 2;
       enemy->skills[15].Copies = -1;
       enemy->skills[16].Copies = -1;
    }

      // ------------------------------ Cap HP --------------------------

      // Don Quixote Phase 2
      if (isId(player->ID, "Don Quixote") == 0 && player->HP <= 0 &&
          player->Passive == 0) {

        printf("\n%s: \"If that's what you really yearn for...\"\n", player->name);

        sleep(1);

        printf("\n%s tranforms to 'Sancho:The Second Kindred of Don Quixote'\n",
               player->name);

        player->MAX_HP = 723;
        player->HP = 723;
        player->name = "Sancho:The Second Kindred of Don Quixote";
        player->ID = "Sancho:The Second Kindred of Don Quixote";
        player->Sanity = 0;
        player->sanityGainBase = 6;
        player->sanityLossBase = 4;
        player->immuneToPanicSkip = 1;
        player->Passive = 1;

        // Disable the old S0, S1, S2
        player->skills[0] = player->skills[3];
         player->skills[1] = player->skills[3];
         player->skills[2] = player->skills[3];
        player->skills[0].Copies = -1; // in pick skill function copies -1 will auto delete skill from lastused 
        player->skills[1].Copies = -1;
        player->skills[2].Copies = -1;
        player->defenseSkill[0].Copies = -1;

        // Set copies for the newly mapped primary skills
        player->skills[3].Copies = 4;
        player->skills[4].Copies = 4;
        player->skills[5].Copies = 4;

        player->skills[6].Copies = 3;
        player->skills[7].Copies = 3;
        player->skills[8].Copies = 3;
        player->skills[9].Copies = 3;
        player->skills[10].Copies = 3;
        player->skills[11].Copies = 3;

        sleep(1);

        clearDebuffsOnDeath(player, enemy);

        if (isId(enemy->ID,
                       "Don Quixote:The Manager of La Manchaland") == 0) {
          printf(
              "\n%s: I shall show you. Even if it’s a false dream... I will still "
              "move forward without hesitation!\n",
              player->name);
        } else {
          printf("\n%s: \"Let wrap it up.\"\n", player->name);
        }
      }

      // Sukuna:King of Curse cursed reverse technique
      if (isId(player->ID, "Sukuna:King of Curse") == 0 && player->HP <= 0 && player->Passive == 0) {

        player->Passive = 1;

        player->HP = player->MAX_HP;
        player->Sanity = 45;

        printf("\n%s used 'Cursed Reverse Technique', heal up to Max HP, heal Sanity to 45 (Once per Encounter)\n", player->name);

        sleep(1);

        printf("\n%s: \"Arm yourself...\"\n", player->name);

        sleep(1);
      }

      // Jia Qiu Anti low
      if (isId(player->ID, "Jia Qiu") == 0 && player->HP <= player->MAX_HP * 0.4 &&
          player->Passive == 3) {

        player->Passive = 4;

        player->HP = (int)(player->MAX_HP * 0.4);

        printf("\n%s blocked, cap HP to 40%% for this turn\n", player->name);

        sleep(1);

        if (player->Stagger > 0) {
            player->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            player->name);

          sleep(1);
        }
      }

      // Jia Qiu LAST
      if (isId(player->ID, "Jia Qiu") == 0 && player->HP <= player->MAX_HP * 0.2 &&
          player->Passive == 5) {

        player->Passive = 6;

        player->HP = (int)(player->MAX_HP * 0.2);

        printf("\n%s blocked, cap HP to 20%% for this turn\n", player->name);

        sleep(1);

        if (player->Stagger > 0) {
            player->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            player->name);

          sleep(1);
        }
      }

      // Erlking Heathcliff Anti low
      if (isId(player->ID, "Erlking Heathcliff") == 0 && player->HP <= 50 &&
          player->Passive == 1) {

        player->Passive = 2;

        player->HP = 50;

        printf("\n%s used 'Withstand', cap HP to 50 for this turn\n", player->name);

        sleep(1);

        if (player->Stagger > 0) {
          player->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            player->name);

          sleep(1);
        }

        sleep(1);

        printf("\n%s: \"Please, Catherine. Appear before me and tear me asunder. Let me see your eyes as I expire.\"\n", player->name);

        sleep(1);
      }

    // ----------------- King in Binds -----------------------

    // King in Binds Bandages of the King in Binds
    if (isId(enemy->ID, "King in Binds") == 0 && enemy->HP <= enemy->MAX_HP * 0.2 &&
        enemy->Passive == 1) {

      inflictStatus(player->Sinking, 2, 1, 0, 99, 0, 99);

      printf("\n%s gain +2 Sinking Stack (%d) and +1 Sinking Count (%d) from 'Bandages of the King in Binds'\n", player->name, player->Sinking[0], player->Sinking[1]);
    }

// King in Binds anti low
if (isId(player->ID, "King in Binds") == 0 && player->HP <= player->MAX_HP * 0.2 &&
    player->skills[0].active == 0) {

  player->skills[0].active = 1;

  player->Passive = 1;

  player->HP = (int)(player->MAX_HP * 0.2);

  printf("\n%s snapping Bandages, cap HP to 20%% for this turn, apply 'Bandages of the King in Binds' on all enemies.\n", player->name);
}

    // ---------------------- Anti death effect ----------------------

    // Erlking Heathcliff Faded promise for wild hunt
    if (isId(enemy->ID, "Erlking Heathcliff") == 0 && isId(player->ID, "Heathcliff:Wild Hunt") == 0 && (enemy->skills[7].active == 0))  {

          enemy->skills[7].active--;

      if (player->HP < 1) player->HP = 1;

    }

    // Hong lu:The Lord of Hongyuan - Passive
    if ((isId(player->ID, "Hong lu:The Lord of Hongyuan")) == 0 && player->skills[5].active == 0) {

      player->skills[5].active--;

      if (player->HP < 1) player->HP = 1;

    }

      // Heishou Pack - You Branch Adept Heathcliff - Anti death Passive
      if ((isId(player->ID, "Heishou Pack - You Branch Adept Heathcliff")) == 0 && player->skills[3].active == -1) {

        player->skills[3].active--;

        if (player->HP < 1) player->HP = 1;

      }

      // Meursault:Blade Lineage Mentor - Passive
      if (isId(player->ID, "Meursault:Blade Lineage Mentor") == 0 && player->Passive == -1) {

        player->Passive--;

        if (player->HP < 1) player->HP = 1;

      }

    // --------------------------------------------------

    // Binah - phase 2
    if (isId(player->ID, "Binah") == 0 && !player->Passive && player->HP <= player->MAX_HP*0.5) {

      player->Passive = 1;

      printf("\n%s: \"Ara~... I really surprised that you pushed me this far; then let's get a bit 'Serious'.\"\n", player->name);

      sleep(1);

          player->HP = 1150;
           player->MAX_HP = 1150;
          player->DamageUp[1] += 100;
          player->FinalPowerUp[1] += 5;

          player->skills[0].name = "Fairy";
           player->skills[0].BasePower += 3;
          player->skills[0].Unbreakable = 2;

          player->skills[1].name = "Chain";
          player->skills[1].BasePower += 1;
          player->skills[1].CoinPower += 2;
          player->skills[1].Unbreakable = 1;

          player->skills[2].name = "Pillar";
          player->skills[2].BasePower += 1;
          player->skills[2].CoinPower += 2;
          player->skills[2].Unbreakable = 1;

          player->skills[3].name = "Lock";
          player->skills[3].Coins += 1;
          player->skills[3].Unbreakable = 2;
          player->skills[3].CoinPower -= 4;

          player->skills[4].name = "Shockwave";
          player->skills[4].BasePower += 3;
          player->skills[4].CoinPower += 1;
          player->skills[4].Unbreakable = 3;

      player->defenseSkill[0].name = "Spatial Manipulation";
      player->defenseSkill[0].BasePower += 5;
      player->defenseSkill[0].CoinPower += 5;

      printf("\n%s at 50%% or less HP, 'Serious' activated! increase HP and Max HP to 1150, gains +100%% damage and 5 Final Power for one turn; then gain new Skills set (Once per Encounter) (Cannot be defeat until this effect activated)\n", player->name);

      sleep(1);

      if (isId(enemy->ID, "Fixer grade 9?") == 0) {
      printf("\n%s gains 'Shin (心) - The Black Silence', Defense +50, Offense +15, +1 Base Power and +10%% damage for every 10 different Sanity, All Skills become Unbreakable Coins\n",
        enemy->name);

    }
    }



    // ----------------- No Effect allow below here ------------------------

    // --- Decrement Sanity Lock ---
    if (player->SanityFreezeTurns > 0) {
        player->SanityFreezeTurns--;
        if (player->SanityFreezeTurns == 0) player->SanityFreezeTurns = 0;
    }

    // ---------------------------------

    // for character panic recover

    // --- Panic Recovery ---
    if (player->Sanity <= -45 && player->SanityFreezeTurns != 1) {

      // Check if the enemy is permanently locked in panic (-1)
      if (player->SanityFreezeTurns == 0) {

          player->Sanity = 0;

        printf("\n%s snaps out of panic! (Sanity reset to 0)\n", player->name);


        // Buff after panic (Only applies if they successfully recovered)
        if (isId(player->ID, "Erlking Heathcliff") == 0) {
            player->FinalPowerUp[1] += 2;
          printf("\n%s gains 2 Final Power up next turn\n", player->name);
        } 
        else if (isId(player->ID, "Lei heng") == 0) {
          int healSanity = (player->MAX_HP - player->HP) / player->MAX_HP * 100;
            if (healSanity > 30) healSanity = 30;

              updateSanity(player, healSanity);
              printf("\n%s heals Sanity by this unit's missing HP (%d - Max 30)\n", player->name, player->Sanity);
          }
        else if (isId(player->ID, "Sancho:The Second Kindred of Don Quixote") == 0 && player->Passive >= 1) {

            player->Passive -= 3;
          if (player->Passive < 1) player->Passive = 1;
          player->FinalPowerUp[1] += 3;
          player->DamageUp[1] += 30;

              printf("\n%s consumes 3 Hardblood (%d) to gain 3 Final Power Up and +30%% damage next turn\n", player->name, player->Passive);
          }
        else if (isId(player->ID, "Sukuna:King of Curse") == 0) {

              player->ClashPowerUp[1] += 3;

                printf("\n%s gain 2 Clash Power Up next turn\n", player->name);
            }
      } 
    }



    // --- ลดจำนวนเทิร์น Stagger ---
    if (player->Stagger > 0) {
        player->Stagger--;
        if (player->Stagger == 0) {
            printf("\n%s has recovered from STAGGER!\n", player->name);
        }
    }




    clearTurnEffects(player);
    resetCharacterSkillsBuffs(player);

    if (isId(enemy->ID, "Evil Bandit") == 0 && enemy->HP <= 0) {

    printf("\n%s: \"SHIT!\"\n", enemy->name);

    sleep(1);

    printf("\n%s: \"I can't lose like this...\"\n", enemy->name);

    sleep(1);

    printf("\n%s: \"I...I...\"\n", enemy->name);

    sleep(1);

    printf("\n%s: \"Tricked YOU!\"\n", enemy->name);

    sleep(1);

    printf("\n%s: \"HAHAHA!!!\"\n", enemy->name);

    sleep(1);

    printf("\nSuddenly A sword from sky fly through the air and lace between your group and him. The wind from impact push away your group from him\n");

    sleep(3);

    printf("\n%s: \"Good luck with him. HAHAHA!\"\n", enemy->name);

    sleep(1);

    printf("\n%s: \"Stop him!\"\n", player->name);

    sleep(1);

    printf("\nHowever A man in the hoodie cover the head and body, along with one more sword besides jump from the nearby building and grab the sword and stop you\n");

    sleep(3);

    printf("\nYou stop and know this would happen... Still he's strong...\n");

     sleep(1);

    printf("\n%s gains 5 'Team of Friend'\n", player->name);

    sleep(1);

    printf("\n'Team of Friend':\n"
      " - Gain Teammate equal to Stack, each has different HP"
      " - When attack: Your team attack 1 more time (Once per skills; All teams' Sanity equal to this unit's)\n"
      " - When taking damage that bring this unit's HP to 0, tranfer damage to one of your Teammate\n"
      " - If any of your teammate reach 0 HP, they use 'Retreat', then loses 1 'Team of Friend' Stack\n"
      "\x1b[1;30mYou are the who strongest here, so don't even think to fall that easy while we are here!\x1b[0m\n");

    sleep(3);

    // --- เริ่มการเปลี่ยนร่าง (The Real Boss Appears) ---
    printf("\n[!!!] WARNING: TARGET CHANGED [!!!]\n");
    sleep(1);

    // ล้างบัฟ/ดีบัฟเก่าของ Bandit ทิ้งให้หมด
    clearDebuffsOnDeath(enemy, player);
    initializeCharacterBuffs(enemy); 
    clearTurnEffects(enemy);

    // ตั้งค่าบอสตัวจริง
    enemy->name = "????????";
    enemy->ID = "Your Best Friend";
    enemy->MAX_HP = 1200;
    enemy->HP     = 1200;
    enemy->MinSpeed = 4;
    enemy->MaxSpeed = 8;
    enemy->Sanity = 0;
    enemy->hasSanity = 1;
    enemy->sanityGainBase = 12; // เก่งกว่าคนปกติ
    enemy->sanityLossBase = 6;
    enemy->immuneToPanicSkip = 1;
    enemy->Stagger = 0; // ไม่ให้เริ่มมาแล้วติด Stagger

    // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable, Type
    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter

    // ตั้งสกิลใหม่ให้บอสตัวจริง
    enemy->numSkills = 8;
    enemy->skills[0] = (SkillStats){"One White Sword Style: Split the Sky", 6, 4, 2, 40, 25, 1.0, 0, 0, 3, 1, 0};
    enemy->skills[1] = (SkillStats){"Supreme White Sword Style: Split the Heaven", 5, 3, 5, 40, 25, 1.0, 0, 1, 0, 1, 0};
    enemy->skills[2] = (SkillStats){"One Black Sword Style: Split the Below One", 14, -4, 2, 40, 25, 1.0, 0, 0, 3, 1, 0};
    enemy->skills[3] = (SkillStats){"Supreme Black Sword Style: Split the Hell", 20, -3, 5, 40, 25, 1.0, 0, 1, 0, 1, 0};
    enemy->skills[4] = (SkillStats){"One Sword Style: Split the Earth", 10, 5, 4, 42, 25, 1, 0, 4, 1, 1, 0};
    enemy->skills[5] = (SkillStats){"Supreme One Sword Style: Split the World", 15, 2, 10, 42, 25, 1, 0, 10, 0, 1, 0};
    enemy->skills[6] = (SkillStats){"Degraded Two Sword Style: Love...", 8, 2, 1, 45, 25, 1, 0, 1, 0, 1, 0};
    enemy->skills[7] = (SkillStats){"Degraded Two Sword Style: Hate...", 10, -2, 1, 45, 25, 1, 0, 1, 0, 1, 0};

    // ตั้งสกิลป้องกันใหม่ (อาจจะเป็น Counter แทน Evade)
    enemy->numDefenseSkills = 2;
    enemy->defenseSkill[0] = (SkillStats){"Gift from Hell", 16, -6, 2, 40, 25, 1.0, 0, 2, 2, 1, 5};
    enemy->defenseSkill[1] = (SkillStats){"Cursed from Heaven", 7, 13, 1, 40, 25, 1.0, 0, 1, 2, 1, 4};

    // Name
    printf("\nName: ????????\n");
    printf("HP: %.0f / %.0f\n", enemy->HP, enemy->MAX_HP);
    printf("Speed: %d ~ %d\n", enemy->MinSpeed, enemy->MaxSpeed);

          //Taunt
            printf("\"I know this day will come...\"\n\n");

      //Taunt
      printf("\"He knows who you are\"\n\n");

            //Description
            printf("Can't see who is he... but he feeling familiar\n\n");

            //Passive
             printf("Passive Skills:\n");
      printf("\n 1. Burn\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (Turn End: When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), Turn End: Take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
      printf("\n 2. Dark Burn\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (Turn End: When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), Turn End: Take fixed true damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
      printf("\n 3. Tremor\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Trigger by 'Tremor Burst', Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP/4) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess. Turn End: Lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
      printf("\n 4. Tremor - White\n Change when Triggered by Amplitude Conversion\n"
        " - Gain 10%% Damage Down for every 10 (Stack + Count)\n"
        " - On Tremor Burst, Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess.\n"
        " - Turn End: Lose 1 Count. When reach 0 Count, loses all Stack and Change back to Normal 'Tremor' too (Max 99 Stack/Count)\n");
      printf("\n 5. Tremor - Black\n Change when Triggered by Amplitude Conversion\n"
        " - Gain 10%% Take Damage Up for every 10 (Stack + Count)\n"
        " - On Tremor Burst, Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess.\n"
        " - Turn End: Lose 1 Count. When reach 0 Count, loses all Stack and Change back to Normal 'Tremor' too (Max 99 Stack/Count)\n");
      printf("\n 6. Tremor - Grey\n Change when Triggered by Amplitude Conversion\n"
        " - Gain 10%% Take Damage Up and 10%% Damage Down for every 5 (Stack + Count)\n"
        " - On Tremor Burst, Raise Stagger Threshold equal to (Stack x 2); then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess.\n"
        " - Turn End: Lose 1 Count. When reach 0 Count, loses all Stack and Change back to Normal 'Tremor' too (Max 99 Stack/Count)\n"
        "\n\x1b[1;30m I don't usually use my Two Sword Style that much, because of its strong\x1b[0m\n");
      printf("\n 7. Love and Hate\n Start the Encounter with \x1b[0;31mLove\x1b[0m\n"
        " - If this unit loses all its shield or at 70%% HP or less, Convert \x1b[0;31mLove\x1b[0m to \x1b[0;93mHate\x1b[0m, and heals HP up to 100%%\n"
        " - at 65%% HP or less, Gain \x1b[0;93mShin (心) - The Grey Reaper\x1b[0m\n"
        "\n\x1b[1;30m You're are my friend... Is it? so that what friend do to each other, huh? I Hate YOU\x1b[0m\n");
    printf("\n 8. Love\n Lose 50 Offense Level and 50 Defense Level; gain Shield HP equal to (Max HP) (Activate once per encounter)\n"
      "\n\x1b[1;30m I thought we are good as long as I keep it up\x1b[0m\n");
    printf("\n 9. Hate\n Gain +0.5%% Damage Multiplier\n"
      "\n\x1b[1;30m But suddenly one day... You left me... Haha... I can't blame you for that, I supposed\x1b[0m\n");
printf("\n 10. Shin (心) - The Grey Reaper\n"
  " - Turn Start: Gain 100 Shield, +3 Offense Level, +3 Defense Level and 5 Attack Power Up\n"
  " - Min & Max Speed +3\n"
  " - Using Base Skill gain +3 Poise Stack and +3 Poise Count\n"
  " - On Hit, inflict +1 Dark Burn Stack and +1 Dark Burn Count\n"
  " - When Hit, inflict +2 Tremor Stack\n"
  "\n\x1b[1;30m Master... You used to be the one who taught me, now you are the one who oppose me...\x1b[0m\n");
printf("\n 11. Himinvateinn\n Gain by using Certain Skill:\n"
  " - On Hit, inflict +1 Tremor Stack and +1 Tremor Count (Once per turn)\n"
    " - While possesing, fixed Coin Power to Positive, if this unit's equipped Defense Skill, Take -50%% Damage\n"
    " - When Hit, inflict +1 Tremor Stack\n"
  " - While possesing this and \x1b[0;93mNiðbrandr\x1b[0m, this unit...\n"
  "\n\x1b[1;30m A White Sword relic, wield to power beyond the sky and heaven, opposite with the one Black Sword relic, how ironically that something that this vast opposite can be wield by one person\x1b[0m\n");
printf("\n 12. Niðbrandr\n Gain by using Certain Skill:\n"
    " - On Hit, inflict +1 Burn Stack and +1 Burn Count (Once per turn)\n"
      " - While possesing, fixed Coin Power to Negative if this unit's equipped Attack Skill or Clashable Counter Skill, Deal +20%% Damage on Unbreakable Coins\n"
      " - On Clash Win: inflict +1 Dark Burn\n"
  " - While possesing this and \x1b[0;93mHiminvateinn\x1b[0m, this unit...\n"
  "\n\x1b[1;30m A Black Sword relic, wield to power deep than the ground and the hell, opposite with the one White Sword relic, how ironically that something that this vast opposite can be wield by one person\x1b[0m\n");
      printf("\n 13. A Genius - Adaptation\n Every time this unit lost the clash:\n"
        " - Gain 1 \x1b[0;93mAdaptation\x1b[0m\n"
          " - If this unit has \x1b[0;31mLove\x1b[0m, Enemy heals 5 Sanity\n"
          " - If this unit has \x1b[0;93mHate\x1b[0m, Gain 1 Base Power next turn\n"
      "\n\x1b[1;30m A swordsman born of pure genius. While the world envies his effortless grace, does the one behind the blade can find any joy in this path? perhaps he already lost it...\x1b[0m\n");
      printf("\n 14. Adaptation\n"
        " - Max Stack: 10\n"
          " - Can be consumes by Certain Skills\n"
          " - Gain +1 Attack Power and +1 Defense Power for every 2 Stack\n"
        " - Gain +1 Offense Level and +1 Defense Level for every 4 Stack\n"
      " - At 10 Stack, Gain +1 Base Power and +1 Clash Power\n"
      "\n\x1b[1;30m How ironic... it took only a few minutes for me to see through your technique... and how this will end...\x1b[0m\n");
      printf("\n 15. Murderer\n"
        " - When this unit deal HP or Shield HP damage, Gain 1 Attack Power Up and 1 Base Power Down\n"
          " - When Enemy lost 'Team of Friend', Gain 10 Base Power Down and 50%% Take Damage Up\n"

        "\n\x1b[1;30m Is my hands shaking? Ha... I thought I used to this already\x1b[0m\n\n"
        
          " - If Enemy's HP at 30%% or less, Gain 3 Offense Level Up and 5 Defense Level Down\n"

        "\n\x1b[1;30m Is this almost over, right?\x1b[0m\n\n"
        
      " - When Enemy lost all 'Team of Friend', Gain 30 Base Power Down and 100%% Take Damage Up for one turn\n"
      "\n\x1b[1;30m Yes... I killed them, all of them, regardless their background story, I did my job... ah... Whom was the first person I killed, huh? Is it my father? or my mother? ha... None of that does not matter anymore, the point is... I'm not feeling regret... Yes, I don't feel it... how terrify\x1b[0m\n");
      printf("\n 16. Panic Recovery\n Turn End: if in Panic, Take 5%% fixed True Damage then reset SP to 30.\n"
        
         "\n\x1b[1;30m Feeling much better, I need to keep my calm\x1b[0m\n");
      printf("\n 17. Fixed Panic\n This unit's Panic Type does not change when inflicted with an effect that changes Panic Types. Instead, this unit is inflicted with an effect that is inflicted against Non-SP Units.\n");
      printf("\n 18. Panic Type - Rancor\n"
        " Low Morale:\n"
        "  - Turn Start: Gain 5 Defense Level Down, if this unit lost the Clash last turn, gain +2 Final Power, +2 Attack Power Up, +2 Defense Power Up and 5 Defense Down\n"
        " Panic:\n"
        "  - Combat Start: This unit attack with 'Degraded Two Sword Style: Hate...' as Unopposed attack. Turn Start: Gain 10 Defense Level Down, if this unit lost the Clash last turn, gain +2 Final Power, +4 Attack Power Up, +4 Defense Power Up and 10 Defense Down\n");

      printf("\nSkills (%d Attack Skills, %d Defense Skills):\n", enemy->numSkills, enemy->numDefenseSkills);

    printf("\nAttack Skills %d:\n\n", enemy->numSkills);

      for (int i = 0; i < enemy->numSkills; i++) {
        SkillStats s = enemy->skills[i];
          printf(" %d. %s (%s)\n", i + 1, s.name, getSkillTypeName(s.skillType));

        if (s.Unbreakable > 0) {
            if (!s.Clashable) {
            printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Unclashable)\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
            } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Clashable)\n",
               s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
          }  else 
          if (!s.Clashable) {
            printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Unclashable)\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
            } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Clashable)\n",
               s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
        }

     printf("\nDefense Skills %d:\n\n", enemy->numDefenseSkills);

    for (int i = 0; i < enemy->numDefenseSkills; i++) {
      SkillStats s = enemy->defenseSkill[i];
        printf(" %d. %s (%s)\n", i + 1, s.name, getSkillTypeName(s.skillType));

      if (s.Unbreakable > 0) {
          if (!s.Clashable) {
          printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Unclashable)\n",
                 s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
          } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Clashable)\n",
             s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
        }  else 
        if (!s.Clashable) {
          printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Unclashable)\n",
                 s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
          } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Clashable)\n",
             s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
      }

    sleep(5);

    printf("\n%s: \"You've wasted enough of our time.\"\n", enemy->name);

    sleep(2);

      }

    // -------------------------------------------------------- Lost CutScene --------------------------------------------------------

    if (isId(player->ID, "Muga Ryōshū") != 0) {

    // Lost CutScene
    if (isId(enemy->ID, "Lei heng") == 0 && enemy->HP <= enemy->MAX_HP*0.2) {

      enemy->HP = 10000;

      enemy->Stagger = 0;

      enemy->BasePowerUp[0] += 20;

      printf("\n%s: \"I'll be frank, y'all. Real impressed that you even pushed me this far.\"\n", enemy->name);

      sleep(2);

      attackPhase(enemy, &enemy->skills[0],
        enemy->skills[0].Offense, enemy->skills[0].Defense,
        player, &player->skills[0], player->skills[0].Offense,
         player->skills[0].Defense, enemy->skills[0].Coins, 0 , 0);

      enemy->HP = 1000000;

      sleep(1);

      if (strstr(player->name, "Ryoshu") != NULL) {
        printf("\n%s: \"But... Yoshihide\"\n", enemy->name);
      } else if (isId(player->ID, "Meursault:The Thumb") == 0) {
        printf("\n%s: \"But... Chacuihu\"\n", enemy->name);
      } else {
       printf("\n%s: \"But... %s...\"\n", enemy->name, player->name);
      }

      sleep(3);

      enemy->HP = 100000;

      attackPhase(enemy, &enemy->skills[2],
        enemy->skills[2].Offense, enemy->skills[2].Defense,
        player, &player->skills[0], player->skills[0].Offense,
         player->skills[0].Defense, enemy->skills[2].Coins, 0 , 0);

      printf("\n%s: \"... Ya darn sure oughta've harder if ya really wanted to win!\"\n", enemy->name);

      enemy->HP = 1000000;

      sleep(3);

      player->HP = 1;
      enemy->HP = 0;


    } else if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP <= 0) {

      printf("\n%s: \"Fine, You win.\"\n", enemy->name);

      sleep(1);

    } else {

    // ---------------------------------------------------------------------------------------------------------------

    }

    }

  }

























  // ------------------------- King in Binds Battle -------------------------
void runKingInBindsBattle(
        Character *player,
        Character *boss,
        int *playerSkill1, int *playerSkill2, int *playerSkill3, int playerDefenseSkill,
        int *playerLastUnused,
        int *enemySkill1,  int *enemySkill2,  int *enemySkill3,
        int *enemyLastUnused)
{
    KingDmgBonus = 0.0f;
   KingClashBonus = 0;

    // ============================================================
    // PHASE 1 — Knight (shadow of the player)
    // ============================================================

  const char *realID = player->name;

  // 1. ตั้งชื่อแยกให้จบตั้งแต่ตรงนี้
  static char pTag[256], eTag[256];
  sprintf(pTag, "[Player] %s", realID);
  sprintf(eTag, "[Enemy] %s", realID);

  Character enemy = *player; 

  player->name = pTag;
    enemy.name = eTag;

  // Reset shadow's Accelerating Future stacks to 0 so it builds ClashPower from scratch
  enemy.skills[4].active  = 0;
  enemy.skills[17].active = 0;

  printf("\nThe King in Binds raises his hand...\n");
  sleep(1);
  printf("\nA body that covered with Red Bandage snaps out, A shadow emerges — bearing your very visage.\n");
  sleep(2);
  printf("\n'A Knight' has appeared! (A mirror of %s)\n\n", player->name);
  sleep(1);

   runCombatEvent(player, boss, &kingFirstEvent, playerSkill1, playerSkill2, playerSkill3);

      if (KingDmgBonus > 0.0f) {
          for (int i = 0; i < player->numSkills; i++)
              player->skills[i].DmgMutiplier += KingDmgBonus;
          KingDmgBonus = 0.0f;
      }

  // Knight skill pool — แยกจาก player
  int kSkill1 = -1, kSkill2 = -1, kSkill3 = -1, kLastUnused = -1;
  getSkills(&enemy, &kSkill1, &kSkill2, &kSkill3, kLastUnused, enemy.numSkills);

  SkillStats *playerSkillEffective = NULL;  

  /*
  player->MAX_HP = 3000;
  player->HP = 3000;
    enemy.MAX_HP = 3000;
    enemy.HP = 3000;*/


  while (player->HP > 0 && enemy.HP > 0) {

    if (KingClashBonus > 0) {
            player->ClashPowerUp[0] += KingClashBonus;
    }

      printf("\n--- Turn %d (Knight Phase) ---\n", TurnCount);

    SkillStats *selectedEnemyPtr = NULL;

    int playerSkillIndex = -1;
    int enemySkillIndex = -1;

      // handleTurnStart ด้วย player จริงเป็น p1, knight เป็น enemy
    // 1. รัน Passive ของ Player (ตัวเรา)
    // ส่งสกิลของเรา (*playerSkill1...) เข้าช่องที่ 4-5 และสกิลของร่างเงา (kSkill1...) เข้าช่องที่ 6-7

    handleTurnStart(player, &enemy, &selectedEnemyPtr,
      playerSkill1, playerSkill2, playerSkill3, &kSkill1, &kSkill2, &kSkill3);

    // 2. รัน Passive ของ Knight (ร่างเงา)
    // สลับตำแหน่ง: ส่งร่างเงาเข้าช่องแรก, ส่งสกิลร่างเงาเข้าช่องที่ 4-5 และสกิลของเราเข้าช่องที่ 6-7
    handleTurnStart(&enemy, player, &playerSkillEffective,
      &kSkill1, &kSkill2, &kSkill3, playerSkill1, playerSkill2, playerSkill3);

    int IsplayerUnableToAct = isPanicked(player) || isStaggered(player);
    int IsenemyUnableToAct  = isPanicked(&enemy)  || isStaggered(&enemy);

    // ถ้าทั้งคู่ทำอะไรไม่ได้เลย
    if (IsplayerUnableToAct && IsenemyUnableToAct) {
          printf("\nBoth are unable to act! They recover...\n");
          if (isPanicked(player)) { player->Sanity = 0; }
      if (isPanicked(&enemy)) { enemy.Sanity = 0; }
          TurnCount++;
          continue;
      }

    if (IsplayerUnableToAct) {
      if (isStaggered(player)) printf("\n%s is STAGGERED and cannot act!\n", player->name);
      else if (isPanicked(player)) printf("\n%s is in PANIC and cannot act!\n", player->name);

      // Lose Envy Resonance
        if (isId(player->ID, "The Middle Little Brother Sinclair") == 0) {

            player->Passive = 0;

          printf("\n%s loses all Envy Resonance\n", player->name);

        }
    }

    if (IsenemyUnableToAct) {
      if (isStaggered(&enemy)) printf("\n%s is STAGGERED and cannot act!\n", enemy.name);
      else if (isPanicked(&enemy)) printf("\n%s is in PANIC and cannot act!\n", enemy.name);

      // Lose Envy Resonance
      if (isId(enemy.name, "The Middle Little Brother Sinclair") == 0) {

            enemy.Passive = 0;

        printf("\n%s loses all Envy Resonance\n", enemy.name);

      }
    }

      // แสดง HP
    printf("\nCurrent HP:\n");

    // แสดงผลของ Player
    if (player->Stagger > 0) printf("[Stagger] ");
    printf("%s = %.2f / %.2f", player->name, player->HP, player->MAX_HP);
    if (player->Shield > 0 || player->TempShield > 0) printf(" (Shield %.2f)", player->Shield + player->TempShield);
    printf("\n");

    // แสดงผลของ Enemy
    if (enemy.Stagger > 0) printf("[Stagger] ");
    printf("%s = %.2f / %.2f", enemy.name, enemy.HP, enemy.MAX_HP);
    if (enemy.Shield > 0 || enemy.TempShield > 0) printf(" (Shield %.2f)", enemy.Shield + enemy.TempShield);
    printf("\n");

      if (player->hasSanity || enemy.hasSanity) {
          printf("[Sanity] ");
          if (player->hasSanity)
              printf("%s: %d (%s) ", player->name, player->Sanity, getSanityStatus(player));
          if (enemy.hasSanity)
              printf("| %s: %d (%s)", enemy.name, enemy.Sanity, getSanityStatus(&enemy));
          printf("\n");
      }

    // ---------- Before Fight -----------

    // 1. รัน Passive ของ Player (ตัวเรา)
    // ส่งสกิลของเรา (*playerSkill1...) เข้าช่องที่ 4-5 และสกิลของร่างเงา (kSkill1...) เข้าช่องที่ 6-7
    handleBeforeFight(player, &enemy, &selectedEnemyPtr,
      *playerSkill1, *playerSkill2, kSkill1, kSkill2);

    // 2. รัน Passive ของ Knight (ร่างเงา)
    // สลับตำแหน่ง: ส่งร่างเงาเข้าช่องแรก, ส่งสกิลร่างเงาเข้าช่องที่ 4-5 และสกิลของเราเข้าช่องที่ 6-7
      handleBeforeFight(&enemy, player, &playerSkillEffective,
      kSkill1, kSkill2, *playerSkill1, *playerSkill2);

    // -------------------------------------------------------
      // Knight เลือก skill — แสดง SKILL ของ TURN นี้ก่อน
      // (kSkill1/kSkill2 ที่ roll มาจาก turn ก่อน)
      // -------------------------------------------------------
    // สุ่ม: ร่างเงามีโอกาส 20% ที่จะใช้สกิลป้องกัน (เหมือน AI Limbus ทั่วไป)
    if (enemy.numDefenseSkills > 0 && (rand() % 100 < 20)) {
        selectedEnemyPtr = &enemy.defenseSkill[0];

    } else {

    enemySkillIndex = (rand() % 2 == 0) ? kSkill1 : kSkill2;
    kLastUnused = (enemySkillIndex == kSkill1) ? kSkill2 : kSkill1;
       selectedEnemyPtr = &enemy.skills[enemySkillIndex];

    }

    int ECoinBoost = 0;
    if (selectedEnemyPtr->CoinPower >= 0) {
          ECoinBoost = enemy.PlusCoinPowerBoost[0] - enemy.PlusCoinPowerDrop[0];
    } else {
          ECoinBoost = enemy.MinusCoinPowerDrop[0] - enemy.PlusCoinPowerBoost[0];
    }

    int playerTempOffense = 0, playerTempDefense = 0;
    int enemyTempOffense  = 0, enemyTempDefense  = 0;

    playerTempOffense += (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]);
    playerTempDefense += (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]);
    enemyTempOffense  += (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]);
    enemyTempDefense  += (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]);

      // แสดง knight skill
      if (!IsenemyUnableToAct) {
          SkillStats *ks = selectedEnemyPtr;
          if (ks->Unbreakable > 0 && (ks->Clashable || ks->skillType != 0))
              printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(ks->skillType),
                     ks->name, ks->BasePower + (enemy.BasePowerUp[0] - enemy.BasePowerDown[0]),
                     ks->CoinPower + ECoinBoost, ks->Coins,
                     ks->Offense + (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]),
                     ks->Defense + (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]),
                     ks->Unbreakable);
          else if (ks->Unbreakable <= 0 && (ks->Clashable || ks->skillType != 0))
              printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(ks->skillType),
                ks->name, ks->BasePower + (enemy.BasePowerUp[0] - enemy.BasePowerDown[0]),
                   ks->CoinPower + ECoinBoost, ks->Coins,
                     ks->Offense + (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]),
                     ks->Defense + (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]));
          else if (ks->Unbreakable > 0 && !ks->Clashable && ks->skillType == 0)
              printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(ks->skillType),
                ks->name, ks->BasePower + (enemy.BasePowerUp[0] - enemy.BasePowerDown[0]),
                   ks->CoinPower + ECoinBoost, ks->Coins,
                     ks->Offense + (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]),
                     ks->Defense + (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]),
                     ks->Unbreakable);
          else if (ks->Unbreakable <= 0 && !ks->Clashable && ks->skillType == 0)
              printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(ks->skillType),
                ks->name, ks->BasePower + (enemy.BasePowerUp[0] - enemy.BasePowerDown[0]),
                   ks->CoinPower + ECoinBoost, ks->Coins,
                     ks->Offense + (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]),
                     ks->Defense + (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]));
      }

      // -------------------------------------------------------
      // Player เลือก skill — แสดง skill ของ turn นี้
      // (playerSkill1/playerSkill2 ที่ roll มาจาก turn ก่อน)
      // -------------------------------------------------------

        int PCoinBoost1 = 0;
        if (player->skills[*playerSkill1].CoinPower >= 0) {
                PCoinBoost1 = player->PlusCoinPowerBoost[0] - player->PlusCoinPowerDrop[0];
        } else {
                PCoinBoost1 = player->MinusCoinPowerDrop[0] - player->PlusCoinPowerBoost[0];
        }

        int PCoinBoost2 = 0;
        if (player->skills[*playerSkill2].CoinPower >= 0) {
                PCoinBoost2 = player->PlusCoinPowerBoost[0] - player->PlusCoinPowerDrop[0];
        } else {
                PCoinBoost2 = player->MinusCoinPowerDrop[0] - player->PlusCoinPowerBoost[0];
        }

        int PCoinBoost3 = 0;
        if (player->skills[*playerSkill3].CoinPower >= 0) {
                PCoinBoost3 = player->PlusCoinPowerBoost[0] - player->PlusCoinPowerDrop[0];
        } else {
                PCoinBoost3 = player->MinusCoinPowerDrop[0] - player->PlusCoinPowerBoost[0];
        }

        if (!IsplayerUnableToAct) {
        printf("\nDashboard Skills:\n");

          if (player->skills[*playerSkill1].Unbreakable > 0 && player->skills[*playerSkill1].Clashable) {
              printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                    getSkillTypeName(player->skills[*playerSkill1].skillType),
                     player->skills[*playerSkill1].name,
                     player->skills[*playerSkill1].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill1].CoinPower + PCoinBoost1,
                     player->skills[*playerSkill1].Coins,
                     player->skills[*playerSkill1].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill1].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                     player->skills[*playerSkill1].Unbreakable);
          } else if (player->skills[*playerSkill1].Unbreakable <= 0 && player->skills[*playerSkill1].Clashable) {
              printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(player->skills[*playerSkill1].skillType),
                     player->skills[*playerSkill1].name,
                     player->skills[*playerSkill1].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill1].CoinPower + PCoinBoost1,
                     player->skills[*playerSkill1].Coins,
                     player->skills[*playerSkill1].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill1].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
          } else if (player->skills[*playerSkill1].Unbreakable > 0 && !player->skills[*playerSkill1].Clashable) {
              printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(player->skills[*playerSkill1].skillType),
                     player->skills[*playerSkill1].name,
                     player->skills[*playerSkill1].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill1].CoinPower + PCoinBoost1,
                     player->skills[*playerSkill1].Coins,
                     player->skills[*playerSkill1].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill1].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                     player->skills[*playerSkill1].Unbreakable);
          } else {
              printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(player->skills[*playerSkill1].skillType),
                     player->skills[*playerSkill1].name,
                     player->skills[*playerSkill1].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill1].CoinPower + PCoinBoost1,
                     player->skills[*playerSkill1].Coins,
                     player->skills[*playerSkill1].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill1].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
          }

          if (player->skills[*playerSkill2].Unbreakable > 0 && player->skills[*playerSkill2].Clashable) {
              printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(player->skills[*playerSkill2].skillType),
                     player->skills[*playerSkill2].name,
                     player->skills[*playerSkill2].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill2].CoinPower + PCoinBoost2,
                     player->skills[*playerSkill2].Coins,
                     player->skills[*playerSkill2].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill2].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                     player->skills[*playerSkill2].Unbreakable);
          } else if (player->skills[*playerSkill2].Unbreakable <= 0 && player->skills[*playerSkill2].Clashable) {
              printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(player->skills[*playerSkill2].skillType),
                     player->skills[*playerSkill2].name,
                     player->skills[*playerSkill2].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill2].CoinPower + PCoinBoost2,
                     player->skills[*playerSkill2].Coins,
                     player->skills[*playerSkill2].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill2].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
          } else if (player->skills[*playerSkill2].Unbreakable > 0 && !player->skills[*playerSkill2].Clashable) {
              printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(player->skills[*playerSkill2].skillType),
                     player->skills[*playerSkill2].name,
                     player->skills[*playerSkill2].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill2].CoinPower + PCoinBoost2,
                     player->skills[*playerSkill2].Coins,
                     player->skills[*playerSkill2].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill2].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                     player->skills[*playerSkill2].Unbreakable);
          } else {
              printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(player->skills[*playerSkill2].skillType),
                     player->skills[*playerSkill2].name,
                     player->skills[*playerSkill2].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill2].CoinPower + PCoinBoost2,
                     player->skills[*playerSkill2].Coins,
                     player->skills[*playerSkill2].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill2].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
          }

        // Next Skill
          if (player->skills[*playerSkill3].Unbreakable > 0 && player->skills[*playerSkill3].Clashable) {
              printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(player->skills[*playerSkill3].skillType),
                     player->skills[*playerSkill3].name,
                     player->skills[*playerSkill3].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill3].CoinPower + PCoinBoost3,
                     player->skills[*playerSkill3].Coins,
                     player->skills[*playerSkill3].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill3].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                     player->skills[*playerSkill3].Unbreakable);
          } else if (player->skills[*playerSkill3].Unbreakable <= 0 && player->skills[*playerSkill3].Clashable) {
              printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(player->skills[*playerSkill3].skillType),
                     player->skills[*playerSkill3].name,
                     player->skills[*playerSkill3].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill3].CoinPower + PCoinBoost3,
                     player->skills[*playerSkill3].Coins,
                     player->skills[*playerSkill3].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill3].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
          } else if (player->skills[*playerSkill3].Unbreakable > 0 && !player->skills[*playerSkill3].Clashable) {
              printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(player->skills[*playerSkill3].skillType),
                     player->skills[*playerSkill3].name,
                     player->skills[*playerSkill3].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill3].CoinPower + PCoinBoost3,
                     player->skills[*playerSkill3].Coins,
                     player->skills[*playerSkill3].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill3].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                     player->skills[*playerSkill3].Unbreakable);
          } else {
              printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(player->skills[*playerSkill3].skillType),
                     player->skills[*playerSkill3].name,
                     player->skills[*playerSkill3].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                     player->skills[*playerSkill3].CoinPower + PCoinBoost3,
                     player->skills[*playerSkill3].Coins,
                     player->skills[*playerSkill3].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                     player->skills[*playerSkill3].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
          }


        // Defense Skill

        int PCoinBoostDef = 0;
        if (player->defenseSkill[playerDefenseSkill].CoinPower >= 0) {
                  PCoinBoostDef = player->PlusCoinPowerBoost[0] - player->PlusCoinPowerDrop[0];
        } else {
                  PCoinBoostDef = player->MinusCoinPowerDrop[0] - player->PlusCoinPowerBoost[0];
        }

        printf("\n");
        if (player->defenseSkill[playerDefenseSkill].Unbreakable > 0) {
          printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
            getSkillTypeName(player->defenseSkill[playerDefenseSkill].skillType),
                 player->defenseSkill[playerDefenseSkill].name,
                 player->defenseSkill[playerDefenseSkill].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                 player->defenseSkill[playerDefenseSkill].CoinPower + PCoinBoostDef,
                 player->defenseSkill[playerDefenseSkill].Coins,
                 player->defenseSkill[playerDefenseSkill].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                 player->defenseSkill[playerDefenseSkill].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                 player->defenseSkill[playerDefenseSkill].Unbreakable);
        } else if (player->defenseSkill[playerDefenseSkill].Unbreakable <= 0) {
          printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
            getSkillTypeName(player->defenseSkill[playerDefenseSkill].skillType),
                 player->defenseSkill[playerDefenseSkill].name,
                 player->defenseSkill[playerDefenseSkill].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                 player->defenseSkill[playerDefenseSkill].CoinPower + PCoinBoostDef,
                 player->defenseSkill[playerDefenseSkill].Coins,
                 player->defenseSkill[playerDefenseSkill].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                 player->defenseSkill[playerDefenseSkill].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
        }
        printf("\n");

          int choice;
          while (1) {
            printf("Choose skill (1-2, or 0 to Use Defense instead of Skill 1): ");
            if (scanf("%d", &choice) == 1 && (choice == 1 || choice == 2 || choice == 0))
              break;
            while (getchar() != '\n')
              ;
          }

          if (choice == 0) {
            // --- กรณีเลือก Guard (แทนที่ Skill 1) ---
            playerSkillEffective = &player->defenseSkill[0];

            // สำคัญ: ตั้งค่าให้ระบบมองว่าเรา "ใช้" playerSkill1 ไปแล้ว
            playerSkillIndex = *playerSkill1; // (เพื่อให้ระบบหลังบ้านดึงชื่อหรือตำแหน่งมาใช้อ้างอิงได้)
            playerLastUnused = playerSkill2; // สกิลช่อง 2 จะถูกเก็บไว้

            // สุ่มสกิลใหม่มาแทนที่ช่องที่ 1 (และเลื่อนช่อง 3 มาช่อง 2 ตามระบบ getSkills ของคุณ)
            getSkills(player, playerSkill1, playerSkill2, playerSkill3, *playerLastUnused, player->numSkills);

          } else if (choice == 1 || choice == 2) {
            // --- กรณีเลือกสกิล 1 หรือ 2 ปกติ ---
          playerSkillIndex = (choice == 1 ? *playerSkill1 : *playerSkill2);
          playerLastUnused = (choice == 1 ? playerSkill2 : playerSkill1);
            playerSkillEffective = &player->skills[playerSkillIndex]; // ชี้ไปที่สกิลโจมตี

          // Roll new skill to replace used one
            getSkills(player, playerSkill1, playerSkill2, playerSkill3, *playerLastUnused, player->numSkills);

        } else {
          // Player can't act, just pick a random skill for defensive purposes
          playerSkillIndex = *playerSkill1;
        }
      }

     // roll skill ใหม่หลังเลือกแล้ว (สำหรับ turn หน้า)
      getSkills(&enemy, &kSkill1, &kSkill2, &kSkill3,
                kLastUnused, enemy.numSkills);


      // -------------------------------------------------------
      // getEffectiveSkill — knight ใช้ชื่อ player ชั่วคราว
      // เพื่อให้ passive buff ใน getEffectiveSkill ทำงาน
      // -------------------------------------------------------

      SkillStats *enemySkillEffective = NULL;

    if (IsplayerUnableToAct) {
      SkillStats dummy = player->defenseSkill[0];
      getEffectiveSkill(player, &enemy, &dummy, &playerTempOffense, &playerTempDefense);
      playerSkillEffective = NULL;
    } else {
      playerSkillEffective = getEffectiveSkill(player, &enemy, playerSkillEffective,
                        &playerTempOffense, &playerTempDefense);
    }

    if (IsenemyUnableToAct) {
      SkillStats dummyK = enemy.defenseSkill[0];
      getEffectiveSkill(&enemy, player, &dummyK, &enemyTempOffense, &enemyTempDefense);
      enemySkillEffective = NULL;
    } else {
      enemySkillEffective = getEffectiveSkill(&enemy, player, selectedEnemyPtr,
                        &enemyTempOffense, &enemyTempDefense);
    }

      // -------------------------------------------------------
      // Combat (เหมือน main loop ทุกอย่าง)
      // ชื่อ knight ยังเป็น player->name อยู่ในช่วงนี้
      // -------------------------------------------------------

    int playerGoesFirst = 0;

  int pType = (playerSkillEffective != NULL) ? playerSkillEffective->skillType : -1;
  int eType = (enemySkillEffective != NULL) ? enemySkillEffective->skillType : -1;

        if (eType == 3) { 
          // ถ้าศัตรูใช้ Counter (Type 3) เราต้องตีก่อนเสมอ
          playerGoesFirst = 1;
        } else if (pType == 3) { 
          // ถ้าเราใช้ Counter (Type 3) ศัตรูต้องตีก่อนเสมอ
          playerGoesFirst = 0;
        } else if (player->Speed > enemy.Speed) {
          playerGoesFirst = 1;
        } else if (enemy.Speed > player->Speed) {
          playerGoesFirst = 0;
        } else {
          playerGoesFirst = (rand() % 2 == 0);
        }

        int canPlayerClash = (playerSkillEffective != NULL) && 
                             (pType == 0 || pType == 4 || pType == 5) && !IsplayerUnableToAct;
        int canEnemyClash  = (enemySkillEffective != NULL) && 
                             (eType == 0 || eType == 4 || eType == 5) && !IsenemyUnableToAct;

  int willClash = (playerSkillEffective != NULL && enemySkillEffective != NULL) &&
                  playerSkillEffective->Clashable && 
                  enemySkillEffective->Clashable && 
                  canPlayerClash && canEnemyClash;

           if (!willClash) {

             if (pType == 0 || eType == 0) {

               // ถ้าเป็น Guard (Type 1) ให้ทอยโล่ทันที
               if (pType == 1) {
                 defensePhase(player, playerSkillEffective, &enemy, selectedEnemyPtr);
                 }

               if (eType == 1) {
                   defensePhase(&enemy, selectedEnemyPtr, player, playerSkillEffective);
               }

      if (playerGoesFirst == 1) {
        if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0)) {
          attackPhase(player, playerSkillEffective, playerTempOffense,
                      playerTempDefense, &enemy, enemySkillEffective,
                      enemyTempOffense, enemyTempDefense,
                      playerSkillEffective->Coins, 0, 0);
        }
        if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0 || enemySkillEffective->skillType == 3)) {
          attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
                      enemyTempDefense, player, playerSkillEffective,
                      playerTempOffense, playerTempDefense,
                      enemySkillEffective->Coins, 0, 0);
        }
      } else if (playerGoesFirst == 0) {
        if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0)) {
          attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
                      enemyTempDefense, player, playerSkillEffective,
                      playerTempOffense, playerTempDefense,
                      enemySkillEffective->Coins, 0, 0);
        }
        if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0 || playerSkillEffective->skillType == 3)) {
          attackPhase(player, playerSkillEffective, playerTempOffense,
                      playerTempDefense, &enemy, enemySkillEffective,
                      enemyTempOffense, enemyTempDefense,
                      playerSkillEffective->Coins, 0, 0);
        }
      }
             }

    } else if (IsplayerUnableToAct && (!IsenemyUnableToAct && enemySkillEffective != NULL && enemySkillEffective->skillType == 0)) {
      attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
                  enemyTempDefense, player, playerSkillEffective,
                  playerTempOffense, playerTempDefense,
                  enemySkillEffective->Coins, 0, 0);

    } else if ((!IsplayerUnableToAct && playerSkillEffective != NULL && playerSkillEffective->skillType == 0) && IsenemyUnableToAct) {
      attackPhase(player, playerSkillEffective, playerTempOffense,
                  playerTempDefense, &enemy, enemySkillEffective,
                  enemyTempOffense, enemyTempDefense,
                  playerSkillEffective->Coins, 0, 0);

    } else if (playerSkillEffective->skillType == 0 || enemySkillEffective->skillType == 0) {
      ClashResult clash =
          clashPhase(player, playerSkillEffective, playerTempOffense,
                     playerTempDefense, &enemy, enemySkillEffective,
                     enemyTempOffense, enemyTempDefense, player, 0, 0);

      if (clash.winner == 1) {
        if (clash.playerskillUsed->skillType == 4) {
          // --- [Clashable Guard Win Effect] ---
          enemy.Tremor[4] += clash.playerFinalPower;
          printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                  player->name, player->name, enemy.name, clash.playerFinalPower);

          sleep(1);
          if (enemy.Tremor[4] > 100 && enemy.Stagger <= 0) {
            enemy.Stagger += 2;
            printf("\n%s Staggered for one turn\n", enemy.name);
            sleep(1);
             enemy.Tremor[4] = 0;
          }

        } else {
          attackPhase(
              player, clash.playerskillUsed,
              clash.playerTempOffense, clash.playerTempDefense,
              &enemy, clash.enemyskillUsed,
              clash.enemyTempOffense, clash.enemyTempDefense,
              (clash.playerskillUsed->Unbreakable > 0)
                  ? ((clash.playerCoins > clash.playerskillUsed->Unbreakable)
                         ? clash.playerCoins : clash.playerskillUsed->Unbreakable)
                  : clash.playerCoins,
              clash.playerUnbreakableLost, clash.ClashCount);
        }
      } else if (clash.winner == 2) {
        if (clash.enemyskillUsed->skillType == 4) {
          // --- [Clashable Guard Win Effect] ---
          player->Tremor[4] += clash.enemyFinalPower;
          printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                  enemy.name, enemy.name, player->name, clash.enemyFinalPower);
          sleep(1);
          if (player->Tremor[4] > 50 && player->Stagger <= 0) {
            player->Stagger += 2;
            printf("\n%s Staggered for one turn\n", player->name);
            sleep(1);
            player->Tremor[4] = 0;
          }
        } else {
          attackPhase(
              &enemy, clash.enemyskillUsed,
              clash.enemyTempOffense, clash.enemyTempDefense,
              player, clash.playerskillUsed,
              clash.playerTempOffense, clash.playerTempDefense,
              (clash.enemyskillUsed->Unbreakable > 0)
                  ? ((clash.enemyCoins > clash.enemyskillUsed->Unbreakable)
                         ? clash.enemyCoins : clash.enemyskillUsed->Unbreakable)
                  : clash.enemyCoins,
              clash.enemyUnbreakableLost, clash.ClashCount);
        }
      }
    }
      // -------------------------------------------------------
      // คืนชื่อ knight กลับก่อน handleTurnEnd
      // เพื่อให้ output แสดงว่า "A Knight" ไม่ใช่ player->name
      // handleTurnEnd จะทำ passive ของ player ถูกต้อง
      // เพราะ player pointer ตัวจริงยังเป็น player อยู่
      // -------------------------------------------------------

    printf("\n--- Turn End ---\n");

    handleTurnEnd(player, &enemy, playerSkillEffective, enemySkillEffective); // รันของ Player
    handleTurnEnd(&enemy, player, enemySkillEffective, playerSkillEffective); // รันของ Knight

    // ... (สุ่มสกิลใหม่สำหรับเทิร์นหน้า) ...
    TurnCount++;
  }

  // Player ตายใน knight phase
  if (player->HP <= 0) return;

  clearDebuffsOnDeath(boss, player);

    // ============================================================
    // PHASE 2 — Knight ตาย → Event → Boss ออกมา
    // ============================================================
  // คืนชื่อเดิมให้ Player ทันที เพื่อไปสู้กับ Boss ตัวจริงแบบไม่มี Tag
  player->name = realID;
  player->ID = realID;

    printf("\nThe shadow dissipates into thin air...\n");
    sleep(2);

    int sanityLoss = 5 * (TurnCount - 1);
    if (sanityLoss > 45) sanityLoss = 45;
    updateSanity(player, -sanityLoss);
    printf("\n%s lose %d Sanity (Max 45) from Passive 'Closing of the Banquet' (%d)\n",
           player->name ,sanityLoss, player->Sanity);
    sleep(1);

    printf("\nThe King in Binds descends from his throne...\n");
    sleep(1);
    printf("\nGrand Welcome...\n");
    sleep(2);

    getSkills(boss, enemySkill1, enemySkill2, enemySkill3,
              *enemyLastUnused, boss->numSkills);

    // ============================================================
    // PHASE 3 — Boss phase
    // ============================================================

  int GrandWelcome = 0;

    while (player->HP > 0 && boss->HP > 0) {

        printf("\n--- Turn %d (King in Binds) ---\n", TurnCount);

      int eIdx = -1;

      if (GrandWelcome == 0) {
        GrandWelcome = 1;
          runCombatEvent(player, boss, &kingMidEvent, playerSkill1, playerSkill2, playerSkill3);
        if (KingDmgBonus > 0.0f) {
          for (int i = 0; i < player->numSkills; i++)
              player->skills[i].DmgMutiplier += KingDmgBonus;
          KingDmgBonus = 0.0f;
        }

      }

      if (KingClashBonus > 0) {
        player->ClashPowerUp[0] += KingClashBonus;
      }

      // ใช้ฟังก์ชันใหม่ที่เราสร้างเพื่อเลือกว่าจะ "โจมตี" หรือ "ป้องกัน" ตามค่า Copies
      int decision = pickEnemyActionWeighted(boss, *enemySkill1, *enemySkill2);

      SkillStats *enemySkillEffective = NULL;

      if (decision >= 100) {
          // --- กรณีเลือก Defense Skill ---
          int defIdx = decision - 100;
          enemySkillEffective = &boss->defenseSkill[defIdx];
        eIdx = *enemySkill1;

      } else {
          // --- กรณีเลือกท่าโจมตีปกติ ---
          eIdx = (decision == 1 ? *enemySkill1 : *enemySkill2);
          *enemyLastUnused = (eIdx == *enemySkill1 ? *enemySkill2 : *enemySkill1);
          enemySkillEffective = &boss->skills[eIdx];

      }

      // For enemy
      getSkills(boss, enemySkill1, enemySkill2, enemySkill3,
      *enemyLastUnused, boss->numSkills);

      handleTurnStart(player, boss, &enemySkillEffective, playerSkill1, playerSkill2, playerSkill3, enemySkill1, enemySkill2, enemySkill3);

      int IsplayerUnableToAct = isPanicked(player) || isStaggered(player);
      int IsenemyUnableToAct  = isStaggered(boss);

      if (IsplayerUnableToAct) {
        if (isStaggered(player)) printf("\n%s is STAGGERED and cannot act!\n", player->name);
        else if (isPanicked(player)) printf("\n%s is in PANIC and cannot act!\n", player->name);

        // Lose Envy Resonance
        if (isId(player->ID, "The Middle Little Brother Sinclair") == 0) {

            player->Passive = 0;

          printf("\n%s loses all Envy Resonance\n", player->name);

        }

      }

      if (IsenemyUnableToAct) {
        if (isStaggered(boss)) printf("\n%s is STAGGERED and cannot act!\n", boss->name);
        else if (isPanicked(boss)) printf("\n%s is in PANIC and cannot act!\n", boss->name);

      }


        // แสดง HP
      printf("\nCurrent HP:\n");

      // แสดงผลของ Player
      if (player->Stagger > 0) printf("[Stagger] ");
      printf("%s = %.2f / %.2f", player->name, player->HP, player->MAX_HP);
      if (player->Shield > 0 || player->TempShield > 0) printf(" (Shield %.2f)", player->Shield + player->TempShield);
      printf("\n");

      // แสดงผลของ Enemy
      if (boss->Stagger > 0) printf("[Stagger] ");
      printf("%s = %.2f / %.2f", boss->name, boss->HP, boss->MAX_HP);
      if (boss->Shield > 0 || boss->TempShield > 0) printf(" (Shield %.2f)", boss->Shield + boss->TempShield);
      printf("\n");

        if (player->hasSanity)
            printf("[Sanity] %s: %d (%s)\n",
                   player->name, player->Sanity, getSanityStatus(player));

        // Apply player passives + enemy turn-start forced-skill logic
          handleBeforeFight(player, boss, &enemySkillEffective, *playerSkill1, *playerSkill2, *enemySkill1, *enemySkill2);
          handleBeforeFight(boss, player, &playerSkillEffective, *enemySkill1, *enemySkill2, *playerSkill1, *playerSkill2);

      // Player picks one skill (only if can act)
      int playerSkillIndex;

      int playerTempOffense = 0, playerTempDefense = 0;
      int enemyTempOffense = 0, enemyTempDefense = 0;
      playerTempOffense += (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]);
      playerTempDefense += (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]);
      enemyTempOffense += (boss->OffenseLevelUp[0] - boss->OffenseLevelDown[0]);
      enemyTempDefense += (boss->DefenseLevelUp[0] - boss->DefenseLevelDown[0]);

          int ECoinBoost = 0;
          if (enemySkillEffective->CoinPower >= 0) {
                ECoinBoost = boss->PlusCoinPowerBoost[0] - boss->PlusCoinPowerDrop[0];
          } else {
                ECoinBoost = boss->MinusCoinPowerDrop[0] - boss->PlusCoinPowerBoost[0];
          }

          if (!IsenemyUnableToAct) {
          if (enemySkillEffective->Unbreakable > 0 && (enemySkillEffective->Clashable || enemySkillEffective->skillType != 0)) {
            printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d "
                   "Defense %d Unbreakable %d)\n",
              getSkillTypeName(enemySkillEffective->skillType),
                   enemySkillEffective->name,
                   enemySkillEffective->BasePower + (boss->BasePowerUp[0] - boss->BasePowerDown[0]),
                   enemySkillEffective->CoinPower + ECoinBoost,
                   enemySkillEffective->Coins,
                   enemySkillEffective->Offense + (boss->OffenseLevelUp[0] - boss->OffenseLevelDown[0]),
                   enemySkillEffective->Defense + (boss->DefenseLevelUp[0] - boss->DefenseLevelDown[0]),
                   enemySkillEffective->Unbreakable);
          } else if (enemySkillEffective->Unbreakable <= 0 && (enemySkillEffective->Clashable || enemySkillEffective->skillType != 0)) {
            printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d "
                   "Defense %d Breakable)\n",
              getSkillTypeName(enemySkillEffective->skillType),
                   enemySkillEffective->name,
              enemySkillEffective->BasePower + (boss->BasePowerUp[0] - boss->BasePowerDown[0]),
                 enemySkillEffective->CoinPower + ECoinBoost,
                 enemySkillEffective->Coins,
                 enemySkillEffective->Offense + (boss->OffenseLevelUp[0] - boss->OffenseLevelDown[0]),
                   enemySkillEffective->Defense + (boss->DefenseLevelUp[0] - boss->DefenseLevelDown[0]));
          } else if (enemySkillEffective->Unbreakable > 0 && !enemySkillEffective->Clashable && enemySkillEffective->skillType == 0) {
            printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d "
                   "Defense %d Unbreakable %d)\n",
              getSkillTypeName(enemySkillEffective->skillType),
                   enemySkillEffective->name,
              enemySkillEffective->BasePower + (boss->BasePowerUp[0] - boss->BasePowerDown[0]),
                 enemySkillEffective->CoinPower + ECoinBoost,
                 enemySkillEffective->Coins,
                 enemySkillEffective->Offense + (boss->OffenseLevelUp[0] - boss->OffenseLevelDown[0]),
                 enemySkillEffective->Defense + (boss->DefenseLevelUp[0] - boss->DefenseLevelDown[0]),
                   enemySkillEffective->Unbreakable);
          } else if (enemySkillEffective->Unbreakable <= 0 && (!enemySkillEffective->Clashable && enemySkillEffective->skillType == 0)) {
            printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d "
                   "Defense %d Breakable)\n",
              getSkillTypeName(enemySkillEffective->skillType),
                   enemySkillEffective->name,
                    enemySkillEffective->BasePower + (boss->BasePowerUp[0] - boss->BasePowerDown[0]),
                 enemySkillEffective->CoinPower + ECoinBoost,
                 enemySkillEffective->Coins,
                 enemySkillEffective->Offense + (boss->OffenseLevelUp[0] - boss->OffenseLevelDown[0]),
                 enemySkillEffective->Defense + (boss->DefenseLevelUp[0] - boss->DefenseLevelDown[0]));
          }
          }

          int PCoinBoost1 = 0;
          if (player->skills[*playerSkill1].CoinPower >= 0) {
                  PCoinBoost1 = player->PlusCoinPowerBoost[0] - player->PlusCoinPowerDrop[0];
          } else {
                  PCoinBoost1 = player->MinusCoinPowerDrop[0] - player->PlusCoinPowerBoost[0];
          }

          int PCoinBoost2 = 0;
          if (player->skills[*playerSkill2].CoinPower >= 0) {
                  PCoinBoost2 = player->PlusCoinPowerBoost[0] - player->PlusCoinPowerDrop[0];
          } else {
                  PCoinBoost2 = player->MinusCoinPowerDrop[0] - player->PlusCoinPowerBoost[0];
          }

          int PCoinBoost3 = 0;
          if (player->skills[*playerSkill3].CoinPower >= 0) {
                  PCoinBoost3 = player->PlusCoinPowerBoost[0] - player->PlusCoinPowerDrop[0];
          } else {
                  PCoinBoost3 = player->MinusCoinPowerDrop[0] - player->PlusCoinPowerBoost[0];
          }

          if (!IsplayerUnableToAct) {
          printf("\nDashboard Skills:\n");

            if (player->skills[*playerSkill1].Unbreakable > 0 && player->skills[*playerSkill1].Clashable) {
                printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                      getSkillTypeName(player->skills[*playerSkill1].skillType),
                       player->skills[*playerSkill1].name,
                       player->skills[*playerSkill1].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill1].CoinPower + PCoinBoost1,
                       player->skills[*playerSkill1].Coins,
                       player->skills[*playerSkill1].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill1].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                       player->skills[*playerSkill1].Unbreakable);
            } else if (player->skills[*playerSkill1].Unbreakable <= 0 && player->skills[*playerSkill1].Clashable) {
                printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill1].skillType),
                       player->skills[*playerSkill1].name,
                       player->skills[*playerSkill1].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill1].CoinPower + PCoinBoost1,
                       player->skills[*playerSkill1].Coins,
                       player->skills[*playerSkill1].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill1].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
            } else if (player->skills[*playerSkill1].Unbreakable > 0 && !player->skills[*playerSkill1].Clashable) {
                printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill1].skillType),
                       player->skills[*playerSkill1].name,
                       player->skills[*playerSkill1].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill1].CoinPower + PCoinBoost1,
                       player->skills[*playerSkill1].Coins,
                       player->skills[*playerSkill1].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill1].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                       player->skills[*playerSkill1].Unbreakable);
            } else {
                printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill1].skillType),
                       player->skills[*playerSkill1].name,
                       player->skills[*playerSkill1].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill1].CoinPower + PCoinBoost1,
                       player->skills[*playerSkill1].Coins,
                       player->skills[*playerSkill1].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill1].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
            }

            if (player->skills[*playerSkill2].Unbreakable > 0 && player->skills[*playerSkill2].Clashable) {
                printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill2].skillType),
                       player->skills[*playerSkill2].name,
                       player->skills[*playerSkill2].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill2].CoinPower + PCoinBoost2,
                       player->skills[*playerSkill2].Coins,
                       player->skills[*playerSkill2].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill2].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                       player->skills[*playerSkill2].Unbreakable);
            } else if (player->skills[*playerSkill2].Unbreakable <= 0 && player->skills[*playerSkill2].Clashable) {
                printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill2].skillType),
                       player->skills[*playerSkill2].name,
                       player->skills[*playerSkill2].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill2].CoinPower + PCoinBoost2,
                       player->skills[*playerSkill2].Coins,
                       player->skills[*playerSkill2].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill2].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
            } else if (player->skills[*playerSkill2].Unbreakable > 0 && !player->skills[*playerSkill2].Clashable) {
                printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill2].skillType),
                       player->skills[*playerSkill2].name,
                       player->skills[*playerSkill2].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill2].CoinPower + PCoinBoost2,
                       player->skills[*playerSkill2].Coins,
                       player->skills[*playerSkill2].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill2].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                       player->skills[*playerSkill2].Unbreakable);
            } else {
                printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill2].skillType),
                       player->skills[*playerSkill2].name,
                       player->skills[*playerSkill2].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill2].CoinPower + PCoinBoost2,
                       player->skills[*playerSkill2].Coins,
                       player->skills[*playerSkill2].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill2].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
            }

          // Next Skill
            if (player->skills[*playerSkill3].Unbreakable > 0 && player->skills[*playerSkill3].Clashable) {
                printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill3].skillType),
                       player->skills[*playerSkill3].name,
                       player->skills[*playerSkill3].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill3].CoinPower + PCoinBoost3,
                       player->skills[*playerSkill3].Coins,
                       player->skills[*playerSkill3].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill3].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                       player->skills[*playerSkill3].Unbreakable);
            } else if (player->skills[*playerSkill3].Unbreakable <= 0 && player->skills[*playerSkill3].Clashable) {
                printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill3].skillType),
                       player->skills[*playerSkill3].name,
                       player->skills[*playerSkill3].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill3].CoinPower + PCoinBoost3,
                       player->skills[*playerSkill3].Coins,
                       player->skills[*playerSkill3].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill3].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
            } else if (player->skills[*playerSkill3].Unbreakable > 0 && !player->skills[*playerSkill3].Clashable) {
                printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill3].skillType),
                       player->skills[*playerSkill3].name,
                       player->skills[*playerSkill3].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill3].CoinPower + PCoinBoost3,
                       player->skills[*playerSkill3].Coins,
                       player->skills[*playerSkill3].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill3].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                       player->skills[*playerSkill3].Unbreakable);
            } else {
                printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill3].skillType),
                       player->skills[*playerSkill3].name,
                       player->skills[*playerSkill3].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                       player->skills[*playerSkill3].CoinPower + PCoinBoost3,
                       player->skills[*playerSkill3].Coins,
                       player->skills[*playerSkill3].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                       player->skills[*playerSkill3].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
            }


          // Defense Skill

          int PCoinBoostDef = 0;
          if (player->defenseSkill[playerDefenseSkill].CoinPower >= 0) {
                    PCoinBoostDef = player->PlusCoinPowerBoost[0] - player->PlusCoinPowerDrop[0];
          } else {
                    PCoinBoostDef = player->MinusCoinPowerDrop[0] - player->PlusCoinPowerBoost[0];
          }

          printf("\n");
          if (player->defenseSkill[playerDefenseSkill].Unbreakable > 0) {
            printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player->defenseSkill[playerDefenseSkill].skillType),
                   player->defenseSkill[playerDefenseSkill].name,
                   player->defenseSkill[playerDefenseSkill].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                   player->defenseSkill[playerDefenseSkill].CoinPower + PCoinBoostDef,
                   player->defenseSkill[playerDefenseSkill].Coins,
                   player->defenseSkill[playerDefenseSkill].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                   player->defenseSkill[playerDefenseSkill].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]),
                   player->defenseSkill[playerDefenseSkill].Unbreakable);
          } else if (player->defenseSkill[playerDefenseSkill].Unbreakable <= 0) {
            printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player->defenseSkill[playerDefenseSkill].skillType),
                   player->defenseSkill[playerDefenseSkill].name,
                   player->defenseSkill[playerDefenseSkill].BasePower + (player->BasePowerUp[0] - player->BasePowerDown[0]),
                   player->defenseSkill[playerDefenseSkill].CoinPower + PCoinBoostDef,
                   player->defenseSkill[playerDefenseSkill].Coins,
                   player->defenseSkill[playerDefenseSkill].Offense + (player->OffenseLevelUp[0] - player->OffenseLevelDown[0]),
                   player->defenseSkill[playerDefenseSkill].Defense + (player->DefenseLevelUp[0] - player->DefenseLevelDown[0]));
          }
          printf("\n");

            int choice;
            while (1) {
              printf("Choose skill (1-2, or 0 to Use Defense instead of Skill 1): ");
              if (scanf("%d", &choice) == 1 && (choice == 1 || choice == 2 || choice == 0))
                break;
              while (getchar() != '\n')
                ;
            }

          if (choice == 0) {
            // --- กรณีเลือก Guard (แทนที่ Skill 1) ---
            playerSkillEffective = &player->defenseSkill[0]; // ชี้ไปที่สกิลป้องกัน

            // สำคัญ: ตั้งค่าให้ระบบมองว่าเรา "ใช้" playerSkill1 ไปแล้ว
            playerSkillIndex = *playerSkill1; // (เพื่อให้ระบบหลังบ้านดึงชื่อหรือตำแหน่งมาใช้อ้างอิงได้)
            playerLastUnused = playerSkill2; // สกิลช่อง 2 จะถูกเก็บไว้

            // สุ่มสกิลใหม่มาแทนที่ช่องที่ 1 (และเลื่อนช่อง 3 มาช่อง 2 ตามระบบ getSkills ของคุณ)
            getSkills(player, playerSkill1, playerSkill2, playerSkill3, *playerLastUnused, player->numSkills);

          } else if (choice == 1 || choice == 2) {
            // --- กรณีเลือกสกิล 1 หรือ 2 ปกติ ---
          playerSkillIndex = (choice == 1 ? *playerSkill1 : *playerSkill2);
          playerLastUnused = (choice == 1 ? playerSkill2 : playerSkill1);
            playerSkillEffective = &player->skills[playerSkillIndex]; // ชี้ไปที่สกิลโจมตี

          // Roll new skill to replace used one
            getSkills(player, playerSkill1, playerSkill2, playerSkill3, *playerLastUnused, player->numSkills);

          } else {
          // Player can't act, just pick a random skill for defensive purposes
          playerSkillIndex = *playerSkill1;
          }
        }


      // --- คำนวณค่าสำหรับ Player ---
      if (IsplayerUnableToAct) {
          SkillStats dummy = player->defenseSkill[0]; // โคลนค่าจากท่าป้องกัน 0
          getEffectiveSkill(player, &enemy, &dummy, &playerTempOffense, &playerTempDefense);
          playerSkillEffective = NULL; // บังคับเป็น NULL ตามที่ต้องการ
      } else {
          playerSkillEffective = getEffectiveSkill(player, boss, playerSkillEffective,
                                                  &playerTempOffense, &playerTempDefense);
      }

      // --- คำนวณค่าสำหรับ Enemy ---
      if (IsenemyUnableToAct) {
          SkillStats dummyE = boss->defenseSkill[0]; // โคลนท่าป้องกันศัตรู
          getEffectiveSkill(boss, player, &dummyE, &enemyTempOffense, &enemyTempDefense);
          enemySkillEffective = NULL; // บังคับเป็น NULL
      } else {
          enemySkillEffective = getEffectiveSkill(boss, player, enemySkillEffective,
                                                  &enemyTempOffense, &enemyTempDefense);
      }


      int playerGoesFirst = 0;

          int pType = (playerSkillEffective != NULL) ? playerSkillEffective->skillType : -1;
          int eType = (enemySkillEffective != NULL) ? enemySkillEffective->skillType : -1;

          if (eType == 3) { 
            // ถ้าศัตรูใช้ Counter (Type 3) เราต้องตีก่อนเสมอ
            playerGoesFirst = 1;
          } else if (pType == 3) { 
            // ถ้าเราใช้ Counter (Type 3) ศัตรูต้องตีก่อนเสมอ
            playerGoesFirst = 0;
          } else if (player->Speed > enemy.Speed) {
            playerGoesFirst = 1;
          } else if (boss->Speed > player->Speed) {
            playerGoesFirst = 0;
          } else {
            playerGoesFirst = (rand() % 2 == 0);
          }

        // Combat — เหมือน main() ทุกอย่าง
     if (IsplayerUnableToAct && (!IsenemyUnableToAct && enemySkillEffective != NULL && enemySkillEffective->skillType == 0)) {

          attackPhase(boss, enemySkillEffective, enemyTempOffense,
                      enemyTempDefense, player, playerSkillEffective,
                      playerTempOffense, playerTempDefense,
                      enemySkillEffective->Coins, 0, 0);

      } else if ((!IsplayerUnableToAct && playerSkillEffective != NULL && playerSkillEffective->skillType == 0) && IsenemyUnableToAct) {

          attackPhase(player, playerSkillEffective, playerTempOffense,
                      playerTempDefense, boss, enemySkillEffective,
                      enemyTempOffense, enemyTempDefense,
                      playerSkillEffective->Coins, 0, 0);

      } else if (!IsenemyUnableToAct && !IsplayerUnableToAct) {

       int canPlayerClash = (playerSkillEffective != NULL) && 
                            (pType == 0 || pType == 4 || pType == 5) && !IsplayerUnableToAct;
       int canEnemyClash  = (enemySkillEffective != NULL) && 
                            (eType == 0 || eType == 4 || eType == 5) && !IsenemyUnableToAct;

       int willClash = (playerSkillEffective != NULL && enemySkillEffective != NULL) &&
                       playerSkillEffective->Clashable && 
                       enemySkillEffective->Clashable && 
                       canPlayerClash && canEnemyClash;

           if (!willClash) {

             if (pType == 0 || eType == 0) {

               // ถ้าเป็น Guard (Type 1) ให้ทอยโล่ทันที
               if (pType == 1) {
                 defensePhase(player, playerSkillEffective, &enemy, enemySkillEffective);
                 }

               if (eType == 1) {
                   defensePhase(&enemy, enemySkillEffective, player, playerSkillEffective);
               }

              if (playerGoesFirst == 1) {
                if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0)) {
                attackPhase(player, playerSkillEffective, playerTempOffense,
                            playerTempDefense, boss, enemySkillEffective,
                            enemyTempOffense, enemyTempDefense,
                            playerSkillEffective->Coins, 0, 0);
                }
                if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0 || enemySkillEffective->skillType == 3)) {
                attackPhase(boss, enemySkillEffective, enemyTempOffense,
                            enemyTempDefense, player, playerSkillEffective,
                            playerTempOffense, playerTempDefense,
                            enemySkillEffective->Coins, 0, 0);
                }
            } else if (playerGoesFirst == 0) {
                  if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0)) {
                attackPhase(boss, enemySkillEffective, enemyTempOffense,
                            enemyTempDefense, player, playerSkillEffective,
                            playerTempOffense, playerTempDefense,
                            enemySkillEffective->Coins, 0, 0);
                  }
                  if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0 || playerSkillEffective->skillType == 3)) {
                attackPhase(player, playerSkillEffective, playerTempOffense,
                            playerTempDefense, boss, enemySkillEffective,
                            enemyTempOffense, enemyTempDefense,
                            playerSkillEffective->Coins, 0, 0);
                  }
            }
             }

        } else if (playerSkillEffective->skillType == 0 || enemySkillEffective->skillType == 0) {

            ClashResult clash =
                clashPhase(player, playerSkillEffective, playerTempOffense,
                           playerTempDefense, boss, enemySkillEffective,
                           enemyTempOffense, enemyTempDefense, player, 0, 0);

            if (clash.winner == 1) {
              if (clash.playerskillUsed->skillType == 4) {
                // --- [Clashable Guard Win Effect] ---
                boss->Tremor[4] += clash.playerFinalPower;
                printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                        player->name, player->name, boss->name, clash.playerFinalPower);
                sleep(1);
                if (boss->Tremor[4] > 50 && boss->Stagger <= 0) {
                  boss->Stagger += 2;
                  printf("\n%s Staggered for one turn\n", boss->name);
                  sleep(1);
                  boss->Tremor[4] = 0;
                }
              } else {
                attackPhase(
                    player, clash.playerskillUsed,
                    clash.playerTempOffense, clash.playerTempDefense,
                    boss, clash.enemyskillUsed,
                    clash.enemyTempOffense, clash.enemyTempDefense,
                    (clash.playerskillUsed->Unbreakable > 0)
                        ? ((clash.playerCoins > clash.playerskillUsed->Unbreakable)
                               ? clash.playerCoins : clash.playerskillUsed->Unbreakable)
                        : clash.playerCoins,
                    clash.playerUnbreakableLost, clash.ClashCount);
              }
            } else if (clash.winner == 2) {
              if (clash.enemyskillUsed->skillType == 4) {
                // --- [Clashable Guard Win Effect] ---
                player->Tremor[4] += clash.enemyFinalPower;
                printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                        boss->name, boss->name, player->name, clash.enemyFinalPower);
                sleep(1);
                if (player->Tremor[4] > 50 && player->Stagger <= 0) {
                  player->Stagger += 2;
                  printf("\n%s Staggered for one turn\n", player->name);
                  sleep(1);
                  player->Tremor[4] = 0;
                }
              } else {
                attackPhase(
                    boss, clash.enemyskillUsed,
                    clash.enemyTempOffense, clash.enemyTempDefense,
                    player, clash.playerskillUsed,
                    clash.playerTempOffense, clash.playerTempDefense,
                    (clash.enemyskillUsed->Unbreakable > 0)
                        ? ((clash.enemyCoins > clash.enemyskillUsed->Unbreakable)
                               ? clash.enemyCoins : clash.enemyskillUsed->Unbreakable)
                        : clash.enemyCoins,
                    clash.enemyUnbreakableLost, clash.ClashCount);
              }
            }
        }

      }

      printf("\n--- Turn End ---\n");

        handleTurnEnd(player, boss, playerSkillEffective, enemySkillEffective);

        handleTurnEnd(boss, player, enemySkillEffective, playerSkillEffective);

        TurnCount++;
    }
  }



















int main() {
  srand(time(NULL));

  // --- ส่วนของ Identity ---
  const char *identity[] = {
      "Meursault:The Thumb",  
      "Meursault:Blade Lineage Mentor",
      "Heathcliff:Wild Hunt", 
      "Hong lu:The Lord of Hongyuan",
      "Yi sang:Fell Bullet",  
      "Don Quixote:The Manager of La Manchaland", 
      "Lobotomy E.G.O::Solemn Lament Yi Sang", 
      "Dawn Office Fixer Sinclair", 
      "Gregor:Firefist", 
      "Heishou Pack - You Branch Adept Heathcliff", 
      "The Middle Little Brother Sinclair", 
      "The House of Spiders: The Index Nursefather Yi Sang", 
      "The One Who Grips Faust",
      "The House of Spiders: The Ring Nursefather Hong Lu",
      "The House of Spiders: The Thumb Nursefather Rodion",
  };
  // คำนวณจำนวนรายการที่มีทั้งหมดใน Array
  int numIdentities = sizeof(identity) / sizeof(identity[0]);

  // --- ส่วนของ Enemy ---
  const char *enemyNames[] = {
      "Bandit",
      "Lei heng",
      "Erlking Heathcliff",
      "Sukuna:King of Curse",
      "Don Quixote",
      "Jia Qiu",
      "King in Binds",
      "The Middle Nursefather - Matthias",
      "Fixer grade 9?"
  };
  int numEnemies = sizeof(enemyNames) / sizeof(enemyNames[0]);

  int selected_identity = -1, selected_enemy = -1;

  int targetIndex = -1; // for secret character

  printf("Limbus Company...\nIdentity:\n");
  for (int i = 0; i < numIdentities; i++)
      printf("%d. %s\n", i + 1, identity[i]);

  Character tempPlayer, tempEnemy; // temporary holders for preview





  // -------------------- Identity Selection ---------------------
  while (1) {
      printf("\nSelect identity (1-%d): ", numIdentities); // add for secret character

    if (scanf("%d", &selected_identity) == 1) {

      // --- Secret Character System (Press 0) ---
      if (selected_identity == 0) {
          printf("\n\x1b[1;30m[???]: The mirror darkens... the light around you begins to fade...\x1b[0m\n");
          usleep(1500000);
          printf("\x1b[1;30m[???]: 'If you desire the hidden truth... name the entity you seek.'\x1b[0m\n");
          printf("\x1b[1;30m[???]: Secret Code : \x1b[0m");

          char secret_code[50];
        if (scanf("%49s", secret_code) != 1) {
            while (getchar() != '\n'); // ล้างบัฟเฟอร์ถ้าอ่านค่าพลาด
            continue; 
        }

          if (strcasecmp(secret_code, "Muga") == 0) {
              printf("\n\x1b[1;31m[!!!]: The mirror shatters! The essence of 'No-self' seeps through...\x1b[0m\n");
              sleep(1);
              selected_identity = 100; // Internal flag for Muga
          } 
          else if (strcasecmp(secret_code, "Binah") == 0 || strcasecmp(secret_code, "Arbiter") == 0) {
              printf("\n\x1b[1;33m[!!!]: A heavy, slow poison fills the air... The Arbiter has arrived.\x1b[0m\n");
              sleep(1);
              selected_identity = 89; // Internal flag for Binah
          }
          else {
              printf("\n[...]: The mirror returns to normal... nothing happened.\n");
              sleep(1);
              continue; // Back to normal selection
          }
      }

    }

      // Map selection to setup index
      if (selected_identity == 100) targetIndex = 99;      // Muga
      else if (selected_identity == 89) targetIndex = 88; // Binah
    else targetIndex = -1;

        if ((selected_identity >= 1 && selected_identity <= numIdentities) || targetIndex > 0) {

          setupCharacters(&tempPlayer, &tempEnemy, selected_identity - 1, 0);

          printf("\n--- Identity Info ---\n");
          printf("Name: %s\n", tempPlayer.name);
          printf("HP: %.0f / %.0f\n", tempPlayer.HP, tempPlayer.MAX_HP);
          printf("Speed: %d ~ %d\n", tempPlayer.MinSpeed, tempPlayer.MaxSpeed);

        if (selected_identity - 1 == 0) {
          //Taunt
          printf("\"Think it over three times, hard, before talking to me. I have ripped out enough tongues today.\"\n\n");

          //Story / Lore
          printf("In a brutal world ruled by syndicate syndicates, 'The Thumb' is a notorious mafia empire governed by absolute hierarchy and strict etiquette, where even a slight slip of the tongue results in death. Serving as a high-ranking 'Capo' of its Eastern Branch, this silent enforcer acts as the perfect weapon of the organization. Known in the lawless Backstreets as the 'Chachihu' (The Winged Tiger) and guided by the fateful Tiantui Star, he wields a specialized gunblade loaded with incendiary rounds to ruthlessly execute rule-breakers, burning away any trace of defiance to maintain absolute order.\n\n");


          //Description
          printf("A powerful character that focus on unbreakable Skills and dealing damage as much as possible, which comes with powerful skills that great for clashing, but clashing can become weak when it come in long term\n\n");

          //Passive
          printf("Passive Skills:\n");
           printf("\n 1. Tiantui Star's Blade [天退星刀]\n Always Active: begin Encounters with Tigermark Round amount based on enemy (Min 12)\n"

             "\n When flipping a Coin that consumes Tigermark Round or Savage Tigermark Round: while not having those 'Unique Ammo' does not cancel this unit's attacks, the Coin's On Hit \"inflict Burn Stack\" and \"inflict Burn Count\" effects do not activate\n");
         printf("\n 2. Chachihu [揷翅虎]\n If this unit equipped Defense Skills for the first time in this Encounter, or if this unit spent all of 'Tigermark Round', or if this unit is Staggered, or HP at 65%% or less HP, Reload 'Savage Tigermark Round' amount based on enemy (Min 8) and activate 'Tiantui Star [天退星]'\n");
          printf("\n 3. Tiantui Star [天退星]\n"
            " - Max & Min Speed +1\n"
            " - If this unit's Speed is faster than the target's by 3 or more, deal +(Speed difference x 2.5)%% damage (Max 20%%)\n"
            " - Inflict +1 more Tremor Stack and Tremor Count with this unit's Skills\n"
            " - At (Savage Tigermark Round Gained)+ (sum of Tigermark Round and Savage Tigermark Round spent) activate 'Shin (心) - Tiantui Star [天退星]' instead\n");
          printf("\n 4. Shin (心) - Tiantui Star [天退星]\n"
            " - Max & Min Speed +3\n"
            " - If this unit's Speed is faster than the target's by 3 or more, deal +(Speed difference x 5)%% damage (Max 40%%)\n"
            " - Inflict +2 more Tremor Stack and Tremor Count with this unit's Skills\n");
         printf("\n 5. Tigermark Round\n 'Unique Ammo', Skill Coins that spend Tigermark Round gain +1 Power and deal +10%% damage (activates only as long as the Coin has Rounds left to spend)\n" 
            " - At 1+ Tigermark Round and 3+ Tigermark Round spent, convert all Coins of 'Tanglecleaver' into Unbreakable Coins\n");
          printf("\n 6. Savage Tigermark Round\n 'Unique Ammo', Skill Coins that spend Savage Tigermark Round gain +2 Power and deal +30%% damage, and inflict +2 more Burn Stack and Burn Count On Hit (activates only as long as the Coin has Rounds left to spend)\n"
           " - At 1+ Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins and Replace 'Tanglecleaver [快刀亂麻]' with 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]'\n" 
          " - At 0 Savage Tigermark Round, convert all Coins of this unit's Attack Skills that spend 'Unique Ammo' to Unbreakable Coins and Gain 'Overheat'\n");
        printf("\n 7. Overheat\n Min & Max Speed +2, Attack Skills Lose (cumulative number of Tigermark Rounds & Savage Tigermark Rounds spent / (cumulative number of Tigermark Rounds & Savage Tigermark Rounds Gained/5)) Clash Power (Max 5); however, gain the following effects(cumulative):\n"
         " - (Savage Tigermark Round Gained)+ Rounds spent: Take 10%% less damage for every 10%% missing HP on self at Turn Start (max 50%%)\n"
         " - (Sum of Savage Tigermark Round + (Tigermark Rounds/2) Gained)+ Rounds spent: On Clash Lose, Unbreakable Coins of this unit's Attack Skills deal +(75 + missing HP percentage on self)%% damage (Max 150%%)\n"
          " - (Sum of Savage Tigermark Round + Tigermark Rounds Gained)+ Rounds spent: Deal +(HP percentage difference)%% damage against targets with higher remaining HP percentage than this unit (Max 50%%)\n");
          printf("\n 8. Burn\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (Turn End: When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), Turn End: Take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
           printf("\n 9. Tremor\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Trigger by 'Tremor Burst', Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP/4) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess. Turn End: Lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf("\n 10. Tremor - Scorch\n Change when Triggered by Amplitude Conversion\n"
            " - On Tremor Burst, Take (Tremor Stack and Burn Stack / 2) fixed Damage and Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP/4) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess.\n"
            " - Turn End: Lose 1 Count. When reach 0 Count, loses all Stack and Change back to Normal 'Tremor' too (Max 99 Stack/Count)\n");
          } 
        else if (selected_identity - 1 == 1) {
          //Taunt
          printf("\"We are not placing our stone here, then? Mm, then the tides drive us to resign.\"\n\n");

          //Story / Lore
            printf("In a forgotten corner of the City, the 'Blade Lineage' fights a desperate turf war to keep the ancient art of swordsmanship alive against modernized syndicates. Standing at the peak of this faction is a legendary teacher who embodies the calm before the storm, hidden beneath a traditional black bamboo hat. Driven by decades of flawless martial training, he wields a massive, single-edged Bong-gum, utilizing lethal counterattacks that can split an enemy in half with a single swing.\n\n");

          //Description
          printf("A character with powerful counter skill and great damage skills with anti-death passive.\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf("\n 1. Swordplay of the Homeland\n In Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. (Once per Encounter)\n");
          printf("\n 2. Yield My Flesh\n When Clashing with 'Yield My Flesh' does not effects by any Clash Power boost, When Clash loses with 'Yield My Flesh', Use Counter 'To Claim Their Bones' to attack back (Cannot be used only if this unit died first)\n");
          printf("\n 3. In Memoriam\n At 60%% or less HP, Apply 'Remembrance' buff on self, Gains buff at 5+ Poise Stack or 7+ Poise Stack on self (Buff base on each Skills)\n");
          printf("\n 4. Overthrow\n After got attacked, gain +1 Final Power next turn (Once per enemy's skill)\n");
          printf("\n 5. Poise\n When Gain by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When Attack: Have a (Stack x 5)%% chance to 'Critical Hit' (Boost damage by 20%%). If this unit Critical Hit, reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf("\n 6. Shadow-vested Bladesinger [着影揮刀]\n If this unit using 'To Claim Their Bones' immediately after activated 'Swordplay of the Homeland', 'To Claim Their Bones' gains following effect:\n"
            "  - Before Attack: gain +20 Poise Stack and +4 Poise Count\n"
            "  - Before Attack: Deal +(Poise Stack on self x 2)%% damage on Critical Hit (Max 100%%)\n"
            "  - Before Attack: Gain +(Poise Stack on self/5) Final Power (Max 10)\n"
            "  - This Skill is not affected by Paralyze\n"
            "  - On Hit with final coin deal +((Poise Stack on self + Poise Count on self) x 2)%% damage\n"
            "  - On Hit, heal HP by 10%% of damage dealt using the Skill\n");
              } 
        else if (selected_identity - 1 == 2) {
          //Taunt
          printf("\"What kindled this flame of wrath that burns within me...? ...No, it doesn't matter why it burns- What matters is that I am the ripping and tearing tempest that will bring about their ruin.\"\n\n");

          //Story / Lore
          printf("Drifting into the darkest timeline of despair, this version of Heathcliff went utterly mad after losing his beloved Catherine, transforming into a terrifying warlord of the dead. Bound to the tragic curse of Wuthering Heights, he roams the battlefield as a literal force of nature fueled by pure malice. Riding a headless wolf name Dullahan, he wields a massive greatsword crafted from his power, summoning a swarm of vengeful spirits to tear his foes apart.\n\n");

          //Description
          printf("A character with great buff and strong damage output with focus on Sanity but also come with less HP and Defense\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf("\n 1. Dullahan\n If this unit equipped Defense Skills and does not have 'Dullahan', gain 1 'Dullahan' next turn (Max 3), Offense +3, Defense -3, Raise Min & Max Speed by 1, If this unit equipped Defense Skills while having 'Dullahan', Turn End: loses all 'Dullahan'.\n"
            " - Turn End: When this unit's mounts 'Dullahan', gain 1 'Dullahan' (Max 3), lose 5 Sanity (if this unit's Sanity at -25 or less; however lose all 'Dullahan'), lose (15 - (Coffin / 2)) Sanity (Min 10).\n"
            " - When lost the Clash, At 15+ Sanity, use 'Lament, Mourn, and Despair' to continue the Clash (Once per Turn)\n");
          printf("\n 2. Call of the Erlking\n When at 50%% or less HP, or at -45 Sanity, Turn Start: if this unit at -45, does not 'Panic' and if this unit does not have 'Dullahan', gain 'Dullahan' and snap out from 'Stagger' and if this unit's Sanity at 0 or less, heal Sanity the further this unit's Sanity is from 0 (heal 2 additionalal Sanity for every missing Sanity; Max 50) (Once per Encounter)\n");
          printf("\n 3. Endless Lamentation\n When mounts 'Dullahan', and using Defense Skills at 15+ Sanity or using 'Requiem', use 'Lament, Mourn, and Despair' instead (if from Defense Skills use as Clashable Counter)\n");
           printf("\n 4. Coffin\n Gain by using 'Requiem' and 'Lament, Mourn, and Despair', gain 20%% Damage Up for every 3 Coffin, gain 1 Clash Power for every 5 Coffin (Max 10)\n");
          printf("\n 5. Impending Ruin\n Inflicted by certain Skill: This units -10%% chance flip Heads\n");
              } 
        else if (selected_identity - 1 == 3) {
          //Taunt
          printf("\"The Lord of Hongyuan marches to war.\"\n\n");

          //Story / Lore
          printf("Born into an unimaginably wealthy, ancient noble family, this carefree young master views the grim realities of the world as nothing more than an entertaining playground. Backed by elite tutors and inherited mystical bloodlines, he marches his ammy known as Heishou Pack, a group of organizations of augmented fighters that serve the upper classes of Hongyuan, his innate combat talent is nothing short of terrifying. Moving across the battlefield with effortless, aristocratic grace, he handles an ornate, flowing glaive, carving through armor with a cheerful smile, he slew all his enemy and state himself as 'Lord of Hongyuan'.\n\n");

          //Description
          printf("A low HP character with great buff, strong damage output, anti-death passive and followers that can help you\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf("\n 1. Embrace the Tarnished Blood and Exsanguinate Others For the Cause\n In Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn, after that apply 'Lordsguard' to the left Heishou Pack (Once per Encounter)\n");
          printf("\n 2. The Heishou Pack\n The Heishou Pack will heed you as their lord, you have 4 Heishou Packs(Mao, Si, Wu and You) as your follower, when using 'Answer Me, Heishou Packs', command one of the remaining Heishou Pack members to attack alongside this unit with 'I Carve the Path of a Lord' Skill; then gain 'Heishou Bolus Contamination [黑獸丸染]' after that 'Retreat' that Heishou Pack make them unable to use entire Encounter. If there is no Heishou Pack left use 'Lonesome Stand: Sacrifice to Claim The Garden [孑孑單身，捨生取园]' instead\n");
          printf("\n 3. The Heishou Lord\n After 'Embrace the Tarnished Blood and Exsanguinate Others For the Cause' activated, when this unit takes damage from any damage skills that can bring their HP down to 0, one of left Heishou Pack use 'Lordsguard' to defense this unit after that 'Retreat' that Heishou Pack make them unable to use entire Encounter, if the damage can't bring their HP down to 0, do not use 'Retreat'\n");
          printf("\n 4. Rupture\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When Hit: Take (Stack) fixed Damage. Then reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf("\n 5. Poise\n When Gain by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When Attack: Have a (Stack x 5)%% chance to 'Critical Hit' (Boost damage by 20%%). If this unit Critical Hit, reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf("\n 6. Heishou Bolus Contamination [黑獸丸染]\n Turn Start: Gain a buff from Stack"
            "\n - at 1+ Stack: Gain +1 Max Speed and +2%% Damage Up for every Stack"
            "\n - at 2+ Stack: Gain +1 Min Speed and +1 Defense Level Up for every Stack"
            "\n - at 3+ Stack: Gain +1 Offense Level Up for every Stack"
            "\n - at 4+ Stack: Gain +4 more Offense Level Up and +10%% Damage Up\n"

            "\n Empowered this unit based on which Heishou Pack that used"
            "\n  Heishou Pack - Mao: Gain +3 Offense"
            "\n  Heishou Pack - Wu: Gain +5 Defense"
            "\n  Heishou Pack - You: Gain +0.1 Damage Mutiplier"
            "\n  Heishou Pack - Si: Gain +1 Base Power");
              } 
        else if (selected_identity - 1 == 4) {
          //Taunt
          printf("\"They would point and jeer at the rags splattered with the blood of fellowship. The fools; only I can grasp the highest degree of tragedy upon this earth.\"\n\n");

          //Story / Lore
          printf("Wandering the desolate outskirts of the City as a weary mercenary sniper, this melancholic genius carries the heavy weight of a broken past and lost technology. His supreme, quiet mind allows him to instantly calculate wind speed, bullet trajectory, and enemy weak points with absolute mathematical perfection. Hidden away in the shadows, he fires a specialized long-range rifle, unleashing armor-piercing bullets that pierce multiple targets from afar or even his own allies.\n\n");

          //Description
          printf("A low HP character with Insane damage output and focus on building up 'Fell Bullet' for better potential\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf("\n 1. Volatilized Memory\n When using Skills expect 'Target Readjustment Fire' gain 'Torn Memory' which use for 'Target Readjustment Fire' to buff it\n");
          printf("\n 2. I shall Fire\n After used 'Target Readjustment Fire', lose (Torn Memory x 2) Sanity, at 7+ 'Torn Memory', lose all 'Torn Memory' to gain 'Fell Bullet'\n");
          printf("\n 3. Fell Bullet\n When lost 'Torn Memory' gains 'Fell Bullet', All skills' Damage Multiplier +0.2 and Clash Power +2 for every Stack; then heal 20 Sanity on self (Stackable). Combat Start: Gain +2 Poise Stack for every Stack, when inflicting Bleed, inflict 1 more Bleed Stack or Count (this effect activates as long as there is Fell Bullet in this unit)\n");
          printf("\n 4. Poise\n When Gain by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When Attack: Have a (Stack x 5)%% chance to 'Critical Hit' (Boost damage by 20%%). If this unit Critical Hit, reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf("\n 5. Bleed\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When this unit tossing the Attack Skill's Coins or Clashing end for one round, take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
              } 
        else if (selected_identity - 1 == 5) {
          //Taunt
          printf("\"The Family will be well-cared for. ...After all, the onus always fell on me to provide for what you abandoned.\"\n\n");

          //Story / Lore
          printf("Behind the hyperactive facade of a justice-loving knight lies the ancient, tragic truth of a 'Bloodfiend' progenitor who sought to build a peaceful amusement park where humans and vampires could coexist of her father without caring about family, this had drove her crazy, she gathers all Kindred of 'Bloodfiend' and end her own father's life and state herself as The New Manager of La Manchaland. Driven by centuries of immortal bloodlust and absolute dominion over blood magic, her true power is catastrophically god-like. Charging with terrifying speed, she wields a gargantuan, blood-infused jousting lance and other weapons, impaling anyone who threatens her life.\n\n");

          //Description
          printf("A low HP and defense character with various for each skill and healing skills\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf("\n 1. Variant Sancho Hardblood Arts\n Turn Start: When at 15+ 'Hardblood' using Variant Sancho Hardblood Arts instead for each skills\n");
          printf("\n 2. Bearer of the Blood Kin\n When at 50%% or less HP, 'Responsibility' activate, Clash Power +1, Deal +20%% damage, Take +20%% damage and gain 3 Hardblood\n");
          printf("\n 3. Blood... is flowing...\n If this unit lost the Clash and equipped Attack Skill that isn't 'Variant Sancho Hardblood' empowered, consumes 5 'Hardblood' to use 'Laughters Will Subside' to continue clashing (Once per Turn), if win the Clash with 'Laughters Will Subside' gain 5 'Hardblood'. At 10+ use 'Variant Sancho Hardblood Arts 15 - Buildup to Finale' instead\n");
          printf("\n 4. Armadura de Sangre\n Gain 10%% Damage Up next turn for every 15%% missing HP at Turn End (Max 30%%)\n");
          printf("\n 5. Hardblood\n When use certain skills gain randomly 2-4 Hardblood. When hit by enemy gain 2 Hardblood (Max 30; 'Hardblood' cannot drop below 1)"
            "\n - at 5+ 'Hardblood': Gain +1 Offense for every 5 Hardblood"
            "\n - at 10+ 'Hardblood': Gain +1 Offense and +1 Defense for every 5 Hardblood\n");
              } 
          else if (selected_identity - 1 == 6) {
            //Taunt
            printf("\"Well do I understand your sentiment on death. Why not lay rest to the impulses in your heart for a moment and converse with me more?\"\n\n");

            //Story / Lore
            printf("Naturally timid and terrified of violence, this young rookie mercenary was taken in by the prestigious 'Dawn Office,' training under the wings of a veteran fiery swordsman. As his courage grows during the heat of combat, his immense latent potential ignites, translating his fear into raw power. Charging onto the front lines with newfound confidence, he swings a heavy greatsword coated in roaring, magical flames to burn away the darkness.\n\n");

            //Description
            printf("A low HP character with a great debuff Skills and strong clashing, but come up with limit attack, sometimes on low Living & The Departed getting weak and need to recharge which uses Sanity\n\n");

            //Passive
            printf("Passive Skills:\n");
            printf("\n 1. The Living & The Departed\n Start encounter with 20 'The Living & The Departed' (Max 20), When inflicts 'Butterfly' on enemy on hit, spent this equal to inflicted numbers\n");
            printf("\n 2. Butterfly\n "
              " When this unit at 0 or higher Sanity\n"
              " - On attack with 'Butterfly', 30%% heals Sanity on self or 70%% loses Sanity on enmey equal to (Butterfly/3; Min 1)\n"
              " When this unit at less than 0 \n"
              " - On attack with 'Butterfly', 70%% heals Sanity on self or 30%% loses Sanity on enmey equal to (Butterfly/3; Min 1)\n"
              " On enemy with loses Sanity on Clash Win, heals Sanity on enmey instead and on enemy without Sanity, deal more damage equal to (Butterfly/3; Min 1) instead\n"
              " On attack enemy with 'Butterfly', if enemy's Sanity less than 0 (enemy with loses Sanity on Clash Win, more than 0 Sanity instead), or without Sanity, deal (Butterfly/2 - enemy's Sanity/5) fixed damage (deals (Butterfly/2 + enemy's Sanity/5) fixed damage to enemy with loses Sanity on Clash Win; deals (Butterfly/2) fixed damage to enemy without Sanity; rounded down)\n"
              " Turn End: this effect Expire\n");
            printf("\n 3. Reload\n When runs out of 'The Living & The Departed', or this unit equipped Defense Skill, Turn End uses 'Reload', while attacking, stop attack and use 'Reload' instead, when 'Reload' is used, spend (30 - (The Living & The Departed))/2 Sanity to gain 20 'The Living & The Departed' and gain Shield HP equal to (Butterfly on the target x 2)%% of Max HP. (Max 40%%)\n");
             printf("\n 4. FromTheCoffinAButterflyTakesFlight\n On Clash Win: Enemy loses Sanity based on Skills used (Enemy with loses Sanity on Clash Win, gains Sanity instead; enemy without Sanity gains 'Butterfly' instead)\n");
                } 
            else if (selected_identity - 1 == 7) {
              //Taunt
              printf("\"Sometimes I get hand tremors... I hope that doesn't make me look like a coward.\"\n\n");

              //Description
              printf("A low HP character that focus on build Sanity and gain buff from high Sanity\n\n");

              //Passive
              printf("Passive Skills:\n");
              printf("\n 1. Unstable Shell of Ego\n Turn Start: At 40+ Sanity, consume 20 Sanity to enter the Volatile E.G.O::Waxen Pinion state. At 30%% or less HP and if this unit's Sanity isn't at -45 at Turn End, reset Sanity to 45; then, enter the Volatile E.G.O::Waxen Pinion state. (Once per Encounter) (this 'Turn Start' effect does not activate repeatedly).\n");
              printf("\n 2. Determination\n Turn Start: At 0 or less SP, if in the Volatile E.G.O::Waxen Pinion state; exit the Volatile E.G.O::Waxen Pinion state and loses all 'Volatile Passion' to gain +(Volatile Passion x 3 - Max 20) Clash Power this turn and next turn\n");
               printf("\n 3. Volatile Passion\n Turn Start: Gain 1 'Volatile Passion' while in the Volatile E.G.O state, gain 1 Final Power, gain +20%% Damage and Take -10%% Damage for every stack. Turn End: lose 5 Sanity for every stack(Max 40 Sanity)\n");
              printf("\n 4. Stigma Workshop Weaponry / Passion\n When this unit at 10+ Sanity, gain Clash Power +(Sanity/10). At 45 Sanity, gain Final Power +3 instead. When in a Volatile E.G.O state, and at 0+ Sanity, gain Coin Power +(Sanity/20). At 45 Sanity, gain Coin Power +3 instead.\n");
                  }
             else if (selected_identity - 1 == 8) {
          //Taunt
          printf("\"They're... all from our Office. Firefist Office.\"\n\n");

               //Story / Lore
               printf("Driven by an unyielding thirst for vengeance, Gregor fights under the banner of his own faction, the Firefist Office, waging a brutal war against the 'Bloodfiend' monsters who took everything from him—his friends, his co-workers, and even his beloved big sister. Armed with volatile, experimental military cybernetics, his terrifying combat prowess comes from years of brutal trench warfare combined with a hardened street-brawling instinct. Pushing his body to the absolute limit, he charges bare-handed into the fray with a mechanized thermal gauntlet, triggering devastating, explosive blasts of fire with every single punch he lands.\n\n");

          //Description
          printf("A High HP and defense character with powerful skill 3 along with damage buff and burn for every skills\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf("\n 1. I'm the only survivor...\n When enemy's HP or this unit's HP at 75%% or less HP, Deal +0.2%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (Max 40%%)\n");
               printf(" - If main target have 30+ (Burn Stack + Burn Count), deal +0.3%% damage instead (Max 60%%)\n");
          printf("\n 2. District 12 Special Workshop Fuel\n When start Encounter gain 100 'District 12 Fuel' use for certain skills, when at 50 or less become 'Overheated Fuel', Buff all skills and Burn inflicting; when 'Overheated Fuel' reach 0, or if this unit equipped Defense Skills use 'I have to keep going for big' instead of current skill\n");
               printf("\n 3. ... All burnt to ashes.\n When attack with skills, inflict 'Burn' based on skills used\n");
               printf("\n 4. Burn\n When Inflicted: At 1+ Count, or at 1+ Stack (Turn End: When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), Turn End: Take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
              }
               else if (selected_identity - 1 == 9) {
                 //Taunt
                 printf("\"Yesss...! Finally! Listen up, gamefowls! Get your talons out! We'll be fightin' the night away tonight, 'till there's no more feed left on the sand circle...!\"\n\n");

                 //Story / Lore
                 printf("Within the Heishou Pack—an elite organization of augmented fighters serving the ruthless upper classes of Hongyuan—this variant of Heathcliff reigns as a deadly 'Adept' of the notorious You Branch. The 'You' division is feared across the lands as a relentless, unstoppable cleanup crew, trained to completely annihilate their targets with everything they have until absolutely nothing is left. In battle, he dual-wields two long swords; one of which can instantly ignite into a searing, volatile flame, allowing him to burn his enemies to ashes even as the chaotic, blazing fire consumes his own body.\n\n");

                 //Description
                 printf("A character sacrifics it's HP for enhance Skills and come with powerful Skill 3 in 2 various\n\n");

                 //Passive
                 printf("Passive Skills:\n");
                 printf("\n 1. Flame Rooster's Death Defiance [炎鳥不死戦]\n In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. Then, at the next Turn Start, heal (20 + Burn Stack on self)%% HP, and remove all Burn on self (Max 49%%; Once per Encounter)\n");
                 printf("\n 2. Burn\n When Inflicted: At 1+ Count, or at 1+ Stack (Turn End: When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), Turn End: Take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                  printf("\n 3. Gamefowl\n Cannot be fall below 1 HP due to Burn damage. When use certain skills gains Burn\n");
                 printf("\n 4. Bloody Storm of Blades\n Combat Start: gain 1 Offense Up and 1 Defense Up for every 20%% missing HP on self (Max 3)\n");
                 printf("\n 5. Bloodflame [血炎]\n Gain by using certain Skills (Max 3 Stack), when attack heal 3 Sanity. At 45+ Sanity, gain 1 Offense for next turn instead (Max 3 times per turn), Turn End: lose 1 Stack\n");
                 printf("\n 6. Battleblood Instinct\n Deal 0.75%% damage for every Stack (Max 20 Stack). At 20+ Stack,  activate 'Rooster's Rampaging Blades Under the Ensanguined Heaven' instead of 'Bloodflame Massacre [血炎亂舞]'\n Gain 'Battleblood Instinct' when meeting one of the following\n");
                 printf(" -  Gain 3 at Clash Start\n"); 
                 printf(" -  Gain 1 when this unit hits with a Base Skill or when this unit takes Burn damage (At less than 50%% HP, gain 1 more Battleblood Instinct)\n");
                     }
                 else if (selected_identity - 1 == 10) {
                    //Taunt
                    printf("\"We crossed everyone on the list for today, and... Alright, time to tell Big Brother we're all set to chase down those hair coupon thieves.\"\n\n");

                   //Story / Lore
                    printf("Adopted by 'The Middle', a fanatical global syndicate that operates like a cult-like crime family, the once-timid Sinclair found a terrifying sense of belonging. Driven by the syndicate's absolute code of vengeance—where any slight against the family is recorded in a great book and repaid tenfold—his inner fury is completely unleashed. Fighting with savage, unhinged brutality, he swings a heavy punch to pulverize anyone who hurts his siblings.\n\n");

                    //Description
                    printf("A High HP and defense character with that focus on stacking passive by taking damage and use Counter skill to attack back to enemy\n\n");

                    //Passive
                    printf("Passive Skills:\n");
                    printf("\n 1. The Middle Never Forgets\n When hit by an enemy, inflict 5 'Vendetta Mark' against the attacker (Once per Turn)\n");
                   printf(" Attack End, consume all 'Vendetta Mark' on the target"
                     "\n  - Every time the target consumes 'Vendetta Mark', gain Book of 'Book of Vengeance [Sinclair]' equal to the amount consumed\n");
                   printf("\n 2. Vendetta Mark\n Take 2%% damage for every Stack (Max 20%%) (Max 10 Stack)\n");
                   printf("\n 3. Book of Vengeance [Sinclair]\n Gain more damage equal to (Stack)%% (Max 30%%) (Max 30 Stack)\n");
                   printf(" Gain the following effect next turn based on Stack:"
                      "\n  - At 10+ Stack: Gain 30%% Damage Up"
                     "\n  - At 20+ Stack: Gain 1 Clash Power Up and 1 Base Power Up"
                     "\n  - At 30 Stack: Gain 50%% Damage Up\n");
                    printf("\n 4. The Middle Tattoo\n Combat Start: If this unit using attack skills except for 'Is it You?!', gain 1 Envy Resonance (Once per Turn; lose this effect if this unit 'Stagger' or 'Panic'), use for certain skills, lose all when use 'Warmup in the East'\n");
                        }
                   else if (selected_identity - 1 == 11) {
                     //Taunt
                     printf("\"A life wherein one is granted not choices to make, but instead, choices made...\" *beep* \"Hah. Would that my darling daughter, too, could have felt the mirth that colors such a life.\"\n\n");

                     //Story / Lore
                     printf("Index is one of syndicate that Bound to the absolute, cryptic commands of 'The Prescripts'—divine orders delivered to guide the lawless Backstreets—this silent caretaker carries out the bizarre, unpredictable will of his organization with machine-like efficiency, treating every assassination as a mandatory chore assigned by fate. Moving like a ghost through the shadows, his ultimate and final Prescript has commanded him to join one of the greatest, most secretive projects of the ruling Fingers syndicates in the place name 'House of Spiders'. Appointed as a 'Nursefather' of the subject name 'Yoshihide', his path is now irrevocably tied to the fate of the mysterious project known as 'Arayashiki'.\n\n");

                     //Description
                     printf("A low-HP Character with the high power and high damage under certain condition and come along with A Powerful 9 Coins skills\n\n");

                     //Passive
                     printf("Passive Skills:\n");
                     printf("\n 1. Prescript Delivered on a Device\n Turn Start:\n"
                       " - Gain 'Prescript: [Device] I' / 'Prescript: [Device] II' / 'Prescript: [Device] III' / 'Prescript: [Device] IV' based on 'Unlock' stage on self"
                       "\n - Inflict 'The Prescript's Target' to the enemy"
                       "\n - Apply 'Mark of the Prescript' to Attack Skills on this unit's Dashboard"
                       "\n · At 'Unlock - II'+, the effect above prioritizes Skill 3 (prioritizes empowered Skill)"
                      "\n - All of the effects above and Prescript execution checks do not trigger when this unit is 'Staggered', or in 'Panic'\n");
                     printf("\n 2. Prescript: [Device] \n "
                       " - Prescript: [Device] I : Use a Skill with 'Mark of the Prescript'.\n"
                       " - Prescript: [Device] II : Hit a target with a Skill with 'Mark of the Prescript'.\n"
                       " - Prescript: [Device] III : Gain Procuration [Hermes]. Repeat this Prescript until Procuration [Hermes] reaches 9 Stacks.\n"
                       " - Prescript: [Device] IV : Eliminate all enemies before next Prescript arrvies.\n");
                     printf("\n 3. The Prescript's Target \n Take +10%% damage from Index units\n");
                    printf("\n 4. The Oracle's Proxy / Unlock\n Turn End: If Prescript was executed this turn at below 'Unlock - II'"
                     "\n - When executing Prescript, if the main target has 'The Prescript's Target', heal 8 Sanity and gain 3 'Grace of the Prescript'\n\n"
                     " Turn End: If this unit is at 'Unlock - II' and Prescript was executed this turn, heal 4 SP\n"
                     " - When executing Prescript, if 'Procuration [Hermes]' has reached 9 Stacks, heal 8 Sanity and gain 3 'Grace of the Prescript'\n\n"
                      "Turn End: If Prescript was not executed this turn, gain 5 'Karmic Consequence'\n"
                      " - At 'Unlock - III', does not gain 'Karmic Consequence'"
                     "\n - If this unit attempted to execute Prescript at Combat Start, but could not target the enemy during the combat phase, does not gain 'Karmic Consequence' at Turn End (applies below 'Unlock - II')\n\n"
                      "Turn End: At 3/6/9 'Grace of the Prescript', gain 'Unlock - I' / 'Unlock - II' / 'Unlock - III'\n\n"
                     "Turn Start: At 'Unlock - III', gain 'Shin (心) - Fate'\n");
                     printf("\n 5. Unlock stage - I / II / III \n" 
                       " Unlock stage I - Defense +1, Combat End: Heal 5 Sanity"
                       "\n Unlock stage II - Defense +2, Combat End: Heal 10 Sanity"
                      "\n Unlock stage III - Defense +3, Combat End: Heal 15 Sanity\n");
                     printf("\n 6. Grace of the Prescript \n Offense +1 for every 3 Stack (Max 9 Stack)\n");
                      printf("\n 7. Procuration [Hermes] \n - Turn Start: at 9 Stack, a Powerful Skill become available"
                        "\n - Can be gained up to (Unlock Stage + 2) Stack per turn"
                        "\n - Max Stack: 9"
                        "\n · At below 'Unlock - II', this effect's maximum stack is limited to 8\n");
                     printf("\n 8. Karmic Consequence \n Turn Start:"
                       "\n · Gain 1 Defense Down for every 10 Stack"
                       "\n · Takes +10 Damage for every 20 Stack"
                       "\n - Max Stack: 100\n");
                     printf("\n 9. Shin (心) - Fate \n - Gain +1 Offense and +1 Defense"
                        "\n - Gain +1 Offense for every 20%% (missing HP percentage on target + missing HP percentage on self; rounded down) (Max 3)"
                        "\n - If this unit's Sanity higher than the target's, deal +1%% damage for every 3 Sanity difference (Max 15%%; units without Sanity considered to be 0 Sanity)\n");
                    printf("\n 10. The Index Nursefather\n Upon entering the Encounter for the first time, gain 'Wound-casing Mask'"
                      "\n - Turn End: If this unit was Staggered for the first time, or at 65%% or less HP, in this Encounter while under this effect, recover from Stagger (excluding forced Stagger) and convert 'Wound-casing Mask' to 'Sizzling Wound'\n");
                     printf("\n 11. Wound-casing Mask \n Offense +2 and Defense -2, take -10%% damage from Cracking Unbreakable Coins\n");
                     printf("\n 12. Sizzling Wound \n Offense +3 and Defense -3, take -25%% damage from Cracking Unbreakable Coins, deal +15%% damage with Attack Skill's Unbreakable Coins. Turn Start: Gain 1 Burn Stack and 1 Bleed Stack\n");
                     printf(" \n13. Oracle Device [Caduceus]\n A random weapon is assigned to every Coin for Base Attack Skills. Each weapon has a unique effect."
                      "\n - When hacking through the ribs with a hatchet... (Skill 1 deal +15%% damage for this Coin, On Hit without Cracking: Gain +5%% damage next turn)"
                      "\n - When penetrating the lungs with a stiletto... (Skill 2 deal +15%% damage for this Coin, On Hit without Cracking: Deals 2 Sanity damage to the target)"
                      "\n - When cleaving through the shoulder and the skull with a bastard sword... (Skill 3 deal +25%% damage for this Coin, This coin deal +5%% damage, On Hit without Cracking: Gain +1 Offense next turn (2 times per turn; including for the same effect in other weapon))"
                      "\n - When punching 10 or more holes in the torso with a rapier... (Skill 2 deal +15%% damage for this Coin, This Coin deal +5%% damage, On Hit without Cracking: Inflict 1 Defense Down (2 times per turn; including for the same effect in other weapon))"
                      "\n - When caving in the back of the skull with a hammer... (Skill 1 deal +15%% damage for this Coin, This Coin deal +5%% damage, On Hit without Cracking: Trigger 'Tremor Burst' with 3 Tremor Stacks, if target took 20 damage from 'Tremor Burst' in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess)"
                     "\n - When rending the body with a great sword... (Skill 3 deal +25%% damage for this Coin, This Coin deal +15%% damage, On Hit without Cracking: Target takes +5%% damage next turn (2 times per turn; including for the same effect in other weapon))"
                     "\n - When boring a 20-inch hole with a lance... (Skill 2 deal +15%% damage for this Coin, This Coin deal +15%% damage, On Hit without Cracking: Target takes +5%% damage next turn (2 times per turn; including for the same effect in other weapon))"
                     "\n - When ripping the flesh to ten thousand strips with a whip... (Skill 1 deal +15%% damage for this Coin, This Coin deal +15%% damage, On Hit without Cracking: Target takes +5%% damage next turn (2 times per turn; including for the same effect in other weapon))"
                     "\n - When lacerating through space itself with a scythe, like a certain someone... (Skill 3 deal +25%% damage for this Coin, This Coin deal +30%% damage, On Hit without Cracking: Deals +20%% damage"
                      "\n\nGain 1 'Procuration [Hermes]' when using Skills with Mark of the Prescript"
                       "\n\nConvert 'Wound-casing Mask' to 'Sizzling Wound' after this unit used 'Furioso-Replica' for the first time in this Encounter"
                       "\n\nTurn Start: If this unit has 'Sizzling Wound' and has 'Furioso-Replica' on the Dashboard, gain 'Indulgence in Prescripts'\n");
                      printf("\n 14. Indulgence in Prescripts \n Base Power +1, Damage +30%%, Clash Power +2\n");
                      printf("\n 15. Imitation of a Life\n Deal +2%% damage with Skills marked with 'Mark of the Prescript' for every 'Grace of the Prescript' on self (Max 16%%)"
                       "\n - At 9 'Grace of the Prescript', deal +20%% damage with Base Skills instead\n\n"
                        "On Hit with Base Attack Skill's Unbreakable Coins, gain 1 'Procuration [Hermes]'"
                       "\n - Attack End: Gain 'Procuration [Hermes]' equal to (# of remaining Unbreakable Coins)"
                       "\n - 'Furioso-Replica' Attack End: Gain 'Procuration [Hermes]' next turn equal to (# of this Skill's remaining Unbreakable Coins / 2) (rounded down)"
                        "\n\nIf 'Procuration [Hermes]' reached 9 Stacks this turn at Turn End, and if this unit does not have a Skill 3 on the Dashboard at the start of the next turn, convert a Base Skill to Skill 3 (prioritizes the Skill on the top Slot's row; Only 1 copy of this Skill can exist on the Dashboard)\n");
                     printf("\n 16. By Unpredictable Whim \n When using 'By Unpredictable Whim', At below Unlock - II, activate the following effects (Once per Encounter):\n"
                       " - Gain Unlock - II\n"
                       " - If Grace of the Prescript Stack is less than 6, raise the Stack to 6\n"
                       " - Gain 5 Karmic Consequence at the start of the next turn for every Grace of the Prescript gained via the effect above\n");
                         }
                     else if (selected_identity - 1 == 12) {
                       //Taunt
                       printf("\"Will you join me... in the great task to purify the abominable filth?\"\n\n");

                       //Story / Lore
                        printf("Standing at the absolute peak of a radical, fanatical cult, this omniscient intellectual rules her followers with an iron will, driven by a deep hatred for cybernetic machinery. Her terrifying power stems from her absolute authority and the intense psychological pressure she exerts over others. Commanding her cultists like chess pieces, she personally purges her heretical enemies using a long, rusted iron nail to punch through them\n\n");

                       //Description
                       printf("A character that focus on inflict negative status to enemy and building Sanity\n\n");

                       //Passive
                       printf("Passive Skills:\n");
                       printf("\n 1. Whistles\n When attack or Evade enemy for 3 times (Once per Skills), Next Turn Start: gains 15 Sanity and apply 2 'Fanatic' on self\n");
                      printf("\n 2. Fanatic\n Skill Final Power +(Stack) against enemy with 'Nail' for this turn\n");
                      printf("\n 3. Nail\n"
                        " - Turn Start: Gain 1 Bleed and (Stack) Bleed Count\n"
                        " - Turn End: Halve this effect's Stack (Rounded down)\n");
                       printf("\n 4. Bleed\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When this unit tossing the Attack Skill's Coins or Clashing end for one round, take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                       printf("\n 5. Gaze\n Inflict by certain skill, Take +20%% damage, when hit by an enemy, Enemy's Attack End: Enemy gains 5 Sanity; then loses 'Gaze'. Turn End: loses this effect\n");
                       printf("\n 6. Such Filth\n Before getting attack by Skill, If this unit has 'Fanatic', consumes all 'Fanatic' and then tossing the Coins to evade the attack with below effect\n"
                         " - Base Power: 4\n"
                         " - Coin Power: 10 + (Consumed 'Fanatic')\n"
                         " - This Coin doesn't effect by Paralyze or any effect from tossing Coin\n"
                         " If this Coin's Power more or equal to Attack Power, ignore the attack damage that dealing this unit (this effect also ignore damage from the damage or hit effect from other source), when this effect activate if this unit get attack, cancel all effect\n");
                       printf("\n 7. You Must Accept the Pain!\n When using 'Execution' and Enemy has 3+ 'Nail', use 'Purify' instead\n");
                       printf("\n 8. Bliss of Execution\n When attack with Skill, After Attack: If this unit has 'Fanatic' and enemy have 6+ 'Nail', use 'I Shall Claim Your Life!' after the attack (Once per time)\n");
                           }
                       else if (selected_identity - 1 == 13) {
                          //Taunt
                          printf("\"Let us explore together— the pulsations of your remaining life.\"\n\n");

                         //Story / Lore
                         printf("Serving a twisted, avant-garde syndicate known as the 'Ring'—which views the act of murder as the ultimate form of artistic expression—this eccentric noble brings a terrifying aesthetic to his household. He thrives on the colorful display of agony, viewing the battlefield as his personal canvas while torturing his targets with unsettling joy, using oversized paintbrushes and heavy chisels to paint the walls with the blood of his victims. However, because his traditional, old-school art style did not fit in with the rest of the 'Ring', he was cast out and exiled. This led him to a mysterious facility known as the 'House of Spiders', where he joined the most secretive project of the ruling Fingers syndicates. Appointed as a 'Nursefather' to a subject named 'Yoshihide', his path is now irrevocably tied to the fate of the mysterious project known as 'Arayashiki'.\n\n");

                          //Description
                          printf("A character that focus on inflict negative status to enemy and Dealing more damage and get buff from them\n\n");

                          //Passive
                          printf("Passive Skills:\n");
                          printf("\n 1. Magnum Opus: Tibia\n Skill End: If the Skill dealt HP or Shield damage, gain +5 \x1b[0;33mCorpus Ingredient\x1b[0m Count"
                            "\n - If target is killed or reach 0 HP even with nullify damage, gain +3 additional \x1b[0;33mCorpus Ingredient\x1b[0m Count\n"

                            "\n Gain 1 \x1b[0;33mCorpus Ingredient\x1b[0m Stack every time this unit consumes 10 cumulative \x1b[0;33mCorpus Ingredient\x1b[0m Count in this Encounter"
                            "\n - If the above effect brings this unit's \x1b[0;33mCorpus Ingredient\x1b[0m Stack to 2 or higher, gain \x1b[0;33mArtwork: Tibia\x1b[0m\n"

                            "\nBase Attack Skills and Clashable Counter Skill count as Skills that gain 'Charge' Count\n");
                         printf("\n 2. Corpus Ingredient\n"
                          " - Max Count : 20 \n"
                           " - Unique Charge\n"
                           " - Interacts the same as normal charge does to effects that raise or reduce Charge Stack or Count\n"
                           " - Turn End: Lose 1 Count \n");
                         printf("\n 3. Artwork: Tibia\n Apply the following effects to this unit's Base Attack Skills and Clashable Counter Skill based on Corpus Ingrdient Stack:\n"
                           " - 1+: Base Power +1\n"
                           " - 2+: If the enemy has Bleed or 'Unique Bleed', deal +10%% damage\n"
                           " - 3+: Inflict +1 more Bleed Stack and Count\n"
                           " - 4+: Base Power +1\n"
                           " - 5+: Base Power +1\n");
                          printf("\n 4. Transcend the Corpus\n Upon entering the Encounter for the first time, gain 21 \x1b[38;2;139;69;19mViewing the Tableau\x1b[0m\n"

                            "\n When The Ring-faction allied Identities use a Skill, lose 1 \x1b[38;2;139;69;19mViewing the Tableau\x1b[0m (once per turn per Identity)\n"

                            "\n Turn End: If this unit does not have \x1b[38;2;139;69;19mViewing the Tableau\x1b[0m, replace one of its Base Skills with \"Closing Time - Installation Art no. 1: Your Flesh and Bones as the Gallery's Seats\" at next Turn Start (prioritizes the Skill on the top Slot's row on the Dashboard)"
                            " - If the replaced Skill becomes inaccessible due to being replaced by a different Skill, activate the above effect again\n"
                            " - After used heal 10 Sanity and apply Somatic Frisson-inspiring Melody (Min & Max Speed +1, Base Attack Skills gain Clash Power +1 and deal +10%% damage) on self, Turn Start: gain 21 \x1b[38;2;139;69;19mViewing the Tableau\x1b[0m\n"

                            "\n Turn End: if this unit was Staggered for the first time in this Encounter while under this effect, recover from Stagger (excluding forced Stagger)\n");
                         printf("\n 5. Viewing the Tableau\n Turn End: Lose 2 Stack\n");
                          printf("\n 6. A Maestro's Critique\n On Hit, if the Attack Skill inflicted a negative effect, heal 4 SP at Attack End (3 times per turn)\n"
                           " - If this unit's SP is at max, gain +10%% Damage Up next turn when this unit heals SP with this effect (once per turn)\n");
                          printf("\n 7. Curating the Exhibition\n If this unit's equipped Defense Skill, before getting attack by Skill, If this unit consume \x1b[0;33mCorpus Ingredient\x1b[0m, use 'Curating the Exhibition' to evade the attack\n");
                         printf("\n 8. Bleed\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When this unit tossing the Attack Skill's Coins or Clashing end for one round, take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                         printf("\n 9. Corpus Theater\n"
                           "\n - Base Stack: 3" 
                           "\n - When this unit gains Bleed from enemy Attack Skills, Randomly gain 1 of the following effects next turn and lose 1 Stack of this effect (once per Skill)"
                           "\n · 3 Bleed Stack"
                           "\n · 1 Bind"
                           "\n · 1 Defense Skill Power Down\n");
                              }
                         else if (selected_identity - 1 == 14) {
                           //Taunt
                           printf("\"... What, surprised? Told ya, didn't I? I don't lose duels. Never have, never will.\"\n\n");

                           //Story / Lore
                            printf("A towering, matriarchal figure within the mafia hierarchy, this proud enforcer uses strict rules and overwhelming authority to govern her domain. Her strength comes from her imposing physical stature, cold composure, and an unyielding pride in her organization. Enforcing the household's strict etiquette without an ounce of hesitation, but due in part to her devoutness to her Famiglia, Valencina was promoted to the rank of Sottocapo, though this lasted a measly three days before she was demoted due to an unknown offense. She was then disowned by the faction, which withdrew its love for her but allowed her to keep her Relic and sword, believing them to be tainted by her ownership. This led her to a mysterious facility known as the 'House of Spiders', where she joined the most secretive project of the ruling Fingers syndicates believing this would bring her back to her prime. Appointed as a 'Nursefather' to a subject named 'Yoshihide', her path is now irrevocably tied to the fate of the mysterious project known as 'Arayashiki'. Sometimes she called her apprentice as 'Textbook'\n\n");

                           //Description
                           printf("A character that focus on long clash, dealing damage and overheat for enhance some Effects and Skills in exchange for lower defense, instead normal one provided a great auto-evade Skill\n\n");

                           //Passive
                           printf("Passive Skills:\n");
                           printf("\n 1. The Eye of Precognition\n Upon entering the Encounter for the first time, gain 30 \x1b[0;33mEye of Precognition\x1b[0m\n"
                             " At 0 \x1b[0;33mEye of Precognition\x1b[0m Stack, convert \x1b[0;33mEye of Precognition\x1b[0m into \x1b[0;33mEye of Precognition - Overheat\x1b[0m\n"

                             "\n\x1b[0;33mEye of Precognition\x1b[0m\n"
                             " - Max Stack: 30\n"
                             " - Can lose up to 10 per turn\n"

                             "\n\x1b[0;33mEye of Precognition - Overheat\x1b[0m\n"
                                " - Max Stack: 30\n"
                                " - Offense Level +3, Defense Level -3\n"
                             " - Turn Start: Gain 10 Stack\n"
                             " - At 30 Stack, convert into \x1b[0;33mEye of Precognition\x1b[0m\n"

                             "\n While this unit has \x1b[0;33mEye of Precognition\x1b[0m, activate the following effects:\n"
                            " - In a Clash, gain 1 \x1b[0;33mAccelerating Future\x1b[0m with every Clash (5 times per Skill)\n"
                            " - When attacked by an Unopposed Attack, \x1b[38;2;139;69;19mUnbreakable Coin\x1b[0m Attack, or losing a Clash against an Attack Skill, activate the \"Precognition\" Skill (Once per turn)\n"

                             "\n\x1b[0;33mAccelerating Future\x1b[0m\n"
                               " - Max Stack: 5\n"
                               " - Base Skills deal +3%% damage for every Stack (Max 15%%)\n"
                             " - Base Skills Clash Power +1 for every 2 Stack\n"
                             " - At 5 Stack, Base Skills Coin Power +1\n"
                             " - Skill End: This effect expires\n"

                             "\n The following causes this unit to lose \x1b[0;33mEye of Precognition\x1b[0m Stack:\n"
                               " - Lose 1 \x1b[0;33mEye of Precognition\x1b[0m with every Clash\n"
                               " - When this unit activates the \"Precognition\" Skill via Passive effect due to being attacked by an Unopposed Attack or \x1b[38;2;139;69;19mUnbreakable Coin\x1b[0m, lose 3 \x1b[0;33mEye of Precognition\x1b[0m\n"
                             " - When this unit activates the \"Precognition\" Skill via Passive due to losing a Clash, lose 5 \x1b[0;33mEye of Precognition\x1b[0m\n"

                             "\n After use \"Precognition\", lose 2 \x1b[0;33mEye of Precognition\x1b[0m"
                            "\n Upon successfully evading all Coins on a Skill, use \"Sezionatura di Coniglio\" as an Unopposed Attack against the attacker (once per turn)\n"

                             "\n At the end of the turn this unit's \x1b[0;33mEye of Precognition\x1b[0m is converted into \x1b[0;33mEye of Precognition - Overheat\x1b[0m, activate the following effects:\n"
                            " - \x1b[0;33mReload\x1b[0m (Lose all currently owned Ammo, and reload back to full) (Once per Encounter)\n"
                            " - At the start of the next turn, if this unit has no \"Disposal\" on its Dashboard, replace a Base Skill with \"Disposal\" (prioritizes the Skill on the Slot's bottom row)\n"
                             " · If target is killed or reach 0 HP even with nullify damage by \"Disposal\", uses \x1b[0;33mReload\x1b[0m (Lose all currently owned Ammo, and reload back to full)\n"

                             "\n The following causes this unit to gain \x1b[0;33mShin (心) - Disgrace\x1b[0m next turn:\n"
                            " - If this unit's \x1b[0;33mEye of Precognition\x1b[0m Stack drops to 0\n"
                            " - If this unit is Staggered\n"

                           "\n\x1b[0;33mShin (心) - Disgrace\x1b[0m\n"
                              " - Min & Max Speed +1\n"
                              " - Base Skills inflict +1 more \x1b[0;31mTremor\x1b[0m and \x1b[0;31mBurn\x1b[0m Stack and Count\n"
                            " - Deal +3%% damage for every 3 \x1b[0;33mPoise\x1b[0m Stack on self (Max 15%%)\n"

                             "\n Turn End: When Staggered for the first time in this Encounter, recover from Stagger (excluding forced Staggers)\n");
                          printf("\n 2. The Shame of Famiglia Bognatelli\n"
                           " Gain 2 \x1b[0;33mPoise\x1b[0m Stack and +2 \x1b[0;33mPoise\x1b[0m Count for every \x1b[38;2;139;69;19mAcceleration Round\x1b[0m spent\n"
                           "\n When flipping a Coin that spends \x1b[38;2;139;69;19mAcceleration Round\x1b[0m, the attack does not get canceled even when this unit is out of \x1b[38;2;139;69;19mAcceleration Round\x1b[0m\n"

                            "\n\x1b[38;2;139;69;19mAcceleration Round\x1b[0m\n"
                              " - Max Capacity: 10\n"
                              " - Unique Ammo\n"
                            " - Spent by Certain Skills\n"

                            "\n If this unit equipped Defense Skill; Turn End: If this unit has less than 5 \x1b[38;2;139;69;19mAcceleration Round\x1b[0m, reload \x1b[38;2;139;69;19mAcceleration Round\x1b[0m to 5 (once per turn)\n"

                           "\n This Identity counts only as an \"Identity that inflicts \x1b[0;31mBurn\x1b[0m and \x1b[0;31mTremor\x1b[0m\"\n"

                           "\n Turn Start: If this unit has \"Disposal\" on its Dashboard, inflict \x1b[38;2;139;69;19mGame Target\x1b[0m on the enemy\n"

                           "\n\x1b[38;2;139;69;19mGame Target\x1b[0m\n"
                             " - Take +15%% damage from The House of Spiders: The Thumb Nursefather Rodion\n");
                             
                          printf("\n 3. La Spada di Palermo\n If sum of \x1b[0;31mTremor Burst\x1b[0m in this Encounter equal 5 or more, apply the following effects:\n"
                            " - If this unit has 1+ \x1b[38;2;139;69;19mAcceleration Round\x1b[0m, Skill 5 and 6 gains Coin Power +1 and deals +25%% damage\n"
                           " - Base Skills deal +1%% damage for every 2 \x1b[0;31mBurn\x1b[0m on the main target (Max 15%%)\n"
                           " · If the main target has Shield, Base Skills deal +3%% damage instead (Max 45%%)\n");
                           printf("\n 4. Burn\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (Turn End: When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), Turn End: Take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                            printf("\n 5. Tremor\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Trigger by 'Tremor Burst', Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP/4) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess. Turn End: Lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                           printf("\n 6. Tremor - Scorch\n Change when Triggered by Amplitude Conversion\n"
                             " - On Tremor Burst, Take (Tremor Stack and Burn Stack / 2) fixed Damage and Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP/4) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess.\n"
                             " - Turn End: Lose 1 Count. When reach 0 Count, loses all Stack and Change back to Normal 'Tremor' too (Max 99 Stack/Count)\n");
                           printf("\n 7. Poise\n When Gain by Certain Skills or effects: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When Attack: Have a (Stack x 5)%% chance to 'Critical Hit' (Boost damage by 20%%). If this unit Critical Hit, reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                               }
                       else if (targetIndex == 99) {
                          //Taunt
                          printf("\"Once it is done... I will take \x1b[1;31mAraya\x1b[1;0m back.\"\n\n");

                         //Story / Lore
                         printf("Born as a clone from the Pinky 'Nursefather' and raised by the five 'Nursefathers' of the Fingers, Ryoshu was never seen as a real daughter, but merely as a tool for their ultimate goal. Everything changed when she discovered her own clone, a child named 'Araya'. Vowing to give the girl a normal life, Ryoshu planned their escape. Knowing the terrifying strength of the 'Nursefathers', she hid Araya inside a time-vault, hoping it would safely accelerate the child's growth—unaware the time dilation only affected herself. Desperate to buy time, Ryoshu unsheathed 'Arayashiki', a cursed sword that consumes her own memories with every strike, leaving wounds that can never heal. She attacked her creators, leaving them heavily scarred, accidentally killing her Pinky Nursefather before escaping into the night.\n\n");

                         printf("Years later, Ryoshu returned with greater power and allies, only to find the Pinky Nursefather somehow alive. Driven by unresolved wrath, she fought and struck her down once again—only to discover a horrifying truth: this new Pinky Nursefather was her own daughter, Araya, who had grown up filled with fury after being abandoned in the vault. Shattered by the realization of what she had done, Ryoshu cast aside her own humanity to achieve the state of 'Muga'—the total emptiness of self. Unsheathing the memory-devouring 'Arayashiki' once more, she is no longer bound by mortal morals or syndicate rules. She moves through the battlefield like an executioner ghost, wielding her legendary odachi with terrifying, god-like speed. Her blade leaves trails of dark, cursed brushstrokes that slice through reality itself. For her, every cry of agony is music, and every drop of spilled blood is the perfect paint required to finish her eternal, crimson canvas.\n\n");

                          //Description
                          printf("Erasing me... Erasing you...\n\n");

                          //Passive
                          printf("Passive Skills:\n");
                          printf("\n 1. Wading Through a Dream, the Self Nowhere to be Found [無我夢中]\n"
                            " Turn Start: Gain Muga [無我] equal to current turn count\n"
                            " Gain 4 Muga [無我] for every use of this unit's Coin\n\n"

                            " \x1b[1;30m'I must make it look as though Araya is already safely out of the House of Spiders.'\x1b[0m\n\n"

                            " \x1b[1;30m'I'll stage it by raising this blade against the Nursefathers—that should sell the story that I am trying to stop them from going after Araya.' \x1b[0m\n");
                         printf("\n 2. Muga [無我]\n"
                         " - Turn Start: Gain 1 Offense Level Up and 1 Defense Level Up for every 10 Stack\n"
                            " - The more this effect stacks...\n"
                            " - Max Stack: 100\n\n"

                             " \x1b[1;30m'So I must hold back on using the blade. And once I'm ready, I will return to that House.'\x1b[0m\n");
                         printf("\n 3. Like the Naraka of Avīci and Raurava [阿鼻叫喚]\n"
                           " Encounter Start: Gain 'Tiagnsha Star's Blade - Arayashiki [天殺星刀阿賴耶識]' and fix SP at -44\n\n"

                            " When taking Sanity damage including effects caused by Sinking, take (Sanity damage x 3) HP damage instead\n\n"

                              " \x1b[1;30m'I know better than anyone how strong and tenacious the Nursefathers are.'\x1b[0m\n\n"

                           " \x1b[1;30m'If I am to deceive them, I'll need to stage a desperate act with everything at my disposal.'\x1b[0m\n\n"

                           " \x1b[1;30m'Plan how I'll take on each of them—how their swords will clash with mine.'\x1b[0m\n\n"

                           " \x1b[1;30m'I may lose control over my own mind, but I will make sure to run the picture through my head over and over.'\x1b[0m\n");
                      printf("\n 4. Tiansha Star's Blade - Arayashiki [天殺星刀阿賴耶識]\n"
                             " - Min & Max Speed +6\n"
                        " - Offense Level +6\n"
                       " - Inflict 3 more Bleed Stack with Skills\n"
                       " - Turn Start: Gain 3 Offense Level Up for every hit enemy attack this unit (Max 6)\n\n"

                             " \x1b[1;30m'The heavens themselves are not spared—heaven, earth, man, and the self. This star rises only for the one who perceives all existence and time as a single whole to sever them all.\n'\x1b[0m\n");
                         printf("\n 5. Severed and Torn until Even the Form is Undone [支離滅裂]\n"
                            " On Hit, inflict 2 Sever the Thread [切絲]\n"
                          " - Turn Start: Inflict 1 more Sever the Thread [切絲] for every 10 Muga [無我] on self (Max 4)\n\n"

                          " When hit, take -(Muga [無我] on self + Sever the Thread [切絲] on target)%% damage (Max 90%%)\n\n"

                          " If there is an enemy that has 100 Sever the Thread [切絲] after this unit finishes all attacks with its Skills, use a powerful Skill on target\n\n"

                           " \x1b[1;30m'I'm not strong enough to kill the Nursefathers—this I know. But I can still incapacitate and hold them back, if only for a little while.'\x1b[0m\n\n"

                            " \x1b[1;30m'And then...'\x1b[0m\n\n"

                            " \x1b[1;30m'And then, I'll leave...'\x1b[0m\n\n"

                           " \x1b[1;30m'I'll leave and come back stronger.'\x1b[0m\n\n"

                            " \x1b[1;30m'Secure the help of someone stronger than me, or gain more allies... Either way, I must come back with a guaranteed means of dealing with them once and for all.'\x1b[0m\n");
                         printf("\n 6. Sever the Thread [切絲]\n"
                          " - Turn Start:\n"
                          " · Gain +3 Bleed Stack and +2 Bleed Count\n"
                          " · Take damage equal to (Stack / 3)\n"
                          " - When hit, take damage equal to (Stack / 5)\n"
                         " - The more this effect stacks...\n"
                          " - Max Stack: 100\n\n"

                           " \x1b[1;30m'Let everything be severed—you, me, all that has been, and all that will be.'\x1b[0m\n\n"

                            " \x1b[1;30m'Once it is done... I will take Araya back.'\x1b[0m\n");
          }
             else if (targetIndex == 88) {
        //Taunt
        printf("\"You bear a poison, heavy and slow... yet deadly. I know you well, even though you know nothing about me.'\n\n");

               //Story / Lore
               printf("Standing above the Wings that govern the City as an enigmatic Arbiter of the Head, Binah watches over the world below with a cold, philosophical detachment. Her terrifying strength comes from a 'Singularity'—the absolute control over the extraction of dark, reality-warping concepts and the manifestation of physical degradation. Sitting quietly amidst her golden tea leaves, she speaks in cryptic, poetic metaphors while effortlessly crushing her heretical enemies using a swarm of monolithic 'Black Spikes' and oppressive shockwaves. For the rulers of the Head, her gaze alone is enough to seal the fate of anyone who dares to defy the absolute laws of the City.\n\n");

        //Description
        printf("??????????????????????????????????????????????????????????????????????????????????\n\n");

        //Passive
        printf("Passive Skills:\n");
        printf("\n 1. The Final Reception\n At 50%% or less HP, activate 'Serious', increase HP and Max HP to 1150, gains +100%% damage and 5 Final Power for one turn; then gain new Skills set (Once per Encounter) (Cannot be defeat until this effect activated)\n");
             printf("\n 2. Fairy\n Inflict by certain Skills, Take (Fairy Stack) fixed damage addition for every hit, if this unit's in activated 'An Arbiter', Take (0.5 x Fairy Stack)%% Max HP fixed damage addition for every hit instead. Turn End: Take (Fairy Stack) fixed true damage (true damage ignore Shield HP); then halve stack (Round down), if this unit's in activated 'An Arbiter', Take (0.5 x Fairy Stack)%% Max HP true fixed damage (true damage ignore Shield HP); then halve stack (Round down) instead\n");
             printf("\n 3. Incomplete Arbiter\n In this Encounter, deals -20%% damage, Final Power -1. When activation 'Serious', activate 'An Arbiter' instead\n");
        printf("\n 4. An Arbiter\n Gains +50%% damage, Final Power +2, Deal +20%% damage and +10 Base Power for every Fairy on enemy, when getting attack by Full Cracking Coin, Take -80%% damage and gain (50 + Missing HP/3) Shield HP (Max 100 per attacked), when getting attack and at 0+ Sanity, consumes 10 Sanity to gain (100 + Missing HP/2) Shield HP (Max 300 per attacked)\n");
            } 

          printf("\nSkills (%d Attack Skills, %d Defense Skills):\n", tempPlayer.numSkills, tempPlayer.numDefenseSkills);

         printf("\nAttack Skills %d:\n\n", tempPlayer.numSkills);

          for (int i = 0; i < tempPlayer.numSkills; i++) {
            SkillStats s = tempPlayer.skills[i];
              printf(" %d. %s (%s)\n", i + 1, s.name, getSkillTypeName(s.skillType));

            if (s.Unbreakable > 0) {
                if (!s.Clashable) {
                printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Unclashable)\n",
                       s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
                } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Clashable)\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
              }  else 
              if (!s.Clashable) {
                printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Unclashable)\n",
                       s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
                } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Clashable)\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
            }

        printf("\nDefense Skills %d:\n\n", tempPlayer.numDefenseSkills);

        for (int i = 0; i < tempPlayer.numDefenseSkills; i++) {
          SkillStats s = tempPlayer.defenseSkill[i];
            printf(" %d. %s (%s)\n", i + 1, s.name, getSkillTypeName(s.skillType));

          if (s.Unbreakable > 0) {
              if (!s.Clashable) {
              printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Unclashable)\n",
                     s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
              } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Clashable)\n",
                 s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
            }  else 
            if (!s.Clashable) {
              printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Unclashable)\n",
                     s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
              } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Clashable)\n",
                 s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
          }

          int confirm;
          printf("\nConfirm this identity? (1 = Yes, 2 = Back): ");
          if (scanf("%d", &confirm) == 1 && confirm == 1) {
              break;
          } else {
              printf("\nReturning to identity list...\n \nIdentity:\n");
              for (int i = 0; i < numIdentities; i++)
                  printf("%d. %s\n", i + 1, identity[i]);
              continue;
          }

      } else {
          while (getchar() != '\n'); // clear input
          printf("Invalid selection. Try again.\n");
      }
  }

  if (selected_identity == 100) {
    printf("You selected Muga Ryōshū\n");
  } else if (selected_identity == 89) {
    printf("You selected Binah\n");
  } else {
    printf("You selected %s\n", identity[selected_identity - 1]);
  }







  // -------------------- Enemy Selection ---------------------

  printf("\nEnemy Options:\n");
  for (int i = 0; i < numEnemies; i++)
      printf("%d. %s\n", i + 1, enemyNames[i]);

  while (1) {
      printf("\nChoose enemy (1-%d): ", numEnemies);

        if (scanf("%d", &selected_enemy) == 1) {

          // --- Secret Character System (Press 0) ---
          if (selected_enemy == 0) {
              printf("\n\x1b[1;30m[???]: The mirror darkens... the light around you begins to fade...\x1b[0m\n");
              usleep(1500000);
              printf("\x1b[1;30m[???]: 'If you desire the hidden truth... name the \x1b[1;31mEnemy\x1b[0m \x1b[1;30myou seek.'\x1b[0m\n");
              printf("\x1b[1;30m[???]: Secret Code : \x1b[0m");

              char secret_code[50];
            if (scanf("%49s", secret_code) != 1) {
                while (getchar() != '\n'); // ล้างบัฟเฟอร์ถ้าอ่านค่าพลาด
                continue; 
            }

              if (strcasecmp(secret_code, "Hate") == 0) {
                  printf("\n\x1b[1;31m[!!!]: The mirror shatters! World of hero, and you are one of them, pursuing Evil...\x1b[0m\n");
                  sleep(1);
                    selected_enemy = 100; // Internal flag for Evil Bandit
              } 
              else {
                  printf("\n[...]: The mirror returns to normal... nothing happened.\n");
                  sleep(1);
                  continue; // Back to normal selection
              }
          }

        }

          // Map selection to setup index
          if (selected_enemy == 100) targetIndex = 99;      // Evil Bandit
        else targetIndex = -1;

            if ((selected_enemy >= 1 && selected_enemy <= numIdentities) || targetIndex > 0) {

          // Setup temp characters for info preview
          setupCharacters(&tempPlayer, &tempEnemy, 0, selected_enemy - 1);

          printf("\n--- Enemy Info ---\n");
          printf("Name: %s\n", tempEnemy.name);
          printf("HP: %.0f / %.0f\n", tempEnemy.HP, tempEnemy.MAX_HP);
          printf("Speed: %d ~ %d\n", tempEnemy.MinSpeed, tempEnemy.MaxSpeed);

          if (selected_enemy - 1 == 0) {
            //Taunt
            printf("\"Give me your money!\"\n\n");

            //Story / Lore
             printf("Desperate scavengers lurking in the lawless law of the Backstreets, these low-life criminals prey on the weak just to survive another day. Lacking any formal training or mystical powers, they rely purely on numbers, ambush tactics, and rusted cleavers. While weak individually, their frantic, unpredictable desperation makes them a dangerous threat when gathering in large, chaotic packs.\n\n");
            
            //Description
            printf("just a normal Bandit.\n\n");

            //Passive
             printf("Passive Skills: -\n");
          } 
          else if (selected_enemy - 1 == 1) {
            //Taunt
            printf("\"That's right, ya shrimps. Ya gotta first wrack them teensy' brains o' yours, gotta think real hard 'bout whether you even come close to my rank before runnin' ya mouths, ya hear?\"\n\n");

            //Story / Lore
            printf("Known as the 'Winged Tiger' who held the seat of the 'Tiantui Star' within the Pinky, Lei Heng hid his identity while closely operating with the Thumb. He share a dark, deeply personal past with Ryoshu, claiming to have taught her how to wield a sword and calling her by her true name, 'Yoshihide'. Infamous for his absolute cruelty, he once sliced off an attendant's arm and ripped out his tongue for a single sign of disrespect. In battle, he utilizes a newly learned Shin, unleashing a devastating ring of 'Mang' from his massive blade to heavily mutate and slaughter his opponents.\n\n");

            //Description
            printf("A high-HP, high-Defense boss who focuses on building strength through repeated clashes, and attack with powerful attack\n\n");

            //Passive
             printf("Passive Skills:\n");
            printf("\n 1. Fixed Panic\n This unit's Panic Type does not change when inflicted with an effect that changes Panic Types. Instead, this unit is inflicted with an effect that is inflicted against Non-SP Units.\n");
            printf("\n 2. Panic Recovery\n If this unit is Panicked still can act, Turn End: reset Sanity to 0 and heal Sanity by this unit's missing HP (Max 30)\n");
            printf("\n 3. Tigermark Round Reload\n Turn End: at 90%% or less HP, or at the end of the 3nd turn, reload 'Tigermark Round' and gain a new pattern\n");
            printf("\n 4. Lei Heng [雷橫]\n Turn End: at 80%% or less HP, or at the end of the 5th turn, gain a new pattern, gain 25 Inner Strength [底力] and use a powerful attack 'Tanglecleaver', repeat every 5rd turn, +10 to Min & Max Speed for turns in which this unit uses this Skill\n");
            printf("\n 5. Tiantui Star's Blade [天退星刀]\n Unopposed Attacks deal +30%% damage On Hit and inflict +10%% Take Damage Up next turn\n"

             "\n Skill Coins that spent Tigermark Round deal +20%% damage On Hit and inflict 2 Defense Level Down next turn\n"

              "\nTigermark Round\n"
              " - Unique Ammo\n"
              " - Spent by certain Skills\n"
              " - Max Stack: 6\n"

              "\n The above effects do not activate if the target equipped a Defense Skill\n");
            printf("\n 6. Tiantui Star [天退星]\n When HP at 60%% or less HP at Turn End, or if this unit is set to use a powerful attack ('Tanglecleaver') next turn for the second time this Encounter, if this unit is Staggered, recover from Stagger, activate 'Tiantui Star [天退星]' and lose all Tigermark Round; then convert the effect Tigermark Round to Savage Tigermark Round and gain 18 Savage Tigermark Round then reload immediately\n"

              "\n - If this unit spends all Savage Tigermark Round, lose all Tigermark Round; then convert the effect to Tigermark Round and gain 12 Savage Tigermark Round and reload immediately next 2 turn; if this unit gained Savage Tigermark Round, gain +20%% Take Damage Up next turn and 2 Clash Power Down next turn\n"

              "\nSavage Tigermark Round\n"
             " - Unique Ammo\n"
             " - Spent by certain Skills (activates the same way as Tigermark Round does)\n"
             " - The Stack and Count of this are \"Loaded Ammo\" and \"Remaining Ammo\", respectively\n"
             " · Max Stack (Loaded Ammo): 6\n"
             " - Coins that spent this Ammo deal +30%% damage, and inflict double the normal amount of Burn Stack and Count\n"
             " - When all Loaded Ammo and Remaining Ammo are spent, converts back to Tigermark Round\n"

              "\n 'Tiantui Star [天退星]'\n"
              " - Gain 10%% damage and 1 Final Power for every 20%% HP missing (Max 3 each)\n"
              " - Inflict +2 more Tremor Stack with this unit's Skills\n"
              " - If this unit's Speed is faster than the target's by 3 or more, deal +(Speed difference x 5)%% damage (Max 20%%)\n"
              " - Convert the final Coin of 'Triple Slash - Blast' to an Unbreakable Coin\n"
              " - This buff changes into 'Shin (心) - Tiantui Star [天退星]' when using a specific pattern\n");
            printf("\n 7. Lei Heng, The Pinky's Tiantui Star\nTurn End: at 40%% or less HP, or if this unit is set to use a powerful attack ('Tanglecleaver') next turn for the third time this Encounter, Replace the powerful attack 'Tanglecleaver' with 'Savage Tigerslayer's Perfected Flurry of Blades' +15 to Min & Max Speed for turns in which this unit uses this Skill and convert 'Inner Strength [底力]' to 'Extreme Strength [極力]', convert 'Tiantui Star [天退星]' to 'Shin (心) - Tiantui Star [天退星]'\n"

              "\n 'Shin (心) - Tiantui Star [天退星]'\n"
                " - Gain 10%% damage and 1 Final Power for every 15%% HP missing (Max 5 each)\n"
                " - Inflict +3 more Tremor Stack and +1 more Tremor Count with this unit's Skills\n"
              " - If this unit's Speed is faster than the target's by 3 or more, deal +(Speed difference x 5)%% damage (Max 20%%)\n"
                " - Convert the final Coins of 'Double Slash - Blast' and 'Triple Slash - Blast' to Unbreakable Coins\n");
             printf("\n 8. Chachihu [揷翅虎]\n Combat Start: based on this unit's Speed value,\n");
            printf(" - Heal Sanity equal to Speed value. At less than 0 Sanity, double the heal amount. At more than 15 Sanity, does not activate this effects");
            printf("\n - Gains ((Speed Value / 3) x 10%%) Damage Up. (Max 50%%)\n");
            printf(" At -45 Sanity, does not activate above effects\n");
            printf("\n 9. Inner Strength [底力]\n When using Skills gain 'Inner Strength [底力]' which use for 'Tanglecleaver' and 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]'.\n"
              " - Gain when win Clash +(Clash Count) and +(Clash Count x 2) on 'Extreme Strength [極力]'\n" 
              " - Gain when attack +(Attack Coins x 2) and +(Attack Coins x 3) on 'Extreme Strength [極力]'\n"
              " Turn End: next turn, gain Haste based on the total Stack consumed this turn (1 Haste per 25 Stack; Max 5)\n");
            printf("\n 10. Tiantui Star's Blade - Overheat\n When using Skill 'Tanglecleaver' or 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]' next turn equal to the number of times those skills were used (Max 5) (Clash Power -(Stack), Take (10 x Stackable)%% more damage) next turn\n");
            printf("\n 11. Ten Blades of the East\n At the Turn Start of gaining a new pattern: "
              "\n - Gain 1 Severing Slash [切斬] (Target takes +50%% damage) for one turn"
              "\n - If this unit is Staggered, recover from Stagger"
              "\n - Heal 5 Sanity for every 10%% missing HP on self (Max 20)\n");
            printf("\n 12. Chosen Prey\n at 90%% or less HP, or at the end of the 3nd turn\n"
              " Turn Start: At (50 - current Sanity)%% chance, inflict 1 Prey against the target that dealt the most damage to this unit last turn\n"
              
              "\n Prey"
              "\n - In a Clash with an enemy, Clash Power -3"
              "\n - Combat Start:"
              "\n · If this unit equipped an Attack Skill, deal -50%% damage against Lei Heng with Attack Skills"
              "\n · If this unit equipped a Defense Skill, take -75%% damage from Lei Heng"
              "\n - If this unit wins a Clash against Lei Heng while under this effect, heal 20%% HP"
              "\n - Expires at Turn End; this unit heals 10 Sanity\n"
              
              "\nAfter Lei Heng attacks the target with Prey, if the target is:\n"
              " - Staggered, Target lose 10 Sanity and gain 1 Clash Power Down for this turn and next turn (once per turn)\n");
            printf("\n 13. The Thumb's Officer, Capo IIII\n In a Clash, inflict +(1 + (# of Clashes / 3)) Tremor Count (Max 3)\n");
            printf("\n 14. Burn\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (Turn End: When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), Turn End: Take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
            printf("\n 15. Tremor\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Trigger by 'Tremor Burst', Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess. Turn End: Lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
            printf("\n 16. Tremor - Scorch\n Change when Triggered by Amplitude Conversion\n"
               " - On Tremor Burst, Take (Tremor Stack and Burn Stack / 2) fixed Damage and Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (target's Max HP) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess.\n"
               " - Turn End: Lose 1 Count. When reach 0 Count, loses all Stack and Change back to Normal 'Tremor' too (Max 99 Stack/Count)\n");
            printf("\n 17. I'll be frank\n at 20%% or less HP, End the encounter, whatever enemy is at 0%% HP or not\n");
              }
        else if (selected_enemy - 1 == 2) {
          //Taunt
          printf("\"We are not deserve to even breath...\"\n\n");

          //Story / Lore
          printf("This is an alternate version of Heathcliff who lost everything and went insane. He gained the power to cross Mirror Worlds with a single, brutal purpose: to find and kill every other version of himself, believing his own existence brings only pain to his beloved. He rides a headless horse, uses a giant greatsword made from his power and wield a coffin of his beloved at his back, and commands an army of ghosts name 'Wild hunt'.\n\n");

          //Description
          printf("A high-HP, high-Defense boss who focuses on building sanity through passive, and attack with powerful attack\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf("\n 1. Heart of Vengeance\n If this unit is Panicked still can act and if not at -45 Sanity, Combat Start: heal 15 Sanity, after reset Sanity to 0 and gain 2 Final Power up\n");
           printf("\n 2. Antagonism\n If the target's current HP is higher than this unit's (%%), Clash Power +2 and deal +20%% damage\n");
           printf("\n 3. Long-awaited Moment\n When Clashing\n - Clash Win: Heal 5 Sanity on self \n - Clash Lose: Lose 5 Sanity on self and Gain 1 Final Power Down\n");
          printf("\n 4. May She... Wake in Torment!\n Turn End: if this unit is at 70%% or less HP, or at the end of the 6nd turn, gain new pattern\n");
          printf("\n 5. Withstand\n At 50 or less HP, Cap Hp to 50 and recover from 'Stagger' (Once per Encounter)\n");
          printf("\n 6. Every Heathcliff Must Die...\n At 50 or less HP, fixed Speed to 1 and use 'Every Heathcliff Must Die...'\n");
            } 
        else if (selected_enemy - 1 == 3) {
          //Taunt
          printf("\"Know your place... Fool...\"\n\n");

          //Story / Lore
          printf("He is an ancient, evil curse who arrived through a catastrophic dimensional anomaly. Possessing an immense amount of cursed energy and supreme arrogance, his goal is pure destruction. He kills opponents instantly with invisible, flying air slashes called 'Cleave' and 'Dismantle', and use his domain where it cut everything within 200m range.\n\n");

          //Description
          printf("A boss who focus on dealing damage, which come up with great clash skills. He's boring\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf("\n 1. Binding Vow - Open\n When dealing damage with Cursed Technique (excluding 'Cursed Technique - Fuga:Open [鐚]'), gains 'Binding Vow - Open' equal to (damage/4; rounded down), use for empower 'Cursed Technique - Fuga:Open [鐚]'\n");
           printf("\n 2. The Strongest Of History\n Clash win: gain 5 Attack Power up next turn\n");
           printf("\n 3. King\n If this unit is Panicked still can act, after reset Sanity to 0 and gain 2 Clash Power up next turn\n");
          printf("\n 4. Chanting\n Turn Start: For every 3 Turns, this unit uses 'Chanting', if this unit is set to use 'Chanting' 3 times, next turn use 'World Cutting Slash', after use 'World Cutting Slash' gains 'Lost of Cursed Energy'\n");
          printf("\n 5. Lost of Cursed Energy\n Gains 5 Final Power Down, take +30%% damage and deal -50%% damage. Turn End: Expire this effect\n");
           printf("\n 6. Cursed Reverse Techinque\n In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. Turn End: Heal HP to Max HP and heal Sanity to 45 (Once per Encounter)\n");
          printf("\n 7. Domain Expansion\n after use 'Cursed Reverse Techinque', use 'Domain Expansion:Malevolent Shrine'. When 'Domain Expansion:Malevolent Shrine' activated, activate below effect\n"
            " - Turn End: Deal fixed damage equal to (5%% of target's Max HP) to all enemies and gain 5 'Binding Vow - Open' (this 'Turn End' effect does activate repeatedly)\n"
            " - 'Cursed Technique - Fuga:Open [鐚]' gains +5 Base Power and convert to Unbreakable Coin\n");
            } 
        else if (selected_enemy - 1 == 4) {
          //Taunt
          printf("\"Dreaming end... so what?\"\n\n");

          //Story / Lore
          printf("This is Don Quixote who has succumbed to her true Bloodfiend instincts, driven mad by her broken dreams of justice. Stripped of all restraint, her immortal vampire physiology and charging speed are now completely out of control. Blind to reason, she charges forward with a massive jousting lance, impaling everyone in her way.\n\n");

          //Description
          printf("A low-HP boss who along with great heal along with strong passive buff to Skills\n\n");

          //Passive
           printf("Passive Skills:\n");
           printf("\n 1. Sancho:The Second Kindred of Don Quixote\n When HP reachs to 0 tranform into 'Sancho:The Second Kindred of Don Quixote'\n");
          printf("\n 2. Hardblood\n In 'Sancho:The Second Kindred of Don Quixote' Phase: Start Combat: gain 1 'Hardblood' use for certain skills and buff, when using some certain skills gains some as well (Max 30; 'Hardblood' cannot drop below 1)"
              "\n - at 10+ 'Hardblood': Gain +1 Offense for every 5 Hardblood"
              "\n - at 20+ 'Hardblood': Gain +1 Offense and +1 Defense for every 5 Hardblood\n");
          printf("\n 3. In Dreams\n In 'Sancho:The Second Kindred of Don Quixote' Phase: if this unit is Panicked still can act and after that reset Sanity to 0 and consumes 3 Hardblood to gains 3 Final Power Up and +30%% damage \n");
          printf("\n 4. If we can be freed from this excruciating sickness\n In 'Sancho:The Second Kindred of Don Quixote' Phase: On Hit, heal 40%% of the damage dealt.\n"
            " - This Passive heals +1%% more HP for every missing HP on self (Max 20%%)\n"
            " - For Unbreakable Coins: this effect does not activate on Hit After Clash Lose.\n"
            "Every Turn Start: heal (percentage missing HP/2) HP. (Max 30)\n"
            "Every Turn Start: at -15 or less Sanity, consume (5 - current Sanity/5) Hardblood (Rounded down) to gain (percentage missing HP/2) Sanity and gain 2 Clash Power Up\n"
            "On Hit without Clash Lose, gain 3 Hardblood\n");
          printf("\n 5. I'll Pierce You!\n In 'Sancho:The Second Kindred of Don Quixote' Phase: at HP 60%% or less HP, Use Variant Don Quixote Style: Sancho Arts 2 - La Sangre instead (Once per Encounter)\n");
          printf("\n 6. End Dreams\n In 'Sancho:The Second Kindred of Don Quixote' Phase: at HP 40%% or less HP, Use La Aventura Ha Terminado instead (Once per Encounter), After Attack: when this unit loses Clash consumes 5 Hardblood to take -25%% damage and gain 25 Shield HP (Activates for the left of Encounter)\n");
            } else if (selected_enemy - 1 == 5) {
          //Taunt
          printf("\"I shall afford you neither the wherewithal nor the time to mask your ruminations. Bring forth a real answer; do not let it languish behind your tongue.\"\n\n");

          //Story / Lore
          printf("Formerly known as Kong Qiu and holding the prestigious seat of the 'Tiangang Star' within the Pinky, Jia Qiu is a deeply philosophical mastermind who fights for a burning, ethereal form of justice designed to restore order. Driven by his 'Way' shaped like flame, he dominated the Family Hierarch Evaluations through absolute tactical superiority and unmatched combat prowess. He want to ask you that 'What Hongyuan needs?'\n\n");

          //Description
          printf("A strong boss that can with powerful attack, debuff and unable to beat... but he's not giving it his all.\n\n");

          //Passive
           printf("Passive Skills:\n");
           printf("\n 1. Effloresced E.G.O::Érlì\n First Turn Start: heal 30 Sanity and gain 'A Sliver of Anticipation', He's not giving it his all\n");
          printf("\n 2. A Sliver of Anticipation\n When possess, Lose 35 Offense and 35 Defense, Deal 30%% more damage\n");
          printf("\n 3. Infinite Song of Erudition\n After Attack: heal 5 Sanity\n");
          printf("\n 4. Panic Recovery\n Turn End: if in Panic, reset Sanity to 0\n");
           printf("\n 5. I shall know your answer.\n At 85%% or less HP, then gain new pattern\n");
          printf("\n 6. I still await your answer.\n At 60%% or less HP, apply 3 Dialogues to enemy. Dialogues: Turn End: heal 5 Sanity, When HP drop to 0 heal up to max HP; then lose 1 stack\n");
          printf("\n 7. Do not fear the futility.\n At 40%% or less HP, Turn End: Cap HP to 40%% and recover from 'Stagger'; then use 'Like a Roaring Storm' at Turn Start\n");
          printf("\n 8. perhaps they must be shaken afore you are to speak your truth.\n At 20%% or less HP, Turn End: Cap HP to 20%% and recover from 'Stagger'; then use a powerful attack 'Tiangang Star - Form (格)' at Turn Start\n");
            } else if (selected_enemy - 1 == 6) {
          //Taunt
          printf("\"Grand Welcome...\"\n\n");

          //Description
          printf("The King in Binds (O-01-20-12) is a WAW-class Abnormality, the boss without 'Sanity' that focus on decrease enemy Sanity, he's the king... and the king shall have a knight beside\n\n");

          //Story / Lore
          printf("A god-like Abnormality born from the collective human trauma of restriction, this entity manifests as a skinny, shifting humanoid sitting upon a black and gold throne. The surrounding crimson curtains and heavy ribbons converge toward the figure, binding it tightly to its seat beneath a small, golden crown. Though chained to its throne, the King is fully capable of stepping down at will during combat, manifesting a dark black-and-gold sword wrapped in red ribbons. It dominates the battlefield through terrifying spatial authority, using a unique weapon adorned with a spiked, ray-shaped knuckle-guard to slash its targets or summoning multiple copies of the blade through crimson portals to impale all who dare to breach its domain.\n\n");

          //Passive
           printf("Passive Skills:\n");
           printf("\n 1. The King Shall Have a Knight Beside\n Start Encounter: This unit Summons 'A Knight' that imitate from 'Enemy'. Turn Start: Start the 'Combat Event'\n");
          printf("\n 2. Closing of the Banquet\n When 'A Knight' reaches 0 HP, Turn End: All enemy lose (5 x Turn used) Sanity (Max 45), Turn Start: Start the 'Combat Event'; then this unit joins the Battle\n");
          printf("\n 3. Refracted Struggle\n On Clashing: This unit gain (Clash Count/2) in a Clash (Rounded down) (This effect activing repeating on new Clash Round)\n");
          printf("\n 4. Snapping Bandages\n Turn End: at 20%% or less HP, apply 'Bandages of the King in Binds' on all enemies. Turn Start: Recover from 'Stagger', then use a powerful attack 'Present Thyself Before the King', repeat every 4th turn\n");
          printf("\n 5. Bandages of the King in Binds\n Turn End: Gain +2 Sinking Stack and +1 Sinking Count\n");
          printf("\n 6. Thou Wilt Sink\n When attack with certain skills, inflict 'Sinking' based on skills used\n");
          printf("\n 7. Bound by Guilt\n When attack with certain skills, inflict 'Tremor' or trigger 'Tremor Burst' based on skills used\n");
          printf("\n 8. Sinking\n When Inflicted: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When Hit: Lose (Stack) Sanity (if this unit doesn't have Sanity, take (Stack) fixed damage instead). Then reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf("\n 9. Tremor\n When Inflicted: At 1+ Count, or at 1+ Stack (When triggering, at 1+ Count and 0 Stack, Count as 1 Stack, if at 0 Count and 1+ Stack, Count as 1 Count), When Trigger by 'Tremor Burst', Raise Stagger Threshold equal to Stack; then reduce 1 Count, if this unit's Stagger Threshold at (Max HP/4) in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess. Turn End: Lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
            } else if (selected_enemy - 1 == 7) {
          //Taunt
          printf("\"I don't need some stupid crutch like that 'Shin (心)' crap. I just gotta pull as much power as I can from these tattoos and grit my teeth, and I'll overpower your fancy little tricks no problem.\"\n\n");

          //Story / Lore
          printf("Driven by a deep, childhood despair of being unable to possess a highly acclaimed toy, Matthias grew into a man obsessed with absolute ownership. Though he rose to the rank of Great Brother within the Middle, he betrayed the syndicate's code of absolute loyalty during a Ruins expedition—murdering his own Great Brother and Great Sister out of pure envy to claim the legendary burning Relic, 'Lævateinn'. Banished yet satisfied simply to own the sword, he later served as the Middle's representative to raise Ryoshu into a 'perfect blade'. He spoiled her with toys but protected her with monstrous violence, once beheading her instructor and maiming her classmates over a minor scuffle. Even after Ryoshu turned on him and severed his right arm, Matthias felt no hatred, only confusion. Burdened by heavy debts to the Middle, he now spends his days with his new apprentice, Kira, indulging in a reckless, unsustainable lifestyle while clinging to his stolen treasures.\n\n");

          //Description
          printf("A boss who focus on Counter Attack and deal a ton of damage... He's too strong for the Sinner.\n\n");

          //Passive
           printf("Passive Skills:\n");
           printf("\n 1. Vengeance Tattoos [\x1b[9mGreat Brother\x1b[29m]\n Boost his Power...\n");
          printf("\n 2. The Book of Vengeance\n Encounter Start: Inflict \x1b[0;31mThe Book of Vengeance\x1b[0m on all Sinners\n\n"

            "\x1b[0;31mThe Book of Vengeance\x1b[0m: When this unit hit a target with this effect, heal 7 Sanity (Once per skill)\n\n"

            "If this unit hit with more than 3 Coin Skills this turn, gain 1 \x1b[0;33mThe Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]\x1b[0m next turn for every excess Coin that hit\n");
          printf("\n 3. The Middle - Vengeance Tattoo [\x1b[9mGreat Brother\x1b[29m]\n"
            " - Max Stack: 2\n"
            " - Expires at Turn End\n"
            " - Deal +10%% damage\n\n"

            " \x1b[1;30mA Great Brother whose prestige was secured by his loyalty; a fool who has abandoned that very thing over a sword.\x1b[0m\n");
          printf("\n 4. My Daughter's Education\n On Clash Win, gain 1 \x1b[0;33mCheck This Out, Kiddo!\x1b[0m\n\n"

            " On Hit with a Skill, gain 1 \x1b[0;33mCheck This Out, Kiddo!\x1b[0m (Once per Skill; this effect does not activate for Cracked Coins)\n\n"

            " On Clash Win with the first-used Skill in a turn, gain 1 more \x1b[0;33mCheck This Out, Kiddo!\x1b[0m and the Skill gains Coin Power +1 (Once per 3 turns)\n");
          printf("\n 5. Check This Out, Kiddo!\n"
            " - Max Stack: 10\n"
            " - Reduced by 1 when this unit is hit by 2 Skills\n"
            " - Base Power +1 for every 2 Stack\n"
            " - Skills deal +5%% damage for every Stack\n");
          printf("\n 6. Resentment\n When hit by 2 Skills, gain 1 \x1b[0;31mThe Middle - Grudge\x1b[0m and lose 1 \x1b[0;33mCheck This Out, Kiddo!\x1b[0m (number of hits taken is always tracked)\n\n"

            " At max \x1b[0;31mThe Middle - Grudge\x1b[0m Stack, use a powerful attack\n\n"

            " Turn End: At less than 90%% / 60%% / 40%% HP, max out this unit's \x1b[0;31mThe Middle - Grudge\x1b[0m Stack\n");
          printf("\n 7. The Middle - Grudge\n"
            " - Max Stack: 10\n"
            " - Reduced when using certain Skills\n"
            " - For every Stack, Offense Level +1, Defense Level -2, and Counter Skills deal +5%% damage\n"
            " - At max Stack, take +20%% damage\n\n"

             " \x1b[1;30mThe Middle never forgets.\x1b[0m\n");
          printf("\n 7. Vengeance Tattoos Max Output... Unseal Lævateinn!\n When 'Lævateinn' is unsealed, this unit gains \x1b[0;33mRidiculous Grit\x1b[0m next turn\n");
          printf("\n 8. Ridiculous Grit\n gains following effect:\n"
             " - Take -(Missing HP percentage)%% damage from Skill (Max 90%%; Rounded down)\n"
              " - On Clash Win, heal (Clash count x 5) more Sanity (Max 15)\n"
              " - Turn End: If this unit lost Sanity this turn due to its Skill effects, gain +30%% Damage Up next turn\n");
          printf("\n 9. Panic Recovery\n Turn End: if in Panic, reset SP to 0.\n");
          printf("\n 10. Fixed Panic\n This unit's Panic Type does not change when inflicted with an effect that changes Panic Types. Instead, this unit is inflicted with an effect that is inflicted against Non-SP Units.\n");
          printf("\n 11. Panic Type - Mad Rampage\n"
            "Low Morale:\n"
            "Turn Start: Gain 2 Attack Skill Power Up and 4 Defense Level Down; gain +10%% Damage Up for every 30 HP damage taken last turn (Max 50%%)\n"
            "Panic:\n"
            "Turn Start: Gain 3 Attack Skill Power Up and 5 Defense Level Down; gain +10%% Damage Up for every 25 HP damage taken last turn (Max 50%%)\n");
            } else if (selected_enemy - 1 == 8) {
          //Taunt
          printf("\"That's that, and this is this.\"\n\n");

          //Story / Lore
          printf("He has nothing but sorrow... and he wants nothing more\n\n");

          //Description
          printf("????????????????????????????????????????\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf("\n 1. Agony\n Before Start Encounter: Lose 40 Offense and Lose 45 Defense, At 50%% or less HP, Gain 10 Offense and lose 5 Defense \n");
          printf("\n 2. Black Heart\n If this unit is Panicked still can act, when lose clash heal Sanity instead, when win clash lose Sanity instead\n");
           printf("\n 3. Vengeance For Nothing\n Every end of Turn 3rd, at 0+ Sanity, loses ((Further from 0 Sanity/2) + (5 for every 20%% missing HP)) Sanity (Rounded down) and gain (3 + (1 for every 10%% missing HP)) Black Silence, at less than 0 Sanity, loses (5 + (5 for every 30%% missing HP)) Sanity and gain +2%% damage for every Black Silence next ture\n");
          printf("\n 4. Black Silence\n When win clash with Skills gain 3 Black Silence and gain 1 when lose clash, use for certain Skills (Max 60)\n");
          printf("\n 5. Furioso\n When using any Base Skills\n"
            "  - Attack End: Skills become unavailable\n"
            " After all Skills except 'Furioso' had been used\n"
            "  - Turn End: Recover from 'Stagger' and use 'Furioso' next turn\n"
            "  - 'Furioso' Attack End: All Skills copies set to 1 and become available (Excluding 'Furioso')\n");
            } else if (targetIndex == 99) {
                //Taunt
                printf("\"I am the one and only! This world shall bow under my shadow!\"\n\n");

                //Story / Lore
                printf("In the world of villain and hero, he is the one evilest... but is it really that easy to defeat him?\n\n");

                //Description
                printf("He's almost done! just mere one hit then all over... Isn't it?\n\n");

                //Passive
                 printf("Passive Skills:\n");
                 printf("\n 1. Exhausted\n Turn End: All Skills lose 1 Base Power\n");
                 printf("\n 2. Alomst defeat\n Encounter Start: This unit's Hp equal to 4\n");
                  printf("\n 3. ???????????????\n Turn End: If this unit takes damage, ?????????????????????????????????????????????????, then this unit retreat\n");
                  }

          printf("\nSkills (%d Attack Skills, %d Defense Skills):\n", tempEnemy.numSkills, tempEnemy.numDefenseSkills);

        printf("\nAttack Skills %d:\n\n", tempEnemy.numSkills);

          for (int i = 0; i < tempEnemy.numSkills; i++) {
            SkillStats s = tempEnemy.skills[i];
              printf(" %d. %s (%s)\n", i + 1, s.name, getSkillTypeName(s.skillType));

            if (s.Unbreakable > 0) {
                if (!s.Clashable) {
                printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Unclashable)\n",
                       s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
                } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Clashable)\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
              }  else 
              if (!s.Clashable) {
                printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Unclashable)\n",
                       s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
                } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Clashable)\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
            }

         printf("\nDefense Skills %d:\n\n", tempEnemy.numDefenseSkills);

        for (int i = 0; i < tempEnemy.numDefenseSkills; i++) {
          SkillStats s = tempEnemy.defenseSkill[i];
            printf(" %d. %s (%s)\n", i + 1, s.name, getSkillTypeName(s.skillType));

          if (s.Unbreakable > 0) {
              if (!s.Clashable) {
              printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Unclashable)\n",
                     s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
              } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d (Clashable)\n",
                 s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
            }  else 
            if (!s.Clashable) {
              printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Unclashable)\n",
                     s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
              } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable Copies %d (Clashable)\n",
                 s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Copies);
          }

          int confirm;
          printf("\nConfirm this enemy? (1 = Yes, 2 = Back): ");
          if (scanf("%d", &confirm) == 1 && confirm == 1) {
              break;
          } else {
              printf("\nReturning to enemy list...\n \nEnemy Options:\n");
              for (int i = 0; i < numEnemies; i++)
                  printf("%d. %s\n", i + 1, enemyNames[i]);
              continue;
          }

      } else {
          while (getchar() != '\n');
          printf("Invalid selection. Try again.\n");
      }
  }

  if (selected_enemy == 100) {
    printf("You selected Evil Bandit?\n");

    sleep(1);

    printf("\nIs it... really that easy?\n");

    sleep(1);
  } else {
    printf("You selected %s\n", enemyNames[selected_enemy - 1]);
  }
  
  //-------------------------------------------------------------






  Character player, enemy;

  // Use actual selections here
  setupCharacters(&player, &enemy, selected_identity - 1, selected_enemy - 1);

  if (strcmp(player.name, "Don Quixote:The Manager of La Manchaland") == 0 &&
      strcmp(enemy.name, "Don Quixote") == 0) {

    printf("\n%s: \"Our dream... was over.\"\n", player.name);

    sleep(1);

    printf("\n%s: \"NO! Our dream won't end. Never! FOREVER!\"\n", enemy.name);

    sleep(2);

  } else if (strcmp(player.name, "Heathcliff:Wild Hunt") == 0 &&
             strcmp(enemy.name, "Erlking Heathcliff") == 0) {

    printf("\n%s: \"It matters not what kind of Heathcliff you are! I shall embrace every Catherine mine!!!\"\n",
           player.name);

    sleep(1);

    printf("\n%s: \"You are nothing more than a Heathcliff, soon to disappear.\"\n",
           enemy.name);

    sleep(1);

    enemy.skills[7].active = 1;

    printf("\n%s gains 1 'Faded Promise', In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn (Once per Encounter)\n",
       player.name);

    sleep(2);
  } else if (strcmp(player.name, "Meursault:The Thumb") == 0 &&
             strcmp(enemy.name, "Lei heng") == 0) {

    printf("\n%s: \"Huh... not bad at all! That's a solid sword pick, lad!\"\n",
           enemy.name);

    sleep(1);

    printf("\n%s: \".... You too.\"\n", player.name);

    sleep(2);
  } else if (strcmp(player.name, "Muga Ryōshū") == 0 &&
             strcmp(enemy.name, "Lei heng") == 0) {

    printf("\n%s: \"Finally decidin' to put it all on the line, ain't ya?\"\n",
           enemy.name);

    sleep(1);

    printf("\n%s: \"So this is that 'Muga [無我]' state I've heard 'em whisperin' 'bout... lookin' like ya've gone and emptied out your own head entirely!\"\n", enemy.name);

    sleep(1);

    printf("\n%s: \"Come on then, shrimp! Let's see if that empty soul o' yours can actually keep up with my rhythm!\" *Reload*\n", enemy.name);

    sleep(2);

    enemy.skills[0].name = "Double Slash - Blast [爆]";
    enemy.skills[0].CoinPower += 1;
    enemy.skills[1].name = "Triple Slash - Blast [爆]";
    enemy.skills[1].CoinPower += 1;

    enemy.skills[3].Copies = 4;
    enemy.defenseSkill[0].Copies = 2;

    enemy.MAX_HP += 1000;
    enemy.HP += 1000;
    enemy.Sanity += 45;
    enemy.sanityLossBase = 3;
    enemy.Passive += 50;

    for (int i = 0; i <= 9; i++) {
      enemy.skills[i].BasePower += 10;
      enemy.defenseSkill[i].BasePower += 10;
    }

    printf("\n%s gains +1000 Max HP, heals +45 Sanity, 50 Extreme Strength [極力] and all Skills gain +10 Base Power at start of the Encounter\n", enemy.name);

    sleep(1);

    printf("\n%s gains 'Shin (心) - Tiantui Star [天退星]'\n", enemy.name);

    sleep(1);

    GainNewPattern(&enemy, &player);

    sleep(1);

  } else if (strcmp(player.name, "Hong lu:The Lord of Hongyuan") == 0 &&
             strcmp(enemy.name, "Jia Qiu") == 0) {

    printf("\n%s: \"I never thought I'd ever see you in Hongyuan again, big brother... Fuhu, so in this world, you managed to survive. You seem to be carving out your own path much like myself.\"\n",
           player.name);

    sleep(1);

    printf("\n%s: \"Your presence before me is an adequate 'declaration'. Now... tell me. Is this the 'answer' you stand for?\"\n",
           enemy.name);

    sleep(2);
  } else if (strcmp(player.name, "Binah") == 0 &&
             strcmp(enemy.name, "Fixer grade 9?") == 0) {

    printf("\n%s: \"Long time no see... Roland, seems like you doing NOT indeed FINE, huh?\"\n",
           player.name);

    sleep(1);

    printf("\n%s: \"Out of my way, Binah. They all need to pay...\"\n",
           enemy.name);

    sleep(1);

    printf("\n%s: \"I'm sure this isn't what she want.\"\n",
           player.name);

    sleep(1);

    printf("\n%s: \"...that's that and this is this.\"\n",
           enemy.name);

    sleep(2);
  } else if (strcmp(player.name, "The House of Spiders: The Index Nursefather Yi Sang") == 0 &&
             strcmp(enemy.name, "Fixer grade 9?") == 0) {

    printf("\n%s: \"You can't even protect your own family...\"\n",
           enemy.name);

    sleep(1);

    printf("\n%s: \"... This is how prescript went for me\"\n",
           player.name);

    sleep(1);

    printf("\n%s: \"Stand still and watching someone from your finger killing your own family before you... Is that what you want?\"\n",
           enemy.name);

    sleep(1);

    printf("\n%s: \"Forget it, I had enough\"\n",
           enemy.name);

    sleep(1);

    printf("\n%s: \"...\" *beep*\n",
       player.name);

    sleep(1);

    printf("\n%s & %s: \"Then is then, and now is now.\" / \"That's that and this is this\"\n",
           player.name, enemy.name);

    sleep(1);

    printf("\n%s gains 'Will of Prescript' (+3 Offense and +5 Defense for this Encounter) and 'Deep Internal Conflict' (Clash Power -3 for this Encounter)\n",
           player.name);

    for (int i = 0; i < player.numSkills; i++) {

      player.skills[i].Offense += 3;
      player.skills[i].Defense += 5;

    }

    player.defenseSkill[0].Offense += 3;
    player.defenseSkill[0].Defense += 5;

    sleep(1);

    printf("\n%s gains 'Furioso' (All Skills' 1 Breakable Coins convert to Unbreakable Coins (Excluding 'Durandal'), beginning with the final Coin)\n",
           enemy.name);

    for (int i = 0; i < enemy.numSkills; i++) {

      if (enemy.skills[i].Unbreakable <= 0) {
      enemy.skills[i].Unbreakable += 1;
      }

    }

    sleep(2);
  } else if (strcmp(player.name, "The Middle Little Brother Sinclair") == 0 &&
             strcmp(enemy.name, "Lei heng") == 0) {

    printf("\n%s: \"The Thumb... still acting as the fist of the hierarchy, as always.\"\n",
           player.name);

    sleep(1);

    printf("\n%s: \"Hah! Seems the Middle’s got a lot to learn about shuttin' folks up. You’re still waggin' that tongue like they never even touched ya! Guess I’ll have to be the one to pay the 'fee' to close that mouth for good!\"\n",
           enemy.name);

    sleep(2);
  } else if (strcmp(player.name, "Dawn Office Fixer Sinclair") == 0 &&
     strcmp(enemy.name, "Fixer grade 9?") == 0) {

  printf("\n%s: \"Hope you realize your stigma.\"\n",
   player.name);

  sleep(1);

  printf("\n%s: \"I used to know someone... from your office\"\n",
   enemy.name);

  sleep(2);
  } else if (strcmp(player.name, "The House of Spiders: The Index Nursefather Yi Sang") == 0 &&
     strcmp(enemy.name, "Lei heng") == 0) {

  printf("\n%s: \"... *beep* A familiar face. Is it written in the Prescript that we should meet?\"\n",
   player.name);

  sleep(1);

  printf("\n%s: \"*smoking* I could tell from that mask... You're just another puppet of those orders.\"\n",
   enemy.name);

  sleep(2);
  } else if (strcmp(player.name, "Hong lu:The Lord of Hongyuan") == 0 &&
     strcmp(enemy.name, "King in Binds") == 0) {

  printf("\n%s: \"A King in Binds? What a pathetic!\"\n",
   player.name);

  sleep(2);
  } else if (strcmp(enemy.name, "The Middle Nursefather - Matthias") == 0) {

    if (strcmp(player.name, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {

    printf("\n%s: \"You look like someone I knew, ain't you?\"\n",
           enemy.name);

    sleep(1);

    printf("\n%s: *beep* \"Haha... Fate's got a twisted sense of humor. Hope I don't remind you of her *too* much...\"\n",
           player.name);

      sleep(1);

      printf("\n%s: \"Wouldn't want to make this more personal than it already is.\"\n",
           player.name);

      sleep(1);

    } else if (strstr(player.name, "Ryoshu") != NULL || strstr(player.name, "Ryōshū") != NULL) {

      printf("\n%s: \"Yoshihide...\"\n", enemy.name);

      sleep(1);

      printf("\n%s: \"My wayward little girl... Welcome back to the Family's embrace.\"\n", enemy.name);

      sleep(1);

      if (isId(player.ID, "Muga Ryōshū") == 0) {

      printf("\n%s: \"Ditched the ink and the family to play pretend with these losers, huh?\n", enemy.name);

      sleep(2);

        printf("\n%s: *Grabs his weapon* \"The Book says debts gotta be settled in red. Don't go croakin' on me too fast now.\"\n", enemy.name);

        sleep(2);

        printf("\n%s gains 'Final Book of Yoshihide' (All Skills' Base Power +10 and +1.0 Damage Mutipler)\n", enemy.name);

        for (int i = 0; i <= 9; i++) {
          enemy.skills[i].BasePower += 10;
          enemy.defenseSkill[i].BasePower += 10;
          enemy.skills[i].DmgMutiplier += 1;
          enemy.defenseSkill[i].DmgMutiplier += 1;
        }

        sleep(1);
      }

    }

    printf("\nMoses uses ?????\n");

    sleep(1);

    updateSanity(&player, 45);
    if (player.sanityGainBase >= 0) {
    player.sanityGainBase += 5;
    } else {
      player.sanityLossBase += 5;
    }

    printf("\nTurn Start: %s gains 30 Shield and gains (Sanity/10) Offense Level Up and Defense Level Up (rounded down); Sanity Base Gain +5 (for loses on clash character, gain Sanity Base Loss +5 instead) and heals 45 Sanity for this Encounter\n",
       player.name);

    sleep(1);

    printf("\nMoses: \"Hope this help you.\"\n");

    sleep(1);

    printf("\nMoses retreats...\n");

    sleep(2);
  } else if (isId(enemy.ID, "Evil Bandit") == 0) {

      printf("\n%s: \"Hey! come back here!\n", player.name);

      sleep(1);

        printf("\n%s: \"HAHAHA! Catch me if you can!\"\n", enemy.name);

        sleep(2);

  } else if (strcmp(player.name, "The House of Spiders: The Thumb Nursefather Rodion") == 0) {
    
    if (strcmp(enemy.name, "Lei heng") == 0) {

    printf("\n%s: \"Hah! How's it feel watchin' that fancy ticket o' yours turn into nothin' but a scrap o' blank paper, huh, shrimp?\"\n",
           enemy.name);

    sleep(2);

    printf("\n%s: \"Shut your damn mouth, you mannerless brat! I ain't here to trade jokes with a stray like you. I'm gonna enjoy rippin' that tongue out!\"\n",
           player.name);

    sleep(2);
  }

    if (strcmp(enemy.name, "The Middle Nursefather - Matthias") == 0) {

      printf("\n%s: \"God, you're just as loud and grating as she was. And that sword... deserves a wielder with actual class, not a savage like you. What a waste of steel.\"\n",
         player.name);

      sleep(2);

      printf("\n%s: \"The resemblance is uncanny—you really are a spitting image of her! But you’re still just a nuisance... a fly buzzing around, doing nothing but dodging. Maybe I'll gouge those 'Eye' out to see if you're actually worth the effort. And don't touch the sword—it belongs to ME!\"\n",
         enemy.name);

      sleep(2);
    }

  }


  printf("\n%s HP %.2f / %.2f\n", player.name, player.HP, player.MAX_HP);
  printf("%s HP %.2f / %.2f\n", enemy.name, enemy.HP, enemy.MAX_HP);

  sleep(1);

  // Track two skills and last unused
  int playerSkill1 = -1, playerSkill2 = -1, playerSkill3 = -1, playerLastUnused = -1, playerDefenseSkill = 0;
  int enemySkill1 = -1, enemySkill2 = -1, enemySkill3 = -1, enemyLastUnused = -1;

  SkillStats *playerSkillEffective = NULL;  // For effective skill stats 
  SkillStats *enemySkillEffective = NULL; 

  // First turn: roll both skills
  getSkills(&player, &playerSkill1, &playerSkill2, &playerSkill3, playerLastUnused,
               player.numSkills);
  getSkills(&enemy, &enemySkill1, &enemySkill2, &enemySkill3, enemyLastUnused,
               enemy.numSkills);

  // Battle loop
  if (selected_enemy - 1 == 6) {

    // For King in Binds

          if (selected_enemy - 1 == 6) {

              runKingInBindsBattle(
                  &player, &enemy,
                  &playerSkill1, &playerSkill2, &playerSkill3, playerDefenseSkill, &playerLastUnused,
                  &enemySkill1,  &enemySkill2,  &enemySkill3,  &enemyLastUnused);

    }

  } else { 

    while (player.HP > 0 && enemy.HP > 0) {

    printf("\n--- Turn %d ---\n", TurnCount);

      int enemySkillIndex = -1;

      // ใช้ฟังก์ชันใหม่ที่เราสร้างเพื่อเลือกว่าจะ "โจมตี" หรือ "ป้องกัน" ตามค่า Copies
      int decision = pickEnemyActionWeighted(&enemy, enemySkill1, enemySkill2);

      if (decision >= 100) {
          // --- กรณีเลือก Defense Skill ---
          int defIdx = decision - 100;
          enemySkillEffective = &enemy.defenseSkill[defIdx];
          enemySkillIndex = enemySkill1;


      } else {
          // --- กรณีเลือกท่าโจมตีปกติ ---
          enemySkillIndex = (decision == 1 ? enemySkill1 : enemySkill2);
          enemyLastUnused = (enemySkillIndex == enemySkill1 ? enemySkill2 : enemySkill1);
          enemySkillEffective = &enemy.skills[enemySkillIndex];

      }

      // For enemy
      getSkills(&enemy, &enemySkill1, &enemySkill2, &enemySkill3, enemyLastUnused,
                   enemy.numSkills);

      //------------------- Turn Start ----------------------------

        handleTurnStart(&player, &enemy, &enemySkillEffective, &playerSkill1, &playerSkill2, &playerSkill3, &enemySkill1, &enemySkill2, &enemySkill3);

      // Check who can act this turn
      int IsplayerUnableToAct = isPanicked(&player) || isStaggered(&player);
      int IsenemyUnableToAct  = isPanicked(&enemy)  || isStaggered(&enemy);

      // ถ้าทั้งคู่ทำอะไรไม่ได้เลย
      if (IsplayerUnableToAct && IsenemyUnableToAct) {
            printf("\nBoth are unable to act! They recover...\n");
            if (isPanicked(&player)) { player.Sanity = 0; }
        if (isPanicked(&enemy)) { enemy.Sanity = 0; }
            TurnCount++;
            continue;
        }

      if (IsplayerUnableToAct) {
        if (isStaggered(&player)) printf("\n%s is STAGGERED and cannot act!\n", player.name);
        else if (isPanicked(&player)) printf("\n%s is in PANIC and cannot act!\n", player.name);

        // Lose Envy Resonance
        if (isId(player.name, "The Middle Little Brother Sinclair") == 0) {

            player.Passive = 0;

          printf("\n%s loses all Envy Resonance\n", player.name);

        }
      }

      if (IsenemyUnableToAct) {
        if (isStaggered(&enemy)) printf("\n%s is STAGGERED and cannot act!\n", enemy.name);
        else if (isPanicked(&enemy)) printf("\n%s is in PANIC and cannot act!\n", enemy.name);

        // Lose Envy Resonance
        if (isId(enemy.name, "The Middle Little Brother Sinclair") == 0) {

            enemy.Passive = 0;

          printf("\n%s loses all Envy Resonance\n", enemy.name);

        }
      }

      printf("\nCurrent HP:\n");

      // แสดงผลของ Player
      if (player.Stagger > 0) printf("[Stagger] ");
      printf("%s = %.2f / %.2f", player.name, player.HP, player.MAX_HP);
      if (player.Shield > 0 || player.TempShield > 0) printf(" (Shield %.2f)", player.Shield + player.TempShield);
      printf("\n");

      // แสดงผลของ Enemy
      if (enemy.Stagger > 0) printf("[Stagger] ");
      printf("%s = %.2f / %.2f", enemy.name, enemy.HP, enemy.MAX_HP);
      if (enemy.Shield > 0 || enemy.TempShield > 0) printf(" (Shield %.2f)", enemy.Shield + enemy.TempShield);
      printf("\n");


      if (player.hasSanity || enemy.hasSanity) {
        printf("[Sanity] ");
        if (player.hasSanity)
          printf("%s: %d (%s) ", player.name, player.Sanity, getSanityStatus(&player));
        if (enemy.hasSanity)
          printf("| %s: %d (%s)", enemy.name, enemy.Sanity, getSanityStatus(&enemy));
        printf("\n");
      }

        // ------------------ Before fight -----------------------

      handleBeforeFight(&player, &enemy, &enemySkillEffective, playerSkill1, playerSkill2, enemySkill1, enemySkill2);
      handleBeforeFight(&enemy, &player, &playerSkillEffective, enemySkill1, enemySkill2, playerSkill1, playerSkill2);

      // Player picks one skill (only if can act)
      int playerSkillIndex;

      int playerTempOffense = 0, playerTempDefense = 0;
      int enemyTempOffense = 0, enemyTempDefense = 0;
      playerTempOffense += (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]);
      playerTempDefense += (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]);
      enemyTempOffense += (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]);
      enemyTempDefense += (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]);

      int ECoinBoost = 0;
      if (enemySkillEffective->CoinPower >= 0) {
            ECoinBoost = enemy.PlusCoinPowerBoost[0] - enemy.PlusCoinPowerDrop[0];
      } else {
            ECoinBoost = enemy.MinusCoinPowerDrop[0] - enemy.PlusCoinPowerBoost[0];
      }

      if (!IsenemyUnableToAct) {
      if (enemySkillEffective->Unbreakable > 0 && (enemySkillEffective->Clashable || enemySkillEffective->skillType != 0)) {
        printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Unbreakable %d)\n",
          getSkillTypeName(enemySkillEffective->skillType),
               enemySkillEffective->name,
               enemySkillEffective->BasePower + (enemy.BasePowerUp[0] - enemy.BasePowerDown[0]),
               enemySkillEffective->CoinPower + ECoinBoost,
               enemySkillEffective->Coins,
               enemySkillEffective->Offense + (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]),
               enemySkillEffective->Defense + (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]),
               enemySkillEffective->Unbreakable);
      } else if (enemySkillEffective->Unbreakable <= 0 && (enemySkillEffective->Clashable || enemySkillEffective->skillType != 0)) {
        printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Breakable)\n",
          getSkillTypeName(enemySkillEffective->skillType),
               enemySkillEffective->name,
          enemySkillEffective->BasePower + (enemy.BasePowerUp[0] - enemy.BasePowerDown[0]),
             enemySkillEffective->CoinPower + ECoinBoost,
             enemySkillEffective->Coins,
             enemySkillEffective->Offense + (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]),
               enemySkillEffective->Defense + (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]));
      } else if (enemySkillEffective->Unbreakable > 0 && !enemySkillEffective->Clashable && enemySkillEffective->skillType == 0) {
        printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Unbreakable %d)\n",
          getSkillTypeName(enemySkillEffective->skillType),
               enemySkillEffective->name,
          enemySkillEffective->BasePower + (enemy.BasePowerUp[0] - enemy.BasePowerDown[0]),
             enemySkillEffective->CoinPower + ECoinBoost,
             enemySkillEffective->Coins,
             enemySkillEffective->Offense + (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]),
             enemySkillEffective->Defense + (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]),
               enemySkillEffective->Unbreakable);
      } else if (enemySkillEffective->Unbreakable <= 0 && (!enemySkillEffective->Clashable && enemySkillEffective->skillType == 0)) {
        printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Breakable)\n",
          getSkillTypeName(enemySkillEffective->skillType),
               enemySkillEffective->name,
                enemySkillEffective->BasePower + (enemy.BasePowerUp[0] - enemy.BasePowerDown[0]),
             enemySkillEffective->CoinPower + ECoinBoost,
             enemySkillEffective->Coins,
             enemySkillEffective->Offense + (enemy.OffenseLevelUp[0] - enemy.OffenseLevelDown[0]),
             enemySkillEffective->Defense + (enemy.DefenseLevelUp[0] - enemy.DefenseLevelDown[0]));
      }
    }

      int PCoinBoost1 = 0;
      if (player.skills[playerSkill1].CoinPower >= 0) {
              PCoinBoost1 = player.PlusCoinPowerBoost[0] - player.PlusCoinPowerDrop[0];
      } else {
              PCoinBoost1 = player.MinusCoinPowerDrop[0] - player.PlusCoinPowerBoost[0];
      }

      int PCoinBoost2 = 0;
      if (player.skills[playerSkill2].CoinPower >= 0) {
              PCoinBoost2 = player.PlusCoinPowerBoost[0] - player.PlusCoinPowerDrop[0];
      } else {
              PCoinBoost2 = player.MinusCoinPowerDrop[0] - player.PlusCoinPowerBoost[0];
      }

      int PCoinBoost3 = 0;
      if (player.skills[playerSkill3].CoinPower >= 0) {
              PCoinBoost3 = player.PlusCoinPowerBoost[0] - player.PlusCoinPowerDrop[0];
      } else {
              PCoinBoost3 = player.MinusCoinPowerDrop[0] - player.PlusCoinPowerBoost[0];
      }

    if (!IsplayerUnableToAct) {
      printf("\nDashboard Skills:\n");

        if (player.skills[playerSkill1].Unbreakable > 0 && player.skills[playerSkill1].Clashable) {
            printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player.skills[playerSkill1].skillType),
                   player.skills[playerSkill1].name,
                   player.skills[playerSkill1].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill1].CoinPower + PCoinBoost1,
                   player.skills[playerSkill1].Coins,
                   player.skills[playerSkill1].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill1].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]),
                   player.skills[playerSkill1].Unbreakable);
        } else if (player.skills[playerSkill1].Unbreakable <= 0 && player.skills[playerSkill1].Clashable) {
            printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill1].skillType),
                   player.skills[playerSkill1].name,
                   player.skills[playerSkill1].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill1].CoinPower + PCoinBoost1,
                   player.skills[playerSkill1].Coins,
                   player.skills[playerSkill1].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill1].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]));
        } else if (player.skills[playerSkill1].Unbreakable > 0 && !player.skills[playerSkill1].Clashable) {
            printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill1].skillType),
                   player.skills[playerSkill1].name,
                   player.skills[playerSkill1].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill1].CoinPower + PCoinBoost1,
                   player.skills[playerSkill1].Coins,
                   player.skills[playerSkill1].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill1].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]),
                   player.skills[playerSkill1].Unbreakable);
        } else {
            printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill1].skillType),
                   player.skills[playerSkill1].name,
                   player.skills[playerSkill1].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill1].CoinPower + PCoinBoost1,
                   player.skills[playerSkill1].Coins,
                   player.skills[playerSkill1].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill1].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]));
        }

        if (player.skills[playerSkill2].Unbreakable > 0 && player.skills[playerSkill2].Clashable) {
            printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill2].skillType),
                   player.skills[playerSkill2].name,
                   player.skills[playerSkill2].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill2].CoinPower + PCoinBoost2,
                   player.skills[playerSkill2].Coins,
                   player.skills[playerSkill2].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill2].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]),
                   player.skills[playerSkill2].Unbreakable);
        } else if (player.skills[playerSkill2].Unbreakable <= 0 && player.skills[playerSkill2].Clashable) {
            printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill2].skillType),
                   player.skills[playerSkill2].name,
                   player.skills[playerSkill2].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill2].CoinPower + PCoinBoost2,
                   player.skills[playerSkill2].Coins,
                   player.skills[playerSkill2].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill2].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]));
        } else if (player.skills[playerSkill2].Unbreakable > 0 && !player.skills[playerSkill2].Clashable) {
            printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill2].skillType),
                   player.skills[playerSkill2].name,
                   player.skills[playerSkill2].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill2].CoinPower + PCoinBoost2,
                   player.skills[playerSkill2].Coins,
                   player.skills[playerSkill2].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill2].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]),
                   player.skills[playerSkill2].Unbreakable);
        } else {
            printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill2].skillType),
                   player.skills[playerSkill2].name,
                   player.skills[playerSkill2].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill2].CoinPower + PCoinBoost2,
                   player.skills[playerSkill2].Coins,
                   player.skills[playerSkill2].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill2].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]));
        }

      // Next Skill
        if (player.skills[playerSkill3].Unbreakable > 0 && player.skills[playerSkill3].Clashable) {
            printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill3].skillType),
                   player.skills[playerSkill3].name,
                   player.skills[playerSkill3].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill3].CoinPower + PCoinBoost3,
                   player.skills[playerSkill3].Coins,
                   player.skills[playerSkill3].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill3].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]),
                   player.skills[playerSkill3].Unbreakable);
        } else if (player.skills[playerSkill3].Unbreakable <= 0 && player.skills[playerSkill3].Clashable) {
            printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill3].skillType),
                   player.skills[playerSkill3].name,
                   player.skills[playerSkill3].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill3].CoinPower + PCoinBoost3,
                   player.skills[playerSkill3].Coins,
                   player.skills[playerSkill3].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill3].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]));
        } else if (player.skills[playerSkill3].Unbreakable > 0 && !player.skills[playerSkill3].Clashable) {
            printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill3].skillType),
                   player.skills[playerSkill3].name,
                   player.skills[playerSkill3].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill3].CoinPower + PCoinBoost3,
                   player.skills[playerSkill3].Coins,
                   player.skills[playerSkill3].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill3].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]),
                   player.skills[playerSkill3].Unbreakable);
        } else {
            printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill3].skillType),
                   player.skills[playerSkill3].name,
                   player.skills[playerSkill3].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
                   player.skills[playerSkill3].CoinPower + PCoinBoost3,
                   player.skills[playerSkill3].Coins,
                   player.skills[playerSkill3].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
                   player.skills[playerSkill3].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]));
        }


      // Defense Skill

      int PCoinBoostDef = 0;
      if (player.defenseSkill[playerDefenseSkill].CoinPower >= 0) {
                PCoinBoostDef = player.PlusCoinPowerBoost[0] - player.PlusCoinPowerDrop[0];
      } else {
                PCoinBoostDef = player.MinusCoinPowerDrop[0] - player.PlusCoinPowerBoost[0];
      }
      
      printf("\n");
      if (player.defenseSkill[playerDefenseSkill].Unbreakable > 0) {
        printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
          getSkillTypeName(player.defenseSkill[playerDefenseSkill].skillType),
               player.defenseSkill[playerDefenseSkill].name,
               player.defenseSkill[playerDefenseSkill].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
               player.defenseSkill[playerDefenseSkill].CoinPower + PCoinBoostDef,
               player.defenseSkill[playerDefenseSkill].Coins,
               player.defenseSkill[playerDefenseSkill].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
               player.defenseSkill[playerDefenseSkill].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]),
               player.defenseSkill[playerDefenseSkill].Unbreakable);
      } else if (player.defenseSkill[playerDefenseSkill].Unbreakable <= 0) {
        printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
          getSkillTypeName(player.defenseSkill[playerDefenseSkill].skillType),
               player.defenseSkill[playerDefenseSkill].name,
               player.defenseSkill[playerDefenseSkill].BasePower + (player.BasePowerUp[0] - player.BasePowerDown[0]),
               player.defenseSkill[playerDefenseSkill].CoinPower + PCoinBoostDef,
               player.defenseSkill[playerDefenseSkill].Coins,
               player.defenseSkill[playerDefenseSkill].Offense + (player.OffenseLevelUp[0] - player.OffenseLevelDown[0]),
               player.defenseSkill[playerDefenseSkill].Defense + (player.DefenseLevelUp[0] - player.DefenseLevelDown[0]));
      }
      printf("\n");

      int choice;
      while (1) {
        printf("Choose skill (1-2, or 0 to Use Defense instead of Skill 1): ");
        if (scanf("%d", &choice) == 1 && (choice == 1 || choice == 2 || choice == 0))
          break;
        while (getchar() != '\n')
          ;
      }

      if (choice == 0) {
        // --- กรณีเลือก Guard (แทนที่ Skill 1) ---
        playerSkillEffective = &player.defenseSkill[0]; // ชี้ไปที่สกิลป้องกัน

        // สำคัญ: ตั้งค่าให้ระบบมองว่าเรา "ใช้" playerSkill1 ไปแล้ว
        playerSkillIndex = playerSkill1; // (เพื่อให้ระบบหลังบ้านดึงชื่อหรือตำแหน่งมาใช้อ้างอิงได้)
        playerLastUnused = playerSkill2; // สกิลช่อง 2 จะถูกเก็บไว้

        // สุ่มสกิลใหม่มาแทนที่ช่องที่ 1 (และเลื่อนช่อง 3 มาช่อง 2 ตามระบบ getSkills ของคุณ)
        getSkills(&player, &playerSkill1, &playerSkill2, &playerSkill3, playerLastUnused, player.numSkills);

      } else if (choice == 1 || choice == 2) {
        // --- กรณีเลือกสกิล 1 หรือ 2 ปกติ ---
      playerSkillIndex = (choice == 1 ? playerSkill1 : playerSkill2);
      playerLastUnused = (choice == 1 ? playerSkill2 : playerSkill1);
        playerSkillEffective = &player.skills[playerSkillIndex]; // ชี้ไปที่สกิลโจมตี

      // Roll new skill to replace used one
      getSkills(&player, &playerSkill1, &playerSkill2, &playerSkill3, playerLastUnused,
                   player.numSkills);
    } else {
      // Player can't act, just pick a random skill for defensive purposes
      playerSkillIndex = playerSkill1;
    }
  }   // closes if (!IsplayerUnableToAct)

      // --- คำนวณค่าสำหรับ Player ---
      if (IsplayerUnableToAct) {
          SkillStats dummy = player.defenseSkill[0]; // โคลนค่าจากท่าป้องกัน 0
          getEffectiveSkill(&player, &enemy, &dummy, &playerTempOffense, &playerTempDefense);
          playerSkillEffective = NULL; // บังคับเป็น NULL ตามที่ต้องการ
      } else {
          playerSkillEffective = getEffectiveSkill(&player, &enemy, playerSkillEffective,
                                                  &playerTempOffense, &playerTempDefense);
      }

      // --- คำนวณค่าสำหรับ Enemy ---
      if (IsenemyUnableToAct) {
          SkillStats dummyE = enemy.defenseSkill[0]; // โคลนท่าป้องกันศัตรู
          getEffectiveSkill(&enemy, &player, &dummyE, &enemyTempOffense, &enemyTempDefense);
          enemySkillEffective = NULL; // บังคับเป็น NULL
      } else {
          enemySkillEffective = getEffectiveSkill(&enemy, &player, enemySkillEffective,
                                                  &enemyTempOffense, &enemyTempDefense);
      }

      int playerGoesFirst = 0;

      int pType = (playerSkillEffective != NULL) ? playerSkillEffective->skillType : -1;
      int eType = (enemySkillEffective != NULL) ? enemySkillEffective->skillType : -1;

      if (eType == 3) { 
        // ถ้าศัตรูใช้ Counter (Type 3) เราต้องตีก่อนเสมอ
        playerGoesFirst = 1;
      } else if (pType == 3) { 
        // ถ้าเราใช้ Counter (Type 3) ศัตรูต้องตีก่อนเสมอ
        playerGoesFirst = 0;
      } else if (player.Speed > enemy.Speed) {
        playerGoesFirst = 1;
      } else if (enemy.Speed > player.Speed) {
        playerGoesFirst = 0;
      } else {
        playerGoesFirst = (rand() % 2 == 0);
      }

    // Handle different combat scenarios based on who can act
      if (IsplayerUnableToAct && (!IsenemyUnableToAct && enemySkillEffective != NULL && enemySkillEffective->skillType == 0)) {

        attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
                    enemyTempDefense, &player, playerSkillEffective,
                    playerTempOffense, playerTempDefense,
                    enemySkillEffective->Coins, 0, 0);

      } else if ((!IsplayerUnableToAct && playerSkillEffective != NULL && playerSkillEffective->skillType == 0) && IsenemyUnableToAct) {
        // Enemy is panicked (punching bag), player attacks freely

        attackPhase(&player, playerSkillEffective, playerTempOffense,
                    playerTempDefense, &enemy, enemySkillEffective, enemyTempOffense,
          enemyTempDefense,
                    playerSkillEffective->Coins, 0, 0);

      } else if (!IsenemyUnableToAct && !IsplayerUnableToAct) {

        int canPlayerClash = (playerSkillEffective != NULL) && 
                             (pType == 0 || pType == 4 || pType == 5) && !IsplayerUnableToAct;
        int canEnemyClash  = (enemySkillEffective != NULL) && 
                             (eType == 0 || eType == 4 || eType == 5) && !IsenemyUnableToAct;

        int willClash = (playerSkillEffective != NULL && enemySkillEffective != NULL) &&
                        playerSkillEffective->Clashable && 
                        enemySkillEffective->Clashable && 
                        canPlayerClash && canEnemyClash;

         if (!willClash) {

           if (pType == 0 || eType == 0) {

             // ถ้าเป็น Guard (Type 1) ให้ทอยโล่ทันที
             if (pType == 1) {
             defensePhase(&player, playerSkillEffective, &enemy, enemySkillEffective);
             }

             if (eType == 1) {
                 defensePhase(&enemy, enemySkillEffective, &player, playerSkillEffective);
             }

          if (playerGoesFirst == 1) {

            if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0)) {
        attackPhase(&player, playerSkillEffective, playerTempOffense,
          playerTempDefense, &enemy, enemySkillEffective, enemyTempOffense,
        enemyTempDefense,
          playerSkillEffective->Coins, 0, 0);
            }

            if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0 || enemySkillEffective->skillType == 3)) {
          attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
            enemyTempDefense, &player, playerSkillEffective,
            playerTempOffense, playerTempDefense,
            enemySkillEffective->Coins, 0, 0);
            }

        } else if (playerGoesFirst == 0) {

            if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0)) {
          attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
            enemyTempDefense, &player, playerSkillEffective,
            playerTempOffense, playerTempDefense,
            enemySkillEffective->Coins, 0, 0);
            }

            if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0 || playerSkillEffective->skillType == 3)) {
          attackPhase(&player, playerSkillEffective, playerTempOffense,
            playerTempDefense, &enemy, enemySkillEffective, enemyTempOffense,
          enemyTempDefense,
            playerSkillEffective->Coins, 0, 0);
            }

        }
           }

    } else if (playerSkillEffective->skillType == 0 || enemySkillEffective->skillType == 0) {

      // Normal clash - both can act
      ClashResult clash =
          clashPhase(&player, playerSkillEffective, playerTempOffense,
                     playerTempDefense, &enemy, enemySkillEffective,
                     enemyTempOffense, enemyTempDefense, &player, 0, 0);

      //attackPhase(Character *attacker, SkillStats *atk, int atkTempOffense,
       //int atkTempDefense, Character *defender, SkillStats *defSkill,
       //int defTempOffense, int defTempDefense, int remainingCoins,
       //int Unbreakable, int clashCount)
      if (clash.winner == 1) {
        if (clash.playerskillUsed->skillType == 4) {
          // --- [Clashable Guard Win Effect] ---
          enemy.Tremor[4] += clash.playerFinalPower;
          printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                  player.name, player.name, enemy.name, clash.playerFinalPower);
          sleep(1);
          if (enemy.Tremor[4] > 100 && enemy.Stagger <= 0) {
            enemy.Stagger += 2;
            printf("\n%s Staggered for one turn\n", enemy.name);
            sleep(1);
            enemy.Tremor[4] = 0;
          }

          // เมื่อ Guard ชนะ จะไม่เกิดการ attackPhase ปกติ (เพราะเป็นสกิลป้องกัน)
        } else {
          attackPhase(
              &player, clash.playerskillUsed, clash.playerTempOffense,
              clash.playerTempDefense, &enemy, clash.enemyskillUsed,
              clash.enemyTempOffense, clash.enemyTempDefense,
              (clash.playerskillUsed->Unbreakable > 0)
                  ? ((clash.playerCoins > clash.playerskillUsed->Unbreakable)
                         ? clash.playerCoins : clash.playerskillUsed->Unbreakable)
                  : clash.playerCoins,
              clash.playerUnbreakableLost,
              clash.ClashCount);
        }
      } else if (clash.winner == 2) {
        if (clash.enemyskillUsed->skillType == 4) {
          // --- [Clashable Guard Win Effect] ---
          player.Tremor[4] += clash.enemyFinalPower;
          printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                  enemy.name, enemy.name, player.name, clash.enemyFinalPower);
          sleep(1);
          if (player.Tremor[4] > 50 && player.Stagger <= 0) {
            player.Stagger += 2;
            printf("\n%s Staggered for one turn\n", player.name);
            sleep(1);
            player.Tremor[4] = 0;
          }

          // เมื่อ Guard ชนะ จะไม่เกิดการ attackPhase ปกติ (เพราะเป็นสกิลป้องกัน)
        } else {
          attackPhase(
              &enemy, clash.enemyskillUsed, clash.enemyTempOffense,
              clash.enemyTempDefense, &player, clash.playerskillUsed,
              clash.playerTempOffense, clash.playerTempDefense,
              (clash.enemyskillUsed->Unbreakable > 0)
                  ? ((clash.enemyCoins > clash.enemyskillUsed->Unbreakable)
                         ? clash.enemyCoins : clash.enemyskillUsed->Unbreakable)
                  : clash.enemyCoins,
              clash.enemyUnbreakableLost,
              clash.ClashCount);
        }
      }

      }
    }   // closes combat chain (if/else if/else for IsplayerUnableToAct)

    printf("\n--- Turn End ---\n");

    // 1. จบเทิร์นของผู้เล่น (จัดการกระสุน/Sanity/ล้างบัฟผู้เล่น)
    handleTurnEnd(&player, &enemy, playerSkillEffective, enemySkillEffective);

    // 2. จบเทิร์นของบอส (จัดการความสามารถบอส/ล้างบัฟบอส)
    handleTurnEnd(&enemy, &player, enemySkillEffective, playerSkillEffective);

    TurnCount++;
  }   // closes while (player.HP > 0 && enemy.HP > 0)

}   // closes else block

  printf("\n--- Battle Result ---\n");
  if (player.HP <= 0 && enemy.HP <= 0) {
    printf("It's a draw!\n");
  } else if (player.HP <= 0) {
    printf("You lost! %s defeated you.\n", enemy.name);
  } else if (enemy.HP <= 0) {
    printf("Victory! You defeated %s.\n", enemy.name);
  }

  return 0;

}   // closes main
