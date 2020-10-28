//============================================================================
// Name        : game.cpp
// Author      : alex
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include<fstream>
#include<string>
#include<sstream>
#include "GPIO.h"
#include "bitmaps.h"
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/timeb.h>
#include <chrono>

#include"oled_driver.h"
#include "adc.h"

using namespace std;
using namespace exploringBB;

#define GHOSTS 4

#define INVISIBLE 0
#define VISIBLE 1

/* Timer */
uint64_t millis()
{
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::
                  now().time_since_epoch()).count();
    return ms;
}


///* ANALOG */
//#define LDR_PATH "/sys/bus/iio/devices/iio:device0/in_voltage"
//int readAnalog(int number){
//// returns the input as an int
//stringstream ss;
//ss << LDR_PATH << number << "_raw";
//fstream fs;
//fs.open(ss.str().c_str(), fstream::in);
//fs >> number;
//fs.close();
//return number;
//}

void system_init()
{
    /* Initialize I2C bus and connect to the I2C Device */
    if(init_i2c_dev(I2C_DEV2_PATH, SSD1306_OLED_ADDR) == 0)
    {
        printf("(Main)i2c-2: Bus Connected to SSD1306\r\n");
    }
    else
    {
        printf("(Main)i2c-2: OOPS! Something Went Wrong\r\n");
        exit(1);
    }

    /* Run SDD1306 Initialization Sequence */
    display_Init_seq();

    /* Clear display */
    clearDisplay();
}


class Object
{
  private:
	int x, y; /* position on the screen */
	int w,h; //  obj size
	int dx, dy; /* movement vector */
	bool move;

 public:
	void set_x(int x_val)
    {
		x = x_val;
    }

	int get_x()
	{
		return x;
	}

	void set_y(int y_val)
    {
		y = y_val;
    }

	int get_y()
	{
		return y;
	}

	void set_h(int h_val)
    {
		h = h_val;
    }

	int get_h()
	{
		return h;
	}

	void set_w(int w_val)
    {
		w = w_val;
    }

	int get_w()
	{
		return w;
	}

	void set_dx(int dx_val)
    {
		dx = dx_val;
    }

	int get_dx()
	{
		return dx;
	}

	void set_dy(int dy_val)
    {
		dy = dy_val;
    }

	int get_dy()
	{
		return dy;
	}

	void set_move(int move_val)
    {
		move = move_val;
    }

	int get_move()
	{
		return move;
	}

} boy, ghost[GHOSTS], coin;

//inisilise starting position and sizes of game elemements
static void init_game()
{
	for(int i = 0; i < GHOSTS; i++)
	{
		ghost[i].set_x(rand() % 100);
		ghost[i].set_y (rand() % 40);
		ghost[i].set_w (16);
		ghost[i].set_h (12);
		ghost[i].set_dy (2);
		ghost[i].set_dx (2);
		ghost[i].set_move (true);
	}

	boy.set_x(5);
	boy.set_y (5);
	boy.set_w (16);
	boy.set_h (14);
	boy.set_dy (3);
	boy.set_dx (3);
	boy.set_move (true);

	coin.set_x(rand() % 120);
	coin.set_y (rand() % 52);
	coin.set_w (8);
	coin.set_h (8);
	coin.set_dy (0);
	coin.set_dx (0);
	coin.set_move (true);
}

//if return value is 1 collision occured. if return is 0, no collision.
int check_collision( Object  a,  Object  b)
{
	int left_a, left_b;
	int right_a, right_b;
	int top_a, top_b;
	int bottom_a, bottom_b;

	left_a = a.get_x();
	right_a = a.get_x() + a.get_w();
	top_a = a.get_y();
	bottom_a = a.get_y() + a.get_h();

	left_b = b.get_x();
	right_b = b.get_x() + b.get_w();
	top_b = b.get_y();
	bottom_b = b.get_y() + b.get_h();

	if (left_a > right_b) {
		return 0;
	}

	if (right_a < left_b) {
		return 0;
	}

	if (top_a > bottom_b) {
		return 0;
	}

	if (bottom_a < top_b) {
		return 0;
	}
	return 1;
}

static void move_ghost()
{
	for(int i=0; i <GHOSTS; i++)
	{
		/* Move the ghost by its motion vector. */
		ghost[i].set_x(ghost[i].get_x() + ghost[i].get_dx());
		ghost[i].set_y(ghost[i].get_y() + ghost[i].get_dy());

		/* Turn the ghost around if it hits the edge of the screen. */
		if (ghost[i].get_x() < 0 || ghost[i].get_x() > (SCREEN_WIDTH - 16))
		{
			ghost[i].set_dx(-ghost[i].get_dx());
		}

		if (ghost[i].get_y() < 0 || ghost[i].get_y() > SCREEN_HEIGHT - 16)
		{
			ghost[i].set_dy(-ghost[i].get_dy());
		}

	}
}

void game_start_screen()
{
	clearDisplay();
	setTextSize(2);
	setTextColor(WHITE);
    setCursor(5,7);
    print_str("GOLD&GHOST");
	setTextSize(1);
	setCursor(40,45);
	print_str("press A");
	Display();
}

void game_end_screen( int score)
{
	clearDisplay();
	setTextSize(2);
	setTextColor(WHITE);
    setCursor(10,5);
    print_str("GAME OVER");
	setTextSize(1);
	setCursor(25,30);
	print_str("score: ");
	setCursor(80,30);
	printNumber(score, DEC);
	setCursor(11,50);
	print_str("press A to restart");
	Display();
}

void show_scre_life(int life, int score)
{
	drawRect(0,0,life,3,1);
	setTextSize(1);
	setCursor(110,0);
	printNumber(score, DEC);
	Display();
}


void game_startup(bool *start_game, int button_val, int *life, int *score)
{
	while(!start_game)
	{
		if(button_val)
	 	{
			*start_game = true;
	 		*life = 64;
	 		*score = 0;
	 		init_game();
	 		clearDisplay();
	 	}
	 }
}
void game_value_reset(bool *start_game, int *life, int *score)
{
	*start_game = true;
	*life = 64;
	*score = 0;
	clearDisplay();
}


	class Triangle
	{
	private:
		int x0, y0;
		int x1, y1;
		int x2, y2;

	public:

		Triangle(int a0, int b0, int a1, int b1, int a2, int b2)
		{
			x0 = a0;
			y0 = b0;

			x1 = a1;
			y1 = b1;

			x2 = a2;
			y2 = b2;
		}

		void set_x0(int x)
			{ x0 = x; }

		int get_x0()
			{return  x0;}

		void set_y0(int y)
			{ y0 = y; }

		int get_y0()
			{return  y0;}

		void set_x1(int x)
			{ x1 = x;}

		int get_x1()

			{return  x1;}

		void set_y1(int y)
			{ y1 = y; }

		int get_y1()
			{return  y1;}

		void set_x2(int x)
			{ x2 = x; }

		int get_x2()
			{return  x2;}

		void set_y2(int y)
			{ y2 = y;}

		int get_y2()
			{return  y2;}

		virtual void Rotate(int x0,int y0,int x1,int y1,int x2,int y2, float Angle);
	};

	void Triangle :: Rotate(int x0,int y0,int x1,int y1,int x2,int y2, float Angle)
	{
	    int x,y,a0,b0,a1,b1,a2,b2,p=x0,q=y0;

	    drawTriangle(x0,  y0, x1, y1,  x2 , y2 , BLACK);
	    Angle=(Angle*3.14)/180;
	    a0=p+(x0-p)*cos(Angle)-(y0-q)*sin(Angle);
	    b0=q+(x0-p)*sin(Angle)+(y0-q)*cos(Angle);
	    a1=p+(x1-p)*cos(Angle)-(y1-q)*sin(Angle);
	    b1=q+(x1-p)*sin(Angle)+(y1-q)*cos(Angle);
	    a2=p+(x2-p)*cos(Angle)-(y2-q)*sin(Angle);
	    b2=q+(x2-p)*sin(Angle)+(y2-q)*cos(Angle);
	    printf("Rotate");
	    drawTriangle(a0,  b0, a1, b1,  a2 , b2, WHITE);
	}

int main()
{
	bool debug = false;
	cout << "BOY vs Ghost" << endl; // prints !!!Hello World!!!
	system_init();

	int ghosts = sizeof(ghost)/sizeof(ghost[0]);
	GPIO button_a(115);
	int x = 0;
	int y = 0;
	int a = 0;
	int life = 64;
	int score = 0;
	int score_prescaler = 0;
	bool start_game = false;

	uint64_t t_ms;
	bool boy_damage = false;

	int ang = 10;

	Triangle T1(2, 2, 2, 10, 20, 10);


//	for(int i= 0; i < 50; i ++)
	while(1)
	{
		x = readAnalog(0);
		y = readAnalog(2);


//		if(x > 2000)
//			x1--;
//			x0--;
//
//		if(x < 1300)
//			x1++;
//			x0--;


//		clearDisplay();
//		drawTriangle(x0,  y0, x1, y1,  x2 , y2 , WHITE);
		T1.Rotate(2, 2, 2, 10, 20, 10, ang);

		Display();
		ang ++;



		usleep(1000);

	}


//	t_ms = millis();
//	x = readAnalog(0);
//	y = readAnalog(2);
//	a = button_a.getValue();

//	drawBitmap(boy.get_x(), boy.get_y(),  r_leg_up, boy.get_w(), boy.get_h(), VISIBLE);


	return 0;
}

