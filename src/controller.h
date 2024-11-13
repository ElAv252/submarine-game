
#include "../raylib/src/raylib.h"


#define TERMINAL_YELLOW "\033[0;33m"

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

extern void CellDraw(Cell cell, int num);
extern void GridDraw(int cell[COLS][ROWS]);
extern void SetSubmarine(int grid_diffe[COLS][ROWS], int *max_size_submarine, const char is_vertical, const int temp, int *amount_of_submarines, int *j, int *turn_indication, Sound click_sound, char is_fight_started);
extern void CellDiffeDraw(int grid_diffe[COLS][ROWS], char is_fight_started, char *flip_turn);
extern char IsSubmarinePositionOk(int x, int y, int grid_diffe[COLS][ROWS], char is_vertical, int *max_size_submarine);
extern void DrawScore(int score_a, int score_b);
extern void ChooseDirection(int is_vertical);
extern void ChangeDirection(char *is_vertical);
extern void ShootOnSubmarine(int grid_diffe[COLS][ROWS], char *flip_turn, Sound shoot_sound, Sound click_sound);
extern void CheckForWin(int player_1[COLS][ROWS], int player_2[COLS][ROWS], Sound game_over_sound, char *game_not_over);
extern void DrawWhoPlay(char which_player_is);
extern void DrawWelcomeMessage(char *welcome_message);
extern void DrawPlayerGrid(int grid_diffe[COLS][ROWS]);
extern void DrawHit();
