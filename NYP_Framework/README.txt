DM2292 Studio Project 3 Group 9

Controls:
[Planet Selection]
Left click : select planet to visit
Enter : visit selected planet

[Ship Combat]
WASD : move around the ship
Left click : select action

[Platforming Planets/Levels, Common]
AD : move and face left and right
WS : move up and down on vines, face up and down
E : shoot projectile
R : give 1 life to respawn at checkpoint
I : bring up planet inventory

[Planet Inventory, Common]
Left click : (on resource) destroy 1 of said resource

[Jungle Planet] 
Q : (while in river water) collect river water; (while standing on an unbloomed bouncy bloom) bloom the bouncy bloom; (else) heal 
    player's health by 10 and reduce poison level by 1
F : tie vine around rock (when standing by a rock with vine in inventory)
G : place burnable block (when with burnable blocks in inventory)

[Terrestrial Planet]
1 : change colour to yellow (when yellow orb in possession and charged)
2 : change colour to red (when red orb in possession and charged)
3 : change colour to green (when green orb in possession and charged)
4 : change colour to blue (when blue orb in possession and charged)

[Snow Planet]
Q : activate shield mode (when shield power up in possession and unused)
F : freeze all water in the level (when ice shard power up in possession and unused)
G : activate beserk mode (when beserk power up in possession and unused)

[Ship Upgrade]
WASD : move around the ship
Left click : select action

----------------------------------------------------------------------------------------------------------------------------------------
How to Play:
[Planet Selection]
Any coloured planet can be visited for you to collect resources from.

[Ship Combat]
Use the top right console (green) to attack enemy spacecrafts and restart ventilation to keep your oxygen level high. Repair your ship
to prevent a game over from ship damage. Once all enemies are defeated, the top middle door will light up blue which means you can leave
to visit the planet you have landed at.

[Platforming Planets/Levels]
Platform through using the controls to collect resources and make it back to the ship by making it to the blue door.

[Ship Upgrade]
Upgrade storage to hold more items. Upgrade ventilation and weapons along the side to aid in ship combat. Once all upgrades are done,
use the top left console to select which planet to visit next.

----------------------------------------------------------------------------------------------------------------------------------------
Tile Index:
0 : empty space
1 : generic resource (pick scrap metal or battery to spawn)

Interactables
[Jungle Planet]
2 : poison sprout
3 : poison fog 
4 : poison explosion
10 : river water 
11 : enemy (intelligent tracker) healing spot in river water
20 : unbloomed bouncy bloom
21 : bloomed bouncy bloom
30 : rock (no vine)
31 : rock with vine tied to it, vine leading to the left
32 : vine left corner (connects vine tied with rock to dangling vine)
33 : vine dangling on the left of its right neighbour
34 : rock with vine tied to it, vine leading to the right
35 : vine right corner (connects vine tied with rock to dangling vine)
36 : vine dangling on the right of its left neighbour
40 : spot to set moving leaves
41 : transitional block used to switch moving leaf blocks
50 : temporary "poof" teleportation effect left behind
51 : down button pressed
52 : up button pressed
53 : right button pressed
54 : left button pressed
55 : invisible trigger for shooting pop up
60 : down button 1
61 : right button 1
62 : left button 1
64 : down button 2
68 : down button 3
100 - 109 : waypoints for the first Vigilant Teleporter of the map
130 - 139 : waypoints for the first Patrol Team of the map
140 - 149 : waypoints for the second Patrol Team of the map
150 - 159 : waypoints for the third Patrol Team of the map
197 : burnable block collectable
198 : vine collectable
199 : ironwood

[Terrestrial Planet]
201 : yellow tile (hollow)
202 : red tile (hollow)
203 : green tile (hollow)
204 : blue tile (hollow)
205 : brown tile (hollow)
210 : rope post (coiled)
211 : rope post (uncoiled left)
212 : rope corner (left)
213 : rope length (left)
214 : rope post (uncoiled right)
215 : rope corner (right)
216 : rope length (right)
221 : yellow orb
222 : red orb
223 : green orb
224 : blue orb
230 : black flag
231 : red flag
240 : spikes (up)
241 : spikes (left)
242 : spikes (down)
243 : spikes (right)
250 : alarm box
251 : alarm light (off)
252 : alarm light (on)
260 : antidote pill
270 : switch (left)
271 : switch (right)
299 : exit door
300-398 : waypoints
399 : energy quartz

[Snow Planet]
400-488:waypoints
489-499: sign (tutorial)
500: fur
501: fur coat
502: power-up shield
503: power-up berserk
504: power-up freeze water shard
505: ice rope top
506: ice rope mid
507: ice rope bot
595: water top
596: water
599 : ice crystal


Solid Blocks
[Jungle Planet]
600 : burnable bush
601 : burnable bush on fire
602 : burnable bush being dissolved
610 : grass ground
611 : grass ground top left rounded corner
612 : grass ground top right rounded corner
620 : moving leaves
621 : leaves
622 : ironwood tree trunk
630 : stone ground
631 : stone ground grassy
632 : stone ground mossy
640 : metal barrier blocks that down button 1 will make disappear
641 : metal barrier blocks that right button 1 will make disappear
642 : metal barrier blocks that left button 1 will make disappear
644 : metal barrier blocks that down button 2 will make disappear
648 : metal barrier blocks that down button 3 will make disappear

[Terrestrial Planet]
800 : ground tile (solid)
801 : yellow tile (solid)
802 : red tile (solid)
803 : green tile (solid)
804 : blue tile (solid)
805 : brown tile (solid)
810 : ground wire corner (bottom left)
811 : ground wire corner (bottom right)
812 : ground wire corner (top left)
813 : ground wire corner (top right)
814 : ground wire length (horizontal)
815 : ground wire length (vertical)

[Snow Planet]
1000: regular ground with snow (top snow)
1001: regular ground with snow (left snow)
1002: regular ground with snow (right snow)
1003: regular ground with snow (bottom snow)
1004: regular ground without snow
1100: ice

[Space Ship]
1210: Planet / Space Ship Interior wall
1211: Ship corner 1
1212: Ship corner 2
1213: Ship corner 3
1214: Ship corner 4
1215: Ship left wall


Player : 1400

Enemies
[Jungle Planet]
1600 : vigilant teleporter
1610 : shy chaser
1620 : patrol team
1630 : intelligent tracker

[Terrestrial Planet]
1800 : Sentry
1801 : Veteran
1802 : Turret
1803 : Dummy

[Snow Planet]
2000: Snow Wolf Brown
2001: Snow Wolf White
2002: Snow Wolf Boss 