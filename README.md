# Imitation-Limbus-Company-Clash-and-fight-system-in-print-output-only
Trying to Imitate my favorite game for Testing my skill and for fun in C.

---

# 🇺🇸 English Version
## Imitation Limbus Company - Text RPG
A C-based console simulation of the combat mechanics inspired by Limbus Company. This project features turn-based combat, a Sanity system, Skill Clashes, and unique passives for various Identities and Enemies.

### Features
* **Core Mechanics:**
    * **Clash System:** Compare skill power to destroy enemy coins.
    * **Sanity System:** Affects coin flip odds (Heads/Tails) and Morale states (Normal, Low Morale, Panic).
    * **Complex Passives:** Implementation of unique mechanics for different units.

### How to Install and Run
* **Requirements:** A C compiler (like `gcc`) and a terminal/command prompt.
* **Compile:**
    ```bash
    gcc main.c -o limbus_game -lm
    ```
* **Run:**
    ```bash
    ./limbus_game
    ```
    *(On Windows, run the generated .exe file).*

### How to Play
1.  **Select Identity:** Choose a character (Type the numbers) from the list. The game will display their Passive skills and Stats.
2.  **Select Enemy:** Choose an opponent (Type the numbers). Review their HP and skills.
3.  **Battle Loop:**
    * **Turn Start:** Check HP, Sanity, and Shields. Read any passive triggers.
    * **Choose Skill:** Input 1 or 2 to select a skill for the turn.
    * **Clash Phase:** Watch the dice roll. The side with higher Power wins the coin toss and destroys one of the opponent's coins.
    * **Attack Phase:** Any remaining coins after the clash will deal damage to the opponent's HP/Shield.
    * **Turn End:** Buffs are updated, Sanity is adjusted, and Burn damage is applied.
    * **Victory/Defeat:** The battle ends when either HP drops to 0.

### Mechanics Guide
* **Sanity:**
    * Ranges from -45 to 45.
    * **High Sanity (>0):** Increases chance of Heads (Power up).
    * **Low Sanity (<0):** Decreases chance of Heads.
    * **Panic (-45):** Character usually skips their turn (unless Immune).
* **Special Skills:**
    * **Unclashable:** This attack will land after the opponent lands their attack first.
    * **Unbreakable:** Even if these coins lose a duel, they will still attack after the opponent, but damage is reduced by 50%.
* **Buffs:**
    * **Damage Up:** Increases final damage dealt.
    * **Protection:** Reduces damage taken (percentage).
    * **Power Boost:** Increases Base/Coin/Final Power for clashes/attacks.

---

# 🇹🇭 Thai Version
## Imitation Limbus Company - เกมจำลองการต่อสู้แบบ Text RPG
โปรแกรมภาษา C ที่จำลองระบบการต่อสู้จากเกม Limbus Company โปรเจกต์นี้มีระบบต่อสู้แบบผลัดกันเล่น (Turn-based), ระบบ Sanity, ระบบการดวล (Clash), และสกิลพิเศษ (Passive)

### ฟีเจอร์หลัก (Features)
* **กลไกหลัก:**
    * **ระบบการดวล (Clash System):** เปรียบเทียบพลังสกิลเพื่อทำลายเหรียญของศัตรู
    * **ระบบ Sanity:** ส่งผลต่อโอกาสได้หัว/ก้อยในการทอยเหรียญ และสถานะจิตใจ
    * **สกิลพิเศษ:** รวมเอากลไกเฉพาะตัวของแต่ละตัวละครมาใช้งาน

### วิธีการติดตั้งและรัน (How to Install and Run)
* **ความต้องการ:** ต้องมี Compiler ภาษา C (เช่น `gcc`) และ Terminal
* **คอมไพล์ (Compile):**
    ```bash
    gcc main.c -o limbus_game
    ```
* **รัน (Run):**
    ```bash
    ./limbus_game
    ```

### วิธีการเล่น (How to Play)
1.  **เลือกตัวละคร (Identity):** พิมพ์ตัวเลขเพื่อเลือกตัวละคร ระบบจะแสดงสเตตัสและสกิลติดตัว
2.  **เลือกศัตรู (Enemy):** พิมพ์ตัวเลขเพื่อเลือกคู่ต่อสู้
3.  **วงจรการต่อสู้ (Battle Loop):**
    * **เริ่มตา (Turn Start):** ตรวจสอบ HP, Sanity, โล่ และเงื่อนไขสกิลพิเศษ
    * **เลือกสกิล (Choose Skill):** พิมพ์ 1 หรือ 2 เพื่อเลือกท่าโจมตีในเทิร์นนั้น
    * **เฟสการดวล (Clash Phase):** ฝั่งที่มีพลังสูงกว่าจะทำลายเหรียญของอีกฝั่ง 1 เหรียญ
    * **เฟสโจมตี (Attack Phase):** เหรียญที่เหลืออยู่จะสร้างความเสียหายใส่ HP หรือโล่
    * **จบตา (Turn End):** อัปเดต Buffs, ปรับ Sanity และคำนวณดาเมจต่อเนื่อง (เช่น Burn)
    * **ชนะ/แพ้:** การต่อสู้จบลงเมื่อ HP ของฝั่งใดฝั่งหนึ่งเหลือ 0

### คำอธิบายกลไก (Mechanics Guide)
* **Sanity (ความมั่นใจ):**
    * มีค่าตั้งแต่ -45 ถึง 45
    * **ค่าบวก (>0):** เพิ่มโอกาสทอยได้หน้าหัว (เพิ่มพลังโจมตี)
    * **ค่าลบ (<0):** ลดโอกาสทอยได้หน้าหัว
    * **Panic (-45):** ตัวละครจะติดสถานะตื่นตระหนกและมักจะข้ามเทิร์นนั้น
* **สกิลพิเศษ:**
    * **Unclashable (ปะทะไม่ได้):** ข้ามการดวลและโจมตีเข้าเป้าแน่นอน หลังจากที่ศัตรูโจมตีใส่เราเสร็จแล้ว
    * **Unbreakable (เหรียญป้องกัน):** แม้จะแพ้การดวล เหรียญจะยังคงอยู่และโจมตีสวนกลับหลังจากถูกโจมตี (ดาเมจลดลง 50%)
* **Buffs (สถานะเสริม):**
    * **Damage Up:** เพิ่มความเสียหายสุดท้าย
    * **Protection:** ลดความเสียหายที่ได้รับ (คิดเป็น %)
    * **Power Boost:** เพิ่มค่าพลังพื้นฐานและพลังเหรียญ

---

### Credits & Disclaimer
This is a fan-made project written in C for educational purposes. Limbus Company and all related characters, names, and mechanics are property of Project Moon.
