#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "raylib.h"

#define BOARD_SIZE 8
#define TILE_SIZE 42
#define TILE_TYPES 5
#define SCORE_FONT_SIZE 32

const char tile_chars[TILE_TYPES] = {'#', '@', '$', '%', '&'};

char board[BOARD_SIZE][BOARD_SIZE];
int score = 200;
bool matched[BOARD_SIZE][BOARD_SIZE] = {0};
float fall_offset[BOARD_SIZE][BOARD_SIZE] = {0};
float fall_speed = 8.0f;

Vector2 grid_origin;
Texture2D background;
Font score_font;
Vector2 selected_tile = {-1, -1};

char random_tile() {
    return tile_chars[rand() % TILE_TYPES];
}

void swap_tiles(int x1, int y1, int x2, int y2) {
    char temp = board[y1][x1];

    board[y1][x1] = board[y2][x2];
    board[y2][x2] = temp;
}

bool are_tiles_adjacent(Vector2 a, Vector2 b) {
    return(abs((int)a.x - (int)b.x) + abs((int)a.y - (int)b.y)) == 1;
}

bool find_matches() {
    bool found = false;

    for(int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            matched[y][x] = false;
        }
    }

    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE - 2; ++x) {
            char t = board[y][x];

            if(t == board[y][x + 1] && t == board[y][x + 2]) {
                matched[y][x] = matched[y][x + 1] = matched[y][x + 2] = true;

                score += 10;

                found = true;
            }
        }
    }

    for(int x = 0; x < BOARD_SIZE; ++x) {
        for(int y = 0; y < BOARD_SIZE - 2; ++y) {
            char t = board[y][x];

            if(t == board[y + 1][x] && (t == board[y + 2][x])) {
                matched[y][x] = matched[y + 1][x] = matched[y + 2][x] = true;

                score += 10;

                found = true;
            }
        }
    }

    return found;
}

void resolve_matches() {
    for(int x = 0; x < BOARD_SIZE; ++x) {
        int write_y = BOARD_SIZE - 1;

        for(int y = BOARD_SIZE - 1; y >= 0; --y) {
            if(!matched[y][x]) {
                if(y != write_y) {
                    board[write_y][x] = board[y][x];
                    fall_offset[write_y][x] = (write_y - y) * TILE_SIZE;
                    board[y][x] = ' ';
                }

                write_y--;
            }
        }

        while(write_y >= 0) {
            board[write_y][x] = random_tile();
            fall_offset[write_y][x] = (write_y + 1) * TILE_SIZE;

            write_y--;
        }
    }
}

void init_board() {
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            board[y][x] = random_tile();
        }
    }

    int grid_width = BOARD_SIZE * TILE_SIZE;
    int grid_height = BOARD_SIZE * TILE_SIZE;

    grid_origin = (Vector2){
        (GetScreenWidth() - grid_width) / 2
        , (GetScreenHeight() - grid_height) / 2
    };
}

int main(void) {
    const int screen_width = 800;
    const int screen_height = 400;

    InitWindow(screen_width, screen_height, "Raylib 2D ASCII MATCH 3");
    SetTargetFPS(60);
    srand(time(NULL));

    background = LoadTexture("cosmos.jpeg");
    score_font = LoadFontEx("RygaRegular.ttf", SCORE_FONT_SIZE, NULL, 0);

    init_board();

    Vector2 mouse = { 0, 0 };

    while(!WindowShouldClose()) {
        mouse = GetMousePosition();

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int x = (mouse.x - grid_origin.x) / TILE_SIZE;
            int y = (mouse.y - grid_origin.y) / TILE_SIZE;

            if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
                Vector2 current_tile = (Vector2) {x, y};

                if(selected_tile.x < 0) {
                    selected_tile = current_tile;
                }
                else {
                    if(are_tiles_adjacent(selected_tile, current_tile)) {
                        swap_tiles(
                            selected_tile.x
                            , selected_tile.y
                            , current_tile.x
                            , current_tile.y
                        );

                        if(find_matches()) {
                            resolve_matches();
                        }
                        else {
                            swap_tiles(
                                selected_tile.x
                                , selected_tile.y
                                , current_tile.x
                                , current_tile.y
                            );
                        }
                    }

                    selected_tile = (Vector2){ -1, -1 };
                }
            }
        }

        for(int y = 0; y < BOARD_SIZE; ++y) {
            for(int x = 0; x < BOARD_SIZE; ++x) {
                if(fall_offset[y][x] > 0) {
                    fall_offset[y][x] -= fall_speed;

                    if(fall_offset[y][x] < 0) {

                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexturePro(
            background
            , (Rectangle) {
                0
                , 0
                , background.width
                , background.height
            }
            , (Rectangle) {
                0
                , 0
                , GetScreenWidth()
                , GetScreenHeight()
            }
            , (Vector2) {0, 0}
            , 0.0f
            , WHITE
        );

        for(int y = 0; y < BOARD_SIZE; ++y) {
            for(int x = 0; x < BOARD_SIZE; ++x) {
                Rectangle rect = {
                    grid_origin.x +(x * TILE_SIZE)
                    , grid_origin.y + (y * TILE_SIZE)
                    , TILE_SIZE
                    , TILE_SIZE
                };

                DrawRectangleLinesEx(rect, 1, DARKGRAY);

                if(board[y][x] != ' ') {
                    DrawTextEx(
                        GetFontDefault()
                        , TextFormat("%c", board[y][x])
                        , (Vector2) {
                            rect.x + 12
                            , rect.y + 8 - fall_offset[y][x]
                        }
                        , 20
                        , 1
                        , matched[y][x] ? GREEN : WHITE
                    );
                }
            }
        }

        if(selected_tile.x >= 0) {
            DrawRectangleLinesEx(
                (Rectangle){
                    grid_origin.x + (selected_tile.x * TILE_SIZE)
                    , grid_origin.y + (selected_tile.y * TILE_SIZE)
                    , TILE_SIZE
                    , TILE_SIZE
                }
                , 2
                , YELLOW
            );
        }

        DrawTextEx(
            score_font
            , TextFormat("SCORE: %d", score)
            , (Vector2) {20, 20}
            , SCORE_FONT_SIZE
            , 1.0f
            , YELLOW
        );

        EndDrawing();
    }

    UnloadTexture(background);
    UnloadFont(score_font);

    CloseWindow();

    return 0;
}