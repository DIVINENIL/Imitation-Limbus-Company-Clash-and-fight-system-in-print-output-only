# Imitation-Limbus-Company-Clash-and-fight-system-in-print-output-only
Trying to Imitating my favorite game for Testing my skill and for fun in C



# English Version
# Imitation Limbus Company - Text RPG
A C-based console simulation of the combat mechanics inspired by Limbus Company. This project features turn-based combat, a Sanity system, Skill Clashes, and unique passives for various Identities and Enemies.

# Features

# Core Mechanics:
Clash System: Compare skill power to destroy enemy coins.
Sanity System: Affects coin flip odds (Heads/Tails) and Morale states (Normal, Low Morale, Panic).
Complex Passives: Implementation of unique mechanics

# How to Install and Run
Requirements: A C compiler (like gcc) and a terminal/command prompt.
Compile:
gcc main.c -o limbus_game -lm
(Note: -lm links the math library if needed, though your code primarily uses standard libs).
Run:
./limbus_game
(On Windows, run the generated .exe file).

# How to Play
Select Identity: Choose a character (Type the numbers) from the list. The game will display their Passive skills and Stats.
Select Enemy: Choose an opponent (Type the numbers). Review their HP and skills.

# Battle Loop:
Turn Start: Check HP, Sanity, and Shields. Read any passive triggers.
Choose Skill: Input 1 or 2 to select a skill for the turn.
Clash Phase: Watch the dice roll. The side with higher Power wins the coin toss and destroys one of the opponent's coins.
Attack Phase: Any remaining coins after the clash will deal damage to the opponent's HP/Shield.
Turn End: Buffs are updated, Sanity is adjusted, and Burn damage is applied.
Victory/Defeat: The battle ends when either HP drops to 0.

# Mechanics Guide
Sanity:
Ranges from -45 to 45.
High Sanity (>0): Increases chance of Heads (Power up) on coin tosses.
Low Sanity (<0): Decreases chance of Heads.
Panic (-45): Character usually skips their turn (unless Immune).
Clash:
Two skills clash coin-by-coin.
Winner keeps their coin; Loser loses one.
If a skill is "Unclashable" that attack will land, after oppsite land an attack first
If a skill has "Unbreakable" coins, Even if these coins lose a duel, they will still attack after being attacked by the enemy, but the damage will be reduced by half.

Buffs:
Damage Up: Increases final damage dealt.
Protection: Reduces damage taken (percentage).
Power Boost: Increases Base/Coin/Final Power for clashes/attacks.
and else...

# Credits & Disclaimer
This is a fan-made project written in C for educational purposes. Limbus Company and all related characters, names, and mechanics are property of Project Moon.







# Thai Version
# Imitation Limbus Company - เกมจำลองการต่อสู้แบบ Text RPG
โปรแกรมภาษา C ที่จำลองระบบการต่อสู้จากเกม Limbus Company โปรเจกต์นี้มีระบบต่อสู้แบบผลัดกันเล่น (Turn-based), ระบบ Sanity (ความมั่นใจ), ระบบการดวล (Clash), และสกิลพิเศษ (Passive) ที่หลากหลายสำหรับตัวละครต่างๆ

# ฟีเจอร์หลัก (Features)

# กลไกหลัก:
ระบบการดวล (Clash System): เปรียบเทียบพลังสกิลเพื่อทำลายเหรียญของศัตรู
ระบบ Sanity: ส่งผลต่อโอกาสได้หัว/ก้อยในการทอยเหรียญ และสถานะจิตใจ
สกิลพิเศษซับซ้อน: รวมเอากลไกเฉพาะตัวของตัวละคร

# วิธีการติดตั้งและรัน (How to Install and Run)
ความต้องการ: ต้องมี Compiler ภาษา C (เช่น gcc) และ Terminal/Command Prompt
คอมไพล์ (Compile):
gcc main.c -o limbus_game
รัน (Run):
./limbus_game
(สำหรับ Windows ให้รันไฟล์ .exe ที่เกิดขึ้น)

# วิธีการเล่น (How to Play)
เลือกตัวละคร (Identity): เลือกตัวละคร (พิมพ์ตัวเลข) จากรายชื่อ เกมจะแสดงสกิลพิเศษและสเตตัสของตัวละคร
เลือกศัตรู (Enemy): เลือกคู่ต่อสู้ (พิมพ์ตัวเลข) ตรวจสอบ HP และสกิลของศัตรู

# วงจรการต่อสู้ (Battle Loop):
เริ่มตาน (Turn Start): ตรวจสอบ HP, Sanity และโล่ป้องกัน อ่านเงื่อนไขของสกิลพิเศษที่ทำงาน
เลือกสกิล (Choose Skill): พิมพ์ 1 หรือ 2 เพื่อเลือกสกิลสำหรับเทิร์นนั้น
เฟสการดวล (Clash Phase): จำลองการทอยเหรียญ ฝั่งที่มีพลัง (Power) สูงกว่าจะชนะการทอยและทำลายเหรียญของอีกฝั่ง 1 เหรียญ
เฟสโจมตี (Attack Phase): เหรียญที่เหลืออยู่หลังจากการดวลจะสร้างความเสียหายให้กับ HP/โล่ของศัตรู
จบตาน (Turn End): อัปเดต Buffs, ปรับ Sanity และคำนวณดาเมจจากสถานะลุกไหม้ (Burn)
ชนะ/แพ้: การต่อสู้จะจบลงเมื่อ HP ของใครฝั่งหนึ่งตกถึง 0

# คำอธิบายกลไก (Mechanics Guide)
Sanity (ความมั่นใจ):
มีค่าตั้งแต่ -45 ถึง 45
ค่าสูง (>0): เพิ่มโอกาสได้หัว (เพิ่มพลัง) เมื่อทอยเหรียญ
ค่าต่ำ (<0): ลดโอกาสได้หัว
ตื่นตระหนก (Panic) ที่ -45: ตัวละครจะข้ามตานปกติ (ยกเว้นบางตัวที่ Immune)
Clash (การดวล):
สกิลทั้งสองฝั่งนำเหรียญมาดวลกันเหรียญต่อเหรียญ
ฝั่งชนะเก็บเหรียญไว้ ฝั่งแพ้เสียเหรียญ 1 เหรียญ
หากสกิลเป็นแบบ "Unclashable" (ปะทะไม่ได้) การโจมตีนั้นจะ เข้าเป้าแน่นอน โดยจะเกิดขึ้น หลังจากที่ฝ่ายตรงข้ามโจมตีใส่เราเสร็จสิ้นก่อนครับแล้วตัวละครของเราจึงจะโจมตีสวนกลับไปโดยอัตโนมัติ
หากสกิลมี "Unbreakable" (เหรียญป้องกัน) เหรียญเหล่าแม้จะแพ้การดวล แต่จะยังคงโจมตีหลังจะถูกศัตรูโจมตีแล้ว หากแต่ดาเมจจะลดลงครึ่งหนึ่ง
Buffs (เสริมพลัง):
Damage Up: เพิ่มดาเมจสุดท้ายที่สร้าง
Protection: ลดดาเมจที่ได้รับ (เป็นเปอร์เซ็นต์)
Power Boost: เพิ่มพลัง Base/Coin/Final สำหรับการดวลและโจมตี
และ อื่นๆ

# เครดิตและข้อจำกัด
โปรเจกต์นี้เป็นผลงานแฟนเมดขึ้นด้วยภาษา C เพื่อการศึกษา Limbus Company และตัวละคร ชื่อ รวมถึงกลไกต่างๆ เป็นลิขสิทธิ์ของ Project Moon
