#include "battle_city.h"
#include "map.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>

/*
 * GENERATED BY BC_MEM_PACKER
 * DATE: Wed Jul 08 21:00:48 2015
 */

// ***** 16x16 IMAGES *****
#define IMG_16x16_block			0x017F //2 - blok
#define IMG_16x16_enemie			0x023F //5 - nepr
#define IMG_16x16_bckgnd			0x027F //0 - poz
#define IMG_16x16_door		0x01FF //4 - vrata
#define IMG_16x16_bomberman			0x013F //1 - bomberm
#define IMG_16x16_brick	0x01BF //3 - cigla
#define IMG_16x16_bomb 		0x00FF //6 - bomba
// ***** MAP *****

#define MAP_BASE_ADDRESS			0x02BF // MAP_OFFSET in battle_city.vhd
#define MAP_X							0
#define MAP_X2							640
#define MAP_Y							4
#define MAP_W							64
#define MAP_H							56

#define REGS_BASE_ADDRESS               ( MAP_BASE_ADDRESS + MAP_WIDTH * MAP_HEIGHT )
//#define REGS_BASE_ADDRESS               (5439)

#define BTN_DOWN( b )                   ( !( b & 0x01 ) )
#define BTN_UP( b )                     ( !( b & 0x10 ) )
#define BTN_LEFT( b )                   ( !( b & 0x02 ) )
#define BTN_RIGHT( b )                  ( !( b & 0x08 ) )
#define BTN_SHOOT( b )                  ( !( b & 0x04 ) )

#define TANK1_REG_L                     8
#define TANK1_REG_H                     9
#define TANK_AI_REG_L                   4
#define TANK_AI_REG_H                   5
#define TANK_AI_REG_L2                  6
#define TANK_AI_REG_H2                  7
#define TANK_AI_REG_L3                  2
#define TANK_AI_REG_H3                  3
#define TANK_AI_REG_L4                  10
#define TANK_AI_REG_H4                  11
#define TANK_AI_REG_L5                  12
#define TANK_AI_REG_H5                  13
#define TANK_AI_REG_L6                  14
#define TANK_AI_REG_H6                  15
#define TANK_AI_REG_L7                  16
#define TANK_AI_REG_H7                  17
#define TANK_AI_REG_L8					18
#define TANK_AI_REG_H8					19
#define BASE_REG_L						0
#define BASE_REG_H	                    1


int lives = 0;
int score = 0;
int mapPart = 1;
int udario_glavom_skok = 0;
int map_move = 0;
int brojac = 0;
int udario_u_blok = 0;

typedef enum {
	b_false, b_true
} bool_t;

typedef enum {
	DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL, BOMB
} direction_t;

typedef struct {
	unsigned int x;
	unsigned int y;
	direction_t dir;
	unsigned int type;

	bool_t destroyed;

	unsigned int reg_l;
	unsigned int reg_h;
} characters;

characters bomberman = { 128,	                        // x
		49, 		                     // y
		DIR_RIGHT,              		// dir
		IMG_16x16_bomberman,  			// type

		b_false,                		// destroyed

		TANK1_REG_L,            		// reg_l
		TANK1_REG_H             		// reg_h
		};

characters enemie1 = { 304,						// x
		49,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemie,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L,            		// reg_l
		TANK_AI_REG_H             		// reg_h
		};

characters enemie2 = { 432,						// x
		81,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemie,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L2,            		// reg_l
		TANK_AI_REG_H2             		// reg_h
		};

characters enemie3 = { 330,						// x
		272,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemie,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L3,            		// reg_l
		TANK_AI_REG_H3             		// reg_h
		};

characters enemie4 = { 635,						// x
		431,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemie,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L4,            		// reg_l
		TANK_AI_REG_H4             		// reg_h
		};

characters enemie5 = { 635,						// x
		431,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemie,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L5,            		// reg_l
		TANK_AI_REG_H5             		// reg_h
		};

characters enemie6 = { 635,						// x
		431,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemie,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L6,            		// reg_l
		TANK_AI_REG_H6             		// reg_h
		};

characters enemie7 = { 635,						// x
		431,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_enemie,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L7,            		// reg_l
		TANK_AI_REG_H7             		// reg_h
		};

characters bombc = { 0,						// x
		0,						// y
		DIR_LEFT,              		// dir
		IMG_16x16_bomb,  		// type

		b_false,                		// destroyed

		TANK_AI_REG_L8,            		// reg_l
		TANK_AI_REG_H8             		// reg_h
		};

unsigned int rand_lfsr113(void) {
	static unsigned int z1 = 12345, z2 = 12345;
	unsigned int b;

	b = ((z1 << 6) ^ z1) >> 13;
	z1 = ((z1 & 4294967294U) << 18) ^ b;
	b = ((z2 << 2) ^ z2) >> 27;
	z2 = ((z2 & 4294967288U) << 2) ^ b;

	return (z1 ^ z2);
}

static void chhar_spawn(characters * chhar) {
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
			(unsigned int )0x8F000000 | (unsigned int )chhar->type);
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x);
}

static void map_update(characters * mario) {
	int x, y;
	long int addr;

	for (y = 0; y < MAP_HEIGHT; y++) {
		for (x = 0; x < MAP_WIDTH; x++) {
			addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
					+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
			switch (map1[y][x]) {
			case 0:
				Xil_Out32(addr, IMG_16x16_bckgnd);
				break;
			case 1:
				Xil_Out32(addr, IMG_16x16_bomberman);
				break;
			case 2:
				Xil_Out32(addr, IMG_16x16_block);
				break;
			case 3:
				Xil_Out32(addr, IMG_16x16_brick);
				break;
			case 4:
				Xil_Out32(addr, IMG_16x16_door);
				break;
			case 5:
				Xil_Out32(addr, IMG_16x16_enemie);
				break;
			case 6:
				Xil_Out32(addr, IMG_16x16_bomb);
				break;
			default:
				Xil_Out32(addr, IMG_16x16_bckgnd);
				break;
			}
		}
	}
}

static void map_reset(unsigned char * map) {

	unsigned int i;

	for (i = 0; i <= 20; i += 2) {
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + i ),
				(unsigned int )0x0F000000);
	}

}

int obstackles_detection(int x, int y, int deoMape, unsigned char * map,
		int dir) {
	unsigned char mario_position_right;
	unsigned char mario_position_left;
	unsigned char mario_position_up;
	unsigned char mario_position_down;

	float Xx = x;
	float Yy = y;

	int roundX = 0;
	int roundY = 0;

	roundX = floor(Xx / 16);
	roundY = floor(Yy / 16);

	mario_position_right = map1[roundY][roundX + 1];
	mario_position_left = map1[roundY][roundX - 1];
	mario_position_up = map1[roundY - 1][roundX];
	mario_position_down = map1[roundY + 1][roundX];
	if (dir == 0) {
		switch (mario_position_right) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 2;
			break;
		case 3:
			return 3;
			break;
		case 4:
			return 4;
			break;
		case 5:
			return 5;
			break;
		case 6:
			return 6;
			break;

		}
	} else if (dir == 1) {
		switch (mario_position_left) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 2;
			break;
		case 3:
			return 3;
			break;
		case 4:
			return 4;
			break;
		case 5:
			return 5;
			break;
		case 6:
			return 6;
			break;

		}
	} else if (dir == 2) {
		switch (mario_position_up) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 2;
			break;
		case 3:
			return 3;
			break;
		case 4:
			return 4;
			break;
		case 5:
			return 5;
			break;
		case 6:
			return 6;
			break;
		}
	}
	else if (dir == 3) {
			switch (mario_position_down) {
			case 0:
				return 0;
				break;
			case 1:
				return 1;
				break;
			case 2:
				return 2;
				break;
			case 3:
				return 3;
				break;
			case 4:
				return 4;
				break;
			case 5:
				return 5;
				break;
			case 6:
				return 6;
				break;
			}
		}

}


static bool_t mario_move(unsigned char * map, characters * mario,
		direction_t dir, int start_jump) {
	unsigned int x;
	unsigned int y;
	int i, j;

	int obstackle = 0;

	x = mario->x;
	y = mario->y;


	if (dir == DIR_LEFT) {
		obstackle = obstackles_detection(x, y, mapPart, map, 1);
		if(obstackle == 0){
			x-=16;
		}
	} else if (dir == DIR_RIGHT) {
		obstackle = obstackles_detection(x, y, mapPart, map, 0);
		if(obstackle == 0){
			x+=16;
		}
	} else if (dir == DIR_UP) {
		obstackle = obstackles_detection(x, y, mapPart, map, 2);
		if(obstackle == 0){
			y-=16;
		}
	} else if (dir == DIR_DOWN){
		obstackle = obstackles_detection(x, y, mapPart, map, 3);
		if(obstackle == 0){
			y+=16;
		}
	}


	mario->x = x;
	mario->y = y;

	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + mario->reg_h ),
			(mario->y << 16) | mario->x);

	for (i = 0; i < 1000000; i++) {
			}

	return b_false;
}

static void set_bomb(characters *bomberman, characters *bombc, int *bomb){

	if(bomberman->x == bombc->x && bomberman->y == bombc->y){

			  chhar_spawn(&bombc);
	}
}


void battle_city() {

	unsigned int buttons, tmpBtn = 0, tmpUp = 0;
	int i, change = 0, jumpFlag = 0;
	int block;
	int bomb = 0;

	map_reset(map1);
	map_update(&bomberman);

	//chhar_spawn(&enemie1);
	//chhar_spawn(&enemie2);
	//chhar_spawn(&enemie3);
	//chhar_spawn(&enemie4);
	chhar_spawn(&bomberman);

	while (1) {

		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );



		direction_t d = DIR_STILL;
		if (BTN_LEFT(buttons)) {
			d = DIR_LEFT;
		} else if (BTN_RIGHT(buttons)) {
			d = DIR_RIGHT;
		} else if (BTN_UP(buttons)){
			d = DIR_UP;
		} else if (BTN_DOWN(buttons)){
			d = DIR_DOWN;
		}

		if(BTN_SHOOT(buttons)){
			bomb = 1;
			bombc.x = bomberman.x;
			bombc.y = bomberman.y;
		}

		//set_bomb(&bomberman, &bombc, bomb);

		int start_jump = 0;
		mario_move(map1, &bomberman, d, start_jump);

		map_update(&bomberman);
		set_bomb(&bomberman, &bombc, &bomb);
		for (i = 0; i < 100000; i++) {
		}

	}
}

