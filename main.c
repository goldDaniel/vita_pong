#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>


#define SCREEN_WIDTH 940
#define SCREEN_HEIGHT 544

#define PADDLE_SPEED 6;

typedef struct RECTANGLE Rect;
struct RECTANGLE
{
    float x; 
    float y;

    float width;
    float height;
};

typedef struct VECTOR2 Vector2;
struct VECTOR2
{
    float x;
    float y;
};

Rect playerRect;
Rect aiRect;

Rect ball;
Vector2 ballSpeed;

void clampToScreen(Rect* rect);
int areRectanglesColliding(Rect rect1, Rect rect2);
float lerp(float a, float b, float t);

int main()
{
	SceCtrlData pad;
	vita2d_pvf *pvf;
	float rad = 0.0f;

	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	pvf = vita2d_load_default_pvf();

	memset(&pad, 0, sizeof(pad));

    playerRect.width = aiRect.width = 32;
    playerRect.height = aiRect.height = 128;
    playerRect.x = 16;
    playerRect.y = SCREEN_HEIGHT - playerRect.height / 2;

    aiRect.x = SCREEN_WIDTH - 16;
    aiRect.y = SCREEN_HEIGHT - aiRect.width / 2;

    ball.width = ball.height = 16;
    ball.x = SCREEN_WIDTH / 2 - ball.width / 2;
    ball.y = SCREEN_HEIGHT / 2 + ball.height / 2;

    ballSpeed.x = 4;
    ballSpeed.y = 4;

    int paused = 0;

    int playerScore = 0;
    int aiScore = 0;

    unsigned int ballColor = 0;

    //GAMELOOP
	while (1) 
    {
		sceCtrlPeekBufferPositive(0, &pad, 1);

        if(pad.buttons & SCE_CTRL_START)
		{
            paused = 1;
        }	
        if(pad.buttons & SCE_CTRL_CROSS)
        {
            paused = 0;
        }
        

		vita2d_start_drawing();
		vita2d_clear_screen();
        if(paused) 
        {
            vita2d_pvf_draw_text(pvf, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, RGBA8(0,255,0,255), 2.0f, "paused");    
            vita2d_pvf_draw_text(pvf, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 104, RGBA8(0,255,0,255), 2.0f, "PRESS X TO UNPAUSE");    
        }
		else
        {
            aiRect.y = lerp(aiRect.y, ball.y - aiRect.width / 2, 0.15f);

            if(pad.buttons & SCE_CTRL_UP)
            {
                playerRect.y -= PADDLE_SPEED;
            }
            if(pad.buttons & SCE_CTRL_DOWN)
            {
                playerRect.y += PADDLE_SPEED;
            }

            ball.x += ballSpeed.x;
            ball.y += ballSpeed.y;

            if(ball.x < 0) ballSpeed.x = -ballSpeed.x;
            else if(ball.x + ball.width > SCREEN_WIDTH) ballSpeed.x = -ballSpeed.x;
            
            if(ball.y <= 0) ballSpeed.y = -ballSpeed.y;
            else if(ball.y + ball.height > SCREEN_HEIGHT) ballSpeed.y = -ballSpeed.y;

            clampToScreen(&playerRect);
            clampToScreen(&aiRect);
            
            if(ball.x < 0)
            {
                aiScore++;

                ball.width = ball.height = 16;
                ball.x = SCREEN_WIDTH / 2 - ball.width / 2;
                ball.y = SCREEN_HEIGHT / 2 + ball.height / 2;

                ballSpeed.x = 4;
                ballSpeed.y = 4;
            }
            else if(ball.x + ball.width > SCREEN_WIDTH)
            {
                playerScore++;
                ball.width = ball.height = 16;
                ball.x = SCREEN_WIDTH / 2 - ball.width / 2;
                ball.y = SCREEN_HEIGHT / 2 + ball.height / 2;

                ballSpeed.x = 4;
                ballSpeed.y = 4;
            }
        }
		

        for(int y = -47 - 23; y < SCREEN_HEIGHT; y += 94)
        {
            Rect r;
            r.width = 8;
            r.height = 47;
            r.x = SCREEN_WIDTH / 2 - r.width / 2;
            r.y = y;

            vita2d_draw_rectangle(r.x, r.y, r.width, r.height, RGBA8(255, 255, 255, 255));
        }



        vita2d_draw_rectangle(playerRect.x, playerRect.y, playerRect.width, playerRect.height, RGBA8(255, 0, 0, 255));
        vita2d_draw_rectangle(aiRect.x, aiRect.y, aiRect.width, aiRect.height, RGBA8(255, 0, 0, 255));

        if(areRectanglesColliding(playerRect, ball) || areRectanglesColliding(ball, playerRect) ||
           areRectanglesColliding(aiRect, ball) || areRectanglesColliding(ball, aiRect))
        {
            int ran = rand();
            
            if(ballSpeed.x < 0)
            {
                ballSpeed.x -= 0.2f;
            }
            else
            {
                ballSpeed.x += 0.2f;
            }

            if(ballSpeed.y < 0)
            {
                ballSpeed.y -= 0.1f;
            }
            else
            {
                ballSpeed.y += 0.1f;   
            }

            if(ran % 2 == 0)
            {
                ballSpeed.y = -ballSpeed.y;   
            }
            ballSpeed.x = -ballSpeed.x;
        }
        else
        {
            ballColor = RGBA8(255, 255, 255, 255);
        }

        vita2d_draw_rectangle(ball.x, ball.y, ball.width, ball.height, ballColor);

        vita2d_pvf_draw_textf(pvf, SCREEN_WIDTH / 2 - 200, 96, RGBA8(255,255,255,255), 5.0f, "%d", playerScore);    
        vita2d_pvf_draw_textf(pvf, SCREEN_WIDTH / 2 + 200, 96, RGBA8(255,255,255,255), 5.0f, "%d", aiScore);    

		vita2d_end_drawing();
		vita2d_swap_buffers();

		rad += 0.1f;
	}

	/*
	 * vita2d_fini() waits until the GPU has finished rendering,
	 * then we can free the assets freely.
	 */
	vita2d_fini();
	vita2d_free_pvf(pvf);

	sceKernelExitProcess(0);
	return 0;
}

void clampToScreen(Rect* rect)
{
    if(rect->x < 0) rect->x = 0;
    if(rect->y < 0) rect->y = 0;
    if(rect->x  + rect->width > SCREEN_WIDTH) rect->x = SCREEN_WIDTH - rect->width;
    if(rect->y + rect-> height > SCREEN_HEIGHT) rect->y = SCREEN_HEIGHT - rect->height;
}

int areRectanglesColliding(Rect rect1, Rect rect2)
{
    int horizontalColliding = 0;
    int verticalColliding =   0;
    if(rect1.x > rect2.x && rect1.x < rect2.x + rect2.width)
    {
        horizontalColliding = 1;
    }
    else if(rect1.x + rect1.width > rect2.x && rect1.x + rect1.width < rect2.x + rect2.width)
    {
        horizontalColliding = 1;
    }

    if(rect1.y > rect2.y && rect1.y < rect2.y + rect2.width)
    {
        verticalColliding = 1;
    }
    else if(rect1.y + rect1.width > rect2.y && rect1.y + rect1.height < rect2.y + rect2.height)
    {
        verticalColliding = 1;
    }
    

    return (horizontalColliding && verticalColliding);
}

float lerp(float a, float b, float t)
{
    return (a + t*(b - a));
}