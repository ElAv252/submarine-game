#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/model.h"
#include "src/view.h"
#include "src/controller.h"
#include "raylib/src/raylib.h"

#define TERMINAL_YELLOW "\033[0;33m"

/*
============================================
              Submarine Game

CONST DATA:
    [
        0 - Empty cell
        1 - Submarine cell
        2 - Submarine damaged cell
        3 - Damaged empty cell
    ]

TODO:
1) Define the DS of the project.
2) Create a good grid, include matrix for it. - When i click on some cell, i need to know what is postion[x, y] that i click on. -> DONE
3) Add option to add submarine to the grid. -> DONE
4) Fix the problem with the most top left cell [0, 0]. -> DONE
5) Add the ruls of the games (Change the grid_diffe to matrix).
    [
        A. Need to limit the submarine size for each submarine. -> Solution Restrict the user. -> DONE
    ]
6) Need to make the game responsive. [Need to check it more]
7) Need to work and finish the turn system. -> DONE
8) Need to change the grid in the fight.
    [
        A. Need to change the grid for the use that he will be able to see his yellow shoot(2) and the the empty cells(0) -> DONE
        B. Need to create a mini grid for the user that he wil be able to see his grid when the turn is of him.
    ]

============================================
*/

const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 1050;
const int COLS = 25;
const int ROWS = 25;
const int CELL_WIDTH = (SCREEN_WIDTH/COLS)-2;
const int CELL_HEIGHT = (SCREEN_HEIGHT/ROWS)-10;

typedef struct Cell {
    int i;
    int j;
} Cell;

void CellDraw(Cell cell, int num);
void GridDraw(int cell[COLS][ROWS]);
void SetSubmarine(int grid_diffe[COLS][ROWS], int *max_size_submarine, const char is_vertical, const int temp, int *amount_of_submarines, int *j, int *turn_indication, Sound click_sound, char is_fight_started);
void CellDiffeDraw(int grid_diffe[COLS][ROWS], char is_fight_started);
char IsSubmarinePositionOk(int x, int y, int grid_diffe[COLS][ROWS], char is_vertical, int *max_size_submarine);
void DrawScore(int score_a, int score_b);
void ChooseDirection(int is_vertical);
void ChangeDirection(char *is_vertical);
void CellDamagedDraw(int grid_diffe[COLS][ROWS]);
void ShootOnSubmarine(int grid_diffe[COLS][ROWS], char *flip_turn, Sound shoot_sound);
void CheckForWin(int player_1[COLS][ROWS], int player_2[COLS][ROWS], Sound game_over_sound, char *game_not_over);
void DrawWhoPlay(char which_player_is);
void DrawWelcomeMessage(char *welcome_message);

/*
============================================
        M A I N - P R O G R A M
============================================
*/

int main()
{
    // Set-up the window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Submarine Game");
    SetTargetFPS(60);

    // Set-up camera
    Camera2D camera = {0};
    camera.offset = (Vector2){0, 0};
    camera.target = (Vector2){0, 0};
    camera.rotation = 0;
    camera.zoom = 1;

    // Set-up audio
    InitAudioDevice();

    if (!IsAudioDeviceReady()) {
        printf("%sERROR: The audio device is not ready, check it!\n", TERMINAL_YELLOW);
        return 0;
    }
    Sound welcome_sound = LoadSound("./resources/mixkit-medieval-show-fanfare-announcement-226.wav");
    Sound game_over_sound = LoadSound("./resources/mixkit-arcade-retro-game-over-213.wav");
    Sound click_sound = LoadSound("./resources/mixkit-video-game-retro-click-237.wav");
    Sound shoot_sound = LoadSound("./resources/plasma-gun-fire-162136.mp3");
    PlaySound(welcome_sound);

    int grid[COLS][ROWS];
    int player_1[COLS][ROWS];
    int player_2[COLS][ROWS];
    int max_size_submarine = 5;
    char is_vertical = 1;
    int *amount_of_submarines, amount_of_submarines_cells = 0, index = 0, j = 0, turn_indication = 0;
    char flip_turn = 1, game_not_over = 1, welcome_message = 1, is_fight_started = 0;

    for (int i = 1; i <= max_size_submarine; i++) amount_of_submarines_cells+=i;

    amount_of_submarines = calloc(sizeof(int), amount_of_submarines_cells);
    if (amount_of_submarines == NULL) return 0;

    for (int i = max_size_submarine; i >= 1; i--) {
        for (int j = max_size_submarine; j >= i; j--) {
            amount_of_submarines[index++] = i;
        }
    }

    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            grid[i][j] = 0;
        }
    }

    // Game loop
    while (!WindowShouldClose())
    {
        // ----------------------Draw----------------------
        BeginDrawing();
            BeginMode2D(camera);
                ClearBackground(BLACK);

                if (welcome_message) DrawWelcomeMessage(&welcome_message);
                else {
                    GridDraw(grid);
                    DrawWhoPlay(flip_turn);

                    if (turn_indication <= amount_of_submarines_cells) {
                        SetSubmarine(player_1, &max_size_submarine, is_vertical, amount_of_submarines_cells, amount_of_submarines, &j, &turn_indication, click_sound, is_fight_started);
                    }
                    else if (turn_indication <= (amount_of_submarines_cells*2)) {
                        flip_turn = 0;
                        if (turn_indication == amount_of_submarines_cells+1) j = 0;
                        SetSubmarine(player_2, &max_size_submarine, is_vertical, amount_of_submarines_cells, amount_of_submarines, &j, &turn_indication, click_sound, is_fight_started);
                    }
                    else if (turn_indication > (amount_of_submarines_cells*2)) {
                        is_fight_started = 1;
                        if (game_not_over) {
                            CellDiffeDraw(flip_turn ? player_1 : player_2, is_fight_started);
                            ShootOnSubmarine(flip_turn ? player_1 : player_2, &flip_turn, shoot_sound);
                        }
                        CheckForWin(player_1, player_2, game_over_sound, &game_not_over);
                    }
                    ChooseDirection(is_vertical);
                    ChangeDirection(&is_vertical);
                }

            DrawFPS(0, 0);
            EndMode2D();
        EndDrawing();
        // --------------------------------------------
    }

    free(amount_of_submarines);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}


/*
============================================
            F U N C T I O N S
============================================
*/

void CellDraw(Cell cell, int num)
{
    DrawText(TextFormat("%d", num), cell.j * CELL_WIDTH+10, SCREEN_HEIGHT-250, 25, WHITE); // X
    DrawText(TextFormat("%d", num), SCREEN_WIDTH-40, cell.j * CELL_HEIGHT, 25, WHITE); // Y
    DrawRectangleLines(cell.i * CELL_WIDTH, cell.j * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, WHITE);
}

void GridDraw(int grid[COLS][ROWS])
{
    int i, j;
    for (i = 0; i < COLS; i++) {
        for (j = 0; j < ROWS; j++) {
            CellDraw((Cell){i, j}, j+1);
        }
    }
}

void CellDiffeDraw(int grid_diffe[COLS][ROWS], char is_fight_started)
{
    int x;
    int y;
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            if (grid_diffe[i][j] == 1 && !is_fight_started) {
                x = j * CELL_WIDTH;
                y = i * CELL_HEIGHT;
                DrawRectangle(x, y, CELL_WIDTH, CELL_HEIGHT, BLUE);
            } else if (grid_diffe[i][j] == 2) {
                x = j * CELL_WIDTH;
                y = i * CELL_HEIGHT;
                DrawRectangle(x, y, CELL_WIDTH, CELL_HEIGHT, RED);
            }
            else if (grid_diffe[i][j] == 3) {
                x = j * CELL_WIDTH;
                y = i * CELL_HEIGHT;
                DrawRectangle(x, y, CELL_WIDTH, CELL_HEIGHT, YELLOW);
            }
        }
    }
}

void SetSubmarine(int grid_diffe[COLS][ROWS], int *max_size_submarine, const char is_vertical, const int temp, int *amount_of_submarines, int *j, int *turn_indication, Sound click_sound, char is_fight_started)
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mPos = GetMousePosition();
        int x = mPos.x / CELL_WIDTH;
        int y = mPos.y / CELL_HEIGHT;

        if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
            if (IsSubmarinePositionOk(x, y, grid_diffe, is_vertical, &amount_of_submarines[*j]) && *max_size_submarine && (*j <= temp)) {
                PlaySound(click_sound);
                for (int i = 0; i < amount_of_submarines[*j]; i++) {
                    is_vertical ? grid_diffe[y+i][x] = 1 : (grid_diffe[y][x+i] = 1);
                    printf("X: %d, Y: %d\n", x, y);
                }
                (*turn_indication)++;
                (*j)++;
            }
        }
    }
    CellDiffeDraw(grid_diffe, is_fight_started);
}

char IsSubmarinePositionOk(int x, int y, int grid_diffe[COLS][ROWS], char is_vertical, int *max_size_submarine)
{
    int is_cell_ok = 0;
    for (int i = 0; i < *max_size_submarine; i++) {
        if (is_vertical) {
            if (
                (
                    (
                        y == 0                                  &&
                        x == 0                                  &&
                        !grid_diffe[y+i][x+1]                   &&
                        !grid_diffe[y+i][x]                     &&
                        !grid_diffe[y+*max_size_submarine][x]
                    )
                    ||
                    (
                        y == 24                                 &&
                        x == 0                                  &&
                        !grid_diffe[y-i][x+1]                   &&
                        !grid_diffe[y-i][x]
                    )
                    ||
                    (
                        y == 24                                 &&
                        x == 24                                 &&
                        !grid_diffe[y-i][x-1]                   &&
                        !grid_diffe[y-i][x]
                    )
                    ||
                    (
                        y == 0                                  &&
                        !grid_diffe[y+i][x]                     &&
                        !grid_diffe[y+i][x-1]                   &&
                        !grid_diffe[y+i][x+1]                   &&
                        !grid_diffe[y+*max_size_submarine][x]   &&
                        !grid_diffe[y+*max_size_submarine][x-1] &&
                        !grid_diffe[y+*max_size_submarine][x+1]
                    )
                    ||
                    (
                        y == 24                                 &&
                        !grid_diffe[y-i][x-1]                   &&
                        !grid_diffe[y-i][x+1]                   &&
                        !grid_diffe[y-i][x]                     &&
                        !grid_diffe[y-*max_size_submarine][x]   &&
                        !grid_diffe[y-*max_size_submarine][x+1] &&
                        !grid_diffe[y-*max_size_submarine][x-1]
                    )
                    ||
                    (
                        x == 0                                  &&
                        !grid_diffe[y+i][x]                     &&
                        !grid_diffe[y-1][x+1]                   &&
                        !grid_diffe[y-1][x]                     &&
                        !grid_diffe[y+i][x+1]                   &&
                        !grid_diffe[y+*max_size_submarine][x+1] &&
                        !grid_diffe[y+*max_size_submarine][x]
                    )
                    ||
                    (
                        x == 24                                 &&
                        !grid_diffe[y+i][x]                     &&
                        !grid_diffe[y-1][x]                     &&
                        !grid_diffe[y-1][x-1]                   &&
                        !grid_diffe[y+i][x-1]                   &&
                        !grid_diffe[y+*max_size_submarine][x-1] &&
                        !grid_diffe[y+*max_size_submarine][x]
                    )
                    ||
                    (
                        !grid_diffe[y+i][x]                     &&
                        !grid_diffe[y+i][x+1]                   &&
                        !grid_diffe[y+i][x-1]                   &&
                        !grid_diffe[y-1][x-1]                   &&
                        !grid_diffe[y-1][x]                     &&
                        !grid_diffe[y-1][x+1]                   &&
                        !grid_diffe[y+*max_size_submarine][x-1] &&
                        !grid_diffe[y+*max_size_submarine][x]   &&
                        !grid_diffe[y+*max_size_submarine][x+1]
                    )
                )
            ) is_cell_ok++;
        } else {
            if (
                (
                    (
                        y == 0                                  &&
                        x == 0                                  &&
                        !grid_diffe[y+1][x+i]                   &&
                        !grid_diffe[y][x+i]                     &&
                        !grid_diffe[y][x+*max_size_submarine]
                    )
                    ||
                    (
                        y == 24                                 &&
                        x == 0                                  &&
                        !grid_diffe[y+1][x-i]                   &&
                        !grid_diffe[y][x-i]
                    )
                    ||
                    (
                        y == 24                                 &&
                        x == 24                                 &&
                        !grid_diffe[y-1][x-i]                   &&
                        !grid_diffe[y][x-i]
                    )
                    ||
                    (
                        y == 0                                  &&
                        !grid_diffe[y][x+i]                     &&
                        !grid_diffe[y+1][x+i]                   &&
                        !grid_diffe[y][x+*max_size_submarine]   &&
                        !grid_diffe[y+1][x+*max_size_submarine]
                    )
                    ||
                    (
                        y == 24                                 &&
                        !grid_diffe[y-1][x-i]                   &&
                        !grid_diffe[y+1][x-i]                   &&
                        !grid_diffe[y][x-i]                     &&
                        !grid_diffe[y][x-*max_size_submarine]   &&
                        !grid_diffe[y+1][x-*max_size_submarine] &&
                        !grid_diffe[y-1][x-*max_size_submarine]
                    )
                    ||
                    (
                        x == 0                                  &&
                        !grid_diffe[y][x+i]                     &&
                        !grid_diffe[y+1][x-1]                   &&
                        !grid_diffe[y][x-1]                     &&
                        !grid_diffe[y+1][x+i]                   &&
                        !grid_diffe[y+1][x+*max_size_submarine] &&
                        !grid_diffe[y][x+*max_size_submarine]
                    )
                    ||
                    (
                        x == 24                                 &&
                        !grid_diffe[y][x+i]                     &&
                        !grid_diffe[y][x-1]                     &&
                        !grid_diffe[y-1][x-1]                   &&
                        !grid_diffe[y-1][x+i]                   &&
                        !grid_diffe[y-1][x+*max_size_submarine] &&
                        !grid_diffe[y][x+*max_size_submarine]
                    )
                    ||
                    (
                        !grid_diffe[y][x+i]                     &&
                        !grid_diffe[y+1][x+i]                   &&
                        !grid_diffe[y-1][x+i]                   &&
                        !grid_diffe[y-1][x-1]                   &&
                        !grid_diffe[y][x-1]                     &&
                        !grid_diffe[y+1][x-1]                   &&
                        !grid_diffe[y-1][x+*max_size_submarine] &&
                        !grid_diffe[y][x+*max_size_submarine]   &&
                        !grid_diffe[y+1][x+*max_size_submarine]
                    )
                )
            ) is_cell_ok++;
        }
    }

    char vertical_or_horizontal_checking =
        is_cell_ok == *max_size_submarine &&
        (
            is_vertical ?
                y + *max_size_submarine <= COLS
                :
                x + *max_size_submarine <= ROWS
        );

    if (vertical_or_horizontal_checking) return 1;
    return 0;
}

void DrawScore(int score_a, int score_b)
{
    DrawText(TextFormat("Player A: %d", score_a), 10, SCREEN_HEIGHT-200, 25, BEIGE);
    DrawText(TextFormat("Player b: %d", score_b), 10, SCREEN_HEIGHT-175, 25, BEIGE);
}

void ChooseDirection(int is_vertical)
{
    DrawText(TextFormat("You can choose the direction of the submarine.\nSo if you want vertical click on arrow UP in the keyborad, but if you want horizontal, so click on arrow LEFT."), 10, SCREEN_HEIGHT-120, 22, WHITE);
    DrawText(TextFormat("The direction is: %s", is_vertical ? "vertical": "horizontal"), 10, SCREEN_HEIGHT-40, 25, RED);
}

void ChangeDirection(char *is_vertical)
{
    if (IsKeyDown(KEY_UP)) *is_vertical = 1;
    else if (IsKeyDown(KEY_LEFT)) *is_vertical = 0;
}

void ShootOnSubmarine(int grid_diffe[COLS][ROWS], char *flip_turn, Sound shoot_sound)
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mPos = GetMousePosition();
        int x = mPos.x / CELL_WIDTH;
        int y = mPos.y / CELL_HEIGHT;

        if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
            if (grid_diffe[y][x] == 1) {
                grid_diffe[y][x] = 2;
            }
            else if (!grid_diffe[y][x]) {
                grid_diffe[y][x] = 3;
            }
            PlaySound(shoot_sound);
            *flip_turn ? *flip_turn = 0 : (*flip_turn = 1);
        }
    }
}

void CheckForWin(int player_1[COLS][ROWS], int player_2[COLS][ROWS], Sound game_over_sound, char *game_not_over)
{
    int there_is_winner = 0, the_winner_is, check_player_1 = 0, check_player_2 = 0;
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            player_1[i][j] != 1 ? check_player_1++ : 0;
            player_2[i][j] != 1 ? check_player_2++ : 0;
        }
    }
    int sum_of_cells = COLS * ROWS;
    if (check_player_1 == sum_of_cells) {
        there_is_winner = 1;
        the_winner_is = 1;
        *game_not_over = 0;
    }
    if (check_player_2 == sum_of_cells) {
        there_is_winner = 1;
        the_winner_is = 0;
        *game_not_over = 0;
    }

    if (there_is_winner) {
        char *win_msg = the_winner_is ? "Player 2 is the winner!!" : "Player 1 is the winner!!";
        char *lose_msg = the_winner_is ? "Player 1 you are a loser!!" : "Player 2 you are a loser!!";
        char font_size = 50;
        Vector2 win_msg_length = MeasureTextEx(GetFontDefault(), win_msg, font_size, 1.0f);
        Vector2 lose_msg_length = MeasureTextEx(GetFontDefault(), lose_msg, font_size, 1.0f);
        DrawText(win_msg, (SCREEN_WIDTH-win_msg_length.x)/2.5f, (SCREEN_HEIGHT-win_msg_length.y)/2.5f, font_size, RED);
        DrawText(lose_msg, (SCREEN_WIDTH-lose_msg_length.x)/2.5f, (SCREEN_HEIGHT-lose_msg_length.y)/2, font_size, RED);
        PlaySound(game_over_sound);
    }
}
// TODO: Check all that text about the turns.
void DrawWhoPlay(char which_player_is)
{
    char *msg = which_player_is ? "The board is of Player 1" : "The board is of Player 2";
    DrawText(msg, (SCREEN_WIDTH-(TextLength(msg))) / 3, SCREEN_HEIGHT-40, 25, BEIGE);
}


void DrawWelcomeMessage(char *welcome_message)
{
    char *msg_1 = "Welcome to the Battle-Ship Game";
    char *msg_2 = "You can click on KEY ARROW LEFT and KEY ARROW UP to change the direction of the submarine,\n\
    and you can click with the mouse on any cell that you want to place the submarine.\n\
    And when the fight is begin, you can click on KEY SPACE to see your board.";
    char *msg_3 = "Please press Enter to continue";
    char *msg_4 = "[-_-]";

    if (IsKeyDown(KEY_ENTER)) *welcome_message = 0;

    char font_size= 25;
    Vector2 msg_1_length = MeasureTextEx(GetFontDefault(), msg_1, font_size*2, 1.0f);
    Vector2 msg_2_length = MeasureTextEx(GetFontDefault(), msg_2, font_size, 1.0f);
    Vector2 msg_3_length = MeasureTextEx(GetFontDefault(), msg_3, font_size*1.2f, 1.0f);
    Vector2 msg_4_length = MeasureTextEx(GetFontDefault(), msg_4, font_size*2.5f, 1.0f);
    DrawText(msg_1, (SCREEN_WIDTH-msg_1_length.x)/2.5f, (SCREEN_HEIGHT)/4, font_size*2, GOLD);
    DrawText(msg_4, (SCREEN_WIDTH-msg_4_length.x)/2, (SCREEN_HEIGHT)/3, font_size*2.5f, GOLD);
    DrawText(msg_2, (SCREEN_WIDTH-msg_2_length.x)/2.5f, (SCREEN_HEIGHT)/2, font_size, GOLD);
    DrawText(msg_3, (SCREEN_WIDTH-msg_3_length.x)/2.5f, (SCREEN_HEIGHT)/1.5f, font_size*1.2f, CLITERAL(Color){ 85, 52, 43, 255 });
}
