
//Breakout    
//main.cpp
//tell compiler to not include many unneeded header files.
#define WIN32_LEAN_AND_MEAN
//need this for windows stuff.
#include <windows.h>
#include <stdlib.h>
//now let's include our bitmapobject definitions
#include "bitmapobject.h"


//let's give our window a name
#define WINDOWCLASS "Breakout"
//let's give our window a title...er caption.
#define WINDOWTITLE "Breakout"

//since we're using square tiles, let's only use a single size.
const int TILESIZE=8;
//now for the map...
const int MAPWIDTH=70;
const int MAPHEIGHT=50;
const int TILENODRAW=-1;
const int TILEBLACK=0;
const int TILEGREY=1;
const int TILEBLUE=2;
const int TILERED=3;	
const int TILEGREEN=4;
const int TILEYELLOW=5;
const int TILEWHITE=6;
const int TILESTEEL=7;
const int TILEPURPLE=8;
const int TILEBALL=9;

//Rectangular blocks require two sizes
const int BLOCKWIDTH = 40;
const int BLOCKHEIGHT = 24;

struct Ball 
{
	int x;
	int y;
	int xvelocity;
	int yvelocity;
};

Ball ball;         

struct Block
{
    int x;
    int y;
    Block *next;
};

Block *head = new Block;
Block *current = head;

bool GameInit(); // game initialization function
void GameLoop(); //where the game actually takes place
void GameDone(); //clean up! 
void DrawTile(int x, int y, int tile); //coordinates & tile type
void DrawMap(); //draw the whole map.. render function, basically
void MovePaddle(int x); //coordinates to move.
void MoveBall();
void CollisionTest(); //test collision of blocks
void NewGame(); //make a new game!
//void DestroyBlock(Block *current);//Destroy a Block


HINSTANCE hInstMain=NULL; //main app handle
HWND hWndMain=NULL; //main window handle


int Map[MAPWIDTH][MAPHEIGHT+1]; //the game map!

int paddle = MAPWIDTH/2;
DWORD start_time;  //used in timing
bool GAMESTARTED=false; //used by NewBlock()

//map for the program
BitMapObject bmoMap;
//Tile images
BitMapObject bmoTiles;
//Block images
BitMapObject bmoBlocks;

int score = 0;

LRESULT CALLBACK TheWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	//which message did we get?
	switch(uMsg)
	{
	case WM_KEYDOWN:
		{
			//check for escape key
			if(wParam==VK_ESCAPE)
			{
				DestroyWindow(hWndMain);
				return(0);//handled message
			}
			else if(wParam==VK_LEFT) //check for left arrow key
			{
				MovePaddle(-1);
				return(0);//handled message
			}
			else if(wParam==VK_RIGHT) //check for right arrow key
			{
				MovePaddle(1);
				return(0);//handled message
			}
		}break;
	case WM_DESTROY://the window is being destroyed
		{

			//tell the application we are quitting
			PostQuitMessage(0);

			//handled message, so return 0
			return(0);

		}break;
	case WM_PAINT://the window needs repainting
		{
			//a variable needed for painting information
			PAINTSTRUCT ps;
			
			//start painting
			HDC hdc=BeginPaint(hwnd,&ps);

			//redraw the map
			BitBlt(hdc,0,0,TILESIZE*MAPWIDTH+TILESIZE,TILESIZE*MAPHEIGHT,bmoMap,0,0,SRCCOPY);

			//end painting
			EndPaint(hwnd,&ps);
					
			//handled message, so return 0
			return(0);
		}break;
	}

	//pass along any other message to default message handler
	return(DefWindowProc(hwnd,uMsg,wParam,lParam));
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	//assign instance to global variable
	hInstMain=hInstance;

	//create window class
	WNDCLASSEX wcx;

	//set the size of the structure
	wcx.cbSize=sizeof(WNDCLASSEX);

	//class style
	wcx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	//window procedure
	wcx.lpfnWndProc=TheWindowProc;

	//class extra
	wcx.cbClsExtra=0;

	//window extra
	wcx.cbWndExtra=0;

	//application handle
	wcx.hInstance=hInstMain;

	//icon
	wcx.hIcon=LoadIcon(NULL,IDI_APPLICATION);

	//cursor
	wcx.hCursor=LoadCursor(NULL,IDC_ARROW);

	//background color
	wcx.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);

	//menu
	wcx.lpszMenuName=NULL;

	//class name
	wcx.lpszClassName=WINDOWCLASS;

	//small icon
	wcx.hIconSm=NULL;

	//register the window class, return 0 if not successful
	if(!RegisterClassEx(&wcx)) return(0);

	//create main window
	hWndMain=CreateWindowEx(0,WINDOWCLASS,WINDOWTITLE, WS_BORDER | WS_SYSMENU | WS_CAPTION| WS_VISIBLE,0,0,320,240,NULL,NULL,hInstMain,NULL);

	//error check
	if(!hWndMain) return(0);

	//if program initialization failed, then return with 0
	if(!GameInit()) return(0);

	//message structure
	MSG msg;

	//message pump
	for( ; ; )	
	{
		//look for a message
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			//there is a message

			//check that we arent quitting
			if(msg.message==WM_QUIT) break;
			
			//translate message
			TranslateMessage(&msg);

			//dispatch message
			DispatchMessage(&msg);
		}

		//run main game loop
		GameLoop();
		
	}
	
	//clean up program data
	GameDone();

	//return the wparam from the WM_QUIT message
	return(msg.wParam);
}


bool GameInit()
{
	//set the client area size
	RECT rcTemp;
	SetRect(&rcTemp,0,0,MAPWIDTH*TILESIZE,MAPHEIGHT*TILESIZE);//560x400 client area
	AdjustWindowRect(&rcTemp,WS_BORDER | WS_SYSMENU | WS_CAPTION| WS_VISIBLE,FALSE);//adjust the window size based on desired client area
	SetWindowPos(hWndMain,NULL,0,0,rcTemp.right-rcTemp.left,rcTemp.bottom-rcTemp.top,SWP_NOMOVE);//set the window width and height

	//create map image
	HDC hdc=GetDC(hWndMain);
	bmoMap.Create(hdc,MAPWIDTH*TILESIZE,MAPHEIGHT*TILESIZE);
	FillRect(bmoMap,&rcTemp,(HBRUSH)GetStockObject(BLACK_BRUSH));
	ReleaseDC(hWndMain,hdc);

	bmoTiles.Load(NULL,"tiles.bmp");
	bmoBlocks.Load(NULL,"blocks.bmp");
	NewGame();

	return(true);//return success
}

void GameDone()
{
	//clean up code goes here
}


void GameLoop()
{
	if( (GetTickCount() - start_time) > 43)
	{
		printf("tick");
		MoveBall();
		start_time=GetTickCount();
	}

}

void NewGame()
{
	start_time=GetTickCount();
	GAMESTARTED=false;

	//srand(GetTickCount());
	
    ball.x = MAPWIDTH/4;
    ball.y = MAPHEIGHT/3;
	ball.xvelocity = 1;
	ball.yvelocity = 1;
	
	//start out the map
	for(int x=0;x< MAPWIDTH;x++)
	{
		for(int y=0;y< MAPHEIGHT+1;y++)
		{
			if(y==MAPHEIGHT) //makes Y-collision easier.
				Map[x][y]=TILEGREY;
    			else
				Map[x][y]=TILEBLACK;
		}
	}
	DrawMap();
}

void DrawTile(int x,int y,int tile)//put a tile
{
	//mask first
	BitBlt(bmoMap,x*TILESIZE,y*TILESIZE,TILESIZE,TILESIZE,bmoTiles,tile*TILESIZE,0,SRCAND);
	//then image
	BitBlt(bmoMap,x*TILESIZE,y*TILESIZE,TILESIZE,TILESIZE,bmoTiles,tile*TILESIZE,0,SRCPAINT);
}


void DrawBlock(int x,int y,int tile) //Put a block
{
    //make first
    BitBlt(bmoMap,x*BLOCKWIDTH,y*BLOCKHEIGHT,BLOCKWIDTH,BLOCKHEIGHT,bmoBlocks,tile*BLOCKWIDTH,0 ,SRCAND);
    //then  image
    BitBlt(bmoMap,x*BLOCKWIDTH,y*BLOCKHEIGHT,BLOCKWIDTH,BLOCKHEIGHT,bmoBlocks,tile*BLOCKWIDTH,0,SRCPAINT);
}

        
void DrawMap()//draw screen
{
	int xmy, ymx;

	//draw the map
	//loop through the positions
	for(xmy=0;xmy< MAPWIDTH;xmy++)
			for(ymx=0;ymx< MAPHEIGHT;ymx++)
				DrawTile(xmy,ymx,TILEGREY);

    DrawTile(ball.x, ball.y, TILEBLACK); // Draw ball
    
    DrawTile(paddle-2, MAPHEIGHT-1, TILEWHITE);    
    DrawTile(paddle-1, MAPHEIGHT-1, TILEWHITE);
    DrawTile(paddle, MAPHEIGHT - 1, TILEWHITE);
    DrawTile(paddle+1, MAPHEIGHT-1, TILEWHITE);			
    DrawTile(paddle+2, MAPHEIGHT-1, TILEWHITE);

    xmy = 0; ymx = 0;
    while(xmy<(MAPWIDTH*TILESIZE)/BLOCKWIDTH)
    {
      current->x = xmy;
      current->y = 0;
      
      DrawBlock(current->x,current->y,0);
      
      current->next = new Block;
      current = current->next;
      xmy++;
    }
    
        xmy = 0; ymx = 0;
    while(xmy<(MAPWIDTH*TILESIZE)/BLOCKWIDTH)
    {
      current->x = xmy;
      current->y = 2;
      
      DrawBlock(current->x,current->y,0);
      
      current->next = new Block;
      current = current->next;
      xmy++;
    }
 	InvalidateRect(hWndMain,NULL,FALSE);
}


void DrawBall(int x, int y)
{
    //fill in old ball space
    DrawTile(x, y, TILEGREY);

	//draw moving ball
    DrawTile(ball.x, ball.y, TILEBALL);
    
    InvalidateRect(hWndMain,NULL,FALSE);
}


void DrawPaddle(int prepaddle)
{	
    //redraw the empty space
    DrawTile(prepaddle-2, MAPHEIGHT-1, TILEGREY);    
    DrawTile(prepaddle-1, MAPHEIGHT-1, TILEGREY);
    DrawTile(prepaddle, MAPHEIGHT - 1, TILEGREY);
    DrawTile(prepaddle+1, MAPHEIGHT-1, TILEGREY);			
    DrawTile(prepaddle+2, MAPHEIGHT-1, TILEGREY);
    //draw paddle
    DrawTile(paddle-2, MAPHEIGHT-1, TILEWHITE);    
    DrawTile(paddle-1, MAPHEIGHT-1, TILEWHITE);
    DrawTile(paddle, MAPHEIGHT - 1, TILEWHITE);
    DrawTile(paddle+1, MAPHEIGHT-1, TILEWHITE);			
    DrawTile(paddle+2, MAPHEIGHT-1, TILEWHITE);
    
    InvalidateRect(hWndMain,NULL,FALSE);
}



void MovePaddle(int x)
{
   if((paddle-2)+x >= 0 && (paddle+2)+x < MAPWIDTH)
   {
    int prepaddle = paddle;
    paddle+=x;
    
	DrawPaddle(prepaddle);
   }
}


void MoveBall()
{
 int x = ball.x;
 int y = ball.y;

  CollisionTest();
  ball.x += ball.xvelocity;
  ball.y += ball.yvelocity;
  
  DrawBall(x, y);
}


void CollisionTest()
{
	int newx = ball.x + ball.xvelocity;
	int newy = ball.y + ball.yvelocity;
	int counter;

	      if(newx < 0 || newx > MAPWIDTH)
		    ball.xvelocity *= -1;
        
          if(newy < 0)
            ball.yvelocity *= -1;
          
          if(newy == MAPHEIGHT -1 && (newx==paddle||newx==paddle-2||newx==paddle-1||newx==paddle+1||newx==paddle+2))
            ball.yvelocity *= -1;
            
            current = head;
         if(newy< MAPHEIGHT)
         while(current!=NULL)
         {
            if((newx*TILESIZE >= current->x*BLOCKWIDTH && newx*TILESIZE <= (current->x*BLOCKWIDTH)+BLOCKWIDTH)
            && (newy*TILESIZE>=current->y*BLOCKHEIGHT && newy*TILESIZE<=(current->y*BLOCKHEIGHT)+BLOCKHEIGHT))
            {
             if(newx*TILESIZE==current->x*BLOCKWIDTH || newx*TILESIZE==(current->x*BLOCKWIDTH)+BLOCKWIDTH
             || ball.x*TILESIZE==(current->x*BLOCKWIDTH)+TILESIZE && ball.y*TILESIZE==(current->y*BLOCKHEIGHT)+TILESIZE
             && ((newy*TILESIZE)+TILESIZE!=current->y*BLOCKHEIGHT || (newy*TILESIZE)-TILESIZE!=(current->y*BLOCKHEIGHT)+BLOCKHEIGHT
             && ball.x*TILESIZE!=(current->x*BLOCKWIDTH)+TILESIZE))//ADD HERE
             {
          /*    Block *temp = head;
              while(temp!=NULL)
              { 
               if((newy*TILESIZE)+TILESIZE==temp->y*BLOCKHEIGHT || (newy*TILESIZE)-TILESIZE==(temp->y*BLOCKHEIGHT)+BLOCKHEIGHT)
               {
                 DrawBlock(temp->x, temp->y, TILEGREY);
                 Block *next = temp->next;
                 delete temp;
                 temp = next; 
                 ball.yvelocity *= -1;
               }
               else
                temp = temp->next;
              }*/                
              DrawBlock(current->x, current->y, TILEGREY);
              Block *next = current->next;
              delete current;
              current = next; 
              ball.xvelocity *= -1;
             }
             else
             {
              DrawBlock(current->x, current->y, TILEGREY);
              Block *next = current->next;
              delete current;
              current = next; 
              ball.yvelocity *= -1; 
             } 
            }
            else
             current = current->next; 
         }   
            
          if(newy >= MAPHEIGHT)
            GameDone();
}

/*
void DestroyBlock(Block *current)
{
  DrawBlock(current->x, current->y, TILEGREY);
  Block *temp = current->next;
  delete current;
  current = temp;
}
  */
