10      REM
20      REM HIRES80 Picture Test
30      REM

40      REM
50      REM FOR EMULATOR : DEADC0DE.ORG/apple2
70      REM

90      REM
100     REM INITIALIZE
110     REM
120     STR80 = -16383 : COL80 = -16371 : NOMIX = -16302 : MBD = -16300
130     AUX = -16299 : FRCTXT = -16290 : HCLEER = 768

150     HGR : POKE STR80,0 : POKE NOMIX,0 : POKE COL80,0
160     POKE FRCTXT,0 : HCOLOR=3
170     REM MOVE "LDA #$20, JMP $F3EA" TO HCLEER
180     POKE HCLEER,169 : POKE HCLEER+1,32 : POKE HCLEER+2,76
190     POKE HCLEER+3,234 : POKE HCLEER+4,243
200     POKE AUX,0 : CALL HCLEER : REM CLEAR AUX

220     HPLOT 140,0 TO 140,26 : HPLOT 140,167 TO 140,191 : REM DRAW VERT LINE
230     POKE MBD,0 : HPLOT 139,0 TO 139,26 : HPLOT 139,167 TO 139,191

250     REM
260     REM LOAD DHIRES PICTURE
270     REM
280     PRINT CHR$(4)"BLOAD MEMMOVE.OBJ,A$1E80"
290     PRINT CHR$(4)"BLOAD AH_1,A$2000"
300     CALL 7808
310     POKE 7987,255 : REM PING TEST
320     PRINT CHR$(4)"BLOAD AH_0,A$2000"
330     POKE 7987,255 : REM PING TEST