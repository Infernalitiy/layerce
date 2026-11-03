#ifndef CORE_TEST
#include "game.h"
#include <graphx.h>
#include <keypadc.h>
#include <ti/vars.h>
#include <sys/timers.h>
#include <stdio.h>
#include <time.h>

/* No full-screen RAM arrays: GraphX uses its existing LCD VRAM pages. */
static Game game;
static int cursor_x=96,cursor_y=120,selected=-1;
static uint8_t paused=0,speed=1,view=1,bench=0;
static const char *notice="2nd: place/select";
static size_t startup_os_free;
static uint32_t debt,slow_events,bench_started;
static uint32_t fps,sps,sim_average,sim_max,draw_average,draw_max;
static uint32_t frames,steps,sim_sum,draw_sum,sim_peak,draw_peak;
static uint32_t sample_start;

enum { C_BG,C_GRASS,C_PATH_EDGE,C_PATH,C_PANEL,C_INK,C_WHITE,C_MUTED,
       C_RED,C_BLUE,C_GREEN,C_YELLOW,C_PINK,C_TOWER,C_ACCENT,C_DART };
static const uint8_t bloon_colors[6]={C_INK,C_RED,C_BLUE,C_GREEN,C_YELLOW,C_PINK};
static const char * const modes[TARGET_COUNT]={"FIRST","LAST","STRONG"};

static uint32_t now_ticks(void) { return (uint32_t)clock(); }
static void text(int x,int y,const char *s,uint8_t c) {
    gfx_SetTextFGColor(c); gfx_PrintStringXY(s,x,y);
}
static void number(int x,int y,const char *name,uint32_t value,uint8_t color) {
    char line[40];
    snprintf(line,sizeof(line),"%s%lu",name,(unsigned long)value);
    text(x,y,line,color);
}
static void palette(void) {
    static const uint8_t rgb[16][3]={
        {17,27,30},{64,102,76},{122,97,62},{204,181,127},
        {25,42,46},{9,19,22},{241,243,218},{156,183,173},
        {232,70,68},{59,146,224},{88,202,99},{251,210,70},
        {240,113,175},{157,102,56},{122,230,207},{245,239,199}
    };
    unsigned i;
    for (i=0;i<16;++i) gfx_palette[i]=gfx_RGBTo1555(rgb[i][0],rgb[i][1],rgb[i][2]);
    gfx_SetTextTransparentColor(255);
    gfx_SetTextBGColor(255);
}
static void path_draw(uint8_t color,int radius) {
    unsigned i;
    gfx_SetColor(color);
    for (i=0;i<PATH_POINTS-1;++i) {
        int x=path[i].x,y=path[i].y;
        int dx=(int)path[i+1].x-x,dy=(int)path[i+1].y-y;
        if (dx<0) { x+=dx; dx=-dx; }
        if (dy<0) { y+=dy; dy=-dy; }
        if (dx) gfx_FillRectangle(x-radius,y-radius,dx+radius*2+1,radius*2+1);
        else gfx_FillRectangle(x-radius,y-radius,radius*2+1,dy+radius*2+1);
        gfx_FillCircle(x,y,radius);
    }
    gfx_FillCircle(path[PATH_POINTS-1].x,path[PATH_POINTS-1].y,radius);
}
static void scene(void) {
    unsigned i;
    gfx_FillScreen(C_BG);
    gfx_SetColor(C_GRASS); gfx_FillRectangle(0,24,256,184);
    gfx_SetClipRegion(0,24,256,208);
    path_draw(C_PATH_EDGE,9); path_draw(C_PATH,7);
    if (selected>=0 && game.towers[selected].active) {
        gfx_SetColor(C_ACCENT);
        gfx_Circle(game.towers[selected].x,game.towers[selected].y,TOWER_RANGE);
    }
    for (i=0;i<MAX_TOWERS;++i) if (game.towers[i].active) {
        Tower *t=&game.towers[i];
        gfx_SetColor(C_INK); gfx_FillCircle(t->x,t->y,7);
        gfx_SetColor(C_TOWER); gfx_FillCircle(t->x,t->y,5);
        gfx_SetColor(C_DART); gfx_Line(t->x,t->y,t->x+8,t->y-4);
        gfx_SetColor(C_WHITE); gfx_SetPixel(t->x-2,t->y-2);
    }
    for (i=0;i<MAX_BLOONS;++i) if (game.bloons[i].layer) {
        Bloon *b=&game.bloons[i];
        int x=b->x/FP,y=b->y/FP;
        gfx_SetColor(C_INK); gfx_FillCircle(x,y,5);
        gfx_SetColor(bloon_colors[b->layer]); gfx_FillCircle(x,y,4);
        /* Avoid SetPixel's special screen-only clipping at path entrance. */
        if (x>=2 && x<254) { gfx_SetColor(C_WHITE); gfx_SetPixel(x-1,y-2); }
    }
    gfx_SetColor(C_DART);
    for (i=0;i<MAX_DARTS;++i) if (game.darts[i].ttl)
        gfx_FillCircle(game.darts[i].x/FP,game.darts[i].y/FP,1);
    gfx_SetColor(game_can_place(&game,cursor_x,cursor_y)?C_ACCENT:C_RED);
    gfx_Rectangle(cursor_x-7,cursor_y-7,15,15);
    gfx_Line(cursor_x-10,cursor_y,cursor_x-4,cursor_y);
    gfx_Line(cursor_x+4,cursor_y,cursor_x+10,cursor_y);
    gfx_SetClipRegion(0,0,320,240);
    gfx_SetColor(C_PANEL); gfx_FillRectangle(256,24,64,184);
    text(8,8,"LAYER / CE",C_ACCENT);
    number(154,8,"$",game.money,C_WHITE);
    number(248,8,"HP ",game.lives,C_WHITE);
    number(260,32,"RD ",game.wave<WAVE_COUNT?game.wave+1:WAVE_COUNT,C_WHITE);
    text(260,46,"/ 8",C_MUTED);
    text(260,66,"DART",C_ACCENT);
    text(260,80,"$180",C_WHITE);
    number(260,100,"B ",game.bloon_count,C_WHITE);
    number(260,114,"D ",game.dart_count,C_WHITE);
    text(260,136,speed==1?"1x":"2x",C_ACCENT);
    if (selected>=0 && game.towers[selected].active) {
        text(260,158,modes[game.towers[selected].mode],C_WHITE);
        text(260,174,"DEL",C_MUTED); text(260,188,"SELL",C_MUTED);
    }
    number(5,212,"FPS ",fps,C_MUTED);
    number(75,212,"SPS ",sps,C_MUTED);
    number(150,212,"Bpk ",game.stats.peak_bloons,C_MUTED);
    text(237,212,paused?"PAUSED":(game.running?"WAVE":"READY"),C_ACCENT);
    if (!game.lives) text(5,226,"DEFEAT. GRAPH: new / CLEAR: exit",C_RED);
    else if (game.won) text(5,226,"8 ROUNDS CLEAR. GRAPH: new",C_YELLOW);
    else if (bench==2) text(5,226,"BENCH DONE. TRACE: stats",C_YELLOW);
    else if (debt) text(5,226,"SLOW: retaining simulation steps",C_YELLOW);
    else text(5,226,notice,C_WHITE);
}
static void box(const char *title) {
    gfx_SetColor(C_INK); gfx_FillRectangle(12,27,296,181);
    gfx_SetColor(C_ACCENT); gfx_Rectangle(12,27,296,181);
    text(24,39,title,C_ACCENT);
}
static void help(void) {
    box("LAYER DEFENSE  v0.1");
    text(24,59,"Arrows      Move cursor",C_WHITE);
    text(24,73,"2nd/ENTER   Place or select",C_WHITE);
    text(24,87,"ZOOM        Start next round",C_WHITE);
    text(24,101,"MODE        Pause / resume",C_WHITE);
    text(24,115,"+           1x / 2x speed",C_WHITE);
    text(24,129,"ALPHA       Target mode",C_WHITE);
    text(24,143,"DEL         Sell selected",C_WHITE);
    text(24,157,"TRACE       Measurements",C_WHITE);
    text(24,171,"Y= help/back | CLEAR exit",C_WHITE);
    text(24,190,"2nd: play | WINDOW: stress*",C_YELLOW);
}
static void diagnostics(void) {
    char line[48];
    box("MEASUREMENTS (game suspended)");
    snprintf(line,sizeof(line),"FPS %lu  SPS %lu  clock %lu Hz",
        (unsigned long)fps,(unsigned long)sps,(unsigned long)CLOCKS_PER_SEC);
    text(24,59,line,C_WHITE);
    snprintf(line,sizeof(line),"SIM ticks avg %lu / max %lu",
        (unsigned long)sim_average,(unsigned long)sim_max);
    text(24,73,line,C_WHITE);
    snprintf(line,sizeof(line),"DRAW ticks avg %lu / max %lu",
        (unsigned long)draw_average,(unsigned long)draw_max);
    text(24,87,line,C_WHITE);
    snprintf(line,sizeof(line),"Game %u B  Bloon/Dart/Tower %u/%u/%u",
        (unsigned)sizeof(Game),(unsigned)sizeof(Bloon),(unsigned)sizeof(Dart),(unsigned)sizeof(Tower));
    text(24,101,line,C_WHITE);
    number(24,115,"OS free at launch (not heap) ",startup_os_free,C_MUTED);
    snprintf(line,sizeof(line),"Peaks B%u D%u  slow events %lu",
        (unsigned)game.stats.peak_bloons,(unsigned)game.stats.peak_darts,(unsigned long)slow_events);
    text(24,129,line,C_WHITE);
    snprintf(line,sizeof(line),"Pool waits B%lu D%lu",
        (unsigned long)game.stats.spawn_waits,(unsigned long)game.stats.shot_waits);
    text(24,143,line,C_WHITE);
    snprintf(line,sizeof(line),"Spawn %lu kill %lu leak %lu pop %lu",
        (unsigned long)game.stats.spawned,(unsigned long)game.stats.destroyed,
        (unsigned long)game.stats.escaped,(unsigned long)game.stats.pops);
    text(24,157,line,C_WHITE);
    text(24,175,"ms = clock ticks * 1000 / clock Hz",C_MUTED);
    text(24,190,"TRACE: back | CLEAR: exit",C_ACCENT);
}
static void reset_metrics(uint32_t now) {
    fps=sps=sim_average=draw_average=0;
    frames=steps=sim_sum=draw_sum=sim_peak=draw_peak=0;
    sim_max=draw_max=slow_events=debt=0; sample_start=now;
}
static void sample(uint32_t now) {
    uint32_t duration=now-sample_start;
    if (duration<(uint32_t)CLOCKS_PER_SEC) return;
    fps=frames*(uint32_t)CLOCKS_PER_SEC/duration;
    sps=steps*(uint32_t)CLOCKS_PER_SEC/duration;
    sim_average=steps?sim_sum/steps:0;
    draw_average=frames?draw_sum/frames:0;
    if (sim_peak>sim_max) sim_max=sim_peak;
    if (draw_peak>draw_max) draw_max=draw_peak;
    frames=steps=sim_sum=draw_sum=sim_peak=draw_peak=0; sample_start=now;
}
enum { K_PLACE=1,K_START=2,K_PAUSE=4,K_SPEED=8,K_TARGET=16,K_SELL=32,
       K_HELP=64,K_STATS=128,K_EXIT=256,K_STRESS=512,K_NEW=1024 };
static uint16_t controls(void) {
    uint16_t k=0;
    if (kb_IsDown(kb_Key2nd)||kb_IsDown(kb_KeyEnter)) k|=K_PLACE;
    if (kb_IsDown(kb_KeyZoom)) k|=K_START;
    if (kb_IsDown(kb_KeyMode)) k|=K_PAUSE;
    if (kb_IsDown(kb_KeyAdd)) k|=K_SPEED;
    if (kb_IsDown(kb_KeyAlpha)) k|=K_TARGET;
    if (kb_IsDown(kb_KeyDel)) k|=K_SELL;
    if (kb_IsDown(kb_KeyYequ)) k|=K_HELP;
    if (kb_IsDown(kb_KeyTrace)) k|=K_STATS;
    if (kb_IsDown(kb_KeyClear)) k|=K_EXIT;
    if (kb_IsDown(kb_KeyWindow)) k|=K_STRESS;
    if (kb_IsDown(kb_KeyGraph)) k|=K_NEW;
    return k;
}
int main(void) {
    uint8_t old_mode;
    uint16_t previous;
    uint32_t last,last_draw,last_move,accumulator=0;
    void *unused;
    game_init(&game);
    startup_os_free=os_MemChk(&unused);
    old_mode=kb_GetMode();
    kb_SetMode(MODE_3_CONTINUOUS);
    gfx_Begin(); palette(); gfx_SetDrawBuffer();
    last=last_draw=last_move=now_ticks(); reset_metrics(last);
    previous=controls(); /* Held launcher key must be released before action. */
    for (;;) {
        uint32_t now=now_ticks(),elapsed=now-last;
        uint16_t current=controls(),pressed=current & (uint16_t)~previous;
        uint8_t old_view=view;
        bool suspended;
        unsigned batch=0;
        previous=current; last=now;
        if (pressed & K_EXIT) break;
        if (pressed & K_HELP) view=view==1?0:1;
        if (pressed & K_STATS) view=view==2?0:2;
        if (view==1 && (pressed & K_PLACE)) view=0;
        else if (!view && (pressed & K_PLACE) && game.lives && !game.won && !bench) {
            int index=game_tower_at(&game,cursor_x,cursor_y);
            if (index>=0) { selected=index; notice="ALPHA target / DEL sell"; }
            else {
                index=game_place(&game,cursor_x,cursor_y);
                if (index>=0) { selected=index; notice="Dart placed. ZOOM: next wave"; }
                else if (index==PLACE_POOR) notice="Need $180 for a dart tower";
                else if (index==PLACE_FULL) notice="Tower limit: 24";
                else notice="Blocked: path, edge, or tower";
            }
        }
        if ((pressed & K_STRESS) && !game.running && !game.wave && !game.tower_count) {
            game_stress(&game); bench=1; bench_started=now; paused=0; speed=1; view=0;
            selected=-1; accumulator=0; reset_metrics(now);
            notice="STRESS 30s / TRACE stats";
        }
        if ((pressed & K_NEW) && (!game.lives || game.won || bench==2)) {
            game_init(&game); bench=0; paused=0; speed=1; view=1; selected=-1;
            accumulator=0; reset_metrics(now); notice="2nd: place/select";
        }
        if (!view) {
            if (pressed & K_PAUSE) paused=!paused;
            if (pressed & K_SPEED) speed=speed==1?2:1;
            if (!bench && (pressed & K_START)) game_start_wave(&game);
            if (!bench && selected>=0 && game.towers[selected].active) {
                if (pressed & K_TARGET) game.towers[selected].mode=
                    (game.towers[selected].mode+1)%TARGET_COUNT;
                if (pressed & K_SELL) { game_sell(&game,selected); selected=-1; notice="Sold: +$126"; }
            }
            if (now-last_move>=(uint32_t)CLOCKS_PER_SEC/60) {
                if (kb_IsDown(kb_KeyLeft)) cursor_x-=2;
                if (kb_IsDown(kb_KeyRight)) cursor_x+=2;
                if (kb_IsDown(kb_KeyUp)) cursor_y-=2;
                if (kb_IsDown(kb_KeyDown)) cursor_y+=2;
                if (cursor_x<8) cursor_x=8;
                if (cursor_x>247) cursor_x=247;
                if (cursor_y<33) cursor_y=33;
                if (cursor_y>198) cursor_y=198;
                last_move=now;
            }
        }
        if (old_view!=view) {
            frames=steps=sim_sum=draw_sum=sim_peak=draw_peak=0; sample_start=now;
            elapsed=0; accumulator=0;
        }
        suspended=paused || view || !game.running || bench==2;
        if (suspended) accumulator=0;
        else {
            /* Convert wall time to 30 Hz units. No fractional tick truncation. */
            if (elapsed>(uint32_t)CLOCKS_PER_SEC/4) {
                elapsed=(uint32_t)CLOCKS_PER_SEC/4; ++slow_events;
            }
            accumulator+=elapsed*SIM_HZ*speed;
            if (accumulator>8UL*(uint32_t)CLOCKS_PER_SEC) {
                accumulator=8UL*(uint32_t)CLOCKS_PER_SEC; ++slow_events;
            }
            while (accumulator>=(uint32_t)CLOCKS_PER_SEC && batch<4 && game.running) {
                uint32_t begin=now_ticks(),used;
                game_tick(&game);
                if (bench==1) game_stress_refill(&game);
                used=now_ticks()-begin; sim_sum+=used;
                if (used>sim_peak) sim_peak=used;
                ++steps; ++batch; accumulator-=(uint32_t)CLOCKS_PER_SEC;
            }
            /* Cap wall-time debt, never skip game ticks or move entities in jumps.
               Under sustained overload, game time slows; UI and counters expose it. */
        }
        debt=accumulator>=(uint32_t)CLOCKS_PER_SEC;
        if (bench==1 && now-bench_started>=30UL*(uint32_t)CLOCKS_PER_SEC) {
            bench=2; paused=1; view=2;
        }
        if (!view) sample(now);
        if (now-last_draw>=(uint32_t)CLOCKS_PER_SEC/30) {
            uint32_t begin=now_ticks(),used;
            scene();
            if (view==1) help();
            if (view==2) diagnostics();
            gfx_SwapDraw();
            used=now_ticks()-begin;
            if (!view) {
                ++frames; draw_sum+=used;
                if (used>draw_peak) draw_peak=used;
            } else sample_start=now;
            last_draw=now;
        }
    }
    gfx_End();
    kb_SetMode(old_mode);
    return 0;
}
#endif
