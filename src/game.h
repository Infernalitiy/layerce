#ifndef LAYERCE_GAME_H
#define LAYERCE_GAME_H
#include <stdbool.h>
#include <stdint.h>

/* Portable deterministic simulation; all positions use 1/16 pixel units. */
#define SIM_HZ 30
#define MAX_BLOONS 96
#define MAX_DARTS 64
#define MAX_TOWERS 24
#define TOWER_COST 180
#define TOWER_REFUND 126
#define TOWER_RANGE 58
#define TOWER_PERIOD 12
#define WAVE_COUNT 8
#define PATH_POINTS 8
#define FP 16

typedef enum { TARGET_FIRST, TARGET_LAST, TARGET_STRONG, TARGET_COUNT } TargetMode;
typedef enum { PLACE_BLOCKED=-1, PLACE_POOR=-2, PLACE_FULL=-3 } PlaceError;
typedef struct { uint16_t x; uint8_t y; } Point;
typedef struct { uint16_t distance; int16_t x,y; uint8_t layer; } Bloon;
typedef struct { int16_t x,y,vx,vy; uint8_t ttl; } Dart;
typedef struct { uint16_t x; uint8_t y,cooldown,mode,active; } Tower;
typedef struct { uint8_t layer,count,gap; } WaveGroup;
typedef struct { uint8_t first,groups; uint16_t bonus; } Wave;
typedef struct {
    uint32_t ticks,spawned,destroyed,escaped,pops,spawn_waits,shot_waits;
    uint16_t peak_bloons,peak_darts;
} GameStats;
typedef struct {
    Bloon bloons[MAX_BLOONS];
    Dart darts[MAX_DARTS];
    Tower towers[MAX_TOWERS];
    GameStats stats;
    uint16_t money,lives;
    uint8_t wave,running,won,bloon_count,dart_count,tower_count;
    uint8_t group,remaining,spawn_timer;
} Game;

extern const Point path[PATH_POINTS];
extern const WaveGroup wave_groups[];
extern const Wave waves[WAVE_COUNT];
extern const uint8_t layer_speed[6];

void game_init(Game *g);
uint16_t game_path_length(void);
void game_path_position(uint16_t distance, int16_t *x, int16_t *y);
bool game_can_place(const Game *g, int x, int y);
int game_place(Game *g, int x, int y);
int game_tower_at(const Game *g, int x, int y);
bool game_sell(Game *g, int index);
bool game_start_wave(Game *g);
void game_tick(Game *g);
int game_find_target(const Game *g, const Tower *tower);
/* Also used by tests and the isolated stress scenario. Returns false at cap. */
bool game_spawn(Game *g, uint8_t layer, uint16_t distance);
bool game_damage(Game *g, unsigned index);
void game_stress(Game *g);
void game_stress_refill(Game *g);
#endif
