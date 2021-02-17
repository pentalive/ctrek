
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


/*
** calculate the distance -thanks Pythagorus
*/
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
      if (b > 2) {
	b = 0;
      } else {
	b = 1;
      }

      k = random() % 10;
      if (k > 4) k = 0;
      if (b > 0) k++;
      
      bases = bases + b;
      klingons = klingons + k;

      quadrant[x][y][STAR] = s;
      quadrant[x][y][BASE] = b;
      quadrant[x][y][KLIG] = k;    
      quadrant[x][y][SCAN] = FALSE;
      
    }
  }
}

/*
** Build a sector as we arrive.
*/

void BuildSector(void) {

  int x,y,i,j;


  /*
  ** make our sector empty
  */
  for(x=1; x<9; x++) {
    for(y=1; y<9; y++) {
      sector[x][y]=EMTY;
    }
  }

  /*
  ** place the enterprise
  */
  sector[esx][esy] = ENTP;

  /*
  ** Place stuff first stars, then each in turn
  */
  for(i=STAR; i<KLIB; i++) {
    /* but don't place more enterprises */
    if (i != ENTP) {
      
      /* for each of this kind of item... */
      for(j=0; j<quadrant[eqx][eqy][i]; j++) {
	
	/* find an empty spot for the next item */
	do {
	  x = random() % 8 + 1;
	  y = random() % 8 + 1;
	} while (sector[x][y] != EMTY);

	/* and put it there */
	sector[x][y] = i;
      }
    }
  }
}

/* print the short name for a device */
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

/* print a long name for a device */
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


/* did a device fail when we tried to use it? */
int devicefail(int thisdevice) {
  
  if (device[thisdevice] > (random() % 99)) {
    return FALSE;
  } else {
    return TRUE;
  }
}
    


void DoDisplay(void) {

  int x,y;


  /* set condtion */
  cond=GREEN;
  for (x=-1; x<2; x++) {
    for (y=-1; y<2; y++) {
      if (sector[esx+x][esy+y] == BASE) {
	cond = DOCKED;
      }
      /* 
      ** while we are doing the items +-1 xy around us, 
      ** set the long range scan value to true.
      */
      quadrant[eqx+x][eqy+y][SCAN] = TRUE;
    }
  }

  /* a klingon anywhere in this quadrant - red alert */
  if (quadrant[eqx][eqy][KLIG] > 0) cond = RED;


  /* if docked resupply and repair */
  if (cond == DOCKED) {
    /* restock */
    energy = 5000;
    ptorps = 10;

    /* repairs */
    for (x = 1; x<8; x++) {
      device[x] = device[x] + rand() % 5;
      if (device[x] > 99) {
	device[x] = 99;
      }
    }
  }

  /*
  ** do the actual display
  */
  printf("CTREK  [q:%d,%d : s:%d,%d]  Energy:%d  (SBK)",
	 eqx,eqy,esx,esy,energy);

  switch (cond) {
  case GREEN: printf("-Green-"); break;
  case RED:   printf("--RED--"); break;
  case DOCKED:printf("Docked-"); break;
  }
  
  printf("\n Torps:%d ",ptorps);
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
  int x,y;     /* 1st & 2nd parameters for command usually x and y coord */
  int tx,ty;   /* Torpedo x and y as the torpedo travels to target       */
  int cdone;   /* True if Torpedo is moving false to exit move torp loop */
  int dist;    /* The result of a distance calculation                   */
  int percent; /* Energy applied to phasers increases chances of hitting */

  do {
    c = getchar();
  } while ( ! isalpha(tolower(c)));

  switch (c) {

    /* 
    ** stop for repairs, consume a day and repair some systems
    */
  case 'r':
    for (x = 1; x<8; x++) {
      device[x] = device[x] + rand() % 5;
      if (device[x] > 99) {
	device[x] = 99;
      }
    }
    if (energy < 3000) {
      energy = energy + 500;
    }
    if (energy >5000) {
      energy = 5000;
    }
    dayz--;
    break;

    /*
    ** quit - leave game
    */
  case 'q': done = TRUE; break;

    /*
    ** display help
    */
  case 'h':
    printf("                   --- commands ---                 \n");
    printf("Q quit        End game                                  \n");
    printf("W warp    x,y Move to another sector                    \n");
    printf("I impulse x,y Move within a sector                      \n");
    printf("T torpedo x,y Launch a photon torpedo                   \n");
    printf("P phasers %%w Attack using phasors                      \n");
    printf("R Repair  sys Fix systems (5 points unless docked)      \n");
    printf("H Help        This list of commands                     \n");
    printf("\n\n");
    break;


  case 'p':
    
    percent = 10*getdigit() + getdigit();
    if (quadrant[eqx][eqy][KLIG] > 0) {
      if (energy < (15 * percent)) {
	printf("Not enough energy to charge phaser banks\n");
      } else {
	energy = energy - 15 * percent;
	if (devicefail(PHA)) {
	  printf("Phaser system offline \n");
	} else {
	  for (x=1; x<9; x++) {
	    for(y=1; y<9; y++) {
	      if (sector[x][y] == KLIG) {
		printf("Klingon at %d,%d ",x,y);
		if (rand()%99 <= percent) {
		  printf("destroyed\n");
		  sector[x][y] = EMTY;
		  quadrant[eqx][eqy][KLIG]--;
		  klingons--;
		} else {
		  printf("survived\n");
		}
	      }
	    }
	  }
	}
      }
    } else {
      printf("No targets in this quadrant\n");
    }
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
	  printf("Torpedo track:");
	  do {
	    if (tx > x) tx--;
	    if (tx < x) tx++;
	    if (ty > y) ty--;
	    if (ty < y) ty++;

	    if (sector[tx][ty] == EMTY) {
	      printf(" %d,%d ",tx,ty);
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
    if (x>8 || y>8 || x<1 || y<1) {
      printf("Navigatonal computer rejects coordinates [%d,:%d]\n",x,y);
    } else {
      if (devicefail(WDA)) {
	printf("Warp Field Failure \n");
      } else {
	dist = distance(eqx,eqy,x,y);
	if ((50 * dist) > energy) {
	  printf("insufficient energy for that warp transit\n");
	} else {
	printf("Distance %d ",dist);
	printf("Warp tranist complete - record ship arival at %d,%d\n",x,y);
	eqx = x;
	eqy = y;
	dayz = dayz - dist;
	BuildSector();
	energy = energy - (50*dist);
	}
      }
    }
    break;
    
  case 'i':
    x = getdigit();
    y = getdigit();
    if ( x > 8 || y > 8 || x<1 || y<1 ) {
      printf("Navigational computer rejects coordinate %d,%d\n",x,y);
    } else {
      if (devicefail(IDA)) {
	printf("Impulse Drive failure\n");
      } else {
	if (energy < 3000) {
	  printf("Insufficient energyd for impulse transit\n");
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
    }
    break;
  }
}

void DoKlingons(void) {
  int i,j;  /* a coordinate */
  int dmg;  /* damage done  */
  int sys;  /* system getting damaged */

  
  for(i=1; i<9; i++) {
    for(j=1; j<9; j++) {
      
      /* not in same quad with enterprise */
      if (i!= eqx && j != eqy) {
	
	/* A base and a klingon in same quadrant */
	if (quadrant[i][j][BASE] == 1 && quadrant[i][j][KLIG]>1) {
	  
	  printf("Base at %d,%d is under attack -",i,j);
	  /* a small chance the klingons kill the base */
	  if (rand() % 99 < 2) {
	    printf("and was destroyed \n");
	    quadrant[i][j][BASE] = 0;
	  } else {
	    printf("and continues to survive \n");
	  }
	}
      }
    }
  }

  /*
  ** In our quadrant
  */
  for(i=1; i<9; i++) {
    for(j=1; j<9; j++) {
      if (sector[i][j] == KLIG) {
	dmg = rand() % 20;
	sys = (rand() % 7) + 1;
	device[sys] = device[sys] - dmg;
	if (device[sys] < 0) {
	  device[sys] = 0;
	}
	/* DoDisplay(); */
	printf("Klingon at [%d,%d] fires - %d damage to ",i,j,dmg);
	ShortNameDevice(sys);
	printf("\n");
      }
    } 
  }
}



int main(int argc, char ** argv) {

  int i,j;

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
    DoKlingons();

    if (energy < 100) {
      printf("Out of energy, the enterprise floats listlessley waiting \n");
      printf("rescue from the nearest Klingon\n");
      done = TRUE;
    }

    if (klingons = 0) {
      printf("Your mission was a sucess, \n");
      printf("the Klingon threat has been removed\n");
      done = TRUE;
    }

    j = 0;
    for (i=1; i<9; i++) {
      j = j + device[i];
    }
    if (j < 5) {
      printf("Your mission failed\n");
      printf("The destroyed hulk of the enterprise floats \n");
      printf("listlessly in space\n");
      done = TRUE;
    }

    
  } while (! done);
}

