
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#define TRUE 1
#define FALSE 0


#define SCAN 0


/*
**Global Variables
*/

enum conditions
  {
    DOCKED,   /* Docked next to a starbase with no klingons around */
    RED,      /* Klingons in this quadrant                         */
    GREEN     /* Not Docked no klingons                            */
  };

enum systems
  {
    NOT, /* No device type 0 place holder */
    SRS, /* Short Range Sensors    */
    LRS, /* Long Range Sensors     */
    IDA, /* Impulse Drive Assembly */
    WDA, /* Warp Drive Assembly    */
    PHA, /* Phasers                */
    PHO, /* Photon Torpedos        */
    DMG, /* Damage Control         */
    LSS  /* Life Support Systems   */
  };


enum thingsinspace
  {
    EMTY, /* Empty Space                           */
    STAR, /* a Star                                */
    BASE, /* One of our bases                      */
    KLIG, /* Klingon Ship                          */
    ENTP, /* US - The heros!                       */
    KLIB, /* A Klingon Base   -not using these yet */
    PTRP  /* A photon torpedo -not using these yet */
  };


/* 
** I am using elements 1 to 8 even though element 0 and 9 are available
** because of this when I check the nighbors around a specific cell I do not 
** have to worry about wraping or doing special tests to insure I am looking 
** in a valid (1-8) cell as [0] and [9] will automatically be empty
**
** Because of this (as I am displaying the 1-8 elements my devices must be
** numbered 1 to 8 inclusive as well.
*/

int quadrant[10][10][4];
int sector[10][10];
int device[10];

int eqx,eqy,esx,esy;        /* Enterprise Location                         */
int bases, klingons,dayz;   /* counts of bases, klingons, days to complete */
int done;                   /* game done? (out of days,energy or klingons?)*/
int cond;                   /* current sector condition (red,green,docked) */
int ptorps;                 /* current supply of photon torpedos           */
int energy;                 /* current supply of energy                    */


int distance(int x1, int y1, int x2, int y2) {
  return (int) sqrtf( (float) (((x1-x2)*(x1-x2)) + ((y1-y2)*(y1-y2)))   );
}



/*
** setup
**
** Perform one time setup steps.
*/
void Setup(void) {

  int i;
  
  srandom(time(NULL));
  
  eqx = random() % 8 + 1;
  eqy = random() % 8 + 1;
  esx = random() % 8 + 1;
  esy = random() % 8 + 1;

  energy = 5000;
  ptorps = 10;
  
  for(i=1; i<=8; i++) device[i]=99;

}


/*
** build quadrant
**
** create quadrants preparing for game
*/
void BuildQuadrant(void) {
  int x,y;
  int s,b,k;

  bases = 0;
  klingons = 0;
  
    
  for (x=1; x<9; x++) {
    for (y=1; y<9; y++) {

      s = random() % 10;
      if (s > 3) s = 1;
      
      b = random() % 6;
      if (b > 2) b = 0;

      k = random() % 10;
      if (k > 4) k = 0;
      if (b > 0) k++;
      
      bases = bases + b;
      klingons = klingons + k;

      quadrant[x][y][STAR] = s;
      quadrant[x][y][BASE] = b;
      quadrant[x][y][KLIG] = k;    
      quadrant[x][y][SCAN] = TRUE;
      
    }
  }
}

void BuildSector(void) {

  int x,y,i,j;
  
  for(x=1; x<9; x++) {
    for(y=1; y<9; y++) {
      sector[x][y]=EMTY;
    }
  }

  sector[esx][esy] = ENTP;

  for(i=STAR; i<KLIB; i++) {
    if (i != ENTP) {
      for(j=0; j<quadrant[eqx][eqy][i]; j++) {
	do {
	  x = random() % 8 + 1;
	  y = random() % 8 + 1;
	} while (sector[x][y] != EMTY);
	sector[x][y] = i;
      }
    }
  }
}

void ShortNameDevice(int which) {
  switch (which) {
  case NOT: printf("NOT"); break;
  case SRS: printf("SRS"); break;
  case LRS: printf("LRS"); break;
  case WDA: printf("WDA"); break;
  case IDA: printf("IDA"); break;
  case PHA: printf("PHA"); break;
  case PHO: printf("PHO"); break;
  case DMG: printf("DMG"); break;
  case LSS: printf("LSS"); break;
  }
}

void LongNameDevice(int which) {
  switch (which) {
  case NOT: printf("Device zero");break;
  case SRS: printf("Short Range Sensors"); break;
  case LRS: printf("Long Range Sensors"); break;
  case WDA: printf("Warp Drive Assembly"); break;
  case IDA: printf("Impulse Drive Assembly"); break;
  case PHA: printf("Phasors"); break;
  case PHO: printf("Photon Torpedos"); break;
  case DMG: printf("Damage Controll"); break;
  case LSS: printf("Life Support"); break;
  }
}


int devicefail(int thisdevice) {
  
  if (device[thisdevice] > (random() % 99)) {
    return FALSE;
  } else {
    return TRUE;
  }
}
    


void DoDisplay(void) {

  int x,y;


 
  cond=GREEN;
  for (x=-1; x<2; x++) {
    for (y=-1; y<2; y++) {
      if (sector[esx+x][esy+y] == BASE) {
	cond = DOCKED;
      }
    }
  }
  if (quadrant[eqx][eqy][KLIG] > 0) cond = RED;

  if (cond == DOCKED) {
    energy = 5000;
    ptorps = 10;
  }

  printf("CTREK  [%d,%d : %d,%d]  Energy:%d  (SBK)",eqx,esx,eqy,esy,energy);

  switch (cond) {
  case GREEN: printf("Green "); break;
  case RED:   printf(" RED  "); break;
  case DOCKED:printf("Docked"); break;
  }
  printf(" Torps:%d ",ptorps);
  printf(" Bases/Klingons: %d/%d",bases,klingons);
  printf("\n");

  printf("    1  2  3  4  5  6  7  8  sys %%%%");
  printf("    1    2    3    4    5    6    7    8\n");
  for (x=1; x<9; x++) {

    printf("%d :",x);


    /* short range sensors */
    for (y=1; y<9; y++) {
      
      if (esx==x && esy==y) {
	printf("[");
      } else {
	printf(" ");
      }

      if (devicefail(SRS)) {
	printf("@");
      } else {	
	switch (sector[x][y]) {
	case EMTY: printf("."); break;
	case STAR: printf("*"); break;
	case BASE: printf("#"); break;
	case ENTP: printf("+"); break;
	case KLIG: printf("-"); break;
	case KLIB: printf("="); break;
	case PTRP: printf("~"); break;
	default: printf("?"); break;
	}
      }
      
      if (esx==x && esy==y) {
	printf("]");
      } else {
	printf(" ");
      }

    }

    /* device status */

    printf("|");
    if (devicefail(DMG)){
      printf("@@@ @@ |");
    } else {
      ShortNameDevice(x);
      printf(" %02d |", device[x]);
    }
    
    /* long range sensors */
    for(y=1; y<9; y++) {

      
      if (eqx==x && eqy==y) {
	printf("[");
      } else {
	printf(" ");
      }

      if (devicefail(LRS)) {
	printf("@@@");
      }else{
	if (quadrant[x][y][SCAN]) {
	  printf("%d",quadrant[x][y][STAR]);
	  printf("%d",quadrant[x][y][BASE]);
	  printf("%d",quadrant[x][y][KLIG]);
	} else {
	  printf("...");
	}
      }
      
      if (eqx==x && eqy==y) {
	printf("]");
      } else {
	printf(" ");
      }
    }
    printf("\n");
  }
  printf(" Days: %d \n",dayz);
}

/*
** get a digit out of the input stream tossing all other character types
** return as an int between 0 and 9 inclusive.
*/
int getdigit(void) {
  char c;
  int i;

  do {
    c = getchar();
  } while (! isdigit(c));
  i = c - '0';
  return i;
}


void DoCommand(void) {
  char c,w;
  int x,y;    /* 1st & 2nd parameters for command usually x and y coord */
  int tx,ty;  /* Torpedo x and y as the torpedo travels to target       */
  int cdone;  /* True if Torpedo is moving false to exit move torp loop */
  int dist;   /* The result of a distance calculation                   */

  do {
    c = getchar();
  } while ( ! isalpha(tolower(c)));

  switch (c) {
  case 'q': done = TRUE; break;

  case 'h':
    printf("                   --- commands ---                 \n");
    printf("Q quit        End game                                  \n");
    printf("W warp    x,y Move to another sector                    \n");
    printf("I impulse x,y Move within a sector                      \n");
    printf("T torpedo x,y Launch a photon torpedo                   \n");
    printf("P phasers %pw Attack using phasors                      \n");
    printf("R Repair  sys Fix systems (5 points unless docked)      \n");
    printf("H Help        This list of commands                     \n");
    printf("\n\n");
    break;

    
  case 't':
    x = getdigit();
    y = getdigit();
    cdone = FALSE;
    if (x <=0 || x > 8 || y<=0 || y>8) {
      printf("Targeting computer rejects your coordinates\n");
    } else {
      if (sector[x][y] != KLIG) {
	printf("No viabale target at %d,%d\n",x,y);
      } else {
	if (devicefail(PHO)) {
	  printf("Photon torpedo system failure \n");
	} else {
	  printf("Torpedo Away...\n");
	  ptorps--;
	  tx = esx;
	  ty = esy;
	  do {
	    if (tx > x) tx--;
	    if (tx < x) tx++;
	    if (ty > y) ty--;
	    if (ty < y) ty++;
	  
	    if (sector[tx][ty] == EMTY) {
	      printf("Torpedo Track %d,%d\n",tx,ty);
	    } else {
	      if (sector[tx][ty] == KLIG) {
		printf("The Klingon at %d,%d destroyed\n",tx,ty);
		sector[tx][ty] = EMTY;
		quadrant[eqx][eqy][KLIG]--;
		klingons--;
		cdone = TRUE;
	      }
	      if (sector[tx][ty] == STAR) {
		printf("The star at %d,%d burps\n",tx,ty);
		cdone = TRUE;
	      }
	      if (sector[tx][ty] == BASE) {
		printf("The base at %d,%d destroyed - Way to go\n",tx,ty);
		bases--;
		quadrant[eqx][eqy][BASE]--;
		sector[tx][ty] = EMTY;
		cdone = TRUE;
	      }
	    }
	    if (tx == x && ty == y) cdone = TRUE;
	    if (tx > 8 || tx < 1 || ty > 8 || ty < 1) cdone = TRUE;
	  } while (! cdone);
	}
      }
    }
    break;

    
  case 'w':
    x = getdigit();
    y = getdigit();
    if (x>8 || y>8) {
      printf("Navigatonal computer rejects coordinates [%d,:%d]\n",x,y);
    } else {
      if (devicefail(WDA)) {
	printf("Warp Field Failure \n");
      } else {
	dist = distance(eqx,eqy,x,y);
	printf("Distance %d ",dist);
	printf("Warp tranist complete - record ship arival at %d,%d\n",x,y);
	eqx = x;
	eqy = y;
	dayz = dayz - dist;
	BuildSector();
	energy = energy - (50*dist);
      }
    }
    break;
    
  case 'i':
    x = getdigit();
    y = getdigit();
    if ( x > 8 || y > 8) {
      printf("Navigational computer rejects coordinate %d,%d\n",x,y);
    } else {
      if (devicefail(IDA)) {
	printf("Impulse Drive failure\n");
      } else {
	if (sector[x][y] != EMTY) {
	  printf("Destination occupied transit inhibited\n");
	} else {
	  printf("Transit to %d,%d complete\n",x,y);
	  energy = energy - 3000;
	  sector[esx][esy] = EMTY;
	  sector[x][y] = ENTP;
	  esx = x;
	  esy = y;
	}
      }
    }
    break;
  }
}


int main(int argc, char ** argv) {

  

  Setup();
  BuildQuadrant();
  BuildSector();

  dayz = (klingons * 30) / bases;

  printf("You have %d days to seek and destroy %d klingons\n",dayz,klingons);
  printf("with support from %d bases\n",bases);

  done = FALSE;
  do {
    DoDisplay();
    DoCommand();
  } while (! done);
}

