#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "xsdk.h"

#define VER "Version 1.20"
#define NOTSYSOP <90

typedef struct {          
                char uname[26];     /* Winner of Last Lottery */
				uint usrnum;        /* User number of Last Winner */
                ulong cdtswon,      /* How many credits were won last */
				      cdts;         /* Credits in Lottery, always >= 0 */
                uint win1,          /* Winning Number for this Lottery */
                     win2,
                     win3;
				} lottery;     	    /* Structure for the Bank data file */

lottery lotto;                      /* Lottery Data Structure */

ulong credits=0, src_credits=0, tgt_credits=0;
char src_name[81], tgt_name[81], out[128], in[128], moddir[128];
int src_num=0, tgt_num=0, i, j, length;
int num1=0, num2=0, num3=0, MAX=0;
ulong COST = 0L, START_POT = 0L;
node_t *node;
char yn;
int LOG = 0, DEBUG=0, MSGALL=0;

void moduserdat(void);

void switches(char *swtch)
{
if (!strcmp(swtch, "DEBUG"))
  DEBUG=1;
if (!strcmp(swtch, "LOG"))
  LOG=1;
if (!strcmp(swtch, "MSGALL"))
  MSGALL=1;
}

/****************************************************************************/
/* Updates the MODUSER.DAT file that SBBS reads to ajust the user's credits */
/* This function is called whenever the user's credits are adjust so that   */
/* the file will be current in any event.									*/
/****************************************************************************/
void moduserdat()
{
	char str[128];
	FILE *stream;

sprintf(str,"%sMODUSER.DAT",node_dir);
if((stream=fopen(str,"wt"))==NULL) {
	bprintf("Error opening %s for write\r\n",str);
	return; }
if (DEBUG)	
  bprintf("\nN1KCredits written to MODUSER.DAT: %ld.N\n", (long)src_credits);	
fprintf(stream,"%ld",(long)src_credits);
fclose(stream);

}

int golotto(void)
{
	char str[128];
	int file;
	unsigned long int size =0L;
	char msg[128];
    int win = 0;

size=(unsigned long int)sizeof(lotto);
sprintf(str,"%sLOTTERY.DAT",data_dir);
if ((file=nopen(str, O_RDWR))==-1) {
	bprintf("\nHRCreating %sN\r\n",str);
	close(file);
	if ((file=nopen(str, O_RDWR|O_CREAT))==-1) {
	  bprintf("HRCan't create %s.  Aborting.N\r\n", str);
	  sprintf(msg, "HRCouldn't create %s while %s was on!N\n", str, src_name);
	  putsmsg(1, msg);
	  pause();
	  exit(0);
    }
    strcpy(lotto.uname, sys_op);
    lotto.usrnum = 1;
    lotto.cdts = START_POT;
    lotto.cdtswon = 0L;
    randomize();
    lotto.win1 = (uint) random(MAX) + 1;
    lotto.win2 = (uint) random(MAX) + 1;
    lotto.win3 = (uint) random(MAX) + 1;
    if (DEBUG)
      bprintf("\nWCurrent Jackpot numbers are: %d %d %d.\n\nN", lotto.win1, lotto.win2, lotto.win3);    
}
lseek(file, 0L, SEEK_SET);
if (DEBUG)
  bprintf("\n\nN1KEntering LOTTERY function while loop to read LOTTERY.DAT.N\n");	
while (!eof(file))
  read(file, &lotto, (unsigned)size);	
if (DEBUG) {
  bprintf("\nWCurrent Lottery Jackpot numbers are: %d %d %d.\n\nN", lotto.win1, lotto.win2, lotto.win3);  
  bprintf("\nWYour numbers are:            %d %d %d.\n\nN", num1, num2, num3);
}  
lotto.cdts=lotto.cdts+COST;
bprintf("\nGCurrent lottery jackpot is NRHI%ldNG credits.", lotto.cdts);
bprintf("\nG%s won the last lottery jackpot for NRH%ldNG credits.", lotto.uname, lotto.cdtswon);
src_credits=-COST;
if ((lotto.win1==num1) && (lotto.win2==num2) && (lotto.win3==num3)) {
  win = 1;
  bprintf("\n\nNHRYou win the Logoff Lottery worth NHYI%ldNHR!!!", lotto.cdts);
  bputs("\nNBHDon't forget you can share your winnings using the Credit Transfer Bank!!N");
  bprintf("\n\nNHYPlay the Logoff Lottery next time you call %s!!N\n\n", sys_name);
  strcpy(lotto.uname, user_name);
  lotto.usrnum = user_number;
  lotto.cdtswon=lotto.cdts;
  src_credits=lotto.cdts;
  lotto.cdts = START_POT;
  randomize();
  lotto.win1 = (uint) random(MAX) + 1;
  lotto.win2 = (uint) random(MAX) + 1;
  lotto.win3 = (uint) random(MAX) + 1;  
  sprintf(msg, "YHLottery: NHC%sNG won the lottery of RHI%ldNG credits!!N\r\n", lotto.uname, lotto.cdtswon);
  putsmsg(1,msg);
  pause();
}
lseek(file, 0L, SEEK_SET);
write(file, &lotto, (unsigned)size);
close(file);
moduserdat();
return (win);
}	

int main(int argc, char **argv)
{
char *p;
cls();

p=getenv("SBBSNODE");
if(!node_dir[0] && p)
	strcpy(node_dir,p);
if((!node_dir[0]) || (argc < 4)){	  /* node directory not specified */
	bputs("RHISBBSNODE variable must be set!N\n\r");
	bputs("Rusage: lottery <1-9> <Cost to play> <Starting Pot> [debug] [msgall]\n");
    bputs("R       A number between 1 and 9 inclusive for range.N\n");
    bputs("R       Credits to deducted from user for playing!N\n");
    bputs("R       How much to start lottery jackpot when someone wins!N\n");
	bputs("Roptions: debug prints info on the screen to help find problems.N\n");
    bputs("R         msgall will send a message to user 1 whenever someone plays!N\n\r");
    bputs("YOrder of the options is very important!!N\n");

	getch();
	return(1); }
if(node_dir[strlen(node_dir)-1]!='\\')  /* make sure node_dir ends in '\' */
	strcat(node_dir,"\\");

initdata(); 								/* read XTRN.DAT and more */	
checkline();								/* Check for Carrier */

for (i=1; i<argc; i++) {
   length = strlen(argv[i]);
   for (j=0; j<length; j++)
	 argv[i][j] = toupper(argv[i][j]);
   if (DEBUG)
	 bprintf("Argv[%d] = %s!\n", i, argv[i]);
}
for (i=1; i<argc; i++)
  switches(argv[i]);

MAX=atoi(argv[1]);
COST=(ulong)atol(argv[2]);
START_POT=(ulong)atol(argv[3]);

if ((MAX < 0) || (MAX > 9)) {
  bputs("\nHRINumber must be between 1 and 9 inclusive!");
  exit(0);
}  
  
bprintf("\n\rBHLogoff Lottery %sN", VER);
bprintf("\nMH(C)1994 by Dean Lodzinski, Hologram ComputingN");
bprintf("\nMH908-727-1914 Fidonet 1:107/633 Atarinet 51:4/0N");
bprintf("\nYH(XSDK v%s)N\r\n",xsdk_ver);

nodesync();

bprintf("\nNGPlay the Logoff Lottery on %s!\n", sys_name);
bprintf("NGIt only costs RH%ldNG credits to play!\n", COST);
if (user_cdt < COST) {
  bputs("\nHRSorry, you don't have enough credits to play!!!N\n");
  pause();
  exit(0);
}
checkline();
if (!yesno("\nGWould you like to play the Logoff Lottery?")) {
  bprintf("\nGSee you next time!\nN");
  exit(0);
}
if (MSGALL) {
  sprintf(out, "YHLottery: NCH%s NGplayed the lottery!!N\r\n", user_name);
  putsmsg(1, out);
}
while ( (num1 < 1) || (num1 > MAX) ) {
  checkline();
  nodesync();
  bprintf("\nNYHEnter 1st number from 1 to %d : ", MAX);
  getstr(in, 1, K_NUMBER);
  num1=(uint)atoi(in);
}
while ( (num2 < 1) || (num2 > MAX) ) {
  checkline();
  nodesync();
  bprintf("\nNYHEnter 2nd number from 1 to %d : ", MAX);
  getstr(in, 1, K_NUMBER);
  num2=(uint)atoi(in);
}  
while ( (num3 < 1) || (num3 > MAX) ) {
  nodesync();
  checkline();
  bprintf("\nNYHEnter 3rd number from 1 to %d : ", MAX);
  getstr(in, 1, K_NUMBER);
  num3=(uint)atoi(in);
}

bprintf("\n\nNCYour numbers are: YH%d %d %dNC.\n", num1, num2,num3);

if (!golotto()) {
  bputs("\n\nNMHSorry, you didn't win the Logoff Lottery, but thanks for playing.");
  bputs("\nNMHThe winning numbers only change when someone wins, so remember your picks!N");
  bputs("\nNMHBetter luck next time!!\n\nN");
}
return(1);
}

