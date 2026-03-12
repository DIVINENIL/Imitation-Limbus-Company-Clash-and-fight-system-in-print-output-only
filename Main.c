#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_SKILLS 20

int OLD_SANITYP1;
int OLD_SANITYP2;

int TurnCount = 1;

int ClashPity = 1;

typedef struct {
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
} SkillStats;

typedef struct {
  const char *name;
  SkillStats skills[MAX_SKILLS];
  int numSkills;
  float HP;
  float MAX_HP;
  float Shield;
  int Passive;
  int Sanity;         // Add this: -45 to 45
  int hasSanity;      // Add this: 1 = has sanity system, 0 = immune to sanity
  int sanityGainBase; // Base sanity gain on clash win (default 10)
  int sanityLossBase; // Base sanity loss on clash loss (default 8)
  int immuneToPanicSkip; // Add this: 1 = still acts when panicked, 0 = normal
                         // panic
 int SanityFreezeTurns; // 0 = no lock, >0 = locked for this many turns, -1 Can't Snap out of Panic

  // Buff
  int CoinPowerBoost;
  int FinalPowerBoost;
  int AttackPowerBoost;
  int BasePowerBoost;
  int ClashPower;
  float DamageUp;
  float Protection;
  int Paralyze;
  float DmgMutiplierBoost;
  int OffenseBoost;
  int DefenseBoost;

  // BuffNextTurn
  int CoinPowerBoostNextTurn;
  int FinalPowerBoostNextTurn;
  int AttackPowerBoostNextTurn;
  int BasePowerBoostNextTurn;
  int ClashPowerNextTurn;
  float DamageUpNextTurn;
  float ProtectionNextTurn;
  int ParalyzeNextTurn;
  float DmgMutiplierBoostNextTurn;
  int OffenseBoostNextTurn;
  int DefenseBoostNextTurn;

} Character;

typedef struct {
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
} ClashResult;

//---------------------Buff system-----------------
void initializeCharacterBuffs(Character *c) {
  // Current turn buffs
  c->CoinPowerBoost = 0;
  c->FinalPowerBoost = 0;
  c->AttackPowerBoost = 0;
  c->BasePowerBoost = 0;
  c->ClashPower = 0;
  c->DamageUp = 0;
  c->Protection = 0;
  c->Paralyze = 0;
  c->DmgMutiplierBoost = 0;
  c->OffenseBoost = 0;
  c->DefenseBoost = 0;

  c->SanityFreezeTurns = 0;

  // Next turn buffs
  c->CoinPowerBoostNextTurn = 0;
  c->FinalPowerBoostNextTurn = 0;
  c->AttackPowerBoostNextTurn = 0;
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
  c->CoinPowerBoost = 0;
  c->FinalPowerBoost = 0;
  c->AttackPowerBoost = 0;
  c->BasePowerBoost = 0;
  c->ClashPower = 0;
  c->DamageUp = 0;
  c->Protection = 0;
  c->Paralyze = 0;
  c->DmgMutiplierBoost = 0;
  c->OffenseBoost = 0;
  c->DefenseBoost = 0;

    c->CoinPowerBoost = c->CoinPowerBoostNextTurn;
    c->FinalPowerBoost = c->FinalPowerBoostNextTurn;
    c->BasePowerBoost = c->BasePowerBoostNextTurn;
    c->AttackPowerBoost = c->AttackPowerBoostNextTurn;
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

//--------------------------Sanity functions----------------------------

// Helper to handle Sanity changes, locking, and clamping
void updateSanity(Character *c, int delta) {
  if (!c->hasSanity) return;

  // 1. Handle Healing Lock
  if (delta > 0) {
    if (c->SanityFreezeTurns > 0) {
      // Optional: Print message or just ignore silently
      return; // Block the heal
    }
  }

  // 2. Apply Change
  c->Sanity += delta;

  // 3. Clamp Sanity (-45 to 45)
  if (c->Sanity > 45) c->Sanity = 45;
  if (c->Sanity < -45) c->Sanity = -45;
  // 4. Trigger Lock if Sanity dropped to -45 (and wasn't already locked)
  if (c->Sanity <= -45 && c->SanityFreezeTurns == 0) {
    c->SanityFreezeTurns = 2; // Locks for the rest of this turn + the next turn
  }
}

// Check if character should skip turn due to panic
int isPanicked(Character *c) {
  if (c->hasSanity == 0)
    return 0; // No sanity = no panic
  if (c->immuneToPanicSkip)
    return 0; // Immune to panic skip
  if (c->Sanity <= -45) {
    printf("\n%s is in PANIC and cannot act!\n", c->name);
  }
  return (c->Sanity <= -45); // Normal panic check
}

// Get Sanity status message
const char *getSanityStatus(Character *c) {
  if (c->Sanity <= -45) {
    if (strcasecmp(c->name, "Lei heng") == 0) return "Beastly Instinct"; 
  if (strcasecmp(c->name, "Erlking Heathcliff") == 0) return "Revenge"; 
     if (strcasecmp(c->name, "Sukuna:King of Curse") == 0) return "King"; 
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0) return "Reawakening Joy Of Carnage"; 
   if (strcasecmp(c->name, "Fixer grade 9?") == 0) return "Black Heart"; 
    if (strcasecmp(c->name, "Jia Qiu") == 0) return "Jia Qiu"; 

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
    if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0) {
      c->sanityGainBase = 6;

    } else if (strcasecmp(c->name, "Erlking Heathcliff") == 0) {

      updateSanity(c, 15);
      printf("\n%s heals 15 Sanity (%d)\n", c->name, c->Sanity);
      sleep(1);
    } else if (strcasecmp(c->name, "Fixer grade 9?") == 0) {
      c->sanityLossBase = -7;
      
    }
    
  }
  
  // LOW MORALE (-30 to -44 Sanity)
  if (c->Sanity <= -30 && c->Sanity > -45) {
    if (strcasecmp(c->name, "Lei heng") == 0) {

      c->DamageUp += 10;
      c->FinalPowerBoost += 1;
      c->Protection -= 10;

      printf("\nWhile 'Low Morale', %s gains Final Power +1, deal 10%% more "
             "damage, take 10%% more damage.\n",
             c->name);
      sleep(1);

    } else if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0) {
      *offense += 2;
      c->sanityGainBase = 10;

      printf(
          "\nWhile 'Low Morale', %s gains Offense +2, Sanity Heal Efficiency +4.\n",
          c->name);
      sleep(1);

    } else if (strcasecmp(c->name, "Erlking Heathcliff") == 0) {
      c->DamageUp += 30;
      *defense -= 3;
      updateSanity(c, 15);

      printf("\n%s heals 15 Sanity\n", c->name);

      sleep(1);
      
      printf("\nWhile 'Low Morale', %s gains 30%% more damange, Defense -3.\n",
             c->name);
      sleep(1);
    } else if (strcasecmp(c->name, "Fixer grade 9?") == 0) {
      c->ClashPower += 2;
      *defense -= 3;
      c->sanityGainBase = -12; // Negative gain = loss on win

      printf("\nWhile 'Low Morale', %s gains 2 Clash Power Up, Defense -3, Sanity Loss Efficiency +6.\n",
             c->name);
      sleep(1);
    } else if (strcasecmp(c->name, "Jia Qiu") == 0) {
      
      c->DamageUp += 10;

      printf(
          "\nWhile 'Low Morale', %s deals 10%% more damange.\n",
          c->name);
      sleep(1);

    } else if (strcasecmp(c->name, "Sukuna:King of Curse") == 0) {

      c->DamageUp += 30;

      printf(
          "\nWhile 'Low Morale', %s deals 30%% more damange.\n",
          c->name);
      sleep(1);

    }
    
  }

  // PANIC (-45 Sanity)
  if (c->Sanity <= -45) {
    if (strcasecmp(c->name, "Lei heng") == 0) {

      c->DamageUp += 20;
      c->CoinPowerBoost += 2;
      c->Protection -= 20;

      printf("\nWhile 'Beastly Instinct', %s gains Coin Power +2, deal 20%% more damage, "
             "take 20%% more damage.\n",
             c->name);

      sleep(1);

    } else if (strcasecmp(c->name,
                          "Sancho:The Second Kindred of Don Quixote") == 0) {
      *offense += 3;
      *defense += 6;

      printf("\nWhile 'Reawakening Joy Of Carnage', %s gains Offense +3 and Defense +6.\n", c->name);
      sleep(1);

    } else if (strcasecmp(c->name, "Erlking Heathcliff") == 0) {

      c->DamageUp += 50;
      *defense -= 6;

      printf("\nWhile 'Revenge', %s gains 50%% more damage and Defense -6.\n",
             c->name);
      sleep(1);
    } else if (strcasecmp(c->name, "Fixer grade 9?") == 0) {

      c->DamageUp += 30;
      *defense -= 5;
      c->AttackPowerBoost += 3;

      printf("\nWhile 'Black Heart', %s gains 30%% more damange, 3 Attack Power Up, Defense -5.\n",
             c->name);
      sleep(1);
    } else if (strcasecmp(c->name, "Jia Qiu") == 0) {

      c->DamageUp += 20;

      printf("\nWhile 'Jia Qiu', %s deals 20%% more damage.\n",
             c->name);
      sleep(1);
    } else if (strcasecmp(c->name, "Sukuna:King of Curse") == 0) {

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
    if (c->skills[i].Copies > 0)
      pool[count++] = i;
  }

  if (count == 0) {
      *s1 = *s2 = *s3 = -1; // fallback
    return;
  }

  // Copies = 0 delected skills Copies > 0 skills normal Copies < 0 keep but no more in skill pool

  if (lastUnused >= 0 && lastUnused < maxSkills && c->skills[lastUnused].Copies != 0) {
    *s1 = lastUnused;
    // s2 can now be any skill, including lastUnused
    if (c->skills[*s2].Copies != 0) {
      *s2 = *s3;
    } else {
      *s2 = pickSkill(pool, count, c);
    }
    *s3 = pickSkill(pool, count, c);
  } else {
    *s1 = pickSkill(pool, count, c);
    *s2 = pickSkill(pool, count, c);
    *s3 = pickSkill(pool, count, c);
  }
}










// Modified when gained new pattern
void GainNewPattern(Character *c, Character *c2) {

  if (strcasecmp(c->name, "Lei heng") == 0) {
    
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
















// ----------------------Attack phase-------------------------------
void attackPhase(Character *attacker, SkillStats *atk, int atkTempOffense,
                 int atkTempDefense, Character *defender, SkillStats *defSkill,
                 int defTempOffense, int defTempDefense, int remainingCoins,
                 int Unbreakable, int clashCount) {
  // printf("\n%s attacks %s with %s\n", attacker->name, defender->name,
  // atk->name);

  if (attacker->HP > 0) {

  if (remainingCoins <= 0) {
    printf("\nNo coins left to attack.\n");
    return;
  }

    int ClashLostAttack = 0; // ← Character's clash lost coins in after attack

    if (atk->Unbreakable > 0 && Unbreakable == atk->Unbreakable) {
      ClashLostAttack = 1;
    }

  //---------------------------Before Attack Buff----------------------------

    // Sukuna:King of Curse Chanting
    if (strcasecmp(attacker->name, "Sukuna:King of Curse") == 0 && atk == &attacker->skills[4]) {

      attacker->skills[4].active++; // Chant Count

    }

    // Sukuna:King of Curse - Taunt Skill 6
    if (strcasecmp(attacker->name, "Sukuna:King of Curse") == 0 && (atk == &attacker->skills[6])) {

      printf("\n%s: Domain Expansion.......... Malevolent Shrine\n", attacker->name);

      sleep(1);
    }

    // ------------- Don Quixote:The Manager of La Manchaland -------------------
    
    // Don Quixote:The Manager of La Manchaland - Tanut
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[3] )) {

      printf("\n%s: Variant Sancho Hardblood Arts 6th... Tear Apart!\n", attacker->name);

      sleep(1);
      
    }

    // Don Quixote:The Manager of La Manchaland - Tanut
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[4] )) {

      printf("\n%s: Variant Sancho Hardblood Arts 8th... Split apart!\n", attacker->name);

      sleep(1);

    }

    // Don Quixote:The Manager of La Manchaland - Tanut
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[7] )) {

      printf("\n%s: Hardblood Arts 15th... Building up to the finale!\n", attacker->name);

      sleep(1);

    }

    // Don Quixote:The Manager of La Manchaland - Tanut
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[5] )) {

      printf("\n%s: Ascendant Sancho Hardblood Arts... La Sangre.\n", attacker->name);

      sleep(1);

    }

    // --------------------------------------------------------------

  // Heishou Pack - You Branch Adept Heathcliff - Battleblood Instinct buff
  if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && attacker->Passive > 0) {

    float gain = attacker->Passive * 0.75f;

      attacker->DamageUp += gain;

      printf("\n%s deals +0.75%% damage(%.2f%%) for every Battleblood Instinct Stack (%d)\n", attacker->name, gain, attacker->Passive);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Taunt Skill 3
  if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2])) {

    printf("\n%s: Bloodtalons... conflagrate!\n", attacker->name);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Taunt Skill 4
  else if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[3])) {

    printf("\n%s: Bloodtalons... conflagrate!\n", attacker->name);

    sleep(1);

    printf("\n%s: Hahahahahahaha!! Rooster’s Rampaging Blades Under the Ensanguined Heaven [血天下雞舞亂刀]!!\n", attacker->name);

    sleep(1);
  }

  // Binah lost
  if (strcasecmp(defender->name, "Binah") == 0 && ClashLostAttack && defender->Passive) {
    
      defender->Protection += 80;

    int shieldGain = 50 + ((defender->MAX_HP - defender->HP)/3);
    if (shieldGain > 100) shieldGain = 100;

    defender->Shield += shieldGain;

    printf("\n%s getting attack by Full Clash Lost Coins, take -80%% damage and gain (50 + Missing HP/3) (%d - Max 100) Shield HP (%.2f)\n", defender->name, shieldGain, defender->Shield);
  }
    else if (strcasecmp(defender->name, "Binah") == 0 && defender->Passive && defender->Sanity >= 0) {

    updateSanity(defender, -10);

    int shieldGain = 100 + ((defender->MAX_HP - defender->HP)/2);
    
      defender->Shield += shieldGain;

    printf("\n%s at 0+ Sanity, consumes 10 Sanity(%d) to gain (100 + Missing HP/2) (%d) Shield HP (%.2f)\n", defender->name, defender->Sanity, shieldGain, defender->Shield);
  }

    // ---------------------------------- Dawn Office Fixer Sinclair ----------------------------------------

  // Dawn Office Fixer Sinclair - Skill Buff base form
  if (strcasecmp(attacker->name, "Dawn Office Fixer Sinclair") == 0 && !attacker->skills[3].active && attacker->Sanity >= 45) {

    attacker->FinalPowerBoost += 3;

    printf("\n%s at 45 Sanity, gains 3 Final Power\n",
           attacker->name);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff Ego form
    if (strcasecmp(attacker->name, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && attacker->Sanity >= 45) {

      attacker->CoinPowerBoost += 3;

      printf("\n%s at 45 Sanity, gains 3 Coin Power\n",
             attacker->name);

      sleep(1);

    }

  // Dawn Office Fixer Sinclair - Skill Buff base form S2 45 sp
  if (strcasecmp(attacker->name, "Dawn Office Fixer Sinclair") == 0 && !attacker->skills[3].active && attacker->Sanity >= 45 && (atk == &attacker->skills[1])) {

    attacker->AttackPowerBoost += 3;

    printf("\n%s at 45 Sanity, gains 2 Attack Power\n",
           attacker->name);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S4
  if (strcasecmp(attacker->name, "Dawn Office Fixer Sinclair") == 0 && (atk == &attacker->skills[3] || atk == &attacker->skills[2]) && attacker->skills[3].active) {

      int boost = (abs(attacker->Sanity)) * 5;

      attacker->DamageUp += boost;

      printf("\n%s gains +5%% Damage (%d%%) for every Sanity further from 0 (%d Sanity)\n",
             attacker->name, boost, attacker->Sanity);

      sleep(1);

  }

    // Dawn Office Fixer Sinclair - Tanut
    if (strcasecmp(attacker->name, "Dawn Office Fixer Sinclair") ==
                   0 &&
               (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && !attacker->skills[3].active) {

      printf("\n%s: I'll carve this stigma... into you!\n", attacker->name);

      sleep(1);

    }

    // Dawn Office Fixer Sinclair - Tanut
    if (strcasecmp(attacker->name, "Dawn Office Fixer Sinclair") ==
                   0 &&
               (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && attacker->skills[3].active) {

      printf("\n%s: Burn in this passion.\n", attacker->name);

      sleep(1);

    }

    // ------------------------------------------------------------------------

  // Meursault:The Thumb Unbreakable BUff
  if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 && (attacker->skills[2].active >= 14) && attacker->Passive <= 0 && attacker->skills[3].active && (Unbreakable == atk->Coins)) {

    float missing = (attacker->MAX_HP - attacker->HP) / attacker->MAX_HP; // fraction of HP missing (0.0 - 1.0)
    int SkillUp = ((int)(missing * 100.0f)) + 75;  // 75% + missing
    if (SkillUp > 150) SkillUp = 150;      // cap at 50%

      attacker->DamageUp += SkillUp;

       printf("\n%s at 14+ Rounds spent: On Clash Lose, Unbreakable Coins of this unit's Attack Skills deal +(75 + missing HP percentage on self)%% damage(%d%% - Max 150%%)\n", attacker->name, SkillUp);

    sleep(1);

  }

    // Meursault:The Thumb 20+ BUff
    if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 && (attacker->skills[2].active >= 14) && attacker->Passive <= 0 && attacker->skills[3].active) {
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
  if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[10] || atk == &attacker->skills[3] || atk == &attacker->skills[6])) {

    int boost = rand() % 30 + 1;
    
      attacker->DamageUp += boost;

    printf("\n%s deals %d%% more damage\n", attacker->name, boost);

    sleep(1);
  }

  // Jia Qiu - Taunt S4
  if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[4] || atk == &attacker->skills[11])) {

    printf("\n%s: Cut them Down, Mao.\n", attacker->name);

    sleep(1);
  }

  // Jia Qiu - Taunt S12
  if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[12])) {

    printf("\n%s: Heed me, Zilu.\n", attacker->name);

    sleep(1);
  }

  // Jia Qiu - Taunt S15
  if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[15])) {

    printf("\n%s: Is it your companions who hold your tongue? Then... perhaps they must be shaken afore you are to speak your truth.\n", attacker->name);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
     (atk == &attacker->skills[2])) {

    int gain = 12 * attacker->Passive;

    attacker->DamageUp += gain;

    printf("\n%s deals 12%% more damage(%d%%) for every Coffin (%d)\n",
       attacker->name, gain, attacker->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
     (atk == &attacker->skills[3])) {

    int gain = 10 * attacker->Passive;

    attacker->DamageUp += gain;

    printf("\n%s deals 10%% more damage(%d%%) for every Coffin (%d)\n",
       attacker->name, gain, attacker->Passive);

      sleep(1);
  }

    // Heathcliff:Wild Hunt – buff Dullahan
    if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
       (atk == &attacker->skills[3])) {

      int gain = 20 * attacker->skills[0].active;

      attacker->DamageUp += gain;

      printf("\n%s deals 20%% more damage(%d%%) for every Dullahan (%d)\n",
         attacker->name, gain, attacker->skills[0].active);

        sleep(1);
    }

  // FireFist – Attack buff s1
  if (strcasecmp(attacker->name, "Gregor:Firefist") == 0 && atk == &attacker->skills[0]) {

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
if (strcasecmp(attacker->name, "Gregor:Firefist") == 0 && atk == &attacker->skills[1]) {

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
if (strcasecmp(attacker->name, "Gregor:Firefist") == 0 && (atk == &attacker->skills[2])) {

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
  if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
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

   //----------------------- Lei heng ---------------------
  // Lei heng – skill 4
  if (strcasecmp(attacker->name, "Lei heng") == 0 &&
      (atk == &attacker->skills[3])) {

    printf("\n%s: Eyes up here, boys! Don'tcha go losin' yer heads now!\n",
           attacker->name);

    sleep(1);
  }

  // Lei heng – skill 6
  if (strcasecmp(attacker->name, "Lei heng") == 0 &&
      (atk == &attacker->skills[5])) {

    printf("\n%s: Y'all don't go on huntin' tigers without preparin' yerselves "
           "to get chomped 'tween one of them jaws!\n",
           attacker->name);

    sleep(1);
  }
 
  //-----------------------------------------

  // Meursault:Blade Lineage Mentor - Yield my flesh
  if (strcasecmp(attacker->name, "Meursault:Blade Lineage Mentor") == 0 &&
      (atk == &attacker->skills[2])) {

      int DamageBuff = 2 * (abs(attacker->Sanity)) > 75 ? 75 : 5 * (abs(attacker->Sanity));

    attacker->DamageUp += DamageBuff;

      printf("\n%s deals 2%% more damage for further Sanity from 0 (%d%% - Max 75%%)\n", attacker->name, DamageBuff);

      sleep(1);
    
  }

  // Meursault:Blade Lineage Mentor - To claim thier bones
  if (strcasecmp(attacker->name, "Meursault:Blade Lineage Mentor") == 0 &&
      (atk == &attacker->skills[3] || atk == &attacker->skills[2])) {

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

  // Yi sang:Fell Bullet - Torn Memory
  if (strcasecmp(attacker->name, "Yi sang:Fell Bullet") == 0 &&
      (atk == &attacker->skills[0] && attacker->skills[0].active ||
       atk == &attacker->skills[1] && attacker->skills[1].active)) {

    printf("\n%s deals more +15%% damage(%d%%) for every Torn Memory(%d)\n",
           attacker->name, attacker->Passive * 15, attacker->Passive);

    sleep(1);
  }
  if (strcasecmp(attacker->name, "Yi sang:Fell Bullet") == 0 &&
      atk == &attacker->skills[2] && attacker->skills[2].active) {

    printf("\n%s deals more +30%% damage(%d%%) for every Torn Memory(%d)\n",
           attacker->name, attacker->Passive * 30, attacker->Passive);

    sleep(1);
  }

    // Hong lu:The Lord of Hongyuan - Skill 3 deal more damage on HP
    if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[4] || atk == &attacker->skills[3])) {

      float missingSelf  = (float)(attacker->MAX_HP  - attacker->HP)  / attacker->MAX_HP * 100.0f;
      float missingEnemy = (float)(defender->MAX_HP - defender->HP) / defender->MAX_HP * 100.0f;

      float damageboost = (missingSelf + missingEnemy) / 1.0f;
      if (damageboost > 50.0f) damageboost = 50.0f;

      attacker->DamageUp += damageboost;

        printf("\n%s deals +1%% damage for every 1%% (missing HP percentage on target + missing HP percentage on self) (%.0f%% - Max 50%%)\n", attacker->name, damageboost);

      sleep(1);
    }

  // Hong lu:The Lord of Hongyuan - S3 Buff
  if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 &&
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
    if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[3] || atk == &attacker->skills[4])) {

      float missingSelf  = (float)(attacker->MAX_HP  - attacker->HP)  / attacker->MAX_HP * 100.0f;
      float missingEnemy = (float)(defender->MAX_HP - defender->HP) / defender->MAX_HP * 100.0f;

      int Boost = abs((int)(missingSelf - missingEnemy)) / 6;
      if (Boost > 3) Boost = 3;

      if (Boost > 0) {

      attacker->CoinPowerBoost += Boost;

        printf("\n%s gains +1 Coin Power for every 6%% HP different (%d - Max 3)\n", attacker->name, Boost);

      sleep(1);

      }
    }

    // Sancho:The Second Kindred of Don Quixote - Heal mechnics
   if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") == 0 && Unbreakable <= 0) {

    int healvalue = 40;

     int missingHP = (int)(((attacker->MAX_HP - attacker->HP) / attacker->MAX_HP) * 100);
      if (missingHP > 20) missingHP = 20;

     if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
      (atk == &attacker->skills[10] || atk == &attacker->skills[11] || atk == &attacker->skills[12] || atk == &attacker->skills[13])) {
      healvalue += 100;
     }
     
     healvalue += missingHP;
     
     printf("\nOn Hit with this Skill: heal %d%% of the HP damage dealt\n", healvalue);

     sleep(1);
     
   }  // Sancho:The Second Kindred of Don Quixote - certain heal skill
     else if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") ==
             0 &&
         (atk == &attacker->skills[10] || atk == &attacker->skills[11] || atk == &attacker->skills[12] || atk == &attacker->skills[13])) {

       printf("\nOn Hit with this Skill: heal 100%% of the HP damage dealt\n");

       sleep(1);
     }

  // Don Quixote:The Manager of La Manchaland and Sancho - Heal mechnics
  if ((strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
           0 &&
       (atk == &attacker->skills[0] || atk == &attacker->skills[1] ||
        atk == &attacker->skills[4]))) {

    printf("\nOn Hit with this Skill: heal 30%% of the HP damage dealt (Max "
           "10)\n");

    sleep(1);
  } else if ((strcasecmp(attacker->name,
                         "Don Quixote:The Manager of La Manchaland") == 0 &&
              (atk == &attacker->skills[2]))) {

    printf("\nOn Hit with this Skill: heal 50%% of the HP damage dealt (Max 10)\n");

    sleep(1);
  } else if (strcasecmp(attacker->name,
                        "Don Quixote:The Manager of La Manchaland") == 0 &&
             (atk == &attacker->skills[3] || atk == &attacker->skills[5])) {

    printf("\nOn Hit with this Skill: heal 50%% of the HP damage dealt (Max "
           "20)\n");

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Ult skill 12
  if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
      atk == &attacker->skills[12]) {

    printf("\n%s: I'll pierce you!\n", attacker->name);

    sleep(1);

  }
    
  // Sancho:The Second Kindred of Don Quixote - Ultimate
  if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
      atk == &attacker->skills[13]) {

    if (Unbreakable == attacker->skills[13].Coins) {
      printf("\n%s Clash lost, Reduce 50%% damage\n", attacker->name);
      attacker->DamageUp -= 50;
    }

    sleep(1);

    if (strcasecmp(defender->name, "Don Quixote:The Manager of La Manchaland") == 0) { 
      printf("\n%s: No matter what... No matter how many times... I'll still go for our dream!!!\n", attacker->name); 
    } else { 
      printf("\n%s: You dream too! Will end...\n", attacker->name); 
    }

    sleep(1);
  }

  
  // Hong lu:The Lord of Hongyuan - Lordsguard
  const char *HeshinPacks = NULL;
  if (strcasecmp(defender->name, "Hong lu:The Lord of Hongyuan") == 0 &&
      defender->skills[5].active == 0 &&
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
      printf("\n%s: Protect Hongyuan as ordered. (Heshin Packs - %s uses "
             "Lordsguard)\n",
             defender->name, HeshinPacks);

      sleep(1);

      printf("\n%s attacks 'Heshin Packs - %s' instead!\n", attacker->name,
             HeshinPacks);
    }
  }

    // Roland – Mang (心)
    if (strcasecmp(attacker->name, "Fixer grade 9?") == 0 && strcasecmp(defender->name, "Binah") == 0 && attacker->Sanity >= 0 && defender->Passive == 1) {

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
  if (strcasecmp(attacker->name, "Fixer grade 9?") ==
          0 &&
      atk == &attacker->skills[9]) {

    if (ClashLostAttack) {
      printf("\n%s lost the Clash, Reduce 50%% damage\n", attacker->name);
      attacker->DamageUp -= 50;

       sleep(1);
    }

    printf("\n%s consumes all Black Silence and gain +5%% Damage (%d%%) for every 3 Black Silence(%d)\n", attacker->name, 5*(attacker->Passive / 3), attacker->Passive);

    attacker->Passive = 0;
    attacker->DamageUp += 5*(attacker->Passive / 3);

    sleep(1);

     printf("\n%s: Gone Angle...\n", attacker->name);

    sleep(1);
  }

  //----------------------------------------------------------------




  

  printf("\n--- Attack Phase ---\n");
  int totalPower = atk->BasePower + attacker->BasePowerBoost;
  int totalDamage = 0;

  printf("%s starts attack with %s\n", attacker->name, atk->name);
  
  if (atk->Unbreakable > 0 && atk->Coins != atk->Unbreakable) {
    // Condition 1: Some breakable and some unbreakable coins remaining
    if (Unbreakable > 0) {
        // Output with Clash lost coins details
        printf("Tossing %d coins for attack (from remaining clash %d breakable "
               "coins and %d Unbreakable coins (%d Clash lost coins, Halve those coins's Damage)):\n",
               remainingCoins, remainingCoins - atk->Unbreakable, atk->Unbreakable, Unbreakable);
    } else {
        // Output without Clash lost coins details
        printf("Tossing %d coins for attack (from remaining clash %d breakable "
               "coins and %d Unbreakable coins):\n",
               remainingCoins, remainingCoins - atk->Unbreakable, atk->Unbreakable);
    }
  } else if (atk->Unbreakable > 0 && atk->Coins == atk->Unbreakable) {
    // Condition 2: Only unbreakable coins remaining
    if (Unbreakable > 0) {
        // Output with Clash lost coins details
        printf("Tossing %d coins for attack (from remaining clash %d Unbreakable "
               "coins (%d Clash lost coins, Halve those coins's Damage)):\n",
               remainingCoins, atk->Unbreakable, Unbreakable);
    } else {
        // Output without Clash lost coins details
        printf("Tossing %d coins for attack (from remaining clash %d Unbreakable "
               "coins):\n",
               remainingCoins, atk->Unbreakable);
    }
  } else {
    // Condition 3: Only breakable coins remaining (atk->Unbreakable is 0 or less)
    printf("Tossing %d coins for attack (from remaining clash %d Breakable "
           "coins):\n",
           remainingCoins, remainingCoins);
  }
  
  printf("%-10s %-10s %-10s", "Coin", "Power", "Damage");

  for (int i = 0; i < remainingCoins; i++) {

    if (tossCoinWithSanity(attacker)) {
      // Check paralyze
      if (attacker->Paralyze > 0) { // ← Character's paralyze
        totalPower += 0;
        attacker->Paralyze--; // ← Character's paralyze
      } else {
        totalPower += atk->CoinPower + attacker->CoinPowerBoost;
        if (totalPower <= 0) totalPower = 0;
      }

      // ------------------ On Head hit ------------------------

      
      // Heishou Pack - You Branch Adept Heathcliff Skill 3 burn
      if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i != remainingCoins - 1 && attacker->HP >= attacker->MAX_HP*0.5) {

        if (defender->Shield > 0) {
          defender->Shield -= attacker->skills[0].active;
            if (defender->Shield < 0) {
                // คำนวณส่วนที่ทะลุเกราะ
                defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                defender->Shield = 0;             // เกราะหมดแล้ว
            }
        } else {
            defender->HP -= attacker->skills[0].active;
        }
        
        if (attacker->HP < 1)
            attacker->HP = 1;

           printf("\n%s Coins On Head Hit, at 50%%+ HP, take Burn damage by Burn Stack on self (%d)", attacker->name,attacker->skills[0].active);

      }

      // Heathcliff:Wild Hunt - Tanut Skill 3-1
      if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") ==
                     0 &&
                 (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

        printf("\n\n%s: Remain buried in your regrets!\n", attacker->name);


        sleep(1);
      }

      // Heathcliff:Wild Hunt - Tanut Skill 3-2
      if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") ==
                     0 &&
                 (atk == &attacker->skills[3]) && i == remainingCoins - 1) {
        
        printf("\n\n%s: Disappear with the storm...\n", attacker->name);
        

        sleep(1);
      }

      // ----------------------------------------------------------------

    } else {

       // ------------------ On Tail Hit ------------------------

      // Heathcliff:Wild Hunt - Tanut Skill 3-1
      if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") ==
                     0 &&
                 (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

        printf("\n\n%s: Remain buried in your regrets...\n", attacker->name);


        sleep(1);
      }

      // Heathcliff:Wild Hunt - Tanut Skill 3-2
      if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") ==
                     0 &&
                 (atk == &attacker->skills[3]) && i == remainingCoins - 1) {

        printf("\n\n%s: Disappear with the storm!\n", attacker->name);


        sleep(1);
      }

      // ----------------------------------------------------------------
      
    }

    int currentPower = totalPower;

    int bonus = 0;

    // Last coin power bonus (from character buffs only)
    if (i == remainingCoins - 1) {
        bonus = attacker->FinalPowerBoost + attacker->AttackPowerBoost;
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
if (modifiedPower < 0.0f) modifiedPower = 0.0f;

    // Adjust multiplier if defender is Unbreakable
    float adjustedMultiplier = atk->DmgMutiplier / ((Unbreakable > 0) ? 2.0f : 1.0f);

    // Add clash count bonus: +1% damage per clash (up to the current clash count)
    float clashMultiplier = 1.0f;  // 1% per clash
    if (Unbreakable <= 0) {
      clashMultiplier = 1.0f + (clashCount * 0.01f);  // 1% per clash
    }

    int Damage = (int)(modifiedPower * adjustedMultiplier * clashMultiplier);



    
    

    // Yi sang:Fell Bullet - Torn Memory
    if (strcasecmp(attacker->name, "Yi sang:Fell Bullet") == 0 &&
        (atk == &attacker->skills[0] && attacker->skills[0].active ||
         atk == &attacker->skills[1] && attacker->skills[1].active)) {

        Damage += (int)(Damage * (0.15 * attacker->Passive));
    }

    // Yi sang:Fell Bullet - Torn Memory
    if (strcasecmp(attacker->name, "Yi sang:Fell Bullet") == 0 &&
        atk == &attacker->skills[2] && attacker->skills[2].active) {

        Damage += (int)(Damage * (0.30 * attacker->Passive));
    }

    // --------------------------------- Don Quixote:The Manager of La Manchaland ---------------------------------------

    // ------------------------ Don Quixote:The Manager of La Manchaland --------------

    // Don Quixote:The Manager of La Manchaland - Tanut Skill 2
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[1]) && i == remainingCoins - 1) {

      printf("\n\n%s: Until you fall\n", attacker->name);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Tanut Skill 3
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: Impale through!\n", attacker->name);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Skill 2 in both Buff dmg
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
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
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[5]) && i == remainingCoins - 1) {

      printf("\n\n%s: Time to end this.\n", attacker->name);

      sleep(1);
    }

    // --------------------------------------------------------------------------

    // Don Quixote:The Manager of La Manchaland - Skill 2-2 dmg
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[1]) && i == remainingCoins - 1 && attacker->Passive >= 2) {

      int boost = (attacker->Passive/2) * 15;
      if (boost > 75) boost = 75;

      attacker->DamageUp += boost;

      printf("\n%s's last coin deals +15%% damage(%d%% - Max 75%%) for every 2 HardBlood (%d)", attacker->name, boost, attacker->Passive);

      sleep(1);
    }

    // Don Quixote:The Manager of La Manchaland - Skill 2-2 dmg
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
                   0 &&
               (atk == &attacker->skills[4]) && i == remainingCoins - 1 && attacker->Passive >= 2) {

      int boost = (attacker->Passive) * 20;
      if (boost > 150) boost = 150;

      attacker->DamageUp += boost;

      printf("\n%s's last coin deals +20%% damage(%d%% - Max 150%%) for every 2 HardBlood (%d)", attacker->name, boost, attacker->Passive);

      sleep(1);
    }

    
      // FireFist – Attack buff s3
      if (strcasecmp(attacker->name, "Gregor:Firefist") == 0 && (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

        printf("\n\n%s: RAAHHHHHHH!!! You vermin-like bastards!\n", attacker->name);

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
      if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[5] || atk == &attacker->skills[13] || atk == &attacker->skills[15]) && i == remainingCoins - 1) {

          int boost = abs((int)(5 * attacker->Sanity));  // 4% per missing fuel
          if (boost > 200) boost = 200;                    // cap at 100%
          attacker->DamageUp += boost;

           printf("\n%s's last coin deal +5%% damage for the further this unit's Sanity from 0 (%d%% - Max 200%%)", attacker->name, boost);

        sleep(1);
      }

    // Jia Qiu - S11
    if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[11] && i == remainingCoins - 1)) {

      int boost = abs(defender->Sanity);
      if (boost > 10) boost = 10;
      
          Damage += boost;

      printf("\n%s deal additional damage equal to the further enemy's Sanity from 0 (%d - Max 10)", attacker->name, boost);

      sleep(1);
    }

    // Sukuna:King of Curse - Tanut Skill 5
    if (strcasecmp(attacker->name, "Sukuna:King of Curse") ==
                   0 &&
               (atk == &attacker->skills[4]) && i == remainingCoins - 1) {

      if (attacker->skills[4].active == 1) {
      printf("\n\n%s: Scale of dragon\n", attacker->name);
      } else if (attacker->skills[4].active == 2) {
          printf("\n\n%s: Recoil\n", attacker->name);
          } else if (attacker->skills[4].active == 3) {
        printf("\n\n%s: Twin meteor\n", attacker->name);
        }

      sleep(1);
    }

    // ------------------- Heathcliff:Wild Hunt ---------------------

    // Heathcliff:Wild Hunt - Tanut Skill 2
    if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") ==
                   0 &&
               (atk == &attacker->skills[1]) && i == 2) {

      if (attacker->skills[0].active <= 0) {
      printf("\n\n%s: I'll rip you apart.\n", attacker->name);
      } else {
        printf("\n\n%s: Tear them all to shreds.\n", attacker->name);

        sleep(1);

        printf("\n%s: Dullahan!\n", attacker->name);
      }

      sleep(1);
    }

    // ------------------------------------------------------------------

    // Hong lu:The Lord of Hongyuan - Tanut Skill 2
    if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") ==
                   0 &&
               (atk == &attacker->skills[1]) && i == 0) {

      printf("\n\n%s: I'll personally sever your neck.\n", attacker->name);

      sleep(1);
    }

    // ---------------------------------------------------------------------

    // Lobotomy E.G.O::Solemn Lament Yi Sang - Tanut Skill 3
    if (strcasecmp(attacker->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
                   0 &&
               (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: Rest in peace.\n", attacker->name);

      sleep(1);
    }

    // Yi sang:Fell Bullet - Tanut Skill 3
    if (strcasecmp(attacker->name, "Yi sang:Fell Bullet") ==
                   0 &&
               (atk == &attacker->skills[2]) && i == 0) {

      printf("\n\n%s: I shall fire and pierce you as you are.\n", attacker->name);

      sleep(1);
    }

    // ---------------------------------------- Meursault:The Thumb --------------------------------

    // Meursault:The Thumb - Tanut Skill 3-2
    if (strcasecmp(attacker->name, "Meursault:The Thumb") ==
                   0 &&
               (atk == &attacker->skills[3]) && i == 0) {

      printf("\n\n%s: I shall now face you with all my might\n", attacker->name);

      sleep(2);
    }

     // Meursault:The Thumb - Tanut Skill 3
      if (strcasecmp(attacker->name, "Meursault:The Thumb") ==
                     0 &&
                 (atk == &attacker->skills[2] || atk == &attacker->skills[3]) && i == remainingCoins - 1) {

        if (attacker->Passive > 0) {
        printf("\n\n%s: Firing all rounds!\n", attacker->name);
        } else {
          printf("\n\n%s: Firing all rounds...\n", attacker->name);
        }

        sleep(1);
      }

    // Meursault:The Thumb S1-1
    if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[0]) && i == remainingCoins - 1 && attacker->Passive > 0 && !attacker->skills[3].active) {

        attacker->Passive--;
        attacker->skills[2].active++;
        attacker->DamageUp += 10;

         printf("\n%s spends 1 Tigermark Round(%d) to deal +10%% damage", attacker->name, attacker->Passive);

      sleep(1);
      
    } // Meursault:The Thumb S1-2
    else if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[0]) && i == remainingCoins - 1 && attacker->Passive > 0 && attacker->skills[3].active) {

        attacker->Passive--;
        attacker->skills[2].active++;
        attacker->DamageUp += 20;

         printf("\n%s spends 1 Savage Tigermark Round(%d) to deal +20%% damage", attacker->name, attacker->Passive);

      sleep(1);
    }

    // Meursault:The Thumb S2-1
    if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[1]) && attacker->Passive > 0 && !attacker->skills[3].active) {

      if (i == remainingCoins - 2) {

        attacker->Passive--;
        attacker->skills[2].active++;
          attacker->DamageUp += 5;

           printf("\n%s spends 1 Tigermark Round(%d) to deal +5%% damage", attacker->name, attacker->Passive);

        sleep(1);
      } else if (i == remainingCoins - 1) {
        attacker->Passive--;
        attacker->skills[2].active++;
        attacker->DamageUp += 35;

         printf("\n%s spends 1 Tigermark Round(%d) to deal +30%% damage", attacker->name, attacker->Passive);

      sleep(1);
      }
      
    } // Meursault:The Thumb S2-2
    else if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[1]) && attacker->Passive > 0 && attacker->skills[3].active) {

        if (i == remainingCoins - 2) {

          attacker->Passive--;
          attacker->skills[2].active++;
            attacker->DamageUp += 10;

             printf("\n%s spends 1 Savage Tigermark Round(%d) to deal +10%% damage", attacker->name, attacker->Passive);

          sleep(1);
        } else if (i == remainingCoins - 1) {
          attacker->Passive--;
          attacker->skills[2].active++;
          attacker->DamageUp += 70;

           printf("\n%s spends 1 Savage Tigermark Round(%d) to deal +60%% damage", attacker->name, attacker->Passive);

        sleep(1);
        }

      }
    
    // Meursault:The Thumb S3-1
    if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[2]) && !attacker->skills[3].active && attacker->Passive > 0) {

      if (i == remainingCoins - 1) {
        
        attacker->Passive--;
        attacker->skills[2].active++;
        int boost = 50; 
        attacker->DamageUp += boost;

         printf("\n%s spends 1 Tigermark Round(%d) to deal +50%% damage", attacker->name, attacker->Passive);

      sleep(1);
      } else {

        attacker->Passive--;
        attacker->skills[2].active++;
          attacker->DamageUp += 10;

           printf("\n%s spends 1 Tigermark Round(%d) to deal +10%% damage", attacker->name, attacker->Passive);

        sleep(1);
        
      }
      
    }

    // Meursault:The Thumb S3-2
    if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 && (atk == &attacker->skills[3]) && attacker->skills[3].active && attacker->Passive > 0) {

      if (i == remainingCoins - 1) {

        attacker->Passive--;
        attacker->skills[2].active++;
        int boost = 50; 
        attacker->DamageUp += boost;

         printf("\n%s spends 1 Savage Tigermark Round(%d) to deal +50%% damage", attacker->name, attacker->Passive);

      sleep(1);
      } else if (i == remainingCoins - 3) {

        attacker->Passive--;
        attacker->skills[2].active++;
          attacker->DamageUp += 20;

           printf("\n%s spends 1 Savage Tigermark Round(%d) to deal +20%% damage", attacker->name, attacker->Passive);

        sleep(1);

      } else if (i == remainingCoins - 2) {

        attacker->Passive--;
        attacker->skills[2].active++;
          attacker->DamageUp += 20;

           printf("\n%s spends 1 Savage Tigermark Round(%d) to deal +20%% damage", attacker->name, attacker->Passive);

        sleep(1);

      }

    }

    // ------------------------------------------------------------

    // Hong lu:The Lord of Hongyuan S2 Last coins
    if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[4]) && i == remainingCoins - 1) {

      printf("\n\n%s: Clear the path.\n", attacker->name);

      sleep(1);
    }

    // Hong lu:The Lord of Hongyuan S2 Last coins
    if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[1] || atk == &attacker->skills[4]) && i == remainingCoins - 1) {

        attacker->DamageUp += 50;

         printf("\n%s's last coin deal +50%% damage", attacker->name);

      sleep(1);
    }


    // Meursault:Blade Lineage Mentor S2 Last coins
    if (strcasecmp(attacker->name, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[1]) && i == 2) {

        attacker->DamageUp += 60;

         printf("\n%s's this coin deal +60%% damage", attacker->name);

      sleep(1);
    }

    // Fixer grade 9? S8 Last coins
    if (strcasecmp(attacker->name, "Fixer grade 9?") == 0 && (atk == &attacker->skills[7]) && i == remainingCoins - 1) {

        attacker->DamageUp += 50;

         printf("\n%s's last coin deal +50%% damage", attacker->name);

      sleep(1);
    }

    
    

    double damageUp = 1.0 + (attacker->DamageUp / 100.0);       // increase by DamageUp%
    double protectionUp = 1.0 - (defender->Protection / 100.0); // reduce by Protection%
    if(damageUp < 0) damageUp = 0;
    if(protectionUp < 0) protectionUp = 0;

    int finalDamage = (int)(Damage * damageUp * protectionUp);

    if (defender->Shield > 0) {
      defender->Shield -= finalDamage;
        if (defender->Shield < 0) {
            // คำนวณส่วนที่ทะลุเกราะ
            defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
            defender->Shield = 0;             // เกราะหมดแล้ว
        }
    } else {
        defender->HP -= finalDamage;
    }

    if (defender->HP < 0)
      defender->HP = 0;

    totalDamage += finalDamage;

    printf("\n%-10d %-10d %-10d", i + 1, currentPower, finalDamage);



// --------------- Meursault:Blade Lineage Mentor -----------------

    // Meursault:Blade Lineage Mentor - Gain on attack
    if (strcasecmp(attacker->name, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[0])) {

      int gain = 5;

      attacker->DamageUp += gain;
      attacker->DamageUpNextTurn += gain;

      printf(" +%d%% damage this turn and next turn", gain);

    }

    // Meursault:Blade Lineage Mentor - Gain on attack
    if (strcasecmp(attacker->name, "Meursault:Blade Lineage Mentor") == 0 && (atk == &attacker->skills[1]) && i == 0) {

      int gain = 15;

        attacker->DamageUp += gain;
      attacker->DamageUpNextTurn += gain;

      printf(" +%d%% damage this turn and next turn", gain);

    }

    // ---------------------------------------------------------

    // -------------------------------- Heishou Pack - You Branch Adept Heathcliff --------------------------------

    // Heishou Pack - You Branch Adept Heathcliff - Gain on attack
    if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0) {

      int gain = 1;
      if (attacker->HP < attacker->MAX_HP * 0.5) gain += 1;
      
        attacker->Passive += gain;
      if (attacker->Passive > 20) attacker->Passive = 20;

      printf(" +%d Battleblood Instinct (%d)", gain, attacker->Passive);

    }

    // Heishou Pack - You Branch Adept Heathcliff - Bloodflame buff
    if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && attacker->skills[2].active > 0 && i < 3) {

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
    if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i != remainingCoins - 1) {

        attacker->skills[0].active += 2;
      if (attacker->skills[0].active > 99) attacker->skills[0].active = 99;

      printf("\n%s applies +2 Burn Stack(%d) on self", attacker->name, attacker->skills[0].active);

      sleep(1);
    }
    
    // Heishou Pack - You Branch Adept Heathcliff Skill 2 Last coins
    if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[1]) && i == remainingCoins - 1) {

        attacker->skills[0].active += 3;
      if (attacker->skills[0].active > 99) attacker->skills[0].active = 99;
        attacker->skills[1].active++;
      if (attacker->skills[1].active > 99) attacker->skills[1].active = 99;

      printf("\n%s applies +3 Burn Stack(%d) and +1 Burn Count(%d) on self", attacker->name, attacker->skills[0].active, attacker->skills[1].active);

      sleep(1);

      float damageboost = attacker->skills[0].active * 2.0f;
      if (damageboost > 20) damageboost = 20;
      int damage = finalDamage * (damageboost / 100);
      
      if (defender->Shield > 0) {
        defender->Shield -= damage;
          if (defender->Shield < 0) {
              // คำนวณส่วนที่ทะลุเกราะ
              defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
              defender->Shield = 0;             // เกราะหมดแล้ว
          }
      } else {
          defender->HP -= damage;
      }
      
      if (defender->HP < 0)
        defender->HP = 0;

      totalDamage += damage;

         printf("\n%s deals %d ((Burn Stack on self x 2)%% of this Coin's final damage) addition damage (%.0f%% - Max 20%%)", attacker->name, damage, damageboost);

      sleep(1);
    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 3 Last coins
    if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2]) && i == remainingCoins - 1) {

      float damageboost = (attacker->skills[0].active + attacker->skills[1].active) * 2.0f;
      if (damageboost > 20) damageboost = 20;
      int damage = finalDamage * (damageboost / 100);

      if (defender->Shield > 0) {
        defender->Shield -= damage;
          if (defender->Shield < 0) {
              // คำนวณส่วนที่ทะลุเกราะ
              defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
              defender->Shield = 0;             // เกราะหมดแล้ว
          }
      } else {
          defender->HP -= damage;
      }
      
      if (defender->HP < 0)
        defender->HP = 0;

      totalDamage += damage;

         printf("\n%s deals %d ((Burn Stack and Count on self x 2)%% of this Coin's final damage) addition damage (%.0f%% - Max 20%%)", attacker->name, damage, damageboost);

      sleep(1);
    }

    // Heishou Pack - You Branch Adept Heathcliff Skill 4 Last coins
    if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[3]) && i == remainingCoins - 1) {

      float damageboost = 20.0f;
      int damage = finalDamage * (damageboost / 100);

      if (defender->Shield > 0) {
        defender->Shield -= damage;
          if (defender->Shield < 0) {
              // คำนวณส่วนที่ทะลุเกราะ
              defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
              defender->Shield = 0;             // เกราะหมดแล้ว
          }
      } else {
          defender->HP -= damage;
      }
      
      if (defender->HP < 0)
        defender->HP = 0;

      totalDamage += damage;

         printf("\n%s deals %d (20%% of this Coin's final damage) addition damage", attacker->name, damage);

      sleep(1);

      float healvalue = finalDamage + damage;

      if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && remainingCoins > attacker->skills[3].Coins && (attacker->skills[0].active >= 20 || attacker->HP <= attacker->MAX_HP * 0.5)) {
        
        attacker->HP += healvalue;

        if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;

        printf("\n%s's final Coin, heal HP by the amount of damage the above effect dealt (%.0f)", attacker->name, healvalue);

        sleep(1);
        
      } else if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && remainingCoins <= attacker->skills[3].Coins && attacker->skills[0].active < 20 && attacker->HP > attacker->MAX_HP * 0.5) {
        
        attacker->HP += healvalue;

        if (attacker->HP > attacker->MAX_HP) attacker->HP = attacker->MAX_HP;


        printf("\n%s's final Coin, heal HP by the amount of damage the above effect dealt (%0.f)", attacker->name, healvalue);

         sleep(1);
    }
    }
    // ----------------------------------------------------------------


    // -------------------------------- Gregor:Firefist --------------------------------
    
    // Gregor:Firefist - S1 Burn
    if (strcasecmp(attacker->name, "Gregor:Firefist") == 0 &&
        atk == &attacker->skills[0]) {

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
        printf("+%d Burn Stack (%d) \t +%d Burn Count (%d)", Stack, attacker->skills[0].active, Count, attacker->skills[1].active);
      } else if (Stack > 0) {
           printf(" +%d Burn Stack (%d)", Stack, attacker->skills[0].active);
           } else if (Count > 0) {
             printf(" +%d Burn Count (%d)", Count, attacker->skills[1].active);
           }

      sleep(1);
    }

    // Gregor:Firefist - S2 Burn
    if (strcasecmp(attacker->name, "Gregor:Firefist") == 0 &&
        atk == &attacker->skills[1]) {

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
        printf("+%d Burn Stack on target (%d) \t +%d Burn Count on target (%d)", Stack, attacker->skills[0].active, Count, attacker->skills[1].active);
      } else if (Stack > 0) {
           printf(" +%d Burn Stack on target (%d)", Stack, attacker->skills[0].active);
           } else if (Count > 0) {
             printf(" +%d Burn Count on target (%d)", Count, attacker->skills[1].active);
           }


      sleep(1);
    }

    // Gregor:Firefist - S3 Burn
      if (strcasecmp(attacker->name, "Gregor:Firefist") == 0 &&
          atk == &attacker->skills[2]) {

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
          printf("+%d Burn Stack on target (%d) \t +%d Burn Count on target (%d)", Stack, attacker->skills[0].active, Count, attacker->skills[1].active);
        } else if (Stack > 0) {
             printf(" +%d Burn Stack on target (%d)", Stack, attacker->skills[0].active);
             } else if (Count > 0) {
               printf(" +%d Burn Count on target (%d)", Count, attacker->skills[1].active);
             }


        sleep(1);
      }

    // Gregor:Firefist - S4 Burn
    if (strcasecmp(attacker->name, "Gregor:Firefist") == 0 &&
        atk == &attacker->skills[3]) {

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
        printf("+%d Burn Stack on target (%d) \t +%d Burn Count on target (%d)", Stack, attacker->skills[0].active, Count, attacker->skills[1].active);
      } else if (Stack > 0) {
           printf(" +%d Burn Stack on target (%d)", Stack, attacker->skills[0].active);
           } else if (Count > 0) {
             printf(" +%d Burn Count on target (%d)", Count, attacker->skills[1].active);
           }

      sleep(1);
    }

    // ------------------------------------------------------------------------

    
    // ------------------------------------ Binah -----------------------------------
    
    // Binah - Fairy Skill 1
    if (strcasecmp(attacker->name, "Binah") == 0 && (atk == &attacker->skills[0])) {

      int inflictvalue = 1;

      if (attacker->Passive) inflictvalue = 2;

            attacker->skills[0].active += inflictvalue;

      printf(" Fairy +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

      sleep(1);
    }

    // Binah - Fairy Skill 3
    if (strcasecmp(attacker->name, "Binah") == 0 && (atk == &attacker->skills[2]) && i == 0) {

      int inflictvalue = 3;

      if (attacker->Passive) inflictvalue = 5;

       attacker->skills[0].active += inflictvalue;

      printf(" Fairy +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

    }

    // Binah - Fairy
    if (strcasecmp(attacker->name, "Binah") == 0 && attacker->skills[0].active > 0) {

      if (!attacker->Passive) {
        if (defender->Shield > 0) {
          defender->Shield -= attacker->skills[0].active;
            if (defender->Shield < 0) {
                // คำนวณส่วนที่ทะลุเกราะ
                defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                defender->Shield = 0;             // เกราะหมดแล้ว
            }
        } else {
            defender->HP -= attacker->skills[0].active;
        }
        
      totalDamage += attacker->skills[0].active;

      printf(" Fairy deals %d damage", attacker->skills[0].active);

      } else {

        int Fairydamage = 0.5*((defender->MAX_HP/100)*attacker->skills[0].active);
        
        if (defender->Shield > 0) {
          defender->Shield -= Fairydamage;
            if (defender->Shield < 0) {
                // คำนวณส่วนที่ทะลุเกราะ
                defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                defender->Shield = 0;             // เกราะหมดแล้ว
            }
        } else {
            defender->HP -= Fairydamage;
        }
        
          totalDamage += Fairydamage;

          printf(" Fairy deals %d damage", Fairydamage);

          }

    }

    // -----------------------------------------------------------------------------------------


    // ----------------------------- Lobotomy E.G.O::Solemn Lament Yi Sang ------------------------
    

    // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly s1
    if (strcasecmp(attacker->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
            0 && (attacker->Passive > 0) && (atk == &attacker->skills[0])) {

      int inflictvalue = 1;

      attacker->Passive -= inflictvalue;
      if (attacker->Passive < 0) attacker->Passive = 0;
        attacker->skills[0].active += inflictvalue;

      printf(" Butterfly +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

    } // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly s2
    else if (strcasecmp(attacker->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
            0 && (attacker->Passive > 0) && (atk == &attacker->skills[1])) {

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

      printf(" Butterfly +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);

    } // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly s3
    else if (strcasecmp(attacker->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
            0 && (attacker->Passive >= 0) && (atk == &attacker->skills[2])) {

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

      printf(" Butterfly +%d on enemy(%d)", inflictvalue, attacker->skills[0].active);
        
      } 
      
      if (i == remainingCoins - 1) {

        if (defender->Shield > 0) {
          defender->Shield -= attacker->skills[0].active;
            if (defender->Shield < 0) {
                // คำนวณส่วนที่ทะลุเกราะ
                defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                defender->Shield = 0;             // เกราะหมดแล้ว
            }
        } else {
            defender->HP -= attacker->skills[0].active;
        }
        
        totalDamage += attacker->skills[0].active;
        printf(" Deal more damage equal to Butterfly on enemy(%d)", attacker->skills[0].active);

    } 

    }



    // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly heal lose at 0-
    if (strcasecmp(attacker->name,
                   "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
        (attacker->skills[0].active > 0) && attacker->Sanity < 0) {

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

          if (defender->Shield > 0) {
            defender->Shield -= min;
              if (defender->Shield < 0) {
                  // คำนวณส่วนที่ทะลุเกราะ
                  defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                  defender->Shield = 0;             // เกราะหมดแล้ว
              }
          } else {
              defender->HP -= min;
          }
          
            totalDamage += min;
          
        }
      }

    } // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly heal lose at 0+
      else if (strcasecmp(attacker->name,
                     "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
          (attacker->skills[0].active > 0) && attacker->Sanity >= 0) {

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

            if (defender->Shield > 0) {
              defender->Shield -= min;
                if (defender->Shield < 0) {
                    // คำนวณส่วนที่ทะลุเกราะ
                    defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                    defender->Shield = 0;             // เกราะหมดแล้ว
                }
            } else {
                defender->HP -= min;
            }
            
            totalDamage += min;

            }

        }

        if (defender->hasSanity == 1 && defender->Sanity < 0 && defender->sanityGainBase >= 0) { // Normal
          
          int deal = attacker->skills[0].active/2 - (defender->Sanity/5);

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            if (defender->Shield > 0) {
              defender->Shield -= deal;
                if (defender->Shield < 0) {
                    // คำนวณส่วนที่ทะลุเกราะ
                    defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                    defender->Shield = 0;             // เกราะหมดแล้ว
                }
            } else {
                defender->HP -= deal;
            }

            totalDamage += deal;

          }

              } else if (defender->hasSanity == 1 && defender->Sanity > 0 && defender->sanityGainBase < 0) { // Negative Sanity enemy

          int deal = attacker->skills[0].active/2 + (defender->Sanity/5);

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            if (defender->Shield > 0) {
              defender->Shield -= deal;
                if (defender->Shield < 0) {
                    // คำนวณส่วนที่ทะลุเกราะ
                    defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                    defender->Shield = 0;             // เกราะหมดแล้ว
                }
            } else {
                defender->HP -= deal;
            }

            totalDamage += deal;

          }

              } else if (defender->hasSanity == 0) { // No Sanity enemy

          int deal = attacker->skills[0].active/2;

          if (deal > 0) {

                printf(" \tDeal %d damage on enemy", deal);

            if (defender->Shield > 0) {
              defender->Shield -= deal;
                if (defender->Shield < 0) {
                    // คำนวณส่วนที่ทะลุเกราะ
                    defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
                    defender->Shield = 0;             // เกราะหมดแล้ว
                }
            } else {
                defender->HP -= deal;
            }

            totalDamage += deal;

          }

        }


        
      sleep(1);
    }

    //Lobotomy E.G.O::Solemn Lament Yi Sang while attack Reload
    if (strcasecmp(attacker->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
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

    //Dawn Office Fixer Sinclair S1 EGO FORM REUSE
    if (strcasecmp(attacker->name, "Dawn Office Fixer Sinclair") ==
      0 && attacker->skills[3].active && i == 0 && atk == &attacker->skills[0]) {

        remainingCoins++;

       printf("\n%s in a Volatile E.G.O State, Reuse Coin (once per Skill)", attacker->name);
        
          sleep(1);

      }

    // Fixer grade 9? S6 Reuse
    if (strcasecmp(attacker->name, "Fixer grade 9?") ==
      0 && attacker->Passive >= 10 && i == 0 && atk == &attacker->skills[5] && !ClashLostAttack) {

        remainingCoins++;

      attacker->Passive -= 5;
       if (attacker->Passive < 0) attacker->Passive = 0;

       printf("\n%s at 10+ Black Silence and without Clash Lost, consumes 5 Black Silence (%d) to Reuse Coin (once per Skill)", attacker->name, attacker->Passive);

          sleep(1);

      }

    // Fixer grade 9? S3 Reuse
    if (strcasecmp(attacker->name, "Fixer grade 9?") == 0 && attacker->Passive >= 15 && atk == &attacker->skills[2] && i == remainingCoins - 1 && remainingCoins <= atk->Coins && !ClashLostAttack) {

      attacker->skills[2].Coins = remainingCoins;

      remainingCoins++;

       attacker->Passive -= 5;
       if (attacker->Passive < 0) attacker->Passive = 0;

       printf("\n%s at 15+ Black Silence and without Clash Lost, consumes 5 Black Silence (%d) to Reuse Coin (once per Skill)", attacker->name, attacker->Passive);

          sleep(1);

    } else if (strcasecmp(attacker->name, "Fixer grade 9?") == 0 && atk == &attacker->skills[3]) {
      attacker->skills[2].Coins = 2;
    }

    // -------------------- Heishou Pack - You Branch Adept Heathcliff ---------------------
    
    // Heishou Pack - You Branch Adept Heathcliff - Skill 2 reuse
    if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (attacker->skills[2].active > 0 || attacker->HP < attacker->MAX_HP * 0.5) && atk == &attacker->skills[1] && i == remainingCoins - 1 && remainingCoins <= attacker->skills[1].Coins) {

      attacker->skills[1].Coins = remainingCoins;

      remainingCoins++;

       printf("\n%s has Bloodflame [血炎] or less than 50%% HP, Reuse Coin (Once per Skill)", attacker->name);

          sleep(1);

    } else if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && atk == &attacker->skills[1]) {
      attacker->skills[1].Coins = 3;
    }

    // Heishou Pack - You Branch Adept Heathcliff - Skill 3-2 reuse
    if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (attacker->skills[0].active >= 20 || attacker->HP <= attacker->MAX_HP * 0.5) && atk == &attacker->skills[3] && i == remainingCoins - 1 && remainingCoins <= attacker->skills[3].Coins) {

      attacker->skills[3].Coins = remainingCoins;

      remainingCoins++;

       printf("\n%s has 20+ Burn or less than 50%% HP, Reuse Coin (Once per Skill)", attacker->name);

          sleep(1);

    } else if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && atk == &attacker->skills[3]) {
      attacker->skills[3].Coins = 4;
    }

    // -------------------------------------------------------------

    // Heathcliff:Wild Hunt – Skill 4 addition damage
    if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
       (atk == &attacker->skills[3]) && i == remainingCoins - 1) {

      int damage = abs(attacker->Sanity - defender->Sanity);
      if (damage > 30) damage = 30;

      if (defender->Shield > 0) {
        defender->Shield -= damage;
          if (defender->Shield < 0) {
              // คำนวณส่วนที่ทะลุเกราะ
              defender->HP += defender->Shield; // เพราะ Shield เป็นค่าลบ
              defender->Shield = 0;             // เกราะหมดแล้ว
          }
      } else {
          defender->HP -= damage;
      }
      
      totalDamage += damage;

      printf("\n%s deals more damage base on Sanity different (%d - Max 30)",
         attacker->name, damage);

        sleep(1);
    }
    
    
    // Sancho:The Second Kindred of Don Quixote - Heal mechnics
     if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") == 0 && ClashLostAttack == 0) {

      int healvalue = 40;

       int missingHP = (int)(((float)(attacker->MAX_HP - attacker->HP) / attacker->MAX_HP) * 100);
        if (missingHP > 20) missingHP = 20;

       healvalue += missingHP;

    if (strcasecmp(attacker->name,
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
        else if (strcasecmp(attacker->name,"Sancho:The Second Kindred of Don Quixote") == 0 &&
           (atk == &attacker->skills[10] || atk == &attacker->skills[11] || atk == &attacker->skills[12] || atk == &attacker->skills[13]) && ClashLostAttack == 1) {

         printf(" HP +%d", (int)(attacker->HP + finalDamage) > attacker->MAX_HP
                                 ? (int)(attacker->MAX_HP - attacker->HP)
                                 : finalDamage);

         attacker->HP += (int)(attacker->HP + finalDamage) > attacker->MAX_HP
                             ? (int)(attacker->MAX_HP - attacker->HP)
                             : finalDamage;

        }

    // Don Quixote:The Manager of La Manchaland - Heal mechnics
    if ((strcasecmp(attacker->name,
                    "Don Quixote:The Manager of La Manchaland") == 0 &&
         (atk == &attacker->skills[0] || atk == &attacker->skills[1] ||
          atk == &attacker->skills[4]))) {
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

    } else if ((strcasecmp(attacker->name,
                           "Don Quixote:The Manager of La Manchaland") == 0 &&
                (atk == &attacker->skills[2]))) {
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

    } else if (strcasecmp(attacker->name,
                          "Don Quixote:The Manager of La Manchaland") == 0 &&
               (atk == &attacker->skills[5] || atk == &attacker->skills[3])) {
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
    if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (attacker->skills[15].active > 0) && defender->HP <= 0) {

      if (strcasecmp(defender->name, "Hong lu:The Lord of Hongyuan") == 0) {

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

    

    if (i == remainingCoins - 1) {
      printf("\n(%d bonus) Damage Multiplier: %.2f\n", bonus,
             atk->DmgMutiplier);
    }

    if (Unbreakable > 0) Unbreakable--; // ← Character's clash lost coins

    sleep(1);
  }

  printf("%s total damage dealt (Opponent's defense: %d): %d\n",
         attacker->name, defTempDefense, totalDamage);



  
  
  //---------------- After Attack Buff ----------------------------

    // Erlking Heathcliff Faded promise for wild hunt
    if (strcasecmp(attacker->name, "Erlking Heathcliff") == 0 && strcasecmp(defender->name, "Heathcliff:Wild Hunt") == 0 && (attacker->skills[7].active == 1) && defender->HP <= 0)  {

        attacker->skills[7].active -= 1;

      defender->HP = 1;
        
        printf("\n%s's 'Faded Promise' activated! In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn (Once per Encounter)\n", defender->name);
        
        sleep(1);
    }








    

    // Don Quixote:The Manager of La Manchaland - S3-1 Buff S3-2
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") == 0 &&
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
    if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") == 0 &&
        (atk == &attacker->skills[3] || atk == &attacker->skills[4])) {

      int Hardblood = 10;

      attacker->Passive -= Hardblood;
      if (attacker->Passive < 1) attacker->Passive = 1;

      printf("\n%s consumes %d Hardblood (%d left)\n", attacker->name,
             Hardblood, attacker->Passive);

      sleep(1);
    }

  // Heishou Pack - You Branch Adept Heathcliff Skill 3 after attack
  if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[2])) {

      attacker->skills[1].active += 2;

       printf("\n%s gains 2 Burn Count (%d)\n", attacker->name,attacker->skills[1].active);

    sleep(1);

  }

  // Heishou Pack - You Branch Adept Heathcliff Skill 4 after attack consumes
  if (strcasecmp(attacker->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (atk == &attacker->skills[3])) {

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

  // Binah - phase 2
  if (strcasecmp(defender->name, "Binah") == 0 && !defender->Passive && defender->HP <= defender->MAX_HP*0.5) {

    defender->Passive = 1;
    
    printf("\n%s: Ara~... I really surprised that you pushed me this far; then let's get a bit 'Serious'.\n", defender->name);

    sleep(1);

        defender->HP = 1150;
         defender->MAX_HP = 1150;
        defender->DamageUpNextTurn += 100;
        defender->FinalPowerBoostNextTurn += 5;

        defender->skills[0].name = "Fairy";
         defender->skills[0].BasePower += 3;
        defender->skills[0].Unbreakable = 2;

        defender->skills[1].name = "Chain";
        defender->skills[1].BasePower += 1;
        defender->skills[1].CoinPower += 2;
        defender->skills[1].Unbreakable = 1;

        defender->skills[2].name = "Pillar";
        defender->skills[2].BasePower += 1;
        defender->skills[2].CoinPower += 2;
        defender->skills[2].Unbreakable = 1;

        defender->skills[3].name = "Lock";
        defender->skills[3].Coins += 1;
        defender->skills[3].Unbreakable = 2;
        defender->skills[3].CoinPower -= 4;

        defender->skills[4].name = "Shockwave";
        defender->skills[4].BasePower += 3;
        defender->skills[4].CoinPower += 1;
        defender->skills[4].Unbreakable = 3;

    printf("\n%s at 50%% or less HP, 'Serious' activated! increase HP and Max HP to 1150, gains +100%% damage and 5 Final Power for one turn; then gain new Skills set (Once per Encounter) (Cannot be defeat until this effect activated)\n", defender->name);

    sleep(1);

    if (strcasecmp(attacker->name, "Fixer grade 9?") == 0) {
    printf("\n%s gains 'Shin (心) - The Black Silence', Defense +50, Offense +15, +1 Base Power and +10%% damage for every 10 different Sanity, All Skills become Unbreakable Coins\n",
       attacker->name);

  }
  }
  
  // Binah - Skill 2 debuff
  if (strcasecmp(attacker->name, "Binah") == 0 && (atk == &attacker->skills[1])) {

    int inflictvalue = 1;

        defender->ParalyzeNextTurn += inflictvalue;

    printf("\n%s inflicts %d Paralyze next turn (Fix the Power of 1 Coins to 0 for one turn)\n", attacker->name,inflictvalue);

    sleep(1);
  }

  // Binah - Skill 4 debuff
  if (strcasecmp(attacker->name, "Binah") == 0 && (atk == &attacker->skills[3])) {

    int inflictvalue = 50;

    defender->DamageUp -= inflictvalue;
        defender->DamageUpNextTurn -= inflictvalue;

    printf("\n%s deals -%d%% damage for this turn and next turn\n", defender->name, inflictvalue);

    sleep(1);
  }

  // Binah - Skill 5 debuff serious
  if (strcasecmp(attacker->name, "Binah") == 0 && (atk == &attacker->skills[4])) {

    int inflictvalue = 1;
    
    if (attacker->Passive) {
      inflictvalue = 2;
    }

        defender->ClashPowerNextTurn -= inflictvalue;

    printf("\n%s gain %d Clash Power Down by %s's Skill\n", defender->name, inflictvalue, attacker->name);

    sleep(1);
  }

  // Binah - Skill 3 4 debuff serious
  if (strcasecmp(attacker->name, "Binah") == 0 && attacker->Passive && (atk == &attacker->skills[2] || atk == &attacker->skills[3])) {

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
    if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 && (atk == &attacker->skills[2]) && attacker->Passive > 0) {

        defTempDefense -= attacker->Passive;
        defTempOffense -= attacker->Passive;

         printf("\n%s's last coin, Inflict 1 Offense Down and 1 Defense Down for every Heishou Bolus Contamination [黑獸丸染] (%d)\n", attacker->name, attacker->Passive);

      sleep(1);
    }

    // -------------------------------- Roland ------------------------------
    // Fixer grade 9? S8 Heal
    if (strcasecmp(attacker->name, "Fixer grade 9?") == 0 && (atk == &attacker->skills[8]) && attacker->Passive >= 20) {

      int totalheal = totalDamage;

      if (totalheal > 10) totalheal = 10;
      
          updateSanity(attacker, -(totalDamage));
      if (attacker->Sanity < -45) attacker->Sanity = -45;

         printf("\n%s at 20+ Black Silence, loses Sanity equal to damage dealt (%d - Max 10)\n", attacker->name, totalheal);

      sleep(1);
    }

    // Roland – Mang (心)
    if (strcasecmp(attacker->name, "Fixer grade 9?") == 0 && strcasecmp(defender->name, "Binah") == 0 && attacker->skills[6].active > 0) {

      int Mang = attacker->skills[6].active * 5;

      printf("\nIf %s generated Mang (望), lose 5 Sanity for each Mang (望) generated (%d)\n",
        attacker->name, Mang);

        updateSanity(attacker, -(Mang));

      attacker->skills[6].active = 0;

      sleep(1);
    }

    // ----------------------------------------------------------------

  // Dawn Office Fixer Sinclair - Skill EGO form S4 lose sanity
  if (strcasecmp(attacker->name, "Dawn Office Fixer Sinclair") == 0 && attacker->skills[3].active && (atk == &attacker->skills[3] || atk == &attacker->skills[2])) {

      updateSanity(attacker, -(15));
      if (attacker->Sanity < -45) attacker->Sanity = -45;

      printf("\n%s loses 15 Sanity (%d Sanity)\n",
             attacker->name, attacker->Sanity);

      sleep(1);

  }

  // Meursault:The Thumb Shin buffs (temporary, print once)
  if (strcasecmp(attacker->name, "Meursault:The Thumb") == 0 &&
       attacker->Passive <= 0 && !attacker->skills[3].active) {

    int amount = ((int)(8 * defender->MAX_HP)) / 847;
    if (amount < 8) amount = 8;

    if (strcasecmp(defender->name, "Sancho:The Second Kindred of Don Quixote") == 0 || strcasecmp(defender->name, "Don Quixote") == 0) amount += 2; // pity for boss

    attacker->skills[3].active = 1;

      attacker->Passive = amount;
    
      printf("\n%s spent all Tigermark Round, 'Unrelenting Spirit [剛氣]' activated and reload %d Savage Tigermark Round\n",
             attacker->name, amount);

      sleep(1);

      printf("\n%s: I see that you are worth the cost of my ammunition.\n", attacker->name);

    }

  // Meursault:The Thumb Shin buffs (temporary, print once)
  if (strcasecmp(defender->name, "Meursault:The Thumb") == 0 &&
    defender->HP <= defender->MAX_HP*0.65 && !defender->skills[3].active) {

    int amount = ((int)(8 * attacker->MAX_HP)) / 847;
    if (amount < 8) amount = 8;

      defender->skills[3].active = 1;

    defender->Passive = amount;

      printf("\n%s at 65%% or less HP, 'Unrelenting Spirit [剛氣]' activated and reload %d Savage Tigermark Round\n",
        defender->name, amount);

      sleep(1);

      printf("\n%s: Keh... Why, that's quite good.\n", defender->name);

    }

  // Roland – Buff
  if (strcasecmp(attacker->name, "Fixer grade 9?") == 0 && atk == &attacker->skills[9]) {

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
  if (strcasecmp(attacker->name, "Lei heng") == 0 &&
      (atk == &attacker->skills[2] || atk == &attacker->skills[5])) {

    int clashpowerdebuff = attacker->skills[1].active * 1;
    if (clashpowerdebuff > 5) clashpowerdebuff = 5;
    int takemoredamage = attacker->skills[1].active * 10;
    if (takemoredamage > 50) takemoredamage = 50;
    
    attacker->ClashPowerNextTurn -= clashpowerdebuff;
      attacker->ProtectionNextTurn -= takemoredamage;

        printf("\n%s gained %d Overheat(Clash Power -%d, Take %d%% more damage) next turn\n", attacker->name, attacker->skills[1].active, clashpowerdebuff, takemoredamage);

  }
  
  // Lei heng – inner strength gain
  if (strcasecmp(attacker->name, "Lei heng") == 0 &&
    attacker->skills[0].active == 2) {

    attacker->Passive += remainingCoins * 2;
    if (attacker->Passive >= 25)
      attacker->Passive = 25;

    printf("\n%s gains +%d Inner Strength [底力](%d - Max 25)\n",
           attacker->name, remainingCoins * 2, attacker->Passive);

  } else if (strcasecmp(attacker->name, "Lei heng") == 0 &&
    attacker->skills[0].active == 3) {

    attacker->Passive += remainingCoins * 3;
    if (attacker->Passive >= 50)
      attacker->Passive = 50;

    printf("\n%s gains +%d Extreme Strength [極力](%d - Max 50)\n",
           attacker->name, remainingCoins * 3, attacker->Passive);
  }

  // Sukuna - Black Flash
  if (strcasecmp(attacker->name, "Sukuna:King of Curse") == 0 &&
      atk == &attacker->skills[5] && ClashLostAttack == 0) {

    attacker->HP += (attacker->HP + 25 > attacker->MAX_HP)
                        ? attacker->MAX_HP - attacker->HP
                        : 25;

    updateSanity(attacker, 10);
    if (attacker->Sanity > 45) attacker->Sanity = 45;

    printf("\n%s used Black Flash without Clash Lost Coin, HP +25 (%.2f), Sanity +10 (%d)\n", attacker->name, attacker->HP, attacker->Sanity);

    sleep(1);
  }

  // --------------------- Heathcliff:Wild Hunt -----------------------
  // Heathcliff:Wild Hunt – skill 3 heal sanity
  if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
      atk == &attacker->skills[3]) {

    if (attacker->Sanity < 0) {
      printf("\n%s at less than 0 Sanity, heals 10 Sanity. When at less than 0 Sanity, heal more Sanity the further this unit's Sanity is from 0 (heal 2 additionalal Sanity for every missing Sanity; Max 50)\n",
         attacker->name);
      
        int missingSP = -attacker->Sanity;       // how far below 0
        int extraHeal = 2 * missingSP;           // 2 SP per missing SP
        if (extraHeal > 50) extraHeal = 50;      // cap at 50

        int totalHeal = 10 + extraHeal;          // base 10 + extra
        updateSanity(attacker, totalHeal);

      printf("\n%s heals %d Sanity (%d)\n",
         attacker->name, totalHeal, attacker->Sanity);
    }

      sleep(1);

       attacker->skills[0].active = 0;
    attacker->skills[2].Copies = 1;

      printf("\n%s loses all 'Dullahan'\n", attacker->name);
  }

  // Heathcliff:Wild Hunt – gain coffin
  if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
     ( atk == &attacker->skills[2] || atk == &attacker->skills[3])) {

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
    if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
       (atk == &attacker->skills[3])) {

      updateSanity(attacker, 10);
      if (attacker->Sanity > 45) attacker->Sanity = 45;
      
      printf("\n%s heals 10 Sanity (%d)\n",
         attacker->name, attacker->Sanity);

        sleep(1);
    }

     // Heathcliff:Wild Hunt – Skill 2 loses sanity
      if (strcasecmp(attacker->name, "Heathcliff:Wild Hunt") == 0 &&
         (atk == &attacker->skills[1]) && attacker->skills[0].active > 0) {

        updateSanity(attacker, -(10));
        if (attacker->Sanity < -45) attacker->Sanity = -45;

        printf("\n%s mounted 'Dullahan', loses 10 Sanity (%d)\n",
           attacker->name, attacker->Sanity);

          sleep(1);
      }

  // --------------------------- Hong lu:The Lord of Hongyuan -------------------------- 
  // Hong lu:The Lord of Hongyuan - S3-1 to S3-2
  if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 &&
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

    if (strcasecmp(HeshinPacks, "Mao") == 0) {

      printf("\n%s: Send the Hare.\n", attacker->name);

      sleep(1);

      printf("\n%s: Carve those pests out!\n", attacker->name);

      const char *savename = attacker->name;

      attacker->name = "Heshin Packs - Mao";

      clearTurnSkillBuffs(attacker);

      attackPhase(attacker, &attacker->skills[6], attacker->skills[6].Offense,
        attacker->skills[6].Defense, defender, defSkill,
        defTempOffense, defTempDefense, attacker->skills[6].Coins, 0, clashCount);
      
      attacker->name = savename;

       sleep(1);

      printf("\n%s: Mao...\n", attacker->name);

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

    } else if (strcasecmp(HeshinPacks, "Si") == 0) {

      printf("\n%s: Pierce with the Serpent.\n", attacker->name);

      sleep(1);

      printf("\n%s: Carve those pests out!\n", attacker->name);

      const char *savename = attacker->name;

      attacker->name = "Heshin Packs - Si";

      clearTurnSkillBuffs(attacker);

      attackPhase(attacker, &attacker->skills[7], attacker->skills[7].Offense,
        attacker->skills[7].Defense, defender, defSkill,
        defTempOffense, defTempDefense, attacker->skills[7].Coins, 0, clashCount);

      attacker->name = savename;

       sleep(1);

      printf("\n%s: Si...\n", attacker->name);

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

    } else if (strcasecmp(HeshinPacks, "Wu") == 0) {

      printf("\n%s: Intercept with the Horse.\n", attacker->name);

      sleep(1);

      printf("\n%s: Carve those pests out!\n", attacker->name);

      const char *savename = attacker->name;

      attacker->name = "Heshin Packs - Wu";

      clearTurnSkillBuffs(attacker);

      attackPhase(attacker, &attacker->skills[8], attacker->skills[8].Offense,
        attacker->skills[8].Defense, defender, defSkill,
        defTempOffense, defTempDefense, attacker->skills[8].Coins, 0, clashCount);

      attacker->name = savename;

       sleep(1);

      printf("\n%s: Wu...\n", attacker->name);

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

    } else if (strcasecmp(HeshinPacks, "You") == 0) {

      printf("\n%s: Charge forth, Rooster.\n", attacker->name);

      sleep(1);

      printf("\n%s: Carve those pests out!\n", attacker->name);

      const char *savename = attacker->name;

      attacker->name = "Heshin Packs - You";

      clearTurnSkillBuffs(attacker);

      attackPhase(attacker, &attacker->skills[9], attacker->skills[9].Offense,
        attacker->skills[9].Defense, defender, defSkill,
        defTempOffense, defTempDefense, attacker->skills[9].Coins, 0, clashCount);

      attacker->name = savename;

      updateSanity(attacker, 15);
      if (attacker->Sanity > 45) attacker->Sanity = 45;

      printf("\n%s heals 15 Sanity (%d)\n", attacker->name, attacker->Sanity);

       sleep(1);

      printf("\n%s: You...\n", attacker->name);

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

    printf("\n%s: Retreat. Your life still has a use to fulfil.\n",
      attacker->name);

    sleep(1);

  } else if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 &&
             atk == &attacker->skills[2]) {

     clearTurnSkillBuffs(attacker);

    printf("\n%s: I'll show you myself...\n", attacker->name);

    sleep(1);

    attackPhase(attacker, &attacker->skills[3], attacker->skills[3].Offense,
                attacker->skills[3].Defense, defender, defSkill, defTempOffense,
                defTempDefense, attacker->skills[3].Coins, 0, clashCount);
  }
 
  // Hong lu:The Lord of Hongyuan - S3-1 to S3-2
  if (strcasecmp(attacker->name, "Hong lu:The Lord of Hongyuan") == 0 &&
      atk == &attacker->skills[3]) {

    printf("\n%s: How Daguanyuan has been purged.\n", attacker->name);

    sleep(1);
  }

  // Hong lu:The Lord of Hongyuan - Lordsguard
  if (strcasecmp(defender->name, "Hong lu:The Lord of Hongyuan") == 0 &&
      defender->skills[5].active == 0 && (HeshinPacks != NULL)) {

    if (defender->HP <= 0) {

    defender->HP = 1;

    printf("\n%s: Retreat. Your life still has a use to fulfil.\n",
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

      printf("\n%s: I do not allow retreat until the enemy has been slain.\n",
         defender->name);
      
    }

    sleep(1);

  } // Hong lu:The Lord of Hongyuan - Passive
  else if ((strcasecmp(defender->name, "Hong lu:The Lord of Hongyuan")) == 0 &&
           defender->HP <= 0 && defender->skills[5].active == 1) {

    defender->skills[5].active = 0;

    printf(
        "\n%s's '%s' activeted! Nullity all damage; then apply 'Lordsguard' to "
        "all left Heishou Pack and bring %s's HP to 1 (Once per Encounter)\n",
        defender->name, defender->skills[5].name, defender->name);

    sleep(1);

    printf("\n%s: The Lord will not die.\n", defender->name);

    defender->HP = 1;

    sleep(1);
  }

  // Hong lu:The Lord of Hongyuan - Mao
  if (strcasecmp(attacker->name, "Heshin Packs - Mao") == 0 &&
      atk == &attacker->skills[6]) {

    defender->ClashPowerNextTurn -= 1;

    printf("\n%s gains 1 Clash Power Down next turn by %s's Skill\n", defender->name, attacker->name);

    sleep(1);

  } // Hong lu:The Lord of Hongyuan - Si
  else if (strcasecmp(attacker->name, "Heshin Packs - Si") == 0 &&
      atk == &attacker->skills[7]) {

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
  else if (strcasecmp(attacker->name, "Heshin Packs - Wu") == 0 &&
      atk == &attacker->skills[8]) {

    defender->ProtectionNextTurn -= 20;

    printf("\n%s takes +20%% damage next turn by %s's Skill\n", defender->name, attacker->name);

    sleep(1);

  }
  
  // --------------------------------------------------------------

  // Heishou Pack - You Branch Adept Heathcliff - Anti death Passive
  if ((strcasecmp(defender->name, "Heishou Pack - You Branch Adept Heathcliff")) == 0 &&
      defender->HP <= 0 && defender->skills[3].active == 0) {

    defender->skills[3].active = 1;

    printf("\n%s's 'Flame Rooster's Death Defiance [炎鳥不死戦]' activated! Nullity all damage; then bring %s's HP to 1 (Once per Encounter)\n",
           defender->name, defender->name);

    defender->HP = 1;

    sleep(1);

    printf("\n%s: Flame Rooster's Death Defiance [炎鳥不死戦]... Heh! You really thought I was gonna kick it... Huh?!\n",
       defender->name);

    sleep(1);
  }

  // Meursault:Blade Lineage Mentor - Passive
  if (strcasecmp(defender->name, "Meursault:Blade Lineage Mentor") == 0 &&
      defender->HP <= 0 && defender->Passive != 1) {

    defender->Passive = 1;

    printf("\n%s's 'Swordplay of the Homeland' activated! Nullity all "
           "damage; then bring %s's HP to 1 (Once per Encounter)\n",
           defender->name, defender->name);

    defender->HP = 1;

    sleep(1);
  }

    // Meursault:Blade Lineage Mentor - Passive
    if (strcasecmp(defender->name, "Meursault:Blade Lineage Mentor") == 0) {

      defender->FinalPowerBoostNextTurn += 1;

      printf("\n%s gain +1 Final Power next turn (Once per enemy's skill)\n",
             defender->name);

      sleep(1);
    }

  // Yi sang:Fell Bullet - Torn Memory
  if (strcasecmp(attacker->name, "Yi sang:Fell Bullet") == 0 &&
      (atk == &attacker->skills[0] && attacker->skills[0].active ||
       atk == &attacker->skills[1] && attacker->skills[1].active)) {

    attacker->Passive += (atk == &attacker->skills[0] ? 1 : 3);
    if (attacker->Passive >= 7) {
      attacker->Passive = 7;
      attacker->skills[2].Copies = 3;
    } else {
      attacker->skills[2].Copies = 1;
    }
    printf("\n%s gains %d Torn Memory(%d/7)\n", attacker->name,
           (atk == &attacker->skills[0] ? 1 : 3), attacker->Passive);

    sleep(1);
  }

     // Yi sang:Fell Bullet - Torn Memory S3 lost
    if (strcasecmp(attacker->name, "Yi sang:Fell Bullet") == 0 &&
        atk == &attacker->skills[2] && attacker->skills[2].active) {

      updateSanity(attacker, -(attacker->Passive*2));
      if (attacker->Sanity < -45) attacker->Sanity = -45;

      printf("\n%s loses (Torn Memory x 2) Sanity (%d - Max 14) (Sanity %d)\n",
             attacker->name, attacker->Passive*2, attacker->Sanity);

      sleep(1);
    }

  // Yi sang:Fell Bullet - Torn Memory S3 lost
  if (strcasecmp(attacker->name, "Yi sang:Fell Bullet") == 0 &&
      atk == &attacker->skills[2] && attacker->skills[2].active &&
      attacker->Passive >= 7) {

    updateSanity(attacker, 20);
    if (attacker->Sanity > 45) attacker->Sanity = 45;

    printf("\n%s loses all Torn Memory to gain 'Fell Bullet', All skills' Damage Multiplier +1.0 and heal 20 Sanity on self (%d)\n",
           attacker->name, attacker->Sanity);

    attacker->skills[0].DmgMutiplier += 1;
    attacker->skills[1].DmgMutiplier += 1;
    attacker->skills[2].DmgMutiplier += 1;
    attacker->skills[2].Copies = 1;

    attacker->Passive = 0;

    sleep(1);
  }

    // -------------------------------- Don Quixote:The Manager of La Manchaland --------------------------------

  // Don Quixote:The Manager of La Manchaland - Hardblood gains 5 for counter
  // won
  if (strcasecmp(attacker->name, "Don Quixote:The Manager of La Manchaland") ==
          0 &&
      (atk == &attacker->skills[6])) {

    attacker->Passive += 5;
    if (attacker->Passive > 30) attacker->Passive = 30;

    printf("\n%s gains 5 Hardblood(%d)\n", attacker->name, attacker->Passive);

    sleep(1);
  }

  //------------------------------------Sancho:The Second Kindred of Don Quixote-------------------
    
  // Sancho - Ultimate After attack
  if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
      (atk == &attacker->skills[12]) && attacker->Passive >= 1) {

     attacker->Passive -= 1;
    if (attacker->Passive < 1) attacker->Passive = 1;

    defender->ParalyzeNextTurn += 1;

      printf("\n%s consumes 1 Hardblood(%d) to inflict 1 Paralyze next turn (Fix the Power of 1 Coins to 0 for one turn)\n", attacker->name, attacker->Passive); 

    sleep(1);
  }

    // Sancho - Skill 11
    if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        (atk == &attacker->skills[10])) {

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
    if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") ==
            0 &&
        (atk == &attacker->skills[11])) {

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
     if (strcasecmp(attacker->name, "Sancho:The Second Kindred of Don Quixote") == 0 && ClashLostAttack == 0) {

        attacker->Passive += 3;
       if (attacker->Passive > 30) attacker->Passive = 30;
       
       printf("\n%s gains 3 Hardblood (%d)\n", attacker->name, attacker->Passive); 

       sleep(1);
     }

    // --------------------------------------------------

    // -------------------- Erlking Heathcliff ----------------------------
    // Erlking Heathcliff Skill 2 buff
    if (strcasecmp(attacker->name, "Erlking Heathcliff") == 0 && atk == &attacker->skills[2]) {

       attacker->AttackPowerBoostNextTurn += 3;
       attacker->DamageUpNextTurn += 30;

      printf("\n%s gains 3 Attack Power Up and +30%% damage next turn\n", attacker->name);

      sleep(1);
    }

    // Erlking Heathcliff Skill 3 debuff
    if (strcasecmp(attacker->name, "Erlking Heathcliff") == 0 && (atk == &attacker->skills[3])) {

         defender->ParalyzeNextTurn += 3;

      printf("\n%s inflict 3 Paralyze next turn (Fix the Power of 3 Coins to 0 for one turn)\n", attacker->name);

      sleep(1);
    }

// --------------------------------------------------------------
    
//------------------------------------------------------------------

  // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly
  if (strcasecmp(attacker->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0) {

      printf("\n%s consumed %d The Living & The Departed(%d)\n", attacker->name, attacker->skills[0].active, attacker->Passive); 

    sleep(1);
  }

  // Lobotomy E.G.O::Solemn Lament Yi Sang - SKill 2 gain
  if (strcasecmp(attacker->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 && atk == &attacker->skills[1]) {

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
  if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[4] || atk == &attacker->skills[9])) {
    
      defender->ClashPowerNextTurn -= 1;

    printf("\n%s gains 1 Clash Power Down next turn by %s's Skill\n", defender->name, attacker->name);

    sleep(1);
  }

  // Jia Qiu - S2 and S8
  if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[2] || atk == &attacker->skills[8])) {

        updateSanity(defender, -(5 + (1 * remainingCoins)));
    if (defender->Sanity < -45) defender->Sanity = -45;

    printf("\n%s loses 5 Sanity and loses 1 additional for every one remaining coins of %s's Skill(%d)\n", defender->name, attacker->name, remainingCoins);

    sleep(1);
  }

  // Jia Qiu - S12
  if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[12])) {
  
    int boost = 5*(abs(defender->Sanity));
  
        attacker->DamageUpNextTurn += boost;
  
    printf("\n%s deal +5%% damage for every Sanity enemy further from 0 next turn (%d%%)\n", attacker->name, boost);
  
    sleep(1);
  }

  // Jia Qiu - Passive
  if (strcasecmp(attacker->name, "Jia Qiu") == 0) {
      updateSanity(attacker, 5);
    if (attacker->Sanity > 45) attacker->Sanity = 45;

    printf("\n%s heals 5 Sanity (%d)\n", attacker->name, attacker->Sanity);
  }
  
  // Jia Qiu Phase 1.5
  if (strcasecmp(defender->name, "Jia Qiu") == 0 && defender->HP <= defender->MAX_HP * 0.85 &&
      defender->Passive == 1) {

    printf("\n%s: I am not yet to hear 'your' answer.\n", defender->name);

    sleep(1);

    printf("\n%s: That answer, hiding in the shadows of your hesitation.\n",
           defender->name);

    sleep(1);

    printf("\n%s: That answer that you buried deep within.\n",
           defender->name);

    defender->Passive = 2;

    // Disable the old
    for (int i = 0; i < 2; i++) {
      if (defender->skills[i].Copies > 0) {
        defender->skills[i] = defender->skills[3];
        defender->skills[i].Copies = 0;
      }
    }
    defender->skills[6] = defender->skills[3];
    defender->skills[6].Copies = 0;

    // Set copies for the newly mapped primary skills
     defender->skills[4].Copies = 4;
    defender->skills[2].Copies = 3;
    defender->skills[5].active = 0;
  }

  // Jia Qiu LAST Ult
  if (strcasecmp(attacker->name, "Jia Qiu") == 0 && (atk == &attacker->skills[16])) {

    defender->HP = 1;

    printf("\n%s's HP drop to 1\n", defender->name);
  }

  // -----------------------------------------------------------------

  sleep(1);
}
}




















  

// Returns effective skill and also temporary offense/defense for this turn
SkillStats *getEffectiveSkill(Character *c, Character *c2,
                              SkillStats *chosenSkill, int *tempOffense,
                              int *tempDefense) {
  *tempOffense = chosenSkill->Offense;
  *tempDefense = chosenSkill->Defense;

  // ---------------------------- Binah -----------------------------

  // Binah - Arbiter
  if (strcasecmp(c->name, "Binah") == 0 && !c->Passive) {

    c->DamageUp -= 20;
    c->FinalPowerBoost -= 1;

    printf("\n%s's 'Incomplete Arbiter' activated, deals -20%% damage, Final Power -1\n", c->name);

    sleep(1);
  } else if (strcasecmp(c->name, "Binah") == 0 && c->Passive) {

      c->DamageUp += 50;
      c->FinalPowerBoost += 2;

      printf("\n%s's 'An Arbiter' activated, gains +50%% damage, Final Power +2\n", c->name);

      sleep(1);
    }
  
  // Binah - heal Sanity
  if (strcasecmp(c->name, "Binah") == 0 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[3])) {

    updateSanity(c, 10);
    

    printf("\n%s heals 10 Sanity (%d)\n", c->name, c->Sanity);

    sleep(1);
  }

  // Binah - Skill 5 buff
  if (strcasecmp(c->name, "Binah") == 0 && (chosenSkill == &c->skills[4])) {

    c->Protection += 30;
    c->ProtectionNextTurn += 30;

    printf("\n%s takes -30%% damage for this turn and next turn\n", c->name);

    sleep(1);
  }

  // Binah - Fairy buff
  if (strcasecmp(c->name, "Binah") == 0 && c->skills[0].active > 0 && c->Passive) {

    int boost = c->skills[0].active * 20;

    c->DamageUp += boost;
    c->BasePowerBoost += c->skills[0].active*10;

    printf("\n%s deals +20%% damage(%d%%) and +10 Base Power(%d) for every Fairy on enemy (%d)\n", c->name, boost, c->skills[0].active*10, c->skills[0].active);

    sleep(1);
  }

  // -----------------------------------------------

  // -------------------------- Jia Qiu -----------------------------
  
 // Jia Qiu - heal Sanity on mao or zilu
  if (strcasecmp(c->name, "Jia Qiu") == 0 &&
      (chosenSkill == &c->skills[12] || chosenSkill == &c->skills[4] || chosenSkill == &c->skills[11])) {

    updateSanity(c, 10);
    

    printf("\n%s heals 10 Sanity (%d)\n", c->name, c->Sanity);

    sleep(1);
  }

  // Jia Qiu S15 Nerf
  if (strcasecmp(c->name, "Jia Qiu") == 0 &&
      (chosenSkill == &c->skills[15])) {

    if (strcasecmp(c2->name, "Hong lu:The Lord of Hongyuan") == 0) {
      
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
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
      chosenSkill == &c->skills[2] && c->skills[0].active > 0) {
    
    printf("\nWhen activated 'Dullahan', %s switching '%s' to Skill '%s'\n", c->name,
           chosenSkill->name, c->skills[3].name);

    chosenSkill = &c->skills[3];

    sleep(1);
  }

  // Wild hunt – Buff
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 && c->skills[0].active > 0) {

    *tempOffense += 3;
    *tempDefense -= 3;

    printf("\n%s mounted 'Dullahan', Offense +3, Defense -3\n",
           c->name);

    sleep(1);

    printf("\n%s: Dullahan! Time to ride for death.\n", c->name);

    sleep(1);

  }

  // Heathcliff:Wild Hunt – skill 4 lose sanity
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
      (chosenSkill == &c->skills[3])) {

      updateSanity(c, -(15));
      

    printf("\n%s loses 15 Sanity (%d)\n",
       c->name, c->Sanity);

    sleep(1);
  }
  

  // Heathcliff:Wild Hunt – gain coffin
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
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
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 && c->Passive >= 3) {

    int gain;

    gain = ((c->Passive/3) * 20);
    c->DamageUp += gain;

    printf("\n%s deals 20%% more damage(%d%%) for every 3 Coffin (%d)\n",
       c->name, gain, c->Passive);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – Clash power coffin
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 && c->Passive >= 5) {

    int gain;

    gain = c->Passive/5;
    c->ClashPower += gain;

    printf("\n%s gains 1 Clash Power(%d) for every 5 Coffin (%d)\n",
       c->name, gain, c->Passive);

    sleep(1);
  }

  // Heathcliff:Wild Hunt – buff coffin
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[1]) && c->Passive >= 4) {

    int gain = c->Passive/4;

    c->CoinPowerBoost += gain;

    printf("\n%s gains 1 Coin Power(%d) for every 4 Coffin (%d)\n",
       c->name, gain, c->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0]) && abs(c->Sanity - c2->Sanity) >= 10) {

    int gain = abs(c->Sanity - c2->Sanity)/10;
    if (gain > 2) gain = 2;

    c->CoinPowerBoost += gain;

    printf("\n%s gains 1 Coin Power for every 10 Sanity different (%d - Max 2)\n",
       c->name, gain);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0]) && c->Passive >= 3) {

    c->ClashPower += 1;

    printf("\n%s at 3+ Coffin(%d), Clash Power +1\n",
       c->name, c->Passive);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 1/2
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1]) && abs(c->Sanity - c2->Sanity) >= 10) {

    c->ClashPower += 1;

    printf("\n%s at 10+ Sanity different, Clash Power +1\n",
       c->name);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 3
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
     (chosenSkill == &c->skills[2]) && abs(c->Sanity - c2->Sanity) >= 15) {

    int gain = abs(c->Sanity - c2->Sanity)/15;
    if (gain > 4) gain = 4;

    c->CoinPowerBoost += gain;

    printf("\n%s gains 1 Coin Power for every 15 Sanity different (%d - Max 4)\n",
       c->name, gain);

      sleep(1);
  }

  // Heathcliff:Wild Hunt – buff Skill 4
  if (strcasecmp(c->name, "Heathcliff:Wild Hunt") == 0 &&
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
  if ((strcasecmp(c->name, "Meursault:Blade Lineage Mentor") == 0 &&
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

      printf("\n%s: If you will cut... then wager your life on it.\n", c->name);

    }

  //        Meursault:Blade Lineage Mentor - Skill 1 buff
  if ((strcasecmp(c->name, "Meursault:Blade Lineage Mentor") == 0 && chosenSkill == &c->skills[0])) {

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
  if ((strcasecmp(c->name, "Meursault:Blade Lineage Mentor") == 0 && chosenSkill == &c->skills[1])) {

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
  if (strcasecmp(c->name, "Meursault:The Thumb") == 0 &&
      chosenSkill == &c->skills[2] && c->skills[3].active && c->Passive > 0) {
    
    printf("\n%s at 1+ Savage Tigermark Round, switching '%s' to Skill '%s'\n", c->name,
           chosenSkill->name, c->skills[3].name);

    chosenSkill = &c->skills[3];

    sleep(1);
  }

  // Meursault: The Thumb – Clash power spend tigermark
  if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && !c->skills[3].active && c->Passive > 0) {

    printf("\n%s's Skill Coins spend 'Tigermark Round' gain +1 Final Power and deal +10%% damage (activates only as long as the Coin has Rounds left to spend)\n", c->name);

     c->FinalPowerBoost += 1;
    c->DamageUp += 10;

    sleep(1);
  }

  // Meursault: The Thumb – Clash power spend Savage tigermark
  if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && c->skills[3].active && c->Passive > 0) {

    printf("\n%s's Skill Coins spend 'Savage Tigermark Round' gain +2 Final Power, deal +30%% damage (activates only as long as the Coin has Rounds left to spend)\n", c->name);

    c->FinalPowerBoost += 2;
    c->DamageUp += 30;

    sleep(1);
  }

  // Meursault: The Thumb – S3-1 Unbreakable
  if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && chosenSkill == &c->skills[2] && c->Passive >= 1 && !c->skills[3].active && c->skills[2].active >= 3) {

    printf("\n%s at 1+ Tigermark Round and 3+ Tigermark Round spent, convert all Coins of 'Tanglecleaver' into Unbreakable Coins\n", c->name);

    chosenSkill->Unbreakable = chosenSkill->Coins;

    sleep(1);
  }

  // Meursault: The Thumb – all Unbreakable
  if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && c->Passive >= 1 && c->skills[3].active) {

    printf("\n%s at 1+ Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins\n", c->name);

     chosenSkill->Unbreakable = chosenSkill->Coins;

    sleep(1);
  }

  // Meursault: The Thumb – Overheat
  if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && c->Passive <= 0 && c->skills[3].active) {

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

  // Meursault:The Thumb - Skill 1 and 2 Final Power
  if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && abs(c->Sanity) >= 10 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

    int Buff = abs(c->Sanity);
    int FinalPowerbuff = Buff/10;
    int Max = 3;

    if (chosenSkill == &c->skills[1]) Max = 4;
    
     if (FinalPowerbuff > Max) FinalPowerbuff = Max;

     c->FinalPowerBoost += FinalPowerbuff;

      printf("\n%s gains +1 Final Power for every 10 Sanity further from 0 (%d - Max %d)\n", c->name, FinalPowerbuff, Max);

    sleep(1);
  }

  // Meursault:The Thumb - Skill 3 and 4 coin power
  if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && abs(c->Sanity) >= 20 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int Buff = abs(c->Sanity);
    int CoinPowerbuff = Buff/20;
     if (CoinPowerbuff > 2) CoinPowerbuff = 2;

     c->CoinPowerBoost += CoinPowerbuff;

      printf("\n%s gains +1 Coin Power for every 20 Sanity further from 0 (%d - Max 2)\n", c->name, CoinPowerbuff);

    sleep(1);
  }

  // --------------------------------------------
  
  // Shin buffs (temporary, print once per skill selection)
  if ((strcasecmp(c->name, "Meursault:The Thumb") == 0 &&
       c->skills[3].active) ||
      (strcasecmp(c->name, "Lei heng") == 0 && c->skills[4].active == 1 && c->skills[0].active != 3)) {

    if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && c->skills[2].active < 8) {

      int DamageBuff = (abs(c2->Sanity - c->Sanity)) > 20 ? 20 : (abs(c2->Sanity - c->Sanity));
      c->DamageUp += DamageBuff;
      
      printf("\n%s 'Unrelenting Spirit [剛氣]' activated! \n deal +1%% damage for every different Sanity between enemy and this unit (%d%% - Max 20%%)\n", c->name, DamageBuff);

    } else if (strcasecmp(c->name, "Meursault:The Thumb") == 0 && c->skills[2].active >= 8) {

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
      
      printf("\n%s: That's more like it. Y'all are firin' me up!\n", c->name);

      if (chosenSkill->Unbreakable < chosenSkill->Coins) {
        chosenSkill->Unbreakable = 1;
      }
    }

    sleep(1);
  }

  
 // ---------------------------- Lei heng -----------------------------
  if (strcasecmp(c->name, "Lei heng") == 0 && c->skills[4].active == 1 && c->skills[0].active == 3) {
    
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
    
     printf("\n%s: That's more like it. Y'all are firin' me up!\n", c->name);
 
    if (chosenSkill->Unbreakable < chosenSkill->Coins) {
      chosenSkill->Unbreakable = 1;
    }

  }

  // Lei heng – heal Sanity Passive and Buff dmg Passive
  if (strcasecmp(c->name, "Lei heng") == 0 && c->Sanity > -45) {

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
  if (strcasecmp(c->name, "Lei heng") == 0 && c->HP < c->MAX_HP * 0.4 &&
      (chosenSkill == &c->skills[2])) {

    chosenSkill = &c->skills[5];

    sleep(1);

  }

  // Lei heng – skill 5 buff
  if (strcasecmp(c->name, "Lei heng") == 0 && (chosenSkill == &c->skills[4])) {

    c->DamageUpNextTurn += 10;

    printf("\n%s gains 10%% more damage next turn\n", c->name);

    sleep(1);
  }

  // Lei heng – inner strength skill buff no blast skill 1
  if (strcasecmp(c->name, "Lei heng") == 0 &&
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
  else if (strcasecmp(c->name, "Lei heng") == 0 &&
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
    if (strcasecmp(c->name, "Lei heng") == 0 &&
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
    else if (strcasecmp(c->name, "Lei heng") == 0 &&
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
      if (strcasecmp(c->name, "Lei heng") == 0 &&
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
  if (strcasecmp(c->name, "Lei heng") == 0 &&
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
  else if (strcasecmp(c->name, "Lei heng") == 0 &&
      (chosenSkill == &c->skills[5])) {

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
    if (Boost > 50) {

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
  if (strcasecmp(c->name, "Erlking Heathcliff") == 0) {

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
  if (strcasecmp(c->name, "Erlking Heathcliff") == 0 && chosenSkill == &c->skills[5] && abs(c2->Sanity - c->Sanity) > 0) {

    int buff = abs(c2->Sanity - c->Sanity);
    if (buff > 2) buff = 2;

    c->ClashPower += buff;

    printf("\n%s gains 1 Clash Power for every 5 Sanity different (%d - Max 2)\n", c->name, buff);

    sleep(1);
  }

   // ------------------------------------------------------------
  
// ------------------------------ Gregor:Firefist ------------------------------
    // Gregor:Firefist – Reset passive
  if (strcasecmp(c->name, "Gregor:Firefist") == 0 && c->Passive <= 0) {
    
    printf("\n%s runs out of fuel, use '%s' instead!\n", c->name, c->skills[3].name);

    sleep(1);
    
    printf("\n%s: I've prepped plenty of fuel.\n", c->name);

     sleep(1);

    printf("\n%s regain District 12 Fuel to 100\n", c->name);
    
    c->Passive = 100;
    chosenSkill = &c->skills[3];

    sleep(1);
  }
  
  // Gregor:Firefist – Buff Passive
  if (strcasecmp(c->name, "Gregor:Firefist") == 0) {

    if (c2->HP <= (c2->MAX_HP *0.75) || c->HP <= (c->MAX_HP *0.75)) {

    if (c->skills[3].active > 0 && (c->skills[0].active + c->skills[1].active) < 30) {
      int boost = c->skills[3].active * 0.2;  // 0.2% per consumed fuel
      if (boost > 40) {
            boost = 40;
      }
      c->DamageUp += boost;
      printf("\n%s's HP or %s's HP at 75%% or less HP, Deal +0.2%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (%d%% - Max 40%%)\n",
         c2->name, c->name, boost);

       printf("\n%s: Let's see how much more of this you can take.\n", c->name);
      
    } else if (c->skills[3].active > 0 && (c->skills[0].active + c->skills[1].active) >= 30) {
      int boost = c->skills[3].active * 0.3;  // 0.3% per consumed fuel
      if (boost > 60) {
        boost = 60;
      }
      c->DamageUp += boost;
        printf("\n%s's HP or %s's HP at 75%% or less HP, and main target at 30+ (Burn Stack + Burn Count) (%d), Deal +0.3%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (%d%% - Max 60%%)\n",
           c2->name, c->name, (c->skills[0].active + c->skills[1].active), boost);

         printf("\n%s: Let's see how much more of this you can take.\n", c->name);
      }
    

    sleep(1);

    }
  }

  // Gregor:Firefist - S1 Coin power buff
  if (strcasecmp(c->name, "Gregor:Firefist") == 0 &&
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
  if (strcasecmp(c->name, "Gregor:Firefist") == 0 &&
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
  if (strcasecmp(c->name, "Gregor:Firefist") == 0 &&
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
  if (strcasecmp(c->name, "Gregor:Firefist") == 0 &&
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
    //------------------------------------------------------------------

  // --------------------- Roland --------------------------

  // Roland – Shin (心) - The Black Silence
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && strcasecmp(c2->name, "Binah") == 0 && c2->Passive == 1) {

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
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && c->HP <= c->MAX_HP * 0.5) {
    printf("\n%s: That's that, this is this.... , Offense +10, Defense -5\n",
           c->name);

    *tempOffense += 10;
    *tempDefense -= 5;

    sleep(1);
  }

  //Roland Furioso
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && chosenSkill->Copies != 0 && chosenSkill != &c->skills[9]) {

    chosenSkill->Copies = 0;

    int randomtaunt = rand() % 3 + 1;

    if (randomtaunt == 1) {
      printf("\n%s: You shall feel like I did....\n", c->name);
    } else if (randomtaunt == 2) {
      printf("\n%s: O sorrow... When will I free from you?\n", c->name);
    } else
    printf("\n%s: I have nothing but sorrow... And I want nothing more.\n", c->name);

    sleep(1);
  }

  // Roland – Skill 1 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[0]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 8) Boost = 8;
    
    printf("\n%s gains +1 Base Power (%d - Max 8) for every 4 Black Silence (%d)\n",
           c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;
    
    sleep(1);
  }

  // Roland – Skill 2 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[1]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 5) Boost = 5;

    printf("\n%s gains +1 Base Power (%d - Max 5) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);
  }

  // Roland – Skill 3 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[2]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 5) Boost = 5;

    printf("\n%s gains +1 Coin Power (%d - Max 5) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->CoinPowerBoost += Boost;

    sleep(1);
  }

  // Roland – Skill 4 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[3]) && c->Passive >= 3) {

    int Boost = c->Passive/3;
    if (Boost > 4) Boost = 4;

    printf("\n%s gains +1 Base Power (%d - Max 4) for every 3 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);
  }

  // Roland – Skill 5 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[4]) && c->Passive >= 4) {

    int Boost = c->Passive/4;
    if (Boost > 4) Boost = 4;

    printf("\n%s gains +1 Base Power (%d - Max 4) for every 4 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);
  }

  // Roland – Skill 6 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[5]) && c->Passive >= 4) {

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
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[6]) && c->Passive >= 5) {

    int Boost = c->Passive/5;
    if (Boost > 3) Boost = 3;

    printf("\n%s gains +1 Base Power (%d - Max 3) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->BasePowerBoost += Boost;

    sleep(1);

  }

  // Roland – Skill 8 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[7]) && c->Passive >= 5) {

    int Boost = c->Passive/5;
    if (Boost > 3) Boost = 3;

    printf("\n%s gains +1 Coin Power (%d - Max 3) for every 5 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    c->CoinPowerBoost += Boost;

    sleep(1);

  }

  // Roland – Skill 9 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[8]) && c->Passive >= 10) {

    int Boost = c->Passive/10;
    if (Boost > 2) Boost = 2;

    printf("\n%s converts 1 Coin to Unbreakable Coin (%d - Max 2) for every 10 Black Silence (%d)\n",
         c->name, Boost, c->Passive);

    if (chosenSkill->Unbreakable < chosenSkill->Coins) {
      chosenSkill->Unbreakable += Boost;
    }

    sleep(1);

  }

  // Roland – Skill 10 buff
  if (strcasecmp(c->name, "Fixer grade 9?") == 0 && (chosenSkill == &c->skills[9]) && c->Passive >= 5) {

    int buff = c->Passive/5;
    if (buff > 5) buff = 5;

    printf("\n%s gain +1 Base Power for every 5 Black Silence (%d) (%d - Max 5)\n",
      c->name, c->Passive, buff);

      c->BasePowerBoost += buff;

    sleep(1);
  }

  // Roland – Skill 10 buff for shin (心)
    if (strcasecmp(c->name, "Fixer grade 9?") == 0 && strcasecmp(c2->name, "Binah") == 0 && c2->Passive == 1 && (chosenSkill == &c->skills[9])) {

      int buff = c->Passive;
      if (buff > 20) buff = 20;

      printf("\nIf %s has 'Shin (心) - The Black Silence', gain +1 Base Power (%d - Max 20) for every 1 Black Silence (%d)\n",
        c->name, buff, c->Passive);

        c->BasePowerBoost += buff;

      sleep(1);
    }

  // ---------------------------------------------------------------------

  // Hong lu:The Lord of Hongyuan - Skill 1 deal more damage on HP
  if (strcasecmp(c->name, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[0])) {

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
    float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

    int Boost = abs((int)(missingSelf - missingEnemy)) / 6;
    if (Boost > 1) Boost = 1;

     if (Boost > 0) {

    c->CoinPowerBoost += Boost;

      printf("\n%s at 6%% HP different, Coin Power +1\n", c->name);

    sleep(1);

     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 2 deal more damage on HP
  if (strcasecmp(c->name, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[1])) {

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
    float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

    int Boost = abs((int)(missingSelf - missingEnemy)) / 6;
    if (Boost > 2) Boost = 2;

     if (Boost > 0) {

    c->CoinPowerBoost += Boost;

      printf("\n%s gains +1 Coin Power for every 6%% HP different (%d - Max 2)\n", c->name, Boost);

    sleep(1);

     }
  }

  // Hong lu:The Lord of Hongyuan - Skill 3 deal more damage on HP
  if (strcasecmp(c->name, "Hong lu:The Lord of Hongyuan") == 0 && (chosenSkill == &c->skills[2])) {

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
    float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

    int Boost = abs((int)(missingSelf - missingEnemy)) / 4;
    if (Boost > 3) Boost = 3;

     if (Boost > 0) {

    c->CoinPowerBoost += Boost;

      printf("\n%s gains +1 Coin Power for every 4%% HP different (%d - Max 3)\n", c->name, Boost);

    sleep(1);
     }
  }


  // --------------------------- Yi sang:Fell Bullet -----------------
  // Yi sang:Fell Bullet - Torn Memory
  if (strcasecmp(c->name, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[0] && c->skills[0].active) {

    if (c->Passive < 7) {
      c->Passive += 1;
      if (c->Passive >= 7) {
        c->Passive = 7;
      }
      printf("\n%s gains 1 Torn Memory(%d/7)\n", c->name, c->Passive);
    }

    sleep(1);
  }

  // Yi sang:Fell Bullet - Torn Memory
  if (strcasecmp(c->name, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[2] && c->skills[2].active) {

    if (c->Passive >= 2) {

    printf("\n%s gains +1 Base Power(%d) for every 2 Torn Memory (%d)\n", c->name, c->Passive/2, c->Passive);

       c->BasePowerBoost += c->Passive/2;

    sleep(1);

    }

    if (c->Passive >= 3) {

    printf("\n%s gains +1 Coin Power(%d) for every 3 Torn Memory (%d)\n", c->name, c->Passive/3, c->Passive);

    c->CoinPowerBoost += c->Passive/3;

    sleep(1);

    }

  }

  // Yi sang:Fell Bullet - Torn Memory Buff s1
  if (strcasecmp(c->name, "Yi sang:Fell Bullet") == 0 &&
      chosenSkill == &c->skills[0]) {

    if (c->Passive >= 3) {
    int gain = c->Passive / 3;
    c->BasePowerBoost += gain;

    printf("\n%s gains +1 Base Power(%d) for every 3 Torn Memory (%d)\n", c->name,
           gain, c->Passive);

    sleep(1);
    }

    if (c->Passive >= 3) {
    int gain = c->Passive / 3;
    c->CoinPowerBoost += gain;

    printf("\n%s gains +1 Coin Power(%d) for every 3 Torn Memory (%d)\n", c->name,
           gain, c->Passive);

    sleep(1);
  }
  }

    // Yi sang:Fell Bullet - Torn Memory Buff s2
    if (strcasecmp(c->name, "Yi sang:Fell Bullet") == 0 &&
        chosenSkill == &c->skills[1]) {

      if (c->Passive >= 3) {
      int gain = c->Passive / 3;
      c->CoinPowerBoost += gain;

      printf("\n%s gains +1 Coin Power(%d) for every 3 Torn Memory (%d)\n", c->name,
              gain, c->Passive);

      sleep(1);
    }
    }
      //---------------------------------------------

  // -------------------- Don Quixote:The Manager of La Manchaland ---------------------------

  // Don Quixote:The Manager of La Manchaland - S3-1 Buff S3-2
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") == 0 &&
      chosenSkill == &c->skills[2] && c->Passive >= 15) {

    chosenSkill = &c->skills[5];

    printf("\n%s has 15+ Hardblood(%d), The Manager of La Manchaland Don "
           "Quixote: Empower Skill 3\n",
           c->name, c->Passive);

    sleep(1);

  }

  // Don Quixote:The Manager of La Manchaland - Empower Skill
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") == 0 &&
      chosenSkill == &c->skills[0] && c->Passive >= 15) {

    int Hardblood = 10;

    chosenSkill = &c->skills[3];

    printf("\n%s has 15+ Hardblood(%d), The Priest of La Manchaland Gregor: "
           "Empower Skill 1\n",
           c->name, c->Passive);

    sleep(1);

  } else if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             chosenSkill == &c->skills[1] && c->Passive >= 15) {

    int Hardblood = 10;

    chosenSkill = &c->skills[4];

    printf("\n%s has 15+ Hardblood(%d), The Barber of La Manchaland Outis: "
           "Empower Skill 2\n",
           c->name, c->Passive);

    sleep(1);

  }
  
  // Don Quixote:The Manager of La Manchaland - Hardblood gains 1-5
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") ==
          0 &&
      (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1] ||
    chosenSkill == &c->skills[2])) {

    c->HP -= (int)(c->MAX_HP * 0.03);
    if (c->HP < 1) c->HP = 1;

    int Hardblood = rand() % 3 + 1;

    c->Passive += Hardblood;
    if (c->Passive > 30) c->Passive = 30;

    printf("\n%s consumes 3%% of Max HP(%d) to gain %d Hardblood (%d) (this damage does not lower the unit's HP below 1)\n", c->name, (int)(c->MAX_HP * 0.03), Hardblood,
           c->Passive);

    sleep(1);
  }

  // Don Quixote:The Manager of La Manchaland - Hardblood heal for counter-2 won
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") ==
          0 &&
      (chosenSkill == &c->skills[7])) {

    c->Shield += 5 * (c->Passive);

    printf("\n%s gains 5 Shield HP(%.2f) for every Hardblood consumed (%d)\n", c->name, c->Shield, c->Passive);

    sleep(1);
  }

  // Don Quixote:The Manager of La Manchaland if HP ≤ 30 gains buff
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") == 0 &&
      c->HP <= c->MAX_HP*0.3) {

    c->Passive += 3;

    printf("\n%s at 30%% or less HP, 'Responsibility' activated!, Clash Power +1, Deal +20%% damage, Take +20%% damage, "
           "and gains 3 Hardblood(%d)\n",
           c->name, c->Passive);

    c->ClashPower += 1;
    c->DamageUp += 20;
    c->Protection -= 20;

    sleep(1);
  }

  // Don Quixote:The Manager of La Manchaland - Skill 1/2 in both Buff Coin
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") ==
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

  // Don Quixote:The Manager of La Manchaland - Skill 3 in both Buff Coin
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") ==
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
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             (chosenSkill == &c->skills[3])) {

    float HPmissing = (c->MAX_HP - c->HP) / c->MAX_HP; // 0.0 - 1.0
    int damageUP = 2 * (HPmissing * 100);
    if (damageUP > 50) damageUP = 50;
    
    c->DamageUp += damageUP;

    printf("\n%s deals +2%% damage for every missing HP on self (%d%% - Max 50%%)\n", c->name, damageUP);

    sleep(1);
  }
  
  // Don Quixote:The Manager of La Manchaland - Skill 3 both Buff dmg
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") ==
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

      int boost = (c->Passive/5) * 15;
      if (boost > 50 ) boost = 50;

      c->DamageUp += boost;

      printf("\n%s deals +15%% damage(%d%% - Max 50%%) for every 5 Hardblood (%d)\n", c->name, boost, c->Passive);

      sleep(1);\
      
    } else if (c->Passive >= 5 && chosenSkill == &c->skills[5]) {

    int boost = (c->Passive/5) * 20;
    if (boost > 50) boost = 50;

    c->DamageUp += boost;

    printf("\n%s deals +20%% damage(%d%% - Max 50%%) for every 5 Hardblood (%d)\n", c->name, boost, c->Passive);

    sleep(1);
    }
  }
  
  // Don Quixote:The Manager of La Manchaland - Skill 3-2 Buff base
  if (strcasecmp(c->name, "Don Quixote:The Manager of La Manchaland") ==
                 0 &&
             (chosenSkill == &c->skills[5])) {

    int boost = c->Passive / 10;
     if (boost > 3) boost = 3;

    c->DamageUp += boost;

    printf("\n%s gains 1 Base Power (%d - Max 3) for every 10 Hardblood on self (%d)\n", c->name, boost, c->Passive);

    sleep(1);
  }

  // ----------------------------------------------------------

  

  //-----------------------Sancho:The Second Kindred of Don Quixote ---------------------
  
  // Sancho:The Second Kindred of Don Quixote - Hardblood gains 1
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
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

    } else if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0 && (chosenSkill == &c->skills[5]) && c->HP > 1) {

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
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0) {

    int Hardblood = 1;

    c->Passive += Hardblood;
    if (c->Passive > 30) c->Passive = 30;

    printf("\n%s gains +%d Hardblood(%d)\n", c->name, Hardblood, c->Passive);

    sleep(1);
  }
  
  // Sancho:The Second Kindred of Don Quixote - Hardblood gains 5 with skill 9
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      (chosenSkill == &c->skills[9])) {

    int Hardblood = 5;

    c->Passive += Hardblood;
    if (c->Passive > 30) c->Passive = 30;
    c->Shield += (int)(c->MAX_HP*0.1);

    printf("\n%s gains +%d Hardblood(%d) and %d Shield HP(%.2f)\n", c->name, Hardblood, c->Passive, (int)(c->MAX_HP*0.1), c->Shield);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Hardblood Buff
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0 && c->Passive >= 10 && c->Passive < 20) {

    int boost = (int)(c->Passive / 6);

    *tempOffense += boost;
    *tempDefense += boost;

    printf("\n%s has 10+ Hardblood, gains +1 Offense and Defense(%d) for every 6 stacks(%d)\n", c->name, boost, c->Passive);

    sleep(1);
  } else if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0 && c->Passive >= 20) {

    int boost = 2 * (int)(c->Passive / 6);
    
    *tempOffense += boost;
    *tempDefense += boost;

    printf("\n%s has 20+ Hardblood, gains +2 Offense and Defense(%d) for every 6 stacks(%d)\n", c->name, boost, c->Passive);

    sleep(1);
  }

  // Sancho:The Second Kindred of Don Quixote - Skill 4/5 Buff Coin
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") ==
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
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") ==
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
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") ==
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
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") ==
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
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
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
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") ==
          0 &&
    chosenSkill == &c->skills[12] && c->HP > 1) {

    if (c->Passive >= 5) {
    c->Passive -= 5;
    if (c->Passive < 1) c->Passive = 1;

    printf("\n%s consumes 5 Hardblood(%d) to gain Clash Power +5\n",
           c->name, c->Passive);
    } else if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") ==
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
  if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") ==
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
    if (strcasecmp(c->name, "Sancho:The Second Kindred of Don Quixote") ==  0 &&
        (chosenSkill == &c->skills[5]) && c->Passive >= 4) {

      c->Passive -= 4;
      if (c->Passive < 1) c->Passive = 1;

      updateSanity(c, 10);
      

      printf("\n%s consumes 4 Hardblood(%d) to heal 10 Sanity (%d)\n",
             c->name, c->Passive, c->Sanity);

      sleep(1);
    }
    //-----------------------------------------------------------------------------------------


  // Lobotomy E.G.O::Solemn Lament Yi Sang - Clash power
  if (strcasecmp(c->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
          0 && (chosenSkill != &c->skills[2])) {

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
  else if (strcasecmp(c->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") ==
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

  // ------------------------ Dawn Office Fixer Sinclair ----------------------------
  
  // Dawn Office Fixer Sinclair - Skill Buff S2
  if (strcasecmp(c->name, "Dawn Office Fixer Sinclair") == 0 && !c->skills[3].active && c->Sanity >= 20 && (chosenSkill == &c->skills[1])) {

    c->ClashPower += 1;

    printf("\n%s at 20+ Sanity, gains 1 Clash Power\n",
           c->name);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff base form S1
  if (strcasecmp(c->name, "Dawn Office Fixer Sinclair") == 0 && c->Sanity >= 10 && (chosenSkill == &c->skills[0])) {

    int boost = c->Sanity/10;
    if (boost > 2) boost = 2;

    c->BasePowerBoost += boost;

    printf("\n%s at 10+ Sanity, gains 1 Base Power for every 10 Sanity(%d - Max 2) (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff base form S3
  if (strcasecmp(c->name, "Dawn Office Fixer Sinclair") == 0 && !c->skills[3].active && c->Sanity >= 10 && (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[2])) {

    int boost = c->Sanity/10;
    if (boost > 2) boost = 2;
    
    c->CoinPowerBoost += boost;

    printf("\n%s at 10+ Sanity, gains 1 Coin Power for every 10 Sanity(%d - Max 2) (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S2
  if (strcasecmp(c->name, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && (chosenSkill == &c->skills[1])) {

    updateSanity(c, -(5));
    
    c->AttackPowerBoost += 5;

    printf("\n%s is in a Volatile E.G.O State consumes 5 Sanity (%d) to gain +5 Attack Power\n",
           c->name, c->Sanity);

    sleep(1);

  }

  // Dawn Office Fixer Sinclair - Skill Buff EGO form S4
  if (strcasecmp(c->name, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && (chosenSkill == &c->skills[3] || chosenSkill == &c->skills[2])) {

    if (c->Sanity >= 20) {
    int boost = 2*(c->Sanity/20);

    c->CoinPowerBoost += boost;

    printf("\n%s gains 2 Coin Power(%d) for every 20 Sanity (%d Sanity)\n",
           c->name, boost, c->Sanity);

    sleep(1);

    }

  }
    
  // Dawn Office Fixer Sinclair - Skill Buff base form
 if (strcasecmp(c->name, "Dawn Office Fixer Sinclair") == 0 && !c->skills[3].active && c->Sanity >= 20) {

      c->ClashPower += c->Sanity/20;

      printf("\n%s at 20+ Sanity, gains 1 Clash Power(%d) for every 20 Sanity (%d)\n",
             c->name, c->Sanity/20, c->Sanity);

      sleep(1);

    }
  
  // Dawn Office Fixer Sinclair - Skill Buff Ego form
 if (strcasecmp(c->name, "Dawn Office Fixer Sinclair") == 0 && c->skills[3].active && c->Sanity >= 20) {

      c->CoinPowerBoost += c->Sanity/20;

      printf("\n%s at 20+ Sanity, gains 1 Coin Power(%d) for every 20 Sanity (%d)\n",
             c->name, c->Sanity/20, c->Sanity);

      sleep(1);

    }
  
  // ------------------------------------------------------------------------------

  // --------------------------- Heishou Pack - You Branch Adept Heathcliff -----------------------
  
  // Heishou Pack - You Branch Adept Heathcliff - Bloody Storm of Blades
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->HP <= c->MAX_HP * 0.8) {

    int gain = ((c->MAX_HP - c->HP)/c->MAX_HP) / 0.2;
    if (gain > 3) gain = 3;

    *tempOffense += gain;
    *tempDefense += gain;
    
      printf("\n%s gains 1 Offense (%d) and 1 Defense (%d) for every 20%% missing HP (Max 3 each)\n", c->name, gain, gain);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 4
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->Passive >= 20 && chosenSkill == &c->skills[2]) {

    chosenSkill = &c->skills[3];

      printf("\n%s's Battleblood Instinct at 20+ Stack, activate '%s' instead\n", c->name, c->skills[3].name);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 1 and 2 Clash power
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->skills[0].active >= 10 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

    int clashpower = c->skills[0].active/10;
     if (clashpower > 2) clashpower = 2;

     c->ClashPower += clashpower;

      printf("\n%s gains +1 Clash Power for every 10 Burn Stack(%d) on self (%d - Max 2)\n", c->name, c->skills[0].active, clashpower);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 1 and 2 coin power
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (c->skills[0].active + c->skills[1].active) >= 6 && (chosenSkill == &c->skills[0] || chosenSkill == &c->skills[1])) {

     c->CoinPowerBoost += 1;

      printf("\n%s at 6+ Burn (Stack(%d) + Count(%d)) on self, gains +1 Coin Power\n", c->name, c->skills[0].active, c->skills[1].active);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 and 4 coin power
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (c->skills[0].active + c->skills[1].active) >= 6 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int Buff = (c->skills[0].active + c->skills[1].active)/6;
     if (Buff > 2) Buff = 2;

     c->CoinPowerBoost += Buff;

      printf("\n%s gains +1 Coin Power for every 6 Burn (Stack(%d) + Count(%d)) on self (%d - Max 2)\n", c->name, c->skills[0].active, c->skills[1].active, Buff);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 and 4 Clash power
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && c->HP <= c->MAX_HP * 0.8 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    int clashpower = (c->MAX_HP - c->HP)/c->MAX_HP / 0.2;
     if (clashpower > 3) clashpower = 3;

     c->ClashPower += clashpower;

      printf("\n%s gains +1 Clash Power for every 20%% missing HP on self (%d - Max 3)\n", c->name, clashpower);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 deal more damage on burn
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[2]) && c->skills[0].active > 0) {

    float damageboost = c->skills[0].active * 1.5f;
    if (damageboost > 30.0f) damageboost = 30.0f;
    c->DamageUp += damageboost;

      printf("\n%s deals +1.5%% damage for every Burn Stack on self (%.1f%% - Max 30%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 4 deal more damage on burn
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[3]) && c->skills[0].active > 0) {

    int damageboost = c->skills[0].active * 3;
    if (damageboost > 30) damageboost = 30;
    c->DamageUp += damageboost;

      printf("\n%s deals +3%% damage for every Burn Stack on self (%d%% - Max 30%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 4 deal more damage on HP
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[3])) {

    float missingSelf  = (float)(c->MAX_HP  - c->HP)  / c->MAX_HP * 100.0f;
    float missingEnemy = (float)(c2->MAX_HP - c2->HP) / c2->MAX_HP * 100.0f;

    float damageboost = (missingSelf + missingEnemy) / 2.0f;
    if (damageboost > 50.0f) damageboost = 50.0f;
    
    c->DamageUp += damageboost;

      printf("\n%s deals +1%% damage for every 2%% (missing HP percentage on target + missing HP percentage on self) (%.0f%% - Max 50%%)\n", c->name, damageboost);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 coins
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[2]) && (c->skills[2].active > 0 || c->HP < c->MAX_HP*0.5)) {

    c->skills[2].Unbreakable = c->skills[2].Coins;

      printf("\n%s has Bloodflame [血炎], or less than 50%% HP, convert all Coins to Unbreakable Coins\n", c->name);

    sleep(1);
    
  }

  // Heishou Pack - You Branch Adept Heathcliff - Skill 3 and 4 Gain 3 Bloodflame [血炎] 
  if (strcasecmp(c->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (chosenSkill == &c->skills[2] || chosenSkill == &c->skills[3])) {

    c->skills[2].active += 3;
    if (c->skills[2].active > 3) c->skills[2].active = 3;
 
      printf("\n%s gains 3 Bloodflame [血炎] (%d - Max 3)\n", c->name, c->skills[2].active);

    sleep(1);
  }
  
  // ---------------------------------------------------------
  
  applySanityDebuff(tempOffense, tempDefense, c);

  return chosenSkill;
}









//-----------------------Clashing phase-------------------------------
ClashResult clashPhase(Character *p1, SkillStats *s1, int playerTempOffense,
                       int playerTempDefense, Character *p2, SkillStats *s2,
                       int enemyTempOffense, int enemyTempDefense,
                       Character *fullPlayer, int PContinueUnbreakCoin, int EContinueUnbreakCoin) {
  
  printf("\n--- Clash Phase ---");

  // --------------------- Clash Phase ----------------------

  // Heishou Pack - You Branch Adept Heathcliff passive gain on clash
  if (strcasecmp(p1->name, "Heishou Pack - You Branch Adept Heathcliff") == 0) {
    
    p1->Passive += 3;
    if (p1->Passive > 20) p1->Passive = 20;
    
    printf("\n\n%s gains 3 Battleblood Instinct (%d)\n", p1->name, p1->Passive);

    sleep(1);
  }

  // Heishou Pack - You Branch Adept Heathcliff Skill 1 and 2 gain on clash
  if (strcasecmp(p1->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (s1 == &p1->skills[0] || s1 == &p1->skills[1])) {

    p1->skills[0].active++;
    if (p1->skills[0].active > 99) p1->skills[0].active = 99;
    p1->skills[1].active++;
    if (p1->skills[1].active > 99) p1->skills[1].active = 99;
    
    printf("\n%s applies +1 Burn Stack(%d) and +1 Burn Count(%d) on self\n", p1->name, p1->skills[0].active, p1->skills[1].active);

    sleep(1);
  }

  // --------------------------------------------------------

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

  while (playerCoins > 0 && enemyCoins > 0) {
    printf("\nRound %d:\nPlayer:\t\tEnemy:\n", round);

    clashCount++; // Increment clash count each round

    int maxToss = (playerCoins > enemyCoins) ? playerCoins : enemyCoins;

    int playerTotal = s1->BasePower + p1->BasePowerBoost;
    int enemyTotal = s2->BasePower + p2->BasePowerBoost;

    double roundDelay = 0.5 - (round - 1) * 0.1;
    if (roundDelay < 0.1)
      roundDelay = 0.1;

    printf("%-10d %-10d\n", playerTotal, enemyTotal);
    usleep((int)(roundDelay * 500000));

    // In coin toss section:
    for (int i = 0; i < maxToss; i++) {
      if (i < playerCoins && tossCoinWithSanity(p1)) { // Pass character
        // Check paralyze
        if (p1->Paralyze > 0) { // ← Character's paralyze
          playerTotal += 0;
          p1->Paralyze--; // ← Character's paralyze
        } else {
          playerTotal += s1->CoinPower + p1->CoinPowerBoost;
          if (playerTotal <= 0) playerTotal = 0;
        }
      }
      if (i < enemyCoins && tossCoinWithSanity(p2)) { // Pass character
        // Check paralyze
        if (p2->Paralyze > 0) { // ← Character's paralyze
          enemyTotal += 0;
          p2->Paralyze--; // ← Character's paralyze
        } else {
          enemyTotal += s2->CoinPower + p2->CoinPowerBoost;
          if (enemyTotal <= 0) enemyTotal = 0;
        }
      }

      // Last coin offense bonus
      int bonus = 0;

      // Meursault:Blade Lineage Mentor's Passive
      if (!(strcasecmp(p1->name, "Meursault:Blade Lineage Mentor") == 0 && (s1 == &p1->skills[2]))){
        
      // Check if this is player's last coin
      if (i == playerCoins - 1 &&
          (playerTempOffense > enemyTempOffense || p1->ClashPower != 0 ||
           p1->FinalPowerBoost != 0)) {

        // Calculate player's clash bonus
        int offenseBonus = ((playerTempOffense - enemyTempOffense) / 3);
        if (offenseBonus < 0)
          offenseBonus = 0; // Only positive bonuses

        bonus = p1->ClashPower + p1->FinalPowerBoost + offenseBonus;

        if (bonus != 0) {
          playerTotal += bonus;
          if (playerTotal <= 0) playerTotal = 0;
          printf("Player Clash Power bonus applied: %d\n", bonus);
        }
      } 

      }
      
      // Check if this is enemy's last coin
      if (i == enemyCoins - 1 &&
               (enemyTempOffense > playerTempOffense || p2->ClashPower != 0 ||
                p2->FinalPowerBoost != 0)) {

        // Calculate enemy's clash bonus
        int offenseBonus = ((enemyTempOffense - playerTempOffense) / 3);
        if (offenseBonus < 0)
          offenseBonus = 0; // Only positive bonuses

        bonus = p2->ClashPower + p2->FinalPowerBoost + offenseBonus;

        if (bonus != 0) {
          enemyTotal += bonus;
          if (enemyTotal <= 0) enemyTotal = 0;
          printf("Enemy Clash Power bonus applied: %d\n", bonus);
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
    }



    
    // Determine clash result
    if (playerTotal > enemyTotal && enemyCoins > 0) {
      printf("Player wins this clash! Enemy loses 1 coin.\n");
      enemyCoins--;


      sleep(1);

      // Update Sanity based on clash count
      if (p1->hasSanity && enemyCoins <= 0) {
        int gain = calculateSanityGain(p1, clashCount);
        updateSanity(p1, gain);

        if (ClashPity) {
        ClashPity = 0; // Clash Pity turn off cause player finally won clash
        }
        
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
      }





       //Unbreakable lost
      if (s2->Unbreakable > 0 && enemyUnbreakableLost < s2->Unbreakable && s2->Unbreakable > enemyCoins) {
        enemyUnbreakableLost++;
      }






      // Heishou Pack - You Branch Adept Heathcliff Skill 4 won
      if (strcasecmp(p1->name, "Heishou Pack - You Branch Adept Heathcliff") == 0 &&
          s1 == &p1->skills[3] && enemyCoins <= 0) {

        p1->skills[0].active += 10;
        if (p1->skills[0].active > 99) p1->skills[0].active = 99;
        
        printf("\n%s won the Clash, %s gains 10 Burn Stack (%d)\n", p1->name, p1->name, p1->skills[0].active);

         sleep(1);
      }

      // ---------------------- Lobotomy E.G.O::Solemn Lament Yi Sang -------------------
      
      // Lobotomy E.G.O::Solemn Lament Yi Sang Skill 1 won
      if (strcasecmp(p1->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
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
      }

      }

      // Lobotomy E.G.O::Solemn Lament Yi Sang Skill 2 won
      if (strcasecmp(p1->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
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
        }
        
      }

      // Lobotomy E.G.O::Solemn Lament Yi Sang Skill 2 won
      if (strcasecmp(p1->name, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 &&
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
        }
        
      }

      // Sukuna:King of Curse - skill 8 Clash lost
      if (strcasecmp(p2->name, "Sukuna:King of Curse") == 0 && enemyCoins <= 0 && s2 == &p2->skills[8]) {

        p2->Protection -= 30;
        p2->DamageUp -= 80;

        printf("\n%s lost the Clash, deals -80%% damage and takes +30%% damage\n",
          p2->name);

        sleep(1);
      }


      // -----------------------------------------------------------------

      // Jia Qiu S15 lost
      if (strcasecmp(p2->name, "Jia Qiu") == 0 && enemyCoins <= 0 && s2 == &p2->skills[15]) {
        for (int i = 0; i < p2->numSkills; i++) {
          p2->skills[i].Defense -= 24;
        }
        p2->DamageUp -= 50;
        printf("\n%s lost the Clash, Deal -50%% damage and Defense -24 for this Encounter\n", p2->name);

         sleep(1);
      }
      
      // Meursault:Blade Lineage Mentor Skill 3 won
      if (strcasecmp(p1->name, "Meursault:Blade Lineage Mentor") == 0 &&
          s1 == &p1->skills[2] && enemyCoins <= 0) {
        p1->AttackPowerBoostNextTurn += 3;
        printf("\n%s won the Clash, gains 3 Attack Power Up next turn\n", p1->name);

         sleep(1);
      }
      
      // Meursault:Blade Lineage Mentor Skill 2 won
      if (strcasecmp(p1->name, "Meursault:Blade Lineage Mentor") == 0 &&
          s1 == &p1->skills[1] && enemyCoins <= 0) {
        p1->FinalPowerBoostNextTurn += 1;
        printf("\n%s won the Clash, gains 1 Final Power Up next turn\n", p1->name);

         sleep(1);
      }

      // Heathcliff:Wild Hunt – skill 3 heal sanity
      if (strcasecmp(p1->name, "Heathcliff:Wild Hunt") == 0 &&
          s1 == &p1->skills[2] && enemyCoins <= 0) {
        updateSanity(p1, 10);
        if (p1->Sanity > 45) p1->Sanity = 45;
        printf("\n%s won the Clash, heals 10 Sanity (%d)\n", p1->name, p1->Sanity);

         sleep(1);
      }

      //------------------------- Roland ---------------------------

      // gain Black Silence when lost
      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 && enemyCoins <= 0) {
        p2->Passive += 1;
        if (p2->Passive > 60) p2->Passive = 60;
        printf("\n%s lost the Clash, gains 1 Black Silence(%d - Max 60)\n", p2->name, p2->Passive);

         sleep(1);
      }
      
      // lose Black Silence
      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 &&
          (s2 == &p2->skills[8] || s2 == &p2->skills[1] || s2 == &p2->skills[2]) && enemyCoins <= 0) {
        p2->Passive -= 2;
        if (p2->Passive < 0) p2->Passive = 0;
        p2->Protection -= 20;
        printf("\n%s lost the Clash, loses 2 Black Silence(%d) and take 20%% more damage\n", p2->name, p2->Passive);

         sleep(1);
      }

      // lose Black Silence
      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 &&
          (s2 == &p2->skills[4]) && enemyCoins <= 0 && p2->Passive >= 3) {
        p2->Passive -= 3;
        if (p2->Passive < 0) p2->Passive = 0;
        p2->Protection += 50;
        printf("\n%s lost the Clash, consumes 3 Black Silence(%d) and take 50%% less damage\n", p2->name, p2->Passive);

         sleep(1);
      }

      //-------------------------------------------------------------

      // ---------------------Sancho:The Second Kindred of Don Quixote ---------------------
      // Sancho:The Second Kindred of Don Quixote - Block
      if (strcasecmp(p2->name, "Sancho:The Second Kindred of Don Quixote") ==
              0 &&
          p2->skills[12].active == 0 && p2->skills[13].active == 0 &&
          p2->Passive >= 5 && enemyCoins <= 0) {

        p2->Passive -= 5;
        if (p2->Passive < 1) p2->Passive = 1;

        p2->Shield += 25;
        p2->Protection += 25;

        printf("\n%s consumes 5 Hardblood(%d left) to take -25%% damage and "
               "gain 25 Shield HP (%.2f)\n",
               p2->name, p2->Passive, p2->Shield);

         sleep(1);
      }
      // Sancho:The Second Kindred of Don Quixote - Skill 8 and 9 lost
      if (strcasecmp(p2->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
          (s2 == &p2->skills[7] || s2 == &p2->skills[8]) && enemyCoins <= 0 && p2->Passive >= 4) {
        
        p2->Passive -= 4;
        if (p2->Passive < 1) p2->Passive = 1;
        p2->Shield += 20;
        
        printf("\n%s lost the Clash, consumes 4 Hardblood(%d) to gain 20 Shield HP(%.2f)\n", p2->name, p2->Passive, p2->Shield);

         sleep(1);
      }
      // Sancho:The Second Kindred of Don Quixote - Skill 10 lost
      if (strcasecmp(p2->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
          (s2 == &p2->skills[9]) && enemyCoins <= 0) {
        p2->DamageUp -= 50;
        p2->Protection -= 30;
        printf("\n%s lost the Clash, deals -50%% damage and takes 30%% more damage\n", p2->name);

         sleep(1);
      }
      // Sancho:The Second Kindred of Don Quixote - Skill 14 lost
      if (strcasecmp(p2->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
          (s2 == &p2->skills[13]) && enemyCoins <= 0) {
        p2->Passive -= 5;
        if (p2->Passive < 1) p2->Passive = 1;
        printf("\n%s lost the Clash, loses 5 Hardblood(%d)\n", p2->name, p2->Passive);

         sleep(1);
      }
      //-----------------------------------------

      // Sukuna:King of Curse – Skill 4 lost
      if (strcasecmp(p2->name, "Sukuna:King of Curse") == 0 && s2 == &p2->skills[3] &&
          enemyCoins <= 0) {

        updateSanity(p2, -(10));
        if (p2->Sanity < -45)
          p2->Sanity = -45;

        printf("\n%s loses 10 Sanity (%d)\n", p2->name, p2->Sanity);

        sleep(1);
      }
      

      // --------------------- Lei heng ---------------------
      // Lei heng – Skill 4 lost
      if (strcasecmp(p2->name, "Lei heng") == 0 && s2 == &p2->skills[3] &&
          enemyCoins <= 0) {

        updateSanity(p2, -(10));
        if (p2->Sanity < -45)
          p2->Sanity = -45;

        printf("\n%s loses 10 Sanity (%d)\n", p2->name, p2->Sanity);

        sleep(1);
      } // Lei heng – Skill 3 lost
      else if (strcasecmp(p2->name, "Lei heng") == 0 && s2 == &p2->skills[2] &&
          enemyCoins <= 0) {

        p2->FinalPowerBoost -= 4;
        p2->Paralyze += 1;

        printf("\n%s lost the Clash, gains 4 Final Power down and 1 Paralyze (Fix the Power of 1 Coins to 0 for one turn)\n", p2->name);

        sleep(1);
      } // Lei heng – Skill 6 lost
      else if (strcasecmp(p2->name, "Lei heng") == 0 && s2 == &p2->skills[5] &&
          enemyCoins <= 0) {

        p2->FinalPowerBoost -= 2;
        p2->Paralyze += 6;

        printf("\n%s lost the Clash, gains 2 Final Power down and 6 Paralyze (Fix the Power of 6 Coins to 0 for one turn)\n", p2->name);

        sleep(1);
        
      }
      //-----------------------------------------

      // Dawn Office Fixer Sinclair base form S3 won
      if (strcasecmp(p1->name, "Dawn Office Fixer Sinclair") == 0 &&
          (s1 == &p1->skills[3] || s1 == &p1->skills[2]) && enemyCoins <= 0) {

        updateSanity(p1, 10);
        if (p1->Sanity > 45) p1->Sanity = 45;

        printf("\n%s won the Clash, heals 10 Sanity (%d)\n", p1->name, p1->Sanity);

         sleep(1);
      }

      
      // Erlking Heathcliff – lost the Clash
      if (strcasecmp(p2->name, "Erlking Heathcliff") == 0 && enemyCoins <= 0) {

        updateSanity(p2, -(5));
        if (p2->Sanity < -45) p2->Sanity = -45;
        p2->FinalPowerBoostNextTurn -= 1;

        printf("\n%s lost the Clash, loses 5 Sanity(%d) and gains 1 Final Power Down\n", p2->name, p2->Sanity);
        
         sleep(1);
      }



      // Jia Qiu – answer me lost the Clash
      if (strcasecmp(p2->name, "Jia Qiu") == 0 && s2 == &p2->skills[14] && enemyCoins <= 0) {

        p2->skills[15].active += 1;

        if (strcasecmp(p1->name, "Hong lu:The Lord of Hongyuan") == 0) {

          printf("\n%s lost the Clash, inflicts 1 Uncompromising Imposition (%d)\n", p2->name, p2->skills[15].active);
          
        } else {

           printf("\n%s lost the Clash, inflicts 1 Dialogues (%d)\n", p2->name, p2->skills[15].active);

        }


        sleep(1);
      }
      



      
    } else if (enemyTotal > playerTotal && playerCoins > 0) {
      printf("Enemy wins this clash! Player loses 1 coin.\n");
      playerCoins--;


       sleep(1);




      

      // ------------------------- Counter -----------------------------------------
      // Don Quixote:The Manager of La Manchaland - Counterattack
      if (strcasecmp(p1->name, "Don Quixote:The Manager of La Manchaland") ==
              0 &&
          p1->Passive >= 5 && playerCoins <= 0 &&
          (s1 != &p1->skills[6] && s1 != &p1->skills[7])) {

           clearTurnSkillBuffs(p1);

        // Get Don Quixote's Skill 6 and recalculate its effective values
        SkillStats *chosenSkill = getEffectiveSkill(
            p1, p2, &p1->skills[6], &playerTempOffense, &playerTempDefense);

        p1->Passive -= 5;
        if (p1->Passive < 1) p1->Passive = 1;
        printf("\n%s consumes 5 Hardblood(%d left) to continue clashing with %s (Once per Turn)\n",
            p1->name, p1->Passive, chosenSkill->name);

        sleep(1);

        p1->HP -= (int)(p1->MAX_HP * 0.03);
        if (p1->HP < 1) p1->HP = 1;

        int Hardblood = rand() % 3 + 1;

        p1->Passive += Hardblood;
        if (p1->Passive > 30) p1->Passive = 30;

        printf("\n%s consumes 3%% of Max HP(%d) to gain %d Hardblood (%d) (this damage does not lower the unit's HP below 1)\n", p1->name, (int)(p1->MAX_HP * 0.03), Hardblood,
               p1->Passive);

        sleep(1);

        if (strcasecmp(p1->name, "Don Quixote:The Manager of La Manchaland") ==
                0 &&
            p1->Passive >= 10) {

          clearTurnSkillBuffs(p1);

          printf("\n%s has 10+ Hardblood(%d), The Princess of La Manchaland "
             "Rodion: Empower Defense Skill\n",
             p1->name, p1->Passive);

          p1->Passive -= (p1->Passive) / 2;

          sleep(1);

          printf(
          "\n%s consumes half of Hardblood(%d left) on self to use %s\n",
          p1->name, p1->Passive, p1->skills[7].name);

          SkillStats *chosenSkill = getEffectiveSkill(
              p1, p2, &p1->skills[7], &playerTempOffense, &playerTempDefense);

          int savedEnemyCoins = s2->Coins;
          s2->Coins = enemyCoins;

          // Immediately start a new clash using counter Skill
          ClashResult newClash =
              clashPhase(p1, chosenSkill, playerTempOffense, playerTempDefense,
                         p2, s2, enemyTempOffense, enemyTempDefense, p1, playerUnbreakableLost ,enemyUnbreakableLost);

          // Restore the enemy's coin count for later logic
          s2->Coins = savedEnemyCoins;

          sleep(1);

          return newClash;

        } else {
          int savedEnemyCoins = s2->Coins;
          s2->Coins = enemyCoins;

          // Immediately start a new clash using counter Skill
          ClashResult newClash =
              clashPhase(p1, chosenSkill, playerTempOffense, playerTempDefense,
                         p2, s2, enemyTempOffense, enemyTempDefense, p1, playerUnbreakableLost ,enemyUnbreakableLost);

          // Restore the enemy's coin count for later logic
          s2->Coins = savedEnemyCoins;

          sleep(1);

          return newClash;
        }
      }



      // Counter Wild hunt
        if (strcasecmp(p1->name, "Heathcliff:Wild Hunt") == 0 && p1->Passive >= 5 && p1->Sanity >= 15 && p1->skills[0].active > 0 && !p1->skills[2].active && playerCoins <= 0) {

           clearTurnSkillBuffs(p1);

          p1->skills[2].active = 1;

          printf("\n%s mounted 'Dullahan', at 5+ Coffin(%d) and 15+ Sanity(%d), use '%s' to continue the Clash (Once per Turn)\n",
             p1->name, p1->Passive, p1->Sanity, p1->skills[3].name);

          sleep(1);

          SkillStats *chosenSkill = getEffectiveSkill(
              p1, p2, &p1->skills[3], &playerTempOffense, &playerTempDefense);

          int savedEnemyCoins = s2->Coins;
          s2->Coins = enemyCoins;

          // Immediately start a new clash using counter Skill
          ClashResult newClash =
              clashPhase(p1, chosenSkill, playerTempOffense, playerTempDefense,
                         p2, s2, enemyTempOffense, enemyTempDefense, p1, playerUnbreakableLost ,enemyUnbreakableLost);

          // Restore the enemy's coin count for later logic
          s2->Coins = savedEnemyCoins;

          sleep(1);

          return newClash;

        }

      // -------------------------------------------------------------





      

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
      }


      //Unbreakable lost
      if (s1->Unbreakable > 0 && playerUnbreakableLost < s1->Unbreakable && s1->Unbreakable > playerCoins) {
        playerUnbreakableLost++;
      }

      




      //---------------Sancho:The Second Kindred of Don Quixote ---------------------
     
      // Sancho:The Second Kindred of Don Quixote - Skill 7 won
      if (strcasecmp(p2->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
        s2 == &p2->skills[6] && playerCoins <= 0 && p2->HP > 1) {

        printf("\n%s won the Clash, consumes 4%% of Max HP(%d) to gain 1 Hardblood(%d) and deal 25%% more damage (this damage does not lower the unit's HP below 1)\n",
          p2->name, (int)(p2->MAX_HP * 0.04), p2->Passive);

          p2->HP -= (int)(p2->MAX_HP * 0.04);
        if (p2->HP < 1) p2->HP = 1;

        p2->DamageUp += 25;
        p2->Passive += 1;

        sleep(1);
      }
      
      // Sancho:The Second Kindred of Don Quixote - Skill 8 and 9 won
      if (strcasecmp(p2->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
        (s2 == &p2->skills[7] || s2 == &p2->skills[8]) && playerCoins <= 0) {

        printf("\n%s won the Clash, deals 20%% more damage and more 20%% damage for every 10 Hardblood(%d)\n",
          p2->name, p2->Passive);

        p2->DamageUp += 20 * ((p2->Passive / 10) + 1);

        sleep(1);
      }
      
      // Sancho:The Second Kindred of Don Quixote - Skill 10 won
      if (strcasecmp(p2->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
        (s2 == &p2->skills[9]) && playerCoins <= 0) {

        int ShieldGain = (int)(p2->MAX_HP * 0.15);

        p2->Shield += ShieldGain;

        printf("\n%s won the Clash, gains %d Shield HP(%.2f)\n",
          p2->name, ShieldGain, p2->Shield);

        sleep(1);
      }

      // Sancho:The Second Kindred of Don Quixote - Skill 14 won
      if (strcasecmp(p2->name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
          (s2 == &p2->skills[13]) && playerCoins <= 0) {
        p2->DamageUp += 10;
        if (p2->Passive < 1) p2->Passive = 1;
        printf("\n%s won the Clash, deal +10%% damage\n", p2->name);

         sleep(1);
      }


      

      // Sukuna:King of Curse - Passive Clash win
      if (strcasecmp(p2->name, "Sukuna:King of Curse") == 0 && playerCoins <= 0) {

        p2->AttackPowerBoostNextTurn += 5;

        printf("\n%s won the Clash, gains 5 Attack Power Up next turn\n",
          p2->name);

        sleep(1);
      }

      // Sukuna:King of Curse - skill 8 Clash win
      if (strcasecmp(p2->name, "Sukuna:King of Curse") == 0 && playerCoins <= 0 && s2 == &p2->skills[8]) {

        updateSanity(p2, 10);
        if (p2->Sanity > 45) p2->Sanity = 45;

        printf("\n%s won the Clash, heals 10 Sanity (%d)\n",
          p2->name, p2->Sanity);

        sleep(1);
      }

      
      
      //------------------------------------------------------------------

      //-------------------------- Lei heng ------------------------------
      // Lei heng – skill 3
      if (strcasecmp(p2->name, "Lei heng") == 0 && (s2 == &p2->skills[2]) &&
        playerCoins <= 0) {

        printf(
            "\n%s: Aw heck, don't tell me ny'all are tuckered out already!\n",
            p2->name);

        sleep(1);
      }

      // Lei heng – inner strength gain
      if (strcasecmp(p2->name, "Lei heng") == 0 && p2->skills[0].active == 2 &&
          playerCoins <= 0) {

        p2->Passive += clashCount;
        if (p2->Passive >= 25)
          p2->Passive = 25;

        printf("\n%s gains +%d Inner Strength [底力](%d - Max 25)\n", p2->name,
               clashCount, p2->Passive);

         sleep(1);

      } else if (strcasecmp(p2->name, "Lei heng") == 0 &&
        p2->skills[0].active == 3 && playerCoins <= 0) {

        p2->Passive += clashCount * 2;
        if (p2->Passive >= 50)
          p2->Passive = 50;

        printf("\n%s gains +%d Extreme Strength [極力](%d - Max 50)\n",
               p2->name, clashCount * 2, p2->Passive);

         sleep(1);
      }
      //-------------------------------------------------------------

      //-------------------------- Erlking Heathcliff ------------------------------
      
      // Erlking Heathcliff – won the Clash
      if (strcasecmp(p2->name, "Erlking Heathcliff") == 0 && playerCoins <= 0) {

        updateSanity(p2, 5);
        if (p2->Sanity > 45) p2->Sanity = 45;

        printf("\n%s won the Clash, heals 5 Sanity (%d)\n", p2->name, p2->Sanity);

         sleep(1);
      }
      
      // Erlking Heathcliff – skill 5
      if (strcasecmp(p2->name, "Erlking Heathcliff") == 0 && (s2 == &p2->skills[4]) &&
        playerCoins <= 0) {

        p2->DamageUpNextTurn += 10;
        p2->AttackPowerBoostNextTurn += 1;

        printf(
            "\n%s won the Clash, gains 1 Attack Power Up and deal 10%% more damage next turn\n",
            p2->name);

        sleep(1);
      }

      // Erlking Heathcliff – Skill 6
      if (strcasecmp(p2->name, "Erlking Heathcliff") == 0 && s2 == &p2->skills[5] &&
          playerCoins <= 0) {

        updateSanity(p1, -(5));

        printf(
            "\n%s loses 5 Sanity from %s's Skill (%d)\n",
            p1->name, p2->name, p1->Sanity);

        sleep(1);
      }

      //-------------------------------------------------------------



      


      //------------------------- Roland ---------------------------

      // gain Black Silence when won
      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 && playerCoins <= 0) {
        p2->Passive += 5;
        if (p2->Passive > 60) p2->Passive = 60;
        printf("\n%s won the Clash, gains 5 Black Silence(%d - Max 60)\n", p2->name, p2->Passive);

         sleep(1);
      }

      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 && p2->Passive >= 3 && playerCoins <= 0 && s2 == &p2->skills[1]) {
        p2->Passive -= 3;
        p2->DamageUp += 50;
        if (p2->Passive < 0) p2->Passive = 0;
        printf("\n%s won the Clash, consumes 3 Black Silence(%d) to deal 50%% more damage\n", p2->name, p2->Passive);

         sleep(1);
      }

      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 && p2->Passive >= 5 && playerCoins <= 0 && s2 == &p2->skills[3]) {
        p2->Passive -= 5;
        if (p2->Passive < 0) p2->Passive = 0;
        p1->ParalyzeNextTurn += 2;
        printf("\n%s won the Clash, consumes 5 Black Silence(%d) to inflicts 2 Paralyze next turn (Fix the Power of 2 Coins to 0 for one turn)\n", p2->name, p2->Passive);

         sleep(1);
      }

      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 && p2->Passive >= 2 && playerCoins <= 0 && s2 == &p2->skills[5]) {
        p2->Passive -= 2;
        if (p2->Passive < 0) p2->Passive = 0;
        p1->Protection -= 20;
        printf("\n%s won the Clash, consumes 2 Black Silence(%d), %s take 20%% more damage next turn\n", p2->name, p2->Passive, p1->name);

         sleep(1);
      }

      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 && p2->Passive >= 2 && playerCoins <= 0 && (s2 == &p2->skills[0] || s2 == &p2->skills[4])) {
        p2->Passive -= 2;
        if (p2->Passive < 0) p2->Passive = 0;
        p2->DamageUpNextTurn += 15;
        printf("\n%s won the Clash, consumes 2 Black Silence(%d) to deal 15%% more damage next turn\n", p2->name, p2->Passive);

         sleep(1);
      }

      if (strcasecmp(p2->name, "Fixer grade 9?") == 0 && playerCoins <= 0 && (s2 == &p2->skills[8])) {
        p2->Passive += 5;
        if (p2->Passive > 60) p2->Passive = 60;
        p2->ClashPowerNextTurn += 1;
        printf("\n%s won the Clash, gains 5 Black Silence(%d - Max 60) and Clash Power +1 next turn\n", p2->name, p2->Passive);

         sleep(1);
      }

      //------------------------------------------------------------

      // Dawn Office Fixer Sinclair base form S3 lost
      if (strcasecmp(p1->name, "Dawn Office Fixer Sinclair") == 0 && !p1->skills[3].active &&
         (s1 == &p1->skills[3] || s1 == &p1->skills[2]) && playerCoins <= 0) {

        updateSanity(p1, -(10));
        if (p1->Sanity < -45) p1->Sanity = -45;
        
        printf("\n%s lost the Clash, loses 10 Sanity (%d)\n", p1->name, p1->Sanity);

         sleep(1);
      }

      // Dawn Office Fixer Sinclair EGO form S3 lost
      if (strcasecmp(p1->name, "Dawn Office Fixer Sinclair") == 0 && p1->skills[3].active &&
          (s1 == &p1->skills[3] || s1 == &p1->skills[2]) && playerCoins <= 0) {

        updateSanity(p1, -(30));
        if (p1->Sanity < -45) p1->Sanity = -45;

        printf("\n%s lost the Clash, loses 30 Sanity (%d)\n", p1->name, p1->Sanity);

         sleep(1);
      }

      // Counterattack flag for Skill 3
      if (strcasecmp(p1->name, "Meursault:Blade Lineage Mentor") == 0 &&
          s1 == &p1->skills[2] && playerCoins <= 0) {
        playerLostWithSkill3 = 1;
      }

    } else {
      printf("Clash is a draw! Tossing agains...\n");

      sleep(1);
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

  // printf("Clash result: winner %d Player %d coins, Enemy %d coins\n",
  // result.winner, playerCoins, enemyCoins);

  if (playerUnbreakableLost > 0 && playerCoins <= 0) {
    usleep(500000);
    attackPhase(p2, result.enemyskillUsed, result.enemyTempOffense,
                result.enemyTempDefense, p1, result.playerskillUsed,
                result.playerTempOffense, result.playerTempDefense,
                (result.enemyCoins > result.enemyskillUsed->Unbreakable)
                    ? result.enemyCoins
                    : result.enemyskillUsed->Unbreakable,
      result.enemyUnbreakableLost
      , clashCount);
    usleep(500000);
    if (p1->HP > 0) {
      printf("\n%s lost the Clash with Unbreakable Coins (Halve the Damage)\n", p1->name);
      usleep(500000);
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

  if (enemyUnbreakableLost > 0 && enemyCoins <= 0) {
    usleep(500000);
    attackPhase(p1, result.playerskillUsed, result.playerTempOffense,
                result.playerTempDefense, p2, result.enemyskillUsed,
                result.enemyTempOffense, result.enemyTempDefense,
                (result.playerCoins > result.playerskillUsed->Unbreakable)
                    ? result.playerCoins
                    : result.playerskillUsed->Unbreakable,
      result.playerUnbreakableLost
      , clashCount);
    usleep(500000);
    if (p2->HP > 0) {
      printf("\n%s lost the Clash with Unbreakable Coins (Halve the Damage)\n", p2->name);
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

  // Counterattack mechanic
  if (playerLostWithSkill3) {
    usleep(500000);
    attackPhase(p2, result.enemyskillUsed, result.enemyTempOffense,
                result.enemyTempDefense, p1, result.playerskillUsed,
                result.playerTempOffense, result.playerTempDefense,
                (result.enemyCoins > result.enemyskillUsed->Unbreakable)
                    ? result.enemyCoins
                    : result.enemyskillUsed->Unbreakable,
      result.enemyUnbreakableLost
      , clashCount);
    usleep(500000);
    if (p1->HP > 0) {

      clearTurnSkillBuffs(p1);
      
      printf("\nCounterattack triggered\n");
      usleep(500000);
      printf("\nYield my flesh.....\n");
      
      if ((strcasecmp(p1->name, "Meursault:Blade Lineage Mentor") == 0 &&
         p1->HP <= p1->MAX_HP * 0.6)) {


        printf("\n%s HP at 60%% or less HP, Apply 'Remembrance' buff on self, Gains buff at 10+ Sanity or 30+ Sanity further from 0\n",p1->name);

        int PowerBuff;
        int DamageBuff;

        if (abs(p1->Sanity) >= 30) {

          PowerBuff = (3/p1->skills[3].Coins) < 1 ? 1 : (3/p1->skills[3].Coins);
          DamageBuff = (100/p1->skills[3].Coins);
          
          p1->CoinPowerBoost += PowerBuff;
          p1->DamageUp += DamageBuff;

          printf("At 30+ Sanity(%d) further from 0, gain %d Coin Power and gain %d%% more damage\n", p1->Sanity, PowerBuff, DamageBuff);

          sleep(1);
        }
      else if (abs(p1->Sanity) >= 10) {

        PowerBuff = (3/p1->skills[3].Coins) < 1 ? 1 : (3/p1->skills[3].Coins);
        DamageBuff = (50/p1->skills[3].Coins);

        p1->CoinPowerBoost += PowerBuff;
        p1->DamageUp += DamageBuff;

        printf("At 10+ Sanity(%d) further from 0, gain %d Coin Power and gain %d%% more damage\n", p1->Sanity, PowerBuff, DamageBuff);

        sleep(1);
      } 

        printf("\n%s: If you will cut... then wager your life on it.\n", p1->name);

        sleep(1);

      }
      
      usleep(500000);
      
      attackPhase(fullPlayer, &fullPlayer->skills[3],
                  fullPlayer->skills[3].Offense, fullPlayer->skills[3].Defense,
                  p2, result.enemyskillUsed, result.enemyTempOffense,
                  result.enemyTempDefense, fullPlayer->skills[3].Coins, 0 , 0);
      fullPlayer->skills[3].active = 0;

      p2->ParalyzeNextTurn += 5;

      printf("\nTo claim their bones!\n");

      printf("\n%s inflicts 5 Paralyze next turn (Fix the Power of 5 Coins to 0 for one "
             "turn) to %s\n",
             p1->name, p2->name);

      updateSanity(p1, 15);
      if (p1->Sanity > 45) p1->Sanity = 45;

      printf("\n%s heals 15 Sanity (%d)\n",
         p1->name, p1->Sanity);
    }
    result.winner = 99;
  }

  sleep(1);

  return result;
}






// ----------------------------------Setup characters----------------------
void setupCharacters(Character *player, Character *enemy, int pIndex,
                     int eIndex) {
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
    player->skills[0] =
        (SkillStats){"Double Slash - Blast [爆]", 4, 4, 2, 3, 2, 1, 1, 0, 3, 1}; // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    player->skills[1] =
        (SkillStats){"Triple Slash - Blast [爆]", 4, 4, 3, 4, 2, 1, 1, 0, 2, 1};
    player->skills[2] =
        (SkillStats){"Tanglecleaver [快刀亂麻]", 5, 4, 3, 5, 1, 1, 0, 0, 1, 1};
    player->skills[3] = (SkillStats){
        "Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]",3,3,5,6,2,1,0,5,0, 1};
    player->numSkills = 4; // <-- important
  } else if (pIndex == 1) {
    player->name = "Meursault:Blade Lineage Mentor";
    player->HP = 122;
    player->MAX_HP = 122;
    player->skills[0] =
        (SkillStats){"Draw of the Sword", 3, 4, 2, 3, 3, 1, 1, 0, 3, 1};
    player->skills[1] = (SkillStats){"Acupuncture", 3, 5, 3, 3, 3, 1, 1, 0, 2, 1};
    player->skills[2] =
        (SkillStats){"Yield My Flesh", 20, -8, 1, 0, 15, 1, 1, 0, 1, 1};
    player->skills[3] =
        (SkillStats){"To Claim Their Bones", 4, 4, 4, 5, 30, 2, 1, 0, 0, 1};
    player->numSkills = 4; // <-- important
  } else if (pIndex == 2) {
    player->name = "Heathcliff:Wild Hunt";
    player->HP = 113;
    player->MAX_HP = 113;
    player->skills[0] = (SkillStats){"Beheading", 3, 4, 2, 2, -2, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Memorial Procession", 5, 3, 3, 2, -2, 1, 0, 0, 2, 1};
    player->skills[2] = (SkillStats){"Requiem", 6, 6, 2, 5, -2, 1, 0, 0, 1, 1};
    player->skills[3] = (SkillStats){
        "Lament, Mourn, and Despair", 31, -13, 2, 5, -2, 1, 1, 0, 0, 1};
    player->numSkills = 4; // <-- important
  } else if (pIndex == 3) {
    player->name = "Hong lu:The Lord of Hongyuan";
    player->HP = 102;
    player->MAX_HP = 102;
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
    player->numSkills = 5; // <-- important
  } else if (pIndex == 4) {
    player->name = "Yi sang:Fell Bullet";
    player->HP = 106;
    player->MAX_HP = 106;
    player->skills[0] =
        (SkillStats){"See Through Defenses", 5, 6, 1, 1, 2, 1, 1, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Punch Through", 4, 6, 2, 3, 2, 1, 1, 1, 2, 1};
    player->skills[2] =
        (SkillStats){"Target Readjustment Fire", 4, 7, 2, 5, 2, 1, 1, 2, 1, 1};
    player->numSkills = 3; // <-- important
  } else if (pIndex == 5) {
    player->name = "Don Quixote:The Manager of La Manchaland";
    player->HP = 103;
    player->MAX_HP = 103;
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
        (SkillStats){"Variant Sancho Hardblood Arts 8 - Split Apart", 4, 3, 3, 3, 0, 1, 1, 1, 0, 1};
    player->skills[5] =
        (SkillStats){"Ascendant Sancho Hardblood Arts - La Sangre", 5, 5, 4, 5, 0, 1, 1, 4, 0, 1};
    player->skills[6] =
        (SkillStats){"Laughters Will Subside", 5, 4, 2, 3, 5, 1, 1, 0, 0, 1};
    player->skills[7] =
        (SkillStats){"Variant Sancho Hardblood Arts 15 - Buildup to Finale",6,5,2,3,10,1,1,1,0, 1};
    player->numSkills = 8; // <-- important
  } else if (pIndex == 6) {
    player->name = "Lobotomy E.G.O::Solemn Lament Yi Sang";
    player->HP = 106;
    player->MAX_HP = 106;
      player->Passive = 20;
    player->skills[0] =
        (SkillStats){"Celebration for the Departed", 4, 4, 2, 2, 2, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Solemn Lament for the Living", 4, 6, 2, 2, 2, 1, 1, 0, 2, 1};
    player->skills[2] =
        (SkillStats){"Goodbye Now, A Sorrow In You", 4, 3, 4, 5, 2, 1, 1, 0, 1, 1};
    player->numSkills = 3; // <-- important
  } else if (pIndex == 7) {
    player->name = "Dawn Office Fixer Sinclair";
    player->HP = 106;
    player->MAX_HP = 106;
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
    player->numSkills = 4; // <-- important
  } else if (pIndex == 8) {
    player->name = "Gregor:Firefist";
    player->HP = 140;
    player->MAX_HP = 140;
    player->Passive = 100;
    player->skills[0] = (SkillStats){"Flamethrow", 3, 4, 2, 2, 3, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"I'll burn away every last drop of your filthy blood",4, 6, 2, 3, 3, 1, 0, 0, 2, 1};
    player->skills[2] = (SkillStats){"Firefist", 5, 4, 3, 5, 3, 1, 1, 0, 1, 1};
    player->skills[3] = (SkillStats){"I have to keep going for big sis", 9, 7, 1, 2, 3, 1, 0, 0, 0, 1};
    player->numSkills = 4; // <-- important
  } else if (pIndex == 9) {
    player->name = "Heishou Pack - You Branch Adept Heathcliff";
    player->HP = 132;
    player->MAX_HP = 132;
    player->skills[0] = (SkillStats){"Peck 'em", 2, 3, 3, 1, -1, 1, 0, 0, 3, 1};
    player->skills[1] =
        (SkillStats){"Mutilating Talons", 4, 4, 3, 2, -1, 1, 0, 0, 2, 1};
    player->skills[2] = (SkillStats){"Bloodflame Massacre [血炎亂舞]", 4, 3, 4, 3, -1, 1, 0, 0, 1, 1};
    player->skills[3] = (SkillStats){"Rooster's Rampaging Blades Under the Ensanguined Heaven [血天下雞舞亂刀]", 5, 3, 4, 5, -1, 1, 0, 4, 0, 1};
    player->numSkills = 4; // <-- important
  } else { // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    player->name = "Binah";
    player->HP = 100;
    player->MAX_HP = 100;
    player->skills[0] = (SkillStats){"Degraded Fairy", 4, 5, 2, 2, 5, 1, 0, 0, 3, 1};
    player->skills[1] = (SkillStats){"Degraded Chain", 8, 6, 1, 4, 5, 1, 1, 0, 2, 1};
    player->skills[2] = (SkillStats){"Degraded Pillar", 7, 4, 2, 4, 5, 1, 1, 0, 2, 1};
    player->skills[3] = (SkillStats){"Degraded Lock", 9, 11, 1, 6, 5, 1, 0, 0, 1, 1};
    player->skills[4] = (SkillStats){"Degraded Shockwave", 4, 5, 3, 6, 5, 1, 0, 0, 1, 1};
    player->numSkills = 5; // <-- important
  }
  // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active,
  // Unbreakable, Copies, Clashable
  if (eIndex == 0) {
    enemy->name = "Bandit";
    enemy->HP = 150;
    enemy->MAX_HP = 150;
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
    enemy->immuneToPanicSkip = 1;
    enemy->sanityGainBase = 8;
    enemy->sanityLossBase = 7;
    enemy->skills[0] = (SkillStats){"Double Slash", 4, 2, 2, 2, 3, 1, 0, 0, 4, 1};
    enemy->skills[1] = (SkillStats){"Triple Slash", 3, 2, 3, 3, 3, 1, 0, 0, 2, 1};
    enemy->skills[2] =
        (SkillStats){"Tanglecleaver [快刀亂麻]", 8, 12, 1, 6, 3, 1.5, 0, 1, 0, 1};
    enemy->skills[3] = (SkillStats){
        "Blasting Shatterslash [爆碎斬]", 4, 3, 3, 4, 3, 1, 0, 3, 0, 1};
    enemy->skills[4] = (SkillStats){
        "Reloading Tiantui Star's Blade", 4, 4, 2, 1, 3, 1, 0, 0, 0, 1};
    enemy->skills[5] = (SkillStats){
        "Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]", 3, 3, 6, 6, 3, 1, 0, 6, 0, 1};
    enemy->numSkills = 6; // <-- important
  } else if (eIndex == 2) {
    enemy->name = "Erlking Heathcliff";
    enemy->HP = 901;
    enemy->MAX_HP = 901;
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
    enemy->HP = 321;
    enemy->MAX_HP = 321;
    enemy->sanityGainBase = 10;
    enemy->immuneToPanicSkip = 1;
    enemy->skills[0] = (SkillStats){
        "Dismantle", 6, 2, 3, 3, 10, 1, 1, 0, 4, 1}; // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[1] = (SkillStats){"Cleave", 6, 3, 2, 3, 10, 1, 1, 0, 4, 1};
    enemy->skills[2] = (SkillStats){"Blitz speed", 4, 2, 5, 4, 10, 1, 1, 1, 3, 1};
    enemy->skills[3] = (SkillStats){"Fuga:Open", 7, 15, 1, 6, 10, 1, 1, 0, 2, 1};
    enemy->skills[4] =
        (SkillStats){"Chanting", 0, 0, 1, 0, 10, 0, 0, 0, 0, 0};
    enemy->skills[5] = (SkillStats){"Black Flash", 10, 5, 1, 5, 10, 1, 1, 1, 3, 1};
    enemy->skills[6] = (SkillStats){
        "Domain Expansion:Malevolent Shrine", 30, -15, 1, 20, 10, 1.2, 1, 0, 0, 0};
    enemy->skills[7] = (SkillStats){"Know your place...", 2, 1, 5, 5, 10, 1, 1, 1, 2, 0};
    enemy->skills[8] =
      (SkillStats){"World Cutting Slash", 15, 10, 1, 15, 10, 1.25, 0, 1, 0, 1};
    enemy->numSkills = 9; // <-- important
  } else if (eIndex == 4) {
    enemy->name = "Don Quixote";
    enemy->HP = 200;
    enemy->MAX_HP = 200;
    enemy->Passive = 0;
    enemy->skills[0] = (SkillStats){
        "Joust", 4, 4, 1, 2, 2, 1, 1, 0, 3, 1}; // BasePower, CoinPower, Coins, Offense, Defense, DmgMutiplier, active, Unbreakable, Copies, Clashable
    enemy->skills[1] =
        (SkillStats){"Galloping Tilt", 4, 6, 1, 2, 2, 1, 1, 0, 2, 1};
    enemy->skills[2] = (SkillStats){"For Justice!", 3, 3, 3, 2, 2, 1, 1, 0, 1, 1};
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
    enemy->skills[6] =
        (SkillStats){"Do Not Meddle", 3, 3, 2, 37, 0, 1, 1, 0, 3, 1};
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
  } else {
    enemy->name = "Fixer grade 9?";
    enemy->HP = 1897;
    enemy->MAX_HP = 1897;
    enemy->sanityGainBase = -10;
    enemy->sanityLossBase = -7;
    enemy->immuneToPanicSkip = 1;
    enemy->SanityFreezeTurns = -1;
    enemy->skills[0] =
        (SkillStats){"Allas Workshop", 12, -4, 2, 3, -5, 1, 1, 0, 1, 1};
    enemy->skills[1] =
        (SkillStats){"Wheels Industry", 15, -10, 1, 4, -5, 1, 1, 0, 1, 1};
    enemy->skills[2] =
        (SkillStats){"Crystal Atelier", 16, -6, 2, 3, -5, 1, 1, 0, 1, 1};
    enemy->skills[3] =
        (SkillStats){"Zelkova Workshop", 11, -4, 2, 2, -5, 1, 1, 0, 1, 1};
    enemy->skills[4] =
        (SkillStats){"Old Boys Workshop", 10, -5, 1, 2, -5, 1, 1, 0, 1, 1};
    enemy->skills[5] =
        (SkillStats){"Mook Workshop", 10, -7, 1, 2, -5, 1, 1, 0, 1, 1};
    enemy->skills[6] =
        (SkillStats){"Ranga Workshop", 11, -3, 3, 2, -5, 1, 0, 0, 1, 1};
    enemy->skills[7] =
        (SkillStats){"Atelier Logic", 15, -4, 3, 3, -5, 1, 1, 0, 1, 1};
    enemy->skills[8] = (SkillStats){"Durandal", 15, -6, 2, 4, -5, 1, 1, 0, 1, 1};
    enemy->skills[9] =
        (SkillStats){"Furioso", 15, -1, 15, 6, -5, 1, 0, 15, 0, 1};
    enemy->numSkills = 10; // <-- important
  }
  
}

int main() {
  srand(time(NULL));
  const char identity[10][100] = {
      "Meursault:The Thumb",  "Meursault:Blade Lineage Mentor",
      "Heathcliff:Wild Hunt", "Hong lu:The Lord of Hongyuan",
      "Yi sang:Fell Bullet",  "Don Quixote:The Manager of La Manchaland", "Lobotomy E.G.O::Solemn Lament Yi Sang", "Dawn Office Fixer Sinclair", "Gregor:Firefist", "Heishou Pack - You Branch Adept Heathcliff"};
  const char enemyNames[7][100] = {"Bandit",
                                   "Lei heng",
                                   "Erlking Heathcliff",
                                   "Sukuna:King of Curse",
                                   "Don Quixote",
                                   "Jia Qiu",
                                   "Fixer grade 9?"};
  int selected_identity = -1, selected_enemy = -1;

  printf("Limbus Company...\nIdentity:\n");
  for (int i = 0; i < 10; i++)
      printf("%d. %s\n", i + 1, identity[i]);

  Character tempPlayer, tempEnemy; // temporary holders for preview




  
  // -------------------- Identity Selection ---------------------
  while (1) {
      printf("\nSelect identity (1-10): ");
      if (scanf("%d", &selected_identity) == 1 &&
          selected_identity >= 1 && selected_identity <= 11) { // add for binah

          // Setup temp characters for info preview
          setupCharacters(&tempPlayer, &tempEnemy, selected_identity - 1, 0);

          printf("\n--- Identity Info ---\n");
          printf("Name: %s\n", tempPlayer.name);
          printf("HP: %.0f / %.0f\n", tempPlayer.HP, tempPlayer.MAX_HP);
          
        if (selected_identity - 1 == 0) {
          //Taunt
          printf("'Think it over three times, hard, before talking to me. I have ripped out enough tongues today.'\n\n");

          //Description
          printf("A powerful character that focus on unbreakable Skills and dealing damage as much as possible, which comes with powerful skills that great for clashing, but clashing can become weak when it come in long term\n\n");
          
          //Passive
          printf("Passive Skills:\n");
           printf(" 1. Tiantui Star's Blade [天退星刀]\n Always Active: begin Encounters with Tigermark Round amount based on enemy (Min 12)\n");
         printf(" 2. Chachihu [揷翅虎]\n When spent all of 'Tigermark Round', or HP at 65%% or less HP, reload 'Savage Tigermark Round' amount based on enemy (Min 8) and activate 'Unrelenting Spirit [剛氣]'\n");
          printf(" 3. Unrelenting Spirit [剛氣]\n deal +1%% damage for every Sanity different between this unit and enemy (Max 20%%), but at 8+ (sum of Tigermark Round and Savage Tigermark Round spent) activate 'Unrelenting Spirit - Shin [剛氣-心]' instead\n");
          printf(" 4. Unrelenting Spirit - Shin [剛氣-心]\n Defense +3, deal +2%% damage for every Sanity different between this unit and enemy (Max 40%%)\n");
         printf(" 5. Tigermark Round\n Skill Coins that spend Tigermark Round gain +1 Final Power and deal +10%% damage (activates only as long as the Coin has Rounds left to spend)\n" 
            " - At 1+ Tigermark Round and 3+ Tigermark Round spent, convert all Coins of 'Tanglecleaver' into Unbreakable Coins\n");
          printf(" 6. Savage Tigermark Round\n Skill Coins that spend Savage Tigermark Round gain +2 Final Power, deal +30%% damage(activates only as long as the Coin has Rounds left to spend)\n" 
           " - At 1+ Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins and Replace 'Tanglecleaver [快刀亂麻]' with 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]'\n" 
          " - At 0 Savage Tigermark Round, convert all Coins of this unit's Attack Skills to Unbreakable Coins and Gain 'Overheat'\n");
        printf(" 7. Overheat\n Attack Skills Lose (cumulative number of Tigermark Rounds & Savage Tigermark Rounds spent / 4) Clash Power (Max 5); however, gain the following effects(cumulative):\n"
         " - 8+ Rounds spent: Take 10%% less damage for every 10%% missing HP on self at Turn Start (max 50%%)\n"
         " - 14+ Rounds spent: On Clash Lose, Unbreakable Coins of this unit's Attack Skills deal +(75 + missing HP percentage on self)%% damage (Max 150%%)\n"
          " - 20+ Rounds spent: Deal +(HP percentage difference)%% damage against targets with higher remaining HP percentage than this unit (Max 50%%)\n");
          } 
        else if (selected_identity - 1 == 1) {
          //Taunt
          printf("'We are not placing our stone here, then? Mm, then the tides drive us to resign.'\n\n");

          //Description
          printf("A character with powerful counter skill and great damage skills with anti-death passive.\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Swordplay of the Homeland\n In Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. (Once per Encounter)\n");
          printf(" 2. Yield My Flesh\n When Clashing with 'Yield My Flesh' does not effects by any Clash Power boost, When Clash loses with 'Yield My Flesh', Use Counter 'To Claim Their Bones' to attack back (Cannot be used if this unit died first)\n");
          printf(" 3. In Memoriam\n At 60%% or less HP, Apply 'Remembrance' buff on self, Gains buff at 10+ Sanity or 30+ Sanity further from 0 (Buff base on each Skills)\n");
          printf(" 4. Overthrow\n After got attacked, gain +1 Final Power next turn (Once per enemy's skill)\n");
              } 
        else if (selected_identity - 1 == 2) {
          //Taunt
          printf("'What kindled this flame of wrath that burns within me...? ...No, it doesn't matter why it burns- What matters is that I am the ripping and tearing tempest that will bring about their ruin.'\n\n");

          //Description
          printf("A character with great buff and strong damage output with focus on Sanity but also come with less HP and Defense\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Dullahan\n When at 45+ Sanity, gain 1 'Dullahan' next turn (Max 3), Offense +3, Defense -3. Turn End: When this unit's mounts 'Dullahan', gain 1 'Dullahan' (Max 3), lose 5 Sanity, if this unit's Sanity at -25 or less; however lose all 'Dullahan', lose (15 - (Coffin / 2)) Sanity (Min 10). When lost the Clash, At 5+ Coffin and 15+ Sanity, use 'Lament, Mourn, and Despair' to continue the Clash (Once per Turn)\n");
          printf(" 2. Call of the Erlking\n When at 50%% or less HP, or at -45 Sanity, Turn Start: if this unit at -45, does not 'Panic' and if this unit does not have 'Dullahan', gain 'Dullahan' and if this unit's Sanity at -25 or less, heal Sanity the further this unit's Sanity is from 0 (heal 2 additionalal Sanity for every missing Sanity; Max 50) (Once per Encounter)\n");
          printf(" 3. Endless Lamentation\n When mounts 'Dullahan', and using 'Requiem', use 'Lament, Mourn, and Despair' instead\n");
           printf(" 4. Coffin\n Gain by using 'Requiem' and 'Lament, Mourn, and Despair', gain 20%% damage for every 3 Coffin, gain 1 Clash Power for every 5 Coffin (Max 10)\n");
              } 
        else if (selected_identity - 1 == 3) {
          //Taunt
          printf("'The Lord of Hongyuan marches to war.'\n\n");

          //Description
          printf("A low HP character with great buff, strong damage output, anti-death passive and followers that can help you\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Embrace the Tarnished Blood and Exsanguinate Others For the Cause\n In Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn, after that apply 'Lordsguard' to the left Heishou Pack (Once per Encounter)\n");
          printf(" 2. The Heishou Pack\n The Heishou Pack will heed you as their lord, you have 4 Heishou Packs(Mao, Si, Wu and You) as your follower, when using 'Answer Me, Heishou Packs', command one of the remaining Heishou Pack members to attack alongside this unit with 'I Carve the Path of a Lord' Skill; then gain buff from Heishou Pack after that 'Retreat' that Heishou Pack make them unable to use entire Encounter. If there is no Heishou Pack left use 'Lonesome Stand: Sacrifice to Claim The Garden [孑孑單身，捨生取园]' instead\n");
          printf(" 3. The Heishou Lord\n After 'Embrace the Tarnished Blood and Exsanguinate Others For the Cause' activated, when this unit takes damage that can bring their HP down to 0, one of left Heishou Pack use 'Lordsgurad' to defense this unit after that 'Retreat' that Heishou Pack make them unable to use entire Encounter, if the damage can't bring their HP down to 0, do not use 'Retreat'\n");
              } 
        else if (selected_identity - 1 == 4) {
          //Taunt
          printf("'They would point and jeer at the rags splattered with the blood of fellowship. The fools; only I can grasp the highest degree of tragedy upon this earth.'\n\n");

          //Description
          printf("A low HP character with OVERKILL damage output\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Volatilized Memory\n When using Skills expect 'Target Readjustment Fire' gain 'Torn Memory' which use for 'Target Readjustment Fire' to buff it\n");
          printf(" 2. Fell Bullet\n When lost 'Torn Memory' gains 'Fell Bullet'. gains 1.0 Damage Multiper for every Fell Bullet\n");
              } 
        else if (selected_identity - 1 == 5) {
          //Taunt
          printf("'The Family will be well-cared for. ...After all, the onus always fell on me to provide for what you abandoned.'\n\n");

          //Description
          printf("A low HP and defense character with various for each skill and healing skills\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. Variant Sancho Hardblood Arts\n Turn Start: When at 15+ 'Hardblood' using Variant Sancho Hardblood Arts instead for each skills\n");
          printf(" 2. Bearer of the Blood Kin\n When at 30%% or less HP, 'Responsibility' activate, Clash Power +1, Deal +20%% damage, Take +20%% damage and gain 3 Hardblood\n");
          printf(" 3. Armadura de Sangre\n If this unit lost the Clash, consumes 5 'Hardblood' to use 'Laughters Will Subside' to continue clashing (Once per Turn), if won the Clash with 'Laughters Will Subside' gain 5 'Hardblood'. At 10+ use 'Variant Sancho Hardblood Arts 15 - Buildup to Finale' instead\n");
          printf(" 4. Hardblood\n When use certain skills gain randomly 1-3 Hardblood (Max 30) ('Hardblood' cannot drop below 1)\n");
              } 
          else if (selected_identity - 1 == 6) {
            //Taunt
            printf("'Well do I understand your sentiment on death. Why not lay rest to the impulses in your heart for a moment and converse with me more?'\n\n");

            //Description
            printf("A low HP character with a great debuff Skills and strong clashing, but come up with limit attack, sometimes on low Living & The Departed getting weak and need to recharge which uses Sanity\n\n");

            //Passive
            printf("Passive Skills:\n");
            printf(" 1. The Living & The Departed\n Start encounter with 20 'The Living & The Departed' (Max 20), When inflicts 'Butterfly' on enemy, spent this equal to inflicted numbers\n");
            printf(" 2. Butterfly\n When this unit at 0 or higher Sanity, On attack with 'Butterfly', 30%% heals Sanity on self or 70%% loses Sanity on enmey equal to (Butterfly/3; Min 1), but when this unit at less than 0, On attack with 'Butterfly', 70%% heals Sanity on self or 30%% loses Sanity on enmey equal to (Butterfly/3; Min 1), but on enemy with loses Sanity on Clash Win, heals Sanity on enmey instead and on enemy without Sanity, deal more damage equal to (Butterfly/3; Min 1) instead, On attack enemy with 'Butterfly', if enemy's Sanity less than 0 (enemy with loses Sanity on Clash Win, more than 0 Sanity instead), or without Sanity, deal (Butterfly/2 - enemy's Sanity/5) fixed damage (deals (Butterfly/2 + enemy's Sanity/5) fixed damage to enemy with loses Sanity on Clash Win; deals (Butterfly/2) fixed damage to enemy without Sanity; rounded down). Expire when Turn End\n");
            printf(" 3. Reload\n When runs out of 'The Living & The Departed', Turn End uses 'Reload', while attacking, stop attack and use 'Reload' instead, when 'Reload' is used, spend 15 Sanity to gain 20 'The Living & The Departed' and gain Shield equal to (Butterfly on the target x 2)%% of Max HP. (Max 40%%)\n");
                } 
            else if (selected_identity - 1 == 7) {
              //Taunt
              printf("'Sometimes I get hand tremors... I hope that doesn't make me look like a coward.'\n\n");

              //Description
              printf("A low HP character that focus on build Sanity and gain buff from high Sanity\n\n");

              //Passive
              printf("Passive Skills:\n");
              printf(" 1. Unstable Shell of Ego\n Turn Start: At 40+ Sanity, consume 20 Sanity to enter the Volatile E.G.O::Waxen Pinion state. At 30%% or less HP and if this unit's SP isn't at -45 at Turn End, reset SP to 45; then, enter the Volatile E.G.O::Waxen Pinion state. (Once per Encounter) (this 'Turn Start' effect does not activate repeatedly).\n");
              printf(" 2. Determination\n Turn Start: At 0 or less SP, if in the Volatile E.G.O::Waxen Pinion state; exit the Volatile E.G.O::Waxen Pinion state and loses all 'Volatile Passion' to gain +(Volatile Passion x 3 - Max 20) Clash Power this turn and next turn\n");
               printf(" 3. Volatile Passion\n Turn Start: Gain 1 'Volatile Passion' while in the Volatile E.G.O state, gain 1 Final Power, gain +20%% damage for every stack. Turn End: lose 5 Sanity for every stack(Max 40 Sanity)\n");
              printf(" 4. Stigma Workshop Weaponry / Passion\n When this unit at 20+ Sanity, gain Clash Power +(Sanity/20). At 45 Sanity, gain Final Power +3 instead. When in a Volatile E.G.O state, and at 0+ Sanity, gain Coin Power +(Sanity/20). At 45 Sanity, gain Coin Power +3 instead.\n");
                  }
             else if (selected_identity - 1 == 8) {
          //Taunt
          printf("'They're... all from our Office. Firefist Office.'\n\n");

          //Description
          printf("A High HP and defense character with powerful skill 3 along with damage buff and burn for every skills\n\n");

          //Passive
          printf("Passive Skills:\n");
          printf(" 1. I'm the only survivor...\n When enemy's HP or this unit's HP at 75%% or less HP, Deal +0.2%% damage for every District 12 Fuel and Overheated Fuel this unit consumed in this Encounter (Max 40%%)\n");
               printf(" - If main target have 30+ (Burn Stack + Burn Count), deal +0.3%% damage instead (Max 60%%)\n");
          printf(" 2. District 12 Special Workshop Fuel\n When start Encounter gain 100 'District 12 Fuel' use for certain skills, when at 50 or less become 'Overheated Fuel', Buff all skills and Burn inflicting; when 'Overheated Fuel' reach 0 use 'I have to keep going for big' instead of current skill\n");
               printf(" 3. ... All burnt to ashes.\n When attack with skills, inflict 'Burn' based on skills used\n");
               printf(" 4. Burn\n - At 1+ Count, or at 1+ Stack (Turn End: If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), Turn End: Deal fixed damage equal to Stack. Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
              }
               else if (selected_identity - 1 == 9) {
                 //Taunt
                 printf("'Yesss...! Finally! Listen up, gamefowls! Get your talons out! We'll be fightin' the night away tonight, 'till there's no more feed left on the sand circle...!'\n\n");

                 //Description
                 printf("A character sacrifics it's HP for enhance Skills and come with powerful Skill 3 in 2 various\n\n");

                 //Passive
                 printf("Passive Skills:\n");
                 printf(" 1. Flame Rooster's Death Defiance [炎鳥不死戦]\n In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. Then, at the next Turn Start, heal (20 + Burn Stack on self)%% HP, and remove all Burn on self (Max 49%%; once per Encounter)\n");
                 printf(" 2. Burn\n At 1+ Count, or at 1+ Stack (Turn End: If at 1+ Count and 0 Stack, gain 1 Stack, if at 0 Count and 1+ Stack, gain 1 Count), Turn End: Deal fixed damage equal to Stack. Then lose 1 Count. When reach 0 Count, loses all Stack too (Max 99 Stack/Count)\n");
                  printf(" 3. Gamefowl\n Cannot be fall below 1 HP due to Burn damage.\n");
                 printf(" 4. Bloody Storm of Blades\n Combat Start: gain 1 Offense Up and 1 Defense Up for every 20%% missing HP on self (Max 3)\n");
                 printf(" 5. Bloodflame [血炎]\n Gain by using certain Skills (Max 3 Stack), when attack heal 3 Sanity. At 45+ Sanity, gain 1 Offense for next turn instead (Max 3 times per turn), Turn End: lose 1 Stack\n");
                 printf(" 6. Battleblood Instinct\n Deal 0.75%% damage for every Stack (Max 20 Stack). At 20+ Stack,  activate 'Rooster's Rampaging Blades Under the Ensanguined Heaven' instead of 'Bloodflame Massacre [血炎亂舞]'\n Gain 'Battleblood Instinct' when meeting one of the following\n");
                 printf(" -  Gain 3 at Clash Start\n"); 
                 printf(" -  Gain 1 when this unit hits with a Attack Skill or when this unit takes Burn damage (At less than 50%% HP, gain 1 more Battleblood Instinct)\n");
                     }
           else {
        //Taunt
        printf("'You bear a poison, heavy and slow... yet deadly. I know you well, even though you know nothing about me.'\n\n");

        //Description
        printf("??????????????????????????????????????????????????????????????????????????????????\n\n");

        //Passive
        printf("Passive Skills:\n");
        printf(" 1. The Final Reception\n At 50%% or less HP, activate 'Serious', increase HP and Max HP to 1150, gains +100%% damage and 5 Final Power for one turn; then gain new Skills set (Once per Encounter) (Cannot be defeat until this effect activated)\n");
             printf(" 2. Fairy\n Inflict by certain Skills, Take (Fairy Stack) fixed damage addition for every hit, if this unit's in activated 'An Arbiter', Take (0.5 x Fairy Stack)%% Max HP fixed damage addition for every hit instead. Turn End: Take (Fairy Stack) fixed damage; then halve stack (Round down), if this unit's in activated 'An Arbiter', Take (0.5 x Fairy Stack)%% Max HP fixed damage; then halve stack (Round down) instead\n");
             printf(" 3. Incomplete Arbiter\n In this Encounter, deals -20%% damage, Final Power -1. When activation 'Serious', activate 'An Arbiter' instead\n");
        printf(" 4. An Arbiter\n Gains +50%% damage, Final Power +2, Deal +20%% damage and +10 Base Power for every Fairy on enemy, when getting attack by Full Clash Lost Coin, Take -80%% damage and gain (50 + Missing HP/3) Shield HP (Max 100), when getting attack and at 0+ Sanity, consumes 10 Sanity to gain (100 + Missing HP/2) Shield HP\n");
            } 
        
          printf("\nSkills (%d total):\n", tempPlayer.numSkills);

          for (int i = 0; i < tempPlayer.numSkills; i++) {
              SkillStats s = tempPlayer.skills[i];
              printf(" %d. %s\n", i + 1, s.name);
            if (s.Unbreakable > 0) {
                if (!s.Clashable) {
                printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d Unclashable\n",
                       s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
                } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d Clashable\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
              }  else 
              if (!s.Clashable) {
                printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable %d Copies %d Unclashable\n",
                       s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
                } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable %d Copies %d Clashable\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
            }

          int confirm;
          printf("\nConfirm this identity? (1 = Yes, 2 = Back): ");
          if (scanf("%d", &confirm) == 1 && confirm == 1) {
              break;
          } else {
              printf("\nReturning to identity list... \nIdentity:\n");
              for (int i = 0; i < 10; i++)
                  printf("%d. %s\n", i + 1, identity[i]);
              continue;
          }

      } else {
          while (getchar() != '\n'); // clear input
          printf("Invalid selection. Try again.\n");
      }
  }

  printf("You selected %s\n", identity[selected_identity - 1]);





  
  // -------------------- Enemy Selection ---------------------

  printf("\nEnemy Options:\n");
  for (int i = 0; i < 7; i++)
      printf("%d. %s\n", i + 1, enemyNames[i]);

  while (1) {
      printf("\nChoose enemy (1-7): ");
      if (scanf("%d", &selected_enemy) == 1 &&
          selected_enemy >= 1 && selected_enemy <= 7) {

          // Setup temp characters for info preview
          setupCharacters(&tempPlayer, &tempEnemy, 0, selected_enemy - 1);

          printf("\n--- Enemy Info ---\n");
          printf("Name: %s\n", tempEnemy.name);
          printf("HP: %.0f / %.0f\n", tempEnemy.HP, tempEnemy.MAX_HP);

          if (selected_enemy - 1 == 0) {
            //Taunt
            printf("'Give me your money!'\n\n");

            //Description
            printf("just a normal Bandit.\n\n");

            //Passive
             printf("Passive Skills: -\n");
          } 
          else if (selected_enemy - 1 == 1) {
            //Taunt
            printf("'That's right, ya shrimps. Ya gotta first wrack them teensy' brains o' yours, gotta think real hard 'bout whether you even come close to my rank before runnin' ya mouths, ya hear?'\n\n");

            //Description
            printf("A high-HP, high-Defense boss who focuses on building strength through repeated clashes, and attack with powerful attack\n\n");

            //Passive
             printf("Passive Skills:\n");
            printf(" 1. Panic Recovery\n If this unit is Panicked still can act, after reset SP to 0 and heal Sanity by this unit's missing HP (Max 30)\n");
            printf(" 2. Tigermark Round Reload\n Turn End: at 90%% or less HP, or at the end of the 2nd turn, gain a new pattern\n");
            printf(" 3. Lei Heng [雷橫]\n Turn End: at 80%% or less HP, or at the end of the 4th turn, gain a new pattern, gain 25 Inner Strength [底力] and use a powerful attack 'Tanglecleaver', repeat every 3rd turn\n");
            printf(" 4. Tiantui Star [天退星]\n When HP at 60%% or less HP, or if this unit is set to use a powerful attack ('Tanglecleaver') next turn for the second time this Encounter, activate 'Unrelenting Spirit [剛氣]', gain 10%% damage and 1 Final Power for every 20%% HP missing (Max 3 each), deal +1%% damage for every Sanity different between this unit and enemy (Max 20%%), All skills' breakable coin become unbreakable coin\n");
            printf(" 5. Lei Heng, The Pinky's Tiantui Star\nTurn End: at 40%% or less HP, or if this unit is set to use a powerful attack ('Tanglecleaver') next turn for the third time this Encounter, Replace the powerful attack 'Tanglecleaver' with 'Savage Tigerslayer's Perfected Flurry of Blades' and convert 'Inner Strength [底力]' to 'Extreme Strength [極力]', convert 'Unrelenting Spirit [剛氣]' to 'Unrelenting Spirit - Shin [剛氣-心]', gain 10%% damage and 1 Final Power for every 15%% HP missing (Max 5 each), deal +2%% damage for every Sanity different between this unit and enemy (Max 40%%)\n");
             printf(" 6. Chachihu [揷翅虎]\n Combat Start: At (50 - current Sanity)%% chance\n");
            printf(" - Randomly heal Sanity between 2-4. At less than 0 Sanity, double the heal amount, at more than 15 Sanity, does not activate");
            printf("\n - Randomly gains between 1-30%% Damage Up. At less than 0 Sanity, does not activate\n");
            printf(" At -45 Sanity, does not activate above effects\n");
            printf(" 7. Inner Strength [底力]\n When using Skills gain 'Inner Strength [底力]' which use for 'Tanglecleaver' and 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]'. Gain when Win Clash +(Clash Count) and +(Clash Count x 2) on 'Extreme Strength [極力]' and gain when attack +(Attack Coins x 2) and +(Attack Coins x 3) on 'Extreme Strength [極力]'\n");
            printf(" 8. Tiantui Star's Blade - Overheat\n When using Skill 'Tanglecleaver' or 'Savage Tigerslayer's Perfected Flurry of Blades [超絕猛虎殺擊亂斬]' next turn equal to the number of times those skills were used (Max 5) (Clash Power -(Stack), Take (10 x Stackable)%% more damage) next turn\n");
            printf(" 9. Ten Blades of the East\n At the Turn Start of gaining a new pattern: "
              "\n - Gain 1 Severing Slash [切斬] (Target takes +50%% damage) for one turn"
              "\n - Heal 5 Sanity for every 10%% missing HP on self (Max 20)\n");
              } 
        else if (selected_enemy - 1 == 2) {
          //Taunt
          printf("'We are not deserve to even breath...'\n\n");

          //Description
          printf("A high-HP, high-Defense boss who focuses on building sanity through passive, and attack with powerful attack\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf(" 1. Heart of Vengeance\n If this unit is Panicked still can act and if not at -45 Sanity, Combat Start: heal 15 Sanity, after reset Sanity to 0 and gain 2 Attack Power up\n");
           printf(" 2. Antagonism\n If the target's current HP is higher than this unit's (%%), Clash Power +2 and deal +20%% damage\n");
           printf(" 3. Long-awaited Moment\n When Clashing\n - Clash Win: Heal 5 Sanity on self \n - Clash Lose: Lose 5 Sanity on self and Gain 1 Final Power Down\n");
          printf(" 4. May She... Wake in Torment!\n Turn End: if this unit is at 70%% or less HP, or at the end of the 6nd turn, gain new pattern\n");
          printf(" 5. Withstand\n At 50 or less HP, Cap Hp to 50 (Once per Encounter)\n");
          printf(" 6. Every Heathcliff Must Die...\n At 50 or less HP, use 'Every Heathcliff Must Die...'\n");
            } 
        else if (selected_enemy - 1 == 3) {
          //Taunt
          printf("'Know your place... Fool...'\n\n");

          //Description
          printf("A boss who focus on dealing damage, which come up with great clash skills. He's boring\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf(" 1. Domain Expansion\n At 50 HP or less HP, use 'Domain Expansion:Malevolent Shrine' instead\n");
           printf(" 2. The Strongest Of History\n Clash win: gain 5 Attack Power up next turn\n");
           printf(" 3. King\n If this unit is Panicked still can act, after reset Sanity to 0 and gain 2 Clash Power up next turn\n");
          printf(" 4. Chanting\n Turn Start: For every 3 Turn, this unit uses 'Chanting', if this unit is set to use 'Chanting' 3 times, next turn use 'World Cutting Slash'\n");
           printf(" 5. Cursed Reverse Techinque\n In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn. Turn End: Heal HP to Max HP and gain 30 Sanity (Once per Encounter)\n");
            } 
        else if (selected_enemy - 1 == 4) {
          //Taunt
          printf("'Dreaming end... so what?'\n\n");

          //Description
          printf("A low-HP boss who along with great heal along with strong passive buff to Skills\n\n");

          //Passive
           printf("Passive Skills:\n");
           printf(" 1. Sancho:The Second Kindred of Don Quixote\n When HP reachs to 0 tranform into Sancho:The Second Kindred of Don Quixote'\n");
          printf(" 2. Hardblood\n In 'Sancho:The Second Kindred of Don Quixote' Phase, Start Combat: gain 1 'Hardblood' use for certain skills and buff, when using some certain skills gains some as well (Max 30) ('Hardblood' cannot drop below 1)\n");
          printf(" 3. In Dreams\n In 'Sancho:The Second Kindred of Don Quixote' Phase, if this unit is Panicked still can act and after that reset Sanity to 0 and consumes 3 Hardblood to gains 3 Attack Power Up and +30%% damage \n");
          printf(" 4. If we can be freed from this excruciating sickness\n In 'Sancho:The Second Kindred of Don Quixote' Phase, On Hit, heal 40%% of the damage dealt.\n"
            " - This Passive heals +1%% more HP for every missing HP on self (Max 20%%)\n"
            " - For Unbreakable Coins: this effect does not activate on Hit After Clash Lose.\n"
            "Every Turn Start: heal (percentage missing HP/2) HP. (Max 30)\n"
            "Every Turn Start: at -15 or less Sanity, consume (5 - current Sanity/5) Hardblood (Rounded down) to gain (percentage missing HP/2) Sanity and gain 2 Clash Power Up\n"
            "On Hit without Clash Lose, gain 3 Hardblood\n");
          printf(" 5. I'll pirece you!\n In 'Sancho:The Second Kindred of Don Quixote' Phase, at HP 60%% or less HP, Use Variant Don Quixote Style: Sancho Arts 2 - La Sangre instead (Once per Encounter)\n");
          printf(" 6. End Dreams\n In 'Sancho:The Second Kindred of Don Quixote' Phase, at HP 40%% or less HP, Use La Aventura Ha Terminado instead (Once per Encounter), After Attack: when this unit loses Clash consumes 5 Hardblood to take -25%% damage and gain 25 Shield HP (Activates for the left of Encounter)\n");
            } else if (selected_enemy - 1 == 5) {
          //Taunt
          printf("'I shall afford you neither the wherewithal nor the time to mask your ruminations. Bring forth a real answer; do not let it languish behind your tongue.'\n\n");

          //Description
          printf("A strong boss that can with powerful attack, debuff and unable to beat... but he's not giving it his all.\n\n");
          
          //Passive
           printf("Passive Skills:\n");
           printf(" 1. Effloresced E.G.O::Érlì\n First Turn Start: heal 30 SP and gain 'A Sliver of Anticipation', He's not giving it his all\n");
          printf(" 2. A Sliver of Anticipation\n When possess, Lose 35 Offense and 35 Defense, Deal 30%% more damage\n");
          printf(" 3. Infinite Song of Erudition\n After Attack: heal 5 Sanity\n");
          printf(" 4. Panic Recovery\n Turn End: if in Panic, reset SP to 0\n");
           printf(" 5. I shall know your answer.\n At 85%% or less HP, then gain new pattern\n");
          printf(" 6. I still await your answer.\n At 60%% or less HP, apply 3 Dialogues to enemy. Dialogues: Turn End: heal 5 Sanity, When HP drop to 0 heal up to max HP; then lose 1 stack\n");
          printf(" 7. Do not fear the futility.\n At 30%% or less HP, Turn End: Cap HP to 30%%; then use 'Like a Roaring Storm' at Turn Start\n");
          printf(" 8. perhaps they must be shaken afore you are to speak your truth.\n At 10%% or less HP, Turn End: Cap HP to 10%%; then use a powerful attack 'Tiangang Star - Form (格)' at Turn Start\n");
            }  else {
          //Taunt
          printf("'That's that, and this is this.'\n\n");

          //Description
          printf("????????????????????????????????????????\n\n");

          //Passive
           printf("Passive Skills:\n");
          printf(" 1. Agony\n At 50%% or less HP, Gain 10 Offense and lose 5 Defense \n");
          printf(" 2. Black Heart\n If this unit is Panicked still can act, when lose clash heal Sanity instead, when win clash lose Sanity instead\n");
           printf(" 3. Vengeance For Nothing\n Every end of Turn 3rd, at 0+ Sanity, loses (Further from 0 Sanity/2) Sanity (Rounded down) and gain (3 + (1 for every 10 Sanity)) Black Silence, at less than 0 Sanity, loses 5 Sanity and gain +2%% damage for every Black Silence next ture\n");
          printf(" 4. Black Silence\n When win clash with Skills gain 5 Black Silence and gain 1 when lose clash, use for certain Skills (Max 60)\n");
          printf(" 5. Furioso\n After all Skills except 'Furioso' had been used, use 'Furioso' next turn\n");
            } 
            
          printf("\nSkills (%d total):\n", tempEnemy.numSkills);

        for (int i = 0; i < tempEnemy.numSkills; i++) {
            SkillStats s = tempEnemy.skills[i];
            printf(" %d. %s\n", i + 1, s.name);
          if (s.Unbreakable > 0) {
            if (!s.Clashable) {
            printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d Unclashable\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
            } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Unbreakable %d Copies %d Clashable\n",
               s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
          }  else 
          if (!s.Clashable) {
            printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable %d Copies %d Unclashable\n",
                   s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
            } else printf("    BasePower %d CoinPower %d Coins %d Offense %d Defense %d Breakable %d Copies %d Clashable\n",
               s.BasePower, s.CoinPower, s.Coins, s.Offense, s.Defense, s.Unbreakable, s.Copies);
        }

          int confirm;
          printf("\nConfirm this enemy? (1 = Yes, 2 = Back): ");
          if (scanf("%d", &confirm) == 1 && confirm == 1) {
              break;
          } else {
              printf("\nReturning to enemy list...\nEnemy Options:\n");
              for (int i = 0; i < 7; i++)
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

   //Initialize OLD_SANITY to current sanity so Turn 1 doesn't reset to 0
    OLD_SANITYP1 = player.Sanity;
    OLD_SANITYP2 = enemy.Sanity;

  if (strcmp(player.name, "Don Quixote:The Manager of La Manchaland") == 0 &&
      strcmp(enemy.name, "Don Quixote") == 0) {

    printf("\n%s: Our dream... was over.\n", player.name);

    sleep(1);

    printf("\n%s: NO! Our dream won't end. Never! FOREVER!\n", enemy.name);

    sleep(2);
    
  } else if (strcmp(player.name, "Heathcliff:Wild Hunt") == 0 &&
             strcmp(enemy.name, "Erlking Heathcliff") == 0) {

    printf("\n%s: It matters not what kind of Heathcliff you are! I shall "
           "embrace every Catherine mine!!!\n",
           player.name);

    sleep(1);

    printf("\n%s: You are nothing more than a Heathcliff, soon to disappear.\n",
           enemy.name);

    sleep(1);

    enemy.skills[7].active = 1;

    printf("\n%s gains 1 'Faded Promise', In this Encounter, when this unit takes damage that brings their HP down to 0, nullify that damage; then, this unit's HP cannot drop below 1 for the turn (Once per Encounter)\n",
       player.name);

    sleep(2);
  } else if (strcmp(player.name, "Meursault:The Thumb") == 0 &&
             strcmp(enemy.name, "Lei heng") == 0) {

    printf("\n%s: Huh... not bad at all! That's a solid sword pick, lad!\n",
           enemy.name);

    sleep(1);

    printf("\n%s: .... You too.\n", player.name);

    sleep(2);
  } else if (strcmp(player.name, "Hong lu:The Lord of Hongyuan") == 0 &&
             strcmp(enemy.name, "Jia Qiu") == 0) {

    printf("\n%s: I never thought I'd ever see you in Hongyuan again, big brother... Fuhu, so in this world, you managed to survive. You seem to be carving out your own path much like myself.\n",
           player.name);

    sleep(1);

    printf("\n%s: Your presence before me is an adequate 'declaration'. Now... tell me. Is this the 'answer' you stand for?\n",
           enemy.name);

    sleep(2);
  } else if (strcmp(player.name, "Binah") == 0 &&
             strcmp(enemy.name, "Fixer grade 9?") == 0) {

    printf("\n%s: Long time no see... Roland, seems like you doing NOT indeed FINE, huh?\n",
           player.name);

    sleep(1);

    printf("\n%s: Out of my way, Binah. They all need to pay...\n",
           enemy.name);

    sleep(1);

    printf("\n%s: I'm sure this isn't what she want.\n",
           player.name);

    sleep(1);

    printf("\n%s: ...that's that and this is this.\n",
           enemy.name);

    sleep(2);
  }

  printf("\n%s HP %.2f / %.2f\n", player.name, player.HP, player.MAX_HP);
  printf("%s HP %.2f / %.2f\n", enemy.name, enemy.HP, enemy.MAX_HP);

  sleep(1);

  // Track two skills and last unused
  int playerSkill1 = -1, playerSkill2 = -1, playerSkill3 = -1, playerLastUnused = -1;
  int enemySkill1 = -1, enemySkill2 = -1, enemySkill3 = -1, enemyLastUnused = -1;

  // First turn: roll both skills
  getSkills(&player, &playerSkill1, &playerSkill2, &playerSkill3, playerLastUnused,
               player.numSkills);
  getSkills(&enemy, &enemySkill1, &enemySkill2, &enemySkill3, enemyLastUnused,
               enemy.numSkills);
  
  // Battle loop
  while (player.HP > 0 && enemy.HP > 0) {

    printf("\n--- Turn %d ---\n", TurnCount);

    // Check who can act this turn
    int IsplayerPanicked = isPanicked(&player);
    int IsenemyPanicked = isPanicked(&enemy);

    // If both can't act, both recover
    if (IsplayerPanicked && IsenemyPanicked) {
      printf("\nBoth are unable to act! They recover...\n");
      player.Sanity = 0;
      enemy.Sanity = 0;
      continue;
    }

    // Enemy picks one skill (only if can act)
    int enemySkillIndex = (rand() % 2 == 0 ? enemySkill1 : enemySkill2);
    enemyLastUnused =
        (enemySkillIndex == enemySkill1 ? enemySkill2 : enemySkill1);

    // For enemy
    getSkills(&enemy, &enemySkill1, &enemySkill2, &enemySkill3, enemyLastUnused,
                 enemy.numSkills);

    //------------------- Turn Start ----------------------------

      // ---------------- Sancho:The Second Kindred of Don Quixote -----------------------

    // Sancho:The Second Kindred of Don Quixote – heal HP at start
    if (strcasecmp(enemy.name, "Sancho:The Second Kindred of Don Quixote") == 0) {

      int healvalue = (((enemy.MAX_HP - enemy.HP) / enemy.MAX_HP) * 100)/2;
      if (healvalue > 30) healvalue = 30;
      if (healvalue > 0) {

      enemy.HP += healvalue;
        if (enemy.HP > enemy.MAX_HP) enemy.HP = enemy.MAX_HP;

      printf("\n%s heals (percentage missing HP/2) HP (%d - Max 30)\n",
             enemy.name, healvalue);

      sleep(1);
      }
    }

      // Sancho:The Second Kindred of Don Quixote – heal sanity at -15 Sanity or less
      if (strcasecmp(enemy.name, "Sancho:The Second Kindred of Don Quixote") == 0 && enemy.Sanity <= -15 && enemy.HP > 1) {

        int healvalue = ((enemy.MAX_HP - enemy.HP) / enemy.MAX_HP * 100)/2;
        int consumed = 5 - (enemy.Sanity/5);
        if (consumed > enemy.HP) consumed = enemy.HP + 1;

        if (enemy.Passive >= consumed) {
        enemy.Passive -= consumed;
        if (enemy.Passive < 1) enemy.Passive = 1;
        updateSanity(&enemy, healvalue);
        enemy.ClashPower += 2;

        printf("\n%s at -15 or less Sanity, consumes %d Hardblood (%d left) to gain %d Sanity and gain 2 Clash Power Up\n",
               enemy.name, consumed, enemy.Passive, healvalue);

        sleep(1);
        }
      }

    // Sancho:The Second Kindred of Don Quixote with Don Quixote:The Manager of La Manchaland - Power down
    if (strcmp(player.name, "Don Quixote:The Manager of La Manchaland") == 0 &&
        strcmp(enemy.name, "Sancho:The Second Kindred of Don Quixote") == 0 && enemy.skills[5].active == 0) {

      enemy.skills[5].active = 1;

      enemy.Sanity = -45;
      enemy.SanityFreezeTurns = 1;

      printf("\n%s gains 'Call of Mother', Start Phase with -45 Sanity\n", enemy.name);

      sleep(1);

      printf("\n%s: Don't you hear it? A call...\n", player.name);

      sleep(1);
    }

      // -----------------------------------------------------------


    // -------------------------- Heathcliff:Wild Hunt -------------------------
    // Wild hunt – Buff
    if (strcasecmp(player.name, "Heathcliff:Wild Hunt") == 0 && player.Sanity >= 45 && player.skills[0].active <= 0) {

      player.skills[0].active++;
      if (player.skills[0].active > 3) player.skills[0].active = 3;

      printf("\n%s at 45+ Sanity, gains 'Dullahan'\n",
             player.name);

      sleep(1);
    }

    // Heathcliff:Wild Hunt - Call of erlking
      if (strcasecmp(player.name, "Heathcliff:Wild Hunt") == 0 && (player.HP <= player.MAX_HP * 0.5 || player.Sanity <= -45) && player.skills[0].active == 0 && !player.skills[1].active) {

        player.skills[1].active = 1;
        player.skills[0].active += 1;
        if (player.skills[0].active > 3) player.skills[0].active = 3;

        if (player.Sanity <= -45) IsplayerPanicked = 0;

        printf("\n%s at 50%% or less HP, or at -45 Sanity, gain 'Dullahan' (Once per Encounter)\n", player.name);

        if (player.Sanity <= -25) {
        int missingSP = -player.Sanity;       // how far below 0
          int extraHeal = 2 * missingSP;           // 2 SP per missing SP
          if (extraHeal > 50) extraHeal = 50;      // cap at 50

          int totalHeal = 10 + extraHeal;          // base 10 + extra
        updateSanity(&player, totalHeal);

        printf("\n%s heals %d Sanity(%d)\n",
          player.name, totalHeal, player.Sanity);
        }

      }

    // ------------------------------------------------


    // Erlking Heathcliff – always use skill 9 if HP ≤ 50
    if (strcasecmp(enemy.name, "Erlking Heathcliff") == 0 && enemy.HP <= 50 &&
      !enemy.skills[8].active && enemy.Passive == 2) {

      enemy.skills[8].active = 1;

      enemySkillIndex = 8;

    }

    

    // --------------------- Heishou Pack - You Branch Adept Heathcliff ------------------

    // Heishou Pack - You Branch Adept Heathcliff - Skill 3 Unbreakable coins reset
    if (strcasecmp(player.name, "Heishou Pack - You Branch Adept Heathcliff") == 0) {

      player.skills[2].Unbreakable = 0;

      if (player.Passive >= 20 && player.HP <= player.MAX_HP * 0.5) player.skills[2].Copies = 3; // S3 Pity
      else player.skills[2].Copies = 1;

    }

    // Heishou Pack - You Branch Adept Heathcliff - Heal from anti death
    if (strcasecmp(player.name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && player.skills[3].active == 1) {

      player.skills[3].active = 2;

      int healHPpercentag = player.skills[0].active + 20;
      if (healHPpercentag > 49) healHPpercentag = 49;
      int healvalue = (player.MAX_HP * healHPpercentag/100);

      player.HP += (player.HP + healvalue) > player.MAX_HP ? player.MAX_HP - player.HP : healvalue;

      printf("\n%s heals %d%% HP (%.2f), and remove all Burn on self (Max 49%%; Once per Encounter)\n",
        player.name, healHPpercentag, player.HP);

        player.skills[0].active = 0;
        player.skills[1].active = 0;

    }

    // --------------------------------------------------------

    // Meursault:The Thumb - Skill 3 Unbreakable coins reset
    if (strcasecmp(player.name, "Meursault:The Thumb") == 0) {

      player.skills[2].Unbreakable = 0;

    }

    if (strcasecmp(player.name, "Meursault:The Thumb") == 0 && !player.skills[3].active && player.Passive == 0) {
    
      int amount = ((int)(12 * enemy.MAX_HP)) / 847;
      if (amount < 12) amount = 12;

      if (strcasecmp(enemy.name, "Sancho:The Second Kindred of Don Quixote") == 0 || strcasecmp(enemy.name, "Don Quixote") == 0) amount += 4; // pity for boss

      player.Passive = amount;
      
      printf("\n%s gains %d Tigermark Rounds\n", player.name, amount);

      sleep(1);

    }

    // ------------------- Dawn Office Fixer Sinclair -----------------------------

    // Dawn Office Fixer Sinclair - Ego
    if (strcasecmp(player.name, "Dawn Office Fixer Sinclair") == 0 && !player.skills[3].active && player.HP <= player.MAX_HP * 0.3 && player.Sanity > -45 && player.skills[0].active) {

      player.skills[0].active = 0;
       player.Sanity = 45;
        player.skills[2] = player.skills[5];
        player.skills[3] = player.skills[5];
        player.skills[3].active = 1;
        player.skills[2].Copies = -1;
        player.skills[3].Copies = 1;

      printf("\n%s at 30%% or less HP and Sanity isn't -45, reset Sanity to 45; then enters the Volatile E.G.O::Waxen Pinion state (Once per Encounter)\n", player.name);

      sleep(1);

      printf("\n%s: If sorrow has become my indelible stigma, then... I choose to rise with it instead!\n", player.name);

      sleep(1);

    } else if (strcasecmp(player.name, "Dawn Office Fixer Sinclair") == 0 && !player.skills[3].active && player.Sanity >= 40) {

       updateSanity(&player, -(20));
      player.skills[2] = player.skills[5];
      player.skills[3] = player.skills[5];
      player.skills[3].active = 1;
      player.skills[2].Copies = -1;
      player.skills[3].Copies = 1;

      printf("\n%s consumes 20 Sanity(%d) to enter the Volatile E.G.O::Waxen Pinion state\n", player.name, player.Sanity);

      sleep(1);

      printf("\n%s: This time... I'll definitely put an end to this!\n", player.name);

      sleep(1);

    } else if (strcasecmp(player.name, "Dawn Office Fixer Sinclair") == 0 && player.skills[3].active && player.Sanity <= 0) {

      int clashpowerbuff = 3 * player.Passive;
      if (clashpowerbuff > 20) clashpowerbuff = 20;

       player.Passive = 0;
      player.skills[3] = player.skills[4];
      player.skills[2] = player.skills[4];
      player.skills[3].active = 0;
      player.skills[2].Copies = 1;
      player.skills[3].Copies = -1;
      player.ClashPower = clashpowerbuff;
      player.ClashPowerNextTurn = clashpowerbuff;
      

        printf("\n%s exits the Volatile E.G.O::Waxen Pinion state and loses all 'Volatile Passion' to gain +(Volatile Passion x 3) (%d - Max 20) Clash Power for this turn and next turn\n", player.name, clashpowerbuff);

        sleep(1);

      printf("\n%s: ... I have to be bold!\n", player.name);

      sleep(1);


      }

    // Dawn Office Fixer Sinclair - Volatile Passion
    if (strcasecmp(player.name, "Dawn Office Fixer Sinclair") == 0 && player.skills[3].active) {

        player.Passive += 1;

      player.FinalPowerBoost += 1 * player.Passive;
        player.DamageUp += 20 * player.Passive;

      printf("\n%s gains 1 'Volatile Passion', gain 1 Final Power(%d), gain +20%% damage(%d%%) for every stack (%d)\n", player.name, 1 * player.Passive, 20 * player.Passive, player.Passive);

      sleep(1);

    }

    // ------------------------------------------------------------------ 

    // ------------------- Jia Qiu -----------------------------
    // Jia Qiu - Power down
    if (strcasecmp(enemy.name, "Jia Qiu") == 0) {
      
      if (enemy.Passive == 0) {
        
      enemy.Passive = 1;

      for (int i = 0; i < enemy.numSkills; i++) {
          enemy.skills[i].Offense -= 35;
          enemy.skills[i].Defense -= 35;
      }

      enemy.DamageUp += 30;
        updateSanity(&enemy, 30);

      printf("\n%s gains 'A Sliver of Anticipation', He's not giving it his all. Heal 30 Sanity, Lose 35 Offense, 35 Defense and Deal 30%% more damage\n",
             enemy.name);

      sleep(1);

      } else {
        enemy.DamageUp += 30;
      }
    }
    
    // Jia Qiu - Last attack at 85% HP
      if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.HP < enemy.MAX_HP * 0.85 && enemy.Passive == 2 && enemy.skills[5].active == 0) {

        enemySkillIndex = 5;

        enemy.skills[5].active = 1;
      }
    
    // -------------------------------------------------------------

    //--------------------- Lei heng -----------------------------

    // Lei heng – if HP ≤ 60%
    if (strcasecmp(enemy.name, "Lei heng") == 0 && (enemy.HP <= enemy.MAX_HP * 0.6 || (enemy.skills[1].active >= 1 && enemy.skills[2].active >= 3)) &&
      enemy.skills[0].active == 2 && enemy.skills[4].active == 0) {

      enemy.skills[4].active = 1; // Active 'Unrelenting Spirit [剛氣]'

      printf("\n%s activates 'Unrelenting Spirit [剛氣]'!\n", enemy.name);

      sleep(1);

      printf("\n%s: Can't leave a dance unfinished. Ain't that right?\n", enemy.name);

       sleep(1);
    }

    // Lei heng – if HP ≤ 40%
    if (strcasecmp(enemy.name, "Lei heng") == 0 && (enemy.HP <= enemy.MAX_HP * 0.4 || (enemy.skills[1].active >= 2 && enemy.skills[2].active >= 3)) &&
      enemy.skills[0].active == 2) {

      enemy.skills[0].active = 3; // Phase 4

      printf("\n%s converts 'Inner Strength [底力]' to 'Extreme Strength [極力]'\n",
        enemy.name);

      sleep(1);

      printf("\n%s converts 'Unrelenting Spirit [剛氣]' to 'Unrelenting Spirit - Shin [剛氣-心]'\n",
        enemy.name);

      sleep(1);

      printf("\n%s replaces the powerful attack '%s' with '%s'\n", enemy.name,
        enemy.skills[2].name, enemy.skills[5].name);

      sleep(1);

      GainNewPattern(&enemy, &player);

    }
    
    // Lei heng – skill 3 using first fight
    if (strcasecmp(enemy.name, "Lei heng") == 0 && enemy.skills[2].active == 0 && enemy.skills[0].active == 2) {

      enemy.skills[2].active = 1; // Turn Count
      enemySkillIndex = 2; 
      enemy.skills[1].active++; // Overheat count
      if (enemy.skills[1].active > 5) enemy.skills[1].active = 5;

    }

    // Lei heng – skill 3 every 3 turns
    else if (strcasecmp(enemy.name, "Lei heng") == 0 && (enemy.skills[2].active > 0 && enemy.skills[2].active < 3) && enemy.skills[0].active == 2) {

      enemy.skills[2].active++; // Turn Count

    }
    else if (strcasecmp(enemy.name, "Lei heng") == 0 && (enemy.skills[2].active >= 3) && enemy.skills[0].active == 2) {

      enemySkillIndex = 2; 
      enemy.skills[1].active++; // Overheat count
      if (enemy.skills[1].active > 5) enemy.skills[1].active = 5;

    }
    
    // Lei heng – skill 6 using first fight
    if (strcasecmp(enemy.name, "Lei heng") == 0 && enemy.skills[5].active == 0 && enemy.skills[0].active == 3) {

      enemy.skills[5].active = 1;
      enemySkillIndex = 5; 
      enemy.skills[1].active++; // Overheat count
      if (enemy.skills[1].active > 5) enemy.skills[1].active = 5;

    }

    // Lei heng – skill 6 every 3 turns
      else if (strcasecmp(enemy.name, "Lei heng") == 0 && (enemy.skills[5].active > 0 && enemy.skills[5].active < 3) && enemy.skills[0].active == 3) {

        enemy.skills[5].active++; // Turn Count

      }
      else if (strcasecmp(enemy.name, "Lei heng") == 0 && (enemy.skills[5].active >= 3) && enemy.skills[0].active == 3) {

        enemySkillIndex = 5; 
        enemy.skills[1].active++; // Overheat count
        if (enemy.skills[1].active > 5) enemy.skills[1].active = 5;

      }
    
    //-------------------------------------------------------------

    // ------------------------- Sancho --------------------------

    // Sancho – skill 13 using first fight
    if (strcasecmp(enemy.name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      enemy.HP <= enemy.MAX_HP * 0.6 && enemy.skills[12].active) {

      enemy.skills[12].active = 0;
    enemySkillIndex = 12;

    }
    // Sancho – skill 14 using first fight
    if (strcasecmp(enemy.name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
      enemy.HP <= enemy.MAX_HP * 0.3 && enemy.skills[13].active && enemy.skills[12].active == 0) {

      enemy.skills[13].active = 0;
    enemySkillIndex = 13;

    sleep(1);
    }
    
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------

    if (player.HP < 0) player.HP = 0;
    if (enemy.HP < 0) enemy.HP = 0;
    if (player.Shield > 0 && enemy.Shield > 0) {
      printf("\nCurrent HP:\n%s = %.2f / %.2f (Shield %.2f)\n%s = %.2f / %.2f (Shield %.2f)\n",
             player.name, player.HP, player.MAX_HP, player.Shield,
             enemy.name, enemy.HP, enemy.MAX_HP, enemy.Shield);
    } else if (player.Shield > 0) {
      printf("\nCurrent HP:\n%s = %.2f / %.2f (Shield %.2f)\n%s = %.2f  / %.2f\n",
             player.name, player.HP, player.MAX_HP, player.Shield,
             enemy.name, enemy.HP, enemy.MAX_HP);
    } else if (enemy.Shield > 0) {
      printf("\nCurrent HP:\n%s = %.2f / %.2f\n%s = %.2f / %.2f (Shield %.2f)\n",
             player.name, player.HP, player.MAX_HP,
             enemy.name, enemy.HP, enemy.MAX_HP, enemy.Shield);
    } else {
      printf("\nCurrent HP:\n%s = %.2f / %.2f\n%s = %.2f / %.2f\n",
             player.name, player.HP, player.MAX_HP,
             enemy.name, enemy.HP, enemy.MAX_HP);
    }

    // Display Sanity status
    if (player.hasSanity || enemy.hasSanity) {
      printf("[Sanity] ");

      if (player.hasSanity) {
        printf("%s: %d (%s) ", player.name, player.Sanity,
               getSanityStatus(&player));
      }

      if (enemy.hasSanity) {
        printf("| %s: %d (%s)", enemy.name, enemy.Sanity,
               getSanityStatus(&enemy));
      }

      printf("\n");

    }
    
      // ------------------ Before fight -----------------------


      // --------------------------- Sukuna:King of Curse --------------------------------

      
      // Sukuna:King of Curse Chanting
      if (strcasecmp(enemy.name, "Sukuna:King of Curse") == 0 && enemy.skills[4].active < 3 && TurnCount % 3 == 0) {

         enemySkillIndex = 4;

      }

      // Sukuna:King of Curse World Cutting Slash
      else if (strcasecmp(enemy.name, "Sukuna:King of Curse") == 0 && enemy.skills[4].active == 3) {

       enemySkillIndex = 8;

        enemy.skills[4].active = 0;

        printf("\n%s: No more playing around, brat.\n",
          enemy.name);

        sleep(1);

      }

      // ---------------------------------------------------------------------


       // ------------------ Jia Qiu -----------------------

      // Jia Qiu - Last attack at 10% HP
        if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.HP < enemy.MAX_HP * 0.1 && enemy.Passive == 6) {

          printf("\n%s: I expect you to fight to your deaths in this crucial struggle, reversible as they may be.\n",
            enemy.name);

            enemySkillIndex = 15;

          enemy.Passive = 7;

        }

      // Jia Qiu - Taunt S14
      if (strcasecmp(enemy.name, "Jia Qiu") == 0 && &enemy.skills[enemySkillIndex] == &enemy.skills[14]) {

        printf("\n%s: It must lie there still, shrouded it may be.\n",
          enemy.name);

      }

      // Jia Qiu - Last attack at 30% HP
      if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.HP < enemy.MAX_HP * 0.3 && enemy.Passive == 4) {

        printf("\n%s: Do not fear the futility. There will be time for that once you have spoken your mind.\n",
          enemy.name);

          enemySkillIndex = 16;

        enemy.Passive = 5;

      }

      // Jia Qiu - Last attack at 85% HP
        if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.HP < enemy.MAX_HP * 0.85 && enemy.Passive == 2 && enemy.skills[5].active == 0) {

          enemySkillIndex = 5;

          enemy.skills[5].active = 1;
        }

      // -------------------------------------------------------------

      //Roland Furioso
      if (strcasecmp(enemy.name, "Fixer grade 9?") == 0) {

        int allInactive = 1; // assume all inactive

        // check skills 0–8
        for (int i = 0; i <= 8; i++) {
            if (enemy.skills[i].Copies == 1) {
                allInactive = 0; // found one active
                break;
            }
        }

        if (allInactive == 1) {

          for (int i = 0; i <= 8; i++) {
            enemy.skills[i].Copies = 1;
          }
          
          printf("\n%s: I just wanted to stop the cycle. To make it end somewhere.\n",
          enemy.name);

          enemySkillIndex = 9;

          sleep(1);
        } else {
            // Normal random selection
            enemySkillIndex = (rand() % 2 == 0 ? enemySkill1 : enemySkill2);
        }

      } 

      // Lei heng – skill 3 and skill 6 Before fight
        if (strcasecmp(enemy.name, "Lei heng") == 0 &&
            (&enemy.skills[enemySkillIndex] == &enemy.skills[2] ||
             (&enemy.skills[enemySkillIndex] == &enemy.skills[5]))) {

          printf("\n%s: I'maboutta drop somethin' big on y'all! Don't let it kill "
                 "y'all now and spoil the fun!\n",
                 enemy.name);

          sleep(1);
        }

        // Sancho – Before fight
        if (strcasecmp(enemy.name, "Sancho:The Second Kindred of Don Quixote") == 0 &&
            (&enemy.skills[enemySkillIndex] == &enemy.skills[12] ||
             (&enemy.skills[enemySkillIndex] == &enemy.skills[13]))) {

          if (strcasecmp(player.name, "Don Quixote:The Manager of La Manchaland") == 0) {

            printf("\n%s: My name is Sancho!\n",
                   enemy.name);

            printf("\n%s: ...\n",
               player.name);

            sleep(1);

            printf("\n%s: And I, Sancho, declare upon my honor; this lance shall end that festering, slothful dream!\n", enemy.name);

            printf("\n%s: You... Are like... Him... what's a juvenile dream!\n",
               player.name);

          } else {
            printf("\n%s: That's just attachment; my dream has already ended\n", enemy.name);
          }

          sleep(1);
        }

      // Don Quixote:The Manager of La Manchaland – Before fight
      if (strcasecmp(player.name, "Don Quixote:The Manager of La Manchaland") == 0 &&
          (&player.skills[playerSkill1] == &player.skills[2] ||
           (&player.skills[playerSkill2] == &player.skills[2])) && player.Passive >= 15) {

          printf("\n%s: When it comes to making weapons, I had surpassed my father.\n",
            player.name);

        sleep(1);

      } else if (strcasecmp(player.name, "Don Quixote:The Manager of La Manchaland") == 0 && player.Passive >= 15) {

        printf("\n%s: With your weapons... I will lead this bloody battle to victory.\n",
          player.name);

        sleep(1);

      }

      // -------------------------------------------------

      // Sukuna - Domain Expansion:Malevolent Shrine
      if (strcasecmp(enemy.name, "Sukuna:King of Curse") == 0 && enemy.HP <= 50) {

        printf("\n%s at 50 HP or less than, use '%s' instead!\n", enemy.name, enemy.skills[6].name);

        sleep(1);

        printf("\n%s: Brat... You just savor it.\n",
          enemy.name);

        enemySkillIndex = 6;

        sleep(1);

      }

    if (!IsenemyPanicked) {
      if (enemy.skills[enemySkillIndex].Unbreakable > 0 && enemy.skills[enemySkillIndex].Clashable) {
        printf("\nEnemy uses %s (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Unbreakable %d)\n",
               enemy.skills[enemySkillIndex].name,
               enemy.skills[enemySkillIndex].BasePower,
               enemy.skills[enemySkillIndex].CoinPower,
               enemy.skills[enemySkillIndex].Coins,
               enemy.skills[enemySkillIndex].Offense + enemy.OffenseBoost,
               enemy.skills[enemySkillIndex].Defense + enemy.DefenseBoost,
               enemy.skills[enemySkillIndex].Unbreakable);
      } else if (enemy.skills[enemySkillIndex].Unbreakable <= 0 && enemy.skills[enemySkillIndex].Clashable) {
        printf("\nEnemy uses %s (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Breakable)\n",
               enemy.skills[enemySkillIndex].name,
               enemy.skills[enemySkillIndex].BasePower,
               enemy.skills[enemySkillIndex].CoinPower,
               enemy.skills[enemySkillIndex].Coins,
               enemy.skills[enemySkillIndex].Offense + enemy.OffenseBoost,
               enemy.skills[enemySkillIndex].Defense + enemy.DefenseBoost);
      } else if (enemy.skills[enemySkillIndex].Unbreakable > 0 && !enemy.skills[enemySkillIndex].Clashable) {
        printf("\nEnemy uses %s (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Unbreakable %d)\n",
               enemy.skills[enemySkillIndex].name,
               enemy.skills[enemySkillIndex].BasePower,
               enemy.skills[enemySkillIndex].CoinPower,
               enemy.skills[enemySkillIndex].Coins,
               enemy.skills[enemySkillIndex].Offense + enemy.OffenseBoost,
               enemy.skills[enemySkillIndex].Defense + enemy.DefenseBoost,
               enemy.skills[enemySkillIndex].Unbreakable);
      } else if (enemy.skills[enemySkillIndex].Unbreakable <= 0 && !enemy.skills[enemySkillIndex].Clashable) {
        printf("\nEnemy uses %s (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d "
               "Defense %d Breakable)\n",
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
    
    if (!IsplayerPanicked) {
      printf("\nYour skills:\n");
      if (player.skills[playerSkill1].Unbreakable > 0 && player.skills[playerSkill1].Clashable) {
        printf("1. %s (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
               "Unbreakable %d)\n",
               player.skills[playerSkill1].name,
               player.skills[playerSkill1].BasePower,
               player.skills[playerSkill1].CoinPower,
               player.skills[playerSkill1].Coins,
               player.skills[playerSkill1].Offense + player.OffenseBoost,
               player.skills[playerSkill1].Defense + player.DefenseBoost,
               player.skills[playerSkill1].Unbreakable);
      } else if (player.skills[playerSkill1].Unbreakable <= 0 && player.skills[playerSkill1].Clashable) {
        printf("1. %s (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
               "Breakable)\n",
               player.skills[playerSkill1].name,
               player.skills[playerSkill1].BasePower,
               player.skills[playerSkill1].CoinPower,
               player.skills[playerSkill1].Coins,
           player.skills[playerSkill1].Offense + player.OffenseBoost,
           player.skills[playerSkill1].Defense + player.DefenseBoost);
      } else if (player.skills[playerSkill1].Unbreakable > 0 && !player.skills[playerSkill1].Clashable) {
          printf("1. %s (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
                 "Unbreakable %d)\n",
                 player.skills[playerSkill1].name,
                 player.skills[playerSkill1].BasePower,
                 player.skills[playerSkill1].CoinPower,
                 player.skills[playerSkill1].Coins,
                 player.skills[playerSkill1].Offense + player.OffenseBoost,
                 player.skills[playerSkill1].Defense + player.DefenseBoost,
                 player.skills[playerSkill1].Unbreakable);
        } else if (player.skills[playerSkill1].Unbreakable <= 0 && !player.skills[playerSkill1].Clashable) {
          printf("1. %s (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
                 "Breakable)\n",
                 player.skills[playerSkill1].name,
                 player.skills[playerSkill1].BasePower,
                 player.skills[playerSkill1].CoinPower,
                 player.skills[playerSkill1].Coins,
             player.skills[playerSkill1].Offense + player.OffenseBoost,
             player.skills[playerSkill1].Defense + player.DefenseBoost);
        } 
      if (player.skills[playerSkill2].Unbreakable > 0 && player.skills[playerSkill2].Clashable) {
        printf("2. %s (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
               "Unbreakable %d)\n",
               player.skills[playerSkill2].name,
               player.skills[playerSkill2].BasePower,
               player.skills[playerSkill2].CoinPower,
               player.skills[playerSkill2].Coins,
               player.skills[playerSkill2].Offense + player.OffenseBoost,
               player.skills[playerSkill2].Defense + player.DefenseBoost,
               player.skills[playerSkill2].Unbreakable);
      } else if (player.skills[playerSkill2].Unbreakable <= 0 && player.skills[playerSkill2].Clashable) {
        printf("2. %s (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
               "Breakable)\n",
               player.skills[playerSkill2].name,
               player.skills[playerSkill2].BasePower,
               player.skills[playerSkill2].CoinPower,
               player.skills[playerSkill2].Coins,
           player.skills[playerSkill2].Offense + player.OffenseBoost,
           player.skills[playerSkill2].Defense + player.DefenseBoost);
      } else if (player.skills[playerSkill2].Unbreakable > 0 && !player.skills[playerSkill2].Clashable) {
          printf("2. %s (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
                 "Unbreakable %d)\n",
                 player.skills[playerSkill2].name,
                 player.skills[playerSkill2].BasePower,
                 player.skills[playerSkill2].CoinPower,
                 player.skills[playerSkill2].Coins,
                 player.skills[playerSkill2].Offense + player.OffenseBoost,
                 player.skills[playerSkill2].Defense + player.DefenseBoost,
                 player.skills[playerSkill2].Unbreakable);
        } else if (player.skills[playerSkill2].Unbreakable <= 0 && !player.skills[playerSkill2].Clashable) {
          printf("2. %s (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
                 "Breakable)\n",
                 player.skills[playerSkill2].name,
                 player.skills[playerSkill2].BasePower,
                 player.skills[playerSkill2].CoinPower,
                 player.skills[playerSkill2].Coins,
             player.skills[playerSkill2].Offense + player.OffenseBoost,
             player.skills[playerSkill2].Defense + player.DefenseBoost);
        } 

      // Next slot
      if (player.skills[playerSkill3].Unbreakable > 0 && player.skills[playerSkill3].Clashable) {
        printf("Next Skill: %s (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
               "Unbreakable %d)\n",
               player.skills[playerSkill3].name,
               player.skills[playerSkill3].BasePower,
               player.skills[playerSkill3].CoinPower,
               player.skills[playerSkill3].Coins,
               player.skills[playerSkill3].Offense + player.OffenseBoost,
               player.skills[playerSkill3].Defense + player.DefenseBoost,
               player.skills[playerSkill3].Unbreakable);
      } else if (player.skills[playerSkill3].Unbreakable <= 0 && player.skills[playerSkill3].Clashable) {
        printf("Next Skill: %s (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
               "Breakable)\n",
               player.skills[playerSkill3].name,
               player.skills[playerSkill3].BasePower,
               player.skills[playerSkill3].CoinPower,
               player.skills[playerSkill3].Coins,
           player.skills[playerSkill3].Offense + player.OffenseBoost,
           player.skills[playerSkill3].Defense + player.DefenseBoost);
      } else if (player.skills[playerSkill3].Unbreakable > 0 && !player.skills[playerSkill3].Clashable) {
          printf("Next Skill: %s (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
                 "Unbreakable %d)\n",
                 player.skills[playerSkill3].name,
                 player.skills[playerSkill3].BasePower,
                 player.skills[playerSkill3].CoinPower,
                 player.skills[playerSkill3].Coins,
                 player.skills[playerSkill3].Offense + player.OffenseBoost,
                 player.skills[playerSkill3].Defense + player.DefenseBoost,
                 player.skills[playerSkill3].Unbreakable);
        } else if (player.skills[playerSkill3].Unbreakable <= 0 && !player.skills[playerSkill3].Clashable) {
          printf("Next Skill: %s (Unclashable) (BasePower %d CoinPower %d Coins %d Offense %d Defense %d "
                 "Breakable)\n",
                 player.skills[playerSkill3].name,
                 player.skills[playerSkill3].BasePower,
                 player.skills[playerSkill3].CoinPower,
                 player.skills[playerSkill3].Coins,
             player.skills[playerSkill3].Offense + player.OffenseBoost,
             player.skills[playerSkill3].Defense + player.DefenseBoost);
        } 

      int choice;
      while (1) {
        printf("Choose skill (1-2): ");
        if (scanf("%d", &choice) == 1 && (choice == 1 || choice == 2))
          break;
        while (getchar() != '\n')
          ;
      }
      playerSkillIndex = (choice == 1 ? playerSkill1 : playerSkill2);
      playerLastUnused = (choice == 1 ? playerSkill2 : playerSkill1);
    } else {
      // Player can't act, just pick a random skill for defensive purposes
      playerSkillIndex = playerSkill1;
    }

    // Roll new skill to replace used one
    getSkills(&player, &playerSkill1, &playerSkill2, &playerSkill3, playerLastUnused,
                 player.numSkills);

    int playerTempOffense, playerTempDefense;
    int enemyTempOffense, enemyTempDefense;

    playerTempOffense += player.OffenseBoost;
    playerTempDefense += player.DefenseBoost;

    enemyTempOffense += enemy.OffenseBoost;
    enemyTempDefense += enemy.DefenseBoost;

    SkillStats *playerSkillEffective =
        getEffectiveSkill(&player, &enemy, &player.skills[playerSkillIndex],
                          &playerTempOffense, &playerTempDefense);

    SkillStats *enemySkillEffective =
        getEffectiveSkill(&enemy, &player, &enemy.skills[enemySkillIndex],
                          &enemyTempOffense, &enemyTempDefense);

    // Handle different combat scenarios based on who can act
    if (!player.skills[playerSkillIndex].Clashable && enemy.skills[enemySkillIndex].Clashable) {

      attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
        enemyTempDefense, &player, playerSkillEffective,
        playerTempOffense, playerTempDefense,
        enemySkillEffective->Coins, 0, 0);

      attackPhase(&player, playerSkillEffective, playerTempOffense,
        playerTempDefense, &enemy, enemySkillEffective, enemyTempOffense,
      enemyTempDefense,
        playerSkillEffective->Coins, 0, 0);
      
    } else if (player.skills[playerSkillIndex].Clashable && !enemy.skills[enemySkillIndex].Clashable) {

      attackPhase(&player, playerSkillEffective, playerTempOffense,
        playerTempDefense, &enemy, enemySkillEffective, enemyTempOffense,
      enemyTempDefense,
        playerSkillEffective->Coins, 0, 0);

      attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
        enemyTempDefense, &player, playerSkillEffective,
        playerTempOffense, playerTempDefense,
        enemySkillEffective->Coins, 0, 0);

      } else if (!player.skills[playerSkillIndex].Clashable && !enemy.skills[enemySkillIndex].Clashable) {

      int randomattack = rand() % 2 + 1;

      if (randomattack == 1) {

      attackPhase(&player, playerSkillEffective, playerTempOffense,
        playerTempDefense, &enemy, enemySkillEffective, enemyTempOffense,
      enemyTempDefense,
        playerSkillEffective->Coins, 0, 0);

        attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
          enemyTempDefense, &player, playerSkillEffective,
          playerTempOffense, playerTempDefense,
          enemySkillEffective->Coins, 0, 0);

      } else {

        attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
          enemyTempDefense, &player, playerSkillEffective,
          playerTempOffense, playerTempDefense,
          enemySkillEffective->Coins, 0, 0);

        attackPhase(&player, playerSkillEffective, playerTempOffense,
          playerTempDefense, &enemy, enemySkillEffective, enemyTempOffense,
        enemyTempDefense,
          playerSkillEffective->Coins, 0, 0);
        
      }

      }
    else if (IsplayerPanicked && !IsenemyPanicked) {

      attackPhase(&enemy, enemySkillEffective, enemyTempOffense,
                  enemyTempDefense, &player, playerSkillEffective,
                  playerTempOffense, playerTempDefense,
                  enemySkillEffective->Coins, 0, 0);

    } else if (!IsplayerPanicked && IsenemyPanicked) {
      // Enemy is panicked (punching bag), player attacks freely
      
      attackPhase(&player, playerSkillEffective, playerTempOffense,
                  playerTempDefense, &enemy, enemySkillEffective, enemyTempOffense,
        enemyTempDefense,
                  playerSkillEffective->Coins, 0, 0);

    } else {
      // Normal clash - both can act
      ClashResult clash =
          clashPhase(&player, playerSkillEffective, playerTempOffense,
                     playerTempDefense, &enemy, enemySkillEffective,
                     enemyTempOffense, enemyTempDefense, &player, 0, 0);

      //attackPhase(Character *attacker, SkillStats *atk, int atkTempOffense,
       //int atkTempDefense, Character *defender, SkillStats *defSkill,
       //int defTempOffense, int defTempDefense, int remainingCoins,
       //int Unbreakable, int clashCount)
      if (clash.winner == 1)
        attackPhase(
            &player, clash.playerskillUsed, clash.playerTempOffense,
            clash.playerTempDefense, &enemy, clash.enemyskillUsed,
            clash.enemyTempOffense, clash.enemyTempDefense,
            (clash.playerskillUsed->Unbreakable > 0)
                ? ((clash.playerCoins > clash.playerskillUsed->Unbreakable) ? 
          clash.playerCoins : clash.playerskillUsed->Unbreakable) : clash.playerCoins,
          clash.playerUnbreakableLost,
            clash.ClashCount);       
      else if (clash.winner == 2)
        attackPhase(
            &enemy, clash.enemyskillUsed, clash.enemyTempOffense,
            clash.enemyTempDefense, &player, clash.playerskillUsed,
            clash.playerTempOffense, clash.playerTempDefense,
            (clash.enemyskillUsed->Unbreakable > 0)
                ? ((clash.enemyCoins > clash.enemyskillUsed->Unbreakable) ? 
          clash.enemyCoins : clash.enemyskillUsed->Unbreakable) : clash.enemyCoins,
          clash.enemyUnbreakableLost,
          clash.ClashCount);
    }

    


    printf("\n--- Turn End ---\n");

    
    // ----------------------------- Turn End ------------------------------

    // --- Decrement Sanity Lock ---
    if (player.SanityFreezeTurns > 0) {
        player.SanityFreezeTurns--;
        if (player.SanityFreezeTurns == 0) {
        }
    }
    if (enemy.SanityFreezeTurns > 0) {
        enemy.SanityFreezeTurns--;
        if (enemy.SanityFreezeTurns == 0) {
        }
    }
    
    // ---------------------------------

    // for character panic recover

    // --- Player Panic Recovery ---
    if (OLD_SANITYP1 <= -45 && player.Sanity <= -45) {

        // Only recover if not permanently frozen (-1)
        if (player.SanityFreezeTurns != -1) {
            player.Sanity = 0;
            player.SanityFreezeTurns = 0;
          
            printf("\n%s snaps out of panic! (Sanity reset to 0)\n", player.name);
            OLD_SANITYP1 = player.Sanity;
        } else {
            OLD_SANITYP1 = player.Sanity; // Update tracker but don't recover
        }
    } else {
      OLD_SANITYP1 = player.Sanity;
    }

    // --- Enemy Panic Recovery ---
    if (OLD_SANITYP2 <= -45 && enemy.Sanity <= -45) {

      // Check if the enemy is permanently locked in panic (-1)
      if (enemy.SanityFreezeTurns != -1) {

        enemy.Sanity = 0;
        enemy.SanityFreezeTurns = 0;

        printf("\n%s snaps out of panic! (Sanity reset to 0)\n", enemy.name);
        OLD_SANITYP2 = enemy.Sanity;


        // Buff after panic (Only applies if they successfully recovered)
        if (strcasecmp(enemy.name, "Erlking Heathcliff") == 0) {
          enemy.AttackPowerBoostNextTurn += 2;
          printf("\n%s gains 2 Attack Power up next turn\n", enemy.name);
        } 
        else if (strcasecmp(enemy.name, "Lei heng") == 0) {
          int healSanity = (enemy.MAX_HP - enemy.HP) / enemy.MAX_HP * 100;
            if (healSanity > 30) healSanity = 30;

              updateSanity(&enemy, healSanity);
              printf("\n%s heals Sanity by this unit's missing HP (%d - Max 30)\n", enemy.name, enemy.Sanity);
          }
        else if (strcasecmp(enemy.name, "Sancho:The Second Kindred of Don Quixote") == 0 && enemy.Passive >= 1) {

            enemy.Passive -= 3;
          if (enemy.Passive < 1) enemy.Passive = 1;
          enemy.AttackPowerBoostNextTurn += 3;
          enemy.DamageUpNextTurn += 30;

              printf("\n%s consumes 3 Hardblood (%d) to gain 3 Attack Power Up and +30%% damage next turn\n", enemy.name, enemy.Passive);
          }
        else if (strcasecmp(enemy.name, "Sukuna:King of Curse") == 0) {

              enemy.ClashPowerNextTurn += 3;

                printf("\n%s gain 2 Clash Power Up next turn\n", enemy.name);
            }
      } else {
          // SanityFreezeTurns is -1: Do not recover, stay panicked
          OLD_SANITYP2 = enemy.Sanity;
      }

    } else {
        OLD_SANITYP2 = enemy.Sanity;
    }










    

    // Erlking Heathcliff Phase 2
    if (strcasecmp(enemy.name, "Erlking Heathcliff") == 0 && (enemy.HP <= enemy.MAX_HP * 0.7 || TurnCount >= 6) &&
        enemy.Passive == 0) {

      enemy.Passive = 1;

      enemy.sanityLossBase = 10;

      enemy.skills[1] = enemy.skills[5];
      enemy.skills[2] = enemy.skills[4];
      enemy.skills[1].Copies = 0;
      enemy.skills[2].Copies = 0;

      enemy.skills[3].Copies = 4;
      enemy.skills[4].Copies = 3;
      enemy.skills[5].Copies = 3;
      enemy.skills[6].Copies = 2;
      enemy.skills[7].Copies = 2;

      printf("\n%s: May you wake in torment, my dear Catherine.\n", enemy.name);

      sleep(1);
    }

     // Heathcliff:Wild Hunt - Dullahan
      if (strcasecmp(player.name, "Heathcliff:Wild Hunt") == 0 && player.skills[0].active > 0) {

        player.skills[2].active = 0; // Reset Counter
        
        player.skills[0].active += 1; // Dullahan
        if (player.skills[0].active > 3) player.skills[0].active = 3;
        
        if (player.skills[0].active >= 3 || player.Sanity < 0) player.skills[2].Copies = 3; // A bit buff for unlucky player

        printf("\n%s gains 1 Dullahan (%d - Max 3)\n",
          player.name, player.skills[0].active);

        if (player.Sanity <= -25) {

          player.skills[0].active = 0;
          player.skills[2].Copies = 1;

          printf("\n%s at -25 or less, loses all 'Dullahan'\n", player.name);
          
        } else {

        updateSanity(&player, -(5));

        printf("\n%s loses 5 Sanity (%d)\n",
          player.name, player.Sanity);

          if (player.Sanity <= -25) {

            player.skills[0].active = 0;
            player.skills[2].Copies = 1;

            printf("\n%s at -25 or less, loses all 'Dullahan'\n", player.name);

          } else {
          
        int losevalue = (15 - (player.Passive/2));
          if (losevalue < 10) losevalue = 10;

        updateSanity(&player, -(losevalue));

        printf("\n%s loses %d Sanity (%d)\n",
          player.name, losevalue, player.Sanity);

          }

        }
        
      }

    // -------------------------------- Lei heng --------------------------------

    // Lei heng – skill 3 and skill 6 turn end
    if (strcasecmp(enemy.name, "Lei heng") == 0 && enemy.skills[0].active == 3 && enemy.skills[3].active > 0) {

      enemy.skills[5].active = 1;

      printf("\n%s heals Sanity equal to Extreme Strength [底力] consumed(%d)\n", enemy.name, enemy.skills[3].active);
      updateSanity(&enemy, enemy.skills[3].active);
        enemy.skills[3].active = 0;

    } else if (strcasecmp(enemy.name, "Lei heng") == 0 && enemy.skills[0].active == 2 && enemy.skills[3].active > 0) {

      enemy.skills[2].active = 1;

      printf("\n%s heals Sanity equal to Inner Strength [底力] consumed(%d)\n", enemy.name, enemy.skills[3].active);
      updateSanity(&enemy, enemy.skills[3].active);
      enemy.skills[3].active = 0;

    } 

    // Lei heng – HP < 90%
    if (strcasecmp(enemy.name, "Lei heng") == 0 && (enemy.HP < enemy.MAX_HP * 0.9 || TurnCount >= 2) && enemy.skills[0].active == 0) {
      
      GainNewPattern(&enemy, &player);

      enemy.skills[0].active = 1; // Phase 2

      printf("\n%s: Huh... y'all ain't half bad! Keep makin' it worth my bullet fees!\n",
             enemy.name);

      enemy.skills[0].name = "Double Slash - Blast [爆]";
      enemy.skills[0].CoinPower += 1;
      enemy.skills[1].name = "Triple Slash - Blast [爆]";
      enemy.skills[1].CoinPower += 1;

      //enemy.skills[2].Copies = 1;
      enemy.skills[3].Copies = 4;
      enemy.skills[4].Copies = 2;

      sleep(1);
    }

    // Lei heng – HP < 80%
    if (strcasecmp(enemy.name, "Lei heng") == 0 && (enemy.HP < enemy.MAX_HP * 0.8 || TurnCount >= 4) && enemy.skills[0].active == 1) {

      GainNewPattern(&enemy, &player);

       enemy.skills[0].active = 2; // Phase 3

      printf("\n%s gains 25 Inner Strength [底力]\n", enemy.name);

      enemy.Passive += 25;
      if (enemy.Passive > 25) enemy.Passive = 25;

      sleep(1);

      printf("\n%s: That's more like it. Y'all are firin' me up!\n",
         enemy.name);

      sleep(1);

    }

     // -------------------------------------------------------------
      

      // Gregor:Firefist - Burn
        if (strcasecmp(player.name, "Gregor:Firefist") == 0 && (player.skills[0].active > 0 || player.skills[1].active > 0) && enemy.HP > 0) {

          int damage = player.skills[0].active > 0 ? player.skills[0].active : 1;
          
            enemy.HP -= damage; // Stack burn min 1
          if (player.HP < 0) player.HP = 0;
          player.skills[1].active--; // Count Burn min 1
          if (player.skills[1].active <= 0) player.skills[1].active = 0;

          printf("\n%s takes %d damage from Burn (Count %d)\n", enemy.name, damage, player.skills[1].active);

          if (player.skills[1].active <= 0) player.skills[0].active = 0;

          sleep(1);
        }
      

    // ----------------------- Heishou Pack - You Branch Adept Heathcliff ----------------
    
    // Heishou Pack - You Branch Adept Heathcliff - Burn
    if (strcasecmp(player.name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && (player.skills[1].active > 0 || player.skills[0].active > 0) && player.HP > 0) {

       int damage = player.skills[0].active > 0 ? player.skills[0].active : 1;
      
      player.HP -= damage;
      if (player.HP < 1) player.HP = 1;
      player.skills[1].active--;
      if (player.skills[1].active <= 0) player.skills[1].active = 0;

      printf("\n%s takes %d damage from Burn (Count %d)\n", player.name, damage, player.skills[1].active);

      if (player.skills[1].active <= 0) player.skills[0].active = 0;

      sleep(1);

      int gain = 1;
      if (player.HP < player.MAX_HP * 0.5) gain += 1;

      player.Passive += gain;
      if (player.Passive > 20) player.Passive = 20;

       printf("\n%s gains +%d Battleblood Instinct (%d)\n", player.name, gain, player.Passive);
    }

    // Heishou Pack - You Branch Adept Heathcliff - Bloodflame
    if (strcasecmp(player.name, "Heishou Pack - You Branch Adept Heathcliff") == 0 && player.skills[2].active > 0) {

      player.skills[2].active--;

       printf("\n%s loses 1 Bloodflame [血炎] (%d)\n", player.name, player.skills[2].active);
    }

    // -------------------------------------------------------------
    
    // Binah - Fairy
    if (strcasecmp(player.name, "Binah") == 0 && player.skills[0].active > 0) {

      if (!player.Passive) {

      enemy.HP -= player.skills[0].active;
      if (enemy.HP < 0) enemy.HP = 0;

      printf("\n%s take damage equal to Fairy on self (%d)\n", enemy.name, player.skills[0].active);

      sleep(1);

      } else {

        int fairydamage = 0.5*((enemy.MAX_HP/100) * player.skills[0].active);
        
        enemy.HP -= fairydamage;
        if (enemy.HP < 0) enemy.HP = 0;

        printf("\n%s take (0.5 x Fairy on self (%d))%% Max HP damage (%d)\n", enemy.name, player.skills[0].active, fairydamage);

        sleep(1);
        
      }

      player.skills[0].active = player.skills[0].active/2;

       printf("\nhalve the Fairy stack on %s (%d)\n", enemy.name, player.skills[0].active);

      sleep(1);
    }

    // Dawn Office Fixer Sinclair - Volatile Passion
    if (strcasecmp(player.name, "Dawn Office Fixer Sinclair") == 0 && player.skills[3].active) {

        updateSanity(&player, -((5 * player.Passive) > 40 ? 40 : (5 * player.Passive)));

      printf("\n%s lose 5 Sanity(%d - Max 40) for every stack (%d) (Sanity %d)\n", player.name, 5 * player.Passive, player.Passive, player.Sanity);

      sleep(1);

    }

    // ------------------------------ Roland -----------------------

    // Fixer grade 9? – heals every 3 turns on 0+ Sanity
    if (strcasecmp(enemy.name, "Fixer grade 9?") == 0 && enemy.Sanity >= 0 && (TurnCount % 3) == 0) {

      int losevalue = (enemy.Sanity/2);

      updateSanity(&enemy, -losevalue);

       printf("\n%s loses %d Sanity (%d)\n", enemy.name, losevalue, enemy.Sanity);

      sleep(1);

      int gainvalue = 3 + (enemy.Sanity/10);

      enemy.Passive += gainvalue;

      printf("\n%s gains %d Black Silence (%d)\n", enemy.name, gainvalue, enemy.Passive);

      sleep(1);

    } else if (strcasecmp(enemy.name, "Fixer grade 9?") == 0 && enemy.Sanity < 0 && (TurnCount % 3) == 0) {

        updateSanity(&enemy, -5);

      printf("\n%s loses 5 Sanity (%d)\n", enemy.name, enemy.Sanity);

      sleep(1);

        enemy.DamageUpNextTurn += enemy.Passive * 2;

      printf("\n%s gains +2%% damage (%d%%) for every Black Silence (%d) next turn\n", enemy.name, enemy.Passive * 2, enemy.Passive);

      sleep(1);

      }

    // Roland - Ultimate
    if (strcasecmp(enemy.name, "Fixer grade 9?") == 0 && enemy.skills[9].active > 0) {

      enemy.skills[9].active -= 1;
      enemy.ClashPowerNextTurn -= enemy.skills[9].active;

      printf("\n%s loses 1 Sorrow(%d)\n", enemy.name, enemy.skills[9].active);

      sleep(1);

    }

    // Roland - Ultimate
    if (strcasecmp(enemy.name, "Fixer grade 9?") ==
            0 &&
      &enemy.skills[enemySkillIndex] == &enemy.skills[9]) {

      printf("\n%s gains 3 Sorrow, gain 1 Clash Power Down for every 1 Stacks, lose 1 Stacks at Turn End\n", enemy.name);

      enemy.skills[9].active = 3;
      enemy.ClashPowerNextTurn -= enemy.skills[9].active;

      sleep(1);

    }
    // -------------------------------------------------------------

      //------------------------ Lobotomy E.G.O::Solemn Lament Yi Sang ---------------------------

    // Lobotomy E.G.O::Solemn Lament Yi Sang - Reload
    if (strcasecmp(player.name, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 && player.Passive <= 0) {

      updateSanity(&player, -(15));
      
       int ShieldGain = (player.skills[0].active * 2) > 40 ? 40 : (player.skills[0].active * 2);
      int Shield = (ShieldGain/100.0f) * player.MAX_HP;

      player.Shield += Shield; 
      
      printf("\n%s use 'Reload', Spends 15 Sanity(%d) to gain 20 The Living & The Departed and gain Shield equal to (Butterfly on the target x 2)%% of Max HP. (%d%% - Max 40%%) (%d - Shield %.2f)\n", player.name, player.Sanity, ShieldGain, Shield, player.Shield);

        player.Passive = 20;

      sleep(1);
    }

    // Lobotomy E.G.O::Solemn Lament Yi Sang - Butterfly
    if (strcasecmp(player.name, "Lobotomy E.G.O::Solemn Lament Yi Sang") == 0 && player.skills[0].active > 0) {
      player.skills[0].active = 0;
        
      printf("\n%s loses all Butterfly on self\n", enemy.name);
    
      sleep(1);
    }

  //--------------------------------------------------------------

    // Jia Qiu enemy heal sanity
    if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.skills[15].active > 0) {

      updateSanity(&player, 5);

        printf("\n%s heals 5 Sanity (%d)\n", player.name, player.Sanity);
        sleep(1);

    }

    // Jia Qiu Phase 2
    if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.HP <= enemy.MAX_HP * 0.6 &&
        enemy.Passive == 2 && enemy.skills[15].active == 0) {

      printf("\n%s: Reflect upon yourself! Exhume your humanity, your righteousness from deep within!\n", enemy.name);

      if (strcasecmp(player.name, "Hong lu:The Lord of Hongyuan") == 0) {

        printf("\n%s: Phew... okay, fine. Then... As the Lord of Hongyuan, I hereby grant you audience.\n", player.name);

        sleep(1);

         printf("\n%s gains 3 Uncompromising Imposition, Turn End heal 5 Sanity, When HP drop to 0 heal up to max HP and gain 1 Final Power; then lose 1 stack\n", player.name);
      } else {
        sleep(1);

         printf("\n%s gains 3 Dialogues, Turn End heal 5 Sanity, When HP drop to 0 heal up to max HP; then lose 1 stack\n", player.name);
      }

      enemy.skills[15].active = 3;

      enemy.Passive = 3;

      // Disable the old
      for (int i = 0; i < enemy.numSkills; i++) {
        if (enemy.skills[i].Copies > 0) {
          enemy.skills[i] = enemy.skills[3];
          enemy.skills[i].Copies = 0;
        }
      }

      // Set copies for the newly mapped primary skills
       enemy.skills[7].Copies = 3;
      enemy.skills[8].Copies = 3;
      enemy.skills[9].Copies = 3;
       enemy.skills[10].Copies = 2;
       enemy.skills[11].Copies = 2;
       enemy.skills[12].Copies = 2;
       enemy.skills[13].Copies = 2;
       enemy.skills[14].Copies = 2;
       enemy.skills[15].Copies = 0;
       enemy.skills[16].Copies = 0;
    }

      // ------------------------------ Cap HP --------------------------

      // Don Quixote Phase 2
      if (strcasecmp(enemy.name, "Don Quixote") == 0 && enemy.HP <= 0 &&
          enemy.Passive == 0) {

        printf("\n%s: If that's what you really yearn for...\n", enemy.name);

        sleep(1);

        printf("\n%s tranforms to 'Sancho:The Second Kindred of Don Quixote'\n",
               enemy.name);

        enemy.MAX_HP = 583;
        enemy.HP = 583;
        enemy.name = "Sancho:The Second Kindred of Don Quixote";
        enemy.Sanity = 0;
        enemy.sanityGainBase = 6;
        enemy.sanityLossBase = 4;
        enemy.immuneToPanicSkip = 1;
        enemy.Passive = 1;

        // Disable the old S0, S1, S2
        enemy.skills[0] = enemy.skills[3];
         enemy.skills[1] = enemy.skills[3];
         enemy.skills[2] = enemy.skills[3];
        enemy.skills[0].Copies = 0; // in pick skill function copies 0 will auto delete skill from lastused 
        enemy.skills[1].Copies = 0;
        enemy.skills[2].Copies = 0;

        // Set copies for the newly mapped primary skills
        enemy.skills[3].Copies = 4;
        enemy.skills[4].Copies = 4;
        enemy.skills[5].Copies = 4;

        enemy.skills[6].Copies = 3;
        enemy.skills[7].Copies = 3;
        enemy.skills[8].Copies = 3;
        enemy.skills[9].Copies = 3;
        enemy.skills[10].Copies = 3;
        enemy.skills[11].Copies = 3;

        sleep(1);

        if (strcasecmp(player.name,
                       "Don Quixote:The Manager of La Manchaland") == 0) {
          printf(
              "\n%s: I shall show you. Even if it’s a false dream... I will still "
              "move forward without hesitation!\n",
              enemy.name);
        } else {
          printf("\n%s: Let wrap it up.\n", enemy.name);
        }
      }

      // Sukuna:King of Curse cursed reverse technique
      if (strcasecmp(enemy.name, "Sukuna:King of Curse") == 0 && enemy.HP <= 0 && enemy.Passive == 0) {

        enemy.Passive = 1;

        enemy.HP = enemy.MAX_HP;
        updateSanity(&enemy, 30);

        printf("\n%s used 'Cursed Reverse Technique', heal up to Max HP, heal 30 Sanity (Once per Encounter)\n", enemy.name);

        sleep(1);

        printf("\n%s: Arm yourself...\n", enemy.name);

        sleep(1);
      }
      
      // Jia Qiu Anti low
      if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.HP <= enemy.MAX_HP * 0.3 &&
          enemy.Passive == 3) {

        enemy.Passive = 4;

        enemy.HP = (int)(enemy.MAX_HP * 0.3);

        printf("\n%s blocked, cap HP to 30%% for this turn\n", enemy.name);
      }

      // Jia Qiu LAST
      if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.HP <= enemy.MAX_HP * 0.1 &&
          enemy.Passive == 5) {

        enemy.Passive = 6;

        enemy.HP = (int)(enemy.MAX_HP * 0.1);

        printf("\n%s blocked, cap HP to 10%% for this turn\n", enemy.name);
      }

      // Erlking Heathcliff Anti low
      if (strcasecmp(enemy.name, "Erlking Heathcliff") == 0 && enemy.HP <= 50 &&
          enemy.Passive == 1) {

        enemy.Passive = 2;

        enemy.HP = 50;

        printf("\n%s used 'Withstand', cap HP to 50 for this turn\n", enemy.name);

        sleep(1);

        printf("\n%s: Please, Catherine. Appear before me and tear me asunder. Let me see your eyes as I expire.\n", enemy.name);

        sleep(1);
      }



      

    clearTurnEffects(&player);
    clearTurnEffects(&enemy);

    OLD_SANITYP1 = player.Sanity;
    OLD_SANITYP2 = enemy.Sanity;


    
    // Lost CutScene
    if (strcasecmp(enemy.name, "Lei heng") == 0 && enemy.HP <= enemy.MAX_HP*0.2) {

      enemy.HP = 1;
      
      printf("\n%s: I'll be frank, y'all. Real impressed that you even pushed me this far.\n", enemy.name);

      sleep(2);

      attackPhase(&enemy, &enemy.skills[0],
        enemy.skills[0].Offense, enemy.skills[0].Defense,
        &player, &player.skills[0], player.skills[0].Offense,
         player.skills[0].Defense, enemy.skills[0].Coins, 0 , 0);

      sleep(2);

      if (strstr(player.name, "Ryoshu") != NULL) {
        printf("\n%s: But... Yoshihide\n", enemy.name);
      } else if (strcasecmp(player.name, "Meursault:The Thumb") == 0) {
        printf("\n%s: But... Chacuihu\n", enemy.name);
      } else {
       printf("\n%s: But... %s...\n", enemy.name, player.name);
      }

      sleep(4);

      attackPhase(&enemy, &enemy.skills[2],
        enemy.skills[2].Offense, enemy.skills[2].Defense,
        &player, &player.skills[0], player.skills[0].Offense,
         player.skills[0].Defense, enemy.skills[2].Coins, 0 , 0);

      printf("\n%s: ... Ya darn sure oughta've harder if ya really wanted to win!\n", enemy.name);

      sleep(3);

      player.HP = 1;
      enemy.HP = 0;


    } else if (strcasecmp(enemy.name, "Jia Qiu") == 0 && enemy.HP <= 0) {

      printf("\n%s: Fine, You win.\n", enemy.name);

      sleep(1);

    } else {

    // ---------------------------

    }

      TurnCount++;
    }
  
  
  printf("\n--- Battle Result ---\n");
  if (player.HP <= 0 && enemy.HP <= 0) {
    printf("It's a draw!\n");
  } else if (player.HP <= 0) {
    printf("You lost! %s defeated you.\n", enemy.name);
  } else if (enemy.HP <= 0) {
    printf("Victory! You defeated %s.\n", enemy.name);
  }

  return 0;
}
