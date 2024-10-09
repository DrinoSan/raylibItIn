// I have no idea what i am doing
// Some code does nothing because i took a existing project and deleted stuff
#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------------
#define FIRST_LEVEL  50
#define SECOND_LEVEL 20

#define WIN_PERCENTAGE 80

enum DIRECTION
{
   DIRECTION_UP,
   DIRECTION_RIGHT,
   DIRECTION_DOWN,
   DIRECTION_LEFT,
   DIRECTION_NONE
};

//--------------------------------------------------------------------------------------
typedef struct Player
{
    Rectangle rec;
    Vector2   speed;
    Color     color;
    Vector2   startLinePos;
    Vector2   endLinePos;
    bool      isDrawing;
    enum DIRECTION direction;
} Player;

//--------------------------------------------------------------------------------------
/// Holds the area to be cut
typedef struct Board
{
   Rectangle rec;
   float
       cutOffPercent;   // Initially it is 0 because we did not cut an anything
   Color color;
   float initialArea;
   float cutOffArea;
} Board;

//--------------------------------------------------------------------------------------
static const int screenWidth  = 800;
static const int screenHeight = 600;

static bool gameOver = false;
static bool pause    = false;
static int  score    = 0;
static bool victory  = false;
static bool  alphaDecrease = false;
static float alpha     = 0.0f;

static Player player = { 0 };
static Sound  firstSound;
static Sound  secondSound;
static Sound  endingSound;
static bool   soundFlag;

static Board board = { 0 };

struct Circle
{
   int   centerX;
   int   centerY;
   float radius;
   Color color;
} Circle;

static struct Circle c1;
static struct Circle c2;
static struct Circle c3;
static struct Circle c4;
static struct Circle c5;

////------------------------------------------------------------------------------------
//// Module Functions Declaration (local)
////------------------------------------------------------------------------------------
static void InitGame( void );          // Initialize game
static void UpdateGame( void );        // Update game (one frame)
static void DrawGame( void );          // Draw game (one frame)
static void UnloadGame( void );        // Unload game
static void UpdateDrawFrame( void );   // Update and Draw (one frame)
static int  isRecBigger( const Rectangle* rec1, const Rectangle* rec2 );

int main(void)
{
   // Initialization
   //--------------------------------------------------------------------------------------

   InitWindow( screenWidth, screenHeight, "Shoot me baby" );
   InitAudioDevice();
   SetMasterVolume( 100.0 );

   InitGame();

   SetTargetFPS( 60 );   // Set our game to run at 60

   // Main game loop
   while ( !WindowShouldClose() )   // Detect window close button or ESC key
   {
      UpdateDrawFrame();
   }

    UnloadGame();

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();   // Close window and OpenGL context
    CloseAudioDevice();
    //--------------------------------------------------------------------------------------

    return 0;
}

void InitGame( void )
{
    // Initialize game variables
    pause    = false;
    gameOver = false;
    victory  = false;
    score    = 0;
    alpha    = 0;

    // Initialize player
    player.rec.x      = 20;
    player.rec.y      = 50;
    player.rec.width  = 20;
    player.rec.height = 20;
    player.speed.x    = 5;
    player.speed.y    = 5;
    player.color      = BLACK;

    player.startLinePos.x = 0;
    player.startLinePos.y = 0;
    player.endLinePos.x   = 0;
    player.endLinePos.y   = 0;
    player.isDrawing      = false;

    player.direction = DIRECTION_NONE;

    board.rec.x         = screenWidth / 2 - 200;
    board.rec.y         = screenHeight / 2 - 200;
    board.rec.width     = 400;
    board.rec.height    = 400;
    board.initialArea   = board.rec.width * board.rec.height;
    board.cutOffPercent = 0;
    board.color         = SKYBLUE;

    Wave w1 = LoadWave( "sound/First.wav" );
    Wave w2 = LoadWave( "sound/Second.wav" );
    Wave w3 = LoadWave( "sound/Ending.wav" );
    if ( !IsWaveReady( w1 ) || !IsWaveReady( w2 ) )
    {
        // Well stuff happens
        exit( 1 );
    }

    firstSound  = LoadSoundFromWave( w1 );
    secondSound = LoadSoundFromWave( w2 );
    endingSound = LoadSoundFromWave( w3 );

    SetSoundVolume( firstSound, 100.0 );
    SetSoundVolume( secondSound, 100.0 );
    SetSoundVolume( endingSound, 100.0 );
    soundFlag = true;
}

//--------------------------------------------------------------------------------------
// Update and Draw (one frame)
void UpdateDrawFrame( void )
{
    UpdateGame();
    DrawGame();
}

//--------------------------------------------------------------------------------------
// Update game (one frame)
void UpdateGame( void )
{
    if ( !gameOver )
    {
        if ( IsKeyPressed( 'P' ) )
            pause = !pause;

        if ( pause )
        {
            return;
        }

        if ( alphaDecrease == false )
        {
            alpha += 0.02f;
        }
        else
        {
            alpha -= 0.02f;
        }

        if ( alpha >= 1.00f )
        {
            alphaDecrease = true;
        }

        if ( player.isDrawing == true &&
             !CheckCollisionPointRec( ( Vector2 ){ player.rec.x, player.rec.y },
                                      board.rec ) )
        {
            player.isDrawing = false;
            // At this point i know i have cut the board into two pieces
            // Check which area is the bigger
            Rectangle rec1;
            Rectangle rec2;
            switch ( player.direction )
            {
            case DIRECTION_RIGHT:
            {
               rec1.x      = board.rec.x;
               rec1.y      = board.rec.y;
               rec1.width  = fabsf( player.endLinePos.x - board.rec.x );
               rec1.height = fabsf( player.startLinePos.y - board.rec.y );

               rec2.x      = player.startLinePos.x;
               rec2.y      = player.startLinePos.y;
               rec2.width  = fabsf( player.endLinePos.x - board.rec.x );
               rec2.height = fabsf( board.rec.y + board.rec.height -
                                    player.startLinePos.y );
               break;
            }
            case DIRECTION_LEFT:
            {
               rec1.x = player.endLinePos.x;
               rec1.y = board.rec.y;
               rec1.width =
                   fabsf( player.startLinePos.x - player.endLinePos.x );
               rec1.height = fabsf( player.startLinePos.y - board.rec.y );

               rec2.x = player.endLinePos.x;
               rec2.y = player.startLinePos.y;
               rec2.width =
                   fabsf( board.rec.x + board.rec.width - player.endLinePos.x );
               rec2.height = fabsf( board.rec.y + board.rec.height -
                                    player.endLinePos.y );
               break;
            }
            case DIRECTION_DOWN:
            {
               rec1.x     = board.rec.x;
               rec1.y     = board.rec.y;
               rec1.width = fabsf( player.startLinePos.x - board.rec.x );
               rec1.height =
                   fabsf( player.startLinePos.y - player.endLinePos.y );

               rec2.x     = player.startLinePos.x;
               rec2.y     = player.startLinePos.y;
               rec2.width = fabsf( board.rec.x + board.rec.width -
                                   player.startLinePos.x );
               rec2.height =
                   fabsf( player.startLinePos.y - player.endLinePos.y );
               break;
            }
            case DIRECTION_UP:
            {
               rec1.x     = board.rec.x;
               rec1.y     = board.rec.y;
               rec1.width = fabsf( player.startLinePos.x - board.rec.x );
               rec1.height =
                   fabsf( player.startLinePos.y - player.endLinePos.y );

               rec2.x     = player.endLinePos.x;
               rec2.y     = player.endLinePos.y;
               rec2.width = fabsf( board.rec.x + board.rec.width -
                                   player.startLinePos.x );
               rec2.height =
                   fabsf( player.startLinePos.y - player.endLinePos.y );
               break;
            }
            default:
            {
               // Wtf is happening
            }
            }

            player.direction = DIRECTION_NONE;
            // Remove smaller area
            // Update new board rec

            switch ( isRecBigger( &rec1, &rec2 ) )
            {
            case 1:
               board.rec = rec1;
               board.cutOffArea += rec2.width * rec2.height;
               break;
            case 2:
               board.rec = rec2;
               board.cutOffArea += rec1.width * rec1.height;
               break;
            case 0:
               board.rec = rec1;
               board.cutOffArea += rec1.width * rec1.height;
               break;
            }

            board.cutOffPercent =
                ( ( board.cutOffArea ) * 100 ) / board.initialArea;

            score = board.cutOffPercent;
            if ( score >= WIN_PERCENTAGE )
            {
               victory = true;
            }
        }

        if ( player.isDrawing == false &&
             CheckCollisionPointRec( ( Vector2 ){ player.rec.x, player.rec.y },
                                     board.rec ) )
        {
            player.startLinePos.x = player.rec.x;
            player.startLinePos.y = player.rec.y;
            player.isDrawing      = true;
        }

        if ( player.isDrawing == true )
        {
            player.endLinePos.x = player.rec.x;
            player.endLinePos.y = player.rec.y;
        }

        // Player movement
        if ( player.direction == DIRECTION_NONE ||
             !CheckCollisionPointRec( ( Vector2 ){ player.rec.x, player.rec.y },
                                      board.rec ) )
        {
           if ( IsKeyDown( KEY_RIGHT ) )
           {
              player.rec.x += player.speed.x;
              player.direction = DIRECTION_RIGHT;
           }
           else if ( IsKeyDown( KEY_LEFT ) )
           {
              player.rec.x -= player.speed.x;
              player.direction = DIRECTION_LEFT;
           }
           else if ( IsKeyDown( KEY_UP ) )
           {
              player.rec.y -= player.speed.y;
              player.direction = DIRECTION_UP;
           }
           else if ( IsKeyDown( KEY_DOWN ) )
           {
              player.rec.y += player.speed.y;
              player.direction = DIRECTION_DOWN;
           }
        }
        else
        {
           switch ( player.direction )
           {
           case DIRECTION_UP:
           {
              player.rec.y -= player.speed.y;
              break;
           }
           case DIRECTION_RIGHT:
           {
              player.rec.x += player.speed.x;
              break;
           }
           case DIRECTION_DOWN:
           {
              player.rec.y += player.speed.y;
              break;
           }
           case DIRECTION_LEFT:
           {
              player.rec.x -= player.speed.x;
              break;
           }
           default:
           {
              TraceLog( LOG_INFO, "Wtf direction u going" );
           }
           }
        }

        // Wall behaviour
        if ( player.rec.x <= 0 )
        {
           player.rec.x = 0;
        }
        if ( player.rec.x + player.rec.width >= screenWidth )
        {
           player.rec.x = screenWidth - player.rec.width;
        }
        if ( player.rec.y <= 0 )
        {
           player.rec.y = 0;
        }
        if ( player.rec.y + player.rec.height >= screenHeight )
        {
           player.rec.y = screenHeight - player.rec.height;
        }
    }
    else
    {
        if ( IsKeyPressed( KEY_ENTER ) )
        {
            InitGame();
            gameOver = false;
        }
    }
}

//--------------------------------------------------------------------------------------
// Draw game (one frame)
void DrawGame( void )
{
    BeginDrawing();

    ClearBackground( RAYWHITE );

    if ( !gameOver )
    {

       DrawRectangleRec( board.rec, board.color );

       DrawRectangleRec( player.rec, player.color );

       DrawLineV( player.startLinePos, player.endLinePos, RED );

       DrawText( "WTF",
                 screenWidth / 2 - MeasureText( "Private Parts", 40 ) / 2,
                 screenHeight / 2 - 40, 40, Fade( BLACK, alpha ) );

       DrawText( TextFormat( "%04i", score ), 20, 20, 40, GRAY );
       // Draw Variables
       TraceLog( LOG_INFO, "----------------------------------------" );
       TraceLog( LOG_INFO, "Alpha:         %f", alpha );
       TraceLog( LOG_INFO, "DeawingStart x:  %f", player.startLinePos.x );
       TraceLog( LOG_INFO, "DeawingStart y:  %f", player.startLinePos.y );
       TraceLog( LOG_INFO, "DeawingEND x:    %f", player.endLinePos.x );
       TraceLog( LOG_INFO, "DeawingEND y:    %f", player.endLinePos.y );
       TraceLog( LOG_INFO, "GetMasterVolume:        %d", GetMasterVolume() );
       TraceLog( LOG_INFO, "board.rec.x:        %f", board.rec.x );
       TraceLog( LOG_INFO, "board.rec.y:        %f", board.rec.y );
       TraceLog( LOG_INFO, "player.rec.x:         %f", player.rec.x );
       TraceLog( LOG_INFO, "player.rec.y:         %f", player.rec.y );
       TraceLog( LOG_INFO, "++++++++++++++++++++++++++++++++++++++++" );

       if ( victory )
          DrawText( "YOU WIN",
                    screenWidth / 2 - MeasureText( "YOU WIN", 40 ) / 2,
                    screenHeight / 2 - 40, 40, BLACK );

       if ( pause )
          DrawText( "GAME PAUSED",
                    screenWidth / 2 - MeasureText( "GAME PAUSED", 40 ) / 2,
                    screenHeight / 2 - 40, 40, GRAY );
    }
    else
    {
        DrawText( "PRESS [ENTER] TO PLAY AGAIN",
                  GetScreenWidth() / 2 -
                      MeasureText( "PRESS [ENTER] TO PLAY AGAIN", 20 ) / 2,
                  GetScreenHeight() / 2 - 50, 20, GRAY );
    }

    EndDrawing();
}

//--------------------------------------------------------------------------------------
void UnloadGame( void )
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

//--------------------------------------------------------------------------------------
/// Function to get bigger rectangle
/// return 1 if rec1 is bigger
/// return 2 if rec2 is bigger
/// return 0 if equal
static int isRecBigger( const Rectangle* rec1, const Rectangle* rec2 )
{
   if ( ( rec1->width * rec1->height ) > ( rec2->width * rec2->height ) )
   {
      return 1;
   }
   if ( ( rec1->width * rec1->height ) < ( rec2->width * rec2->height ) )
   {
      return 2;
   }

   return 0;
}
