# Imitation-Limbus-Company-Clash-and-fight-system-in-print-output-only
Trying to Imitate my favorite game for Testing my skill and for fun in C.

---

# 🇺🇸 English Version
## Imitation Limbus Company - Text RPG
A C-based console simulation of the combat mechanics inspired by Limbus Company. This project features turn-based combat, a Sanity system, Skill Clashes, and unique passives for various Identities and Enemies. (Not all the stuff same as Limbus Company, there might be a bit change for my own fun)

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
    * **Clash End:** Sanity is adjusted base on who lost and won (On won heal, On lost lose, both based on Clash Count and Character base heal and lose Sanity)
    * **Victory/Defeat:** The battle ends when either HP drops to 0.

### Mechanics Guide
* **Sanity:**
    * Ranges from -45 to 45.
    * **High Sanity (>0):** Increases chance of Heads (Power up).
    * **Low Sanity (<0):** Decreases chance of Heads.
    * **Panic (-45):** Character usually skips their turn (unless Immune).
* **Skill:**
    * **BasePower:** The starting value of the Skill before any coins are tossed. Used as the foundation for Clash Power and Attack Power.
    * **CoinPower:** The value added to (or subtracted from) the Base Power for each Heads hit during a coin toss.
    * **Coins:** The number of chances to toss. More coins mean more potential hits and higher maximum Clash/Attack power.
    * **Offense:** Compares with the target's Offense Level. Every 3 levels of difference grant +1 Clash Power and increase Final Damage contrast with target's Defense.
    * **Defense:** Reduces incoming damage based on the difference with the attacker’s Offense Level.
    * **Copies:** The rate at which Skill will appear or skill list. More Copies mean more chance.
    * **Unclashable:** Skip the Clash and This attack will land baseed on Speed if the opponent has more Speed, they land their attack first, if not this attack land first.
    * **Unbreakable:** Even if these coins lose a duel, they will still attack after the opponent, but damage is reduced by 50%.
* **Buffs:**
    * **Damage Up:** Increases final damage dealt.
    * **Protection:** Reduces damage taken (percentage).
    * **Power Boost:** Increases Base/Coin/Final Power for clashes/attacks.
    * and else...

---

# 🇹🇭 Thai Version
## Imitation Limbus Company - เกมจำลองการต่อสู้แบบ Text RPG
โปรแกรมภาษา C ที่จำลองระบบการต่อสู้จากเกม Limbus Company โปรเจกต์นี้มีระบบต่อสู้แบบผลัดกันเล่น (Turn-based), ระบบ Sanity, ระบบการดวล (Clash), และสกิลพิเศษ (Passive) (ไม่ใช่ทุกอย่างจะเหมือนกับ Limbus Company อาจมีการเปลี่ยนแปลงเล็กน้อยเพื่อความสนุกส่วนตัวของผม)

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
    * **จบการดวล (Clash End):** ค่าสติจะถูกปรับตามว่าใครแพ้หรือชนะ (เมื่อชนะจะฟื้นฟู เมื่อแพ้จะสูญเสีย ทั้งสองอย่างขึ้นอยู่กับ จำนวนการปะทะ (Clash Count) และการฟื้นฟูหรือเสียค่าสติมากน้อยแค่น้อยที่ขึ้นอยู่กับตัวละคร)
    * **เฟสโจมตี (Attack Phase):** เหรียญที่เหลืออยู่จะสร้างความเสียหายใส่ HP หรือโล่
    * **ชนะ/แพ้:** การต่อสู้จบลงเมื่อ HP ของฝั่งใดฝั่งหนึ่งเหลือ 0

### คำอธิบายกลไก (Mechanics Guide)
* **Sanity (ความมั่นใจ):**
    * มีค่าตั้งแต่ -45 ถึง 45
    * **ค่าบวก (>0):** เพิ่มโอกาสทอยได้หน้าหัว (เพิ่มพลังโจมตี)
    * **ค่าลบ (<0):** ลดโอกาสทอยได้หน้าหัว
    * **Panic (-45):** ตัวละครจะติดสถานะตื่นตระหนกและมักจะข้ามเทิร์นนั้น
* **Skill:**
    * **BasePower (พลังพื้นฐาน):** ค่าพลังเริ่มต้นของสกิลก่อนเริ่มทอยเหรียญ ใช้เป็นฐานหลักสำหรับการคำนวณพลังปะทะ (Clash) และพลังโจมตี
    * **CoinPower (พลังเหรียญ):** ค่าที่จะนำไปบวกเพิ่ม (หรือลบออก) จากพลังพื้นฐาน ตามจำนวนเหรียญที่ออกหน้า "หัว" ในการทอยแต่ละครั้ง
    * **Coins (จำนวนเหรียญ):** จำนวนครั้งที่มีโอกาสทอยเหรียญ ยิ่งเหรียญมากยิ่งมีโอกาสโจมตีต่อเนื่องได้หลายครั้งและเพิ่มเพดานพลังปะทะสูงสุดได้สูงขึ้น
    * **Offense (ระดับการโจมตี):** ระดับการโจมตีที่ใช้เทียบกับเป้าหมาย โดยทุกๆ 3 เลเวลที่ต่างกันจะมอบโบนัสพลังปะทะ +1 และเพิ่มความเสียหายสุดท้ายเมื่อเทียบกับระดับการป้องกันของศัตรู
    * **Defense (ระดับป้องกัน):** ระดับการป้องกันที่ช่วยลดความเสียหายที่ได้รับ โดยคำนวณจากส่วนต่างระหว่างระดับการป้องกันของเรากับระดับการโจมตีของผู้ที่บุกเข้ามา
    * **Copies (จำนวนก๊อปปี้):** อัตราส่วนหรือจำนวนที่สกิลนั้นๆ จะปรากฏในรายการสกิล ยิ่งมีจำนวนสำเนามาก ยิ่งมีโอกาสสุ่มได้สกิลนั้นมาใช้งานสูงขึ้น
    * **Unclashable (ปะทะไม่ได้):** ข้ามการดวลและการโจมตีนี้ตัดสินผลตาม Speed หากคู่ต่อสู้มี Speed มากกว่า จะได้โจมตีก่อน แต่ถ้าไม่เป็นเช่นนั้น การโจมตีนี้จะเข้าเป้าก่อนเสมอ
    * **Unbreakable (เหรียญที่แตกไม่ได้):** แม้จะแพ้การดวล เหรียญจะยังคงอยู่และโจมตีสวนกลับหลังจากถูกโจมตี (ดาเมจลดลง 50%)
* **Buffs (สถานะเสริม):**
    * **Damage Up:** เพิ่มความเสียหายสุดท้าย
    * **Protection:** ลดความเสียหายที่ได้รับ (คิดเป็น %)
    * **Power Boost:** เพิ่มค่าพลังพื้นฐานและพลังเหรียญ
    * และอื่นๆ

---

### Credits & Disclaimer
This is a fan-made project written in C for educational purposes. Limbus Company and all related characters, names, and mechanics are property of Project Moon.
