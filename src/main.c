#include <stm32f031x6.h>
#include "display.h"
#include "structures.h"
#include "serial.h"
#include <stdlib.h>
#include <stdint.h>

void initClock(void);
void initSysTick(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);

volatile uint32_t milliseconds;


// Screen size 128 160 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160


int rightPressed()
{
	return (GPIOB->IDR & (1 << 4))==0;
}
int leftPressed()
{
	return (GPIOB->IDR & (1 << 5))==0;
}
int upPressed()
{
	return (GPIOA->IDR & (1 << 8)) == 0;
}
int downPressed()
{
	return (GPIOA->IDR & (1 << 11)) == 0;
}

int firePressed()
{
	return (GPIOF->IDR & (1 << 1)) == 0;
}

uint32_t prbs(void);
uint32_t my_random(uint32_t lower,uint32_t upper)
{
    return (prbs()%(upper-lower))+lower;
}


uint32_t shift_register=1234;
uint32_t prbs()
{
	// This is an unverified 31 bit PRBS generator
	// It should be maximum length but this has not been verified 
	unsigned long new_bit=0;
	static int busy=0; // need to prevent re-entrancy here
	if (!busy)
	{
		busy=1;
		new_bit= ((shift_register & (1<<27))>>27) ^ ((shift_register & (1<<30))>>30);
		new_bit= ~new_bit;
		new_bit = new_bit & 1;
		shift_register=shift_register << 1;
		shift_register=shift_register | (new_bit);
		busy=0;
	}
	return shift_register & 0x7fffffff; // return 31 LSB's 
}


const uint16_t valera_14_26[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,57238,57238,57238,57238,57238,0,0,0,0,0,0,14909,23429,23429,57238,57238,65535,1994,57238,0,0,0,0,0,0,14909,23429,57238,57238,57238,57238,57238,57238,0,0,0,0,0,0,14909,23429,23429,57238,23429,57238,57238,23429,0,0,0,0,0,0,57238,57238,57238,57238,23429,23429,23429,23429,0,0,0,0,0,0,0,18721,18721,18721,18721,18721,18721,0,0,0,0,0,0,18721,18721,18721,57913,57913,57913,57913,18721,0,0,0,0,0,0,18721,18721,57913,57913,57913,57913,57913,18721,0,0,0,0,0,0,18721,18721,57913,57913,57913,57913,57913,18721,0,0,0,0,0,0,18721,57913,57913,57913,57913,57913,57913,18721,0,0,0,0,0,0,18721,57238,57238,57913,57913,57913,18721,8489,0,0,0,0,0,0,0,57238,57238,57238,57913,8489,8489,8489,57238,18721,0,0,0,0,0,18721,18721,18721,57238,8489,8489,8489,57238,18721,0,0,0,0,61307,61307,61307,30918,8489,8489,8489,8489,57238,18721,0,0,0,0,61307,61307,61307,30918,61242,61242,61242,61242,0,0,0,0,0,0,0,30918,61307,30918,61242,61242,61242,61242,61242,0,0,0,0,0,0,60722,61307,30918,61242,61242,61242,61242,61242,61242,0,0,0,0,0,60722,61307,30918,0,61242,61242,61242,61242,61242,0,0,0,0,60722,60722,60722,60722,0,0,61242,61242,61242,61242,0,0,0,18721,18721,60722,60722,60722,0,0,61242,18721,18721,61242,61242,0,0,18721,18721,18721,18721,0,0,0,0,18721,18721,18721,18721,0,0,18721,18721,18721,0,0,0,0,0,18721,18721,18721,18721,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
const uint16_t valera_14_26_walking[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,57238,57238,57238,57238,57238,0,0,0,0,0,0,14909,23429,23429,57238,57238,65535,1994,57238,0,0,0,0,0,0,14909,23429,57238,57238,57238,57238,57238,57238,0,0,0,0,0,0,14909,23429,23429,57238,23429,57238,57238,23429,0,0,0,0,0,0,57238,57238,57238,57238,23429,23429,23429,23429,0,0,0,0,0,0,0,18721,18721,18721,18721,18721,18721,0,0,0,0,0,0,18721,18721,57913,57913,57913,57913,57913,18721,0,0,0,0,0,0,18721,18721,57913,57913,57913,57913,57913,18721,0,0,0,0,0,0,18721,18721,57913,57913,57913,57913,57913,57913,0,0,0,0,0,0,18721,18721,8489,57913,57913,57913,57913,57913,0,0,0,0,0,0,18721,18721,8489,57913,57913,57238,57238,57238,0,0,0,0,0,0,18721,18721,8489,8489,57913,18721,18721,18721,18721,0,0,0,0,0,18721,18721,8489,8489,57913,18721,57238,18721,18721,0,0,0,0,0,0,0,8489,8489,8489,18721,61307,61307,61307,30918,0,0,0,0,0,0,8489,8489,8489,8489,61307,61307,61307,30918,0,0,0,0,0,0,61242,61242,61242,61242,61242,61242,61307,30918,0,0,0,0,0,0,61242,61242,61242,61242,61242,52530,61307,30918,0,0,0,0,0,61242,61242,61242,61242,61242,52530,52530,61307,30918,0,0,0,0,61242,61242,61242,61242,0,3378,52530,52530,52530,52530,0,0,0,18721,18721,61242,61242,61242,0,0,52530,18721,18721,52530,52530,0,0,18721,18721,18721,18721,0,0,0,0,18721,18721,18721,18721,0,0,18721,18721,18721,18721,0,0,0,0,18721,18721,18721,18721,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
const uint16_t valera_12_24_shooting[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30918,30918,30918,0,0,0,0,0,0,0,0,0,0,0,30918,61307,30918,0,0,0,0,0,0,0,0,0,0,0,30918,61307,30918,0,0,0,0,0,0,0,0,0,0,0,30918,61307,30918,0,0,0,0,0,0,0,0,0,0,0,57238,18721,18721,0,0,0,0,0,0,0,0,0,0,0,18721,18721,18721,0,0,0,0,0,0,0,0,0,0,0,57913,57913,57913,0,0,0,0,0,0,0,0,0,0,0,57913,57913,57913,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,14909,14909,14909,14909,14909,14909,14909,14909,0,0,0,0,0,0,0,18721,18721,18721,18721,18721,18721,0,0,0,0,0,0,0,0,0,18721,18721,18721,18721,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};


const uint16_t zombie_14_24[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16939,16939,16939,16939,16939,16939,0,0,0,0,0,0,0,0,16939,16939,33604,33604,16939,16939,0,0,0,0,0,0,0,0,16939,33604,33604,33604,33604,16939,0,0,0,0,0,0,0,0,16939,0,40224,33604,0,40224,0,0,0,0,0,0,0,0,33604,33604,33604,16939,33604,33604,0,0,0,0,0,0,0,0,33604,33604,33604,33604,33604,33604,0,0,0,0,0,16524,16524,49350,49350,16939,16939,33604,16939,49350,49350,49350,49350,0,0,16524,16524,49350,49350,49350,49350,49350,49350,49350,49350,49350,49350,0,0,49350,49350,49350,49350,49350,16524,49350,49350,49350,16524,49350,16524,0,0,49350,49350,49350,49350,49350,16524,16524,49350,49350,49350,49350,16524,0,0,49350,49350,49350,49350,49350,49350,49350,49350,49350,49350,16524,16524,0,0,33604,33604,33604,49350,49350,16524,49350,49350,49350,16939,33604,33604,0,0,16939,33604,33604,16524,49350,49350,49350,49350,49350,33604,16939,33604,0,0,33604,16939,33604,49350,49350,49350,49350,49350,49350,33604,33604,16939,0,0,0,0,0,1994,1994,1994,1994,16524,16524,0,0,0,0,0,0,0,0,1994,1994,1994,1994,1994,1994,0,0,0,0,0,0,0,0,1994,1994,1169,1994,1994,1994,0,0,0,0,0,0,0,0,1994,1994,1169,1994,1994,1994,0,0,0,0,0,0,0,0,1994,1994,1169,1994,1994,1994,0,0,0,0,0,0,0,0,1994,1994,1994,1994,1994,1994,0,0,0,0,0,0,0,0,27482,61307,27482,61307,61307,27482,0,0,0,0,0,0,0,0,27482,27482,27482,27482,27482,27482,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

const uint16_t bullet_5_6[] = {0,0,0,0,0,0,0,5387,0,0,0,5387,24117,5387,0,0,5387,24117,5387,0,0,5387,5387,5387,0,0,0,0,0,0,};

const uint16_t heart_9_8[] = {0,0,0,0,0,0,0,0,0,0,0,40224,40224,0,40224,40224,0,0,0,40224,40224,40224,40224,40224,40224,40224,0,0,40224,40224,40224,40224,40224,40224,40224,0,0,0,40224,40224,40224,40224,40224,0,0,0,0,0,40224,40224,40224,0,0,0,0,0,0,0,40224,0,0,0,0,0,0,0,0,0,0,0,0,0,};

const sprite valera_14_26_sprite = {14, 26, valera_14_26}, valera_14_26_walking_sprite = {14, 26, valera_14_26_walking}, valera_14_26_shooting_sprite = {14, 26, valera_12_24_shooting};

const sprite bullet_5_6_sprite = {5, 6, bullet_5_6}, heart_9_8_sprite = {9, 8, heart_9_8}, zombie_14_24_sprite = {14, 24, zombie_14_24};

uint16_t size_bullets = 0;
uint16_t size_zombies= 0;
uint16_t hearts = 3;
uint32_t score = 0;

#define SIZE_bullets 20

void createBullet(image* Valera, image** bullets);

void drawBullets(image** bullets);

void removeBullet(image** bullets, int ind);

void moveBullets(image** bullets);


void drawZombies(image** zombies);

void removeZombie(image** zombies, int ind);

void createZombie(image** zombies);

void moveZombies(image** zombies);

void isZombiesInBullet(image** zombies, image** bullets);

int isZombieInBullet(image* zombie, image* bullet);

void drawHearts();
void drawScore();

void restart_game();


int main()
{
	int reload = 0, spawn_zombie_reload = 35; //define varibale for timers for reloading adn spawning zombies
	
	uint16_t current_game_state = 1, current_game_state_changed = 1; // 1 - menu, 2 - running, 3 - finished

	initClock();
	initSysTick();
	setupIO();
	initSerial();


	image* Valera = malloc(sizeof(image)); //building image of Valera
	buildImage(Valera, 20, SCREEN_HEIGHT - 24 - 4, &valera_14_26_sprite, 0, 0);
	//drawLine(0, SCREEN_HEIGHT-24-8, SCREEN_WIDTH, SCREEN_HEIGHT-24-8, RGBToWord(0xff,0,0));
	
	image* bullets[18]; //defining arrays for our objects that will move
	image* zombies[8];

	
	eputs("\nGame status: Menu");
	while(1)
	{	

		if(current_game_state == 1) // starting status
		{
			if (current_game_state_changed)
			{
				fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
				printText("Press fire button", 5, SCREEN_HEIGHT / 2 - 15, RGBToWord(255, 255, 255), RGBToWord(0, 0, 0));
				printText("to start", 5, SCREEN_HEIGHT / 2, RGBToWord(255, 255, 255), RGBToWord(0, 0, 0));
				current_game_state_changed = 0;
			}

			if(firePressed()) //if fire pressed change status
			{
				current_game_state = 2;
				current_game_state_changed = 1;
				eputs("\nGame status: Running");
			}
		}

		else if (current_game_state == 2) // running status
		{

		if(current_game_state_changed) // if game status changed clear the screen and setup screen for running state
		{
			fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
			drawLine(0, SCREEN_HEIGHT - 24 - 8, SCREEN_WIDTH, SCREEN_HEIGHT - 24 - 8, RGBToWord(0xff, 0, 0));
			putImage_struct(Valera);

			current_game_state_changed = 0;	
		}
		
		
		if (rightPressed()) // right pressed
		{						
			if (Valera->x < 110)
			{
				Valera->x += 1;
				Valera->hmoved = 1;
				Valera->hOrientation=0;
			}						
		}
		if (leftPressed()) // left pressed
		{			
			
			if (Valera->x > 10)
			{
				Valera->x -= 1;
				Valera->hmoved = 1;
				Valera->hOrientation=1;
			}			
		}
		if (firePressed() && reload==0) //fire pressed and reload finished
		{			
			createBullet(Valera, bullets);
			reload = 10;
			Valera->sprite = &valera_14_26_shooting_sprite;
		}

		if(spawn_zombie_reload==0)// if it is time to spawn the zombies
		{
			createZombie(zombies);
		}
		

		if (reload == 3) // if there were 3 ticks from when we shot, we change image back to normal
			Valera->sprite = &valera_14_26_sprite;

		if ((Valera->vmoved) || (Valera->hmoved))
		{
			// only redraw if there has been some movement (reduces flicker)			
			
			if(Valera->sprite==&valera_14_26_sprite) Valera->sprite = &valera_14_26_walking_sprite; //change the image for animation
			else if (Valera->sprite != &valera_14_26_shooting_sprite && Valera->sprite == &valera_14_26_walking_sprite) Valera->sprite = &valera_14_26_sprite;
			

			Valera->hmoved = Valera->vmoved = 0;
		}

		putImage_struct(Valera); // draw our character
		moveBullets(bullets); // move the bullets
		drawBullets(bullets); // draw the bullets on the screen

		moveZombies(zombies); // move the zombies
		drawZombies(zombies); // draw the zombies on the screen

		if (hearts == 0) // if we lost the game change game status to lost
		{
			current_game_state = 3;
			current_game_state_changed = 1;
			eputs("\nGame status: Finished");
		}

		isZombiesInBullet(zombies, bullets); //check if bullets hit the zombie, if yes delete the bullet and zombie

		drawHearts(); //draw current hearts
		drawScore(); //draw game score

		drawLine(0, SCREEN_HEIGHT - 24 - 8, SCREEN_WIDTH, SCREEN_HEIGHT - 24 - 8, RGBToWord(0xff, 0, 0));

		if(reload!=0) reload--; //reload counter
		if(spawn_zombie_reload!=0) spawn_zombie_reload--; //zombie spawn counter
		else spawn_zombie_reload = 35;

	}

		else if(current_game_state==3) //if we lost the game
		{
			if(current_game_state_changed) // setup screen for loosing state, print score
			{
				fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
				printText("Press fire button", 5, SCREEN_HEIGHT / 2 - 60, RGBToWord(255, 255, 255), RGBToWord(0, 0, 0));
				printText("to restart", 5, SCREEN_HEIGHT / 2 - 45, RGBToWord(255, 255, 255), RGBToWord(0, 0, 0));

				printText("Your score: ", 5, SCREEN_HEIGHT / 2, RGBToWord(255, 255, 255), RGBToWord(0, 0, 0));
				printNumber(score, 84, SCREEN_HEIGHT / 2, RGBToWord(255, 255, 255), RGBToWord(0, 0, 0));

				score = 0;
				hearts = 3; //reset hearts and score
				current_game_state_changed = 0;
				spawn_zombie_reload = 35;
				restart_game(zombies, bullets); //removes all remaining bullets and zombies
			}

			if (firePressed()) //if fire pressed change game state back to running
			{
				current_game_state = 2;
				current_game_state_changed = 1;
				eputs("\nGame status: Running");
			}
		}
	
		delay(50);
	}


	return 0;
}

void drawScore() //draws score
{
	printNumber(score, 2, 2, RGBToWord(255, 255, 255), RGBToWord(0, 0, 0));
}

void createBullet(image* Valera, image** bullets)
{
	image* bullet = malloc(sizeof(image));
	buildImage(bullet, Valera->x + Valera->sprite->width / 2, Valera->y - 6, &bullet_5_6_sprite, 0, 0);

	bullets[size_bullets++] = bullet; // append bullet to array of bullets
	                                                    
}

void drawBullets(image** bullets) // draws bullets
{
	for(int i = 0; i<size_bullets; i++)
	{
		putImage_struct(bullets[i]);
	}
}

void removeBullet(image** bullets, int ind)
{
	image *temp_bullet = bullets[ind]; // we put the adress of bullet that needs to be removed int the temp pointer

	for(int i = ind; i<size_bullets; i++)
	{
		bullets[i]=bullets[i+1]; //make the bullets slide to the left in array so that we don't have 0 in our array
	}

	fillRectangle(temp_bullet->x,temp_bullet->y,5,6,0); //draw the balck box where the bullet was placed
	free(temp_bullet); //free the memmory that bullet took
	size_bullets--; //decline the size of bullets
}

void moveBullets(image** bullets)
{
	for(int i = 0; i<size_bullets; i++)
	{
		bullets[i]->y-=1;// for each bullet we move it one pixel higher

		if(bullets[i]->y<4) removeBullet(bullets, i); //if bullet reaches the top of the screen delete it
	}

}

void removeZombie(image** zombies, int ind)
{
	image *temp_zombie = zombies[ind];

	for(int i = ind; i<size_zombies; i++)
	{
		zombies[i]=zombies[i+1];
	}
	fillRectangle(temp_zombie->x,temp_zombie->y,14, 24,0);
	free(temp_zombie);
	size_zombies--;
}

void createZombie(image** zombies)
{
	image* zombie = malloc(sizeof(image));
	buildImage(zombie, my_random(1, SCREEN_WIDTH - 14 - 4), 1, &zombie_14_24_sprite, 0, 0);

	zombies[size_zombies++] = zombie;
}

void moveZombies(image** zombies)
{
	for(int i = 0; i<size_zombies; i++)
	{
		zombies[i]->y+=1;
		if(zombies[i]->y>SCREEN_HEIGHT-9-24-24)//if zombie crosses red line
		{
			removeZombie(zombies, i); //we remove it
			hearts--; //and minus one heart
		}
	}
}

void drawZombies(image** zombies)
{
	for(int i = 0; i<size_zombies; i++)
	{
		putImage_struct(zombies[i]);
	}
}


void isZombiesInBullet(image** zombies, image** bullets)
{
	for(int i = 0; i<size_zombies; i++)
	{
		for(int j =0; j<size_bullets; j++) //get through all zombies and bullets
		{
			
			if(isZombieInBullet(zombies[i], bullets[j])) // if zombie in bullet
			{
				removeZombie(zombies, i); //remove both bullet and zombie
				removeBullet(bullets, j);
				score++; //add one to score

				eputs("\nScore: ");// print our score to the terminal
				printDecimal(score);

				break; //break the loop for current zombie, so the program checks next zombie
			}
		}
	}
}

int isZombieInBullet(image* zombie, image* bullet)
{
	if(isInside(zombie->x, zombie->y,14,24,bullet->x,bullet->y) || isInside(zombie->x, zombie->y,14,24,bullet->x+5,bullet->y))
		return 1;
	return 0;
}

void drawHearts()
{	
	int x1 = SCREEN_WIDTH - 27 - 10;
	for(int i =0; i< hearts; i++)
	{	
		putImage(x1, 3, 9, 8, heart_9_8, 0, 0);
		x1+=10;
	}
	x1 = SCREEN_WIDTH - 27 - 10 +(10*hearts);
	for (int i = hearts; i < 3; i++)
	{	
		fillRectangle(x1,3, 9,8,RGBToWord(0,0,0));
		x1 += 10;
	}
}

void restart_game(image **zombies, image **bullets)
{
	for (int i = 0; i < size_zombies; i++)
	{
		free(zombies[i]);
	}
	for (int i = 0; i < size_bullets; i++)
	{
		free(bullets[i]);
	}
	size_bullets = 0;
	size_zombies = 0;
}

void initSysTick(void)
{
	SysTick->LOAD = 48000;
	SysTick->CTRL = 7;
	SysTick->VAL = 10;
	__asm(" cpsie i "); // enable interrupts
}
void SysTick_Handler(void)
{
	milliseconds++;
}
void initClock(void)
{
// This is potentially a dangerous function as it could
// result in a system with an invalid clock signal - result: a stuck system
        // Set the PLL up
        // First ensure PLL is disabled
        RCC->CR &= ~(1u<<24);
        while( (RCC->CR & (1 <<25))); // wait for PLL ready to be cleared
        
// Warning here: if system clock is greater than 24MHz then wait-state(s) need to be
// inserted into Flash memory interface
				
        FLASH->ACR |= (1 << 0);
        FLASH->ACR &=~((1u << 2) | (1u<<1));
        // Turn on FLASH prefetch buffer
        FLASH->ACR |= (1 << 4);
        // set PLL multiplier to 12 (yielding 48MHz)
        RCC->CFGR &= ~((1u<<21) | (1u<<20) | (1u<<19) | (1u<<18));
        RCC->CFGR |= ((1<<21) | (1<<19) ); 

        // Need to limit ADC clock to below 14MHz so will change ADC prescaler to 4
        RCC->CFGR |= (1<<14);

        // and turn the PLL back on again
        RCC->CR |= (1<<24);        
        // set PLL as system clock source 
        RCC->CFGR |= (1<<1);
}
void delay(volatile uint32_t dly)
{
	uint32_t end_time = dly + milliseconds;
	while(milliseconds != end_time)
		__asm(" wfi "); // sleep
}

void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber)
{
	Port->PUPDR = Port->PUPDR &~(3u << BitNumber*2); // clear pull-up resistor bits
	Port->PUPDR = Port->PUPDR | (1u << BitNumber*2); // set pull-up bit
}
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode)
{
	/*
	*/
	uint32_t mode_value = Port->MODER;
	Mode = Mode << (2 * BitNumber);
	mode_value = mode_value & ~(3u << (BitNumber * 2));
	mode_value = mode_value | Mode;
	Port->MODER = mode_value;
}
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py)
{
	// checks to see if point px,py is within the rectange defined by x,y,w,h
	uint16_t x2,y2;
	x2 = x1+w;
	y2 = y1+h;
	int rvalue = 0;
	if ( (px >= x1) && (px <= x2))
	{
		// ok, x constraint met
		if ( (py >= y1) && (py <= y2))
			rvalue = 1;
	}
	return rvalue;
}

void setupIO()
{
	RCC->AHBENR |= (1 << 18) + (1 << 17) + (1<<22); // enable Ports A and B and F
	display_begin();
	pinMode(GPIOB,4,0);
	pinMode(GPIOB,5,0);
	pinMode(GPIOA,8,0);
	pinMode(GPIOA,11,0);
	pinMode(GPIOF,1,0);
	enablePullUp(GPIOB,4);
	enablePullUp(GPIOB,5);
	enablePullUp(GPIOA,11);
	enablePullUp(GPIOA,8);
	enablePullUp(GPIOF,1);
}



