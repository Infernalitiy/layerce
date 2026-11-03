/* The REAL game.c is linked here; not a second implementation/model. */
#include "../src/game.h"
#include <stdio.h>
#include <string.h>
#ifdef __TICE__
#include <ti/screen.h>
#include <ti/getcsc.h>
#endif
static Game g;
static unsigned failures,checks;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; \
    printf("FAIL line %u: %s\n",(unsigned)__LINE__,#expr); } } while (0)
static void counts(void) {
    unsigned i,b=0,d=0,t=0;
    for(i=0;i<MAX_BLOONS;++i) if(g.bloons[i].layer) ++b;
    for(i=0;i<MAX_DARTS;++i) if(g.darts[i].ttl) ++d;
    for(i=0;i<MAX_TOWERS;++i) if(g.towers[i].active) ++t;
    CHECK(b==g.bloon_count); CHECK(d==g.dart_count); CHECK(t==g.tower_count);
    CHECK(g.stats.spawned==g.stats.destroyed+g.stats.escaped+g.bloon_count);
}
static void placement(void) {
    int t;
    game_init(&g);
    CHECK(!game_can_place(&g,64,100));
    CHECK(!game_can_place(&g,0,20));
    CHECK(game_place(&g,64,100)==PLACE_BLOCKED); CHECK(g.money==500);
    t=game_place(&g,96,120); CHECK(t>=0); CHECK(g.money==320);
    CHECK(game_place(&g,96,120)==PLACE_BLOCKED); CHECK(g.money==320);
    CHECK(game_tower_at(&g,97,120)==t);
    CHECK(game_sell(&g,t)); CHECK(g.money==446); CHECK(!game_sell(&g,t));
    g.money=0; CHECK(game_place(&g,96,120)==PLACE_POOR); CHECK(g.money==0);
    counts();
}
static void layers_and_leaks(void) {
    unsigned i;
    game_init(&g);
    CHECK(game_spawn(&g,5,0));
    for(i=0;i<4;++i) CHECK(game_damage(&g,0));
    CHECK(g.bloons[0].layer==1); CHECK(g.money==504);
    CHECK(game_damage(&g,0)); CHECK(!game_damage(&g,0));
    CHECK(g.stats.pops==5 && g.stats.destroyed==1 && g.money==505);
    CHECK(!game_spawn(&g,6,0)); CHECK(!game_spawn(&g,0,0));
    CHECK(!game_spawn(&g,1,game_path_length()));
    game_init(&g); game_start_wave(&g);
    /* Disable scheduled spawns; controlled five-layer escape must clamp at 0. */
    g.group=waves[0].first+waves[0].groups;
    g.lives=2; CHECK(game_spawn(&g,5,game_path_length()-1));
    game_tick(&g); CHECK(g.lives==0 && !g.running);
    CHECK(g.stats.escaped==1 && g.bloon_count==0); counts();
}
static void targets(void) {
    Tower t={40,40,0,TARGET_FIRST,1};
    game_init(&g);
    game_spawn(&g,1,16*10); game_spawn(&g,4,16*20); game_spawn(&g,2,16*40);
    CHECK(game_find_target(&g,&t)==2);
    t.mode=TARGET_LAST; CHECK(game_find_target(&g,&t)==0);
    t.mode=TARGET_STRONG; CHECK(game_find_target(&g,&t)==1);
    t.x=240; CHECK(game_find_target(&g,&t)==-1); counts();
}
static void capacity_and_cadence(void) {
    unsigned i;
    uint8_t remaining;
    game_init(&g); CHECK(game_start_wave(&g)); CHECK(!game_start_wave(&g));
    for(i=0;i<MAX_BLOONS;++i) CHECK(game_spawn(&g,1,0));
    CHECK(!game_spawn(&g,1,0)); remaining=g.remaining;
    game_tick(&g); CHECK(g.remaining==remaining); CHECK(g.stats.spawn_waits==1);
    game_damage(&g,0); game_tick(&g);
    CHECK(g.remaining==remaining-1); CHECK(g.bloon_count==MAX_BLOONS); counts();
    game_init(&g); game_start_wave(&g); game_tick(&g);
    CHECK(g.stats.spawned==1);
    for(i=0;i<17;++i) game_tick(&g);
    CHECK(g.stats.spawned==1); game_tick(&g); CHECK(g.stats.spawned==2);
}
static void darts_and_round_end(void) {
    unsigned i;
    game_init(&g); game_place(&g,30,38); game_start_wave(&g);
    for(i=0;i<250;++i) game_tick(&g);
    CHECK(g.stats.pops>0); counts();
    /* Force an exhausted scheduler and empty map: award bonus exactly once. */
    game_init(&g); game_start_wave(&g);
    g.group=waves[0].first+waves[0].groups;
    game_tick(&g); CHECK(g.wave==1 && !g.running && g.money==580);
    game_tick(&g); CHECK(g.money==580); CHECK(g.stats.ticks==1);
    /* Saturate dart slots; tower must retry without consuming its cooldown. */
    game_init(&g); game_place(&g,30,38); game_start_wave(&g);
    for(i=0;i<MAX_DARTS;++i) { g.darts[i].ttl=10; g.darts[i].x=200*FP; g.darts[i].y=30*FP; }
    g.dart_count=MAX_DARTS; game_tick(&g);
    CHECK(g.stats.shot_waits==1 && g.towers[0].cooldown==0); counts();
}
static void path_and_campaign(void) {
    int16_t x,y;
    unsigned i,w;
    uint32_t expected=0;
    CHECK(game_path_length()==520*FP);
    game_path_position(64*FP,&x,&y); CHECK(x==64*FP && y==60*FP);
    game_path_position(164*FP,&x,&y); CHECK(x==64*FP && y==160*FP);
    game_path_position(520*FP,&x,&y); CHECK(x==256*FP && y==188*FP);
    /* No defenses, high lives: every scripted enemy must spawn and escape. */
    game_init(&g); g.lives=60000;
    for(w=0;w<WAVE_COUNT;++w) {
        CHECK(game_start_wave(&g));
        for(i=0;i<20000 && g.running;++i) game_tick(&g);
        CHECK(!g.running && g.wave==w+1); counts();
        for(i=waves[w].first;i<waves[w].first+waves[w].groups;++i)
            expected+=wave_groups[i].count;
    }
    CHECK(g.won); CHECK(g.stats.spawned==expected); CHECK(g.stats.escaped==expected);
    CHECK(!game_start_wave(&g));
    game_stress(&g);
    for(i=0;i<900;++i) { game_tick(&g); game_stress_refill(&g); }
    CHECK(g.bloon_count==MAX_BLOONS && g.tower_count==MAX_TOWERS); counts();
}
int main(void) {
#ifdef __TICE__
    os_ClrHome();
#endif
    placement(); layers_and_leaks(); targets(); capacity_and_cadence();
    darts_and_round_end(); path_and_campaign();
    printf("%u checks, %u failures\n",checks,failures);
    printf("Game=%u B, B/D/T=%u/%u/%u\n",(unsigned)sizeof(Game),
        (unsigned)sizeof(Bloon),(unsigned)sizeof(Dart),(unsigned)sizeof(Tower));
#ifdef __TICE__
    printf("Press a key to exit\n");
    while(!os_GetCSC()) {}
#endif
    return failures?1:0;
}
