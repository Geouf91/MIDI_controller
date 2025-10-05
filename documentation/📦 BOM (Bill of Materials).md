**📦 BOM (Bill of Materials)**



**\*\*Microcontrôleur \& Programmation\*\***



1 × ATmega328P-PU (version DIP pour prototypage ou TQFP pour PCB)



1 × Quartz 16 MHz



2 × Condensateurs 22 pF (pour le quartz)



1 × Résistance 10 kΩ (pull-up RESET)



*```*

*On ne peut pas alimenter directment par le CH340G et j'ai une idee pour reprogrammer si besoin, donc cette  partie n'est pas necessaire:*



*1 × CH340G (interface USB ↔ UART, pour programmation et debug série)*



*1 × Quartz 12 MHz (pour CH340G)*



*2 × Condensateurs 22 pF (pour quartz du CH340G)*

*```*





**\*\*Expandeurs d’E/S\*\***



3 × MCP23017 (I²C → 16 GPIO chacun, total 48 GPIO)



3 × Résistances 10 kΩ (pull-up SDA/SCL si pas déjà intégrées sur ton module OLED)



\*\*Encodeurs rotatifs\*\*



12 × Encodeurs rotatifs avec bouton poussoir intégré (par ex. EC11, assez standard)



*```*

*12 × Cadrans / boutons pour encodeurs (knobs)*

*On imprime en 3D les boutons ou tu preferes les commander ?*

*```*





**\*\*\*Affichage\*\* (deja recu)**



1 × OLED SSD1306 I²C (128×64 px, 0.96" ou 1.3", interface I²C, 4 broches : VCC, GND, SDA, SCL)\*





**\*\*MIDI DIN \& communication\*\***



1 × DIN-5 femelle (MIDI IN/OUT) (prévoir 2 si tu veux IN + OUT/THRU)



2 × Résistances 220 Ω (pour série sur TX MIDI OUT, standard MIDI)



1 × Transistor NPN (2N3904 par ex.) (pour driver MIDI OUT si tu veux strictement respecter la spec)





 **\*\*Alimentation\*\***



1 × Régulateur 5 V (AMS1117-5.0 ou mieux un buck LM2596 si entrée >7 V)



Condensateurs de découplage :

-> 4 × 100 nF céramique (près de chaque MCP23017 et MCU)

-> 2 × 10 µF électrolytiques (entrée/sortie régulateur)



1 × Connecteur alimentation (jack DC 5.5×2.1 mm, ou borne à vis)

