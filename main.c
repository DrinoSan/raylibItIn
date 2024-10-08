#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------------
#define NUM_SHOOTS      50
#define NUM_MAX_ENEMIES 50
#define FIRST_WAVE      10
#define SECOND_WAVE     20
#define THIRD_WAVE      50

//--------------------------------------------------------------------------------------
typedef struct Player
{
    Rectangle rec;
    Vector2   speed;
    Color     color;
    Vector2   startLinePos;
    Vector2   endLinePos;
    bool      isDrawing;
} Player;

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

////------------------------------------------------------------------------------------
//// Module Functions Declaration (local)
////------------------------------------------------------------------------------------
static void InitGame( void );          // Initialize game
static void UpdateGame( void );        // Update game (one frame)
static void DrawGame( void );          // Draw game (one frame)
static void UnloadGame( void );        // Unload game
static void UpdateDrawFrame( void );   // Update and Draw (one frame)

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth  = 800;
    const int screenHeight = 600;

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
        else if ( alpha <= 0.00f )
        {
            alphaDecrease = false;
        }

        if ( IsKeyPressed( KEY_SPACE ) && player.isDrawing == true )
        {
            player.isDrawing = false;
        }

        if ( IsKeyPressed( KEY_SPACE ) && player.isDrawing == false )
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
        if ( IsKeyDown( KEY_RIGHT ) )
        {
            player.rec.x += player.speed.x;
            score += player.rec.x;
        }
        if ( IsKeyDown( KEY_LEFT ) )
        {
            player.rec.x -= player.speed.x;
            score -= player.rec.x;
        }
        if ( IsKeyDown( KEY_UP ) )
        {
            player.rec.y -= player.speed.y;
            score += player.rec.y;
        }
        if ( IsKeyDown( KEY_DOWN ) )
        {
            player.rec.y += player.speed.y;
            score -= player.rec.y;
        }

        // Wall behaviour
        if ( player.rec.x <= 0 )
        {
            score        = 0;
            player.rec.x = 0;
        }
        if ( player.rec.x + player.rec.width >= screenWidth )
        {
            score        = 0;
            player.rec.x = screenWidth - player.rec.width;
        }
        if ( player.rec.y <= 0 )
        {
            score        = 0;
            player.rec.y = 0;
        }
        if ( player.rec.y + player.rec.height >= screenHeight )
        {
            score        = 0;
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
        DrawRectangleRec( player.rec, player.color );

        DrawLineV( player.startLinePos, player.endLinePos, RED );

        DrawText( "Private Parts",
                  screenWidth / 2 - MeasureText( "Private Parts", 40 ) / 2,
                  screenHeight / 2 - 40, 40, Fade( BLACK, alpha ) );

        DrawText( TextFormat( "%04i", score ), 20, 20, 40, GRAY );
        // Draw Variables
        TraceLog( LOG_INFO, "----------------------------------------" );
        TraceLog( LOG_INFO, "Alpha:         %f", alpha );
        TraceLog( LOG_INFO, "DeawingStart x:  %d", player.startLinePos.x );
        TraceLog( LOG_INFO, "DeawingStart y:  %d", player.startLinePos.y );
        TraceLog( LOG_INFO, "DeawingEND x:    %d", player.endLinePos.x );
        TraceLog( LOG_INFO, "DeawingEND y:    %d", player.endLinePos.y );
        TraceLog( LOG_INFO, "GetMasterVolume:        %d", GetMasterVolume() );
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

