/*
============Pages===============
0 => Start
1 => Resume
2 => HOWTOPLAY
3 => GamePlay
4 => GameOver
5 => LeaderBoard
6 => Level
7 => credit
===========level================
0 => EASY
1 => MEDIUM
2 => HARD
*/
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define HEIGHT 600
#define WIDTH 800
#define TEXTCOLOR RAYWHITE
#define TEXTCOLOR2 BLACK
#define HINTTEXTCOLOR SKYBLUE
#define FONTSIZE 40
#define HOVER_FONTSIZE 50
#define LINEGAPFORTEXT 60
#define TEXTPOSY 200
#define BALLROWS 16
#define BALLCOLS 14
#define BALLNUM 3 // koita color er ball ase
#define BALLRADIUS ((WIDTH - 12) / (2 * BALLCOLS + 0.5))
#define CANNON_HEIGHT 120
#define CANNON_WIDTH 80
#define VELOCITY_OF_BULLET 20
#define TIMEREMAINING 75
#define BGMVOLUME 3
#define ANIMATION_TIME 0.1
#define g 2.5

Rectangle existedBalls[BALLROWS * BALLCOLS];
int randBallIdx[BALLROWS * BALLCOLS];
int downFallVelocity[BALLROWS * BALLCOLS] = {0}; // Dropping ball feature
bool falling[BALLROWS * BALLCOLS] = {false};     // Dropping ball feature
int ballXadd = 0, ballYadd = 0;

int ballIndex = 0;
int removedBalls = 0;
int initialBallRows = 5;
int score = 0;
int pageIndex = 0;
float timeRemaining;
float timePassed;
float blastAnimation;
bool shooted;
int high_scores[3][5];
char gameOverReason[80];
bool changeColor = false; // color change er agei bullet color change howa bug er fix
int level = 0;

void loadHighScore(int lvl)
{
    FILE *file;
    switch (lvl)
    {
    case 0:
        file = fopen("easyScore.txt", "r");
        break;
    case 1:
        file = fopen("mediumScore.txt", "r");
        break;
    case 2:
        file = fopen("hardScore.txt", "r");
        break;
    }

    if (file != NULL)
    {
        for (int i = 0; i < 5; i++)
        {
            int status = fscanf(file, "%d", &high_scores[lvl][i]);
            if (status != 1)
            {
                high_scores[lvl][i] = 0;
            }
        }
        fclose(file);
    }
    else
    { // creating file for highscores
        for (int i = 0; i < 5; i++)
        {
            high_scores[lvl][i] = 0;
        }
        FILE *file;
        switch (lvl)
        {
        case 0:
            file = fopen("easyScore.txt", "w");
            break;
        case 1:
            file = fopen("mediumScore.txt", "w");
            break;
        case 2:
            file = fopen("hardScore.txt", "w");
            break;
        }
        if (file != NULL)
        {
            for (int i = 0; i < 5; i++)
            {
                fprintf(file, "%d\n", high_scores[lvl][i]);
            }
            fclose(file);
        }
    }
}

void setBalls()
{
    for (int i = 0; i < initialBallRows + level; i++)
    {
        for (int j = 0; j < BALLCOLS; j++)
        {
            if ((i % 2) == 0)
            {
                ballXadd = BALLRADIUS;
            }
            else
            {
                ballXadd = 0;
            }

            existedBalls[i * BALLCOLS + j] = (Rectangle){
                j * BALLRADIUS * 2 + ballXadd,
                i * (BALLRADIUS * 1.735),
                BALLRADIUS * 2,
                BALLRADIUS * 2};

            int probabilityForTimeBall = GetRandomValue(0, 50);
            if (probabilityForTimeBall < level + 2)
                randBallIdx[i * BALLCOLS + j] = 4;
            else if (probabilityForTimeBall == 10||probabilityForTimeBall == 11)
                randBallIdx[i * BALLCOLS + j] = 3;
            else
                randBallIdx[i * BALLCOLS + j] = GetRandomValue(0, 2);
            ballIndex++;
        }
    }
}

void NewGame()
{
    ballIndex = 0;
    removedBalls = 0;
    score = 0;
    timeRemaining = TIMEREMAINING - level * 10;
    blastAnimation = 0;
    timePassed = 0;
    shooted = false;
    changeColor = false;
    strcpy(gameOverReason, "YOU WON!");

    for (int i = 0; i < BALLROWS * BALLCOLS; i++)
    {
        existedBalls[i] = (Rectangle){0, 0, 0, 0};
        falling[i] = false;
        downFallVelocity[i] = 0;
    }

    setBalls();
}

void GameOver(bool isWin)
{
    if (!isWin)
        score /= 2;
    pageIndex = 4;

    int index = -1;
    for (int i = 0; i < 5; i++)
    {
        if (high_scores[level][i] < score)
        {
            index = i;
            break;
        }
    }
    if (index != -1)
    {
        for (int i = 4; i > index; i--)
            high_scores[level][i] = high_scores[level][i - 1];
        high_scores[level][index] = score;

        FILE *highScoreFile;
        switch (level)
        {
        case 0:
            highScoreFile = fopen("easyScore.txt", "w");
            break;
        case 1:
            highScoreFile = fopen("mediumScore.txt", "w");
            break;
        case 2:
            highScoreFile = fopen("hardScore.txt", "w");
            break;
        }
        if (highScoreFile != NULL)
        {
            for (int i = 0; i < 5; i++)
            {
                fprintf(highScoreFile, "%d\n", high_scores[level][i]);
            }
            fclose(highScoreFile);
        }
    }
}

float GetDistance(Vector2 p1, Vector2 p2)
{
    return sqrtf((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

void FindConnectedBalls(int index, bool visited[], int matchedIndices[], int *count)
{
    visited[index] = true;
    matchedIndices[*count] = index;
    (*count)++;

    Vector2 p1 = {existedBalls[index].x + BALLRADIUS, existedBalls[index].y + BALLRADIUS};

    for (int i = 0; i < BALLROWS * BALLCOLS; i++)
    {
        if (!visited[i] && existedBalls[i].width > 0)
        {
            Vector2 p2 = {existedBalls[i].x + BALLRADIUS, existedBalls[i].y + BALLRADIUS};
            float dist = GetDistance(p1, p2);

            if (dist < BALLRADIUS * 2.1)
            {
                FindConnectedBalls(i, visited, matchedIndices, count);
            }
        }
    }
}

bool CheckDownfall(int Index)
{
    bool holdingBallExist = false;

    bool visited[BALLROWS * BALLCOLS] = {false};
    int matchedIndices[BALLROWS * BALLCOLS];
    int count = 0;

    FindConnectedBalls(Index, visited, matchedIndices, &count);

    for (int i = 0; i < count; i++)
    {
        int idx = matchedIndices[i];

        int row = idx / BALLCOLS;

        if (row == 0)
        {
            holdingBallExist = true;
            break;
        }
    }

    if (!holdingBallExist)
    {
        for (int i = 0; i < count; i++)
        {
            int idx = matchedIndices[i];

            if (!falling[idx])
            {
                falling[idx] = true;
                downFallVelocity[idx] = 5;
                removedBalls++;
            }
        }

        return true;
    }

    return false;
}

int placingIndex(Vector2 pos)
{
    int closestIdx = -1;
    float minDistance = 10000000.0;

    for (int i = 0; i < BALLROWS * BALLCOLS; i++)
    {
        if (existedBalls[i].width == 0)
        {
            int r = i / BALLCOLS;
            int c = i % BALLCOLS;
            float xAdd = (r % 2 == 0) ? BALLRADIUS : 0;

            Vector2 cellPos = {
                c * BALLRADIUS * 2 + xAdd + BALLRADIUS,
                r * (BALLRADIUS * 1.735) + BALLRADIUS};

            float dist = GetDistance(pos, cellPos);
            if (dist < minDistance)
            {
                minDistance = dist;
                closestIdx = i;
            }
        }
    }
    return closestIdx;
}

void FindConnectedColorBalls(int index, int targetColor, bool visited[], int matchedIndices[], int *count)
{
    visited[index] = true;
    matchedIndices[*count] = index;
    (*count)++;

    Vector2 p1 = {existedBalls[index].x + BALLRADIUS, existedBalls[index].y + BALLRADIUS};

    for (int i = 0; i < BALLROWS * BALLCOLS; i++)
    {
        if (!visited[i] && existedBalls[i].width > 0 && randBallIdx[i] == targetColor)
        {
            Vector2 p2 = {existedBalls[i].x + BALLRADIUS, existedBalls[i].y + BALLRADIUS};
            float dist = GetDistance(p1, p2);

            if (dist < BALLRADIUS * 2.1)
            {
                FindConnectedColorBalls(i, targetColor, visited, matchedIndices, count);
            }
        }
    }
}

bool CheckSimpleMatches(int hitIndex)
{
    bool isVanished = false;
    bool visited[BALLROWS * BALLCOLS] = {false};
    int matchedIndices[BALLROWS * BALLCOLS];
    int count = 0;
    int targetColor = randBallIdx[hitIndex];

    FindConnectedColorBalls(hitIndex, targetColor, visited, matchedIndices, &count);

    if (count >= 3)
    {
        for (int i = 0; i < count; i++)
        {
            int idx = matchedIndices[i];

            if (!falling[idx])
            {
                existedBalls[idx] = (Rectangle){0, 0, 0, 0};
                falling[idx] = false;
                downFallVelocity[idx] = 0;
                removedBalls++;
            }
        }

        isVanished = true;
    }
    return isVanished;
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Bouncing Ball");

    // BGM
    InitAudioDevice();
    Music menuBgm = LoadMusicStream("assets/sounds/menu.wav");
    Music gameplayResumeBgm = LoadMusicStream("assets/sounds/Gameplay_Resume.mp3");
    Music gameOverBgm = LoadMusicStream("assets/sounds/gameover.mp3");
    Music countDown = LoadMusicStream("assets/sounds/countdown.mp3");
    Music *bgm = &menuBgm;

    // Sound effect
    Sound blast = LoadSound("assets/sounds/blast.mp3");
    Sound bonus = LoadSound("assets/sounds/Bonus.wav");
    Sound lostTime = LoadSound("assets/sounds/lostTime.wav");
    Sound click = LoadSound("assets/sounds/Click.mp3");
    Sound navigate = LoadSound("assets/sounds/navigate.mp3");
    Sound gameover = LoadSound("assets/sounds/gameover_instant.mp3");
    Sound winSound = LoadSound("assets/sounds/win.wav");

    int vol = BGMVOLUME;
    PlayMusicStream(*bgm);
    PlayMusicStream(countDown);

    SetTargetFPS(60);

    // score
    for (int i = 0; i <= 2; i++)
        loadHighScore(i);

    // Global
    Texture2D bg = LoadTexture("assets/bg.png");
    Texture2D logo = LoadTexture("assets/logo.png");

    // Menu Page
    int selectedOption = 0;
    int exit = 0;

    // GameOver
    int selected = 0;

    // resume
    int selected2 = 0;
    const char *Resumehint = "[ PRESS 'P' TO PAUSE ]";

    // level
    int selected3 = 0;

    // leaderboard
    int selected4 = 0;

    // LeaderBoard & About
    const char *backHint = "[ PRESS 'B' TO GO BACK ]";

    Texture2D balls[BALLNUM + 2];

    for (int i = 0; i < BALLNUM; i++)
    {
        char path[50];
        sprintf(path, "assets/ball_%d.png", i + 1);
        balls[i] = LoadTexture(path);
    }
    balls[BALLNUM] = LoadTexture("assets/ball_t.png");
    balls[BALLNUM + 1] = LoadTexture("assets/ball_t-.png");

    // Shooter
    Vector2 cannonBase = {WIDTH / 2.0, HEIGHT * 0.95};
    Vector2 cannonOrigin = {CANNON_WIDTH / 2, CANNON_HEIGHT};
    float cannonAngle = 0;
    Texture2D shooters[BALLNUM];
    for (int i = 0; i < BALLNUM; i++)
    {
        char path[50];
        sprintf(path, "assets/shooter_%d.png", i + 1);
        shooters[i] = LoadTexture(path);
    }

    // Blast
    Texture2D blastShooters[BALLNUM];
    for (int i = 0; i < BALLNUM; i++)
    {
        char path[50];
        sprintf(path, "assets/shooter_%d_blast.png", i + 1);
        blastShooters[i] = LoadTexture(path);
    }

    int shooterIndex = GetRandomValue(0, BALLNUM - 1);
    int shooterIndex2 = GetRandomValue(0, BALLNUM - 1);
    Vector2 bulletPosition = {0, 0};
    Vector2 bulletVelocity = {0, 0};

    while (!WindowShouldClose() && !exit)
    {
        char vol_text[30];
        if (vol != 0)
            strcpy(vol_text, "TURN OFF MUSIC");
        else
            strcpy(vol_text, "TURN ON MUSIC");

        UpdateMusicStream(*bgm);
        BeginDrawing();
        DrawTexture(bg, 0, 0, WHITE);
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))
            PlaySound(navigate);
        if (IsKeyPressed(KEY_ENTER))
            PlaySound(click);
        switch (pageIndex)
        {
        case 0: // Menu
            if (bgm != &menuBgm)
            {
                StopMusicStream(*bgm);
                bgm = &menuBgm;
                PlayMusicStream(*bgm);
            }
            SetMusicVolume(*bgm, vol);
            DrawTexturePro(logo,
                           (Rectangle){0, 0, logo.width, logo.height},
                           (Rectangle){WIDTH / 6, 5, WIDTH / 1.5, HEIGHT / 3},
                           Vector2Zero(), 0, WHITE);

            Rectangle NGbutton = {GetScreenWidth() / 2 - MeasureText("NEW GAME", FONTSIZE) / 2, TEXTPOSY, MeasureText("NEW GAME", FONTSIZE), MeasureTextEx(GetFontDefault(), "NEW GAME", FONTSIZE, 2).y};
            Rectangle Abutton = {GetScreenWidth() / 2 - MeasureText("ABOUT", FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, MeasureText("ABOUT", FONTSIZE), MeasureTextEx(GetFontDefault(), "ABOUT", FONTSIZE, 2).y};
            Rectangle LDbutton = {GetScreenWidth() / 2 - MeasureText("LEADERBOARD", FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, MeasureText("LEADERBOARD", FONTSIZE), MeasureTextEx(GetFontDefault(), "LEADERBOARD", FONTSIZE, 2).y};
            Rectangle VLbutton = {GetScreenWidth() / 2 - MeasureText(vol_text, FONTSIZE) / 2, TEXTPOSY + 3 * LINEGAPFORTEXT, MeasureText(vol_text, FONTSIZE), MeasureTextEx(GetFontDefault(), vol_text, FONTSIZE, 2).y};
            Rectangle Cbutton = {GetScreenWidth() / 2 - MeasureText("CREDITS", FONTSIZE) / 2, TEXTPOSY + 4 * LINEGAPFORTEXT, MeasureText("CREDITS", FONTSIZE), MeasureTextEx(GetFontDefault(), "CREDITS", FONTSIZE, 2).y};
            Rectangle Ebutton = {GetScreenWidth() / 2 - MeasureText("EXIT", FONTSIZE) / 2, TEXTPOSY + 5 * LINEGAPFORTEXT, MeasureText("EXIT", FONTSIZE), MeasureTextEx(GetFontDefault(), "EXIT", FONTSIZE, 2).y};

            if (CheckCollisionPointRec(GetMousePosition(), NGbutton))
                selectedOption = 0;
            if (CheckCollisionPointRec(GetMousePosition(), Abutton))
                selectedOption = 1;
            if (CheckCollisionPointRec(GetMousePosition(), LDbutton))
                selectedOption = 2;
            if (CheckCollisionPointRec(GetMousePosition(), VLbutton))
                selectedOption = 3;
            if (CheckCollisionPointRec(GetMousePosition(), Cbutton))
                selectedOption = 4;
            if (CheckCollisionPointRec(GetMousePosition(), Ebutton))
                selectedOption = 5;

            if (selectedOption == 0)
                DrawText("NEW GAME", GetScreenWidth() / 2 - MeasureText("NEW GAME", HOVER_FONTSIZE) / 2, TEXTPOSY, HOVER_FONTSIZE, TEXTCOLOR); // Hover effect
            else
                DrawText("NEW GAME", GetScreenWidth() / 2 - MeasureText("NEW GAME", FONTSIZE) / 2, TEXTPOSY, FONTSIZE, TEXTCOLOR);

            if (selectedOption == 1)
                DrawText("HOW TO PLAY", GetScreenWidth() / 2 - MeasureText("HOW TO PLAY", HOVER_FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR); // Hover effect
            else
                DrawText("HOW TO PLAY", GetScreenWidth() / 2 - MeasureText("HOW TO PLAY", FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (selectedOption == 2)
                DrawText("LEADERBOARD", GetScreenWidth() / 2 - MeasureText("LEADERBOARD", HOVER_FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR); // Hover effect
            else
                DrawText("LEADERBOARD", GetScreenWidth() / 2 - MeasureText("LEADERBOARD", FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (selectedOption == 3)
                DrawText(vol_text, GetScreenWidth() / 2 - MeasureText(vol_text, HOVER_FONTSIZE) / 2, TEXTPOSY + 3 * LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR); // Hover effect
            else
                DrawText(vol_text, GetScreenWidth() / 2 - MeasureText(vol_text, FONTSIZE) / 2, TEXTPOSY + 3 * LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (selectedOption == 4)
                DrawText("CREDITS", GetScreenWidth() / 2 - MeasureText("CREDITS", HOVER_FONTSIZE) / 2, TEXTPOSY + 4 * LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR); // Hover effect
            else
                DrawText("CREDITS", GetScreenWidth() / 2 - MeasureText("CREDITS", FONTSIZE) / 2, TEXTPOSY + 4 * LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (selectedOption == 5)
                DrawText("EXIT", GetScreenWidth() / 2 - MeasureText("EXIT", HOVER_FONTSIZE) / 2, TEXTPOSY + 5 * LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR); // Hover effect
            else
                DrawText("EXIT", GetScreenWidth() / 2 - MeasureText("EXIT", FONTSIZE) / 2, TEXTPOSY + 5 * LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (IsKeyPressed(KEY_UP) && selectedOption > 0)
                selectedOption--;

            if (IsKeyPressed(KEY_DOWN) && selectedOption < 5)
                selectedOption++;

            // Page Shifting
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                switch (selectedOption)
                {
                case 0: // GamePlay
                    selectedOption = 0;
                    pageIndex = 6;
                    break;

                case 1: // HOW TO PLAY
                    selectedOption = 0;
                    pageIndex = 2;
                    break;
                case 2: // Leaderboard
                    selectedOption = 0;
                    level = 0;
                    pageIndex = 5;
                    break;
                case 3:
                    if (vol == 0)
                        vol = BGMVOLUME;
                    else
                        vol = 0;
                    break;

                case 4: // credits
                    selectedOption = 0;
                    pageIndex = 7;
                    break;
                case 5: // Exit
                    selectedOption = 0;
                    exit = 1;
                    break;
                }
            }

            break;

        case 1: // Resume page
            if (bgm != &gameplayResumeBgm)
            {
                StopMusicStream(*bgm);
                bgm = &gameplayResumeBgm;
                PlayMusicStream(*bgm);
            }
            SetMusicVolume(*bgm, vol);
            // resume,new game,mainmenu,volume,exit

            Rectangle Rbutton = {GetScreenWidth() / 2 - MeasureText("RESUME", FONTSIZE) / 2, TEXTPOSY, MeasureText("RESUME", FONTSIZE), MeasureTextEx(GetFontDefault(), "RESUME", FONTSIZE, 2).y};
            Rectangle NGutton2 = {GetScreenWidth() / 2 - MeasureText("NEW GAME", FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, MeasureText("NEW GAME", FONTSIZE), MeasureTextEx(GetFontDefault(), "NEW GAME", FONTSIZE, 2).y};
            Rectangle MMbutton = {GetScreenWidth() / 2 - MeasureText("MAIN MENU", FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, MeasureText("MAIN MENU", FONTSIZE), MeasureTextEx(GetFontDefault(), "MAIN MENU", FONTSIZE, 2).y};
            Rectangle VLbutton2 = {GetScreenWidth() / 2 - MeasureText(vol_text, FONTSIZE) / 2, TEXTPOSY + 3 * LINEGAPFORTEXT, MeasureText(vol_text, HOVER_FONTSIZE), MeasureTextEx(GetFontDefault(), vol_text, FONTSIZE, 2).y};
            Rectangle Ebutton2 = {GetScreenWidth() / 2 - MeasureText("EXIT", FONTSIZE) / 2, TEXTPOSY + 4 * LINEGAPFORTEXT, MeasureText("EXIT", FONTSIZE), MeasureTextEx(GetFontDefault(), "EXIT", FONTSIZE, 2).y};

            if (CheckCollisionPointRec(GetMousePosition(), Rbutton))
                selected2 = 0;
            if (CheckCollisionPointRec(GetMousePosition(), NGutton2))
                selected2 = 1;
            if (CheckCollisionPointRec(GetMousePosition(), MMbutton))
                selected2 = 2;
            if (CheckCollisionPointRec(GetMousePosition(), VLbutton2))
                selected2 = 3;
            if (CheckCollisionPointRec(GetMousePosition(), Ebutton2))
                selected2 = 4;

            if (selected2 == 0)
                DrawText("RESUME", WIDTH / 2 - MeasureText("RESUME", HOVER_FONTSIZE) / 2, TEXTPOSY, HOVER_FONTSIZE, TEXTCOLOR);
            else
                DrawText("RESUME", WIDTH / 2 - MeasureText("RESUME", FONTSIZE) / 2, TEXTPOSY, FONTSIZE, TEXTCOLOR);

            if (selected2 == 1)
                DrawText("NEW GAME", WIDTH / 2 - MeasureText("NEW GAME", HOVER_FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR);
            else
                DrawText("NEW GAME", WIDTH / 2 - MeasureText("NEW GAME", FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (selected2 == 2)
                DrawText("MAIN MENU", WIDTH / 2 - MeasureText("MAIN MENU", HOVER_FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR);
            else
                DrawText("MAIN MENU", WIDTH / 2 - MeasureText("MAIN MENU", FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (selected2 == 3)
                DrawText(vol_text, WIDTH / 2 - MeasureText(vol_text, HOVER_FONTSIZE) / 2, TEXTPOSY + 3 * LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR);
            else
                DrawText(vol_text, WIDTH / 2 - MeasureText(vol_text, FONTSIZE) / 2, TEXTPOSY + 3 * LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (selected2 == 4)
                DrawText("EXIT", WIDTH / 2 - MeasureText("EXIT", HOVER_FONTSIZE) / 2, TEXTPOSY + 4 * LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR);
            else
                DrawText("EXIT", WIDTH / 2 - MeasureText("EXIT", FONTSIZE) / 2, TEXTPOSY + 4 * LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (IsKeyPressed(KEY_UP) && selected2 > 0)
                selected2--;
            if (IsKeyPressed(KEY_DOWN) && selected2 < 4)
                selected2++;

            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (selected2 == 0)
                    pageIndex = 3;
                else if (selected2 == 1)
                    pageIndex = 3,NewGame();
                else if (selected2 == 2)
                    pageIndex = 0;
                else if (selected2 == 3)
                {
                    if (vol == 0)
                        vol = BGMVOLUME;
                    else
                        vol = 0;
                }
                else
                    exit = true;
                selected2 = 0;
            }
            break;
        case 2: // About
            if (bgm != &menuBgm)
            {
                StopMusicStream(*bgm);
                bgm = &menuBgm;
                PlayMusicStream(*bgm);
            }
            SetMusicVolume(*bgm, vol);
            if (IsKeyPressed(KEY_B))
            {
                PlaySound(navigate);
                pageIndex = 0;
            }

            DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.7f));
            DrawText(backHint, (WIDTH - MeasureText(backHint, FONTSIZE / 2)) / 2, HEIGHT - 35, FONTSIZE / 2, HINTTEXTCOLOR);
            DrawText("HOW TO PLAY", GetScreenWidth() / 2 - MeasureText("HOW TO PLAY", FONTSIZE) / 2, 10, FONTSIZE, TEXTCOLOR);

            DrawText("GAMEPLAY:", 10,TEXTPOSY/4, FONTSIZE/1.25, GOLD);   
            DrawText("Match 3 or more balls of the same to", 15+MeasureText("GAMEPLAY:",FONTSIZE/1.25),TEXTPOSY/4, FONTSIZE/1.25, TEXTCOLOR);
            DrawText("pop them.Clear all balls from the grid to win the", 10,TEXTPOSY/4+LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);
            DrawText("level.", 10,TEXTPOSY/4+2*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);
            
            DrawText("SPECIAL TIME BALLS:", 10,TEXTPOSY/4+3*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, GOLD);   
            DrawText("Black Ball: 05sec Penalty", 15+MeasureText("SPECIAL TIME BALLS:",FONTSIZE/1.25),TEXTPOSY/4+3*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);
            DrawText("RED Ball: 05sec Bonus", 15+MeasureText("SPECIAL TIME BALLS:",FONTSIZE/1.25),TEXTPOSY/4+4*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);
            
            DrawText("CONTROLS:", 10,TEXTPOSY/4+5*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, GOLD);
            DrawText("Aim Cannon: Move Mouse", 15+MeasureText("CONTROLS:",FONTSIZE/1.25),TEXTPOSY/4+5*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);
            DrawText("Fire Ball: SPACE / Left Mouse Click", 15+MeasureText("CONTROLS:",FONTSIZE/1.25),TEXTPOSY/4+6*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);
            DrawText("Pause Game: P Key", 15+MeasureText("CONTROLS:",FONTSIZE/1.25),TEXTPOSY/4+7*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);

            DrawText("GAME OVER:", 10,TEXTPOSY/4+8*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, GOLD);
            DrawText("Timer hits 0 OR balls cross the", 15+MeasureText("GAME OVER:",FONTSIZE/1.25),TEXTPOSY/4+8*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);
            DrawText("lower danger line.", 10,TEXTPOSY/4+9*LINEGAPFORTEXT/1.5, FONTSIZE/1.25, TEXTCOLOR);
            break;

        case 3: // GamePlay
            score = removedBalls * 10;
            if (bgm != &gameplayResumeBgm)
            {
                StopMusicStream(*bgm);
                bgm = &gameplayResumeBgm;
                PlayMusicStream(*bgm);
            }
            SetMusicVolume(*bgm, vol);

            for (int i = 0; i < BALLROWS * BALLCOLS; i++)
            {
                if (existedBalls[i].width > 0 && falling[i])
                {
                    downFallVelocity[i] += g;
                    existedBalls[i].y += downFallVelocity[i];

                    if (existedBalls[i].y > HEIGHT)
                    {
                        existedBalls[i] = (Rectangle){0, 0, 0, 0};
                        falling[i] = false;
                        downFallVelocity[i] = 0;
                    }
                }
            }

            if ((int)timeRemaining == 10)
            {
                StopMusicStream(countDown);
                PlayMusicStream(countDown);
            }
            if (timeRemaining <= 10)
                UpdateMusicStream(countDown),SetMusicVolume(countDown, 5);;

            timeRemaining -= GetFrameTime();
            timePassed += GetFrameTime();
            DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.3));
            DrawText(Resumehint, (WIDTH - MeasureText(Resumehint, FONTSIZE / 2)) / 2, HEIGHT - 20, FONTSIZE / 2, HINTTEXTCOLOR);
            if (removedBalls >= ballIndex || timeRemaining < 0)
            { // GameOver
                score += timeRemaining * 100;

                if (timeRemaining <= 0)
                {
                    strcpy(gameOverReason, "GAMEOVER! TIME UP!");
                    PlaySound(gameover);
                    GameOver(false);
                }
                else
                {
                    PlaySound(winSound);
                    GameOver(true);
                }
            }
            if (IsKeyPressed(KEY_P))
            {
                PlaySound(navigate);
                pageIndex = 1;
            }
            // drawing ball images

            for (int i = 0; i < BALLROWS * BALLCOLS; i++)
            {
                if (existedBalls[i].width > 0)
                    DrawTexturePro(balls[randBallIdx[i]],
                                   (Rectangle){0, 0, balls[randBallIdx[i]].width, balls[randBallIdx[i]].height},
                                   existedBalls[i],
                                   Vector2Zero(), 0, WHITE);
            }

            // shooter
            Vector2 mousePos = GetMousePosition();
            float dx = mousePos.x - cannonBase.x;
            float dy = mousePos.y - cannonBase.y;
            float tempAngle = (atan2f(dy, dx) * RAD2DEG);
            if (tempAngle <= -25 && tempAngle >= -155)
                cannonAngle = tempAngle;

            if (blastAnimation <= 0)
                DrawTexturePro(
                    shooters[shooterIndex],
                    (Rectangle){0, 0, shooters[shooterIndex].width, shooters[shooterIndex].height},
                    (Rectangle){cannonBase.x,cannonBase.y, CANNON_WIDTH,CANNON_HEIGHT},
                    cannonOrigin, cannonAngle + 90, WHITE);
            else
            {
                DrawTexturePro(
                    blastShooters[shooterIndex],
                    (Rectangle){0, 0, blastShooters[shooterIndex].width, blastShooters[shooterIndex].height},
                    (Rectangle){cannonBase.x, cannonBase.y, CANNON_WIDTH, CANNON_HEIGHT},
                    cannonOrigin,cannonAngle + 90,WHITE);
                blastAnimation -= GetFrameTime();
            }
            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (!shooted)
                {
                    PlaySound(blast);
                    blastAnimation = ANIMATION_TIME;
                    shooted = true;
                    bulletPosition.x = cannonBase.x + CANNON_HEIGHT * cos(cannonAngle * DEG2RAD) - BALLRADIUS;
                    bulletPosition.y = cannonBase.y + CANNON_HEIGHT * sin(cannonAngle * DEG2RAD) - BALLRADIUS;

                    bulletVelocity.x = VELOCITY_OF_BULLET * cos(cannonAngle * DEG2RAD);
                    bulletVelocity.y = VELOCITY_OF_BULLET * sin(cannonAngle * DEG2RAD);
                }
            }
            bool isCollision = false;
            if (shooted)
            {
                Rectangle shootedBall = (Rectangle){bulletPosition.x, bulletPosition.y, BALLRADIUS * 2, BALLRADIUS * 2};
                bulletPosition.x += bulletVelocity.x;
                bulletPosition.y += bulletVelocity.y;
                if (bulletPosition.x <= 0 || bulletPosition.x >= WIDTH - BALLRADIUS * 2)
                {
                    bulletVelocity.x = -bulletVelocity.x;
                }

                // collision with roof
                if (bulletPosition.y <= 0)
                {
                    Vector2 bulletCenter = {bulletPosition.x + BALLRADIUS, bulletPosition.y + BALLRADIUS};
                    int targetIdx = placingIndex(bulletCenter);

                    if (targetIdx != -1)
                    {
                        int r = targetIdx / BALLCOLS;
                        int c = targetIdx % BALLCOLS;
                        float xAdd = (r % 2 == 0) ? BALLRADIUS : 0;

                        existedBalls[targetIdx] = (Rectangle){c * BALLRADIUS * 2 + xAdd,r * (BALLRADIUS * 1.735),BALLRADIUS * 2, BALLRADIUS * 2};
                        randBallIdx[targetIdx] = shooterIndex;
                        ballIndex++;

                        CheckSimpleMatches(targetIdx);

                        for (int i = 0; i < BALLROWS * BALLCOLS; i++)
                        {
                            if (existedBalls[i].width > 0)
                            {
                                CheckDownfall(i);
                            }
                        }
                    }
                    changeColor = true;
                    isCollision = true;
                }

                for (int i = 0; i < BALLROWS * BALLCOLS && !isCollision; i++)
                {
                    if (existedBalls[i].width > 0)
                    {
                        Vector2 shootedCenter = {
                            shootedBall.x + BALLRADIUS,
                            shootedBall.y + BALLRADIUS};

                        Vector2 existedCenter = {
                            existedBalls[i].x + BALLRADIUS,
                            existedBalls[i].y + BALLRADIUS};

                        if (CheckCollisionCircles(
                                shootedCenter,
                                BALLRADIUS - 8,
                                existedCenter,
                                BALLRADIUS) &&
                            falling[i] != true)
                        {
                            isCollision = true;

                            if (randBallIdx[i] == 4)
                            {
                                timeRemaining -= 5;
                                PlaySound(lostTime);
                                existedBalls[i] = (Rectangle){0, 0, 0, 0};
                                removedBalls++;

                                for (int i = 0; i < BALLROWS * BALLCOLS; i++)
                                {
                                    if (existedBalls[i].width > 0)
                                    {
                                        CheckDownfall(i);
                                    }
                                }
                            }
                            else if (randBallIdx[i] == 3)
                            {
                                timeRemaining += 5;
                                PlaySound(bonus);
                                existedBalls[i] = (Rectangle){0, 0, 0, 0};
                                removedBalls++;

                                for (int i = 0; i < BALLROWS * BALLCOLS; i++)
                                {
                                    if (existedBalls[i].width > 0)
                                    {
                                        CheckDownfall(i);
                                    }
                                }
                            }
                            else
                            {
                                Vector2 bulletCenter = {
                                    bulletPosition.x + BALLRADIUS,
                                    bulletPosition.y + BALLRADIUS};

                                int targetIdx = placingIndex(bulletCenter);

                                if (targetIdx != -1)
                                {
                                    int r = targetIdx / BALLCOLS;
                                    int c = targetIdx % BALLCOLS;

                                    float xAdd =
                                        (r % 2 == 0) ? BALLRADIUS : 0;

                                    existedBalls[targetIdx] = (Rectangle){
                                        c * BALLRADIUS * 2 + xAdd,
                                        r * (BALLRADIUS * 1.735),
                                        BALLRADIUS * 2,
                                        BALLRADIUS * 2};

                                    randBallIdx[targetIdx] = shooterIndex;
                                    ballIndex++;
                                    bool isVanished = CheckSimpleMatches(targetIdx);

                                    for (int i = 0; i < BALLROWS * BALLCOLS && isVanished; i++)
                                    {
                                        if (existedBalls[i].width > 0)
                                        {
                                            CheckDownfall(i);
                                        }
                                    }

                                    if (!isVanished && (r * (BALLRADIUS * 1.735f)) >= 375)
                                    {
                                        pageIndex = 4;
                                        PlaySound(gameover);
                                        strcpy(gameOverReason, "GAMEOVER! YOU CROSSED THE LINE!");
                                        GameOver(false);
                                    }
                                }
                            }
                            changeColor = true;
                        }
                    }
                }

                DrawTexturePro(
                    balls[shooterIndex],
                    (Rectangle){0, 0, balls[shooterIndex].width, balls[shooterIndex].height},
                    (Rectangle){bulletPosition.x, bulletPosition.y, BALLRADIUS * 2, BALLRADIUS * 2},
                    Vector2Zero(), 0, WHITE);
                if (changeColor)
                {
                    shooted = false;
                    shooterIndex = shooterIndex2;
                    shooterIndex2 = GetRandomValue(0, BALLNUM - 1);
                    changeColor = false;
                }
            }

            // score
            char scores[30];
            sprintf(scores, "SCORE : %d", score);
            char time[20];
            sprintf(time, "TIME: %.0f", timeRemaining);
            DrawText(scores, 10, HEIGHT - MeasureTextEx(GetFontDefault(), scores, FONTSIZE, FONTSIZE / 10).y, FONTSIZE, TEXTCOLOR);

            int nextTextHeight = MeasureTextEx(GetFontDefault(), "NEXT: ", FONTSIZE, FONTSIZE / 10).y;
            int timeHeight = MeasureTextEx(GetFontDefault(), time, FONTSIZE, FONTSIZE / 10).y + nextTextHeight;
            DrawText(time, WIDTH - MeasureText(time, FONTSIZE) - 20, HEIGHT - timeHeight, FONTSIZE, timeRemaining >= 10 ? TEXTCOLOR : RED);
            DrawText("NEXT: ", WIDTH - MeasureText("NEXT: ", FONTSIZE) - 70, HEIGHT - nextTextHeight, FONTSIZE, TEXTCOLOR);
            Rectangle nextColor = {WIDTH - 60, HEIGHT - MeasureTextEx(GetFontDefault(), "NEXT: ", FONTSIZE, FONTSIZE / 10).y, 55, FONTSIZE};
            switch (shooterIndex2)
            {
            case 0:
                DrawRectangleRec(
                    nextColor,
                    (Color){255, 76, 97, 255});
                break;
            case 1:
                DrawRectangleRec(
                    nextColor,
                    (Color){65, 159, 221, 255});
                break;
            case 2:
                DrawRectangleRec(
                    nextColor,
                    (Color){67, 153, 108, 255});
                break;
            }
            break;
        case 4: // GameOver
            if (bgm != &gameOverBgm)
            {
                StopMusicStream(*bgm);
                bgm = &gameOverBgm;
                PlayMusicStream(*bgm);
            }
            StopMusicStream(countDown);
            SetMusicVolume(*bgm, vol);

            DrawText(gameOverReason, WIDTH / 2 - MeasureText(gameOverReason, FONTSIZE) / 2, 20, FONTSIZE, TEXTCOLOR2);

            char high[50];
            sprintf(high, "HIGH SCORE: %d", high_scores[level][0]);
            DrawText(high, WIDTH / 2 - MeasureText(high, FONTSIZE) / 2, 120, FONTSIZE, TEXTCOLOR2);

            DrawText(scores, WIDTH / 2 - MeasureText(scores, FONTSIZE) / 2, TEXTPOSY, FONTSIZE, TEXTCOLOR);
            char timeText[100];
            sprintf(timeText, "TIME: %.0f SEC", timePassed);
            DrawText(timeText, WIDTH / 2 - MeasureText(timeText, FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            Rectangle NGbutton2 = {135 - MeasureText("NEW GAME", FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, MeasureText("NEW GAME", FONTSIZE / 1.5), MeasureTextEx(GetFontDefault(), "NEW GAME", FONTSIZE / 1.5, 2).y};
            Rectangle LDbutton2 = {WIDTH / 2 - MeasureText("LEADERBOARD", FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, MeasureText("LEADERBOARD", FONTSIZE / 1.5), MeasureTextEx(GetFontDefault(), "LEADERBOARD", FONTSIZE / 1.5, 2).y};
            Rectangle Ebutton3 = {650 - MeasureText("EXIT", FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, MeasureText("EXIT", FONTSIZE / 1.5), MeasureTextEx(GetFontDefault(), "EXIT", FONTSIZE / 1.5, 2).y};

            if (CheckCollisionPointRec(GetMousePosition(), NGbutton2))
                selected = 0;

            if (CheckCollisionPointRec(GetMousePosition(), LDbutton2))
                selected = 1;

            if (CheckCollisionPointRec(GetMousePosition(), Ebutton3))
                selected = 2;

            if (selected == 0)
                DrawText("NEW GAME", 135 - MeasureText("NEW GAME", HOVER_FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, HOVER_FONTSIZE / 1.5, TEXTCOLOR);
            else
                DrawText("NEW GAME", 135 - MeasureText("NEW GAME", FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, FONTSIZE / 1.5, TEXTCOLOR);

            if (selected == 1)
                DrawText("LEADERBOARD", WIDTH / 2 - MeasureText("LEADERBOARD", HOVER_FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, HOVER_FONTSIZE / 1.5, TEXTCOLOR);
            else
                DrawText("LEADERBOARD", WIDTH / 2 - MeasureText("LEADERBOARD", FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, FONTSIZE / 1.5, TEXTCOLOR);

            if (selected == 2)
                DrawText("EXIT", 650 - MeasureText("EXIT", HOVER_FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, HOVER_FONTSIZE / 1.5, TEXTCOLOR);
            else
                DrawText("EXIT", 650 - MeasureText("EXIT", FONTSIZE / 1.5) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, FONTSIZE / 1.5, TEXTCOLOR);

            if (IsKeyPressed(KEY_LEFT) && selected > 0)
                selected--;

            if (IsKeyPressed(KEY_RIGHT) && selected < 2)
                selected++;

            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (selected == 0)
                {
                    selected = 0;
                    NewGame();
                    pageIndex = 3;
                }
                else if (selected == 1)
                {
                    selected = 0;
                    level = 0;
                    pageIndex = 5;
                }
                else
                    exit = true;
            }
            break;
        case 5: // Leaderboard
            if (bgm != &menuBgm)
            {
                StopMusicStream(*bgm);
                bgm = &menuBgm;
                PlayMusicStream(*bgm);
            }
            SetMusicVolume(*bgm, vol);

            if (IsKeyPressed(KEY_B))
            {
                PlaySound(navigate);
                selected4 = 0;
                pageIndex = 0;
            }
            DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.7));
            DrawText("LEADERBOARD", GetScreenWidth() / 2 - MeasureText("LEADERBOARD", HOVER_FONTSIZE) / 2, 20, HOVER_FONTSIZE, TEXTCOLOR);
            DrawText(backHint, (WIDTH - MeasureText(backHint, FONTSIZE / 2)) / 2, 0 + HEIGHT - 35, FONTSIZE / 2, HINTTEXTCOLOR);

            Rectangle eb = {GetScreenWidth() / 4 - MeasureText("EASY", FONTSIZE / 1.5) / 2, 90, MeasureText("EASY", FONTSIZE / 1.5), MeasureTextEx(GetFontDefault(), "EASY", FONTSIZE / 1.5, 2).y};
            Rectangle mb = {GetScreenWidth() / 2 - MeasureText("MEDIUM", FONTSIZE / 1.5) / 2, 90, MeasureText("MEDIUM", FONTSIZE / 1.5), MeasureTextEx(GetFontDefault(), "MEDIUM", FONTSIZE / 1.5, 2).y};
            Rectangle hb = {GetScreenWidth() * 0.75 - MeasureText("HARD", FONTSIZE / 1.5) / 2, 90, MeasureText("HARD", FONTSIZE / 1.5), MeasureTextEx(GetFontDefault(), "HARD", FONTSIZE / 1.5, 2).y};

            if (CheckCollisionPointRec(GetMousePosition(), eb))
                selected4 = 0, level = selected4;
            if (CheckCollisionPointRec(GetMousePosition(), mb))
                selected4 = 1, level = selected4;
            if (CheckCollisionPointRec(GetMousePosition(), hb))
                selected4 = 2, level = selected4;

            if (selected4 == 0)
                DrawText("EASY", GetScreenWidth() / 4 - MeasureText("EASY", FONTSIZE) / 2, 90, FONTSIZE, TEXTCOLOR);
            else
                DrawText("EASY", GetScreenWidth() / 4 - MeasureText("EASY", FONTSIZE / 1.5) / 2, 90, FONTSIZE / 1.5, TEXTCOLOR);

            if (selected4 == 1)
                DrawText("MEDIUM", GetScreenWidth() / 2 - MeasureText("MEDIUM", FONTSIZE) / 2, 90, FONTSIZE, TEXTCOLOR);
            else
                DrawText("MEDIUM", GetScreenWidth() / 2 - MeasureText("MEDIUM", FONTSIZE / 1.5) / 2, 90, FONTSIZE / 1.5, TEXTCOLOR);

            if (selected4 == 2)
                DrawText("HARD", GetScreenWidth() * 0.75 - MeasureText("HARD", FONTSIZE) / 2, 90, FONTSIZE, TEXTCOLOR);
            else
                DrawText("HARD", GetScreenWidth() * 0.75 - MeasureText("HARD", FONTSIZE / 1.5) / 2, 90, FONTSIZE / 1.5, TEXTCOLOR);

            if (IsKeyPressed(KEY_LEFT) && selected4 > 0)
            {
                selected4--;
                level = selected4;
            }
            if (IsKeyPressed(KEY_RIGHT) && selected4 < 2)
            {
                selected4++;
                level = selected4;
            }

            for (int i = 0; i < 5; i++)
            {
                char rankText[20], scoreText[20];
                sprintf(rankText, "RANK#%d", i + 1);
                sprintf(scoreText, "%d", high_scores[level][i]);
                DrawText(rankText, 200, 150 + i * LINEGAPFORTEXT, FONTSIZE / 1.25, TEXTCOLOR);
                DrawText(scoreText, 500, 150 + i * LINEGAPFORTEXT, FONTSIZE / 1.25, TEXTCOLOR);
            }
            break;
        case 6: // level
            if (bgm != &menuBgm)
            {
                StopMusicStream(*bgm);
                bgm = &menuBgm;
                PlayMusicStream(*bgm);
            }
            SetMusicVolume(*bgm, vol);

            Rectangle easyButton = {GetScreenWidth() / 2 - MeasureText("EASY", FONTSIZE) / 2, TEXTPOSY, MeasureText("EASY", FONTSIZE), MeasureTextEx(GetFontDefault(), "EASY", FONTSIZE, 2).y};
            Rectangle mediumButton = {GetScreenWidth() / 2 - MeasureText("MEDIUM", FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, MeasureText("MEDIUM", FONTSIZE), MeasureTextEx(GetFontDefault(), "MEDIUM", FONTSIZE, 2).y};
            Rectangle hardButton = {GetScreenWidth() / 2 - MeasureText("HARD", FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, MeasureText("HARD", FONTSIZE), MeasureTextEx(GetFontDefault(), "HARD", FONTSIZE, 2).y};

            if (CheckCollisionPointRec(GetMousePosition(), easyButton))
                selected3 = 0;

            if (CheckCollisionPointRec(GetMousePosition(), mediumButton))
                selected3 = 1;

            if (CheckCollisionPointRec(GetMousePosition(), hardButton))
                selected3 = 2;

            if (selected3 == 0)
                DrawText("EASY", WIDTH / 2 - MeasureText("EASY", HOVER_FONTSIZE) / 2, TEXTPOSY, HOVER_FONTSIZE, TEXTCOLOR);
            else
                DrawText("EASY", WIDTH / 2 - MeasureText("EASY", FONTSIZE) / 2, TEXTPOSY, FONTSIZE, TEXTCOLOR);

            if (selected3 == 1)
                DrawText("MEDIUM", WIDTH / 2 - MeasureText("MEDIUM", HOVER_FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR);
            else
                DrawText("MEDIUM", WIDTH / 2 - MeasureText("MEDIUM", FONTSIZE) / 2, TEXTPOSY + LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (selected3 == 2)
                DrawText("HARD", WIDTH / 2 - MeasureText("HARD", HOVER_FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, HOVER_FONTSIZE, TEXTCOLOR);
            else
                DrawText("HARD", WIDTH / 2 - MeasureText("HARD", FONTSIZE) / 2, TEXTPOSY + 2 * LINEGAPFORTEXT, FONTSIZE, TEXTCOLOR);

            if (IsKeyPressed(KEY_UP) && selected3 > 0)
                selected3--;
            if (IsKeyPressed(KEY_DOWN) && selected3 < 2)
                selected3++;

            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (selected3 == 0)
                    level = 0;
                else if (selected3 == 1)
                    level = 1;
                else
                    level = 2;
                selected3 = 0;
                NewGame();
                pageIndex = 3;
            }
            break;
        case 7: // credits
            if (bgm != &menuBgm)
            {
                StopMusicStream(*bgm);
                bgm = &menuBgm;
                PlayMusicStream(*bgm);
            }
            SetMusicVolume(*bgm, vol);
            if (IsKeyPressed(KEY_B))
            {
                PlaySound(navigate);
                pageIndex = 0;
            }

            DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.7));
            DrawText(backHint, (WIDTH - MeasureText(backHint, FONTSIZE / 2)) / 2, HEIGHT - 35, FONTSIZE / 2, HINTTEXTCOLOR);

            DrawText("CREDITS", WIDTH / 2 - MeasureText("CREDITS", HOVER_FONTSIZE) / 2, 40, HOVER_FONTSIZE, RAYWHITE);
            DrawLine(WIDTH / 2 - MeasureText("CREDITS", HOVER_FONTSIZE) / 2, 90, WIDTH / 2 + MeasureText("CREDITS", HOVER_FONTSIZE) / 2, 90, LIGHTGRAY);

            DrawText("GRAPHICS & ART", WIDTH / 2 - MeasureText("GRAPHICS & ART", FONTSIZE) / 2, TEXTPOSY / 2, FONTSIZE, GOLD);
            DrawText("Background,Ball, Cannon: Kenney.nl ,OpenGameArt.org", WIDTH / 2 - MeasureText("Background,Ball, Cannon: Kenney.nl ,OpenGameArt.org", FONTSIZE * 0.65) / 2, TEXTPOSY / 2 + LINEGAPFORTEXT, FONTSIZE * 0.65, WHITE);
            DrawText("AUDIO & MUSIC", WIDTH / 2 - MeasureText("AUDIO & MUSIC", FONTSIZE) / 2, TEXTPOSY / 2 + 2 * LINEGAPFORTEXT, FONTSIZE, GOLD);
            DrawText("Sound Effects (SFX): OpenGameArt.org, Mixkit.co, Pixabay.com", WIDTH / 2 - MeasureText("Sound Effects (SFX): OpenGameArt.org, Mixkit.co, Pixabay.com", FONTSIZE * 0.65) / 2, TEXTPOSY / 2 + 3 * LINEGAPFORTEXT, FONTSIZE * 0.65, WHITE);
            DrawText("Background Music: OpenGameArt.org, Mixkit.co, Pixabay.com", WIDTH / 2 - MeasureText("Background Music: OpenGameArt.org, Mixkit.co, Pixabay.com", FONTSIZE * 0.65) / 2, TEXTPOSY / 2 + 4 * LINEGAPFORTEXT, FONTSIZE * 0.65, WHITE);
            DrawText("Developed by Md Foisal & Shahariar Sajid Swapno", WIDTH / 2 - MeasureText("Developed by Md Foisal & Shahariar Sajid Swapno", FONTSIZE * 0.65) / 2, TEXTPOSY / 2 + 7 * LINEGAPFORTEXT, FONTSIZE * 0.65, WHITE);
            break;
        }
        EndDrawing();
    }

    // Unload image
    UnloadTexture(bg);
    UnloadTexture(logo);
    for (int i = 0; i < BALLNUM; i++)
    {
        UnloadTexture(shooters[i]);
        UnloadTexture(balls[i]);
        UnloadTexture(blastShooters[i]);
    }
    UnloadTexture(balls[BALLNUM]);
    UnloadTexture(balls[BALLNUM + 1]);

    // Unload Sound BGM
    UnloadMusicStream(menuBgm);
    UnloadMusicStream(gameplayResumeBgm);
    UnloadMusicStream(gameOverBgm);
    UnloadSound(blast);
    UnloadSound(gameover);
    UnloadSound(navigate);
    UnloadSound(click);
    UnloadSound(bonus);
    UnloadSound(lostTime);
    UnloadMusicStream(countDown);
    UnloadSound(winSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}