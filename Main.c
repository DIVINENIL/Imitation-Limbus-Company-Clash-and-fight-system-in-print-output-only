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

  // Buff
  int CoinPowerBoost;
  int FinalPowerBoost;
  int AttackPowerBoost;
  int DefensePowerBoost;
  int BasePowerBoost;
  int ClashPower;
  float DamageUp;
  float Protection;
  int Paralyze;
  float DmgMutiplierBoost;
  int OffenseBoost;
  int DefenseBoost;

  int Bind[2]; // [0] = this turn, [1] = Next turn
  int Haste[2]; // [0] = this turn, [1] = Next turn

  // BuffNextTurn
  int CoinPowerBoostNextTurn;
  int FinalPowerBoostNextTurn;
  int AttackPowerBoostNextTurn;
  int DefensePowerBoostNextTurn;
  int BasePowerBoostNextTurn;
  int ClashPowerNextTurn;
  float DamageUpNextTurn;
  float ProtectionNextTurn;
  int ParalyzeNextTurn;
  float DmgMutiplierBoostNextTurn;
  int OffenseBoostNextTurn;
  int DefenseBoostNextTurn;

  // Status
  int Burn[2]; // [0] = Burn Stack, [1] = Burn Count
  int Bleed[2]; // [0] = Bleed Stack, [1] = Bleed Count
  int Tremor[3]; // [0] = Tremor Stack, [1] = Tremor Count, [2] = Tremor Burst Stack
  int Rupture[2]; // [0] = Rupture Stack, [1] = Rupture Count
  int Sinking[2]; // [0] = Sinking Stack, [1] = Sinking Count
  int Poise[2]; // [0] = Poise Stack, [1] = Poise Count
  int Charge[2]; // [0] = Charge Stack, [1] = Charge Count

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

//---------------------Buff system-----------------
void initializeCharacterBuffs(Character *c) {
  // Current turn buffs
  c->CoinPowerBoost = 0;
  c->FinalPowerBoost = 0;
  c->AttackPowerBoost = 0;
  c->DefensePowerBoost = 0;
  c->BasePowerBoost = 0;
  c->ClashPower = 0;
  c->DamageUp = 0;
  c->Protection = 0;
  c->Paralyze = 0;
  c->DmgMutiplierBoost = 0;
  c->OffenseBoost = 0;
  c->DefenseBoost = 0;

  c->SanityFreezeTurns = 0;

  c->Stagger = 0;

  c->Bind[0] = c->Bind[1] = 0;
  c->Haste[0] = c->Haste[1] = 0;

  // Next turn buffs
  c->CoinPowerBoostNextTurn = 0;
  c->FinalPowerBoostNextTurn = 0;
  c->AttackPowerBoostNextTurn = 0;
  c->DefenseBoostNextTurn = 0;
  c->BasePowerBoostNextTurn = 0;
  c->ClashPowerNextTurn = 0;
  c->DamageUpNextTurn = 0;
  c->ProtectionNextTurn = 0;
  c->ParalyzeNextTurn = 0;
  c->DmgMutiplierBoostNextTurn = 0;
  c->OffenseBoostNextTurn = 0;
  c->DefenseBoostNextTurn = 0;
}

void clearTurnEffects(Character *c) {
  c->TempShield = 0;
  
  c->CoinPowerBoost = 0;
  c->FinalPowerBoost = 0;
  c->AttackPowerBoost = 0;
  c->DefensePowerBoost = 0;
  c->BasePowerBoost = 0;
  c->ClashPower = 0;
  c->DamageUp = 0;
  c->Protection = 0;
  c->Paralyze = 0;
  c->DmgMutiplierBoost = 0;
  c->OffenseBoost = 0;
  c->DefenseBoost = 0;

  c->Bind[0] = 0;
  c->Haste[0] = 0;

  c->Bind[0] = c->Bind[1];
  c->Bind[1] = 0;
  c->Haste[0] = c->Haste[1];
  c->Haste[1] = 0;

    c->CoinPowerBoost = c->CoinPowerBoostNextTurn;
    c->FinalPowerBoost = c->FinalPowerBoostNextTurn;
    c->BasePowerBoost = c->BasePowerBoostNextTurn;
    c->AttackPowerBoost = c->AttackPowerBoostNextTurn;
  c->DefensePowerBoost = c->DefensePowerBoostNextTurn;
    c->ClashPower = c->ClashPowerNextTurn;
    c->DamageUp = c->DamageUpNextTurn;
    c->Protection = c->ProtectionNextTurn;
    c->Paralyze += c->ParalyzeNextTurn;
    c->DmgMutiplierBoost = c->DmgMutiplierBoostNextTurn;
    c->OffenseBoost = c->OffenseBoostNextTurn;
    c->DefenseBoost = c->DefenseBoostNextTurn;
  

  c->CoinPowerBoostNextTurn = 0;
  c->FinalPowerBoostNextTurn = 0;
  c->AttackPowerBoostNextTurn = 0;
  c->DefensePowerBoostNextTurn = 0;
  c->BasePowerBoostNextTurn = 0;
  c->ClashPowerNextTurn = 0;
  c->DamageUpNextTurn = 0;
  c->ProtectionNextTurn = 0;
  c->ParalyzeNextTurn = 0;
  c->DmgMutiplierBoostNextTurn = 0;
  c->OffenseBoostNextTurn = 0;
  c->DefenseBoostNextTurn = 0;
}

void clearTurnSkillBuffs(Character *c) {
  c->CoinPowerBoost = 0;
  c->FinalPowerBoost = 0;
  c->AttackPowerBoost = 0;
  c->BasePowerBoost = 0;
  c->ClashPower = 0;
  c->DamageUp = 0;
  c->DmgMutiplierBoost = 0;
  c->OffenseBoost = 0;
  c->DefenseBoost = 0;
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

// ฟังก์ชันกลางสำหรับหักดาเมจเข้าเกราะและเลือด // true damage = 1, normal damage = 0
void applyDamage(Character *defender, int damage, int trueDamage) {
    if (damage <= 0) return;

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
    
} else {
    
    // HP
    defender->HP -= damage;
    if (defender->HP < 0) defender->HP = 0;

    // Heishou Pack - You Branch Adept Heathcliff save for lost HP
    if (isId(defender->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0) {
        defender->skills[6].active += (int)(damage); 
    }
    
}

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

void TremorBurst(Character *attacker, Character *defender, int TremorBurstStagger, int *totalDamage, int PrintType) {
  
  int deal = attacker->skills[3].active;

  applyDamage(defender, deal, 0);

  if (totalDamage != NULL) {
    *totalDamage += deal;
  }

    defender->Tremor[1] -= 1;
  if (defender->Tremor[1] <= 0) defender->Tremor[1] = 0;

  if (PrintType == 1) {

      printf(" \tTrigger 'Tremor Burst' on target (Stack %d Count %d)", defender->Tremor[0], defender->Tremor[1]);

  } else {

    printf("\n%s triggers 'Tremor Burst' on target (Stack %d Count %d)\n", attacker->name, defender->Tremor[0], defender->Tremor[1]);
    
  }

  if (defender->Tremor[1] <= 0) defender->Tremor[0] = 0;

  if (defender->Tremor[2] > TremorBurstStagger && defender->Stagger <= 0) {

    defender->Stagger += 2;

    if (PrintType == 1) {

      printf(" \tTarget 'Stagger' for one turn");

    } else {

      printf("\n%s Staggered for one turn\n", defender->name);

    }

      defender->Tremor[2] = 0;

  }
}


//--------------------------Sanity functions----------------------------

// Helper to handle Sanity changes, locking, and clamping
void updateSanity(Character *c, int delta) {

  if (isId(c->ID, "Muga Ryōshū") == 0 && (c->Sinking[0] > 0 || c->Sinking[1] > 0)) {
      if (delta < 0) {
          int hpDmg = abs(delta) * 3;
          applyDamage(c, hpDmg, 1); // True damage
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

    return "PANIC";
  }
  if (c->Sanity <= -30)
  return "Low Morale";
  return "Normal";
}

// Apply Low Morale/Panic debuff
void applySanityDebuff(int *offense, int *defense, Character *c) {

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

      c->DamageUp += 10;
      c->FinalPowerBoost += 1;
      c->Protection -= 10;

      printf("\nWhile 'Low Morale', %s gains Final Power +1, deal 10%% more "
             "damage, take 10%% more damage.\n",
             c->name);
      sleep(1);

    } else if (isId(c->ID, "Sancho:The Second Kindred of Don Quixote") == 0) {
      *offense += 2;
      c->sanityGainBase = 10;

      printf(
          "\nWhile 'Low Morale', %s gains Offense +2, Sanity Heal Efficiency +4.\n",
          c->name);
      sleep(1);

    } else if (isId(c->ID, "Erlking Heathcliff") == 0) {
      c->DamageUp += 30;
      *defense -= 3;
      updateSanity(c, 15);

      printf("\n%s heals 15 Sanity\n", c->name);

      sleep(1);
      
      printf("\nWhile 'Low Morale', %s gains 30%% more damange, Defense -3.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Fixer grade 9?") == 0) {
      c->ClashPower += 2;
      *defense -= 3;
      c->sanityGainBase = -12; // Negative gain = loss on win

      printf("\nWhile 'Low Morale', %s gains 2 Clash Power Up, Defense -3, Sanity Loss Efficiency +6.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Jia Qiu") == 0) {
      
      c->DamageUp += 10;

      printf(
          "\nWhile 'Low Morale', %s deals 10%% more damange.\n",
          c->name);
      sleep(1);

    } else if (isId(c->ID, "Sukuna:King of Curse") == 0) {

      c->DamageUp += 30;

      printf(
          "\nWhile 'Low Morale', %s deals 30%% more damange.\n",
          c->name);
      sleep(1);

    }
    
  }

  // PANIC (-45 Sanity)
  if (c->Sanity <= -45) {
    if (isId(c->ID, "Lei heng") == 0) {

      c->DamageUp += 20;
      c->CoinPowerBoost += 2;
      c->Protection -= 20;

      printf("\nWhile 'Beastly Instinct', %s gains Coin Power +2, deal 20%% more damage, "
             "take 20%% more damage.\n",
             c->name);

      sleep(1);

    } else if (isId(c->ID,
                          "Sancho:The Second Kindred of Don Quixote") == 0) {
      *offense += 3;
      *defense += 6;

      printf("\nWhile 'Reawakening Joy Of Carnage', %s gains Offense +3 and Defense +6.\n", c->name);
      sleep(1);

    } else if (isId(c->ID, "Erlking Heathcliff") == 0) {

      c->DamageUp += 50;
      *defense -= 6;

      printf("\nWhile 'Revenge', %s gains 50%% more damage and Defense -6.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Fixer grade 9?") == 0) {

      c->DamageUp += 30;
      *defense -= 5;
      c->AttackPowerBoost += 3;

      printf("\nWhile 'Black Heart', %s gains 30%% more damange, 3 Attack Power Up, Defense -5.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Jia Qiu") == 0) {

      c->DamageUp += 20;

      printf("\nWhile 'Jia Qiu', %s deals 20%% more damage.\n",
             c->name);
      sleep(1);
    } else if (isId(c->ID, "Sukuna:King of Curse") == 0) {

      c->DamageUp += 100;
      c->Protection -= 100;

      printf("\nWhile 'King', %s deals 100%% more damage and take 100%% more damage.\n",
             c->name);
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

    if (totalWeight <= 0) return 1; // Fallback ไป Atk 1

    int r = rand() % totalWeight;
    int cum = 0;

    // ตรวจสอบ Atk 1
    cum += c->skills[s1_idx].Copies;
    if (r < cum) return 1; 

    // ตรวจสอบ Atk 2
    cum += c->skills[s2_idx].Copies;
    if (r < cum) return 2;

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
    
  c2->Protection -= 50;

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

    // 1. ตัดเหรียญและ Glitch ชื่อสกิลศัตรู
  defSkill->name = glitchText(defSkill->name, (100/defSkill->Coins)); 
    defSkill->Coins--;

  if (defSkill->Unbreakable > defSkill->Coins) {
      defSkill->Unbreakable = defSkill->Coins;
  }

  // 2. สุ่มเลือกสกิลของ Ryōshū (Attacker) เพื่อ Glitch ชื่อ
  int glitcheableIndices[MAX_SKILLS];
  int count = 0;

  for (int i = 0; i < attacker->numSkills; i++) {
      // เงื่อนไข: ไม่ใช่ท่าไม้ตาย (index 5), สกิลมีชื่ออยู่จริง, และสกิลยังไม่พัง (Copies >= 0)
      if (i != 5 && attacker->skills[i].name != NULL && attacker->skills[i].Copies >= 0) {
          glitcheableIndices[count++] = i;
      }
  }

  if (count > 0) {
      int randomIdx = glitcheableIndices[rand() % count];
      attacker->skills[randomIdx].name = glitchText(attacker->skills[randomIdx].name, 5); // หายทีละ 10% เพื่อให้เห็นผลชัดขึ้น
  }

    printf("\n\x1b[1;35m[SEVERED]\x1b[0m %s used %s to delete a coin!\n", attacker->name, atkSkill->name);

    // 3. ถ้าสกิลศัตรูพัง (เหรียญหมด)
    if (defSkill->Coins <= 0) {
        defSkill->Copies = -1; 
      
        defender->name = glitchText(defender->name, (100/(defender->numDefenseSkills + defender->numSkills))); // ชื่อศัตรูเริ่มหาย
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

  // Gregor:Firefist - Burn
  if (isId(attacker->ID, "Gregor:Firefist") == 0) {
      attacker->skills[0].active = 0; // Burn Stack
    attacker->skills[1].active = 0; // Burn Count
  }


  // The One Who Grips Faust
  if (isId(attacker->ID, "The One Who Grips Faust") == 0) {
      attacker->skills[0].active = 0; // Bleed Stack
    attacker->skills[1].active = 0; // Bleed Count
    attacker->skills[2].active = 0; // Nail
    attacker->skills[4].active = 0; // Gaze
  }
  

  // Muga Ryōshū
  if (isId(attacker->ID, "Gregor:Firefist") == 0) {
      attacker->skills[0].active = 0; // Sever the thread
  }


  // ------------ Boss ------------

    // King in Binds - Sinking & Tremor
    if (isId(attacker->ID, "King in Binds") == 0) {
        attacker->skills[1].active = 0; // Sinking Stack
        attacker->skills[2].active = 0; // Sinking Count
        attacker->skills[3].active = 0; // Tremor Stack
        attacker->skills[4].active = 0; // Tremor Count
    }

}











void defensePhase(Character *c, SkillStats *ds) {
    if (c->HP <= 0 || isStaggered(c)) return;

    printf("\n--- Defense Phase ---\n");
    printf("%s prepares defense with %s\n", c->name, ds->name);

    // แสดงรายละเอียดเหรียญ
    printf("Tossing %d Coins for Shield:\n", ds->Coins);
    printf("%-10s %-10s %-10s", "Coin", "Power", "Shield");

    int totalPower = ds->BasePower + c->BasePowerBoost;

    for (int i = 0; i < ds->Coins; i++) {
        int isHead = tossCoinWithSanity(c);
        int coinPowerResult = 0;

        if (isHead) {
            // เช็ค Paralyze (อัมพาต) เหมือนตอนโจมตี
            if (c->Paralyze > 0) {
                coinPowerResult = 0;
                c->Paralyze--;
            } else {
                coinPowerResult = ds->CoinPower + c->CoinPowerBoost;
            }
        }

        totalPower += coinPowerResult;
        if (totalPower < 0) totalPower = 0;

        // ใน Limbus Company ค่าพลังที่ทอยได้ของ Guard คือค่า Shield ที่ได้รับ
        printf("\n%-10d %-10d %-10d", i + 1, totalPower, totalPower);

        usleep(500000); // หน่วงเวลาเพื่อให้ดูเหมือนทอยจริง
    }

    c->TempShield += totalPower;
    printf("\n%s gained %d Shield! (Current Total Shield: %.2f)\n", 
           c->name, totalPower, c->Shield + c->TempShield);

    sleep(1);
}











// ----------------------Attack phase-------------------------------
void attackPhase(Character *attacker, SkillStats *atk, int atkTempOffense,
                 int atkTempDefense, Character *defender, SkillStats *defSkill,
                 int defTempOffense, int defTempDefense, int remainingCoins,
                 int Unbreakable, int clashCount) {
  // printf("\n%s attacks %s with %s\n", attacker->name, defender->name,
  // atk->name);

  if (attacker->HP > 0 && !isStaggered(attacker)) {

  if (remainingCoins <= 0) {
    printf("\nNo coins left to attack.\n");
    return;
  }

    int ClashLostAttack = 0; // ← Character's Cracking Coins in after attack
    int initialCrackedCount = Unbreakable; // save for use in some character's effect (This value isn't same as 'Unbreakable' in this function cause 'Unbreakable' is change in this function by -1 every loop but this doesn't change)

    if (atk->Unbreakable > 0 && Unbreakable == atk->Unbreakable) {
      ClashLostAttack = 1;
    }

    // Clashable Guard
    int powerReduction = 0;
    if (defSkill->skillType == 4 && !isStaggered(defender) && !ClashLostAttack) {
        printf("\n--- Defense Phase ---\n");
        printf("%s prepare to mitigate damage with %s: %s (Cracking Coin fixed Coin Power to 1)\n", defender->name, getSkillTypeName(defSkill->skillType), defSkill->name);

        // 1. คำนวณพลังพื้นฐาน (Base + Level Bonus + Buff)
        powerReduction = defSkill->BasePower + defender->BasePowerBoost + defender->DefensePowerBoost + defender->FinalPowerBoost;

        int defenseDiff = defTempDefense - atkTempOffense;
        if (defenseDiff > 0) {
            powerReduction += (defenseDiff / 3);
        }

        // 2. ช่วง Tossing Coins (Visual Part)
        printf("Tossing %d Coins (Fixed Coin Power: 1):\n", defSkill->Coins);
        printf("%-10s %-10s %-10s", "Coin", "Result", "Power");

        for (int j = 0; j < defSkill->Coins; j++) {
            int isHead = tossCoinWithSanity(defender);
            if (isHead) {
                if (attacker->Paralyze > 0) { // ← Character's paralyze
                    powerReduction += 0;
                    defender->Paralyze--;
                  }
                } else {
                    powerReduction += 1 + defender->CoinPowerBoost;
                  if (powerReduction <= 0) powerReduction = 0;
                }
          
            printf("\n%-10d %-10s %-10d", j + 1, (isHead ? "Heads" : "Tails"), powerReduction);
            usleep(400000); 
        }
      
        printf("\n(%d bonus) %s total Defense power: %d\n", ((defenseDiff > 0) ? (defenseDiff / 3) : 0) + defender->DefensePowerBoost + defender->FinalPowerBoost, defender->name, powerReduction);
        sleep(1);
    }

  //--------------------------- Before Attack Buff ----------------------------

    // Heishou Pack - You Branch Adept Heathcliff - Battleblood Instinct buff
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && attacker->Passive > 0) {

    float gain = attacker->Passive * 0.75f;

      attacker->DamageUp += gain;

      printf("\n%s deals +0.75%% damage(%.2f%%) for every Battleblood Instinct Stack (%d)\n", attacker->name, gain, attacker->Passive);

    sleep(1);
    }

    // The Middle Little Brother Sinclair - Passive Buff Mark
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && attacker->skills[0].active > 0) {

      int takevalue = attacker->skills[0].active * 2;
      if (takevalue > 20) takevalue = 20;

       defender->Protection -= takevalue;

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
    
      defender->Protection += 80;

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

    attacker->FinalPowerBoost += 3;

    printf("\n%s at 45 Sanity, gains 3 Final Power\n",
           attacker->name);

    sleep(1);

  }
    
    if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && attacker->Sanity >= 45) {

      attacker->CoinPowerBoost += 3;

      printf("\n%s at 45 Sanity, gains 3 Coin Power\n",
             attacker->name);

      sleep(1);

    }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S2 45 sp
  if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && attacker->Sanity >= 45 && (atk == &attacker->skills[1])) {

    attacker->AttackPowerBoost += 2;

    printf("\n%s at 45 Sanity, gains 2 Attack Power\n",
           attacker->name);

    sleep(1);

  }

    // Dawn Office Fixer Sinclair - Skill Buff ego form S3 45 sp
  if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && attacker->Sanity >= 45 && (atk == &attacker->skills[3] || atk == &attacker->skills[2])) {

    attacker->AttackPowerBoost += 15;

    printf("\n%s at 45 Sanity, gains 15 Attack Power\n",
           attacker->name);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S4
  if (isId(attacker->ID, "Dawn Office Fixer Sinclair") == 0 && (atk == &attacker->skills[3] || atk == &attacker->skills[2]) && attacker->skills[3].active) {

      int boost = (abs(attacker->Sanity)) * 5;

      attacker->DamageUp += boost;

      printf("\n%s gains +5%% Damage (%d%%) for every Sanity further from 0 (%d Sanity)\n",
             attacker->name, boost, attacker->Sanity);

      sleep(1);

  }

    // ------------------------------------------------------------------------

  // Meursault:The Thumb Unbreakable BUff
  if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (attacker->skills[2].active >= 14) && attacker->Passive <= 0 && attacker->skills[3].active && (Unbreakable == atk->Coins)) {

    float missing = (attacker->MAX_HP - attacker->HP) / attacker->MAX_HP; // fraction of HP missing (0.0 - 1.0)
    int SkillUp = ((int)(missing * 100.0f)) + 75;  // 75% + missing
    if (SkillUp > 150) SkillUp = 150;      // cap at 50%

      attacker->DamageUp += SkillUp;

       printf("\n%s at 14+ Rounds spent: On Clash Lose, Unbreakable Coins of this unit's Attack Skills deal +(75 + missing HP percentage on self)%% damage(%d%% - Max 150%%)\n", attacker->name, SkillUp);

    sleep(1);

  }

    // Meursault:The Thumb 20+ BUff
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (attacker->skills[2].active >= 14) && attacker->Passive <= 0 && attacker->skills[3].active) {
    float TargetHP = (defender->HP / defender->MAX_HP) * 100; // fraction of HP missing (0.0 - 1.0)
      float UnitHP = (attacker->HP / attacker->MAX_HP) * 100; // fraction of HP missing (0.0 - 1.0)

      if (attacker->skills[2].active >= 20 && TargetHP > UnitHP) {

        float missingdifferent = TargetHP - UnitHP;

          int SkillUp = (int)(missingdifferent * 100);
          if (SkillUp > 50) SkillUp = 50;      // cap at 50%

          attacker->DamageUp += SkillUp;

        printf("\n%s at 20+ Rounds spent: Deal +(HP percentage difference)%% damage against targets with higher remaining HP percentage than this unit (%d%% - Max 50%%)\n", attacker->name, SkillUp);

      }

      sleep(1);

    }

  // Jia Qiu - S3 and S6 S10
  if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[10] || atk == &attacker->skills[3] || atk == &attacker->skills[6])) {

    int boost = rand() % 30 + 1;
    
      attacker->DamageUp += boost;

    printf("\n%s deals %d%% more damage\n", attacker->name, boost);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
     (atk == &attacker->skills[2])) {

    int gain = 12 * attacker->Passive;

    attacker->DamageUp += gain;

    printf("\n%s deals 12%% more damage(%d%%) for every Coffin (%d)\n",
       attacker->name, gain, attacker->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
     (atk == &attacker->skills[3])) {

    int gain = 10 * attacker->Passive;

    attacker->DamageUp += gain;

    printf("\n%s deals 10%% more damage(%d%%) for every Coffin (%d)\n",
       attacker->name, gain, attacker->Passive);

      sleep(1);
  }

    // Heathcliff:Wild Hunt – buff Dullahan
    if (isId(attacker->ID, "Heathcliff:Wild Hunt") == 0 &&
       (atk == &attacker->skills[3])) {

      int gain = 20 * attacker->skills[0].active;

      attacker->DamageUp += gain;

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
      attacker->DamageUp += 10;
      printf("\n%s consumes %d District 12 Fuel(%d) to deal more 10%% damage\n", attacker->name, consumed, attacker->Passive);
      
      } else if (prevPassive > 50 && attacker->Passive <= 50) {
      attacker->DamageUp += 10;
       printf("\n%s consumes %d District 12 Fuel(%d) to deal more 10%% damage\n", attacker->name, consumed, attacker->Passive);
      sleep(1);
          printf("\nDistrict 12 Fuel becomes Overheated Fuel\n");
      } else {
      attacker->DamageUp += 20;
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
    attacker->DamageUp += 15;
    printf("\n%s consumes %d District 12 Fuel(%d) to deal more 15%% damage\n", attacker->name, consumed, attacker->Passive);

    } else if (prevPassive > 50 && attacker->Passive <= 50) {
    attacker->DamageUp += 15;
     printf("\n%s consumes %d District 12 Fuel(%d) to deal more 15%% damage\n", attacker->name, consumed, attacker->Passive);

    sleep(1);
    
        printf("\nDistrict 12 Fuel becomes Overheated Fuel\n");
    } else {
    attacker->DamageUp += 30;
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
    attacker->DamageUp += boost;

     printf("\n%s consumes up to 25 District 12 Fuel(%d) to deal +2%% damage for every District 12 Fuel consumed (%d%% - Max 50%%)\n", attacker->name, ((attacker->Passive - consumed) < 0) ? 0 : (attacker->Passive - consumed), boost);

    } else if (prevPassive > 50 && (attacker->Passive - consumed) <= 50) {
    
    if (boost > 50) boost = 50;                    // cap at 50%
    attacker->DamageUp += boost;
    
     printf("\n%s consumes up to 25 District 12 Fuel(%d) to deal +2%% damage for every District 12 Fuel consumed (%d%% - Max 50%%)\n", attacker->name, ((attacker->Passive - consumed) < 0) ? 0 : (attacker->Passive - consumed), boost);

    sleep(1);
      
        printf("\nDistrict 12 Fuel becomes Overheated Fuel\n");
    } else {
    
    boost = (int)((0.04 * consumed) * 100);  // 4% per missing fuel
    if (boost > 100) boost = 100;                    // cap at 100%
    attacker->DamageUp += boost;

      printf("\n%s consumes up to 25 Overheated Fuel(%d) to deal +4%% damage for every District 12 Fuel consumed (%d%% - Max 100%%)\n", attacker->name, ((attacker->Passive - consumed) < 0) ? 0 : (attacker->Passive - consumed), boost);
    
    }
  
  sleep(1);

  if (consumed > 0) {

    attacker->DamageUpNextTurn += 20;

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
    attacker->DamageUp += (int)(boost * 100);

    printf("\n%s deals more damage the further this unit's Sanity value is from 45 (+0.3%% damage for every missing Sanity) (%d%% - Max 21%%)\n",
       attacker->name, (int)(boost * 100));

    sleep(1);
  }

    // ------------------ Meursault:Blade Lineage Mentor ------------------

  // Meursault:Blade Lineage Mentor - Yield my flesh
  if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 &&
      (atk == &attacker->skills[2])) {

      int DamageBuff = 2 * abs(attacker->Sanity);
    if (DamageBuff > 75) DamageBuff = 75;

    attacker->DamageUp += DamageBuff;

      printf("\n%s deals 2%% more damage for further Sanity from 0 (%d%% - Max 75%%)\n", attacker->name, DamageBuff);

      sleep(1);
    
  }

  // Meursault:Blade Lineage Mentor - To claim thier bones
  if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 &&
      (atk == &attacker->defenseSkill[0] || atk == &attacker->defenseSkill[1] || atk == &attacker->skills[2])) {

    float missingHPPercent = ((float)(attacker->MAX_HP - attacker->HP) / attacker->MAX_HP) * 100.0f;

    int fullMissingPercent = (int)missingHPPercent;

    float dmgBonus = (float)fullMissingPercent * 0.5f;

    // 4. Apply the maximum limit
    if (dmgBonus > 25.0f) {
        dmgBonus = 25.0f;
    }

    attacker->DamageUp += dmgBonus;

    printf("\n%s deals +0.5%% damage for every 1%% missing HP on self(%.1f%% - Max 25%%)\n",
        attacker->name, dmgBonus);
  }

    // Meursault:Blade Lineage Mentor - Overthrow skill
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 &&
        (atk == &attacker->defenseSkill[0])) {

     attacker->FinalPowerBoostNextTurn += 1;

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

      attacker->DamageUp += damageboost;

        printf("\n%s deals +1%% damage for every 1%% (missing HP percentage on target + missing HP percentage on self) (%.0f%% - Max 50%%)\n", attacker->name, damageboost);

      sleep(1);
    }

  // Hong lu:The Lord of Hongyuan - S3 Buff
  if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 &&
      (atk == &attacker->skills[3] || atk == &attacker->skills[4])) {

    int buff = 10;

    if (atk == &attacker->skills[3]) buff = 20;
    
    attacker->DamageUp += attacker->Passive*buff;

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

      attacker->CoinPowerBoost += Boost;

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
      attacker->DamageUp -= 50;
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
        
        attacker->DamageUp += Mang*(100/defSkill->Coins);
 
      } else {

        attacker->DamageUp += Mang*(1000/defSkill->Coins);
        
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
      
        attacker->BasePowerBoost -= lost;
       attacker->DamageUp -= lost*0.5f;
      
      printf("\n%s loses 1 Base Power and deal -0.5%% damage for every Cracking Unbreakable Coins (%.0f)\n", attacker->name, lost);

       sleep(1);
    }

    printf("\n%s consumes all Black Silence and gain +5%% Damage (%d%%) for every 3 Black Silence(%d)\n", attacker->name, 5*(attacker->Passive / 3), attacker->Passive);

    attacker->Passive = 0;
    attacker->DamageUp += 5*(attacker->Passive / 3);

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
        if (attacker->skills[5].active == 0) {
            attacker->skills[5].active = 1;
            //printf(" [Prescript II: Hit Success!] ");
        }
    }

     }

  //----------------------------------------------------------------

    // --- [The One Who Grips Faust - Such Filth Setup] ---
    int Evaded = 0;
    int IsStillEvaded = 0;
    int evadePower = 0;

    // Evade Skill
    if (defSkill->skillType == 2) {
        Evaded = 1;
      IsStillEvaded = 1;

      printf("\n%s uses 'Evade Skill' (%s)\n", defender->name, defSkill->name);

      sleep(1);

    }

    int fanaticUsed = 0;
    
    // เช็คว่า Faust เป็นฝ่ายรับ และมี Fanatic (Passive) หรือไม่
    if (isId(defender->ID, "The One Who Grips Faust") == 0 && defender->Passive > 0 && Evaded == 0) {
        Evaded = 1;
      IsStillEvaded = 1;
        fanaticUsed = defender->Passive;

      printf("\n%s consumes all Fanatic (%d) to use 'Such Filth'\n", defender->name, defender->Passive);

      defender->Passive = 0; // จ่าย Fanatic ทั้งหมดทันที

      sleep(1);

    }


  

  printf("\n--- Attack Phase ---\n");
    
  int totalPower = atk->BasePower + attacker->BasePowerBoost;
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

  for (int i = 0; i < remainingCoins; i++) {

    if (attacker->HP > 0 && defender->HP > 0) {

    // --------------------------- Coin Buff Section ------------------------------------------

    int CoinBuff = 0;

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
      if (attacker->Paralyze > 0) { // ← Character's paralyze
        totalPower += 0;
        if (isId(attacker->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && atk == &attacker->skills[3]) {
          totalPower += atk->CoinPower; // เพิ่มพลังตามปกติแม้ติดอัมพาต
        } else {
          totalPower += 0; // สกิลอื่นโดน Paralyze ปกติ
          attacker->Paralyze--;
        }
      } else {
        totalPower += CoinBuff + atk->CoinPower + attacker->CoinPowerBoost;
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

        applyDamage(attacker, attacker->skills[0].active, 0);
        
        if (attacker->HP < 1)
            attacker->HP = 1;

           printf("\n%s Coins On Head Hit, at 50%%+ HP, take Burn damage by Burn Stack on self (%d)", attacker->name,attacker->skills[0].active);

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

    int bonus = 0;

    // Last coin power bonus (from character buffs only)
    if (i == remainingCoins - 1) {
      if (atk->skillType == 0) {
        bonus = attacker->FinalPowerBoost + attacker->AttackPowerBoost;
      } else {
        bonus = attacker->FinalPowerBoost + attacker->DefensePowerBoost;
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

    if (defSkill->skillType == 4 && powerReduction > 0) {
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
    if (attacker->Poise[0] > 0 || attacker->Poise[1] > 0) {

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

      attacker->DamageUp += boost;

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

      attacker->DamageUp += boost;

      printf("\n%s's last coin deals +15%% damage(%d%% - Max 75%%) for every 3 HardBlood (%d)", attacker->name, boost, attacker->Passive);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Skill 2-2 dmg
    if (isId(attacker->ID, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[4]) && i == remainingCoins - 1 && attacker->Passive >= 2) {

      int boost = (attacker->Passive/3) * 20;
      if (boost > 150) boost = 150;

      attacker->DamageUp += boost;

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
          attacker->DamageUp += boost;

           printf("\n%s's last coin deal +2%% more damage for every District 12 Fuel consumed by this Skill (%d%% - Max 50%%)", attacker->name, boost);

          } else if (prevPassive > 50 && attacker->Passive <= 50) {

          if (boost > 50) boost = 50;                    // cap at 50%
          attacker->DamageUp += boost;

           printf("\n%s's last coin deal +2%% more damage for every District 12 Fuel consumed by this Skill (%d%% - Max 50%%)", attacker->name, boost);

          } else {

          boost = (int)((0.04 * consumed) * 100);  // 4% per missing fuel
          if (boost > 100) boost = 100;                    // cap at 100%
          attacker->DamageUp += boost;

           printf("\n%s's last coin deal +4%% more damage for every Overheated Fuel consumed by this Skill (%d%% - Max 100%%)", attacker->name, boost);
          }

        sleep(1);
      }

    // Jia Qiu – buff s5 and s13 S15
      if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[5] || atk == &attacker->skills[13] || atk == &attacker->skills[15]) && i == remainingCoins - 1) {

          int boost = abs((int)(5 * attacker->Sanity));  // 4% per missing fuel
          if (boost > 200) boost = 200;                    // cap at 100%
          attacker->DamageUp += boost;

           printf("\n%s's last coin deal +5%% damage for the further this unit's Sanity from 0 (%d%% - Max 200%%)", attacker->name, boost);

        sleep(1);
      }

    // Jia Qiu - S11
    if (isId(attacker->ID, "Jia Qiu") == 0 && (atk == &attacker->skills[11] && i == remainingCoins - 1)) {

      int boost = abs(defender->Sanity);
      if (boost > 10) boost = 10;
      
          Damage += boost;

      printf("\n%s deal additional damage equal to the further enemy's Sanity from 0 (%d - Max 10)", attacker->name, boost);

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

        printf("\n%s deals +(HP percentage the first Coin removed from the main target x 3)%% damage (%d%% - Max 100%%)",
               attacker->name, gain);

        sleep(1);

        }

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
                 (atk == &attacker->skills[7])) {

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
        (atk == &attacker->skills[3]) && i == 0) {

      printf("\n\n%s: \"Eyes up here, boys! Don'tcha go losin' yer heads now!\"\n",
             attacker->name);

      sleep(1);
    }

    // Lei heng – skill 6
    if (isId(attacker->ID, "Lei heng") == 0 &&
        (atk == &attacker->skills[4]) && i == 0) {

      printf("\n\n%s: \"Y'all don't go on huntin' tigers without preparin' yerselves to get chomped 'tween one of them jaws!\"\n",
             attacker->name);

      sleep(1);
    }

    //-----------------------------------------

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

    // Meursault:The Thumb S1-1
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[0]) && i == remainingCoins - 1 && attacker->Passive > 0 && !attacker->skills[3].active) {

        attacker->Passive--;
        attacker->skills[2].active++;
        attacker->DamageUp += 10;

        Damage += (Damage * 0.10f);

         printf("\n%s spends 1 Tigermark Round(%d) to gain +10%% damage this turn (Coin that spent Tigermark Round +1 Power and deal +10%% damage)", attacker->name, attacker->Passive);

      sleep(1);
      
    } // Meursault:The Thumb S1-2
    else if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[0]) && i == remainingCoins - 1 && attacker->Passive > 0 && attacker->skills[3].active) {

        attacker->Passive--;
        attacker->skills[2].active++;
        attacker->DamageUp += 20;

        Damage += (Damage * 0.30f);

         printf("\n%s spends 1 Savage Tigermark Round(%d) to gain +20%% damage this turn (Coin that spent Savage Tigermark Round +2 Power and deal +30%% damage)", attacker->name, attacker->Passive);

      sleep(1);
    }

    // Meursault:The Thumb S2-1
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[1]) && attacker->Passive > 0 && !attacker->skills[3].active) {

      if (i == remainingCoins - 2) {

        attacker->Passive--;
        attacker->skills[2].active++;
          attacker->DamageUp += 5;

          Damage += (Damage * 0.10f);

           printf("\n%s spends 1 Tigermark Round(%d) to gain +5%% damage this turn (Coin that spent Tigermark Round +1 Power and deal +10%% damage)", attacker->name, attacker->Passive);

        sleep(1);
      } else if (i == remainingCoins - 1) {
        attacker->Passive--;
        attacker->skills[2].active++;
        attacker->DamageUp += 35;

        Damage += (Damage * 0.10f);

         printf("\n%s spends 1 Tigermark Round(%d) to gain +30%% damage this turn (Coin that spent Tigermark Round +1 Power and deal +10%% damage)", attacker->name, attacker->Passive);

      sleep(1);
      }
      
    } // Meursault:The Thumb S2-2
    else if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[1]) && attacker->Passive > 0 && attacker->skills[3].active) {

        if (i == remainingCoins - 2) {

          attacker->Passive--;
          attacker->skills[2].active++;
            attacker->DamageUp += 10;

            Damage += (Damage * 0.30f);

             printf("\n%s spends 1 Savage Tigermark Round(%d) to gain +10%% damage this turn (Coin that spent Savage Tigermark Round +2 Power and deal +30%% damage)", attacker->name, attacker->Passive);

          sleep(1);
        } else if (i == remainingCoins - 1) {
          attacker->Passive--;
          attacker->skills[2].active++;
          attacker->DamageUp += 70;

          Damage += (Damage * 0.30f);

           printf("\n%s spends 1 Savage Tigermark Round(%d) to gain +60%% damage this turn (Coin that spent Savage Tigermark Round +2 Power and deal +30%% damage)", attacker->name, attacker->Passive);

        sleep(1);
        }

      }
    
    // Meursault:The Thumb S3-1
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[2]) && !attacker->skills[3].active && attacker->Passive > 0) {

      if (i == remainingCoins - 1) {
        
        attacker->Passive--;
        attacker->skills[2].active++;
        int boost = 50; 
        attacker->DamageUp += boost;

        Damage += (Damage * 0.10f);

         printf("\n%s spends 1 Tigermark Round(%d) to gain +50%% damage this turn (Coin that spent Tigermark Round +1 Power and deal +10%% damage)", attacker->name, attacker->Passive);

      sleep(1);
      } else {

        attacker->Passive--;
        attacker->skills[2].active++;
          attacker->DamageUp += 10;

          Damage += (Damage * 0.10f);

           printf("\n%s spends 1 Tigermark Round(%d) to gain +10%% damage this turn (Coin that spent Tigermark Round +1 Power and deal +10%% damage)", attacker->name, attacker->Passive);

        sleep(1);
        
      }
      
    }

    // Meursault:The Thumb S3-2
    if (isId(attacker->ID, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[3]) && attacker->skills[3].active && attacker->Passive > 0) {

      if (i == remainingCoins - 1) {

        attacker->Passive--;
        attacker->skills[2].active++;
        int boost = 50; 
        attacker->DamageUp += boost;

        Damage += (Damage * 0.30f);

         printf("\n%s spends 1 Savage Tigermark Round(%d) to gain +50%% damage this turn (Coin that spent Savage Tigermark Round +2 Power and deal +30%% damage)", attacker->name, attacker->Passive);

      sleep(1);
      } else if (i == remainingCoins - 3) {

        attacker->Passive--;
        attacker->skills[2].active++;
          attacker->DamageUp += 20;

          Damage += (Damage * 0.30f);

           printf("\n%s spends 1 Savage Tigermark Round(%d) to gain +20%% damage this turn (Coin that spent Savage Tigermark Round +2 Power and deal +30%% damage)", attacker->name, attacker->Passive);

        sleep(1);

      } else if (i == remainingCoins - 2) {

        attacker->Passive--;
        attacker->skills[2].active++;
          attacker->DamageUp += 20;

          Damage += (Damage * 0.30f);

           printf("\n%s spends 1 Savage Tigermark Round (%d) to gain +20%% damage this turn (Coin that spent Savage Tigermark Round +2 Power and deal +30%% damage)", attacker->name, attacker->Passive);

        sleep(1);

      }

    }

    // ------------------------------------------------------------


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

    // Hong lu:The Lord of Hongyuan S2 Last coins
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[4]) && i == remainingCoins - 1) {

      printf("\n\n%s: \"Clear the path.\"\n", attacker->name);

      sleep(1);
    }

    // Hong lu:The Lord of Hongyuan S2 Last coins
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[1] || atk == &attacker->skills[4]) && i == remainingCoins - 1 && IsCritical) {

        attacker->DamageUp += 50;

         printf("\n%s's last coin deal +50%% damage with Critical Hit", attacker->name);

      sleep(1);
    }


    // Meursault:Blade Lineage Mentor S2 Last coins
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[1]) && i == 2) {

        attacker->DamageUp += 60;

         printf("\n%s's this coin deal +60%% damage", attacker->name);

      sleep(1);
    }

    // Fixer grade 9? S8 Last coins
    if (isId(attacker->ID, "Fixer grade 9?") == 0 && (atk == &attacker->skills[7]) && i == remainingCoins - 1) {

        attacker->DamageUp += 50;

         printf("\n%s's last coin deal +50%% damage", attacker->name);

      sleep(1);
    }


    // ---------------------------- The Middle Little Brother Sinclair ------------------------
    
    // The Middle Little Brother Sinclair Skill 2 Last coins
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && (atk == &attacker->skills[1] || (atk == &attacker->skills[3] && attacker->Passive >= 2 && attacker->Passive < 4)) && attacker->Passive >= 3 && i == remainingCoins - 1) {

        attacker->DamageUp += 20;

         printf("\n%s at 3+ Envy Resonance, last coin deal +20%% damage", attacker->name);

      sleep(1);
    }

    // The Middle Little Brother Sinclair Skill 3 Last coins
    if (isId(attacker->ID, "The Middle Little Brother Sinclair") == 0 && (atk == &attacker->skills[2] || (atk == &attacker->skills[3] && attacker->Passive >= 4)) && attacker->Passive > 0 && i == remainingCoins - 1) {

      int damageupValue = attacker->Passive * 5;
      if (damageupValue > 30) damageupValue = 30;
      
        attacker->DamageUp += damageupValue;

         printf("\n%s deals +5%% damage for every Envy Resonance (%d%% - Max 30%%)", attacker->name, damageupValue);

      sleep(1);

      if (attacker->skills[1].active >= 10) {

        int damageupValue = (attacker->skills[1].active/10) * 15;
        if (damageupValue > 45) damageupValue = 45;

        attacker->DamageUp += damageupValue;

      printf("\n%s deals +15%% damage for every 10 Book of Vengeance [Sinclair] (%d%% - Max 45%%)", attacker->name, damageupValue);

      sleep(1);
        
      }
    }

    // ------------------- The One Who Grips Faust --------------------------

    // The One Who Grips Faust Skill 3/4 Last coins
    if (isId(attacker->ID, "The One Who Grips Faust") == 0 && (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && attacker->skills[2].active >= 5 && i == remainingCoins - 1) {

        attacker->DamageUp += 50;

         printf("\n%s at 5+ Nail, %s's last coin deal +50%% damage", defender->name, attacker->name);

      sleep(1);
    }

    // The One Who Grips Faust - Whistles Count
    if (isId(attacker->ID, "The One Who Grips Faust") == 0) {
        if (i == 0) attacker->skills[3].active++; // นับครั้งการทำกิจกรรม (Whistles)
    }

    // ------------------------------------------------------------------------
    

// --------------------------------------------------------------------------------------


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
      
          attacker->DamageUp += 90.0f;

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




    

    if (IsCritical) {
      Damage *= 1.2;
    }

     double damageUp = 1.0 + (attacker->DamageUp / 100.0);       // increase by DamageUp%
      double protectionUp = 1.0 - (defender->Protection / 100.0); // reduce by Protection%
      if(damageUp < 0) damageUp = 0;
      if(protectionUp < 0) protectionUp = 0;
    
    int finalDamage = (int)(Damage * damageUp * protectionUp);


    

    

    // Evaded
    if (Evaded) {

      if (isId(defender->ID, "The One Who Grips Faust") == 0 && fanaticUsed > 0) {

        // คำนวณพลังหลบตามสูตร: Base 4 + (โยนเหรียญที่มีพลัง 10 + Fanatic)
        evadePower = 4 + defender->BasePowerBoost;
        if (tossCoinWithSanity(defender)) {
            evadePower += (10 + fanaticUsed) + defender->CoinPowerBoost + defender->FinalPowerBoost;
        }

      } else {
        evadePower = defSkill->BasePower + defender->BasePowerBoost;

        int IsHeadHit = tossCoinWithSanity(attacker);

        if (IsHeadHit) {
          // Check paralyze
          if (attacker->Paralyze > 0) { // ← Character's paralyze
                evadePower += 0; // สกิลอื่นโดน Paralyze ปกติ
                defender->Paralyze--;
            }
          } else {
              evadePower += CoinBuff + defSkill->CoinPower + defender->CoinPowerBoost + defender->DefensePowerBoost + defender->FinalPowerBoost;
            if (evadePower <= 0) evadePower = 0;
          }
        
      }

      if (evadePower >= currentPower) { // ถ้าพลังหลบมากกว่าหรือเท่ากับพลังโจมตี
        finalDamage = 0;
        
        if (defender->HP > defender->MAX_HP) defender->HP = defender->MAX_HP;
      } else {
        Evaded = 0;       // หลบพลาด! หยุดการหลบในเหรียญที่เหลือ
      }

    } 
    
    if (!Evaded) {

      applyDamage(defender, finalDamage, 0);

    totalDamage += finalDamage;

    }

    printf("\n%-10d %-10d %-10d", i + 1, currentPower, finalDamage);


    if (IsStillEvaded && Evaded) {
      printf(" %d (Evaded)", evadePower);
    } else if (IsStillEvaded && !Evaded) {
      printf(" %d (Cancel)", evadePower);
      IsStillEvaded = 0;
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

                  applyDamage(defender, deal, 0);
                  
                    totalDamage += deal;
                    printf("Trigger 'Tremor Burst' (Stack %d Count 0) ", deal);

                    if (attacker->skills[12].active > 20 && defender->Stagger <= 0) {
                        defender->Stagger += 2;
                        printf("\tTarget 'Stagger' for one turn");
                        attacker->skills[12].active = 0; // Reset
                    }
                    attacker->skills[10].active = 0;
                }

                // เอฟเฟกต์อาวุธอื่นๆ (จำกัด 2 ครั้งต่อเทิร์น)
                if (weapon == 0 && attacker->skills[15].active < 2 && !Evaded) { attacker->skills[15].active++; attacker->DamageUpNextTurn += 5; printf("(Gain +5%% damage next turn) "); }
                if (weapon == 1 && attacker->skills[16].active < 2 && !Evaded) { attacker->skills[16].active++; updateSanity(defender, -2); printf("(Target loses 2 Sanity (%d)) ", defender->Sanity); }
                if (weapon == 2 && attacker->skills[17].active < 2 && !Evaded) { attacker->skills[17].active++; attacker->OffenseBoostNextTurn += 1; printf("(Offense +1 next turn) "); }
                if (weapon == 3 && attacker->skills[18].active < 2 && !Evaded) { attacker->skills[18].active++; defender->DefenseBoostNextTurn -= 1; printf("(Target gains 1 Defense Down Next Turn) "); }
                if (weapon >= 5 && weapon <= 7 && attacker->skills[6].active < 2 && !Evaded) { attacker->skills[6].active++; defender->ProtectionNextTurn -= 5; printf("(Target takes +5%% damage Next Turn) "); }


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

                if (attacker->skills[1].active < hermesHardCap && attacker->skills[13].active < maxGainThisTurn) {
                    attacker->skills[1].active++;
                    attacker->skills[13].active++;
                    printf(" Procuration [Hermes] +1 (%d) ", attacker->skills[1].active);

                  // For prescript III Check
                  if (attacker->Passive == 2) {
                    attacker->skills[5].active = 1;
                    }
                  
                }
            }
        
        }

    // -----------------------------------------------------------------------------------

    



// --------------- Meursault:Blade Lineage Mentor -----------------

    // Meursault:Blade Lineage Mentor - Gain on attack
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[0]) && !Evaded) {

      int gain = 5;

      attacker->DamageUp += gain;
      attacker->DamageUpNextTurn += gain;

      printf(" +%d%% damage this turn and next turn", gain);

    }

    // Meursault:Blade Lineage Mentor - Gain on attack
    if (isId(attacker->ID, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[1]) && i == 0 && !Evaded) {

      int gain = 15;

        attacker->DamageUp += gain;
      attacker->DamageUpNextTurn += gain;

      printf(" +%d%% damage this turn and next turn", gain);

    }

    // ---------------------------------------------------------

    // -------------------------------- Heishou Pack - You Branch Adept Heathcliff --------------------------------

    // Heishou Pack - You Branch Adept Heathcliff - Gain on attack
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && !Evaded) {

      int gain = 1;
      if (attacker->HP < attacker->MAX_HP * 0.5) gain += 1;
      
        attacker->Passive += gain;
      if (attacker->Passive > 20) attacker->Passive = 20;

      printf(" +%d Battleblood Instinct (%d)", gain, attacker->Passive);

    }

    // Heishou Pack - You Branch Adept Heathcliff - Bloodflame buff
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && attacker->skills[2].active > 0 && i < 3 && !Evaded) {

      int gain = 3;
      
      if (attacker->Sanity >= 45) {
        gain = 1;
        attacker->OffenseBoostNextTurn++;

        printf(" \t+%d Offense next turn", gain);
      } else {

      updateSanity(attacker, gain);
      if (attacker->Sanity > 45) attacker->Sanity = 45;

      printf(" \t+%d Sanity (%d)", gain, attacker->Sanity);

      }

    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 3 coins
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i != remainingCoins - 1) {

        attacker->skills[0].active += 2;
      if (attacker->skills[0].active > 99) attacker->skills[0].active = 99;

      printf("\n%s applies +2 Burn Stack(%d) on self", attacker->name, attacker->skills[0].active);

      sleep(1);
    }
    
    // Heishou Pack - You Branch Adept Heathcliff Skill 2 Last coins
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[1]) && i == remainingCoins - 1) {

        attacker->skills[0].active += 3;
      if (attacker->skills[0].active > 99) attacker->skills[0].active = 99;
        attacker->skills[1].active++;
      if (attacker->skills[1].active > 99) attacker->skills[1].active = 99;

      printf("\n%s applies +3 Burn Stack(%d) and +1 Burn Count(%d) on self", attacker->name, attacker->skills[0].active, attacker->skills[1].active);

      sleep(1);

      float damageboost = attacker->skills[0].active * 2.0f;
      if (damageboost > 20) damageboost = 20;
      int damage = finalDamage * (damageboost / 100);
      
      applyDamage(defender, damage, 0);

      totalDamage += damage;

         printf("\n%s deals %d ((Burn Stack on self x 2)%% of this Coin's final damage) addition damage (%.0f%% - Max 20%%)", attacker->name, damage, damageboost);

      sleep(1);
    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 3 Last coins
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

      float damageboost = (attacker->skills[0].active + attacker->skills[1].active) * 2.0f;
      if (damageboost > 20) damageboost = 20;
      int damage = finalDamage * (damageboost / 100);

      applyDamage(defender, damage, 0);

      totalDamage += damage;

         printf("\n%s deals %d ((Burn Stack and Count on self x 2)%% of this Coin's final damage) addition damage (%.0f%% - Max 20%%)", attacker->name, damage, damageboost);

      sleep(1);
    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 4 Last coins
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[3]) && i == remainingCoins - 1) {

      float damageboost = 20.0f;
      int damage = finalDamage * (damageboost / 100);

      applyDamage(defender, damage, 0);

      totalDamage += damage;

         printf("\n%s deals %d (20%% of this Coin's final damage) addition damage", attacker->name, damage);

      sleep(1);

      float healvalue = finalDamage + damage;

      if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && remainingCoins > attacker->skills[3].Coins && (attacker->skills[0].active >= 20 || attacker->HP <= attacker->MAX_HP * 0.5)) {
        
        attacker->HP += healvalue;

        if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;

        printf("\n%s's final Coin, heal HP by the amount of damage the above effect dealt (%.0f)", attacker->name, healvalue);

        sleep(1);
        
      } else if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && remainingCoins <= attacker->skills[3].Coins && attacker->skills[0].active < 20 && attacker->HP > attacker->MAX_HP * 0.5) {
        
        attacker->HP += healvalue;

        if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;


        printf("\n%s's final Coin, heal HP by the amount of damage the above effect dealt (%0.f)", attacker->name, healvalue);

         sleep(1);
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

            printf("\t Deal %d fixed Rupture damage on enemy (Count %d)", deal, defender->Rupture[1]);

          if (defender->Rupture[1] <= 0) defender->Rupture[0] = 0;

         applyDamage(defender, deal, 0);

            totalDamage += deal;

        }

    // Bleed // 0 Stack 1 Count
    if ((attacker->Bleed[0] > 0 || attacker->Bleed[1] > 0) && atk->skillType == 0) {

      int damage = attacker->Bleed[0] > 0 ? attacker->Bleed[0] : 1;

      applyDamage(attacker, damage, 0);

        attacker->Bleed[1]--;

      if (attacker->Bleed[1] <= 0) attacker->Bleed[1] = 0;

      printf("\t Take %d Bleed damage (Count %d)", damage, attacker->Bleed[1]);

      if (attacker->Bleed[1] <= 0) attacker->Bleed[0] = 0;

    }

      // -----------------------------------------

  // --------------------------------------------------------------------------------





  // ----------------------- Inflict Status -----------------------

    // ---------------------- Yi sang:Fell Bullet ----------------------

    // Yi sang:Fell Bullet - DefSkill 1 Gain / Inflict
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 && atk == &attacker->defenseSkill[0] && !Evaded) {

      if (i == remainingCoins - 1) {
          attacker->Poise[0] += 2;
        if (attacker->Poise[0] > 99) attacker->Poise[0] = 99;
        printf("\t [On Hit] Poise Stack +2 (%d)", attacker->Poise[0]);
        attacker->Poise[1] += 2;
        if (attacker->Poise[1] > 99) attacker->Poise[1] = 99;
        printf("\t [On Hit] Poise Count +2 (%d)", attacker->Poise[1]);
      }

    }

    // Yi sang:Fell Bullet - Skill 1 Gain / Inflict
    if (isId(attacker->ID, "Yi sang:Fell Bullet") == 0 && atk == &attacker->skills[0] && !Evaded) {

      if (i == remainingCoins - 1) {
          attacker->Poise[1] += 1;
        if (attacker->Poise[1] > 99) attacker->Poise[1] = 99;
        printf("\t [On Hit] Poise Count +1 (%d)", attacker->Poise[1]);

        if (IsCritical) {
          int inflict = 1;
          if (attacker->skills[2].active > 0) inflict += 1;
          defender->Bleed[1] += inflict;
          if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
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
          attacker->Poise[0] += 2;
        if (attacker->Poise[0] > 99) attacker->Poise[0] = 99;
        printf("\t [On Hit] Poise Stack +2 (%d)", attacker->Poise[0]);

        if (IsCritical) {
          int inflict = 2;
          if (attacker->skills[2].active > 0) inflict += 1;
          defender->Bleed[0] += inflict;
          if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
          printf("\t [On Crit] Bleed Stack +%d on enemy (%d)", inflict, defender->Bleed[0]);
        }

      }

      if (i == remainingCoins - 1) {
        
        attacker->Poise[0] += 2;
        if (attacker->Poise[0] > 99) attacker->Poise[0] = 99;
        printf("\t [On Hit] Poise Stack +2 (%d)", attacker->Poise[0]);

        attacker->Passive += 1;
        if (attacker->Passive > 7) attacker->Passive = 7;

        printf("\t [On Hit] Torn Memory +1 (%d/7)", attacker->Passive);

        if (IsCritical) {
          int inflict = 2;
          if (attacker->skills[2].active > 0) inflict += 1;
          defender->Bleed[0] += inflict;
          if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
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
            defender->Bleed[0] += 3;
        if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit] Bleed Stack +3 (%d)", defender->Bleed[0]);

        int inflict = 1;
        if (attacker->skills[2].active > 0) inflict += 1;
        defender->Bleed[1] += inflict;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
        printf("\t [On Hit] Bleed Count +%d on enemy (%d)", inflict, defender->Bleed[1]);

        if (IsCritical) {
          int inflict = 5;
          if (attacker->skills[2].active > 0) inflict += 1;
          defender->Bleed[0] += inflict;
          if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
          printf("\t [On Crit] Bleed Stack +%d (%d)", inflict, defender->Bleed[0]);

          inflict = 3;
          if (attacker->skills[2].active > 0) inflict += 1;
          defender->Bleed[1] += inflict;
          if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
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
          attacker->Poise[1] += 3;
        if (attacker->Poise[1] > 99) attacker->Poise[1] = 99;
        printf("\t [On Hit] Poise Count +3 (%d)", attacker->Poise[1]);
      }

      if (i == remainingCoins - 1) {
        defender->Rupture[1] += 2;
        if (defender->Rupture[1] > 99) defender->Rupture[1] = 99;
        printf("\t [On Hit] Rupture Count +2 on enemy (%d)", defender->Rupture[1]);
      }

    }

    // Hong lu:The Lord of Hongyuan - Skill 2 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[1] && !Evaded) {

      if (i == remainingCoins - 4) {
        defender->Rupture[1] += 2;
        if (defender->Rupture[1] > 99) defender->Rupture[1] = 99;
        printf("\t [On Hit] Rupture Count +2 on enemy (%d)", defender->Rupture[1]);
      }

      if (i == remainingCoins - 3) {
          attacker->Poise[1] += 2;
        if (attacker->Poise[1] > 99) attacker->Poise[1] = 99;
        printf("\t [On Hit] Poise Count +2 (%d)", attacker->Poise[1]);
      }

      if (i == remainingCoins - 2) {
        defender->Rupture[0] += 1;
        if (defender->Rupture[0] > 99) defender->Rupture[0] = 99;
        printf("\t [On Hit] Rupture Stack +1 on enemy (%d)", defender->Rupture[0]);
      }

      if (i == remainingCoins - 1) {
        defender->Rupture[0] += 2;
        if (defender->Rupture[0] > 99) defender->Rupture[0] = 99;
        printf("\t [On Hit] Rupture Stack +2 on enemy (%d)", defender->Rupture[0]);
      }

    }

    // Hong lu:The Lord of Hongyuan - Skill 3 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[2] && !Evaded) {

      if (i == remainingCoins - 1) {
        defender->Rupture[1] += 5;
        if (defender->Rupture[1] > 99) defender->Rupture[1] = 99;
        printf("\t [On Hit] Rupture Count +5 on enemy (%d)", defender->Rupture[1]);
      }

    }

    // Hong lu:The Lord of Hongyuan - Skill 4 Inflict
      if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[3] && !Evaded) {

          defender->Rupture[0] += 1;
          if (defender->Rupture[0] > 99) defender->Rupture[0] = 99;
          printf("\t [On Hit] Rupture Stack +1 on enemy (%d)", defender->Rupture[0]);

        if (i == remainingCoins - 1) {
          attacker->FinalPowerBoostNextTurn += 2;
          printf("\t [On Hit] Gain +2 Final Power next turn");
          defender->ProtectionNextTurn -= 10;
          printf("\t [On Hit] Target takes +10%% damage next turn");
        }

      }

    // Hong lu:The Lord of Hongyuan - Skill 5 Inflict
    if (isId(attacker->ID, "Hong lu:The Lord of Hongyuan") == 0 && atk == &attacker->skills[4] && !Evaded) {

      if (i == remainingCoins - 2) {
        defender->Rupture[1] += 1;
        if (defender->Rupture[1] > 99) defender->Rupture[1] = 99;
        printf("\t [On Hit] Rupture Count +1 on enemy (%d)", defender->Rupture[1]);
      }

      if (i == remainingCoins - 1) {
        defender->Rupture[0] += 2;
        if (defender->Rupture[0] > 99) defender->Rupture[0] = 99;
        printf("\t [On Hit] Rupture Stack +2 on enemy (%d)", defender->Rupture[0]);
        defender->ProtectionNextTurn -= 10;
        printf("\t [On Hit] Target takes +10%% damage next turn");
      }

    }

    // Heshin Packs - Mao - Skill 7 Inflict
    if (isId(attacker->ID, "Heshin Packs - Mao") == 0 && atk == &attacker->skills[6] && !Evaded) {

      if (i == remainingCoins - 3) {
        defender->Rupture[1] += 2;
        if (defender->Rupture[1] > 99) defender->Rupture[1] = 99;
        printf("\t [On Hit] Rupture Count +2 on enemy (%d)", defender->Rupture[1]);
      }

      if (i == remainingCoins - 2) {
        defender->Rupture[0] += 1;
        if (defender->Rupture[0] > 99) defender->Rupture[0] = 99;
        printf("\t [On Hit] Rupture Stack +1 on enemy (%d)", defender->Rupture[0]);
      }

      if (i == remainingCoins - 1) {
        defender->Rupture[0] += 2;
        if (defender->Rupture[0] > 99) defender->Rupture[0] = 99;
        printf("\t [On Hit] Rupture Stack +2 on enemy (%d)", defender->Rupture[0]);
      }

    }

    // Heshin Packs - Si - Skill 8 Inflict
    if (isId(attacker->ID, "Heshin Packs - Si") == 0 && atk == &attacker->skills[7] && !Evaded) {

      if (i == remainingCoins - 1) {
        defender->Rupture[1] += 2;
        if (defender->Rupture[1] > 99) defender->Rupture[1] = 99;
        printf("\t [On Hit] Rupture Count +2 on enemy (%d)", defender->Rupture[1]);
      }

    }

    // Heshin Packs - Wu - Skill 9 Inflict
    if (isId(attacker->ID, "Heshin Packs - Wu") == 0 && atk == &attacker->skills[8] && !Evaded) {

      if (i == remainingCoins - 3) {
        defender->Rupture[0] += 2;
        if (defender->Rupture[0] > 99) defender->Rupture[0] = 99;
        printf("\t [On Hit] Rupture Stack +2 on enemy (%d)", defender->Rupture[0]);
        defender->Tremor[0] += 2;
        if (defender->Tremor[0] > 99) defender->Tremor[0] = 99;
        printf("\t [On Hit] Tremor Stack +2 on enemy (%d)", defender->Tremor[0]);
      }

      if (i == remainingCoins - 2) {
        defender->Bind[1] += 1;
        printf("\t [On Hit] Bind +1 on enemy next turn (%d) (Speed -(Stack) for one turn)", defender->Bind[1]);
        defender->Tremor[1] += 1;
        if (defender->Tremor[1] > 99) defender->Tremor[1] = 99;
        printf("\t [On Hit] Tremor Count +1 on enemy (%d)", defender->Tremor[1]);
      }

      if (i == remainingCoins - 1) {
        TremorBurst(attacker, defender, 20, &totalDamage, 1);
      }

    }

    // Heshin Packs - You - Skill 10 Inflict
    if (isId(attacker->ID, "Heshin Packs - You") == 0 && atk == &attacker->skills[9] && !Evaded) {

        defender->Burn[0] += 2;
        if (defender->Burn[0] > 99) defender->Burn[0] = 99;
        printf("\t [On Hit] Burn Stack +2 on enemy (%d)", defender->Burn[0]);

      if (i == remainingCoins - 1) {
        defender->Rupture[1] += 3;
        if (defender->Rupture[1] > 99) defender->Rupture[1] = 99;
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
        defender->Bleed[0] += 2; // Bleed Stack
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
       printf("\t [On Hit] Bleed Stack +2 on enemy (%d)", defender->Bleed[0]);
    }

  }

  // The One Who Grips Faust - SKill 2 Inflict
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[1] && !Evaded) {

    if (i == remainingCoins - 3) {
      attacker->skills[2].active += 2; // Nail
      printf("\t [On Hit] Nail +2 on enemy (%d)", attacker->skills[2].active);
        defender->Bleed[1] += 2; // Bleed Count
      if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
       printf("\t [On Hit] Bleed Count +2 on enemy (%d)", defender->Bleed[1]);
    }

    if (i == remainingCoins - 2) {
      attacker->skills[2].active += 3; // Nail
      printf("\t [On Hit] Nail +3 on enemy (%d)", attacker->skills[2].active);
        defender->Bleed[0] += 2; // Bleed Stack
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
       printf("\t [On Hit] Bleed Stack +2 on enemy (%d)", defender->Bleed[0]);
    }

    if (i == remainingCoins - 1) {
      attacker->skills[7].active += 1; // Gaze Next turn
      printf("\t [On Hit] Inflict Gaze on enemy next turn");

      if (IsHeadHit) {
        defender->ParalyzeNextTurn += 1;
        printf("\t [Head Hit] Paralyze +1 on enemy next turn (Fix the Power of 1 Coins to 0 for one turn)");
      }
    }

  }

  // The One Who Grips Faust - SKill 3 Inflict
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[2] && !Evaded) {

    if (i == remainingCoins - 3) {
      attacker->skills[2].active += 2; // Nail
      printf("\t [On Hit] Nail +2 on enemy (%d)", attacker->skills[2].active);
        defender->Bleed[1] += 1; // Bleed Count
      if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
       printf("\t [On Hit] Bleed Count +1 on enemy (%d)", defender->Bleed[1]);
    }

    if (i == remainingCoins - 2) {
      defender->Bleed[0] += 2; // Bleed Stack
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
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

  }

  // The One Who Grips Faust - SKill 5 Inflict
  if (isId(attacker->ID, "The One Who Grips Faust") == 0 && atk == &attacker->skills[4] && !Evaded) {

    if (i == remainingCoins - 1) {
      defender->ProtectionNextTurn -= 50;
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
        }

      attacker->skills[0].active += Stack;
      attacker->skills[1].active += Count;

      if (Count > 0 && Stack > 0) {
        printf("\t [On Hit] Burn Stack +%d on enemy (%d) \t [On Hit] Burn Count +%d on enemy (%d)", Stack, attacker->skills[0].active, Count, attacker->skills[1].active);
      } else if (Stack > 0) {
           printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, attacker->skills[0].active);
           } else if (Count > 0) {
             printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, attacker->skills[1].active);
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

      attacker->skills[0].active += Stack;
      if (attacker->skills[0].active > 99) attacker->skills[0].active = 99;
      attacker->skills[1].active += Count;
      if (attacker->skills[1].active > 99) attacker->skills[1].active = 99;

      if (Count > 0 && Stack > 0) {
        printf("\t [On Hit] Burn Stack +%d on enemy (%d) \t [On Hit] Burn Count +%d on enemy (%d)", Stack, attacker->skills[0].active, Count, attacker->skills[1].active);
      } else if (Stack > 0) {
           printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, attacker->skills[0].active);
           } else if (Count > 0) {
             printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, attacker->skills[1].active);
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

        attacker->skills[0].active += Stack;
        if (attacker->skills[0].active > 99) attacker->skills[0].active = 99;
        attacker->skills[1].active += Count;
        if (attacker->skills[1].active > 99) attacker->skills[1].active = 99;

        if (Count > 0 && Stack > 0) {
          printf("\t [On Hit] Burn Stack +%d on enemy (%d) \t [On Hit] Burn Count +%d on enemy (%d)", Stack, attacker->skills[0].active, Count, attacker->skills[1].active);
        } else if (Stack > 0) {
             printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, attacker->skills[0].active);
             } else if (Count > 0) {
               printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, attacker->skills[1].active);
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

      attacker->skills[0].active += Stack;
      if (attacker->skills[0].active > 99) attacker->skills[0].active = 99;
      attacker->skills[1].active += Count;
      if (attacker->skills[1].active > 99) attacker->skills[1].active = 99;

      if (Count > 0 && Stack > 0) {
        printf("\t [On Hit] Burn Stack +%d on enemy (%d) \t [On Hit] Burn Count +%d on enemy (%d)", Stack, attacker->skills[0].active, Count, attacker->skills[1].active);
      } else if (Stack > 0) {
           printf("\t [On Hit] Burn Stack +%d on enemy (%d)", Stack, attacker->skills[0].active);
           } else if (Count > 0) {
             printf("\t [On Hit] Burn Count +%d on enemy (%d)", Count, attacker->skills[1].active);
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
        
        applyDamage(defender, attacker->skills[0].active, 0);
        
      totalDamage += attacker->skills[0].active;

      printf("\t [On Hit] Fairy deals %d damage", attacker->skills[0].active);

      } else {

        int Fairydamage = 0.5*((defender->MAX_HP/100)*attacker->skills[0].active);
        
        applyDamage(defender, Fairydamage, 1);
        
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

        applyDamage(defender, attacker->skills[0].active, 0);
        
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

          applyDamage(defender, min, 0);
          
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

            applyDamage(defender, min, 0);
            
            totalDamage += min;

            }

        }

        if (defender->hasSanity == 1 && defender->Sanity < 0 && defender->sanityGainBase >= 0) { // Normal
          
          int deal = attacker->skills[0].active/2 - (defender->Sanity/5);

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            applyDamage(defender, deal, 0);

            totalDamage += deal;

          }

              } else if (defender->hasSanity == 1 && defender->Sanity > 0 && defender->sanityGainBase < 0) { // Negative Sanity enemy

          int deal = attacker->skills[0].active/2 + (defender->Sanity/5);

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            applyDamage(defender, deal, 0);

            totalDamage += deal;

          }

              } else if (defender->hasSanity == 0) { // No Sanity enemy

          int deal = attacker->skills[0].active/2;

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            applyDamage(defender, deal, 0);

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

        printf("\n%s runs out of The Living & The Departed, stop attack and use 'Reload' instead, Spends 15 Sanity(%d) to Gain 20 The Living & The Departed and gain Shield equal to (Butterfly on the target x 2)%% of Max HP. (%d%% - Max 40%%) (%d - Shield %.2f)", attacker->name, attacker->Sanity, ShieldGain, Shield, attacker->Shield);

        attacker->Passive = 20;

        sleep(1);

        printf("\n(%d bonus) Damage Multiplier: %.2f\n", bonus,
           atk->DmgMutiplier);

      } else if (atk == &attacker->skills[2] && i != remainingCoins - 2) {

         i = remainingCoins;

        updateSanity(attacker, -(15));
        if (attacker->Sanity < -45) attacker->Sanity = -45;

        int ShieldGain = ((attacker->skills[0].active * 2) > 40 ? 40 : (attacker->skills[0].active * 2));
        int Shield = (ShieldGain/100.0f) * attacker->MAX_HP;
        
        attacker->Shield += Shield;

        printf("\n%s runs out of The Living & The Departed, stop attack and use 'Reload' instead, Spends 15 Sanity(%d) to Gain 20 The Living & The Departed and gain Shield equal to (Butterfly on the target x 2)%% of Max HP. (%d%% - Max 40%%) (%d - Shield %.2f)", attacker->name, attacker->Sanity, ShieldGain, Shield, attacker->Shield);

        attacker->Passive = 20;

        sleep(1);

        printf("\n(%d bonus) Damage Multiplier: %.2f\n", bonus,
           atk->DmgMutiplier);
      }
      
        
          sleep(1);
        
      
      }

    
    // -----------------------------------------------------

    // ------------------- Muga Ryōshū -------------------

    // Muga Ryōshū on attack
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && attacker->skills[0].active/5 > 0) {
        applyDamage(defender, attacker->skills[0].active/5, 0); // deal sever the thread/5 damage
      printf("\t Enemy takes +%d damage", attacker->skills[0].active/5);

      totalDamage += attacker->skills[0].active/5;
    }

    // Muga Ryōshū gains on attack
    if (isId(attacker->ID, "Muga Ryōshū") == 0) {
        attacker->Passive += 4; // Gain 1 Muga for every coin use
        if (attacker->Passive > 100) attacker->Passive = 100;
      printf("\t Muga [無我] +4 (%d - Max 100)", attacker->Passive);

      attacker->skills[0].active += attacker->skills[10].active; // Inflicted sever
      if (attacker->skills[0].active > 100) attacker->skills[0].active = 100;
      printf("\t Sever the Thread [切絲] +%d on enemy (%d - Max 100)", attacker->skills[10].active, attacker->skills[0].active);
    }

    // Muga Ryōshū gains on get hit
    if (isId(defender->ID, "Muga Ryōshū") == 0) {
          defender->skills[11].active += 1; // Gain 1 Muga for every coin use
        if (defender->skills[11].active > 2) defender->skills[11].active = 2;
    }

    // Muga Ryōshū on attack skill 1
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[0] && Unbreakable <= 0) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 2) {
      
      defender->Bleed[0] += bleedstackinf + 1;
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 1, defender->Bleed[0]);
        defender->Bleed[1] += 2;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
          printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 1) {

        defender->Bleed[0] += bleedstackinf + 3;
        if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf +3, defender->Bleed[0]);
          defender->Bleed[1] += 1;
          if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
            printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

        }
      
    }

    // Muga Ryōshū on attack skill 2
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[1] && Unbreakable <= 0) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 2) {

      defender->Bleed[0] += bleedstackinf + 2;
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
        defender->Bleed[1] += 1;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
          printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 1) {

        defender->Bleed[0] += bleedstackinf + 2;
        if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
          defender->Bleed[1] += 2;
          if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
            printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

        }

    }

    // Muga Ryōshū on attack skill 3
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[2] && Unbreakable <= 0) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 3) {

      defender->Bleed[0] += bleedstackinf + 1;
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 1, defender->Bleed[0]);
        defender->Bleed[1] += 1;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
          printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 2) {

      defender->Bleed[0] += bleedstackinf + 2;
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
        defender->Bleed[1] += 1;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
          printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 1) {

        defender->Bleed[0] += bleedstackinf + 2;
        if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
          defender->Bleed[1] += 3;
          if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
            printf("\t [On Hit without Cracking] Bleed Count +3 on target (%d)", defender->Bleed[1]);

        }

    }

    // Muga Ryōshū on attack skill 4
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[3] && Unbreakable <= 0) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 3) {

      defender->Bleed[0] += bleedstackinf + 2;
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
        defender->Bleed[1] += 1;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
          printf("\t [On Hit without Cracking] Bleed Count +1 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 2) {

      defender->Bleed[0] += bleedstackinf + 1;
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 1, defender->Bleed[0]);
        defender->Bleed[1] += 2;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
          printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 1) {

        defender->Bleed[0] += bleedstackinf + 2;
        if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
          defender->Bleed[1] += 2;
          if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
            printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

        }

    }

    // Muga Ryōshū on attack skill 5
    if (isId(attacker->ID, "Muga Ryōshū") == 0 && atk == &attacker->skills[4] && Unbreakable <= 0) {

      int bleedstackinf = 0;
      if (attacker->skills[1].active == 1) bleedstackinf += 3;

      if (i == remainingCoins - 5) {

        defender->Bleed[1] += 2;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
          printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 4) {

        defender->Bleed[1] += 2;
        if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
          printf("\t [On Hit without Cracking] Bleed Count +2 on target (%d)", defender->Bleed[1]);

      }

      if (i == remainingCoins - 3) {

      defender->Bleed[0] += bleedstackinf + 2;
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);

      }

      if (i == remainingCoins - 2) {

      defender->Bleed[0] += bleedstackinf + 2;
      if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
        printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);

      }

      if (i == remainingCoins - 1) {

        defender->Bleed[0] += bleedstackinf + 2;
        if (defender->Bleed[0] > 99) defender->Bleed[0] = 99;
          printf("\t [On Hit without Cracking] Bleed Stack +%d on target (%d)", bleedstackinf + 2, defender->Bleed[0]);
          defender->Bleed[1] += 2;
          if (defender->Bleed[1] > 99) defender->Bleed[1] = 99;
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

    // King in Binds - Sinking
    if (isId(attacker->ID,
                   "King in Binds") == 0 &&
        (attacker->skills[1].active > 0 || attacker->skills[2].active > 0) && !Evaded) {

      if (attacker->skills[1].active <= 0 && attacker->skills[2].active > 0) attacker->skills[1].active++;

        if (defender->hasSanity == 1) { // Normal
        int deal = attacker->skills[1].active;

          updateSanity(defender, -(deal));

          attacker->skills[2].active -= 1;

          if (attacker->skills[2].active <= 0) attacker->skills[2].active = 0;

        printf("\t Sanity -%d on enemy (%d) (Count %d)", deal, defender->Sanity, attacker->skills[2].active);

          if (attacker->skills[2].active <= 0) attacker->skills[1].active = 0;

        } else { // No Sanity enemy

          int deal = attacker->skills[1].active;

          attacker->skills[2].active -= 1;

           if (attacker->skills[2].active <= 0) attacker->skills[2].active = 0;

            printf("\t Deal %d fixed damage from Sinking on enemy (Count %d)", deal, attacker->skills[2].active);

          if (attacker->skills[2].active <= 0) attacker->skills[1].active = 0;

          applyDamage(defender, deal, 0);

            totalDamage += deal;

        }
      }


    // King in Binds skill 1, 3 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[0] || atk == &attacker->skills[2]) && !Evaded) {

        attacker->skills[1].active += 2;
      if (attacker->skills[1].active > 99) attacker->skills[1].active = 99;

        printf("\t [On Hit] Sinking Stack +2 on target (%d)", attacker->skills[1].active);

    }

    // King in Binds skill 2 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[1]) && !Evaded) {

      if (i == remainingCoins - 3) {

        attacker->skills[2].active += 2;
        if (attacker->skills[2].active > 99) attacker->skills[2].active = 99;

        printf("\t [On Hit] Sinking Count +2 on target (%d)", attacker->skills[2].active);

      } else {

          attacker->skills[1].active += 3;
        if (attacker->skills[1].active > 99) attacker->skills[1].active = 99;

          printf("\t [On Hit] Sinking Stack +3 on target (%d)", attacker->skills[1].active);

        }

    }

    // King in Binds skill 4 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[3]) && !Evaded) {

      if (i == remainingCoins - 2) {

        attacker->skills[4].active += 3;
      if (attacker->skills[4].active > 99) attacker->skills[4].active = 99;

        printf("\t [On Hit] Tremor Count +3 on target (%d)", attacker->skills[4].active);

      } else if (i == remainingCoins - 1) {

        if (attacker->skills[3].active <= 0 && attacker->skills[4].active > 0) attacker->skills[3].active++;
        if (attacker->skills[4].active <= 0 && attacker->skills[3].active > 0) attacker->skills[4].active++;
        
        int deal = attacker->skills[3].active;

          attacker->skills[6].BasePower += deal; // Tremor Burst damage store

        applyDamage(defender, deal, 0);

          totalDamage += deal;

        attacker->skills[4].active -= 1;
        if (attacker->skills[4].active <= 0) attacker->skills[4].active = 0;

            printf("\t [On Hit] Trigger 'Tremor Burst' on target (Stack %d Count %d)", attacker->skills[3].active, attacker->skills[4].active);

        if (attacker->skills[4].active <= 0) attacker->skills[3].active = 0;

        if (attacker->skills[6].BasePower > defender->MAX_HP/4 && defender->Stagger <= 0) {

          defender->Stagger += 2;

          printf(" \tTarget 'Stagger' for one turn");

          attacker->skills[6].BasePower = 0;
          
        }
        
      }

    }

    // King in Binds skill 6 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[5]) && !Evaded) {

        attacker->skills[3].active += attacker->skills[1].active;
      if (attacker->skills[3].active > 99) attacker->skills[3].active = 99;

        printf("\t [On Hit] Inflict Tremor Stack equal to Sinking Stack on target (%d)", attacker->skills[1].active);

        int deal = attacker->skills[3].active;

          attacker->skills[6].BasePower += deal; // Tremor Burst damage store

      applyDamage(defender, deal, 0);

          totalDamage += deal;

      if (attacker->skills[3].active <= 0 && attacker->skills[4].active > 0) attacker->skills[3].active++;
      if (attacker->skills[4].active <= 0 && attacker->skills[3].active > 0) attacker->skills[4].active++;

        attacker->skills[4].active -= 1;
        if (attacker->skills[4].active <= 0) attacker->skills[4].active = 0;

            printf("\t [On Hit] Trigger 'Tremor Burst' on target (Stack %d Count %d)", attacker->skills[3].active, attacker->skills[4].active);

        if (attacker->skills[4].active <= 0) attacker->skills[3].active = 0;

        if (attacker->skills[6].BasePower > defender->MAX_HP/4 && defender->Stagger <= 0) {

          defender->Stagger += 2;

          printf(" \tTarget 'Stagger' for one turn");

          attacker->skills[6].BasePower = 0;

        }

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

        int deal = attacker->skills[3].active;

          attacker->skills[6].BasePower += deal; // Tremor Burst damage store

          applyDamage(defender, deal, 0);

          totalDamage += deal;

        attacker->skills[4].active -= 1;
        if (attacker->skills[4].active <= 0) attacker->skills[4].active = 0;

            printf(" \tTrigger 'Tremor Burst' on target (Stack %d Count %d)", attacker->skills[3].active, attacker->skills[4].active);

        if (attacker->skills[4].active <= 0) attacker->skills[3].active = 0;

        if (attacker->skills[6].BasePower > defender->MAX_HP/4 && defender->Stagger <= 0) {

          defender->Stagger += 2;

          printf(" \tTarget 'Stagger' for one turn");

          attacker->skills[6].BasePower = 0;

        }

      attacker->skills[3].active += 5;
      if (attacker->skills[3].active > 99) attacker->skills[3].active = 99;
      
      attacker->skills[4].active += 5;
      if (attacker->skills[4].active > 99) attacker->skills[4].active = 99;

        printf(" \tTremor Stack +5 (%d) and Tremor Count +5 (%d) on target", attacker->skills[3].active, attacker->skills[4].active);

      defender->ProtectionNextTurn -= 10;

      printf("\n\n%s take +10%% damage next turn\n", defender->name);

    }

    }

    // King in Binds skill 5 Debuff
    if (isId(attacker->ID, "King in Binds") == 0 && (atk == &attacker->skills[4]) && !Evaded) {

      if (defender->hasSanity == 1) { // Normal
        int deal = attacker->skills[1].active + attacker->skills[2].active;

        if (defender->Sanity - deal <= -45) {

          int finalSP = defender->Sanity - deal;
          int excessdamage = (-45 - finalSP); // เช่น -45 - (-50) = 5

          updateSanity(defender, -(deal));

          applyDamage(defender, excessdamage, 0);

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

        applyDamage(defender, deal, 0);

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
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (attacker->skills[2].active > 0 || attacker->HP < attacker->MAX_HP * 0.5) && atk == &attacker->skills[1] && i == remainingCoins - 1 && remainingCoins <= attacker->skills[1].Coins) {

      attacker->skills[1].Coins = remainingCoins;

      remainingCoins++;

       printf("\n%s has Bloodflame [血炎] or less than 50%% HP, Reuse Coin (Once per Skill)", attacker->name);

          sleep(1);

    } else if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && atk == &attacker->skills[1]) {
      attacker->skills[1].Coins = 3;
    }

    // Heishou Pack - You Branch Adept Heathcliff - Skill 3-2 reuse
    if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (attacker->skills[0].active >= 20 || attacker->HP <= attacker->MAX_HP * 0.5) && atk == &attacker->skills[3] && i == remainingCoins - 1 && remainingCoins <= attacker->skills[3].Coins) {

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

      applyDamage(defender, damage, 0);
      
      totalDamage += damage;

      printf("\n%s deals more damage base on Sanity different (%d - Max 30)",
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
        defender->FinalPowerBoostNextTurn += 1;
        printf("\n%s's Uncompromising Imposition activated! Heal up to max HP and gain 1 Final Power, lose 1 stack(%d)", defender->name, attacker->skills[15].active);
        sleep(1);
    } else {

        attacker->skills[15].active -= 1;

        defender->HP = defender->MAX_HP;
        printf("\n%s's Dialogues activated! Heal up to max HP, lose 1 stack(%d)", defender->name, attacker->skills[15].active);
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
    

    if (i == remainingCoins - 1) {
      printf("\n(%d bonus) Damage Multiplier: %.2f\n", bonus,
             atk->DmgMutiplier);
    }

    if (Unbreakable > 0) Unbreakable--; // ← Character's Cracking Coins

    sleep(1);
  }

  }

  printf("%s total damage dealt (Opponent's defense: %d): %d\n",
         attacker->name, defTempDefense, totalDamage);



  
  
  //---------------- After Attack Buff ----------------------------

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

      attacker->FinalPowerBoostNextTurn -= 5;
      attacker->DamageUpNextTurn -= 50;
      attacker->Protection -= 30;

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

        // [Attack End] Target Lose 5 Sanity, 3 Def Down, +30% Fragile
        updateSanity(defender, -5);
        defender->DefenseBoostNextTurn -= 3;
        defender->ProtectionNextTurn -= 30; //Fragile: รับดาเมจแรงขึ้น 30%

        printf("\n%s deals 5 Sanity damage (%d), inflict 3 Defense Down next turn and target takes +30%% damage next turn.\n", attacker->name, defender->Sanity);
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

      attacker->skills[1].active += 2;

       printf("\n%s gains 2 Burn Count (%d)\n", attacker->name,attacker->skills[1].active);

    sleep(1);

  }

  // Heishou Pack - You Branch Adept Heathcliff Skill 4 after attack consumes
  if (isId(attacker->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[3])) {

      attacker->Passive = 0;

    attacker->skills[2].Copies = 1;

       printf("\n%s consumes all Battleblood Instinct\n", attacker->name);

    sleep(1);

    if (attacker->skills[0].active > 20) {

      int consumes = (attacker->skills[0].active - 20) > 25 ? 25 : (attacker->skills[0].active - 20);

      attacker->skills[0].active -= consumes;

      int healvalue = consumes * 2;

      attacker->HP += healvalue;
      if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;

    printf("\n%s at more than 20 Burn Stack, consume up to 25 excess Burn (%d) and heal (%d%% - Burn consumed x 2)%% HP\n", attacker->name, consumes, healvalue);

    sleep(1);

    }

  }

  // --------------------------------------- Binah  -----------------------------------------
  
  // Binah - Skill 2 debuff
  if (isId(attacker->ID, "Binah") == 0 && (atk == &attacker->skills[1]) && !Evaded) {

    int inflictvalue = 1;

        defender->ParalyzeNextTurn += inflictvalue;

    printf("\n%s inflicts %d Paralyze next turn (Fix the Power of 1 Coins to 0 for one turn)\n", attacker->name,inflictvalue);

    sleep(1);
  }

  // Binah - Skill 4 debuff
  if (isId(attacker->ID, "Binah") == 0 && (atk == &attacker->skills[3]) && !Evaded) {

    int inflictvalue = 50;

    defender->DamageUp -= inflictvalue;
        defender->DamageUpNextTurn -= inflictvalue;

    printf("\n%s deals -%d%% damage for this turn and next turn\n", defender->name, inflictvalue);

    sleep(1);
  }

  // Binah - Skill 5 debuff serious
  if (isId(attacker->ID, "Binah") == 0 && (atk == &attacker->skills[4]) && !Evaded) {

    int inflictvalue = 1;
    
    if (attacker->Passive) {
      inflictvalue = 2;
    }

        defender->FinalPowerBoost -= inflictvalue;
    defender->FinalPowerBoostNextTurn -= inflictvalue;

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

        // [Attack End] Target Lose 5 Sanity, 3 Def Down, +30% Fragile
        updateSanity(defender, -5);
        defender->DefenseBoostNextTurn -= 3;
        defender->ProtectionNextTurn -= 30; //Fragile: รับดาเมจแรงขึ้น 30%

        printf("\n%s deals 5 Sanity damage (%d), inflict 3 Defense Down next turn and target takes +30%% damage next turn.\n", attacker->name, defender->Sanity);
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

  // Meursault:The Thumb Shin buffs (temporary, print once)
  if (isId(attacker->ID, "Meursault:The Thumb") == 0 &&
       attacker->Passive <= 0 && !attacker->skills[3].active) {

    int amount = ((int)(8 * defender->MAX_HP)) / 847;
    if (amount < 8) amount = 8;

    if (isId(defender->ID, "Sancho:The Second Kindred of Don Quixote") == 0 || isId(defender->ID, "Don Quixote") == 0) amount += 2; // pity for boss
    if (isId(defender->ID, "Sukuna:King of Curse") == 0) amount += 3; // pity for boss

    attacker->skills[3].active = 1;

      attacker->Passive = amount;
    
      printf("\n%s spent all Tigermark Round, 'Unrelenting Spirit [剛氣]' activated and reload %d Savage Tigermark Round\n",
             attacker->name, amount);

      sleep(1);

      printf("\n%s: \"I see that you are worth the cost of my ammunition.\"\n", attacker->name);

    }

  // Meursault:The Thumb Shin buffs (temporary, print once)
  if (isId(defender->ID, "Meursault:The Thumb") == 0 &&
    defender->HP <= defender->MAX_HP*0.65 && !defender->skills[3].active) {

    int amount = ((int)(8 * attacker->MAX_HP)) / 847;
    if (amount < 8) amount = 8;

    if (isId(defender->ID, "Sancho:The Second Kindred of Don Quixote") == 0 || isId(defender->ID, "Don Quixote") == 0) amount += 2; // pity for boss
    if (isId(defender->ID, "Sukuna:King of Curse") == 0) amount += 3; // pity for boss

      defender->skills[3].active = 1;

    defender->Passive = amount;

      printf("\n%s at 65%% or less HP, 'Unrelenting Spirit [剛氣]' activated and reload %d Savage Tigermark Round\n",
        defender->name, amount);

      sleep(1);

      printf("\n%s: \"Keh... Why, that's quite good.\"\n", defender->name);

    }

  // Roland – Buff
  if (isId(attacker->ID, "Fixer grade 9?") == 0 && atk == &attacker->skills[9]) {

    int Losetotal = totalDamage;
    
    updateSanity(attacker, -(Losetotal));
    if (Losetotal > 60) Losetotal = 60;
    if (attacker->Sanity < -45) attacker->Sanity = -45;

    printf("\n%s loses Sanity equal to dealt damage(%d - Max 60)\n",
      attacker->name, Losetotal);
    
    sleep(1);
  }

  //--------------------------------Lei heng---------------------

  // Lei heng – skill 3 or 6
  if (isId(attacker->ID, "Lei heng") == 0 &&
      (atk == &attacker->skills[2] || atk == &attacker->skills[4])) {

    int clashpowerdebuff = attacker->skills[1].active * 1;
    if (clashpowerdebuff > 5) clashpowerdebuff = 5;
    int takemoredamage = attacker->skills[1].active * 10;
    if (takemoredamage > 50) takemoredamage = 50;
    
    attacker->ClashPowerNextTurn -= clashpowerdebuff;
      attacker->ProtectionNextTurn -= takemoredamage;

        printf("\n%s gained %d Overheat(Clash Power -%d, Take %d%% more damage) next turn\n", attacker->name, attacker->skills[1].active, clashpowerdebuff, takemoredamage);

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

      attacker->CoinPowerBoost = 0;

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

      attacker->CoinPowerBoost = 0;

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

      attacker->CoinPowerBoost = 0;

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

      attacker->CoinPowerBoost = 0;

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

     attacker->CoinPowerBoost = 0;

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
  if (isId(attacker->ID, "Heshin Packs - Mao") == 0 &&
      atk == &attacker->skills[6] && !Evaded) {

    defender->ClashPowerNextTurn -= 1;

    printf("\n%s gains 1 Clash Power Down next turn by %s's Skill\n", defender->name, attacker->name);

    sleep(1);

  } // Hong lu:The Lord of Hongyuan - Si
  else if (isId(attacker->ID, "Heshin Packs - Si") == 0 &&
      atk == &attacker->skills[7] && !Evaded) {

    int randomlost = rand() % 2 + 1;

    if (randomlost) {
    
    defender->OffenseBoostNextTurn -= 1;

    printf("\n%s gains 1 Offense Down next turn by %s's Skill\n", defender->name, attacker->name);

    } else if (!randomlost) {

        defender->DefenseBoostNextTurn -= 1;

        printf("\n%s gains 1 Defense Down next turn by %s's Skill\n", defender->name, attacker->name);

        }

    sleep(1);

  } // Hong lu:The Lord of Hongyuan - Wu
  else if (isId(attacker->ID, "Heshin Packs - Wu") == 0 &&
      atk == &attacker->skills[8] && !Evaded) {

    defender->ProtectionNextTurn -= 20;

    printf("\n%s takes +20%% damage next turn by %s's Skill\n", defender->name, attacker->name);

    sleep(1);

  }
  
  // --------------------------------------------------------------

    // Meursault:Blade Lineage Mentor - Passive
    if (isId(defender->ID, "Meursault:Blade Lineage Mentor") == 0) {

      defender->FinalPowerBoostNextTurn += 1;

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

    defender->ParalyzeNextTurn += 1;

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

      defender->ParalyzeNextTurn += 1;

      printf("\n%s inflict 1 Paralyze next turn (Fix the Power of 1 Coins to 0 for one turn)\n", attacker->name);

      sleep(1);

      if (attacker->Passive >= 3) {
       attacker->Passive -= 3;
      if (attacker->Passive < 1) attacker->Passive = 1;

      defender->ParalyzeNextTurn += 2;

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

       attacker->DamageUpNextTurn += 30;
       attacker->FinalPowerBoostNextTurn += 3;

      printf("\n%s gains 3 Final Power Up and +30%% damage next turn\n", attacker->name);

      sleep(1);
    }

    // Erlking Heathcliff Skill 3 debuff
    if (isId(attacker->ID, "Erlking Heathcliff") == 0 && (atk == &attacker->skills[3]) && !Evaded) {

         defender->ParalyzeNextTurn += 3;

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
    
      defender->ClashPowerNextTurn -= 1;

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
  
        attacker->DamageUpNextTurn += boost;
  
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
  }

    // ---------------------- Anti death effect ----------------------

    // Erlking Heathcliff Faded promise for wild hunt
    if (isId(attacker->ID, "Erlking Heathcliff") == 0 && isId(defender->ID, "Heathcliff:Wild Hunt") == 0 && (attacker->skills[7].active == 1) && defender->HP <= 0)  {

      attacker->skills[7].active--;
        
      defender->HP = 1;

      printf("\n%s's 'Faded Promise' activated! In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn (Once per Encounter)\n", defender->name);

      sleep(1);

    }

    // Hong lu:The Lord of Hongyuan - Passive
    if ((isId(defender->ID, "Hong lu:The Lord of Hongyuan")) == 0 &&
             defender->HP <= 0 && defender->skills[5].active == 1) {

        defender->skills[5].active--;

      defender->HP = 1;

      printf("\n%s's '%s' activeted! Nullity all damage; then apply 'Lordsguard' to all left Heishou Pack and bring %s's HP to 1 (Once per Encounter)\n",
        defender->name, defender->skills[5].name, defender->name);

      sleep(1);

      printf("\n%s: \"The Lord will not die.\"\n", defender->name);

      sleep(1);

    }

      // Heishou Pack - You Branch Adept Heathcliff - Anti death Passive
      if ((isId(defender->ID, "Heishou Pack - You Branch Adept Heathcliff")) == 0 &&
          defender->HP <= 0 && defender->skills[3].active == 0) {

          defender->skills[3].active--;
      
        defender->HP = 1;

        printf("\n%s's 'Flame Rooster's Death Defiance [炎鳥不死戦]' activated! Nullity all damage; then bring %s's HP to 1 (Once per Encounter)\n",
          defender->name, defender->name);

        sleep(1);

        printf("\n%s: \"Flame Rooster's Death Defiance [炎鳥不死戦]... Heh! You really thought I was gonna kick it... Huh?!\"\n",
          defender->name);

      }

      // Meursault:Blade Lineage Mentor - Passive
      if (isId(defender->ID, "Meursault:Blade Lineage Mentor") == 0 &&
          defender->HP <= 0 && defender->Passive == 0) {

          defender->Passive--;

            defender->HP = 1;

        printf("\n%s's 'Swordplay of the Homeland' activated! Nullity all damage; then bring %s's HP to 1 (Once per Encounter)\n",
          defender->name, defender->name);

        sleep(1);

      }

    // ------------------------------------------------------------------
    
  // -------------------------------------------------------------------------------------------------------------

  sleep(1);
}
}

























  

// Returns effective skill and also temporary offense/defense for this turn
SkillStats *getEffectiveSkill(Character *c, Character *c2,
                              SkillStats *chosenSkill, int *tempOffense,
                              int *tempDefense) {

  int IsplayerUnableToAct = isPanicked(c) || isStaggered(c);

  if (IsplayerUnableToAct) return 0;


  // ----------------------- Combat Start --------------------------

  *tempOffense += chosenSkill->Offense;
  *tempDefense += chosenSkill->Defense;



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
      c->Protection += reduction;

      printf("\n%s gains +%d Offense and Defense, Damage Reduction +%.0f%%\n", c->name, levelBoost, reduction);

    sleep(1);
  }

  // Muga Ryōshū - Final Power Skill 1/2
  if (isId(c->ID, "Muga Ryōshū") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

    int gain = c->Passive/10;
    if (gain > 3) gain = 3;

    if (gain > 0) {

    c->FinalPowerBoost += gain;

    printf("\n%s gains +1 Final Power (%d - Max 3) for every 10 Muga [無我] on self (%d)\n", c->name, gain, c->Passive);
    
    sleep(1);

    }

    gain = c2->Bleed[0]/6;
    if (gain > 3) gain = 3;

    if (gain > 0) {

    c->FinalPowerBoost += gain;

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

    c->FinalPowerBoost += gain;

    printf("\n%s gains +1 Final Power (%d - Max 5) for every 10 Muga [無我] on self (%d)\n", c->name, gain, c->Passive);

    sleep(1);

    }

    gain = c2->Bleed[0]/6;
    if (gain > 4) gain = 4;

    if (gain > 0) {

    c->FinalPowerBoost += gain;

    printf("\n%s gains +1 Final Power (%d - Max 4) for every 6 Bleed Stack on target (%d)\n", c->name, gain, c2->Bleed[0]);

    sleep(1);

    }
    
  }

  // Muga Ryōshū - Final Power Skill 5
  if (isId(c->ID, "Muga Ryōshū") == 0 && (chosenSkill == &c->skills[4])) {

    int gain = c->Passive/4;
    if (gain > 12) gain = 12;

    if (gain > 0) {

    c->FinalPowerBoost += gain;

    printf("\n%s gains +1 Final Power (%d - Max 12) for every 4 Muga [無我] on self (%d)\n", c->name, gain, c->Passive);

    sleep(1);

    }

  }

  // -------------------- The One Who Grips Faust --------------------

  // The One Who Grips Faust - Passive
  if (isId(c->ID, "The One Who Grips Faust") == 0) {
      // 2. Fanatic Logic: เพิ่มพลังถ้ามี Fanatic และศัตรูติดตะปู
      if (c->Passive > 0 && c->skills[2].active > 0) {
          c->FinalPowerBoost += c->Passive;
          printf("\n%s gains Final Power equal to Fanatic (%d)\n", c->name, c->Passive);

        sleep(1);
      }

      // 7. You Must Accept the Pain!: เปลี่ยนสกิลเมื่อตะปูเยอะ
      if (chosenSkill == &c->skills[2] && c->skills[2].active >= 3) {
          chosenSkill = &c->skills[3]; // ใช้ Purify แทน Execution
          printf("\n%s has 3+ Nails, using 'Purify' instead\n", c2->name);

        sleep(1);
      }

      // 5. Gaze: เป้าหมายรับดาเมจแรงขึ้น 20%
      if (c->skills[4].active > 0) {
          c2->Protection -= 20; 
          printf("\n%s takes +20%% damage from Gaze.\n", c2->name);

        sleep(1);
      }
  }

  // The One Who Grips Faust - Skill 1
  if (isId(c->ID, "The One Who Grips Faust") == 0 && chosenSkill == &c->skills[0]) {

    int gain = c2->Bleed[0]/3;
    if (gain > 1) gain = 1;

    if (gain > 0) {

    c->CoinPowerBoost += gain;

    printf("\n%s at 3+ Bleed Stack (%d), %s gains +1 Coin Power\n",
       c2->name, c2->Bleed[0], c->name);

    sleep(1);
    }

    gain = c2->Bleed[1]/3;
    if (gain > 1) gain = 1;

    if (gain > 0) {

    c->ClashPower += 2;

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

    c->CoinPowerBoost += gain;

    printf("\n%s at 6+ Bleed Stack (%d), %s gains +1 Coin Power\n",
       c2->name, c2->Bleed[0], c->name);

    sleep(1);
    }

     gain = c2->Bleed[1]/3;
      if (gain > 1) gain = 1;

      if (gain > 0) {

      c->ClashPower += 1;

      printf("\n%s at 3+ Bleed Count (%d), %s gains +1 Clash Power\n",
         c2->name, c2->Bleed[1], c->name);

      sleep(1);
      }
  }

  // The One Who Grips Faust - Skill 3
  if (isId(c->ID, "The One Who Grips Faust") == 0 && chosenSkill == &c->skills[2]) {

    c->skills[6].active += 2;

    printf("\n%s gains 2 Fanatic next turn\n",
       c->name);

    sleep(1);

    int gain = (c2->Bleed[0] + c->skills[2].active)/8;
    if (gain > 1) gain = 1;

    if (gain > 0) {

    c->CoinPowerBoost += 2;

    printf("\n%s at 8+ (Bleed Stack + Nail Stack) (%d), %s gains +2 Coin Power\n",
       c2->name, (c2->Bleed[0] + c->skills[2].active), c->name);

    sleep(1);
    }

     gain = c2->Bleed[1]/3;
      if (gain > 1) gain = 1;

      if (gain > 0) {

      c->ClashPower += 2;

      printf("\n%s at 3+ Bleed Count (%d), %s gains +2 Clash Power\n",
         c2->name, c2->Bleed[1], c->name);

      sleep(1);
      }
  }

  // The One Who Grips Faust - Skill 4
  if (isId(c->ID, "The One Who Grips Faust") == 0 && chosenSkill == &c->skills[3]) {

    int gain = (c2->Bleed[0] + c->skills[2].active)/6;
    if (gain > 2) gain = 2;

    if (gain > 0) {

    c->CoinPowerBoost += gain;

    printf("\n%s gains +1 Coin Power (%d - Max 2) for every 6 (Bleed Stack + Nail Stack) (%d)\n",
       c->name, gain, (c2->Bleed[0] + c->skills[2].active));

    sleep(1);
    }

    gain = 1 * (c2->Bleed[0] + c2->Bleed[1]);
    if (gain > 50) gain = 50;

    if (gain > 0) {

    c->DamageUp += gain;

    printf("\n%s deals +1%% damage (%d%% - Max 50%%) for every (Bleed Stack + Bleed Count) Stack (%d)\n",
       c->name, gain, (c2->Bleed[0] + c2->Bleed[1]));

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

        // 2. Final Power +1 for every 10% Missing HP (Self + Target) (Max 3)
        float missP = ((c->MAX_HP - c->HP)/c->MAX_HP + (c2->MAX_HP - c2->HP)/c2->MAX_HP) * 20.0f;
        int Boost = (int)missP; 
        if (Boost > 3) Boost = 3;
        *tempOffense += Boost;
        if (Boost > 0) {printf("\n - Gains +1 Offense for every 20%% (missing HP percentage on target + missing HP percentage on self; rounded down) (%d - Max 3)", Boost);}

        // 3. Deal +1% damage for every 3 Sanity higher than target (Max 15%)
        if (c->Sanity > c2->Sanity) {
            int spDiff = (c->Sanity - c2->Sanity) / 3;
            if (spDiff > 15) spDiff = 15;
            c->DamageUp += spDiff;
            printf("\n - If this unit's Sanity higher than the target's, deals +1%% damage for every 3 Sanity different (%d%% - Max 15%%)",  spDiff);

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
        c->DamageUp += dmgBoost; // +2% ต่อ Grace หรือ 20% ถ้าเต็ม 9
        printf("\n%s +2%% damage with Skills marked with 'Mark of the Prescript' for every 'Grace of the Prescript' on self (%d%% - Max 16%%)\n", c->name, dmgBoost);

      sleep(1);
      }
    }

    *tempOffense += (c->skills[0].active / 3); // Offense +1 ทุก 3 Grace

    if ((c->skills[0].active / 3) > 0) { printf("\n%s gains +1 Offense (%d) for every 3 'Grace of the Prescript' (%d)\n", c->name, (c->skills[0].active / 3), c->skills[0].active); sleep(1);}
  }


  // The House of Spiders: The Index Nursefather Yi Sang - Skill 1 Buff
  if (isId(c->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && chosenSkill == &c->skills[0]) {

    if (c->skills[4].active == 0) {

      c->ClashPower += 1;
      c->DamageUp += 20;

      printf("\n%s's Skill is marked with 'Mark of the Prescript', Clash Power +1 and deal +20%% more damage\n", c->name);

      sleep(1);
      
    }

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
      float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

      int gain = (missingSelf + missingEnemy) / 10;
      if (gain > 2) gain = 2;

    if (gain > 0) {

      c->CoinPowerBoost += gain;

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

      c->FinalPowerBoost += 1;

      printf("\n%s's Skill is marked with 'Mark of the Prescript', Final Power +1\n", c->name);

      sleep(1);

    }

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
      float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

      int gain = (missingSelf + missingEnemy) / 10;
      if (gain > 2) gain = 2;

    if (gain > 0) {

      c->CoinPowerBoost += gain;

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

      c->DamageUp += 20;

      printf("\n%s's Skill is marked with 'Mark of the Prescript', deal +20%% more damage\n", c->name);

      sleep(1);

    }

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
      float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

      int gain = (missingSelf + missingEnemy) / 10;
      if (gain > 2) gain = 2;

    if (gain > 0) {

      c->CoinPowerBoost += gain;

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
    int netCoinPower = c->CoinPowerBoost;
    if (netCoinPower > 0) {
        c->FinalPowerBoost += (netCoinPower * 5);
        c->DamageUp += (netCoinPower * 25.0f);

        // ยกเลิกค่า Coin Power Boost เดิม เพื่อไม่ให้เหรียญมีค่าพลังเพิ่มขึ้นจริงๆ
        c->CoinPowerBoost = 0;
        printf("\n%s Converting %d Coin Power to +%d Final Power, +%d%% Damage", c->name, netCoinPower, netCoinPower * 5, netCoinPower * 25);

      sleep(1);
    }

    printf("\n%s's this Skill is not affected by Paralyze\n", c->name);

      sleep(1);

      c->ClashPower += 3;

    printf("\n%s gains +3 Clash Power\n", c->name);

      sleep(1);

    if (c->skills[4].active == 2) {

      c->DamageUp += 10;

      printf("\n%s's Skill is marked with 'Mark of the Prescript', deal +10%% more damage\n", c->name);

      sleep(1);
      
    }

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
      float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

      int gain = (missingSelf + missingEnemy) / 15;
      if (gain > 2) gain = 2;

      c->FinalPowerBoost += gain;

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

     c->DamageUp += dmgvalue;

    printf("\n%s gains %d%% more damage from 'Book of Vengeance [Sinclair]' (Max 30%%)\n", c->name, c->skills[1].active);

    sleep(1);

    if (c->skills[1].active >= 10) {

       c->DamageUp += 30;

      printf("\n%s at 10+ 'Book of Vengeance [Sinclair]' Stack, gain 30%% Damage Up\n", c->name);

      sleep(1);
    }

    if (c->skills[1].active >= 20) {

       c->ClashPower += 1;
      c->BasePowerBoost += 1;

      printf("\n%s at 20+ 'Book of Vengeance [Sinclair]' Stack, gain 1 Clash Power Up and 1 Base Power Up\n", c->name);

      sleep(1);
    }

    if (c->skills[1].active >= 30) {

       c->DamageUp += 50;

      printf("\n%s at 30 'Book of Vengeance [Sinclair]' Stack, gains 50%% Damage Up\n", c->name);

      sleep(1);
    }

  }

  // The Middle Little Brother Sinclair - Skill 1 Buff
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill == &c->skills[0]) {

    int MissingHP = ((c->MAX_HP - c->HP) / c->MAX_HP) * 100;

    int boost = (int)(abs(MissingHP) / 10);
    if (boost > 0) {

      c->CoinPowerBoost += 1;

       printf("\n%s at 10%% missing HP, Coin Power +1\n", c->name);

    }

    if (c->Sanity < 45) {
    
    updateSanity(c, 5);

    printf("\n%s heals 5 Sanity (%d)\n", c->name, c->Sanity);

    } else {

      c->ClashPower++;

      printf("\n%s at max Sanity, gain +1 Clash Power instead\n", c->name);
      
    }

    sleep(1);
  }

  // The Middle Little Brother Sinclair - Skill 2 Buff
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill == &c->skills[1]) {

    int MissingHP = ((c->MAX_HP - c->HP) / c->MAX_HP) * 100;

    int boost = (int)(abs(MissingHP) / 10);
    if (boost > 0) {

      c->CoinPowerBoost += 1;
      
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

      c->CoinPowerBoost += boost;

       printf("\n%s gains +1 Coin Power for every 10%% missing HP (%d - Max 2)\n", c->name, boost);

       sleep(1);
    }
  }

  // The Middle Little Brother Sinclair - Skill 4 Buff
  if (isId(c->ID, "The Middle Little Brother Sinclair") == 0 && chosenSkill == &c->defenseSkill[0]) {

    c->skills[4] = *chosenSkill;

    c->skills[2].active = 1; // Tell game that using this skill for lose resonance

    int gainvalue = (int)((c->MAX_HP - c->HP) * 0.30f); // 30%% as shield

    c->TempShield += gainvalue;

    printf("\n%s gains 30%% of missing HP as Shield (%d, Rounded down) (Shield %.2f) (Once per Turn)\n", c->name, gainvalue, c->Shield + c->TempShield);

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

      c->DamageUp += 100 + 10*(c->Passive - 6);

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

    c->DamageUp -= 20;
    c->FinalPowerBoost -= 1;

    printf("\n%s's 'Incomplete Arbiter' activated, deals -20%% damage, Final Power -1\n", c->name);

    sleep(1);
  } else if (isId(c->ID, "Binah") == 0 && c->Passive) {

      c->DamageUp += 50;
      c->FinalPowerBoost += 2;

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

    c->Protection += 30;
    c->ProtectionNextTurn += 30;

    printf("\n%s takes -30%% damage for this turn and next turn\n", c->name);

    sleep(1);
  }

  // Binah - Fairy buff
  if (isId(c->ID, "Binah") == 0 && c->skills[0].active > 0 && c->Passive) {

    int boost = c->skills[0].active * 20;

    c->DamageUp += boost;
    c->BasePowerBoost += c->skills[0].active*10;

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

    c->ClashPower -= 2 * c->skills[15].active;

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
    c->DamageUp += gain;

    printf("\n%s deals 20%% more damage(%d%%) for every 3 Coffin (%d)\n",
       c->name, gain, c->Passive);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – Clash power coffin
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 && c->Passive >= 5) {

    int gain;

    gain = c->Passive/5;
    c->ClashPower += gain;

    printf("\n%s gains 1 Clash Power(%d) for every 5 Coffin (%d)\n",
       c->name, gain, c->Passive);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[1]) && c->Passive >= 4) {

    int gain = c->Passive/4;

    c->CoinPowerBoost += gain;

    printf("\n%s gains 1 Coin Power(%d) for every 4 Coffin (%d)\n",
       c->name, gain, c->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0] || chosenSkill == &c->defenseSkill[0]) && abs(c->Sanity - c2->Sanity) >= 10) {

    int gain = abs(c->Sanity - c2->Sanity)/10;
    if (gain > 2) gain = 2;

    c->CoinPowerBoost += gain;

    printf("\n%s gains 1 Coin Power for every 10 Sanity different (%d - Max 2)\n",
       c->name, gain);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0]) && c->Passive >= 3) {

    c->ClashPower += 1;

    printf("\n%s at 3+ Coffin(%d), Clash Power +1\n",
       c->name, c->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1/2
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1] || chosenSkill == &c->defenseSkill[0]) && abs(c->Sanity - c2->Sanity) >= 10) {

    c->ClashPower += 1;

    printf("\n%s at 10+ Sanity different, Clash Power +1\n",
       c->name);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 3
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[2]) && abs(c->Sanity - c2->Sanity) >= 15) {

    int gain = abs(c->Sanity - c2->Sanity)/15;
    if (gain > 4) gain = 4;

    c->CoinPowerBoost += gain;

    printf("\n%s gains 1 Coin Power for every 15 Sanity different (%d - Max 4)\n",
       c->name, gain);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 4
  if (isId(c->ID, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[3]) && abs(45 - c->Sanity) >= 20) {

    int gain = abs(45 - c->Sanity)/20;
    if (gain > 4) gain = 4;

    c->BasePowerBoost += gain;

    printf("\n%s gains 1 Base Power for every 20 Sanity further from 45 (%d - Max 4)\n",
       c->name, gain);

      sleep(1);
  }

  //-------------------------------------------------

// ---------------------- Meursault:Blade Lineage Mentor ---------------------
  
  //        Meursault:Blade Lineage Mentor - Remembrance
  if ((isId(c->ID, "Meursault:Blade Lineage Mentor") == 0 &&
       c->HP <= c->MAX_HP * 0.6)) {

      printf("\n%s HP at 60%% or less HP, Apply 'Remembrance' buff on self, Gains buff at 10+ Sanity or 30+ Sanity further from 0\n",c->name);

    int PowerBuff;
    int ProtectionBuff;
      int DamageBuff;

      if (abs(c->Sanity) >= 30 && chosenSkill != &c->skills[2]) {

        PowerBuff = (4/chosenSkill->Coins) < 1 ? 1 : (4/chosenSkill->Coins);
        DamageBuff = (50/chosenSkill->Coins);

        c->CoinPowerBoost += PowerBuff;
        c->DamageUp += DamageBuff;

        printf("At 30+ Sanity(%d) further from 0, gain %d Coin Power and gain %d%% more damage\n", c->Sanity, PowerBuff, DamageBuff);

        sleep(1);
      }
        
    else if (abs(c->Sanity) >= 10 && chosenSkill != &c->skills[2]) {

      PowerBuff = (3/chosenSkill->Coins) < 1 ? 1 : (3/chosenSkill->Coins);
      DamageBuff = (30/chosenSkill->Coins);

      c->CoinPowerBoost += PowerBuff;
      c->DamageUp += DamageBuff;
      
      printf("At 10+ Sanity(%d) further from 0, gain %d Coin Power and gain %d%% more damage\n", c->Sanity, PowerBuff, DamageBuff);
      
      sleep(1);
    } else if (abs(c->Sanity) >= 30 && chosenSkill == &c->skills[2]) {

      ProtectionBuff = 50;
          DamageBuff = 50;

          c->Protection += ProtectionBuff;
          c->DamageUp += DamageBuff;

          printf("At 30+ Sanity(%d) further from 0, take -%d damage and gain %d%% more damage\n", c->Sanity, ProtectionBuff, DamageBuff);

          sleep(1);
        }

      else if (abs(c->Sanity) >= 10 && chosenSkill == &c->skills[2]) {

          ProtectionBuff = 25;
        DamageBuff = 30;

        c->Protection += ProtectionBuff;
        c->DamageUp += DamageBuff;

        printf("At 10+ Sanity(%d) further from 0, take -%d damage and gain %d%% more damage\n", c->Sanity, ProtectionBuff, DamageBuff);

        sleep(1);
      } 

      printf("\n%s: \"If you will cut... then wager your life on it.\"\n", c->name);

    }

  //        Meursault:Blade Lineage Mentor - Skill 1 buff
  if ((isId(c->ID, "Meursault:Blade Lineage Mentor") == 0 && chosenSkill == &c->skills[0])) {

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    int boost = (int)(abs(HPDifferent) / 5);
    if (boost > 0) {

    c->CoinPowerBoost += 1;

      printf("\n%s at 5%% HP different, Coin Power +1\n", c->name);

      sleep(1);
    }

  }

  //        Meursault:Blade Lineage Mentor - Skill 2 buff
  if ((isId(c->ID, "Meursault:Blade Lineage Mentor") == 0 && chosenSkill == &c->skills[1])) {

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    int boost = (int)(abs(HPDifferent) / 7);
    if (boost > 0) {

    c->CoinPowerBoost += 1;

      printf("\n%s at 7%% HP different, Coin Power +1\n", c->name);

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
  if (isId(c->ID, "Meursault:The Thumb") == 0 && chosenSkill == &c->skills[2] && c->Passive >= 1 && !c->skills[3].active && c->skills[2].active >= 3) {

    printf("\n%s at 1+ Tigermark Round and 3+ Tigermark Round spent, convert all Coins of 'Tanglecleaver' into Unbreakable Coins\n", c->name);

    chosenSkill->Unbreakable = chosenSkill->Coins;

    sleep(1);
  }

  // Meursault: The Thumb – all Unbreakable
  if (isId(c->ID, "Meursault:The Thumb") == 0 && c->Passive >= 1 && c->skills[3].active) {

    printf("\n%s at 1+ Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins\n", c->name);

     chosenSkill->Unbreakable = chosenSkill->Coins;

    sleep(1);
  }

  // Meursault: The Thumb – Overheat
  if (isId(c->ID, "Meursault:The Thumb") == 0 && c->Passive <= 0 && c->skills[3].active) {

    printf("\n%s at 0 Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins and Gain 'Overheat'\n", c->name);

     chosenSkill->Unbreakable = chosenSkill->Coins;

    sleep(1);

    int loseClashpower = c->skills[2].active / 4;
    if (loseClashpower > 5) loseClashpower = 5;

    c->ClashPower -= loseClashpower;

    printf("\nOverheat: Attack Skills Lose (cumulative number of Tigermark Rounds & Savage Tigermark Rounds spent / 4) Clash Power (%d - Max 5); however, gain the following effects(cumulative):\n", loseClashpower);

    printf(" - Cumulative Rounds spent: %d\n", c->skills[2].active);

    if (c->skills[2].active >= 8) {
      
       float missing = (c->MAX_HP - c->HP) / c->MAX_HP; // fraction of HP missing (0.0 - 1.0)
        int SkillUp = (int)(missing / 0.10f) * 10;  // 10% for every 10%
        if (SkillUp > 50) SkillUp = 50;      // cap at 50%

      c->Protection += SkillUp;

      printf(" - 8+ Rounds spent: Take 10%% less damage for every 10%% missing HP on self at Turn Start (%d%% - Max 50%%)\n", SkillUp);
      
    } 
    if (c->skills[2].active >= 14) {

      printf(" - 14+ Rounds spent: On Clash Lose, Unbreakable Coins of this unit's Attack Skills deal +(75 + missing HP percentage on self)%% damage (Max 150%%)\n");
      
    } 
    
    if (c->skills[2].active >= 20) {

      //c->DamageUp += SkillUp;

      printf(" - 20+ Rounds spent: Deal +(HP percentage difference)%% damage against targets with higher remaining HP percentage than this unit (Max 50%%)\n");
      
    }

    sleep(1);
    
  }

  // Meursault:The Thumb - Skill 1 and 2 or defskill Final Power
  if (isId(c->ID, "Meursault:The Thumb") == 0 && abs(c->Sanity) >= 10 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1] || chosenSkill == &c->defenseSkill[0])) {

    int Buff = abs(c->Sanity);
    int FinalPowerbuff = Buff/10;
    int Max = 3;

    if (chosenSkill == &c->skills[1]) Max = 4;
    if (chosenSkill == &c->defenseSkill[0]) Max = 2;
    
     if (FinalPowerbuff > Max) FinalPowerbuff = Max;

     c->FinalPowerBoost += FinalPowerbuff;

      printf("\n%s gains +1 Final Power for every 10 Sanity further from 0 (%d - Max %d)\n", c->name, FinalPowerbuff, Max);

    sleep(1);
  }

  // Meursault:The Thumb - Skill def clash power
  if (isId(c->ID, "Meursault:The Thumb") == 0 && c->Passive <= 0 && (chosenSkill == &c->defenseSkill[0])) {

     c->ClashPower += 2;

      printf("\n%s at 0 'Tigermark Round' or 'Savage Tigermark Round', gains +2 Clash Power\n", c->name);

    sleep(1);
  }

  // Meursault:The Thumb - Skill 3 and 4 coin power
  if (isId(c->ID, "Meursault:The Thumb") == 0 && abs(c->Sanity) >= 20 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int Buff = abs(c->Sanity);
    int CoinPowerbuff = Buff/20;
     if (CoinPowerbuff > 2) CoinPowerbuff = 2;

     c->CoinPowerBoost += CoinPowerbuff;

      printf("\n%s gains +1 Coin Power for every 20 Sanity further from 0 (%d - Max 2)\n", c->name, CoinPowerbuff);

    sleep(1);
  }

  // --------------------------------------------
  
  // Shin buffs (temporary, print once per skill selection)
  if ((isId(c->ID, "Meursault:The Thumb") == 0 &&
       c->skills[3].active) ||
      (isId(c->ID, "Lei heng") == 0 && c->skills[4].active == 1 && c->skills[0].active != 3)) {

    if (isId(c->ID, "Meursault:The Thumb") == 0 && c->skills[2].active < 8) {

      int DamageBuff = (abs(c2->Sanity - c->Sanity)) > 20 ? 20 : (abs(c2->Sanity - c->Sanity));
      c->DamageUp += DamageBuff;
      
      printf("\n%s 'Unrelenting Spirit [剛氣]' activated! \n deal +1%% damage for every different Sanity between enemy and this unit (%d%% - Max 20%%)\n", c->name, DamageBuff);

    } else if (isId(c->ID, "Meursault:The Thumb") == 0 && c->skills[2].active >= 8) {

      int DamageBuff = (2*((int)(abs(c2->Sanity - c->Sanity)))) > 40 ? 40 : (2*(int)(abs(c2->Sanity - c->Sanity)));
      c->DamageUp += DamageBuff;

      printf("\n%s at 8+ (sum of Tigermark Round and Savage Tigermark Round spent) \n - 'Unrelenting Spirit - Shin [剛氣-心]' activated! \n - Defense +3, deal +2%% damage for every different Sanity between enemy and this unit (%d%% - Max 40%%)\n", c->name, DamageBuff);

      *tempDefense += 3;

    } else {

      float missing = (c->MAX_HP - c->HP) / (c->MAX_HP * 0.2); // fraction of HP missing (0.0 - 1.0)
      int SkillDamageUp = (int)(missing) * 10;  // 10% for every 20%
      if (SkillDamageUp > 30) SkillDamageUp = 30;      // cap at 30%

      int SkillFinalPowerBoost = (int)(missing); // +1 for every 20%
      if (SkillFinalPowerBoost > 3) SkillFinalPowerBoost = 3; // cap at 3

      c->DamageUp += SkillDamageUp;
      c->FinalPowerBoost += SkillFinalPowerBoost;

      int DamageBuff = (abs(c2->Sanity - c->Sanity)) > 20 ? 20 : (abs(c2->Sanity - c->Sanity));
      c->DamageUp += DamageBuff;

      printf(
        "\n%s's 'Unrelenting Spirit [剛氣]' activated!\n"
        " - Gain 10%% more damage(%d%%) and +1 Final Power (%d) for every 20%% HP missing (Max 3 each)\n"
        " - deal +1%% damage for every different Sanity between enemy and this unit (%d%% - Max 20%%) \n"
        " - All skills' 1 breakable coin become unbreakable coin\n",
        c->name, SkillDamageUp, SkillFinalPowerBoost, DamageBuff
      );
      
      sleep(1);
      
      printf("\n%s: \"That's more like it. Y'all are firin' me up!\"\n", c->name);

      if (chosenSkill->Unbreakable < chosenSkill->Coins) {
        chosenSkill->Unbreakable = 1;
      }
    }

    sleep(1);
  }

  
 // ---------------------------- Lei heng -----------------------------
    if ((isId(c->ID, "Lei heng") == 0 && c->skills[4].active == 1 && c->skills[0].active == 3) || isId(c2->ID, "Muga Ryōshū") == 0) {

    if (isId(c2->ID, "Muga Ryōshū") == 0) {
      c->skills[4].active = 1;
        c->skills[0].active = 3;
    }
    
    float missing = (c->MAX_HP - c->HP) / (c->MAX_HP * 0.15); // fraction of HP missing (0.0 - 1.0)
    int SkillDamageUp = (int)(missing) * 10;  // 10% for every 15%
    if (SkillDamageUp > 50) SkillDamageUp = 50;      // cap at 50%

    int SkillFinalPowerBoost = (int)(missing); // +1 for every 15%
    if (SkillFinalPowerBoost > 5) SkillFinalPowerBoost = 5; // cap at 3

    c->DamageUp += SkillDamageUp;
    c->FinalPowerBoost += SkillFinalPowerBoost;

    int DamageBuff = (2*((int)(abs(c2->Sanity - c->Sanity)))) > 40 ? 40 : (2*(int)(abs(c2->Sanity - c->Sanity)));
    c->DamageUp += DamageBuff;
    
    printf("\n%s's 'Unrelenting Spirit - Shin [剛氣-心]' activated!\n"
      " - Gain 10%% more damage(%d%%) and +1 Final Power(%d) for every 15%% HP missing (Max 5 each) \n"
      " - deal +2%% damage for every different Sanity between enemy and this unit (%d%% - Max 40%%) \n"
     " - All skills' 1 breakable coin become unbreakable coin\n",
     c->name, SkillDamageUp, SkillFinalPowerBoost, DamageBuff);

    sleep(1);
    
     printf("\n%s: \"That's more like it. Y'all are firin' me up!\"\n", c->name);
 
    if (chosenSkill->Unbreakable < chosenSkill->Coins) {
      chosenSkill->Unbreakable = 1;
    }

  }

  // Lei heng – heal Sanity Passive and Buff dmg Passive
  if (isId(c->ID, "Lei heng") == 0 && c->Sanity > -45) {

      // 1. Calculate the success threshold
      // If sanity is 10, threshold becomes 40.
      int threshold = 50 - c->Sanity;

      // 2. Safety check: ensure the threshold doesn't drop below 0
      if (threshold < 0) threshold = 0;

      // 3. Roll a number between 0 and 99
      // If the roll is less than the threshold, the event happens.
      if ((rand() % 100) < threshold) {
          // SUCCESS (Event triggered)

        // Buff dmg Passive

        if (c->Sanity >= 0) {

        int randombuff = rand() % 30 + 1;

        c->DamageUp += randombuff;

        printf("\n%s deals +%d%% damage\n", c->name, randombuff);

        }

        // heal Sanity Passive
        
        if (c->Sanity <= 15) {

    int randomheal = rand() % 3 + 2;

    if (c->Sanity < 0) randomheal *= 2;

    updateSanity(c, randomheal);
    

    printf("\n%s heals %d Sanity (%d)\n", c->name, randomheal, c->Sanity);

        }

    }

    sleep(1);

  }

  // Lei heng – skill 3 -> skill 6 if HP ≤ 40%
  if (isId(c->ID, "Lei heng") == 0 && c->HP < c->MAX_HP * 0.4 &&
      (chosenSkill == &c->skills[2])) {

    chosenSkill = &c->skills[4];

    sleep(1);

  }

  // Lei heng – skill 5 buff
  if (isId(c->ID, "Lei heng") == 0 && (chosenSkill == &c->skills[4])) {

    c->DamageUpNextTurn += 10;

    printf("\n%s gains 10%% more damage next turn\n", c->name);

    sleep(1);
  }

  // Lei heng – inner strength skill buff no blast skill 1
  if (isId(c->ID, "Lei heng") == 0 &&
    (chosenSkill == &c->skills[0]) && (strstr(c->skills[0].name, "Blast") == NULL)) {

    if (c->skills[0].active == 2) {
      
    int Boost = c->Passive / 4;
    if (Boost > 2) Boost = 2;
      c->FinalPowerBoost += Boost;
    if (Boost > 0)
      printf("\n%s gains +1 Final Power for every 4 Inner Strength [底力] (%d - Max 2)\n", c->name, Boost);
      
    } else if (c->skills[0].active == 3) {

      int Boost = c->Passive / 4;
      if (Boost > 2) Boost = 2;
        c->FinalPowerBoost += Boost;
      if (Boost > 0)
        printf("\n%s gains +1 Final Power for every 4 Extreme Strength [底力] (%d - Max 2)\n", c->name, Boost);

    }

  }  // Lei heng – inner strength skill buff blast skill 1
  else if (isId(c->ID, "Lei heng") == 0 &&
     (chosenSkill == &c->skills[0]) && (strstr(c->skills[0].name, "Blast") != NULL)) {

  if (c->skills[0].active == 2) {

  int Boost = c->Passive / 8;
  if (Boost > 2) Boost = 2;
    c->FinalPowerBoost += Boost;
  if (Boost > 0)
    printf("\n%s gains +1 Final Power for every 8 Inner Strength [底力] (%d - Max 2)\n", c->name, Boost);

  } else if (c->skills[0].active == 3) {

    int Boost = c->Passive / 8;
    if (Boost > 3) Boost = 3;
      c->FinalPowerBoost += Boost;
    if (Boost > 0)
      printf("\n%s gains +1 Final Power for every 8 Extreme Strength [底力] (%d - Max 3)\n", c->name, Boost);

  }

  }

    // Lei heng – inner strength skill buff no blast skill 2
    if (isId(c->ID, "Lei heng") == 0 &&
      (chosenSkill == &c->skills[1]) && (strstr(c->skills[1].name, "Blast") == NULL)) {

      if (c->skills[0].active == 2) {

      int Boost = c->Passive / 4;
      if (Boost > 2) Boost = 2;
        c->FinalPowerBoost += Boost;
      if (Boost > 0)
        printf("\n%s gains +1 Final Power for every 4 Inner Strength [底力] (%d) (%d - Max 2)\n", c->name, c->Passive, Boost);

      } else if (c->skills[0].active == 3) {

        int Boost = c->Passive / 4;
        if (Boost > 2) Boost = 2;
          c->FinalPowerBoost += Boost;
        if (Boost > 0)
          printf("\n%s gains +1 Final Power for every 4 Extreme Strength [底力] (%d) (%d - Max 2)\n", c->name, c->Passive, Boost);

      }

    }  // Lei heng – inner strength skill buff blast skill 2
    else if (isId(c->ID, "Lei heng") == 0 &&
       (chosenSkill == &c->skills[1]) && (strstr(c->skills[1].name, "Blast") != NULL)) {

    if (c->skills[0].active == 2) {

    int Boost = c->Passive / 6;
    if (Boost > 2) Boost = 2;
      c->FinalPowerBoost += Boost;
    if (Boost > 0)
      printf("\n%s gains +1 Final Power for every 6 Inner Strength [底力] (%d) (%d - Max 2)\n", c->name, c->Passive, Boost);

    } else if (c->skills[0].active == 3) {

      int Boost = c->Passive / 6;
      if (Boost > 3) Boost = 3;
        c->FinalPowerBoost += Boost;
      if (Boost > 0)
        printf("\n%s gains +1 Final Power for every 6 Extreme Strength [底力] (%d) (%d - Max 3)\n", c->name, c->Passive, Boost);

    }

    }

      // Lei heng – inner strength skill buff skill 4
      if (isId(c->ID, "Lei heng") == 0 &&
        (chosenSkill == &c->skills[3])) {

        if (c->skills[0].active == 2) {

        int Boost = c->Passive / 10;
        if (Boost > 2) Boost = 2;
          c->CoinPowerBoost += Boost;
        if (Boost > 0)
          printf("\n%s gains +1 Coin Power for every 10 Inner Strength [底力] (%d) (%d - Max 2)\n", c->name, c->Passive, Boost);

        } else if (c->skills[0].active == 3) {

          int Boost = c->Passive / 10;
          if (Boost > 2) Boost = 2;
            c->CoinPowerBoost += Boost;
          if (Boost > 0)
            printf("\n%s gains +1 Coin Power for every 10 Extreme Strength [底力] (%d) (%d - Max 2)\n", c->name, c->Passive, Boost);

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
        c->AttackPowerBoost += Boost;
      
      printf("\n%s at 10 consumed, gains +1 Attack Power for every 10 Stack consumed (%d - Max 2)\n", c->name, Boost);
    }

    int Boost = abs(c2->Sanity - c->Sanity) / 10;
    if (Boost > 5) Boost = 5;
      c->FinalPowerBoost += Boost;
    if (Boost > 0 && c->Passive >= 25)
      printf("\n%s at 25 consumed, gains +1 Final Power for every 10 Sanity different (%d - Max 5)\n", c->name, Boost);

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
        c->AttackPowerBoost += Boost;

      printf("\n%s at 10 consumed, gains +1 Attack Power for every 10 Stack consumed (%d - Max 4)\n", c->name, Boost);
    }

    int Boost = abs(c2->Sanity - c->Sanity) / 10;
    if (Boost > 6) Boost = 6;
      c->FinalPowerBoost += Boost;
    if (Boost > 0 && c->Passive >= 25)
      printf("\n%s at 25 consumed, gains +1 Final Power for every 10 Sanity different (%d - Max 6)\n", c->name, Boost);
    if (Boost >= 50) {

      c->CoinPowerBoost +=1;
      c->DamageUp += 50;
      
      printf("\n%s gains 1 Coin Power and deal +50%% damage for every 50 consumed\n", c->name);
    }

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

    c->ClashPower += 2;
    c->DamageUp += 20;
    
    printf("\n%s gains +2 Clash Power and +20%% damage\n", c->name);

    sleep(1);
    }
  }

  // Erlking Heathcliff – skill 5
  if (isId(c->ID, "Erlking Heathcliff") == 0 && chosenSkill == &c->skills[5] && abs(c2->Sanity - c->Sanity) > 0) {

    int buff = abs(c2->Sanity - c->Sanity);
    if (buff > 2) buff = 2;

    c->ClashPower += buff;

    printf("\n%s gains 1 Clash Power for every 5 Sanity different (%d - Max 2)\n", c->name, buff);

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

    if (c->skills[3].active > 0 && (c->skills[0].active + c->skills[1].active) < 30) {
      int boost = c->skills[3].active * 0.2;  // 0.2% per consumed fuel
      if (boost > 40) {
            boost = 40;
      }
      c->DamageUp += boost;
      printf("\n%s's HP or %s's HP at 75%% or less HP, Deal +0.2%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (%d%% - Max 40%%)\n",
         c2->name, c->name, boost);

       printf("\n%s: \"Let's see how much more of this you can take.\"\n", c->name);
      
    } else if (c->skills[3].active > 0 && (c->skills[0].active + c->skills[1].active) >= 30) {
      int boost = c->skills[3].active * 0.3;  // 0.3% per consumed fuel
      if (boost > 60) {
        boost = 60;
      }
      c->DamageUp += boost;
        printf("\n%s's HP or %s's HP at 75%% or less HP, and main target at 30+ (Burn Stack + Burn Count) (%d), Deal +0.3%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (%d%% - Max 60%%)\n",
           c2->name, c->name, (c->skills[0].active + c->skills[1].active), boost);

         printf("\n%s: \"Let's see how much more of this you can take.\"\n", c->name);
      }
    

    sleep(1);

    }
  }

  // Gregor:Firefist - S1 Coin power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[0]) {

    int gain = c->skills[0].active / 3;

      if (gain > 0) {
        if (gain > 2) gain = 2;

    printf("\n%s gains +1 Coin Power(%d) for every 3 Burn Stack on target(%d) (Max 2)\n", c->name,
           gain, c->skills[0].active);

         c->CoinPowerBoost += gain;

      }

    sleep(1);
    }
  
  // Gregor:Firefist - S2 Coin power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[1]) {

    int gain = c->skills[0].active / 6;

      if (gain > 0) {
        if (gain > 2) gain = 2;

    printf("\n%s gains +1 Coin Power(%d) for every 3 Burn Stack on target(%d) (Max 2)\n", c->name,
           gain, c->skills[0].active);

         c->CoinPowerBoost += gain;

      }

    sleep(1);
    }
  
  // Gregor:Firefist - S3 Base power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[2]) {

    int gain = c->skills[0].active / 6;

      if (gain > 0) {
        if (gain > 3) gain = 3;

    printf("\n%s gains +1 Base Power(%d) for every 6 Burn Stack on target(%d) (Max 3)\n", c->name,
           gain, c->skills[0].active);

         c->BasePowerBoost += gain;

      }

    sleep(1);
  }
  
  // Gregor:Firefist - S3 Coin power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[2]) {

    int gain = c->skills[1].active / 3;

      if (gain > 0) {
        if (gain > 2) gain = 2;

    printf("\n%s gains +1 Coin Power(%d) for every 3 Burn Count on target(%d) (Max 2)\n", c->name,
           gain, c->skills[1].active);

         c->CoinPowerBoost += gain;

      }

    sleep(1);
  }

  // Gregor:Firefist - S3 Final power buff
  if (isId(c->ID, "Gregor:Firefist") == 0 &&
      chosenSkill == &c->skills[2]) {

    int gain = (c->skills[0].active + c->skills[1].active) / 10;

      if (gain > 0) {
        if (gain > 3) gain = 3;

    printf("\n%s gains +1 Final Power(%d) for every 10 (Burn Stack + Count) on target(%d) (Max 3)\n", c->name,
           gain, c->skills[0].active + c->skills[1].active);

         c->FinalPowerBoost += gain;

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
    c->BasePowerBoost += BaseBuff;
    c->DamageUp += DmgBuff;

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

    c->BasePowerBoost += Boost;
    
    sleep(1);
  }

  // Roland – Skill 2 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[1]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 5) Boost = 5;

    printf("\n%s gains +1 Base Power (%d - Max 5) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);
  }

  // Roland – Skill 3 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[2]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 5) Boost = 5;

    printf("\n%s gains +1 Coin Power (%d - Max 5) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->CoinPowerBoost += Boost;

    sleep(1);
  }

  // Roland – Skill 4 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[3]) && c->Passive >= 3) {

    int Boost = c->Passive/3;
    if (Boost > 4) Boost = 4;

    printf("\n%s gains +1 Base Power (%d - Max 4) for every 3 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);
  }

  // Roland – Skill 5 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[4]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 4) Boost = 4;

    printf("\n%s gains +1 Base Power (%d - Max 4) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);
  }

  // Roland – Skill 6 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[5]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 10) Boost = 10;

    printf("\n%s gains +1 Base Power (%d - Max 10) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);

    if (c->Passive >= 5) {

    Boost = c->Passive/5;
    if (Boost > 5) Boost = 5;

    printf("\n%s gains +1 Coin Power (%d - Max 5) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->CoinPowerBoost += Boost;

    sleep(1);
      
    }
    
  }

  // Roland – Skill 7 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[6]) && c->Passive >= 5) {

    int Boost = c->Passive/5;
    if (Boost > 3) Boost = 3;

    printf("\n%s gains +1 Base Power (%d - Max 3) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);

  }

  // Roland – Skill 8 buff
  if (isId(c->ID, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[7]) && c->Passive >= 5) {

    int Boost = c->Passive/5;
    if (Boost > 3) Boost = 3;

    printf("\n%s gains +1 Coin Power (%d - Max 3) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->CoinPowerBoost += Boost;

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

      c->FinalPowerBoost += buff;

    sleep(1);
  }

  // Roland – Skill 10 buff for shin (心)
    if (isId(c->ID, "Fixer grade 9?") == 0 && isId(c2->ID, "Binah") == 0 && c2->Passive == 1 && (chosenSkill == &c->skills[9])) {

      int buff = c->Passive/2;
      if (buff > 10) buff = 10;

      printf("\nIf %s has 'Shin (心) - The Black Silence', gain +1 Base Power (%d - Max 10) for every 2 Black Silence (%d)\n",
        c->name, buff, c->Passive);

        c->BasePowerBoost += buff;

      sleep(1);
    }

  // ---------------------------------------------------------------------

  // --------------------------- Hong lu:The Lord of Hongyuan ---------------------------

  // Hong lu:The Lord of Hongyuan - Skill 1 deal more damage on HP
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[0])) {

    int Boost = (c2->Rupture[0] + c->Poise[0]) / 6;
    if (Boost > 1) Boost = 1;

     if (Boost > 0) {

    c->CoinPowerBoost += Boost;

      printf("\n%s at 6+ (Rupture Stack on target + Poise Stack on self) (%d), Coin Power +1\n", c->name, (c2->Rupture[0] + c->Poise[0]));

    sleep(1);

     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 2 deal more damage on HP
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[1])) {

    int Boost = (c2->Rupture[0] + c->Poise[0]) / 6;
    if (Boost > 2) Boost = 2;

     if (Boost > 0) {

    c->CoinPowerBoost += Boost;

      printf("\n%s gains +1 Coin Power (%d - Max 2) for every 6 (Rupture Stack on target + Poise Stack on self) (%d)\n", c->name, Boost, (c2->Rupture[0] + c->Poise[0]));

    sleep(1);

     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 3 deal more damage on HP
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[2])) {

    int Boost = (c2->Rupture[0] + c->Poise[0]) / 4;
    if (Boost > 3) Boost = 3;

     if (Boost > 0) {

    c->CoinPowerBoost += Boost;

      printf("\n%s gains +1 Coin Power (%d - Max 3) for every 4 (Rupture Stack on target + Poise Stack on self) (%d)\n", c->name, Boost, (c2->Rupture[0] + c->Poise[0]));

    sleep(1);
     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 2 Gain
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[1])) {

    int gain = 3;

     if (gain > 0) {

    c->Poise[0] += gain;
       if (c->Poise[0] > 99) c->Poise[0] = 99;
       if (c->Poise[1] > 99) c->Poise[1] = 99;

      printf("\n%s gains +3 Poise Stack (%d)\n", c->name, c->Poise[0]);

    sleep(1);

     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 2 Gain
  if (isId(c->ID, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[2])) {

    c->Poise[0] += 5;
     c->Poise[1] += 3;
    if (c->Poise[0] > 99) c->Poise[0] = 99;
    if (c->Poise[1] > 99) c->Poise[1] = 99;

      printf("\n%s gains +5 Poise Stack (%d) and +3 Poise Count (%d)\n", c->name, c->Poise[0], c->Poise[1]);

    sleep(1);

  }

  // ---------------------------------------------------------------------------------


  // --------------------------- Yi sang:Fell Bullet -----------------

  // Yi sang:Fell Bullet - Buff s3
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[2]) {

    int gain = c->Poise[0] / 5;
    if (gain > 2) gain = 2;

    if (gain > 0) {

      c->CoinPowerBoost += gain;

      printf("\n%s gains +1 Coin Power for every 5 Poise Stack (%d) on self (%d - Max 2)\n", c->name, c->Poise[0], gain);

    sleep(1);
    }

    gain = c->Poise[1] / 3;
    if (gain > 2) gain = 2;

    if (gain > 0) {

      c->BasePowerBoost += gain;

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

      c->CoinPowerBoost += gain;

      printf("\n%s gains +1 Coin Power for every 5 Poise Stack (%d) on self (%d - Max 2)\n", c->name, c->Poise[0], gain);

    sleep(1);
    }

    gain = c->Poise[1] / 3;
    if (gain > 2) gain = 2;

    if (gain > 0) {

      c->BasePowerBoost += gain;

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

        c->CoinPowerBoost += gain;

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

      c->CoinPowerBoost += gain;

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

     c->Poise[0] += 2;
    if (c->Poise[0] > 99) c->Poise[0] = 99;
    if (c->Poise[1] > 99) c->Poise[1] = 99;

      printf("\n%s gains +2 Poise Stack (%d)\n", c->name, c->Poise[0]);

    sleep(1);

  }

  // Yi sang:Fell Bullet - Poise s3
  if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[2]) {

     c->Poise[0] += c->Passive;
    if (c->Poise[0] > 99) c->Poise[0] = 99;
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

        // Yi sang:Fell Bullet - Fell Bullet Clash power buff
    if (isId(c->ID, "Yi sang:Fell Bullet") == 0 &&
        c->skills[2].active > 0) {

      c->Poise[0] += 3;
      c->Poise[1] += 1;
      if (c->Poise[0] > 99) c->Poise[0] = 99;
      if (c->Poise[1] > 99) c->Poise[1] = 99;

        printf("\n%s gains +3 Poise Stack (%d) and +1 Poise Count (%d)\n", c->name, c->Poise[0], c->Poise[1]);

      sleep(1);

      c->ClashPower += c->skills[2].active * 2;

      printf("\n%s gains +2 Clash Power (%d) for every Fell Bullet (%d)\n", c->name, c->skills[2].active * 2, c->skills[2].active);

      sleep(1);
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

    c->ClashPower += 1;
    c->DamageUp += 20;
    c->Protection -= 20;

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
    
    c->CoinPowerBoost += boost;

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

    c->AttackPowerBoost += boost;

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

    c->CoinPowerBoost += boost;

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
    
    c->DamageUp += damageUP;

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

    c->DamageUp += boost;

    printf("\n%s deals +5%% damage for every 10%% HP different (%.0f%% - Max 50%%)\n", c->name, boost);

    sleep(1);
     }

    if (c->Passive >= 5 && chosenSkill == &c->skills[2]) {

      int boost = (c->Passive/5) * 20;
      if (boost > 100) boost = 100;

      c->DamageUp += boost;

      printf("\n%s deals +20%% damage (%d%% - Max 100%%) for every 5 Hardblood (%d)\n", c->name, boost, c->Passive);

      sleep(1);\
      
    } else if (c->Passive >= 5 && chosenSkill == &c->skills[5]) {

    int boost = (c->Passive/5) * 25;
    if (boost > 100) boost = 100;

    c->DamageUp += boost;

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

    c->DamageUp += boost;

    printf("\n%s gains 1 Base Power (%d - Max 3) for every 10 Hardblood on self (%d)\n", c->name, boost, c->Passive);

    sleep(1);
  }

  // Don Quixote:The Manager of La Manchaland - Hardblood Buff
  if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") == 0 && c->Passive >= 10 && c->Passive < 20) {

    int boost = (int)(c->Passive / 5);

    *tempOffense += boost;

    printf("\n%s has 10+ Hardblood, gains +1 Offense (%d) for every 5 stacks (%d)\n", c->name, boost, c->Passive);

    sleep(1);
  } else if (isId(c->ID, "Don Quixote:The Manager of La Manchaland") == 0 && c->Passive >= 20) {

    int boost = (int)(c->Passive / 5);

    *tempOffense += boost;
    *tempDefense += boost;

    printf("\n%s has 20+ Hardblood, gains +1 Offense and +1 Defense (%d) for every 5 stacks (%d)\n", c->name, boost, c->Passive);

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

    c->CoinPowerBoost += boost;

    printf("\n%s gains +1 Coin Power (%d - Max 2) for every 10 Hardblood on self (%d)\n", c->name, boost, c->Passive);

    sleep(1);
    }

    float P_HPDifferent = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
     float E_HPDifferent = (c2->MAX_HP - c2->HP) / c2->MAX_HP; // 0.0 - 1.0

    int HPDifferent = (P_HPDifferent - E_HPDifferent) * 100;

    boost = (int)(abs(HPDifferent) / 15);
    if (boost > 3) boost = 3;

     if (boost > 0) {

    c->CoinPowerBoost += boost;

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

    c->CoinPowerBoost += boost;

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

    c->BasePowerBoost += boost;

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

    c->CoinPowerBoost += boost;

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

    c->BasePowerBoost += boost;

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

    c->ClashPower += 5;

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
    c->ClashPower += 1;

    printf("\n%s at 10+ The Living & The Departed(%d), gains 1 Clash Power\n",
           c->name, c->Passive);

    sleep(1);
    }

    if (abs(c->Sanity - c2->Sanity) >= 10) {

      int buff = abs(c->Sanity - c2->Sanity)/10;
      if (buff > 2) buff = 2;

    c->CoinPowerBoost += buff;

    printf("\n%s gains 1 Coins Power for every 10 Sanity different (%d - Max 2)\n",
           c->name, buff);

      sleep(1);
    }
    
  }  // Lobotomy E.G.O::Solemn Lament Yi Sang - Clash power
  else if (isId(c->ID, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
            0 && (chosenSkill == &c->skills[2])) {

    if (c->Passive >= 5) {
      c->BasePowerBoost += c->Passive/5;

      printf("\n%s gains 1 Base Power(%d) for every 5 The Living & The Departed(%d)\n",
             c->name, c->Passive/5, c->Passive);

      sleep(1);
    }

    if (abs(c->Sanity - c2->Sanity) >= 10) {

      int buff = abs(c->Sanity - c2->Sanity)/10;
      if (buff > 2) buff = 2;
      
    c->CoinPowerBoost += buff;

    printf("\n%s gains 1 Coins Power for every 10 Sanity different (%d - Max 2)\n",
           c->name, buff);

    sleep(1);
    }
    
    }

    // ------------------------------------------------------------

  // ------------------------ Dawn Office Fixer Sinclair ----------------------------
  
  // Dawn Office Fixer Sinclair - Skill Buff S2
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && !c->skills[3].active && c->Sanity >= 20 && (chosenSkill == &c->skills[1])) {

    c->ClashPower += 1;

    printf("\n%s at 20+ Sanity, gains 1 Clash Power\n",
           c->name);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff base form S1
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->Sanity >= 10 && (chosenSkill == &c->skills[0])) {

    int boost = c->Sanity/10;
    if (boost > 2) boost = 2;

    c->BasePowerBoost += boost;

    printf("\n%s at 10+ Sanity, gains 1 Base Power for every 10 Sanity(%d - Max 2) (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff base form S3
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && !c->skills[3].active && c->Sanity >= 10 && (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[2])) {

    int boost = c->Sanity/10;
    if (boost > 2) boost = 2;
    
    c->CoinPowerBoost += boost;

    printf("\n%s at 10+ Sanity, gains 1 Coin Power for every 10 Sanity(%d - Max 2) (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S2
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && (chosenSkill == &c->skills[1])) {

    updateSanity(c, -(5));
    
    c->AttackPowerBoost += 5;

    printf("\n%s is in a Volatile E.G.O State consumes 5 Sanity (%d) to gain +5 Attack Power\n",
           c->name, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S4
  if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[2])) {

    if (c->Sanity >= 20) {
    int boost = 2*(c->Sanity/20);

    c->CoinPowerBoost += boost;

    printf("\n%s gains 2 Coin Power(%d) for every 20 Sanity (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

    }

  }
    
  // Dawn Office Fixer Sinclair - Skill Buff base form
 if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && !c->skills[3].active && c->Sanity >= 20) {

      c->ClashPower += c->Sanity/20;

      printf("\n%s at 20+ Sanity, gains 1 Clash Power(%d) for every 20 Sanity (%d)\n",
             c->name, c->Sanity/20, c->Sanity);

      sleep(1);

    }
  
  // Dawn Office Fixer Sinclair - Skill Buff Ego form
 if (isId(c->ID, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && c->Sanity >= 20) {

      c->CoinPowerBoost += c->Sanity/20;

      printf("\n%s at 20+ Sanity, gains 1 Coin Power(%d) for every 20 Sanity (%d)\n",
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
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->skills[0].active >= 10 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

    int clashpower = c->skills[0].active/10;
     if (clashpower > 2) clashpower = 2;

     c->ClashPower += clashpower;

      printf("\n%s gains +1 Clash Power for every 10 Burn Stack(%d) on self (%d - Max 2)\n", c->name, c->skills[0].active, clashpower);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 1 and 2 coin power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (c->skills[0].active + c->skills[1].active) >= 6 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

     c->CoinPowerBoost += 1;

      printf("\n%s at 6+ Burn (Stack(%d) + Count(%d)) on self, gains +1 Coin Power\n", c->name, c->skills[0].active, c->skills[1].active);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill def base power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (c->skills[0].active + c->skills[1].active) >= 10 && (chosenSkill == &c->defenseSkill[0])) {

     c->BasePowerBoost += 1;

      printf("\n%s at 10+ Burn (Stack(%d) + Count(%d)) on self, gains +1 Base Power\n", c->name, c->skills[0].active, c->skills[1].active);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill def coin power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->defenseSkill[0]) && c->skills[2].active > 0) {

      c->CoinPowerBoost += 1;

      printf("\n%s has Bloodflame [血炎], gains +1 Coin Power\n", c->name);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill def deal more damage on HP
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->defenseSkill[0])) {

    float damageboost = c->skills[6].active / 3;
    if (damageboost > 30.0f) damageboost = 30.0f;

    c->DamageUp += damageboost;

      printf("\n%s deals +1%% damage for every 3%% HP this unit cumulatively lost in this Encounter (%.0f%% - Max 30%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 and 4 coin power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (c->skills[0].active + c->skills[1].active) >= 6 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int Buff = (c->skills[0].active + c->skills[1].active)/6;
     if (Buff > 2) Buff = 2;

     c->CoinPowerBoost += Buff;

      printf("\n%s gains +1 Coin Power for every 6 Burn (Stack(%d) + Count(%d)) on self (%d - Max 2)\n", c->name, c->skills[0].active, c->skills[1].active, Buff);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 and 4 Clash power
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->HP <= c->MAX_HP * 0.8 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int clashpower = (c->MAX_HP - c->HP)/c->MAX_HP / 0.2;
     if (clashpower > 3) clashpower = 3;

     c->ClashPower += clashpower;

      printf("\n%s gains +1 Clash Power for every 20%% missing HP on self (%d - Max 3)\n", c->name, clashpower);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 deal more damage on burn
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[2]) && c->skills[0].active > 0) {

    float damageboost = c->skills[0].active * 1.5f;
    if (damageboost > 30.0f) damageboost = 30.0f;
    c->DamageUp += damageboost;

      printf("\n%s deals +1.5%% damage for every Burn Stack on self (%.1f%% - Max 30%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 4 deal more damage on burn
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[3]) && c->skills[0].active > 0) {

    int damageboost = c->skills[0].active * 3;
    if (damageboost > 30) damageboost = 30;
    c->DamageUp += damageboost;

      printf("\n%s deals +3%% damage for every Burn Stack on self (%d%% - Max 30%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 4 deal more damage on HP
  if (isId(c->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[3])) {

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
    float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

    float damageboost = (missingSelf + missingEnemy) / 2.0f;
    if (damageboost > 50.0f) damageboost = 50.0f;
    
    c->DamageUp += damageboost;

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
  if (isId(c->ID, "King in Binds") == 0 && c->skills[1].active >= 6 && (chosenSkill == &c->skills[0])) {

    c->CoinPowerBoost += 1;

      printf("\nIf target has 6+ Sinking Stack, Coin Power +1\n");

    sleep(1);
  }

  // King in Binds - Skill 2 or 4 Coin buff
  if (isId(c->ID, "King in Binds") == 0 && c->skills[1].active >= 6 && (chosenSkill == &c->skills[1] || chosenSkill == &c->skills[3])) {

    int gain = c->skills[1].active/6;
    if (gain > 2) gain = 2;

    c->CoinPowerBoost += gain;

      printf("\n%s gains +1 Coin Power (%d - Max 2) for every 6 Sinking Stack on target (%d)\n", c->name, gain, c->skills[1].active);

    sleep(1);
  }

  // King in Binds - Skill 3 Clash Power buff
  if (isId(c->ID, "King in Binds") == 0 && c->skills[3].active >= 1 && (chosenSkill == &c->skills[2])) {

    c->ClashPower += 1;

    printf("\nIf target has 1+ Tremor Stack, Clash Power +1\n");

    sleep(1);
  }

  // King in Binds - Skill 5 Clash Power buff
  if (isId(c->ID, "King in Binds") == 0 && c->skills[3].active >= 3 && (chosenSkill == &c->skills[4])) {

    int gain = c->skills[1].active/3;
    if (gain > 3) gain = 3;

    c->ClashPower += gain;

    printf("\n%s gains +1 Clash Power (%d - Max 3) for every 3 Tremor Stack on target (%d)\n", c->name, gain, c->skills[3].active);

    sleep(1);
  }

  // King in Binds - Skill 6 buff
  if (isId(c->ID, "King in Binds") == 0 && (chosenSkill == &c->skills[5])) {

    c->DefenseBoostNextTurn -= 5;

    c->FinalPowerBoostNextTurn += 2;

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

    c->DamageUp += c->skills[3].active;

    int Coingain = c->skills[3].active/5;
    if (Coingain > 5) Coingain = 5;

    if (Coingain > 0) {
      
    c->CoinPowerBoost += Coingain;

    printf("\n%s gains +1 Coin Power (%d - Max 5) for every 5 'Binding Vow - Open' consumed (%d)\n",
           c->name, Coingain, c->skills[3].active);

    sleep(1);
    }

    int Basegain = c->skills[3].active/10;
    if (Basegain > 3) Basegain = 3;

    if (Basegain > 0) {

    c->BasePowerBoost += Basegain;

    printf("\n%s gains +1 Base Power (%d - Max 3) for every 10 'Binding Vow - Open' consumed (%d)\n",
           c->name, Basegain, c->skills[3].active);

    sleep(1);
    }

    c->skills[3].active = 0;

  }

    // ------------------------------------------------------







  





  
  
  applySanityDebuff(tempOffense, tempDefense, c);



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

    p1->skills[0].active++;
    if (p1->skills[0].active > 99) p1->skills[0].active = 99;
    p1->skills[1].active++;
    if (p1->skills[1].active > 99) p1->skills[1].active = 99;

    printf("\n%s applies +1 Burn Stack(%d) and +1 Burn Count(%d) on self\n", p1->name, p1->skills[0].active, p1->skills[1].active);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff Skill 1 and 2 gain on clash
  if (isId(p1->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (s1 == &p1->defenseSkill[0])) {

    p1->skills[1].active += 3;
    if (p1->skills[1].active > 99) p1->skills[1].active = 99;

    printf("\n%s gains +3 Burn Count on self (%d)\n", p1->name, p1->skills[1].active);

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

    
    int cOff = counterUnit->OffenseBoost, cDef = counterUnit->DefenseBoost;

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

      int cOff = counterUnit->OffenseBoost, cDef = counterUnit->DefenseBoost;
      
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

      int cOff = counterUnit->OffenseBoost, cDef = counterUnit->DefenseBoost;

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

    printf("\n%s won the Clash, %s gains 2 Fanatic next turn\n", p1->name, p1->name);

     sleep(1);
  }

  // --------------------------------------------------------

  // Heishou Pack - You Branch Adept Heathcliff Skill 4 won
  if (isId(p1->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 &&
      s1 == &p1->skills[3] && enemyCoins <= 0) {

    p1->skills[0].active += 10;
    if (p1->skills[0].active > 99) p1->skills[0].active = 99;

    printf("\n%s won the Clash, %s gains 10 Burn Stack (%d)\n", p1->name, p1->name, p1->skills[0].active);

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

    p2->Protection -= 30;
    p2->DamageUp -= 80;
    updateSanity(p2, -10);

    printf("\n%s lost the Clash, loses 10 Sanity (%d), deals -80%% damage and takes +30%% damage\n",
      p2->name, p2->Sanity);

    sleep(1);
  }

  // Sukuna:King of Curse - skill 6 Clash lost
  if (isId(p2->ID, "Sukuna:King of Curse") == 0 && enemyCoins <= 0 && (s2 == &p2->skills[5] || (s2 == &p2->skills[3] && p2->skills[3].Unbreakable > 0))) {

    p2->Paralyze += 1;
    p2->FinalPowerBoost -= 5;

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
    p2->DamageUp -= 50;
    
    printf("\n%s lost the Clash, Deal -50%% damage and Defense -24 next turn for this Encounter\n", p2->name);

     sleep(1);
  }

  // Meursault:Blade Lineage Mentor Skill 3 won
  if (isId(p1->ID, "Meursault:Blade Lineage Mentor") == 0 &&
      s1 == &p1->skills[2] && enemyCoins <= 0) {
    p1->AttackPowerBoostNextTurn += 5;
    printf("\n%s won the Clash, gains 5 Attack Power Up next turn\n", p1->name);

     sleep(1);
  }

  // Meursault:Blade Lineage Mentor Skill 2 won
  if (isId(p1->ID, "Meursault:Blade Lineage Mentor") == 0 &&
      s1 == &p1->skills[1] && enemyCoins <= 0) {
    p1->FinalPowerBoostNextTurn += 1;
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
    p1->OffenseBoostNextTurn += clashCount/3;
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
    p2->Protection -= 20;
    printf("\n%s lost the Clash, loses 3 Black Silence(%d) and take 20%% more damage\n", p2->name, p2->Passive);

     sleep(1);
  }

  // lose Black Silence
  if (isId(p2->ID, "Fixer grade 9?") == 0 &&
      (s2 == &p2->skills[4]) && enemyCoins <= 0 && p2->Passive >= 3) {
    p2->Passive -= 3;
    if (p2->Passive < 0) p2->Passive = 0;
    p2->Protection += 50;
    printf("\n%s lost the Clash, consumes 3 Black Silence(%d) and take 50%% less damage\n", p2->name, p2->Passive);

     sleep(1);
  }

  // Lost Black Silence Furioso
  if (isId(p2->ID, "Fixer grade 9?") == 0 &&
      (s2 == &p2->skills[9]) && enemyCoins <= 0) {
    p2->Passive -= 10;
    if (p2->Passive < 0) p2->Passive = 0;
    p2->Protection -= 40;
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
    p2->Protection += 25;

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
    p2->DamageUp -= 50;
    p2->Protection -= 30;
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

    p2->FinalPowerBoost -= 4;
    p2->Paralyze += 1;

    printf("\n%s lost the Clash, gains 4 Final Power down and 1 Paralyze (Fix the Power of 1 Coins to 0 for one turn)\n", p2->name);

    sleep(1);
  } // Lei heng – Skill 6 lost
  else if (isId(p2->ID, "Lei heng") == 0 && s2 == &p2->skills[4] &&
      enemyCoins <= 0) {

    p2->FinalPowerBoost -= 2;
    p2->Paralyze += 6;

    printf("\n%s lost the Clash, gains 2 Final Power down and 6 Paralyze (Fix the Power of 6 Coins to 0 for one turn)\n", p2->name);

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
    p2->FinalPowerBoostNextTurn -= 1;

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

  p2->skills[2].active -= ReduceValue;
  if (p2->skills[2].active <= 0) { p2->skills[2].active = 0; p2->skills[1].active = 0; }

  printf("\n%s lost the Clash, reduce Sinking Count on target by %d (%d)\n", p2->name, ReduceValue, p2->skills[2].active);

   sleep(1);

  if (s2 == &p2->skills[3]) {

    p1->Sanity -= 10;

    printf("\n%s lost the Clash, %s loses 10 Sanity (%d)\n", p2->name, p1->name, p1->Sanity);

     sleep(1);
    
  }
}

// ------------------------------------------------------------




// ------------------------------------------------------------------------------------------------------------------------------------













  // ------------------------------ enemy win -----------------------------------------------------------------------------------------

  //---------------Sancho:The Second Kindred of Don Quixote ---------------------

  // Sancho:The Second Kindred of Don Quixote - Skill 7 won
  if (isId(p1->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    s1 == &p1->skills[6] && playerCoins <= 0 && p1->HP > 1) {

    printf("\n%s won the Clash, consumes 4%% of Max HP(%d) to gain 1 Hardblood(%d) and deal 25%% more damage (this damage does not lower the unit's HP below 1)\n",
      p1->name, (int)(p1->MAX_HP * 0.04), p1->Passive);

        p1->HP -= (int)(p1->MAX_HP * 0.04);
    if (p1->HP < 1) p1->HP = 1;

      p1->DamageUp += 25;
      p1->Passive += 1;

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Skill 8 and 9 won
  if (isId(p1->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    (s1 == &p1->skills[7] || s1 == &p1->skills[8]) && playerCoins <= 0) {

    printf("\n%s won the Clash, deals 20%% more damage and more 20%% damage for every 10 Hardblood(%d)\n",
      p1->name, p1->Passive);

      p1->DamageUp += 20 * ((p2->Passive / 10) + 1);

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
      p1->DamageUp += 10;
    if (p1->Passive < 1) p1->Passive = 1;
    printf("\n%s won the Clash, deal +10%% damage\n", p1->name);

     sleep(1);
  }




  // Sukuna:King of Curse - Passive Clash win
  if (isId(p1->ID, "Sukuna:King of Curse") == 0 && playerCoins <= 0) {

      p1->AttackPowerBoostNextTurn += 5;

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

    p1->DamageUpNextTurn += 10;
    p1->FinalPowerBoostNextTurn += 1;

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
    p1->DamageUp += 50;
    if (p1->Passive < 0) p1->Passive = 0;
    printf("\n%s won the Clash, consumes 3 Black Silence(%d) to deal 50%% more damage\n", p1->name, p1->Passive);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && p1->Passive >= 5 && playerCoins <= 0 && s1 == &p1->skills[3]) {
    p1->Passive -= 5;
    if (p1->Passive < 0) p1->Passive = 0;
    p2->ParalyzeNextTurn += 2;
    printf("\n%s won the Clash, consumes 5 Black Silence(%d) to inflicts 2 Paralyze next turn (Fix the Power of 2 Coins to 0 for one turn)\n", p1->name, p1->Passive);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && p1->Passive >= 2 && playerCoins <= 0 && s1 == &p1->skills[5]) {
    p1->Passive -= 3;
    if (p1->Passive < 0) p1->Passive = 0;
    p2->Protection -= 20;
    printf("\n%s won the Clash, consumes 3 Black Silence(%d), %s take 20%% more damage next turn\n", p1->name, p1->Passive, p2->name);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && p1->Passive >= 2 && playerCoins <= 0 && (s1 == &p1->skills[0] || s1 == &p1->skills[4])) {
    p1->Passive -= 2;
    if (p1->Passive < 0) p1->Passive = 0;
    p1->DamageUpNextTurn += 15;
    printf("\n%s won the Clash, consumes 2 Black Silence(%d) to deal 15%% more damage next turn\n", p1->name, p1->Passive);

     sleep(1);
  }

  if (isId(p1->ID, "Fixer grade 9?") == 0 && playerCoins <= 0 && (s1 == &p1->skills[8])) {
    p1->Passive += 3;
    if (p1->Passive > 60) p1->Passive = 60;
    p1->ClashPowerNextTurn += 1;
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

  p1->skills[2].active += InfilctValue;
  if (p1->skills[2].active > 99) p1->skills[2].active = 99;

  printf("\n%s won the Clash, infilct +%d Sinking Count (%d)\n", p1->name, InfilctValue, p1->skills[2].active);

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

  if (s1->Unbreakable > 0) {
    printf("\nPlayer: '%s' | BasePower %d | CoinPower %d | Coins %d | Offense %d | "
           "Defense %d | Unbreakable %d\n",
           s1->name, s1->BasePower + p1->BasePowerBoost,
           s1->CoinPower + p1->CoinPowerBoost, s1->Coins, playerTempOffense,
           playerTempDefense, s1->Unbreakable);
  } else {
    printf("\nPlayer: '%s' | BasePower %d | CoinPower %d | Coins %d | Offense %d | "
           "Defense %d | Breakable\n",
           s1->name, s1->BasePower + p1->BasePowerBoost,
           s1->CoinPower + p1->CoinPowerBoost, s1->Coins, playerTempOffense,
           playerTempDefense);
  }
  if (s2->Unbreakable > 0) {
    printf("Enemy: '%s' | BasePower %d | CoinPower %d | Coins %d | Offense %d | "
           "Defense %d | Unbreakable %d\n",
           s2->name, s2->BasePower + p2->BasePowerBoost,
           s2->CoinPower + p2->CoinPowerBoost, s2->Coins, enemyTempOffense,
           enemyTempDefense, s2->Unbreakable);
  } else {
    printf("Enemy: '%s' | BasePower %d | CoinPower %d | Coins %d | Offense %d | "
           "Defense %d | Breakable\n",
           s2->name, s2->BasePower + p2->BasePowerBoost,
           s2->CoinPower + p2->CoinPowerBoost, s2->Coins, enemyTempOffense,
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

    playerTotal = s1->BasePower + p1->BasePowerBoost;
    enemyTotal = s2->BasePower + p2->BasePowerBoost;

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
        if (p1->Paralyze > 0) { // ← Character's paralyze
          playerTotal += 0;
          p1->Paralyze--; // ← Character's paralyze
        } else {
          playerTotal += PlayerCoinBuff + s1->CoinPower + p1->CoinPowerBoost;
          if (playerTotal <= 0) playerTotal = 0;
        }
      }
      if (i < enemyCoins && tossCoinWithSanity(p2)) { // Pass character
        // Check paralyze
        if (p2->Paralyze > 0) { // ← Character's paralyze
          enemyTotal += 0;
          p2->Paralyze--; // ← Character's paralyze
        } else {
          enemyTotal += EnemyCoinBuff + s2->CoinPower + p2->CoinPowerBoost;
          if (enemyTotal <= 0) enemyTotal = 0;
        }
      }

      // Meursault:Blade Lineage Mentor's Passive
      if (!(strcmp(s1->name, "Yield My Flesh") == 0)) {

      // Last coin offense bonus
      int bonus = 0;
        
      // Check if this is player's last coin
      if (i == playerCoins - 1 &&
          (playerTempOffense > enemyTempOffense || p1->ClashPower != 0 ||
           p1->FinalPowerBoost != 0)) {

        // Calculate player's clash bonus
        if (s1->skillType == 4) { // ถ้าเป็น Clashable Guard
          // ทุกๆ 3 Defense Level ที่สูงกว่า Offense ศัตรู ได้ +1 Power
          int defenseDiff = playerTempDefense - enemyTempOffense;
          if (defenseDiff > 0) bonus += (defenseDiff / 3);

          // บวกโบนัสจาก DefensePowerBoost และ ClashPower/FinalPower ตามปกติ
          bonus += p1->DefensePowerBoost + p1->ClashPower + p1->FinalPowerBoost;
        } else { // ถ้าเป็น Attack ปกติ (โค้ดเดิมของคุณ)
          int offenseBonus = ((playerTempOffense - enemyTempOffense) / 3);
          if (offenseBonus < 0) offenseBonus = 0;
          bonus = p1->ClashPower + p1->FinalPowerBoost + offenseBonus;
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
      if (i == enemyCoins - 1 &&
               (enemyTempOffense > playerTempOffense || p2->ClashPower != 0 ||
                p2->FinalPowerBoost != 0 || kingPassiveP2 > 0)) {

        // Calculate enemy's clash bonus
        if (s2->skillType == 4) { // ถ้าเป็น Clashable Guard
          int defenseDiff = enemyTempDefense - playerTempOffense;
          if (defenseDiff > 0) bonus += (defenseDiff / 3);
          bonus += p2->DefensePowerBoost + p2->ClashPower + p2->FinalPowerBoost;
        } else { // ถ้าเป็น Attack ปกติ (โค้ดเดิมของคุณ)
          int offenseBonus = ((enemyTempOffense - playerTempOffense) / 3);
          if (offenseBonus < 0) offenseBonus = 0;
          bonus = p2->ClashPower + p2->FinalPowerBoost + offenseBonus + kingPassiveP2;
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

    // Bleed p1 // 0 Stack 1 Count
    if (p1->Bleed[0] > 0 || p1->Bleed[1] > 0) {

      int damage = p1->Bleed[0] > 0 ? p1->Bleed[0] : 1;

      applyDamage(p1, damage, 0);

        p1->Bleed[1]--;

      if (p1->Bleed[1] <= 0) p1->Bleed[1] = 0;

      printf("\n%s takes %d Bleed damage (Count %d)\n", p1->name, damage, p1->Bleed[1]);

      if (p1->Bleed[1] <= 0) p1->Bleed[0] = 0;

    }

    // Bleed p2 // 0 Stack 1 Count
    if (p2->Bleed[0] > 0 || p2->Bleed[1] > 0) {

      int damage = p2->Bleed[0] > 0 ? p2->Bleed[0] : 1;

      applyDamage(p2, damage, 0);

          p2->Bleed[1]--;

      if (p2->Bleed[1] <= 0) p2->Bleed[1] = 0;

      printf("\n%s takes %d Bleed damage (Count %d)\n", p2->name, damage, p2->Bleed[1]);

      if (p2->Bleed[1] <= 0) p2->Bleed[0] = 0;

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
        p1->Tremor[2] += result.enemyFinalPower;
        printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                p2->name, p2->name, p1->name, result.enemyFinalPower);
        sleep(1);
        if (p1->Tremor[2] > 50 && p1->Stagger <= 0) {
              p1->Stagger += 2;
          printf("\n%s Staggered for one turn\n", p1->name);
          sleep(1);
              p1->Tremor[2] = 0;
        }
       }

        usleep(500000);
        if (p1->HP > 0 && s1->skillType == 0) {
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
      p2->Tremor[2] += result.playerFinalPower;
      printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
              p1->name, p1->name, p2->name, result.playerFinalPower);
      sleep(1);
      if (p2->Tremor[2] > 50 && p2->Stagger <= 0) {
          p2->Stagger += 2;
        printf("\n%s Staggered for one turn\n", p2->name);
        sleep(1);
          p2->Tremor[2] = 0;
      }

     }
    
    usleep(500000);
    if (p2->HP > 0 && s2->skillType == 0) {
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
      Winner->Tremor[2] += result.playerFinalPower;
      printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
        Winner->name, Winner->name, Loser->name, result.playerFinalPower);
        sleep(1);
      if (Loser->Tremor[2] > 50 && p1->Stagger <= 0) {
            Loser->Stagger += 2;
        printf("\n%s Staggered for one turn\n", p2->name);
        sleep(1);
          Loser->Tremor[2] = 0;
      }

      // เมื่อ Guard ชนะ จะไม่เกิดการ attackPhase ปกติ (เพราะเป็นสกิลป้องกัน)
  }
      
    usleep(500000);
    if (Loser->HP > 0) {

      // ----------- Clear yield my flesh Remembrance for claim their bones ----------------
        Loser->DamageUp = 0;
        Loser->DmgMutiplierBoost = 0;
        Loser->OffenseBoost = 0;
        Loser->DefenseBoost = 0;
      
      printf("\nYield My Flesh.....\n");
      
      if ((isId(Loser->ID, "Meursault:Blade Lineage Mentor") == 0 &&
         Loser->HP <= Loser->MAX_HP * 0.6)) {


        printf("\n%s HP at 60%% or less HP, Apply 'Remembrance' buff on self, Gains buff at 10+ Sanity or 30+ Sanity further from 0\n",Loser->name);

        int PowerBuff = 0;
        int DamageBuff = 0;

        if (abs(Loser->Sanity) >= 30) {

          PowerBuff = (3/Loser->defenseSkill[1].Coins) < 1 ? 1 : (3/Loser->defenseSkill[1].Coins);
          DamageBuff = (100/Loser->defenseSkill[1].Coins);
          
          Loser->CoinPowerBoost += PowerBuff;
          Loser->DamageUp += DamageBuff;

          printf("At 30+ Sanity(%d) further from 0, gain %d Coin Power and gain %d%% more damage\n", Loser->Sanity, PowerBuff, DamageBuff);

          sleep(1);
        }
      else if (abs(Loser->Sanity) >= 10) {

        PowerBuff = (3/Loser->defenseSkill[1].Coins) < 1 ? 1 : (3/Loser->defenseSkill[1].Coins);
        DamageBuff = (50/Loser->defenseSkill[1].Coins);

        Loser->CoinPowerBoost += PowerBuff;
        Loser->DamageUp += DamageBuff;

        printf("At 10+ Sanity(%d) further from 0, gain %d Coin Power and gain %d%% more damage\n", Loser->Sanity, PowerBuff, DamageBuff);

        sleep(1);
      } 

        printf("\n%s: \"If you will cut... then wager your life on it.\"\n", Loser->name);

        sleep(1);

      }
      
      usleep(500000);
      
      attackPhase(Loser, &Loser->defenseSkill[1], Loser->defenseSkill[1].Offense, Loser->defenseSkill[1].Defense,
        Winner, WinnerSkill, winOff, winDef, Loser->defenseSkill[1].Coins, 0, 0);
      
      fullPlayer->defenseSkill[1].active = 0;

       Winner->ParalyzeNextTurn += 5;

      printf("\nTo Claim Their Bones!\n");

      printf("\n%s inflicts 5 Paralyze next turn (Fix the Power of 5 Coins to 0 for one "
             "turn) to %s\n",
             Loser->name, Winner->name);

      updateSanity(Loser, 15);
      if (Loser->Sanity > 45) Loser->Sanity = 45;

      printf("\n%s heals 15 Sanity (%d)\n",
        Loser->name, Loser->Sanity);
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

    // 0=Atk, 1=Guard, 2=Evade, 3=Counter, 4=ClashableGuard, 5=ClashableCounter
    player->defenseSkill[0] = (SkillStats){"Overthrow", 8, 10, 1, 5, 3, 1, 0, 0, 0, 0, 3};
    player->defenseSkill[1] =
      (SkillStats){"To Claim Their Bones", 4, 4, 4, 5, 30, 2, 1, 0, 0, 0, 3};

    player->numDefenseSkills = 2; // <-- important
    
    player->numSkills = 4; // <-- important
  } else if (pIndex == 2) {
    player->name = "Heathcliff:Wild Hunt";
    player->HP = 112;
    player->MAX_HP = 112;
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
    enemy->HP = 150;
    enemy->MAX_HP = 150;
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
    enemy->HP = 847;
    enemy->MAX_HP = 847;
      enemy->MinSpeed = 2;
      enemy->MaxSpeed = 4;
    enemy->immuneToPanicSkip = 1;
    enemy->sanityGainBase = 8;
    enemy->sanityLossBase = 7;
    enemy->skills[0] = (SkillStats){"Double Slash", 4, 2, 2, 2, 3, 1, 0, 0, 4, 1};
    enemy->skills[1] = (SkillStats){"Triple Slash", 3, 2, 3, 3, 3, 1, 0, 0, 2, 1};
    enemy->skills[2] =
        (SkillStats){"Tanglecleaver [快刀亂麻]", 8, 12, 1, 6, 3, 1.25, 0, 1, 0, 1};
    enemy->skills[3] = (SkillStats){
        "Blasting Shatterslash [爆碎斬]", 4, 3, 3, 4, 3, 1, 0, 3, 0, 1};
    enemy->skills[4] = (SkillStats){
      "Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]", 3, 3, 6, 6, 3, 1, 0, 6, 0, 1};
    
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
    enemy->HP = 521;
    enemy->MAX_HP = 521;
    enemy->MinSpeed = 4;
    enemy->MaxSpeed = 8;
    enemy->sanityGainBase = 10;
    enemy->sanityLossBase = 7;
    enemy->immuneToPanicSkip = 1;
    enemy->skills[0] = (SkillStats){
        "Cursed Technique - Dismantle", 6, 2, 3, 3, 10, 1, 1, 0, 4, 1}; // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[1] = (SkillStats){"Cursed Technique - Cleave", 6, 3, 2, 3, 10, 1, 1, 0, 4, 1};
    enemy->skills[2] = (SkillStats){"Blitz speed", 4, 2, 5, 4, 10, 1, 1, 1, 3, 1};
    enemy->skills[3] = (SkillStats){"Cursed Technique - Fuga:Open [鐚]", 7, 15, 1, 6, 10, 1, 0, 0, 2, 1};
    enemy->skills[4] =
        (SkillStats){"Chanting", 0, 0, 1, 0, 10, 0, 0, 0, 0, 0};
    enemy->skills[5] = (SkillStats){"Black Flash", 10, 5, 1, 5, 10, 1, 1, 1, 3, 1};
    enemy->skills[6] = (SkillStats){"Know your place...", 2, 1, 5, 1, 10, 1, 1, 1, 2, 0};
    enemy->skills[7] =
      (SkillStats){"Cursed Technique - World Cutting Slash", 20, 15, 1, 7, 10, 1.5, 0, 1, 0, 1};
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
    enemy->defenseSkill[0] = (SkillStats){"Evade", 4, 10, 1, 0, 2, 1, 1, 0, 2, 0, 2};
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
  } else {
    enemy->name = "Fixer grade 9?";
    enemy->HP = 1897;
    enemy->MAX_HP = 1897;
    enemy->MinSpeed = 5;
    enemy->MaxSpeed = 10;
    enemy->sanityGainBase = -7;
    enemy->sanityLossBase = -5;
    enemy->immuneToPanicSkip = 1;
    enemy->SanityFreezeTurns = -1;
    enemy->skills[0] =
        (SkillStats){"Allas Workshop", 12, -4, 2, 3, -5, 1, 1, 0, 5, 1};
    enemy->skills[1] =
        (SkillStats){"Wheels Industry", 15, -10, 1, 5, -5, 1, 1, 0, 2, 1};
    enemy->skills[2] =
        (SkillStats){"Crystal Atelier", 16, -6, 2, 3, -5, 1, 1, 0, 1, 1};
    enemy->skills[3] =
        (SkillStats){"Zelkova Workshop", 11, -4, 2, 2, -5, 1, 1, 0, 3, 1};
    enemy->skills[4] =
        (SkillStats){"Old Boys Workshop", 10, -5, 1, 2, -5, 1, 1, 0, 5, 1};
    enemy->skills[5] =
        (SkillStats){"Mook Workshop", 10, -7, 1, 2, -5, 1, 1, 0, 5, 1};
    enemy->skills[6] =
        (SkillStats){"Ranga Workshop", 11, -3, 3, 2, -5, 1, 0, 0, 3, 1};
    enemy->skills[7] =
        (SkillStats){"Atelier Logic", 15, -4, 3, 3, -5, 1, 1, 0, 1, 1};
    enemy->skills[8] = (SkillStats){"Durandal", 15, -6, 2, 4, -5, 1, 1, 0, 2, 1};
    enemy->skills[9] =
        (SkillStats){"Furioso", 25, -3, 15, 6, -5, 1, 0, 15, 0, 1};
    enemy->numSkills = 10; // <-- important
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
void handleTurnStart(Character *player, Character *enemy, int *enemySkillIndex, int *playerSkill1, int *playerSkill2, int *enemySkill1, int *enemySkill2) {

  //------------------- Turn Start ----------------------------

  // --------------- Muga Ryōshū ---------------

  // Muga Ryōshū – gains Speed
  if (isId(player->ID, "Muga Ryōshū") == 0 && player->skills[0].active/3 > 0) {

      applyDamage(enemy, player->skills[0].active/3, 0);

      printf("\n%s takes %d fixed damage\n", enemy->name, player->skills[0].active/3);

      sleep(1);

    enemy->Bleed[0] += 3;
    if (enemy->Bleed[0] > 99) enemy->Bleed[0] = 99; 
    enemy->Bleed[1] += 1;
    if (enemy->Bleed[1] > 99) enemy->Bleed[1] = 99; 

    printf("\n%s gains +3 Bleed Stack (%d) and +1 Bleed Count (%d)\n", enemy->name, enemy->Bleed[0], enemy->Bleed[1]);

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
      player->Passive += TurnCount;
      if (player->Passive > 100) player->Passive = 100;
    printf("\n%s gains +%d Muga [無我] (%d - Max 100)\n", player->name, TurnCount, player->Passive);

    sleep(1);

      // 2. Severed and Torn: Inflict Sever the Thread more อัตโนมัติทุกต้นเทิร์น
      // สูตร: 1 (พื้นฐาน) + (Muga/10; Max 4)
    player->skills[10].active = 0;

      int moreInflict = 2 + (player->Passive / 10);
      if (moreInflict > 6) moreInflict = 6; 
      player->skills[10].active += moreInflict; // ค่าสูงสุด Inflict บนตัวศัตรู

      // 3. Offense Level จากการโดนตีในตาที่แล้ว (สะสมจาก applyDamage)
      if (player->skills[11].active > 0) {
          player->OffenseBoost += player->skills[11].active * 3;
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

      *enemySkillIndex = 9;

      if (enemy->Stagger > 0) {
          enemy->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            enemy->name);

        sleep(1);
        }

      sleep(1);
    } else {
        // Normal random selection
        *enemySkillIndex = (rand() % 2 == 0 ? *enemySkill1 : *enemySkill2);
    }

  } 

  // ------------------ The House of Spiders: The Index Nursefather Yi Sang ------------------

  // The House of Spiders: The Index Nursefather Yi Sang gains Wound-casing Mask at start
  if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 && player->skills[3].active == 0) {

    player->skills[3].active = 1;
    
    printf("\n%s gains 'Wound-casing Mask'\n", player->name);

    sleep(1);

  }


  // The House of Spiders: The Index Nursefather Yi Sang - Index Target
  if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0) {
      // แปะสถานะ Target ไว้ที่ตัวศัตรู
        player->skills[11].active = 1; // Index Target
    enemy->Protection -= 10;
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
        player->DefenseBoost -= penalty;
        printf("\n%s's Karmic Consequence (%d): Defense Level -%d\n", player->name, player->skills[2].active, penalty);

      sleep(1);
    }
    if (player->skills[2].active >= 20) {
        int penalty = player->skills[2].active / 20;
        player->Protection -= penalty * 10;
        printf("%s's Karmic Consequence (%d): Take +%d damage\n", player->name, player->skills[2].active, penalty * 10);

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
      enemy->ClashPower += 2;

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

    *enemySkillIndex = 8;

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

    int healHPpercentag = player->skills[0].active + 20;
    if (healHPpercentag > 49) healHPpercentag = 49;
    int healvalue = (player->MAX_HP * healHPpercentag/100);

    player->HP += (player->HP + healvalue) > player->MAX_HP ? player->MAX_HP - player->HP : healvalue;

    printf("\n%s heals %d%% HP (%.2f), and remove all Burn on self (Max 49%%; Once per Encounter)\n",
      player->name, healHPpercentag, player->HP);

      player->skills[0].active = 0;
      player->skills[1].active = 0;

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

    player->Passive = amount;

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
    player->ClashPower = clashpowerbuff;
    player->ClashPowerNextTurn = clashpowerbuff;


      printf("\n%s exits the Volatile E.G.O::Waxen Pinion state and loses all 'Volatile Passion' to gain +(Volatile Passion x 3) (%d - Max 20) Clash Power for this turn and next turn\n", player->name, clashpowerbuff);

      sleep(1);

    printf("\n%s: \"... I have to be bold!\"\n", player->name);

    sleep(1);


    }

  // Dawn Office Fixer Sinclair - Volatile Passion
  if (isId(player->ID, "Dawn Office Fixer Sinclair") == 0 && player->skills[3].active) {

      player->Passive += 1;

    player->FinalPowerBoost += 1 * player->Passive;
      player->DamageUp += 20 * player->Passive;

    printf("\n%s gains 1 'Volatile Passion', gain 1 Final Power(%d), gain +20%% damage(%d%%) for every stack (%d)\n", player->name, 1 * player->Passive, 20 * player->Passive, player->Passive);

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

    enemy->DamageUp += 30;
      updateSanity(enemy, 30);

    printf("\n%s gains 'A Sliver of Anticipation', He's not giving it his all. Heal 30 Sanity, Lose 35 Offense, 35 Defense and Deal 30%% more damage\n",
           enemy->name);

    sleep(1);

    } else {
      enemy->DamageUp += 30;
    }
  }

  // Jia Qiu - Last attack at 85% HP
    if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP < enemy->MAX_HP * 0.85 && enemy->Passive == 2 && enemy->skills[5].active == 0 && !isStaggered(enemy)) {

      *enemySkillIndex = 5;

      enemy->skills[5].active = 1;
    }

  // -------------------------------------------------------------

  //--------------------- Lei heng -----------------------------

  // Lei heng – if HP ≤ 60%
  if (isId(enemy->ID, "Lei heng") == 0 && (enemy->HP <= enemy->MAX_HP * 0.6 || (enemy->skills[1].active >= 1 && enemy->skills[2].active >= 3)) &&
    enemy->skills[0].active == 2 && enemy->skills[4].active == 0) {

    enemy->skills[4].active = 1; // Active 'Unrelenting Spirit [剛氣]'

    if (enemy->Stagger > 0) {
      enemy->Stagger = 0;

      printf("\n%s recovers from 'Stagger'\n",
        enemy->name);

      sleep(1);
    }
    
    printf("\n%s activates 'Unrelenting Spirit [剛氣]'!\n", enemy->name);

    sleep(1);

    printf("\n%s: \"Can't leave a dance unfinished. Ain't that right?\"\n", enemy->name);

     sleep(1);
  }

  // Lei heng – if HP ≤ 40%
  if (isId(enemy->ID, "Lei heng") == 0 && (enemy->HP <= enemy->MAX_HP * 0.4 || (enemy->skills[1].active >= 2 && enemy->skills[2].active >= 3)) &&
    enemy->skills[0].active == 2) {

    enemy->skills[0].active = 3; // Phase 4

    printf("\n%s converts 'Inner Strength [底力]' to 'Extreme Strength [極力]'\n",
      enemy->name);

    sleep(1);

    printf("\n%s converts 'Unrelenting Spirit [剛氣]' to 'Unrelenting Spirit - Shin [剛氣-心]'\n",
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
    *enemySkillIndex = 2; 
    enemy->skills[1].active++; // Overheat count
    if (enemy->skills[1].active > 5) enemy->skills[1].active = 5;

  }

  // Lei heng – skill 3 every 3 turns
  else if (isId(enemy->ID, "Lei heng") == 0 && (enemy->skills[2].active > 0 && enemy->skills[2].active < 3) && enemy->skills[0].active == 2) {

    enemy->skills[2].active++; // Turn Count

  }
  else if (isId(enemy->ID, "Lei heng") == 0 && (enemy->skills[2].active >= 3) && enemy->skills[0].active == 2) {

    *enemySkillIndex = 2; 
    enemy->skills[1].active++; // Overheat count
    if (enemy->skills[1].active > 5) enemy->skills[1].active = 5;

  }

  // Lei heng – skill 6 using first fight
  if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[4].active == 0 && enemy->skills[0].active == 3) {

    enemy->skills[4].active = 1;
    *enemySkillIndex = 4; 
    enemy->skills[1].active++; // Overheat count
    if (enemy->skills[1].active > 5) enemy->skills[1].active = 5;

  }

  // Lei heng – skill 6 every 3 turns
    else if (isId(enemy->ID, "Lei heng") == 0 && (enemy->skills[4].active > 0 && enemy->skills[4].active < 3) && enemy->skills[0].active == 3) {

      enemy->skills[4].active++; // Turn Count

    }
    else if (isId(enemy->ID, "Lei heng") == 0 && (enemy->skills[4].active >= 3) && enemy->skills[0].active == 3) {

      *enemySkillIndex = 4; 
      enemy->skills[1].active++; // Overheat count
      if (enemy->skills[1].active > 5) enemy->skills[1].active = 5;

    }

  //-------------------------------------------------------------

  // ------------------------- Sancho --------------------------

  // Sancho – skill 13 using first fight
  if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    enemy->HP <= enemy->MAX_HP * 0.6 && enemy->skills[12].active && !isStaggered(enemy)) {

    enemy->skills[12].active = 0;
  *enemySkillIndex = 12;

  }
  // Sancho – skill 14 using first fight
  if (isId(enemy->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
    enemy->HP <= enemy->MAX_HP * 0.3 && enemy->skills[13].active && enemy->skills[12].active == 0 && !isStaggered(enemy)) {

    enemy->skills[13].active = 0;
  *enemySkillIndex = 13;

  sleep(1);
  }

  //-------------------------------------------------------------------------

  // ------------------------- King in Binds --------------------------

  // King in Binds – skill 6 using first fight
  if (isId(enemy->ID, "King in Binds") == 0 && enemy->skills[5].active == 0 && enemy->skills[0].active == 1) {

    enemy->skills[5].active = 1;
    *enemySkillIndex = 5; 

  }

  // King in Binds – skill 6 every 4 turns
    else if (isId(enemy->ID, "King in Binds") == 0 && (enemy->skills[5].active > 0 && enemy->skills[5].active < 4) && enemy->skills[0].active == 1) {

      enemy->skills[5].active++; // Turn Count

    }
    else if (isId(enemy->ID, "King in Binds") == 0 && (enemy->skills[5].active >= 4) && enemy->skills[0].active == 1 && !isStaggered(enemy)) {

      *enemySkillIndex = 5; 
      enemy->skills[5].active = 1;

    }

  //-------------------------------------------------------------------------

  // Jia Qiu enemy heal
  if (isId(player->ID, "Jia Qiu") == 0 && (player->skills[15].active > 0) && enemy->HP <= 0) {

    if (isId(enemy->ID, "Hong lu:The Lord of Hongyuan") == 0) {

      player->skills[15].active -= 1;

    enemy->HP = enemy->MAX_HP;
      enemy->FinalPowerBoostNextTurn += 1;
      printf("\n%s's Uncompromising Imposition activated! Heal up to max HP and gain 1 Final Power, lose 1 stack(%d)", enemy->name, player->skills[15].active);
      sleep(1);
  } else {

      player->skills[15].active -= 1;

      enemy->HP = enemy->MAX_HP;
      printf("\n%s's Dialogues activated! Heal up to max HP, lose 1 stack(%d)", enemy->name, player->skills[15].active);
      sleep(1);
  }

  }

}



















// ----------------------------------------------------------------
// handleBeforeFight — After showing player hp and sanity
// ----------------------------------------------------------------
void handleBeforeFight(Character *player, Character *enemy, int *enemySkillIndex, int playerSkill1, int playerSkill2, int enemySkill1, int enemySkill2) {

  if (!SpeedState) {
    SpeedState = 1;
  calculateSpeed(player);
  calculateSpeed(enemy);
  }

    // ------------------ Before fight -----------------------

  // The One Who Grips Faust - Purify ready
  if (isId(player->ID, "The One Who Grips Faust") == 0 && player->skills[2].active >= 3 && (&player->skills[playerSkill1] == &player->skills[2] || &player->skills[playerSkill2] == &player->skills[2])) {

          printf("\n%s: \"Higher... Still higher! Let me advance... toward a purer body... Huhu!\"\n", player->name);
    
  }

    // --------------------------- Sukuna:King of Curse --------------------------------


    // Sukuna:King of Curse Chanting
    if (isId(player->ID, "Sukuna:King of Curse") == 0 && player->skills[4].active < 3 && TurnCount % 3 == 0) {

       *enemySkillIndex = 4;

    }

    // Sukuna:King of Curse World Cutting Slash
    else if (isId(player->ID, "Sukuna:King of Curse") == 0 && player->skills[4].active == 3 && !isStaggered(enemy)) {

     *enemySkillIndex = 7;

      player->skills[4].active = 0;

      printf("\n%s: \"No more playing around, brat.\"\n",
        player->name);

      sleep(1);

    }

    // ---------------------------------------------------------------------


     // ------------------ Jia Qiu -----------------------

    // Jia Qiu - Last attack at 10% HP
      if (isId(player->ID, "Jia Qiu") == 0 && player->HP < player->MAX_HP * 0.2 && player->Passive == 6) {

        printf("\n%s: \"I expect you to fight to your deaths in this crucial struggle, reversible as they may be.\"\n",
          player->name);

          *enemySkillIndex = 15;

        player->Passive = 7;

      }

    // Jia Qiu - Taunt S14
    if (isId(player->ID, "Jia Qiu") == 0 && &player->skills[*enemySkillIndex] == &player->skills[14]) {

      printf("\n%s: \"It must lie there still, shrouded it may be.\"\n",
        player->name);

    }

    // Jia Qiu - Last attack at 30% HP
    if (isId(player->ID, "Jia Qiu") == 0 && player->HP < player->MAX_HP * 0.4 && player->Passive == 4) {

      printf("\n%s: \"Do not fear the futility. There will be time for that once you have spoken your mind.\"\n",
        player->name);

        *enemySkillIndex = 16;

      player->Passive = 5;

    }

    // Jia Qiu - Last attack at 85% HP
      if (isId(player->ID, "Jia Qiu") == 0 && player->HP < player->MAX_HP * 0.85 && player->Passive == 2 && player->skills[5].active == 0 && !isStaggered(enemy)) {

        *enemySkillIndex = 5;

        player->skills[5].active = 1;
      }

    // -------------------------------------------------------------

  // The House of Spiders: The Index Nursefather Yi Sang – Before fight with Furioso-Replica with Sizzling Wound
  if (isId(player->ID, "The House of Spiders: The Index Nursefather Yi Sang") == 0 &&
      (&player->skills[playerSkill1] == &player->skills[3] ||
       (&player->skills[playerSkill2] == &player->skills[3])) && player->skills[3].active == 2) {

    player->BasePowerBoost += 1;
    player->DamageUp += 30;
    player->ClashPower += 2;

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
      if (isId(player->ID, "Lei heng") == 0 &&
          (&player->skills[*enemySkillIndex] == &player->skills[2] ||
           (&player->skills[*enemySkillIndex] == &player->skills[4]))) {

        printf("\n%s: \"I'maboutta drop somethin' big on y'all! Don't let it kill "
               "y'all now and spoil the fun!\"\n",
               player->name);

        sleep(1);
      }

      // Sancho – Before fight
      if (isId(player->ID, "Sancho:The Second Kindred of Don Quixote") == 0 &&
          (&player->skills[*enemySkillIndex] == &player->skills[12] ||
           (&player->skills[*enemySkillIndex] == &player->skills[13]))) {

        if (isId(player->ID, "Don Quixote:The Manager of La Manchaland") == 0) {

          printf("\n%s: \"My name is Sancho!\"\n",
                 player->name);

          printf("\n%s: \"...\"\n",
             player->name);

          sleep(1);

          printf("\n%s: \"And I, Sancho, declare upon my honor; this lance shall end that festering, slothful dream!\"\n", player->name);

          printf("\n%s: \"You... Are like... Him... what's a juvenile dream!\"\n",
             player->name);

        } else {
          printf("\n%s: \"That's just attachment; my dream has already ended\"\n", player->name);
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

      printf("\n%s loses 1 Tremor Count (%d)\n", player->name, player->Tremor[1]);

      if (player->Tremor[1] <= 0) player->Tremor[0] = 0;

      sleep(1);
    }


      // Burn
        if ((player->Burn[0] > 0 || player->Burn[1] > 0) && player->HP > 0) {

          int damage = player->Burn[0] > 0 ? player->Burn[0] : 1;

          applyDamage(player, damage, 0);

          if (player->HP < 0) player->HP = 0;
          
            player->Burn[1]--; // Count Burn min 1
          if (player->Burn[1] <= 0) player->Burn[1] = 0;

          printf("\n%s takes %d damage from Burn (Count %d)\n", player->name, damage, player->Burn[1]);

          if (player->Burn[1] <= 0) player->Burn[0] = 0;

          sleep(1);
        }







    // Heathcliff:Wild Hunt - Impending Ruin
    if (player->skills[4].name != NULL && strcmp(player->skills[4].name, "Impending Ruin") == 0 && player->skills[4].active > 0) {
        player->skills[4].active--;
        printf("\n%s loses 1 Impending Ruin (%d)\n", enemy->name, player->skills[4].active);
    }

    // King in Binds - Tremor
    if (isId(enemy->ID, "King in Binds") == 0 && (enemy->skills[3].active > 0 || enemy->skills[4].active > 0) && player->HP > 0) {

        enemy->skills[4].active -= 1;
      if (enemy->skills[4].active <= 0) enemy->skills[4].active = 0;

      printf("\n%s loses 1 Tremor Count (%d)\n", player->name, enemy->skills[4].active);

      if (enemy->skills[4].active <= 0) enemy->skills[3].active = 0;

      sleep(1);
    }
      

      // Gregor:Firefist - Burn
        if (isId(player->ID, "Gregor:Firefist") == 0 && (player->skills[0].active > 0 || player->skills[1].active > 0) && enemy->HP > 0) {

          int damage = player->skills[0].active > 0 ? player->skills[0].active : 1;

          applyDamage(enemy, damage, 0);

          if (enemy->HP < 0) enemy->HP = 0;
          player->skills[1].active--; // Count Burn min 1
          if (player->skills[1].active <= 0) player->skills[1].active = 0;

          printf("\n%s takes %d damage from Burn (Count %d)\n", enemy->name, damage, player->skills[1].active);

          if (player->skills[1].active <= 0) player->skills[0].active = 0;

          sleep(1);
        }
      

    // ----------------------- Heishou Pack - You Branch Adept Heathcliff ----------------
    
    // Heishou Pack - You Branch Adept Heathcliff - Burn
    if (isId(player->ID, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (player->skills[1].active > 0 || player->skills[0].active > 0) && player->HP > 0) {

       int damage = player->skills[0].active > 0 ? player->skills[0].active : 1;
      
      applyDamage(player, damage, 0);
      
      if (player->HP < 1) player->HP = 1;
      player->skills[1].active--;
      if (player->skills[1].active <= 0) player->skills[1].active = 0;

      printf("\n%s takes %d damage from Burn (Count %d)\n", player->name, damage, player->skills[1].active);

      if (player->skills[1].active <= 0) player->skills[0].active = 0;

      sleep(1);

      int gain = 1;
      if (player->HP < player->MAX_HP * 0.5) gain += 1;

      player->Passive += gain;
      if (player->Passive > 20) player->Passive = 20;

       printf("\n%s gains +%d Battleblood Instinct (%d)\n", player->name, gain, player->Passive);
    }

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










    
    // Meursault:The Thumb Shin buffs (temporary, print once)
    if (isId(player->ID, "Meursault:The Thumb") == 0 && (playerSkillUsed == &player->defenseSkill[0] || player->Stagger > 0) && !player->skills[3].active) {

      if (player->Stagger > 0) {
          player->Stagger = 0;

        printf("\n%s recovers from 'Stagger'\n",
          player->name);

        sleep(1);
      }

      int amount = ((int)(8 * player->MAX_HP)) / 847;
      if (amount < 8) amount = 8;

      if (isId(player->ID, "Sancho:The Second Kindred of Don Quixote") == 0 || isId(player->ID, "Don Quixote") == 0) amount += 2; // pity for boss
      if (isId(player->ID, "Sukuna:King of Curse") == 0) amount += 3; // pity for boss

        player->skills[3].active = 1;

          player->Passive = amount;

        printf("\n%s spent all Tigermark Round, 'Unrelenting Spirit [剛氣]' activated and reload %d Savage Tigermark Round\n",
          player->name, amount);

        sleep(1);

        printf("\n%s: \"I see that you are worth the cost of my ammunition.\"\n", player->name);

      }

    // Sukuna:King of Curse Domain expansion
    if (isId(enemy->ID, "Sukuna:King of Curse") == 0 && enemy->skills[8].active > 0) {

      int dealvalue = player->MAX_HP*0.05;

      applyDamage(player, dealvalue, 0);

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

        player->DamageUpNextTurn += gain;
        
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

      if (player->skills[3].active == 2) {player->HP -= 4; printf("\n%s loses 4 HP due to Sizzling Wound\n", player->name);} // Sizzling Wound DOT

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

      player->skills[0].active = gain;

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
       (isId(player->ID, "Heathcliff:Wild Hunt") == 0 && playerSkillUsed == &player->defenseSkill[0] && player->skills[0].active > 0) {

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

    // Lei heng – skill 3 and skill 6 turn end
    if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[0].active == 3 && enemy->skills[3].active > 0) {

      enemy->skills[4].active = 1;

      printf("\n%s heals Sanity equal to Extreme Strength [底力] consumed(%d)\n", enemy->name, enemy->skills[3].active);
      updateSanity(enemy, enemy->skills[3].active);
        enemy->skills[3].active = 0;

    } else if (isId(enemy->ID, "Lei heng") == 0 && enemy->skills[0].active == 2 && enemy->skills[3].active > 0) {

      enemy->skills[2].active = 1;

      printf("\n%s heals Sanity equal to Inner Strength [底力] consumed(%d)\n", enemy->name, enemy->skills[3].active);
      updateSanity(enemy, enemy->skills[3].active);
      enemy->skills[3].active = 0;

    } 

    // Lei heng – HP < 90%
    if (isId(enemy->ID, "Lei heng") == 0 && (enemy->HP < enemy->MAX_HP * 0.9 || TurnCount >= 2) && enemy->skills[0].active == 0) {

      GainNewPattern(enemy, player);

      enemy->skills[0].active = 1; // Phase 2

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
    if (isId(enemy->ID, "Lei heng") == 0 && (enemy->HP < enemy->MAX_HP * 0.8 || TurnCount >= 4) && enemy->skills[0].active == 1) {

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

        enemy->DamageUpNextTurn += enemy->Passive * 2;

      printf("\n%s gains +2%% damage (%d%%) for every Black Silence (%d) next turn\n", enemy->name, enemy->Passive * 2, enemy->Passive);

      sleep(1);

      }

    // Roland - Ultimate
    if (isId(enemy->ID, "Fixer grade 9?") == 0 && enemy->skills[9].active > 0) {

      enemy->skills[9].active -= 1;
      enemy->ClashPowerNextTurn -= enemy->skills[9].active;

      printf("\n%s loses 1 Sorrow(%d)\n", enemy->name, enemy->skills[9].active);

      sleep(1);

    }

    // Roland - Ultimate
    if (isId(enemy->ID, "Fixer grade 9?") ==
            0 &&
      enemySkillUsed == &enemy->skills[9]) {

      printf("\n%s gains 3 Sorrow, gain 1 Clash Power Down for every 1 Stacks, lose 1 Stacks at Turn End\n", enemy->name);

      enemy->skills[9].active = 3;
      enemy->ClashPowerNextTurn -= enemy->skills[9].active;

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
      
      printf("\n%s use 'Reload', Spends %d Sanity (%d) to gain 20 The Living & The Departed and gain Shield equal to (Butterfly on the target x 2)%% of Max HP. (%d%% - Max 40%%) (%d - Shield %.2f)\n", player->name, Spend, player->Sanity, ShieldGain, Shield, player->Shield);

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
        enemy->FinalPowerBoostNextTurn += 1;
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
      if (isId(enemy->ID, "Don Quixote") == 0 && enemy->HP <= 0 &&
          enemy->Passive == 0) {

        printf("\n%s: \"If that's what you really yearn for...\"\n", enemy->name);

        sleep(1);

        printf("\n%s tranforms to 'Sancho:The Second Kindred of Don Quixote'\n",
               enemy->name);

        enemy->MAX_HP = 723;
        enemy->HP = 723;
        enemy->name = "Sancho:The Second Kindred of Don Quixote";
        enemy->Sanity = 0;
        enemy->sanityGainBase = 6;
        enemy->sanityLossBase = 4;
        enemy->immuneToPanicSkip = 1;
        enemy->Passive = 1;

        // Disable the old S0, S1, S2
        enemy->skills[0] = enemy->skills[3];
         enemy->skills[1] = enemy->skills[3];
         enemy->skills[2] = enemy->skills[3];
        enemy->skills[0].Copies = -1; // in pick skill function copies -1 will auto delete skill from lastused 
        enemy->skills[1].Copies = -1;
        enemy->skills[2].Copies = -1;
        enemy->defenseSkill[0].Copies = -1;

        // Set copies for the newly mapped primary skills
        enemy->skills[3].Copies = 4;
        enemy->skills[4].Copies = 4;
        enemy->skills[5].Copies = 4;

        enemy->skills[6].Copies = 3;
        enemy->skills[7].Copies = 3;
        enemy->skills[8].Copies = 3;
        enemy->skills[9].Copies = 3;
        enemy->skills[10].Copies = 3;
        enemy->skills[11].Copies = 3;

        sleep(1);

        clearDebuffsOnDeath(enemy, player);

        if (isId(player->ID,
                       "Don Quixote:The Manager of La Manchaland") == 0) {
          printf(
              "\n%s: I shall show you. Even if it’s a false dream... I will still "
              "move forward without hesitation!\n",
              enemy->name);
        } else {
          printf("\n%s: \"Let wrap it up.\"\n", enemy->name);
        }
      }

      // Sukuna:King of Curse cursed reverse technique
      if (isId(enemy->ID, "Sukuna:King of Curse") == 0 && enemy->HP <= 0 && enemy->Passive == 0) {

        enemy->Passive = 1;

        enemy->HP = enemy->MAX_HP;
        enemy->Sanity = 45;

        printf("\n%s used 'Cursed Reverse Technique', heal up to Max HP, heal Sanity to 45 (Once per Encounter)\n", enemy->name);

        sleep(1);

        printf("\n%s: \"Arm yourself...\"\n", enemy->name);

        sleep(1);
      }
      
      // Jia Qiu Anti low
      if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP <= enemy->MAX_HP * 0.4 &&
          enemy->Passive == 3) {

        enemy->Passive = 4;

        enemy->HP = (int)(enemy->MAX_HP * 0.4);

        printf("\n%s blocked, cap HP to 40%% for this turn\n", enemy->name);

        sleep(1);

        if (enemy->Stagger > 0) {
            enemy->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            enemy->name);

          sleep(1);
        }
      }

      // Jia Qiu LAST
      if (isId(enemy->ID, "Jia Qiu") == 0 && enemy->HP <= enemy->MAX_HP * 0.2 &&
          enemy->Passive == 5) {

        enemy->Passive = 6;

        enemy->HP = (int)(enemy->MAX_HP * 0.2);

        printf("\n%s blocked, cap HP to 20%% for this turn\n", enemy->name);

        sleep(1);

        if (enemy->Stagger > 0) {
            enemy->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            enemy->name);

          sleep(1);
        }
      }

      // Erlking Heathcliff Anti low
      if (isId(enemy->ID, "Erlking Heathcliff") == 0 && enemy->HP <= 50 &&
          enemy->Passive == 1) {

        enemy->Passive = 2;

        enemy->HP = 50;

        printf("\n%s used 'Withstand', cap HP to 50 for this turn\n", enemy->name);

        sleep(1);

        if (enemy->Stagger > 0) {
          enemy->Stagger = 0;

          printf("\n%s recovers from 'Stagger'\n",
            enemy->name);

          sleep(1);
        }

        sleep(1);

        printf("\n%s: \"Please, Catherine. Appear before me and tear me asunder. Let me see your eyes as I expire.\"\n", enemy->name);

        sleep(1);
      }

    // ----------------- King in Binds -----------------------

    // King in Binds Bandages of the King in Binds
    if (isId(enemy->ID, "King in Binds") == 0 && enemy->HP <= enemy->MAX_HP * 0.2 &&
        enemy->Passive == 1) {

      enemy->skills[1].active += 2; // Sinking Stack
      if (enemy->skills[1].active > 99) enemy->skills[1].active = 99;
      enemy->skills[2].active += 1; // Sinking Count
      if (enemy->skills[2].active > 99) enemy->skills[2].active = 99;

      printf("\n%s gain +2 Sinking Stack (%d) and +1 Sinking Count (%d) from 'Bandages of the King in Binds'\n", player->name, enemy->skills[1].active, enemy->skills[2].active);
    }

// King in Binds anti low
if (isId(enemy->ID, "King in Binds") == 0 && enemy->HP <= enemy->MAX_HP * 0.2 &&
    enemy->skills[0].active <= 0) {

  enemy->skills[0].active = 1;

  enemy->Passive = 1;

  enemy->HP = (int)(enemy->MAX_HP * 0.2);

  printf("\n%s snapping Bandages, cap HP to 20%% for this turn, apply 'Bandages of the King in Binds' on all enemies.\n", enemy->name);
}

    // --------------------------------------------------

    // ---------------------- Anti death effect ----------------------

    // Erlking Heathcliff Faded promise for wild hunt
    if (isId(enemy->ID, "Erlking Heathcliff") == 0 && isId(player->ID, "Heathcliff:Wild Hunt") == 0 && (enemy->skills[7].active == 1) && player->HP <= 0)  {

      enemy->skills[7].active--;

      player->HP = 1;

      printf("\n%s's 'Faded Promise' activated! In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn (Once per Encounter)\n", player->name);

      sleep(1);

    }

    // Hong lu:The Lord of Hongyuan - Passive
    if ((isId(player->ID, "Hong lu:The Lord of Hongyuan")) == 0 &&
             player->HP <= 0 && player->skills[5].active == 1) {

        player->skills[5].active--;

      player->HP = 1;

      printf("\n%s's '%s' activeted! Nullity all damage; then apply 'Lordsguard' to all left Heishou Pack and bring %s's HP to 1 (Once per Encounter)\n",
        player->name, player->skills[5].name, player->name);

      sleep(1);

      printf("\n%s: \"The Lord will not die.\"\n", player->name);

      sleep(1);

    }

      // Heishou Pack - You Branch Adept Heathcliff - Anti death Passive
      if ((isId(player->ID, "Heishou Pack - You Branch Adept Heathcliff")) == 0 &&
          player->HP <= 0 && player->skills[3].active == 0) {

          player->skills[3].active--;

        player->HP = 1;

        printf("\n%s's 'Flame Rooster's Death Defiance [炎鳥不死戦]' activated! Nullity all damage; then bring %s's HP to 1 (Once per Encounter)\n",
          player->name, player->name);

        sleep(1);

        printf("\n%s: \"Flame Rooster's Death Defiance [炎鳥不死戦]... Heh! You really thought I was gonna kick it... Huh?!\"\n",
          player->name);

      }

      // Meursault:Blade Lineage Mentor - Passive
      if (isId(player->ID, "Meursault:Blade Lineage Mentor") == 0 &&
          player->HP <= 0 && player->Passive == 0) {

          player->Passive--;

            player->HP = 1;

        printf("\n%s's 'Swordplay of the Homeland' activated! Nullity all damage; then bring %s's HP to 1 (Once per Encounter)\n",
          player->name, player->name);

        sleep(1);

      }

    // Erlking Heathcliff Faded promise for wild hunt
    if (isId(enemy->ID, "Erlking Heathcliff") == 0 && isId(player->ID, "Heathcliff:Wild Hunt") == 0 && (enemy->skills[7].active == 0))  {

          enemy->skills[7].active -= 1;

        player->HP = 1;

    }

    // Hong lu:The Lord of Hongyuan - Passive
    if ((isId(player->ID, "Hong lu:The Lord of Hongyuan")) == 0 && player->skills[5].active == 0) {

      player->skills[5].active--;

      player->HP = 1;

    }

      // Heishou Pack - You Branch Adept Heathcliff - Anti death Passive
      if ((isId(player->ID, "Heishou Pack - You Branch Adept Heathcliff")) == 0 && player->skills[3].active == -1) {

        player->skills[3].active--;

        player->HP = 1;

      }

      // Meursault:Blade Lineage Mentor - Passive
      if (isId(player->ID, "Meursault:Blade Lineage Mentor") == 0 && player->Passive == -1) {

        player->Passive--;

      }

    // Binah - phase 2
    if (isId(player->ID, "Binah") == 0 && !player->Passive && player->HP <= player->MAX_HP*0.5) {

      player->Passive = 1;

      printf("\n%s: \"Ara~... I really surprised that you pushed me this far; then let's get a bit 'Serious'.\"\n", player->name);

      sleep(1);

          player->HP = 1150;
           player->MAX_HP = 1150;
          player->DamageUpNextTurn += 100;
          player->FinalPowerBoostNextTurn += 5;

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
            player->FinalPowerBoostNextTurn += 2;
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
          player->FinalPowerBoostNextTurn += 3;
          player->DamageUpNextTurn += 30;

              printf("\n%s consumes 3 Hardblood (%d) to gain 3 Final Power Up and +30%% damage next turn\n", player->name, player->Passive);
          }
        else if (isId(player->ID, "Sukuna:King of Curse") == 0) {

              player->ClashPowerNextTurn += 3;

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

    // -------------------------------------------------------- Lost CutScene --------------------------------------------------------

    if (isId(player->ID, "Muga Ryōshū") != 0) {
    
    // Lost CutScene
    if (isId(enemy->ID, "Lei heng") == 0 && enemy->HP <= enemy->MAX_HP*0.2) {

      enemy->HP = 1;

      enemy->Stagger = 0;
      
      printf("\n%s: \"I'll be frank, y'all. Real impressed that you even pushed me this far.\"\n", enemy->name);

      sleep(2);

      attackPhase(enemy, &enemy->skills[0],
        enemy->skills[0].Offense, enemy->skills[0].Defense,
        player, &player->skills[0], player->skills[0].Offense,
         player->skills[0].Defense, enemy->skills[0].Coins, 0 , 0);

      enemy->HP = 1;

      sleep(1);

      if (strstr(player->name, "Ryoshu") != NULL) {
        printf("\n%s: \"But... Yoshihide\"\n", enemy->name);
      } else if (isId(player->ID, "Meursault:The Thumb") == 0) {
        printf("\n%s: \"But... Chacuihu\"\n", enemy->name);
      } else {
       printf("\n%s: \"But... %s...\"\n", enemy->name, player->name);
      }

      sleep(3);

      attackPhase(enemy, &enemy->skills[2],
        enemy->skills[2].Offense, enemy->skills[2].Defense,
        player, &player->skills[0], player->skills[0].Offense,
         player->skills[0].Defense, enemy->skills[2].Coins, 0 , 0);

      printf("\n%s: \"... Ya darn sure oughta've harder if ya really wanted to win!\"\n", enemy->name);

      enemy->HP = 1;

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
  SkillStats *enemySkillEffective = NULL; 

  /*
  player->MAX_HP = 3000;
  player->HP = 3000;
    enemy.MAX_HP = 3000;
    enemy.HP = 3000;
    */

  while (player->HP > 0 && enemy.HP > 0) {

    if (KingClashBonus > 0) {
            player->ClashPower += KingClashBonus;
    }

      printf("\n--- Turn %d (Knight Phase) ---\n", TurnCount);

    SkillStats *selectedEnemyPtr = NULL;

    int playerSkillIndex = -1;
    int enemySkillIndex = -1;

      // handleTurnStart ด้วย player จริงเป็น p1, knight เป็น enemy
    // 1. รัน Passive ของ Player (ตัวเรา)
    // ส่งสกิลของเรา (*playerSkill1...) เข้าช่องที่ 4-5 และสกิลของร่างเงา (kSkill1...) เข้าช่องที่ 6-7

    handleTurnStart(player, &enemy, &enemySkillIndex,
      playerSkill1, playerSkill2, &kSkill1, &kSkill2);

    // 2. รัน Passive ของ Knight (ร่างเงา)
    // สลับตำแหน่ง: ส่งร่างเงาเข้าช่องแรก, ส่งสกิลร่างเงาเข้าช่องที่ 4-5 และสกิลของเราเข้าช่องที่ 6-7
    handleTurnStart(&enemy, player, &playerSkillIndex,
      &kSkill1, &kSkill2, playerSkill1, playerSkill2);

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
    handleBeforeFight(player, &enemy, &enemySkillIndex,
      *playerSkill1, *playerSkill2, kSkill1, kSkill2);

    // 2. รัน Passive ของ Knight (ร่างเงา)
    // สลับตำแหน่ง: ส่งร่างเงาเข้าช่องแรก, ส่งสกิลร่างเงาเข้าช่องที่ 4-5 และสกิลของเราเข้าช่องที่ 6-7
      handleBeforeFight(&enemy, player, &playerSkillIndex,
      kSkill1, kSkill2, *playerSkill1, *playerSkill2);

    // -------------------------------------------------------
      // Knight เลือก skill — แสดง SKILL ของ TURN นี้ก่อน
      // (kSkill1/kSkill2 ที่ roll มาจาก turn ก่อน)
      // -------------------------------------------------------
    // สุ่ม: ร่างเงามีโอกาส 20% ที่จะใช้สกิลป้องกัน (เหมือน AI Limbus ทั่วไป)
    if (enemy.numDefenseSkills > 0 && (rand() % 100 < 20)) {
        selectedEnemyPtr = &enemy.defenseSkill[0];

        // ถ้าเป็น Guard (Type 1) ให้ทอยโล่ทันที
        if (selectedEnemyPtr->skillType == 1) {
            defensePhase(&enemy, selectedEnemyPtr);
        }
    } else {
      
    enemySkillIndex = (rand() % 2 == 0) ? kSkill1 : kSkill2;
    kLastUnused = (enemySkillIndex == kSkill1) ? kSkill2 : kSkill1;
       selectedEnemyPtr = &enemy.skills[enemySkillIndex];
      
    }

      // แสดง knight skill
      if (!IsenemyUnableToAct) {
          SkillStats *ks = selectedEnemyPtr;
          if (ks->Unbreakable > 0 && (ks->Clashable || ks->skillType != 0))
              printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(ks->skillType),
                     ks->name, ks->BasePower, ks->CoinPower, ks->Coins,
                     ks->Offense + enemy.OffenseBoost,
                     ks->Defense + enemy.DefenseBoost,
                     ks->Unbreakable);
          else if (ks->Unbreakable <= 0 && (ks->Clashable || ks->skillType != 0))
              printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(ks->skillType),
                     ks->name, ks->BasePower, ks->CoinPower, ks->Coins,
                     ks->Offense + enemy.OffenseBoost,
                     ks->Defense + enemy.DefenseBoost);
          else if (ks->Unbreakable > 0 && !ks->Clashable && ks->skillType == 0)
              printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(ks->skillType),
                     ks->name, ks->BasePower, ks->CoinPower, ks->Coins,
                     ks->Offense + enemy.OffenseBoost,
                     ks->Defense + enemy.DefenseBoost,
                     ks->Unbreakable);
          else if (ks->Unbreakable <= 0 && !ks->Clashable && ks->skillType == 0)
              printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(ks->skillType),
                     ks->name, ks->BasePower, ks->CoinPower, ks->Coins,
                     ks->Offense + enemy.OffenseBoost,
                     ks->Defense + enemy.DefenseBoost);
      }

      // -------------------------------------------------------
      // Player เลือก skill — แสดง skill ของ turn นี้
      // (playerSkill1/playerSkill2 ที่ roll มาจาก turn ก่อน)
      // -------------------------------------------------------

      if (!IsplayerUnableToAct) {
          printf("\nDashboard Skills:\n");

            if (player->skills[*playerSkill1].Unbreakable > 0 && player->skills[*playerSkill1].Clashable) {
                printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                      getSkillTypeName(player->skills[*playerSkill1].skillType),
                       player->skills[*playerSkill1].name,
                       player->skills[*playerSkill1].BasePower,
                       player->skills[*playerSkill1].CoinPower,
                       player->skills[*playerSkill1].Coins,
                       player->skills[*playerSkill1].Offense + player->OffenseBoost,
                       player->skills[*playerSkill1].Defense + player->DefenseBoost,
                       player->skills[*playerSkill1].Unbreakable);
            } else if (player->skills[*playerSkill1].Unbreakable <= 0 && player->skills[*playerSkill1].Clashable) {
                printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill1].skillType),
                       player->skills[*playerSkill1].name,
                       player->skills[*playerSkill1].BasePower,
                       player->skills[*playerSkill1].CoinPower,
                       player->skills[*playerSkill1].Coins,
                       player->skills[*playerSkill1].Offense + player->OffenseBoost,
                       player->skills[*playerSkill1].Defense + player->DefenseBoost);
            } else if (player->skills[*playerSkill1].Unbreakable > 0 && !player->skills[*playerSkill1].Clashable) {
                printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill1].skillType),
                       player->skills[*playerSkill1].name,
                       player->skills[*playerSkill1].BasePower,
                       player->skills[*playerSkill1].CoinPower,
                       player->skills[*playerSkill1].Coins,
                       player->skills[*playerSkill1].Offense + player->OffenseBoost,
                       player->skills[*playerSkill1].Defense + player->DefenseBoost,
                       player->skills[*playerSkill1].Unbreakable);
            } else {
                printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill1].skillType),
                       player->skills[*playerSkill1].name,
                       player->skills[*playerSkill1].BasePower,
                       player->skills[*playerSkill1].CoinPower,
                       player->skills[*playerSkill1].Coins,
                       player->skills[*playerSkill1].Offense + player->OffenseBoost,
                       player->skills[*playerSkill1].Defense + player->DefenseBoost);
            }

            if (player->skills[*playerSkill2].Unbreakable > 0 && player->skills[*playerSkill2].Clashable) {
                printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill2].skillType),
                       player->skills[*playerSkill2].name,
                       player->skills[*playerSkill2].BasePower,
                       player->skills[*playerSkill2].CoinPower,
                       player->skills[*playerSkill2].Coins,
                       player->skills[*playerSkill2].Offense + player->OffenseBoost,
                       player->skills[*playerSkill2].Defense + player->DefenseBoost,
                       player->skills[*playerSkill2].Unbreakable);
            } else if (player->skills[*playerSkill2].Unbreakable <= 0 && player->skills[*playerSkill2].Clashable) {
                printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill2].skillType),
                       player->skills[*playerSkill2].name,
                       player->skills[*playerSkill2].BasePower,
                       player->skills[*playerSkill2].CoinPower,
                       player->skills[*playerSkill2].Coins,
                       player->skills[*playerSkill2].Offense + player->OffenseBoost,
                       player->skills[*playerSkill2].Defense + player->DefenseBoost);
            } else if (player->skills[*playerSkill2].Unbreakable > 0 && !player->skills[*playerSkill2].Clashable) {
                printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill2].skillType),
                       player->skills[*playerSkill2].name,
                       player->skills[*playerSkill2].BasePower,
                       player->skills[*playerSkill2].CoinPower,
                       player->skills[*playerSkill2].Coins,
                       player->skills[*playerSkill2].Offense + player->OffenseBoost,
                       player->skills[*playerSkill2].Defense + player->DefenseBoost,
                       player->skills[*playerSkill2].Unbreakable);
            } else {
                printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill2].skillType),
                       player->skills[*playerSkill2].name,
                       player->skills[*playerSkill2].BasePower,
                       player->skills[*playerSkill2].CoinPower,
                       player->skills[*playerSkill2].Coins,
                       player->skills[*playerSkill2].Offense + player->OffenseBoost,
                       player->skills[*playerSkill2].Defense + player->DefenseBoost);
            }

          // Next Skill
            if (player->skills[*playerSkill3].Unbreakable > 0 && player->skills[*playerSkill3].Clashable) {
                printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill3].skillType),
                       player->skills[*playerSkill3].name,
                       player->skills[*playerSkill3].BasePower,
                       player->skills[*playerSkill3].CoinPower,
                       player->skills[*playerSkill3].Coins,
                       player->skills[*playerSkill3].Offense + player->OffenseBoost,
                       player->skills[*playerSkill3].Defense + player->DefenseBoost,
                       player->skills[*playerSkill3].Unbreakable);
            } else if (player->skills[*playerSkill3].Unbreakable <= 0 && player->skills[*playerSkill3].Clashable) {
                printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill3].skillType),
                       player->skills[*playerSkill3].name,
                       player->skills[*playerSkill3].BasePower,
                       player->skills[*playerSkill3].CoinPower,
                       player->skills[*playerSkill3].Coins,
                       player->skills[*playerSkill3].Offense + player->OffenseBoost,
                       player->skills[*playerSkill3].Defense + player->DefenseBoost);
            } else if (player->skills[*playerSkill3].Unbreakable > 0 && !player->skills[*playerSkill3].Clashable) {
                printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player->skills[*playerSkill3].skillType),
                       player->skills[*playerSkill3].name,
                       player->skills[*playerSkill3].BasePower,
                       player->skills[*playerSkill3].CoinPower,
                       player->skills[*playerSkill3].Coins,
                       player->skills[*playerSkill3].Offense + player->OffenseBoost,
                       player->skills[*playerSkill3].Defense + player->DefenseBoost,
                       player->skills[*playerSkill3].Unbreakable);
            } else {
                printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                  getSkillTypeName(player->skills[*playerSkill3].skillType),
                       player->skills[*playerSkill3].name,
                       player->skills[*playerSkill3].BasePower,
                       player->skills[*playerSkill3].CoinPower,
                       player->skills[*playerSkill3].Coins,
                       player->skills[*playerSkill3].Offense + player->OffenseBoost,
                       player->skills[*playerSkill3].Defense + player->DefenseBoost);
            }


          // Defense Skill
        printf("\n");
          if (player->defenseSkill[playerDefenseSkill].Unbreakable > 0) {
            printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player->defenseSkill[playerDefenseSkill].skillType),
                   player->defenseSkill[playerDefenseSkill].name,
                   player->defenseSkill[playerDefenseSkill].BasePower,
                   player->defenseSkill[playerDefenseSkill].CoinPower,
                   player->defenseSkill[playerDefenseSkill].Coins,
                   player->defenseSkill[playerDefenseSkill].Offense + player->OffenseBoost,
                   player->defenseSkill[playerDefenseSkill].Defense + player->DefenseBoost,
                   player->defenseSkill[playerDefenseSkill].Unbreakable);
          } else if (player->defenseSkill[playerDefenseSkill].Unbreakable <= 0) {
            printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player->defenseSkill[playerDefenseSkill].skillType),
                   player->defenseSkill[playerDefenseSkill].name,
                   player->defenseSkill[playerDefenseSkill].BasePower,
                   player->defenseSkill[playerDefenseSkill].CoinPower,
                   player->defenseSkill[playerDefenseSkill].Coins,
                   player->defenseSkill[playerDefenseSkill].Offense + player->OffenseBoost,
                   player->defenseSkill[playerDefenseSkill].Defense + player->DefenseBoost);
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

        int playerTempOffense = 0, playerTempDefense = 0;
        int enemyTempOffense  = 0, enemyTempDefense  = 0;

        playerTempOffense += player->OffenseBoost;
        playerTempDefense += player->DefenseBoost;
        enemyTempOffense  += enemy.OffenseBoost;
        enemyTempDefense  += enemy.DefenseBoost;

          if (choice == 0) {
            // --- กรณีเลือก Guard (แทนที่ Skill 1) ---
            playerSkillEffective = &player->defenseSkill[0];

            // รันเหรียญป้องกัน
            if (player->defenseSkill[playerDefenseSkill].skillType == 1) {
              defensePhase(player, playerSkillEffective);
              }

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

     // roll skill ใหม่หลังเลือกแล้ว (สำหรับ turn หน้า)
      getSkills(&enemy, &kSkill1, &kSkill2, &kSkill3,
                kLastUnused, enemy.numSkills);

      // -------------------------------------------------------
      // getEffectiveSkill — knight ใช้ชื่อ player ชั่วคราว
      // เพื่อให้ passive buff ใน getEffectiveSkill ทำงาน
      // -------------------------------------------------------

      playerSkillEffective =
          getEffectiveSkill(player, &enemy,
                            playerSkillEffective,
                            &playerTempOffense, &playerTempDefense);

      SkillStats *enemySkillEffective =
          getEffectiveSkill(&enemy, player,
                            selectedEnemyPtr,
                            &enemyTempOffense, &enemyTempDefense);

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
                       (pType == 0 || pType == 4 || pType == 5);
  int canEnemyClash  = (enemySkillEffective != NULL) && 
                       (eType == 0 || eType == 4 || eType == 5);

  int willClash = (playerSkillEffective != NULL && enemySkillEffective != NULL) &&
                  playerSkillEffective->Clashable && 
                  enemySkillEffective->Clashable && 
                  canPlayerClash && canEnemyClash;

           if (!willClash) {

             if (pType != 3 || eType != 3) {
      if (playerGoesFirst == 1) {
        if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0 || playerSkillEffective->skillType == 3)) {
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
        if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0 || enemySkillEffective->skillType == 3)) {
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
          enemy.Tremor[2] += clash.playerFinalPower;
          printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                  player->name, player->name, enemy.name, clash.playerFinalPower);

          sleep(1);
          if (enemy.Tremor[2] > 50 && enemy.Stagger <= 0) {
            enemy.Stagger += 2;
            printf("\n%s Staggered for one turn\n", enemy.name);
            sleep(1);
             enemy.Tremor[2] = 0;
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
          player->Tremor[2] += clash.enemyFinalPower;
          printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                  enemy.name, enemy.name, player->name, clash.enemyFinalPower);
          sleep(1);
          if (player->Tremor[2] > 50 && player->Stagger <= 0) {
            player->Stagger += 2;
            printf("\n%s Staggered for one turn\n", player->name);
            sleep(1);
            player->Tremor[2] = 0;
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

  }

  // Player ตายใน knight phase
  if (player->HP <= 0) return;

  clearDebuffsOnDeath(boss, player);

    // ============================================================
    // PHASE 2 — Knight ตาย → Event → Boss ออกมา
    // ============================================================
  // คืนชื่อเดิมให้ Player ทันที เพื่อไปสู้กับ Boss ตัวจริงแบบไม่มี Tag
  player->name = realID;
  
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
        player->ClashPower += KingClashBonus;
      }

      // ใช้ฟังก์ชันใหม่ที่เราสร้างเพื่อเลือกว่าจะ "โจมตี" หรือ "ป้องกัน" ตามค่า Copies
      int decision = pickEnemyActionWeighted(&enemy, *enemySkill1, *enemySkill2);

      if (decision >= 100) {
          // --- กรณีเลือก Defense Skill ---
          int defIdx = decision - 100;
          enemySkillEffective = &enemy.defenseSkill[defIdx];
            eIdx = *enemySkill1; // เก็บ index ไว้หลอกระบบเผื่อใช้เลื่อนคิวสกิล (แต่มักจะไม่ขยับ)

          // ถ้าเป็น Guard (Type 1) ให้ทอยโล่ทันที
          if (enemySkillEffective->skillType == 1) {
              defensePhase(&enemy, enemySkillEffective);
          }
      } else {
          // --- กรณีเลือกท่าโจมตีปกติ ---
          eIdx = (decision == 1 ? *enemySkill1 : *enemySkill2);
          *enemyLastUnused = (eIdx == *enemySkill1 ? *enemySkill2 : *enemySkill1);
          enemySkillEffective = &enemy.skills[eIdx];

      }

      // For enemy
      getSkills(boss, enemySkill1, enemySkill2, enemySkill3,
      *enemyLastUnused, boss->numSkills);

      handleTurnStart(player, boss, &eIdx, playerSkill1, playerSkill2, enemySkill1, enemySkill2);

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
        if (isStaggered(boss)) printf("\n%s is STAGGERED and cannot act!\n", player->name);
        else if (isPanicked(boss)) printf("\n%s is in PANIC and cannot act!\n", player->name);

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
          handleBeforeFight(player, boss, &eIdx, *playerSkill1, *playerSkill2, *enemySkill1, *enemySkill2);
          handleBeforeFight(boss, player, &eIdx, *enemySkill1, *enemySkill2, *playerSkill1, *playerSkill2);

      if (!IsenemyUnableToAct) {
          if (boss->skills[eIdx].Unbreakable > 0 && (boss->skills[eIdx].Clashable || boss->skills[eIdx].skillType != 0))
            printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(boss->skills[eIdx].skillType),
                   boss->skills[eIdx].name,
                   boss->skills[eIdx].BasePower, boss->skills[eIdx].CoinPower,
                   boss->skills[eIdx].Coins,
                   boss->skills[eIdx].Offense + boss->OffenseBoost,
                   boss->skills[eIdx].Defense + boss->DefenseBoost,
                   boss->skills[eIdx].Unbreakable);
        else if (boss->skills[eIdx].Unbreakable <= 0 && (boss->skills[eIdx].Clashable || boss->skills[eIdx].skillType != 0))
            printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(boss->skills[eIdx].skillType),
                   boss->skills[eIdx].name,
                   boss->skills[eIdx].BasePower, boss->skills[eIdx].CoinPower,
                   boss->skills[eIdx].Coins,
                   boss->skills[eIdx].Offense + boss->OffenseBoost,
                   boss->skills[eIdx].Defense + boss->DefenseBoost);
        else if (boss->skills[eIdx].Unbreakable > 0 && !boss->skills[eIdx].Clashable && boss->skills[eIdx].skillType == 0)
            printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(boss->skills[eIdx].skillType),
                   boss->skills[eIdx].name,
                   boss->skills[eIdx].BasePower, boss->skills[eIdx].CoinPower,
                   boss->skills[eIdx].Coins,
                   boss->skills[eIdx].Offense + boss->OffenseBoost,
                   boss->skills[eIdx].Defense + boss->DefenseBoost,
                   boss->skills[eIdx].Unbreakable);
        else if (boss->skills[eIdx].Unbreakable <= 0 && !boss->skills[eIdx].Clashable && boss->skills[eIdx].skillType == 0)
            printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(boss->skills[eIdx].skillType),
                   boss->skills[eIdx].name,
                   boss->skills[eIdx].BasePower, boss->skills[eIdx].CoinPower,
                   boss->skills[eIdx].Coins,
                   boss->skills[eIdx].Offense + boss->OffenseBoost,
                   boss->skills[eIdx].Defense + boss->DefenseBoost);
      }

        // Player เลือก skill — copy จาก main() ทุกอย่าง
        int playerSkillIndex;

        if (!IsplayerUnableToAct) {
            printf("\nDashboard Skills:\n");

              if (player->skills[*playerSkill1].Unbreakable > 0 && player->skills[*playerSkill1].Clashable) {
                  printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                        getSkillTypeName(player->skills[*playerSkill1].skillType),
                         player->skills[*playerSkill1].name,
                         player->skills[*playerSkill1].BasePower,
                         player->skills[*playerSkill1].CoinPower,
                         player->skills[*playerSkill1].Coins,
                         player->skills[*playerSkill1].Offense + player->OffenseBoost,
                         player->skills[*playerSkill1].Defense + player->DefenseBoost,
                         player->skills[*playerSkill1].Unbreakable);
              } else if (player->skills[*playerSkill1].Unbreakable <= 0 && player->skills[*playerSkill1].Clashable) {
                  printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                    getSkillTypeName(player->skills[*playerSkill1].skillType),
                         player->skills[*playerSkill1].name,
                         player->skills[*playerSkill1].BasePower,
                         player->skills[*playerSkill1].CoinPower,
                         player->skills[*playerSkill1].Coins,
                         player->skills[*playerSkill1].Offense + player->OffenseBoost,
                         player->skills[*playerSkill1].Defense + player->DefenseBoost);
              } else if (player->skills[*playerSkill1].Unbreakable > 0 && !player->skills[*playerSkill1].Clashable) {
                  printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                    getSkillTypeName(player->skills[*playerSkill1].skillType),
                         player->skills[*playerSkill1].name,
                         player->skills[*playerSkill1].BasePower,
                         player->skills[*playerSkill1].CoinPower,
                         player->skills[*playerSkill1].Coins,
                         player->skills[*playerSkill1].Offense + player->OffenseBoost,
                         player->skills[*playerSkill1].Defense + player->DefenseBoost,
                         player->skills[*playerSkill1].Unbreakable);
              } else {
                  printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                    getSkillTypeName(player->skills[*playerSkill1].skillType),
                         player->skills[*playerSkill1].name,
                         player->skills[*playerSkill1].BasePower,
                         player->skills[*playerSkill1].CoinPower,
                         player->skills[*playerSkill1].Coins,
                         player->skills[*playerSkill1].Offense + player->OffenseBoost,
                         player->skills[*playerSkill1].Defense + player->DefenseBoost);
              }

              if (player->skills[*playerSkill2].Unbreakable > 0 && player->skills[*playerSkill2].Clashable) {
                  printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                    getSkillTypeName(player->skills[*playerSkill2].skillType),
                         player->skills[*playerSkill2].name,
                         player->skills[*playerSkill2].BasePower,
                         player->skills[*playerSkill2].CoinPower,
                         player->skills[*playerSkill2].Coins,
                         player->skills[*playerSkill2].Offense + player->OffenseBoost,
                         player->skills[*playerSkill2].Defense + player->DefenseBoost,
                         player->skills[*playerSkill2].Unbreakable);
              } else if (player->skills[*playerSkill2].Unbreakable <= 0 && player->skills[*playerSkill2].Clashable) {
                  printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                    getSkillTypeName(player->skills[*playerSkill2].skillType),
                         player->skills[*playerSkill2].name,
                         player->skills[*playerSkill2].BasePower,
                         player->skills[*playerSkill2].CoinPower,
                         player->skills[*playerSkill2].Coins,
                         player->skills[*playerSkill2].Offense + player->OffenseBoost,
                         player->skills[*playerSkill2].Defense + player->DefenseBoost);
              } else if (player->skills[*playerSkill2].Unbreakable > 0 && !player->skills[*playerSkill2].Clashable) {
                  printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                    getSkillTypeName(player->skills[*playerSkill2].skillType),
                         player->skills[*playerSkill2].name,
                         player->skills[*playerSkill2].BasePower,
                         player->skills[*playerSkill2].CoinPower,
                         player->skills[*playerSkill2].Coins,
                         player->skills[*playerSkill2].Offense + player->OffenseBoost,
                         player->skills[*playerSkill2].Defense + player->DefenseBoost,
                         player->skills[*playerSkill2].Unbreakable);
              } else {
                  printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                    getSkillTypeName(player->skills[*playerSkill2].skillType),
                         player->skills[*playerSkill2].name,
                         player->skills[*playerSkill2].BasePower,
                         player->skills[*playerSkill2].CoinPower,
                         player->skills[*playerSkill2].Coins,
                         player->skills[*playerSkill2].Offense + player->OffenseBoost,
                         player->skills[*playerSkill2].Defense + player->DefenseBoost);
              }

            // Next Skill
          
          if (player->skills[*playerSkill3].Unbreakable > 0 && player->skills[*playerSkill3].Clashable) {
                  printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                    getSkillTypeName(player->skills[*playerSkill3].skillType),
                         player->skills[*playerSkill3].name,
                         player->skills[*playerSkill3].BasePower,
                         player->skills[*playerSkill3].CoinPower,
                         player->skills[*playerSkill3].Coins,
                         player->skills[*playerSkill3].Offense + player->OffenseBoost,
                         player->skills[*playerSkill3].Defense + player->DefenseBoost,
                         player->skills[*playerSkill3].Unbreakable);
              } else if (player->skills[*playerSkill3].Unbreakable <= 0 && player->skills[*playerSkill3].Clashable) {
                  printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                    getSkillTypeName(player->skills[*playerSkill3].skillType),
                         player->skills[*playerSkill3].name,
                         player->skills[*playerSkill3].BasePower,
                         player->skills[*playerSkill3].CoinPower,
                         player->skills[*playerSkill3].Coins,
                         player->skills[*playerSkill3].Offense + player->OffenseBoost,
                         player->skills[*playerSkill3].Defense + player->DefenseBoost);
              } else if (player->skills[*playerSkill3].Unbreakable > 0 && !player->skills[*playerSkill3].Clashable) {
                  printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                    getSkillTypeName(player->skills[*playerSkill3].skillType),
                         player->skills[*playerSkill3].name,
                         player->skills[*playerSkill3].BasePower,
                         player->skills[*playerSkill3].CoinPower,
                         player->skills[*playerSkill3].Coins,
                         player->skills[*playerSkill3].Offense + player->OffenseBoost,
                         player->skills[*playerSkill3].Defense + player->DefenseBoost,
                         player->skills[*playerSkill3].Unbreakable);
              } else {
                  printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                    getSkillTypeName(player->skills[*playerSkill3].skillType),
                         player->skills[*playerSkill3].name,
                         player->skills[*playerSkill3].BasePower,
                         player->skills[*playerSkill3].CoinPower,
                         player->skills[*playerSkill3].Coins,
                         player->skills[*playerSkill3].Offense + player->OffenseBoost,
                         player->skills[*playerSkill3].Defense + player->DefenseBoost);
              }


            // Defense Skill
          printf("\n");
            if (player->defenseSkill[playerDefenseSkill].Unbreakable > 0) {
              printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                getSkillTypeName(player->defenseSkill[playerDefenseSkill].skillType),
                     player->defenseSkill[playerDefenseSkill].name,
                     player->defenseSkill[playerDefenseSkill].BasePower,
                     player->defenseSkill[playerDefenseSkill].CoinPower,
                     player->defenseSkill[playerDefenseSkill].Coins,
                     player->defenseSkill[playerDefenseSkill].Offense + player->OffenseBoost,
                     player->defenseSkill[playerDefenseSkill].Defense + player->DefenseBoost,
                     player->defenseSkill[playerDefenseSkill].Unbreakable);
            } else if (player->defenseSkill[playerDefenseSkill].Unbreakable <= 0) {
              printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
                getSkillTypeName(player->defenseSkill[playerDefenseSkill].skillType),
                     player->defenseSkill[playerDefenseSkill].name,
                     player->defenseSkill[playerDefenseSkill].BasePower,
                     player->defenseSkill[playerDefenseSkill].CoinPower,
                     player->defenseSkill[playerDefenseSkill].Coins,
                     player->defenseSkill[playerDefenseSkill].Offense + player->OffenseBoost,
                     player->defenseSkill[playerDefenseSkill].Defense + player->DefenseBoost);
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

          int playerTempOffense = 0, playerTempDefense = 0;
          int enemyTempOffense  = 0, enemyTempDefense  = 0;

          playerTempOffense += player->OffenseBoost;
          playerTempDefense += player->DefenseBoost;
          enemyTempOffense  += boss->OffenseBoost;
          enemyTempDefense  += boss->DefenseBoost;

          if (choice == 0) {
            // --- กรณีเลือก Guard (แทนที่ Skill 1) ---
            playerSkillEffective = &player->defenseSkill[0]; // ชี้ไปที่สกิลป้องกัน

            // รันเหรียญป้องกัน
            if (player->defenseSkill[playerDefenseSkill].skillType == 1) {
              defensePhase(player, playerSkillEffective);
              }

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
          

        playerSkillEffective =
            getEffectiveSkill(player, boss,
                              playerSkillEffective,
                              &playerTempOffense, &playerTempDefense);
        SkillStats *enemySkillEffective =
            getEffectiveSkill(boss, player,
                              &boss->skills[eIdx],
                              &enemyTempOffense, &enemyTempDefense);

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

      } else {
          
       int canPlayerClash = (playerSkillEffective != NULL) && 
                            (pType == 0 || pType == 4 || pType == 5);
       int canEnemyClash  = (enemySkillEffective != NULL) && 
                            (eType == 0 || eType == 4 || eType == 5);

       int willClash = (playerSkillEffective != NULL && enemySkillEffective != NULL) &&
                       playerSkillEffective->Clashable && 
                       enemySkillEffective->Clashable && 
                       canPlayerClash && canEnemyClash;

           if (!willClash) {

             if (pType != 3 || eType != 3) {

              if (playerGoesFirst == 1) {
                if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0 || playerSkillEffective->skillType == 3)) {
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
                  if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0 || enemySkillEffective->skillType == 3)) {
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
                boss->Tremor[2] += clash.playerFinalPower;
                printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                        player->name, player->name, boss->name, clash.playerFinalPower);
                sleep(1);
                if (boss->Tremor[2] > 50 && boss->Stagger <= 0) {
                  boss->Stagger += 2;
                  printf("\n%s Staggered for one turn\n", boss->name);
                  sleep(1);
                  boss->Tremor[2] = 0;
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
                player->Tremor[2] += clash.enemyFinalPower;
                printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                        boss->name, boss->name, player->name, clash.enemyFinalPower);
                sleep(1);
                if (player->Tremor[2] > 50 && player->Stagger <= 0) {
                  player->Stagger += 2;
                  printf("\n%s Staggered for one turn\n", player->name);
                  sleep(1);
                  player->Tremor[2] = 0;
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

      printf("\n--- Turn End ---\n");

        handleTurnEnd(player, boss, playerSkillEffective, enemySkillEffective);

        handleTurnEnd(boss, player, enemySkillEffective, playerSkillEffective);

        TurnCount++;
    }
  }
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
      if (selected_identity == 999) targetIndex = 99;      // Muga
      else if (selected_identity == 888) targetIndex = 88; // Binah
      else targetIndex = selected_identity - 1;
    
        if ((selected_identity >= 1 && selected_identity <= numIdentities) || targetIndex > 0) {

          // Setup temp characters for info preview
          if (targetIndex > 0) {
            setupCharacters(&tempPlayer, &tempEnemy, targetIndex, 0);
          } else {
          setupCharacters(&tempPlayer, &tempEnemy, selected_identity - 1, 0);
          }

          printf("\n--- Identity Info ---\n");
          printf("Name: %s\n", tempPlayer.name);
          printf("HP: %.0f / %.0f\n", tempPlayer.HP, tempPlayer.MAX_HP);
          printf("Speed: %d ~ %d\n", tempPlayer.MinSpeed, tempPlayer.MaxSpeed);
          
        if (selected_identity - 1 == 0) {
          //Taunt
          printf("\"Think it over three times, hard, before talking to me. I have ripped out enough tongues today.\"\n\n");

          //Description
          printf("A powerful character that focus on unbreakable Skills and dealing damage as much as possible, which comes with powerful skills that great for clashing, but clashing can become weak when it come in long term\n\n");
          
          //Passive
          printf("Passive Skills:\n");
           printf(" 1. Tiantui Star's Blade [天退星刀]\n Always Active: begin Encounters with Tigermark Round amount based on enemy (Min 12)\n");
         printf(" 2. Chachihu [揷翅虎]\n If this unit equipped Defense Skills for the first time in this Encounter, or if this unit spent all of 'Tigermark Round', or if this unit is Staggered, or HP at 65%% or less HP, reload 'Savage Tigermark Round' amount based on enemy (Min 8) and activate 'Unrelenting Spirit [剛氣]'\n");
          printf(" 3. Unrelenting Spirit [剛氣]\n deal +1%% damage for every Sanity different between this unit and enemy (Max 20%%), but at 8+ (sum of Tigermark Round and Savage Tigermark Round spent) activate 'Unrelenting Spirit - Shin [剛氣-心]' instead\n");
          printf(" 4. Unrelenting Spirit - Shin [剛氣-心]\n Defense +3, deal +2%% damage for every Sanity different between this unit and enemy (Max 40%%)\n");
         printf(" 5. Tigermark Round\n Skill Coins that spend Tigermark Round gain +1 Power and deal +10%% damage (activates only as long as the Coin has Rounds left to spend)\n" 
            " - At 1+ Tigermark Round and 3+ Tigermark Round spent, convert all Coins of 'Tanglecleaver' into Unbreakable Coins\n");
          printf(" 6. Savage Tigermark Round\n Skill Coins that spend Savage Tigermark Round gain +2 Power and deal +30%% damage (activates only as long as the Coin has Rounds left to spend)\n" 
           " - At 1+ Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins and Replace 'Tanglecleaver [快刀亂麻]' with 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]'\n" 
          " - At 0 Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins and Gain 'Overheat'\n");
        printf(" 7. Overheat\n Attack Skills Lose (cumulative number of Tigermark Rounds & Savage Tigermark Rounds spent / 4) Clash Power (Max 5); however, gain the following effects(cumulative):\n"
         " - 8+ Rounds spent: Take 10%% less damage for every 10%% missing HP on self at Turn Start (max 50%%)\n"
         " - 14+ Rounds spent: On Clash Lose, Unbreakable Coins of this unit's Attack Skills deal +(75 + missing HP percentage on self)%% damage (Max 150%%)\n"
          " - 20+ Rounds spent: Deal +(HP percentage difference)%% damage against targets with higher remaining HP percentage than this unit (Max 50%%)\n");
          } 
        else if (selected_identity - 1 == 1) {
          //Taunt
          printf("\"We are not placing our stone here, then? Mm, then the tides drive us to resign.\"\n\n");

          //Description
          printf("A character with powerful counter skill and great damage skills with anti-death passive.\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Swordplay of the Homeland\n In Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. (Once per Encounter)\n");
          printf(" 2. Yield My Flesh\n When Clashing with 'Yield My Flesh' does not effects by any Clash Power boost, When Clash loses with 'Yield My Flesh', Use Counter 'To Claim Their Bones' to attack back (Cannot be used only if this unit died first)\n");
          printf(" 3. In Memoriam\n At 60%% or less HP, Apply 'Remembrance' buff on self, Gains buff at 10+ Sanity or 30+ Sanity further from 0 (Buff base on each Skills)\n");
          printf(" 4. Overthrow\n After got attacked, gain +1 Final Power next turn (Once per enemy's skill)\n");
              } 
        else if (selected_identity - 1 == 2) {
          //Taunt
          printf("\"What kindled this flame of wrath that burns within me...? ...No, it doesn't matter why it burns- What matters is that I am the ripping and tearing tempest that will bring about their ruin.\"\n\n");

          //Description
          printf("A character with great buff and strong damage output with focus on Sanity but also come with less HP and Defense\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Dullahan\n If this unit equipped Defense Skills and does not have 'Dullahan', gain 1 'Dullahan' next turn (Max 3), Offense +3, Defense -3, Raise Min & Max Speed by 1, If this unit equipped Defense Skills while having 'Dullahan', Turn End: loses all 'Dullahan'."
            " - Turn End: When this unit's mounts 'Dullahan', gain 1 'Dullahan' (Max 3), lose 5 Sanity, if this unit's Sanity at -25 or less; however lose all 'Dullahan', lose (15 - (Coffin / 2)) Sanity (Min 10)." 
            " - When lost the Clash, At 15+ Sanity, use 'Lament, Mourn, and Despair' to continue the Clash (Once per Turn)\n");
          printf(" 2. Call of the Erlking\n When at 50%% or less HP, or at -45 Sanity, Turn Start: if this unit at -45, does not 'Panic' and if this unit does not have 'Dullahan', gain 'Dullahan' and snap out from 'Stagger' and if this unit's Sanity at 0 or less, heal Sanity the further this unit's Sanity is from 0 (heal 2 additionalal Sanity for every missing Sanity; Max 50) (Once per Encounter)\n");
          printf(" 3. Endless Lamentation\n When mounts 'Dullahan', and using Defense Skills at 15+ Sanity or using 'Requiem', use 'Lament, Mourn, and Despair' instead (if from Defense Skills use as Clashable Counter)\n");
           printf(" 4. Coffin\n Gain by using 'Requiem' and 'Lament, Mourn, and Despair', gain 20%% damage for every 3 Coffin, gain 1 Clash Power for every 5 Coffin (Max 10)\n");
          printf(" 5. Impending Ruin\n Inflicted by certain Skill: This units -10%% chance flip Heads\n");
              } 
        else if (selected_identity - 1 == 3) {
          //Taunt
          printf("\"The Lord of Hongyuan marches to war.\"\n\n");

          //Description
          printf("A low HP character with great buff, strong damage output, anti-death passive and followers that can help you\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Embrace the Tarnished Blood and Exsanguinate Others For the Cause\n In Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn, after that apply 'Lordsguard' to the left Heishou Pack (Once per Encounter)\n");
          printf(" 2. The Heishou Pack\n The Heishou Pack will heed you as their lord, you have 4 Heishou Packs(Mao, Si, Wu and You) as your follower, when using 'Answer Me, Heishou Packs', command one of the remaining Heishou Pack members to attack alongside this unit with 'I Carve the Path of a Lord' Skill; then gain buff from Heishou Pack after that 'Retreat' that Heishou Pack make them unable to use entire Encounter. If there is no Heishou Pack left use 'Lonesome Stand: Sacrifice to Claim The Garden [孑孑單身，捨生取园]' instead\n");
          printf(" 3. The Heishou Lord\n After 'Embrace the Tarnished Blood and Exsanguinate Others For the Cause' activated, when this unit takes damage from any damage skills that can bring their HP down to 0, one of left Heishou Pack use 'Lordsgurad' to defense this unit after that 'Retreat' that Heishou Pack make them unable to use entire Encounter, if the damage can't bring their HP down to 0, do not use 'Retreat'\n");
          printf(" 4. Rupture\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Hit: Take (Stack) fixed Damage. Then reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf(" 5. Poise\n When Gain by Certain Skills: At 1+ Count, or at 1+ Stack (If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Attack: Have a (Stack x 5)%% chance to 'Critical Hit' (Boost damage by 20%%). If this unit Critical Hit, reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
              } 
        else if (selected_identity - 1 == 4) {
          //Taunt
          printf("\"They would point and jeer at the rags splattered with the blood of fellowship. The fools; only I can grasp the highest degree of tragedy upon this earth.\"\n\n");

          //Description
          printf("A low HP character with Insane damage output and focus on building up 'Fell Bullet' for better potential\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Volatilized Memory\n When using Skills expect 'Target Readjustment Fire' gain 'Torn Memory' which use for 'Target Readjustment Fire' to buff it\n");
          printf(" 2. I shall Fire\n After used 'Target Readjustment Fire', lose (Torn Memory x 2) Sanity, at 7+ 'Torn Memory' gain 'Fell Bullet'\n");
          printf(" 3. Fell Bullet\n When lost 'Torn Memory' gains 'Fell Bullet', All skills' Damage Multiplier +0.2 and Clash Power +2 then heal 20 Sanity on self (Stackable). Combat Start: Gain +3 Poise Stack and +1 Poise Count, when inflicting Bleed, inflict more Bleed Stack or Count by 1 (this effect activates as long as there is Fell Bullet in this unit)\n");
          printf(" 4. Poise\n When Gain by Certain Skills: At 1+ Count, or at 1+ Stack (If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Attack: Have a (Stack x 5)%% chance to 'Critical Hit' (Boost damage by 20%%). If this unit Critical Hit, reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf(" 5. Bleed\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When this unit tossing the Attack Skill's Coins or Clashing end for one round, take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
              } 
        else if (selected_identity - 1 == 5) {
          //Taunt
          printf("\"The Family will be well-cared for. ...After all, the onus always fell on me to provide for what you abandoned.\"\n\n");

          //Description
          printf("A low HP and defense character with various for each skill and healing skills\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Variant Sancho Hardblood Arts\n Turn Start: When at 15+ 'Hardblood' using Variant Sancho Hardblood Arts instead for each skills\n");
          printf(" 2. Bearer of the Blood Kin\n When at 50%% or less HP, 'Responsibility' activate, Clash Power +1, Deal +20%% damage, Take +20%% damage and gain 3 Hardblood\n");
          printf(" 3. Blood... is flowing...\n If this unit lost the Clash and equipped Attack Skill that isn't 'Variant Sancho Hardblood' empowered, consumes 5 'Hardblood' to use 'Laughters Will Subside' to continue clashing (Once per Turn), if win the Clash with 'Laughters Will Subside' gain 5 'Hardblood'. At 10+ use 'Variant Sancho Hardblood Arts 15 - Buildup to Finale' instead\n");
          printf(" 4. Armadura de Sangre\n Gain 10%% Damage Up next turn for every 15%% missing HP at Turn End (Max 30%%)\n");
          printf(" 5. Hardblood\n When use certain skills gain randomly 2-4 Hardblood. When hit by enemy gain 2 Hardblood (Max 30; 'Hardblood' cannot drop below 1)"
            "\n - at 10+ 'Hardblood': Gain +1 Offense for every 5 Hardblood"
            "\n - at 20+ 'Hardblood': Gain +1 Offense and +1 Defense for every 5 Hardblood\n");
              } 
          else if (selected_identity - 1 == 6) {
            //Taunt
            printf("\"Well do I understand your sentiment on death. Why not lay rest to the impulses in your heart for a moment and converse with me more?\"\n\n");

            //Description
            printf("A low HP character with a great debuff Skills and strong clashing, but come up with limit attack, sometimes on low Living & The Departed getting weak and need to recharge which uses Sanity\n\n");

            //Passive
            printf("Passive Skills:\n");
            printf(" 1. The Living & The Departed\n Start encounter with 20 'The Living & The Departed' (Max 20), When inflicts 'Butterfly' on enemy, spent this equal to inflicted numbers\n");
            printf(" 2. Butterfly\n "
              " When this unit at 0 or higher Sanity\n"
              " - On attack with 'Butterfly', 30%% heals Sanity on self or 70%% loses Sanity on enmey equal to (Butterfly/3; Min 1)\n"
              " When this unit at less than 0 \n"
              " - On attack with 'Butterfly', 70%% heals Sanity on self or 30%% loses Sanity on enmey equal to (Butterfly/3; Min 1)\n"
              " On enemy with loses Sanity on Clash Win, heals Sanity on enmey instead and on enemy without Sanity, deal more damage equal to (Butterfly/3; Min 1) instead\n"
              " On attack enemy with 'Butterfly', if enemy's Sanity less than 0 (enemy with loses Sanity on Clash Win, more than 0 Sanity instead), or without Sanity, deal (Butterfly/2 - enemy's Sanity/5) fixed damage (deals (Butterfly/2 + enemy's Sanity/5) fixed damage to enemy with loses Sanity on Clash Win; deals (Butterfly/2) fixed damage to enemy without Sanity; rounded down)\n"
              " Turn End: this effect Expire\n");
            printf(" 3. Reload\n When runs out of 'The Living & The Departed', Turn End uses 'Reload', while attacking, stop attack and use 'Reload' instead, when 'Reload' is used, spend 15 Sanity to gain 20 'The Living & The Departed' and gain Shield equal to (Butterfly on the target x 2)%% of Max HP. (Max 40%%)\n");
             printf(" 4. FromTheCoffinAButterflyTakesFlight\n On Clash Win: Enemy loses Sanity based on Skills used (Enemy with loses Sanity on Clash Win, gains Sanity instead; enemy without Sanity gains 'Butterfly' instead)\n");
                } 
            else if (selected_identity - 1 == 7) {
              //Taunt
              printf("\"Sometimes I get hand tremors... I hope that doesn't make me look like a coward.\"\n\n");

              //Description
              printf("A low HP character that focus on build Sanity and gain buff from high Sanity\n\n");

              //Passive
              printf("Passive Skills:\n");
              printf(" 1. Unstable Shell of Ego\n Turn Start: At 40+ Sanity, consume 20 Sanity to enter the Volatile E.G.O::Waxen Pinion state. At 30%% or less HP and if this unit's Sanity isn't at -45 at Turn End, reset Sanity to 45; then, enter the Volatile E.G.O::Waxen Pinion state. (Once per Encounter) (this 'Turn Start' effect does not activate repeatedly).\n");
              printf(" 2. Determination\n Turn Start: At 0 or less SP, if in the Volatile E.G.O::Waxen Pinion state; exit the Volatile E.G.O::Waxen Pinion state and loses all 'Volatile Passion' to gain +(Volatile Passion x 3 - Max 20) Clash Power this turn and next turn\n");
               printf(" 3. Volatile Passion\n Turn Start: Gain 1 'Volatile Passion' while in the Volatile E.G.O state, gain 1 Final Power, gain +20%% damage for every stack. Turn End: lose 5 Sanity for every stack(Max 40 Sanity)\n");
              printf(" 4. Stigma Workshop Weaponry / Passion\n When this unit at 20+ Sanity, gain Clash Power +(Sanity/20). At 45 Sanity, gain Final Power +3 instead. When in a Volatile E.G.O state, and at 0+ Sanity, gain Coin Power +(Sanity/20). At 45 Sanity, gain Coin Power +3 instead.\n");
                  }
             else if (selected_identity - 1 == 8) {
          //Taunt
          printf("\"They're... all from our Office. Firefist Office.\"\n\n");

          //Description
          printf("A High HP and defense character with powerful skill 3 along with damage buff and burn for every skills\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. I'm the only survivor...\n When enemy's HP or this unit's HP at 75%% or less HP, Deal +0.2%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (Max 40%%)\n");
               printf(" - If main target have 30+ (Burn Stack + Burn Count), deal +0.3%% damage instead (Max 60%%)\n");
          printf(" 2. District 12 Special Workshop Fuel\n When start Encounter gain 100 'District 12 Fuel' use for certain skills, when at 50 or less become 'Overheated Fuel', Buff all skills and Burn inflicting; when 'Overheated Fuel' reach 0 use 'I have to keep going for big' instead of current skill\n");
               printf(" 3. ... All burnt to ashes.\n When attack with skills, inflict 'Burn' based on skills used\n");
               printf(" 4. Burn\n When Inflicted: At 1+ Count, or at 1+ Stack (Turn End: If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), Turn End: Take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
              }
               else if (selected_identity - 1 == 9) {
                 //Taunt
                 printf("\"Yesss...! Finally! Listen up, gamefowls! Get your talons out! We'll be fightin' the night away tonight, 'till there's no more feed left on the sand circle...!\"\n\n");

                 //Description
                 printf("A character sacrifics it's HP for enhance Skills and come with powerful Skill 3 in 2 various\n\n");

                 //Passive
                 printf("Passive Skills:\n");
                 printf(" 1. Flame Rooster's Death Defiance [炎鳥不死戦]\n In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. Then, at the next Turn Start, heal (20 + Burn Stack on self)%% HP, and remove all Burn on self (Max 49%%; once per Encounter)\n");
                 printf(" 2. Burn\n When Inflicted: At 1+ Count, or at 1+ Stack (Turn End: If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), Turn End: Take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                  printf(" 3. Gamefowl\n Cannot be fall below 1 HP due to Burn damage. When use certain skills gains Burn\n");
                 printf(" 4. Bloody Storm of Blades\n Combat Start: gain 1 Offense Up and 1 Defense Up for every 20%% missing HP on self (Max 3)\n");
                 printf(" 5. Bloodflame [血炎]\n Gain by using certain Skills (Max 3 Stack), when attack heal 3 Sanity. At 45+ Sanity, gain 1 Offense for next turn instead (Max 3 times per turn), Turn End: lose 1 Stack\n");
                 printf(" 6. Battleblood Instinct\n Deal 0.75%% damage for every Stack (Max 20 Stack). At 20+ Stack,  activate 'Rooster's Rampaging Blades Under the Ensanguined Heaven' instead of 'Bloodflame Massacre [血炎亂舞]'\n Gain 'Battleblood Instinct' when meeting one of the following\n");
                 printf(" -  Gain 3 at Clash Start\n"); 
                 printf(" -  Gain 1 when this unit hits with a Base Skill or when this unit takes Burn damage (At less than 50%% HP, gain 1 more Battleblood Instinct)\n");
                     }
                 else if (selected_identity - 1 == 10) {
                    //Taunt
                    printf("\"We crossed everyone on the list for today, and... Alright, time to tell Big Brother we're all set to chase down those hair coupon thieves.\"\n\n");

                    //Description
                    printf("A High HP and defense character with that focus on stacking passive by taking damage and use Counter skill to attack back to enemy\n\n");

                    //Passive
                    printf("Passive Skills:\n");
                    printf(" 1. The Middle Never Forgets\n When hit by an enemy, inflict 5 'Vendetta Mark' against the attacker (Once per Turn)\n");
                   printf(" Attack End, consume all 'Vendetta Mark' on the target"
                     "\n  - Every time the target consumes 'Vendetta Mark', gain Book of 'Book of Vengeance [Sinclair]' equal to the amount consumed\n");
                   printf(" 2. Vendetta Mark\n Take 2%% damage for every Stack (Max 20%%) (Max 10 Stack)\n");
                   printf(" 3. Book of Vengeance [Sinclair]\n Gain more damage equal to (Stack)%% (Max 30%%) (Max 30 Stack)\n");
                   printf(" Gain the following effect next turn based on Stack:"
                      "\n  - At 10+ Stack: Gain 30%% Damage Up"
                     "\n  - At 20+ Stack: Gain 1 Clash Power Up and 1 Base Power Up"
                     "\n  - At 30 Stack: Gain 50%% Damage Up\n");
                    printf(" 4. The Middle Tattoo\n Combat Start: If this unit using attack skills except for 'Is it You?!', gain 1 Envy Resonance (Once per Turn; lose this effect if this unit 'Stagger' or 'Panic'), use for certain skills, lose all when use 'Warmup in the East'\n");
                        }
                   else if (selected_identity - 1 == 11) {
                     //Taunt
                     printf("\"A life wherein one is granted not choices to make, but instead, choices made...\" *beep* \"Hah. Would that my darling daughter, too, could have felt the mirth that colors such a life.\"\n\n");

                     //Description
                     printf("A low-HP Character with the high power and high damage under certain condition and come along with A Powerful 9 Coins skills\n\n");

                     //Passive
                     printf("Passive Skills:\n");
                     printf(" 1. Prescript Delivered on a Device\n Turn Start:\n"
                       " - Gain 'Prescript: [Device] I' / 'Prescript: [Device] II' / 'Prescript: [Device] III' / 'Prescript: [Device] IV' based on 'Unlock' stage on self"
                       "\n - Inflict 'The Prescript's Target' to the enemy"
                       "\n - Apply 'Mark of the Prescript' to Attack Skills on this unit's Dashboard"
                       "\n · At 'Unlock - II'+, the effect above prioritizes Skill 3 (prioritizes empowered Skill)"
                      "\n - All of the effects above and Prescript execution checks do not trigger when this unit is 'Staggered', or in 'Panic'\n");
                     printf(" 2. Prescript: [Device] \n "
                       " - Prescript: [Device] I : Use a Skill with 'Mark of the Prescript'.\n"
                       " - Prescript: [Device] II : Hit a target with a Skill with 'Mark of the Prescript'.\n"
                       " - Prescript: [Device] III : Gain Procuration [Hermes]. Repeat this Prescript until Procuration [Hermes] reaches 9 Stacks.\n"
                       " - Prescript: [Device] IV : Eliminate all enemies before next Prescript arrvies.\n");
                     printf(" 3. The Prescript's Target \n Take +10%% damage from Index units\n");
                    printf(" 4. The Oracle's Proxy / Unlock\n Turn End: If Prescript was executed this turn at below 'Unlock - II'"
                     "\n - When executing Prescript, if the main target has 'The Prescript's Target', heal 8 Sanity and gain 3 'Grace of the Prescript'\n\n"
                     " Turn End: If this unit is at 'Unlock - II' and Prescript was executed this turn, heal 4 SP\n"
                     " - When executing Prescript, if 'Procuration [Hermes]' has reached 9 Stacks, heal 8 Sanity and gain 3 'Grace of the Prescript'\n\n"
                      "Turn End: If Prescript was not executed this turn, gain 5 'Karmic Consequence'\n"
                      " - At 'Unlock - III', does not gain 'Karmic Consequence'"
                     "\n - If this unit attempted to execute Prescript at Combat Start, but could not target the enemy during the combat phase, does not gain 'Karmic Consequence' at Turn End (applies below 'Unlock - II')\n\n"
                      "Turn End: At 3/6/9 'Grace of the Prescript', gain 'Unlock - I' / 'Unlock - II' / 'Unlock - III'\n\n"
                     "Turn Start: At 'Unlock - III', gain 'Shin (心) - Fate'\n");
                     printf(" 5. Unlock stage - I / II / III \n" 
                       " Unlock stage I - Defense +1, Combat End: Heal 5 Sanity"
                       "\n Unlock stage II - Defense +2, Combat End: Heal 10 Sanity"
                      "\n Unlock stage III - Defense +3, Combat End: Heal 15 Sanity\n");
                     printf(" 6. Grace of the Prescript \n Offense +1 for every 3 Stack (Max 9 Stack)\n");
                      printf(" 7. Procuration [Hermes] \n - Turn Start: at 9 Stack, a Powerful Skill become available"
                        "\n - Can be gained up to (Unlock Stage + 2) Stack per turn"
                        "\n - Max Stack: 9"
                        "\n · At below 'Unlock - II', this effect's maximum stack is limited to 8\n");
                     printf(" 8. Karmic Consequence \n Turn Start:"
                       "\n · Gain 1 Defense Down for every 10 Stack"
                       "\n · Takes +10 Damage for every 20 Stack"
                       "\n - Max Stack: 100\n");
                     printf(" 9. Shin (心) - Fate \n - Gain +1 Offense and +1 Defense"
                        "\n - Gain +1 Offense for every 20%% (missing HP percentage on target + missing HP percentage on self; rounded down) (Max 3)"
                        "\n - If this unit's Sanity higher than the target's, deal +1%% damage for every 3 Sanity different (Max 15%%; units without Sanity considered to be 0 Sanity)\n");
                    printf(" 10. The Index Nursefather\n Upon entering the Encounter for the first time, gain 'Wound-casing Mask'"
                      "\n - Turn End: If this unit was Staggered for the first time, or at 65%% or less HP, in this Encounter while under this effect, recover from Stagger (excluding forced Stagger) and convert 'Wound-casing Mask' to 'Sizzling Wound'\n");
                     printf(" 11. Wound-casing Mask \n Offense +2 and Defense -2, take -10%% damage from Cracking Unbreakable Coins\n");
                     printf(" 12. Sizzling Wound \n Offense +3 and Defense -3, take -25%% damage from Cracking Unbreakable Coins, deal +15%% damage with Attack Skill's Unbreakable Coins. Turn End: Lose 4 HP\n");
                     printf(" 13. Oracle Device [Caduceus]\n A random weapon is assigned to every Coin for Base Attack Skills. Each weapon has a unique effect."
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
                      printf(" 14. Indulgence in Prescripts \n Base Power +1, Damage +30%%, Clash Power +2\n");
                      printf(" 15. Imitation of a Life\n Deal +2%% damage with Skills marked with 'Mark of the Prescript' for every 'Grace of the Prescript' on self (Max 16%%)"
                       "\n - At 9 'Grace of the Prescript', deal +20%% damage with Base Skills instead\n\n"
                        "On Hit with Base Attack Skill's Unbreakable Coins, gain 1 'Procuration [Hermes]'"
                       "\n - Attack End: Gain 'Procuration [Hermes]' equal to (# of remaining Unbreakable Coins)"
                       "\n - 'Furioso-Replica' Attack End: Gain 'Procuration [Hermes]' next turn equal to (# of this Skill's remaining Unbreakable Coins / 2) (rounded down)"
                        "\n\nIf 'Procuration [Hermes]' reached 9 Stacks this turn at Turn End, and if this unit does not have a Skill 3 on the Dashboard at the start of the next turn, convert a Base Skill to Skill 3 (prioritizes the Skill on the top Slot's row; Only 1 copy of this Skill can exist on the Dashboard)\n");
                     printf(" 16. By Unpredictable Whim \n When using 'By Unpredictable Whim', At below Unlock - II, activate the following effects (once per Encounter):\n"
                       " - Gain Unlock - II\n"
                       " - If Grace of the Prescript Stack is less than 6, raise the Stack to 6\n"
                       " - Gain 5 Karmic Consequence at the start of the next turn for every Grace of the Prescript gained via the effect above\n");
                         }
                     else if (selected_identity - 1 == 12) {
                       //Taunt
                       printf("\"Will you join me... in the great task to purify the abominable filth?\"\n\n");

                       //Description
                       printf("A character that focus on inflict negative status to enemy and building Sanity\n\n");

                       //Passive
                       printf("Passive Skills:\n");
                       printf(" 1. Whistles\n When attack or evade (with Passive 'Such Filth') enemy for 3 times, Next Turn Start: gains 15 Sanity and apply 2 'Fanatic' on self\n");
                      printf(" 2. Fanatic\n Skill Final Power +(Stack) against enemy with 'Nail' for this turn\n");
                      printf(" 3. Nail\n"
                        " - Turn Start: Gain 1 Bleed and (Stack) Bleed Count\n"
                        " - Turn End: Halve this effect's Stack (Rounded down)\n");
                       printf(" 4. Bleed\n When Inflicted by Certain Skills: At 1+ Count, or at 1+ Stack (If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When this unit tossing the Attack Skill's Coins or Clashing end for one round, take fixed damage equal to (Stack). Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                       printf(" 5. Gaze\n Inflict by certain skill, Take +20%% damage, when hit by an enemy, Enemy's Attack End: Enemy gains 5 Sanity; then loses 'Gaze'. Turn End: loses this effect\n");
                       printf(" 6. Such Filth\n Before getting attack by Skill, If this unit has 'Fanatic', consumes all 'Fanatic' and then tossing the Coins to evade the attack with below effect\n"
                         " - Base Power: 4\n"
                         " - Coin Power: 10 + (Consumed 'Fanatic')\n"
                         " - This Coin doesn't effect by Paralyze or any effect from tossing Coin\n"
                         " If this Coin's Power more or equal to Attack Power, ignore the attack damage that dealing this unit (this effect also ignore damage from the damage or hit effect from other source), when this effect activate if this unit get attack, cancel all effect\n");
                       printf(" 7. You Must Accept the Pain!\n When using 'Execution' and Enemy has 3+ 'Nail', use 'Purify' instead\n");
                       printf(" 8. Bliss of Execution\n When attack with Skill, After Attack: If this unit has 'Fanatic' and enemy have 6+ 'Nail', use 'I Shall Claim Your Life!' after the attack (Once per time)\n");
                           }
                       else if (targetIndex == 99) {
                          //Taunt
                          printf("\"Once it is done... I will take \x1b[1;31mAraya\x1b[1;0m back.\"\n\n");

                          //Description
                          printf("Erasing me... Erasing you...\n\n");

                          //Passive
                          printf("Passive Skills:\n");
                          printf(" 1. Wading Through a Dream, the Self Nowhere to be Found [無我夢中]\n"
                            " Turn Start: Gain Muga [無我] equal to current turn count\n"
                            " Gain 4 Muga [無我] for every use of this unit's Coin\n\n"

                            " \x1b[1;30m'I must make it look as though Araya is already safely out of the House of Spiders.'\x1b[0m\n\n"

                            " \x1b[1;30m'I'll stage it by raising this blade against the Nursefathers—that should sell the story that I am trying to stop them from going after Araya.' \x1b[0m\n\n");
                         printf(" 2. Muga [無我]\n"
                         " - Turn Start: Gain 1 Offense Level Up and 1 Defense Level Up for every 10 Stack\n"
                            " - The more this effect stacks...\n"
                            " - Max Stack: 100\n\n"

                             " \x1b[1;30m'So I must hold back on using the blade. And once I'm ready, I will return to that House.'\x1b[0m\n\n");
                         printf(" 3. Like the Naraka of Avīci and Raurava [阿鼻叫喚]\n"
                           " Encounter Start: Gain 'Tiagnsha Star's Blade - Arayashiki [天殺星刀阿賴耶識]' and fix SP at -44\n\n"

                            " When taking Sanity damage including effects caused by Sinking, take (Sanity damage x 3) HP damage instead\n\n"

                              " \x1b[1;30m'I know better than anyone how strong and tenacious the Nursefathers are.'\x1b[0m\n\n"

                           " \x1b[1;30m'If I am to deceive them, I'll need to stage a desperate act with everything at my disposal.'\x1b[0m\n\n"

                           " \x1b[1;30m'Plan how I'll take on each of them—how their swords will clash with mine.'\x1b[0m\n\n"

                           " \x1b[1;30m'I may lose control over my own mind, but I will make sure to run the picture through my head over and over.'\x1b[0m\n\n");
                      printf(" 4. Tiansha Star's Blade - Arayashiki [天殺星刀阿賴耶識]\n"
                             " - Min & Max Speed +6\n"
                        " - Offense Level +6\n"
                       " - Inflict 3 more Bleed Stack with Skills\n"
                       " - Turn Start: Gain 3 Offense Level Up for every hit enemy attack this unit (Max 6)\n\n"

                             " \x1b[1;30m'The heavens themselves are not spared—heaven, earth, man, and the self. This star rises only for the one who perceives all existence and time as a single whole to sever them all.'\x1b[0m\n");
                         printf(" 5. Severed and Torn until Even the Form is Undone [支離滅裂]\n"
                            " On Hit, inflict 2 Sever the Thread [切絲]\n"
                          " - Turn Start: Inflict 1 more Sever the Thread [切絲] for every 10 Muga [無我] on self (Max 4)\n\n"

                          " When hit, take -(Muga [無我] on self + Sever the Thread [切絲] on target)%% damage (Max 90%%)\n\n"

                          " If there is an enemy that has 100 Sever the Thread [切絲] after this unit finishes all attacks with its Skills, use a powerful Skill on target\n\n"

                           " \x1b[1;30m'I'm not strong enough to kill the Nursefathers—this I know. But I can still incapacitate and hold them back, if only for a little while.'\x1b[0m\n\n"

                            " \x1b[1;30m'And then...'\x1b[0m\n\n"

                            " \x1b[1;30m'And then, I'll leave...'\x1b[0m\n\n"

                           " \x1b[1;30m'I'll leave and come back stronger.'\x1b[0m\n\n"

                            " \x1b[1;30m'Secure the help of someone stronger than me, or gain more allies... Either way, I must come back with a guaranteed means of dealing with them once and for all.'\x1b[0m\n\n");
                         printf(" 6. Sever the Thread [切絲]\n"
                          " - Turn Start:\n"
                          " · Gain +3 Bleed Stack and +1 Bleed Count\n"
                          " · Take damage equal to (Stack / 3)\n"
                          " - When hit, take damage equal to (Stack / 5)\n"
                         " - The more this effect stacks...\n"
                          " - Max Stack: 100\n\n"

                           " \x1b[1;30m'Let everything be severed—you, me, all that has been, and all that will be.'\x1b[0m\n\n"

                            " \x1b[1;30m'Once it is done... I will take Araya back.'\x1b[0m\n\n");
          }
             else if (targetIndex == 88) {
        //Taunt
        printf("\"You bear a poison, heavy and slow... yet deadly. I know you well, even though you know nothing about me.'\n\n");

        //Description
        printf("??????????????????????????????????????????????????????????????????????????????????\n\n");

        //Passive
        printf("Passive Skills:\n");
        printf(" 1. The Final Reception\n At 50%% or less HP, activate 'Serious', increase HP and Max HP to 1150, gains +100%% damage and 5 Final Power for one turn; then gain new Skills set (Once per Encounter) (Cannot be defeat until this effect activated)\n");
             printf(" 2. Fairy\n Inflict by certain Skills, Take (Fairy Stack) fixed damage addition for every hit, if this unit's in activated 'An Arbiter', Take (0.5 x Fairy Stack)%% Max HP fixed damage addition for every hit instead. Turn End: Take (Fairy Stack) fixed true damage (true damage ignore Shield HP); then halve stack (Round down), if this unit's in activated 'An Arbiter', Take (0.5 x Fairy Stack)%% Max HP true fixed damage (true damage ignore Shield HP); then halve stack (Round down) instead\n");
             printf(" 3. Incomplete Arbiter\n In this Encounter, deals -20%% damage, Final Power -1. When activation 'Serious', activate 'An Arbiter' instead\n");
        printf(" 4. An Arbiter\n Gains +50%% damage, Final Power +2, Deal +20%% damage and +10 Base Power for every Fairy on enemy, when getting attack by Full Cracking Coin, Take -80%% damage and gain (50 + Missing HP/3) Shield HP (Max 100 per attacked), when getting attack and at 0+ Sanity, consumes 10 Sanity to gain (100 + Missing HP/2) Shield HP (Max 300 per attacked)\n");
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
              for (int i = 0; i < numIdentities - 1; i++) // add for binah
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
      if (scanf("%d", &selected_enemy) == 1 &&
          selected_enemy >= 1 && selected_enemy <= numEnemies) {

          // Setup temp characters for info preview
          setupCharacters(&tempPlayer, &tempEnemy, 0, selected_enemy - 1);

          printf("\n--- Enemy Info ---\n");
          printf("Name: %s\n", tempEnemy.name);
          printf("HP: %.0f / %.0f\n", tempEnemy.HP, tempEnemy.MAX_HP);
          printf("Speed: %d ~ %d\n", tempEnemy.MinSpeed, tempEnemy.MaxSpeed);

          if (selected_enemy - 1 == 0) {
            //Taunt
            printf("\"Give me your money!\"\n\n");

            //Description
            printf("just a normal Bandit.\n\n");

            //Passive
             printf("Passive Skills: -\n");
          } 
          else if (selected_enemy - 1 == 1) {
            //Taunt
            printf("\"That's right, ya shrimps. Ya gotta first wrack them teensy' brains o' yours, gotta think real hard 'bout whether you even come close to my rank before runnin' ya mouths, ya hear?\"\n\n");

            //Description
            printf("A high-HP, high-Defense boss who focuses on building strength through repeated clashes, and attack with powerful attack\n\n");

            //Passive
             printf("Passive Skills:\n");
            printf(" 1. Panic Recovery\n If this unit is Panicked still can act, after reset Sanity to 0 and heal Sanity by this unit's missing HP (Max 30)\n");
            printf(" 2. Tigermark Round Reload\n Turn End: at 90%% or less HP, or at the end of the 2nd turn, gain a new pattern\n");
            printf(" 3. Lei Heng [雷橫]\n Turn End: at 80%% or less HP, or at the end of the 4th turn, gain a new pattern, gain 25 Inner Strength [底力] and use a powerful attack 'Tanglecleaver', repeat every 3rd turn\n");
            printf(" 4. Tiantui Star [天退星]\n When HP at 60%% or less HP, or if this unit is set to use a powerful attack ('Tanglecleaver') next turn for the second time this Encounter, if this unit is Staggered, recover from Stagger, activate 'Unrelenting Spirit [剛氣]', gain 10%% damage and 1 Final Power for every 20%% HP missing (Max 3 each), deal +1%% damage for every Sanity different between this unit and enemy (Max 20%%), All skills' breakable coin become unbreakable coin\n");
            printf(" 5. Lei Heng, The Pinky's Tiantui Star\nTurn End: at 40%% or less HP, or if this unit is set to use a powerful attack ('Tanglecleaver') next turn for the third time this Encounter, Replace the powerful attack 'Tanglecleaver' with 'Savage Tigerslayer's Perfected Flurry of Blades' and convert 'Inner Strength [底力]' to 'Extreme Strength [極力]', convert 'Unrelenting Spirit [剛氣]' to 'Unrelenting Spirit - Shin [剛氣-心]', gain 10%% damage and 1 Final Power for every 15%% HP missing (Max 5 each), deal +2%% damage for every Sanity different between this unit and enemy (Max 40%%)\n");
             printf(" 6. Chachihu [揷翅虎]\n Combat Start: At (50 - current Sanity)%% chance\n");
            printf(" - Randomly heal Sanity between 2-4. At less than 0 Sanity, double the heal amount, at more than 15 Sanity, does not activate");
            printf("\n - Randomly gains between 1-30%% Damage Up. At less than 0 Sanity, does not activate\n");
            printf(" At -45 Sanity, does not activate above effects\n");
            printf(" 7. Inner Strength [底力]\n When using Skills gain 'Inner Strength [底力]' which use for 'Tanglecleaver' and 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]'. Gain when Win Clash +(Clash Count) and +(Clash Count x 2) on 'Extreme Strength [極力]' and gain when attack +(Attack Coins x 2) and +(Attack Coins x 3) on 'Extreme Strength [極力]'\n");
            printf(" 8. Tiantui Star's Blade - Overheat\n When using Skill 'Tanglecleaver' or 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]' next turn equal to the number of times those skills were used (Max 5) (Clash Power -(Stack), Take (10 x Stackable)%% more damage) next turn\n");
            printf(" 9. Ten Blades of the East\n At the Turn Start of gaining a new pattern: "
              "\n - Gain 1 Severing Slash [切斬] (Target takes +50%% damage) for one turn"
              "\n - If this unit is Staggered, recover from Stagger"
              "\n - Heal 5 Sanity for every 10%% missing HP on self (Max 20)\n");
            printf(" 9. I'll be frank\n at 20%% or less HP, End the encounter, whatever enemy is at 0%% HP or not\n");
              } 
        else if (selected_enemy - 1 == 2) {
          //Taunt
          printf("\"We are not deserve to even breath...\"\n\n");

          //Description
          printf("A high-HP, high-Defense boss who focuses on building sanity through passive, and attack with powerful attack\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf(" 1. Heart of Vengeance\n If this unit is Panicked still can act and if not at -45 Sanity, Combat Start: heal 15 Sanity, after reset Sanity to 0 and gain 2 Final Power up\n");
           printf(" 2. Antagonism\n If the target's current HP is higher than this unit's (%%), Clash Power +2 and deal +20%% damage\n");
           printf(" 3. Long-awaited Moment\n When Clashing\n - Clash Win: Heal 5 Sanity on self \n - Clash Lose: Lose 5 Sanity on self and Gain 1 Final Power Down\n");
          printf(" 4. May She... Wake in Torment!\n Turn End: if this unit is at 70%% or less HP, or at the end of the 6nd turn, gain new pattern\n");
          printf(" 5. Withstand\n At 50 or less HP, Cap Hp to 50 and recover from 'Stagger' (Once per Encounter)\n");
          printf(" 6. Every Heathcliff Must Die...\n At 50 or less HP, fixed Speed to 1 and use 'Every Heathcliff Must Die...'\n");
            } 
        else if (selected_enemy - 1 == 3) {
          //Taunt
          printf("\"Know your place... Fool...\"\n\n");

          //Description
          printf("A boss who focus on dealing damage, which come up with great clash skills. He's boring\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf(" 1. Binding Vow - Open\n When dealing damage with Cursed Technique (excluding 'Cursed Technique - Fuga:Open [鐚]'), gains 'Binding Vow - Open' equal to (damage/4; rounded down), use for empower 'Cursed Technique - Fuga:Open [鐚]'\n");
           printf(" 2. The Strongest Of History\n Clash win: gain 5 Attack Power up next turn\n");
           printf(" 3. King\n If this unit is Panicked still can act, after reset Sanity to 0 and gain 2 Clash Power up next turn\n");
          printf(" 4. Chanting\n Turn Start: For every 3 Turns, this unit uses 'Chanting', if this unit is set to use 'Chanting' 3 times, next turn use 'World Cutting Slash', after use 'World Cutting Slash' gains 'Lost of Cursed Energy'\n");
          printf(" 5. Lost of Cursed Energy\n Gains 5 Final Power Down, take +30%% damage and deal -50%% damage. Turn End: Expire this effect\n");
           printf(" 6. Cursed Reverse Techinque\n In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. Turn End: Heal HP to Max HP and heal Sanity to 45 (Once per Encounter)\n");
          printf(" 7. Domain Expansion\n after use 'Cursed Reverse Techinque', use 'Domain Expansion:Malevolent Shrine'. When 'Domain Expansion:Malevolent Shrine' activated, activate below effect\n"
            " - Turn End: Deal fixed damage equal to (5%% of target's Max HP) to all enemies and gain 5 'Binding Vow - Open' (this 'Turn End' effect does activate repeatedly)\n"
            " - 'Cursed Technique - Fuga:Open [鐚]' gains +5 Base Power and convert to Unbreakable Coin\n");
            } 
        else if (selected_enemy - 1 == 4) {
          //Taunt
          printf("\"Dreaming end... so what?\"\n\n");

          //Description
          printf("A low-HP boss who along with great heal along with strong passive buff to Skills\n\n");

          //Passive
           printf("Passive Skills:\n");
           printf(" 1. Sancho:The Second Kindred of Don Quixote\n When HP reachs to 0 tranform into 'Sancho:The Second Kindred of Don Quixote'\n");
          printf(" 2. Hardblood\n In 'Sancho:The Second Kindred of Don Quixote' Phase: Start Combat: gain 1 'Hardblood' use for certain skills and buff, when using some certain skills gains some as well (Max 30; 'Hardblood' cannot drop below 1)"
              "\n - at 10+ 'Hardblood': Gain +1 Offense for every 5 Hardblood"
              "\n - at 20+ 'Hardblood': Gain +1 Offense and +1 Defense for every 5 Hardblood\n");
          printf(" 3. In Dreams\n In 'Sancho:The Second Kindred of Don Quixote' Phase: if this unit is Panicked still can act and after that reset Sanity to 0 and consumes 3 Hardblood to gains 3 Final Power Up and +30%% damage \n");
          printf(" 4. If we can be freed from this excruciating sickness\n In 'Sancho:The Second Kindred of Don Quixote' Phase: On Hit, heal 40%% of the damage dealt.\n"
            " - This Passive heals +1%% more HP for every missing HP on self (Max 20%%)\n"
            " - For Unbreakable Coins: this effect does not activate on Hit After Clash Lose.\n"
            "Every Turn Start: heal (percentage missing HP/2) HP. (Max 30)\n"
            "Every Turn Start: at -15 or less Sanity, consume (5 - current Sanity/5) Hardblood (Rounded down) to gain (percentage missing HP/2) Sanity and gain 2 Clash Power Up\n"
            "On Hit without Clash Lose, gain 3 Hardblood\n");
          printf(" 5. I'll Pierce You!\n In 'Sancho:The Second Kindred of Don Quixote' Phase: at HP 60%% or less HP, Use Variant Don Quixote Style: Sancho Arts 2 - La Sangre instead (Once per Encounter)\n");
          printf(" 6. End Dreams\n In 'Sancho:The Second Kindred of Don Quixote' Phase: at HP 40%% or less HP, Use La Aventura Ha Terminado instead (Once per Encounter), After Attack: when this unit loses Clash consumes 5 Hardblood to take -25%% damage and gain 25 Shield HP (Activates for the left of Encounter)\n");
            } else if (selected_enemy - 1 == 5) {
          //Taunt
          printf("\"I shall afford you neither the wherewithal nor the time to mask your ruminations. Bring forth a real answer; do not let it languish behind your tongue.\"\n\n");

          //Description
          printf("A strong boss that can with powerful attack, debuff and unable to beat... but he's not giving it his all.\n\n");
          
          //Passive
           printf("Passive Skills:\n");
           printf(" 1. Effloresced E.G.O::Érlì\n First Turn Start: heal 30 Sanity and gain 'A Sliver of Anticipation', He's not giving it his all\n");
          printf(" 2. A Sliver of Anticipation\n When possess, Lose 35 Offense and 35 Defense, Deal 30%% more damage\n");
          printf(" 3. Infinite Song of Erudition\n After Attack: heal 5 Sanity\n");
          printf(" 4. Panic Recovery\n Turn End: if in Panic, reset Sanity to 0\n");
           printf(" 5. I shall know your answer.\n At 85%% or less HP, then gain new pattern\n");
          printf(" 6. I still await your answer.\n At 60%% or less HP, apply 3 Dialogues to enemy. Dialogues: Turn End: heal 5 Sanity, When HP drop to 0 heal up to max HP; then lose 1 stack\n");
          printf(" 7. Do not fear the futility.\n At 40%% or less HP, Turn End: Cap HP to 40%% and recover from 'Stagger'; then use 'Like a Roaring Storm' at Turn Start\n");
          printf(" 8. perhaps they must be shaken afore you are to speak your truth.\n At 20%% or less HP, Turn End: Cap HP to 20%% and recover from 'Stagger'; then use a powerful attack 'Tiangang Star - Form (格)' at Turn Start\n");
            } else if (selected_enemy - 1 == 6) {
          //Taunt
          printf("\"Grand Welcome...\"\n\n");

          //Description
          printf("The King in Binds (O-01-20-12) is a WAW-class Abnormality, the boss without 'Sanity' that focus on decrease enemy Sanity, he's the king... and the king shall have a knight beside\n\n");

          //Passive
           printf("Passive Skills:\n");
           printf(" 1. The King Shall Have a Knight Beside\n Start Encounter: This unit Summons 'A Knight' that imitate from 'Enemy'. Turn Start: Start the 'Combat Event'\n");
          printf(" 2. Closing of the Banquet\n When 'A Knight' reaches 0 HP, Turn End: All enemy lose (5 x Turn used) Sanity (Max 45), Turn Start: Start the 'Combat Event'; then this unit joins the Battle\n");
          printf(" 3. Refracted Struggle\n On Clashing: This unit gain (Clash Count/2) in a Clash (Rounded down) (This effect activing repeating on new Clash Round)\n");
          printf(" 4. Snapping Bandages\n Turn End: at 20%% or less HP, apply 'Bandages of the King in Binds' on all enemies. Turn Start: Use a powerful attack 'Present Thyself Before the King', repeat every 4th turn\n");
          printf(" 5. Bandages of the King in Binds\n Turn End: Gain +2 Sinking Stack and +1 Sinking Count\n");
          printf(" 6. Thou Wilt Sink\n When attack with certain skills, inflict 'Sinking' based on skills used\n");
          printf(" 7. Bound by Guilt\n When attack with certain skills, inflict 'Tremor' or trigger 'Tremor Burst' based on skills used\n");
          printf(" 8. Sinking\n When Inflicted: At 1+ Count, or at 1+ Stack (If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Hit: Lose (Stack) Sanity (if this unit doesn't have Sanity, take (Stack) fixed damage instead). Then reduce 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
          printf(" 9. Tremor\n When Inflicted: At 1+ Count, or at 1+ Stack (If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), When Trigger by 'Tremor Burst', Take (Stack) Fixed Damage; then reduce 1 Count, if this unit took (Max HP/4) damage from 'Tremor Burst' in this Encounter, if this unit not on 'Stagger' state, enter 'Stagger' state (Cannot act for one turn) and reset this progess. Turn End: Lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
            } else {
          //Taunt
          printf("\"That's that, and this is this.\"\n\n");

          //Description
          printf("????????????????????????????????????????\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf(" 1. Agony\n Before Start Encounter: Lose 40 Offense and Lose 45 Defense, At 50%% or less HP, Gain 10 Offense and lose 5 Defense \n");
          printf(" 2. Black Heart\n If this unit is Panicked still can act, when lose clash heal Sanity instead, when win clash lose Sanity instead\n");
           printf(" 3. Vengeance For Nothing\n Every end of Turn 3rd, at 0+ Sanity, loses ((Further from 0 Sanity/2) + 5 for every 20%% missing HP) Sanity (Rounded down) and gain (3 + (1 for every 10%% missing HP)) Black Silence, at less than 0 Sanity, loses (5 + (5 for every 30%% missing HP)) Sanity and gain +2%% damage for every Black Silence next ture\n");
          printf(" 4. Black Silence\n When win clash with Skills gain 3 Black Silence and gain 1 when lose clash, use for certain Skills (Max 60)\n");
          printf(" 5. Furioso\n After all Skills except 'Furioso' had been used, recover from 'Stagger' and use 'Furioso' next turn\n");
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

  printf("You selected %s\n", enemyNames[selected_enemy - 1]);
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

    enemy.MAX_HP += 1000;
    enemy.HP += 1000;
    enemy.Sanity += 45;
    enemy.sanityLossBase = 3;
    enemy.Passive += 50;

    printf("\n%s gains +1000 Max HP, heals +45 Sanity and 50 Extreme Strength [極力] at start of the Encounter\n", enemy.name);

    sleep(1);

    printf("\n%s gains 'Unrelenting Spirit - Shin [剛氣-心]'\n", enemy.name);

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

    printf("\n%s gains 'Will of Prescript' (+3 Offense for this Encounter) and 'Deep Internal Conflict' (Damage Multiplier -0.2 for this Encounter)\n",
           player.name);

    for (int i = 0; i < player.numSkills; i++) {

      player.skills[i].Offense += 3;
       player.skills[i].DmgMutiplier -= 0.2;

    }

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
          enemySkillIndex = enemySkill1; // เก็บ index ไว้หลอกระบบเผื่อใช้เลื่อนคิวสกิล (แต่มักจะไม่ขยับ)

          // ถ้าเป็น Guard (Type 1) ให้ทอยโล่ทันที
          if (enemySkillEffective->skillType == 1) {
              defensePhase(&enemy, enemySkillEffective);
          }
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

        handleTurnStart(&player, &enemy, &enemySkillIndex, &playerSkill1, &playerSkill2, &enemySkill1, &enemySkill2);

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

      handleBeforeFight(&player, &enemy, &enemySkillIndex, playerSkill1, playerSkill2, enemySkill1, enemySkill2);
      handleBeforeFight(&enemy, &player, &enemySkillIndex, enemySkill1, enemySkill2, playerSkill1, playerSkill2);

      if (!IsenemyUnableToAct) {
      if (enemy.skills[enemySkillIndex].Unbreakable > 0 && (enemy.skills[enemySkillIndex].Clashable || enemy.skills[enemySkillIndex].skillType != 0)) {
        printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Unbreakable %d)\n",
          getSkillTypeName(enemy.skills[enemySkillIndex].skillType),
               enemy.skills[enemySkillIndex].name,
               enemy.skills[enemySkillIndex].BasePower,
               enemy.skills[enemySkillIndex].CoinPower,
               enemy.skills[enemySkillIndex].Coins,
               enemy.skills[enemySkillIndex].Offense + enemy.OffenseBoost,
               enemy.skills[enemySkillIndex].Defense + enemy.DefenseBoost,
               enemy.skills[enemySkillIndex].Unbreakable);
      } else if (enemy.skills[enemySkillIndex].Unbreakable <= 0 && (enemy.skills[enemySkillIndex].Clashable || enemy.skills[enemySkillIndex].skillType != 0)) {
        printf("\nEnemy uses %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Breakable)\n",
          getSkillTypeName(enemy.skills[enemySkillIndex].skillType),
               enemy.skills[enemySkillIndex].name,
               enemy.skills[enemySkillIndex].BasePower,
               enemy.skills[enemySkillIndex].CoinPower,
               enemy.skills[enemySkillIndex].Coins,
               enemy.skills[enemySkillIndex].Offense + enemy.OffenseBoost,
               enemy.skills[enemySkillIndex].Defense + enemy.DefenseBoost);
      } else if (enemy.skills[enemySkillIndex].Unbreakable > 0 && !enemy.skills[enemySkillIndex].Clashable && enemy.skills[enemySkillIndex].skillType == 0) {
        printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Unbreakable %d)\n",
          getSkillTypeName(enemy.skills[enemySkillIndex].skillType),
               enemy.skills[enemySkillIndex].name,
               enemy.skills[enemySkillIndex].BasePower,
               enemy.skills[enemySkillIndex].CoinPower,
               enemy.skills[enemySkillIndex].Coins,
               enemy.skills[enemySkillIndex].Offense + enemy.OffenseBoost,
               enemy.skills[enemySkillIndex].Defense + enemy.DefenseBoost,
               enemy.skills[enemySkillIndex].Unbreakable);
      } else if (enemy.skills[enemySkillIndex].Unbreakable <= 0 && (!enemy.skills[enemySkillIndex].Clashable && enemy.skills[enemySkillIndex].skillType == 0)) {
        printf("\nEnemy uses %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Breakable)\n",
          getSkillTypeName(enemy.skills[enemySkillIndex].skillType),
               enemy.skills[enemySkillIndex].name,
               enemy.skills[enemySkillIndex].BasePower,
               enemy.skills[enemySkillIndex].CoinPower,
               enemy.skills[enemySkillIndex].Coins,
               enemy.skills[enemySkillIndex].Offense + enemy.OffenseBoost,
               enemy.skills[enemySkillIndex].Defense + enemy.DefenseBoost);
      }
    }

    // Player picks one skill (only if can act)
    int playerSkillIndex;

    int playerTempOffense = 0, playerTempDefense = 0;
    int enemyTempOffense = 0, enemyTempDefense = 0;
    playerTempOffense += player.OffenseBoost;
    playerTempDefense += player.DefenseBoost;
    enemyTempOffense += enemy.OffenseBoost;
    enemyTempDefense += enemy.DefenseBoost;

    if (!IsplayerUnableToAct) {
      printf("\nDashboard Skills:\n");

        if (player.skills[playerSkill1].Unbreakable > 0 && player.skills[playerSkill1].Clashable) {
            printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
                  getSkillTypeName(player.skills[playerSkill1].skillType),
                   player.skills[playerSkill1].name,
                   player.skills[playerSkill1].BasePower,
                   player.skills[playerSkill1].CoinPower,
                   player.skills[playerSkill1].Coins,
                   player.skills[playerSkill1].Offense + player.OffenseBoost,
                   player.skills[playerSkill1].Defense + player.DefenseBoost,
                   player.skills[playerSkill1].Unbreakable);
        } else if (player.skills[playerSkill1].Unbreakable <= 0 && player.skills[playerSkill1].Clashable) {
            printf("1. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill1].skillType),
                   player.skills[playerSkill1].name,
                   player.skills[playerSkill1].BasePower,
                   player.skills[playerSkill1].CoinPower,
                   player.skills[playerSkill1].Coins,
                   player.skills[playerSkill1].Offense + player.OffenseBoost,
                   player.skills[playerSkill1].Defense + player.DefenseBoost);
        } else if (player.skills[playerSkill1].Unbreakable > 0 && !player.skills[playerSkill1].Clashable) {
            printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill1].skillType),
                   player.skills[playerSkill1].name,
                   player.skills[playerSkill1].BasePower,
                   player.skills[playerSkill1].CoinPower,
                   player.skills[playerSkill1].Coins,
                   player.skills[playerSkill1].Offense + player.OffenseBoost,
                   player.skills[playerSkill1].Defense + player.DefenseBoost,
                   player.skills[playerSkill1].Unbreakable);
        } else {
            printf("1. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill1].skillType),
                   player.skills[playerSkill1].name,
                   player.skills[playerSkill1].BasePower,
                   player.skills[playerSkill1].CoinPower,
                   player.skills[playerSkill1].Coins,
                   player.skills[playerSkill1].Offense + player.OffenseBoost,
                   player.skills[playerSkill1].Defense + player.DefenseBoost);
        }

        if (player.skills[playerSkill2].Unbreakable > 0 && player.skills[playerSkill2].Clashable) {
            printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill2].skillType),
                   player.skills[playerSkill2].name,
                   player.skills[playerSkill2].BasePower,
                   player.skills[playerSkill2].CoinPower,
                   player.skills[playerSkill2].Coins,
                   player.skills[playerSkill2].Offense + player.OffenseBoost,
                   player.skills[playerSkill2].Defense + player.DefenseBoost,
                   player.skills[playerSkill2].Unbreakable);
        } else if (player.skills[playerSkill2].Unbreakable <= 0 && player.skills[playerSkill2].Clashable) {
            printf("2. %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill2].skillType),
                   player.skills[playerSkill2].name,
                   player.skills[playerSkill2].BasePower,
                   player.skills[playerSkill2].CoinPower,
                   player.skills[playerSkill2].Coins,
                   player.skills[playerSkill2].Offense + player.OffenseBoost,
                   player.skills[playerSkill2].Defense + player.DefenseBoost);
        } else if (player.skills[playerSkill2].Unbreakable > 0 && !player.skills[playerSkill2].Clashable) {
            printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill2].skillType),
                   player.skills[playerSkill2].name,
                   player.skills[playerSkill2].BasePower,
                   player.skills[playerSkill2].CoinPower,
                   player.skills[playerSkill2].Coins,
                   player.skills[playerSkill2].Offense + player.OffenseBoost,
                   player.skills[playerSkill2].Defense + player.DefenseBoost,
                   player.skills[playerSkill2].Unbreakable);
        } else {
            printf("2. %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill2].skillType),
                   player.skills[playerSkill2].name,
                   player.skills[playerSkill2].BasePower,
                   player.skills[playerSkill2].CoinPower,
                   player.skills[playerSkill2].Coins,
                   player.skills[playerSkill2].Offense + player.OffenseBoost,
                   player.skills[playerSkill2].Defense + player.DefenseBoost);
        }

      // Next Skill
        if (player.skills[playerSkill3].Unbreakable > 0 && player.skills[playerSkill3].Clashable) {
            printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill3].skillType),
                   player.skills[playerSkill3].name,
                   player.skills[playerSkill3].BasePower,
                   player.skills[playerSkill3].CoinPower,
                   player.skills[playerSkill3].Coins,
                   player.skills[playerSkill3].Offense + player.OffenseBoost,
                   player.skills[playerSkill3].Defense + player.DefenseBoost,
                   player.skills[playerSkill3].Unbreakable);
        } else if (player.skills[playerSkill3].Unbreakable <= 0 && player.skills[playerSkill3].Clashable) {
            printf("Next Skill | %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill3].skillType),
                   player.skills[playerSkill3].name,
                   player.skills[playerSkill3].BasePower,
                   player.skills[playerSkill3].CoinPower,
                   player.skills[playerSkill3].Coins,
                   player.skills[playerSkill3].Offense + player.OffenseBoost,
                   player.skills[playerSkill3].Defense + player.DefenseBoost);
        } else if (player.skills[playerSkill3].Unbreakable > 0 && !player.skills[playerSkill3].Clashable) {
            printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
              getSkillTypeName(player.skills[playerSkill3].skillType),
                   player.skills[playerSkill3].name,
                   player.skills[playerSkill3].BasePower,
                   player.skills[playerSkill3].CoinPower,
                   player.skills[playerSkill3].Coins,
                   player.skills[playerSkill3].Offense + player.OffenseBoost,
                   player.skills[playerSkill3].Defense + player.DefenseBoost,
                   player.skills[playerSkill3].Unbreakable);
        } else {
            printf("Next Skill | %s: '%s' (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
              getSkillTypeName(player.skills[playerSkill3].skillType),
                   player.skills[playerSkill3].name,
                   player.skills[playerSkill3].BasePower,
                   player.skills[playerSkill3].CoinPower,
                   player.skills[playerSkill3].Coins,
                   player.skills[playerSkill3].Offense + player.OffenseBoost,
                   player.skills[playerSkill3].Defense + player.DefenseBoost);
        }


      // Defense Skill
      printf("\n");
      if (player.defenseSkill[playerDefenseSkill].Unbreakable > 0) {
        printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d)\n",
          getSkillTypeName(player.defenseSkill[playerDefenseSkill].skillType),
               player.defenseSkill[playerDefenseSkill].name,
               player.defenseSkill[playerDefenseSkill].BasePower,
               player.defenseSkill[playerDefenseSkill].CoinPower,
               player.defenseSkill[playerDefenseSkill].Coins,
               player.defenseSkill[playerDefenseSkill].Offense + player.OffenseBoost,
               player.defenseSkill[playerDefenseSkill].Defense + player.DefenseBoost,
               player.defenseSkill[playerDefenseSkill].Unbreakable);
      } else if (player.defenseSkill[playerDefenseSkill].Unbreakable <= 0) {
        printf("Defense Skill - %s: '%s' (BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable)\n",
          getSkillTypeName(player.defenseSkill[playerDefenseSkill].skillType),
               player.defenseSkill[playerDefenseSkill].name,
               player.defenseSkill[playerDefenseSkill].BasePower,
               player.defenseSkill[playerDefenseSkill].CoinPower,
               player.defenseSkill[playerDefenseSkill].Coins,
               player.defenseSkill[playerDefenseSkill].Offense + player.OffenseBoost,
               player.defenseSkill[playerDefenseSkill].Defense + player.DefenseBoost);
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

        // รันเหรียญป้องกัน
        if (player.defenseSkill[playerDefenseSkill].skillType == 1) {
        defensePhase(&player, playerSkillEffective);
        }

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

    playerSkillEffective =
        getEffectiveSkill(&player, &enemy, playerSkillEffective,
                          &playerTempOffense, &playerTempDefense);

    enemySkillEffective =
        getEffectiveSkill(&enemy, &player, &enemy.skills[enemySkillIndex],
                          &enemyTempOffense, &enemyTempDefense);

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

      } else {

        int canPlayerClash = (playerSkillEffective != NULL) && 
                             (pType == 0 || pType == 4 || pType == 5);
        int canEnemyClash  = (enemySkillEffective != NULL) && 
                             (eType == 0 || eType == 4 || eType == 5);

        int willClash = (playerSkillEffective != NULL && enemySkillEffective != NULL) &&
                        playerSkillEffective->Clashable && 
                        enemySkillEffective->Clashable && 
                        canPlayerClash && canEnemyClash;

         if (!willClash) {

           if (pType != 3 || eType != 3) {
          if (playerGoesFirst == 1) {
            
            if (playerSkillEffective != NULL && (playerSkillEffective->skillType == 0 || playerSkillEffective->skillType == 3)) {
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

            if (enemySkillEffective != NULL && (enemySkillEffective->skillType == 0 || enemySkillEffective->skillType == 3)) {
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
          enemy.Tremor[2] += clash.playerFinalPower;
          printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                  player.name, player.name, enemy.name, clash.playerFinalPower);
          sleep(1);
          if (enemy.Tremor[2] > 50 && enemy.Stagger <= 0) {
            enemy.Stagger += 2;
            printf("\n%s Staggered for one turn\n", enemy.name);
            sleep(1);
            enemy.Tremor[2] = 0;
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
          player.Tremor[2] += clash.enemyFinalPower;
          printf("\n%s won the Clash, %s's Guard increases %s's Stagger Threshold by %d!\n",
                  enemy.name, enemy.name, player.name, clash.enemyFinalPower);
          sleep(1);
          if (player.Tremor[2] > 50 && player.Stagger <= 0) {
            player.Stagger += 2;
            printf("\n%s Staggered for one turn\n", player.name);
            sleep(1);
            player.Tremor[2] = 0;
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

  printf("\n--- Battle Result ---\n");
  if (player.HP <= 0 && enemy.HP <= 0) {
    printf("It's a draw!\n");
  } else if (player.HP <= 0) {
    printf("You lost! %s defeated you.\n", enemy.name);
  } else if (enemy.HP <= 0) {
    printf("Victory! You defeated %s.\n", enemy.name);
  }

  return 0;
}   // closes else block
}   // closes main
