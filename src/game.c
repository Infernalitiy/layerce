#include "game.h"
#include <string.h>

const Point path[PATH_POINTS] = {
    {0,60},{64,60},{64,160},{160,160},
    {160,92},{236,92},{236,188},{256,188}
};
/* Q4 pixels per 30 Hz step: 22.5,33.75,45,71.25,90 pixels/sec. */
const uint8_t layer_speed[6] = {0,12,18,24,38,48};
const WaveGroup wave_groups[] = {
    {1,20,18},
    {1,12,12},{2,12,18},
    {2,24,12},
    {2,16,10},{3,16,16},
    {3,30,11},
    {3,20,10},{4,12,16},
    {4,32,10},
    {3,20,8},{4,24,10},{5,20,14}
};
const Wave waves[WAVE_COUNT] = {
    {0,1,80},{1,2,90},{3,1,100},{4,2,110},
    {6,1,120},{7,2,130},{9,1,140},{10,3,180}
};

static int iabs(int a) { return a < 0 ? -a : a; }
static int clamp(int v,int a,int b) { return v<a ? a : (v>b ? b : v); }
static uint16_t segment_length(unsigned s) {
    return (uint16_t)(iabs((int)path[s+1].x-path[s].x) +
                      iabs((int)path[s+1].y-path[s].y));
}
uint16_t game_path_length(void) {
    uint16_t length=0;
    unsigned i;
    for (i=0;i<PATH_POINTS-1;++i) length+=segment_length(i)*FP;
    return length;
}
void game_path_position(uint16_t d,int16_t *x,int16_t *y) {
    unsigned s;
    for (s=0;s<PATH_POINTS-1;++s) {
        uint16_t length=segment_length(s)*FP;
        if (d<=length) {
            *x=(int16_t)(path[s].x*FP);
            *y=(int16_t)(path[s].y*FP);
            if (path[s+1].x>path[s].x) *x+=d;
            if (path[s+1].x<path[s].x) *x-=d;
            if (path[s+1].y>path[s].y) *y+=d;
            if (path[s+1].y<path[s].y) *y-=d;
            return;
        }
        d-=length;
    }
    *x=path[PATH_POINTS-1].x*FP;
    *y=path[PATH_POINTS-1].y*FP;
}
void game_init(Game *g) {
    memset(g,0,sizeof(*g));
    g->money=500;
    g->lives=100;
}
bool game_can_place(const Game *g,int x,int y) {
    unsigned i;
    if (x<8 || x>247 || y<33 || y>198) return false;
    /* Orthogonal segments: exact closest point, circular tower footprint. */
    for (i=0;i<PATH_POINTS-1;++i) {
        int ax=path[i].x,bx=path[i+1].x;
        int ay=path[i].y,by=path[i+1].y;
        int dx=x-clamp(x,ax<bx?ax:bx,ax>bx?ax:bx);
        int dy=y-clamp(y,ay<by?ay:by,ay>by?ay:by);
        if (dx*dx+dy*dy<15*15) return false;
    }
    for (i=0;i<MAX_TOWERS;++i) if (g->towers[i].active) {
        int dx=x-g->towers[i].x,dy=y-g->towers[i].y;
        if (dx*dx+dy*dy<15*15) return false;
    }
    return true;
}
int game_place(Game *g,int x,int y) {
    unsigned i;
    if (!game_can_place(g,x,y)) return PLACE_BLOCKED;
    if (g->money<TOWER_COST) return PLACE_POOR;
    for (i=0;i<MAX_TOWERS;++i) if (!g->towers[i].active) {
        Tower *t=&g->towers[i];
        t->x=(uint16_t)x; t->y=(uint8_t)y;
        t->cooldown=0; t->mode=TARGET_FIRST; t->active=1;
        g->money-=TOWER_COST; ++g->tower_count;
        return (int)i;
    }
    return PLACE_FULL;
}
int game_tower_at(const Game *g,int x,int y) {
    unsigned i;
    int found=-1,best=101;
    for (i=0;i<MAX_TOWERS;++i) if (g->towers[i].active) {
        int dx=x-g->towers[i].x,dy=y-g->towers[i].y;
        int d=dx*dx+dy*dy;
        if (d<best) { best=d; found=(int)i; }
    }
    return found;
}
bool game_sell(Game *g,int index) {
    if (index<0 || index>=MAX_TOWERS || !g->towers[index].active) return false;
    g->towers[index].active=0;
    --g->tower_count;
    g->money+=TOWER_REFUND;
    return true;
}
bool game_spawn(Game *g,uint8_t layer,uint16_t distance) {
    unsigned i;
    if (!layer || layer>5 || distance>=game_path_length()) return false;
    for (i=0;i<MAX_BLOONS;++i) if (!g->bloons[i].layer) {
        Bloon *b=&g->bloons[i];
        b->distance=distance; b->layer=layer;
        game_path_position(distance,&b->x,&b->y);
        ++g->bloon_count; ++g->stats.spawned;
        if (g->bloon_count>g->stats.peak_bloons) g->stats.peak_bloons=g->bloon_count;
        return true;
    }
    return false;
}
bool game_damage(Game *g,unsigned index) {
    Bloon *b;
    if (index>=MAX_BLOONS || !g->bloons[index].layer) return false;
    b=&g->bloons[index];
    --b->layer; ++g->money; ++g->stats.pops;
    if (!b->layer) { --g->bloon_count; ++g->stats.destroyed; }
    return true;
}
bool game_start_wave(Game *g) {
    if (g->running || g->won || !g->lives || g->wave>=WAVE_COUNT) return false;
    g->group=waves[g->wave].first;
    g->remaining=wave_groups[g->group].count;
    g->spawn_timer=0;
    g->running=1;
    return true;
}
int game_find_target(const Game *g,const Tower *t) {
    int best=-1;
    unsigned i;
    for (i=0;i<MAX_BLOONS;++i) if (g->bloons[i].layer) {
        const Bloon *b=&g->bloons[i];
        int dx=b->x/FP-t->x,dy=b->y/FP-t->y;
        bool better=false;
        if (iabs(dx)>TOWER_RANGE || iabs(dy)>TOWER_RANGE ||
            dx*dx+dy*dy>TOWER_RANGE*TOWER_RANGE) continue;
        if (best<0) better=true;
        else {
            const Bloon *old=&g->bloons[best];
            if (t->mode==TARGET_LAST) better=b->distance<old->distance;
            else if (t->mode==TARGET_STRONG) {
                better=b->layer>old->layer ||
                    (b->layer==old->layer && b->distance>old->distance);
            } else better=b->distance>old->distance;
        }
        if (better) best=(int)i;
    }
    return best;
}
static bool fire(Game *g,const Tower *t,const Bloon *b) {
    unsigned i;
    for (i=0;i<MAX_DARTS;++i) if (!g->darts[i].ttl) {
        Dart *d=&g->darts[i];
        int dx=b->x/FP-t->x,dy=b->y/FP-t->y;
        int divisor=iabs(dx)>iabs(dy)?iabs(dx):iabs(dy);
        if (!divisor) divisor=1;
        d->x=(int16_t)(t->x*FP); d->y=(int16_t)(t->y*FP);
        /* Straight, non-homing shot; diagonal speed is intentionally approximate. */
        d->vx=(int16_t)(dx*112/divisor); d->vy=(int16_t)(dy*112/divisor);
        d->ttl=14;
        ++g->dart_count;
        if (g->dart_count>g->stats.peak_darts) g->stats.peak_darts=g->dart_count;
        return true;
    }
    ++g->stats.shot_waits;
    return false;
}
static void spawn_tick(Game *g) {
    const Wave *w=&waves[g->wave];
    if (g->group>=w->first+w->groups) return;
    if (g->spawn_timer) { --g->spawn_timer; if (g->spawn_timer) return; }
    if (!game_spawn(g,wave_groups[g->group].layer,0)) {
        ++g->stats.spawn_waits; /* Retry next step; never decrement remaining. */
        return;
    }
    g->spawn_timer=wave_groups[g->group].gap;
    if (--g->remaining==0) {
        ++g->group;
        if (g->group<w->first+w->groups) g->remaining=wave_groups[g->group].count;
    }
}
static void move_bloons(Game *g) {
    unsigned i;
    uint16_t end=game_path_length();
    for (i=0;i<MAX_BLOONS;++i) if (g->bloons[i].layer) {
        Bloon *b=&g->bloons[i];
        b->distance+=layer_speed[b->layer];
        if (b->distance>=end) {
            g->lives=g->lives>b->layer ? g->lives-b->layer : 0;
            b->layer=0; --g->bloon_count; ++g->stats.escaped;
        } else game_path_position(b->distance,&b->x,&b->y);
    }
}
static void move_darts(Game *g) {
    unsigned i,j,sub;
    for (i=0;i<MAX_DARTS;++i) if (g->darts[i].ttl) {
        Dart *d=&g->darts[i];
        /* Two sampled half-steps keep displacement below collision diameter. */
        for (sub=0;sub<2 && d->ttl;++sub) {
            d->x+=(sub ? d->vx-d->vx/2 : d->vx/2);
            d->y+=(sub ? d->vy-d->vy/2 : d->vy/2);
            if (d->x<0 || d->x>=256*FP || d->y<24*FP || d->y>=208*FP) {
                d->ttl=0; break;
            }
            for (j=0;j<MAX_BLOONS;++j) if (g->bloons[j].layer) {
                int dx=d->x/FP-g->bloons[j].x/FP;
                int dy=d->y/FP-g->bloons[j].y/FP;
                if (iabs(dx)<=6 && iabs(dy)<=6 && dx*dx+dy*dy<=36) {
                    game_damage(g,j); d->ttl=0; break;
                }
            }
        }
        if (d->ttl) --d->ttl;
        if (!d->ttl) --g->dart_count;
    }
}
void game_tick(Game *g) {
    unsigned i;
    const Wave *w;
    if (!g->running || !g->lives || g->won) return;
    ++g->stats.ticks;
    spawn_tick(g);
    move_bloons(g);
    if (!g->lives) { g->running=0; return; }
    for (i=0;i<MAX_TOWERS;++i) if (g->towers[i].active) {
        Tower *t=&g->towers[i];
        if (t->cooldown) --t->cooldown;
        if (!t->cooldown) {
            int target=game_find_target(g,t);
            if (target>=0 && fire(g,t,&g->bloons[target])) t->cooldown=TOWER_PERIOD;
        }
    }
    move_darts(g);
    w=&waves[g->wave];
    if (g->group>=w->first+w->groups && !g->bloon_count) {
        g->money+=w->bonus;
        g->running=0; ++g->wave;
        /* End-of-round projectiles have no remaining gameplay purpose. */
        memset(g->darts,0,sizeof(g->darts)); g->dart_count=0;
        if (g->wave==WAVE_COUNT) g->won=1;
    }
}
void game_stress(Game *g) {
    unsigned i;
    game_init(g);
    g->money=60000; g->lives=60000;
    /* Fixed synthetic layout, not an economy-balanced campaign. */
    for (i=0;i<MAX_TOWERS;++i) {
        Tower *t=&g->towers[i];
        t->x=(uint16_t)(16+(i%8)*30); t->y=(uint8_t)(40+(i/8)*65);
        t->active=1; t->mode=(uint8_t)(i%TARGET_COUNT);
    }
    g->tower_count=MAX_TOWERS;
    game_start_wave(g);
    for (i=0;i<MAX_BLOONS;++i)
        game_spawn(g,(uint8_t)(1+i%5),(uint16_t)(i*(game_path_length()-64)/MAX_BLOONS));
}
void game_stress_refill(Game *g) {
    while (g->bloon_count<MAX_BLOONS) {
        uint32_t n=g->stats.spawned;
        game_spawn(g,(uint8_t)(1+n%5),(uint16_t)((n%MAX_BLOONS)*
            (game_path_length()-64)/MAX_BLOONS));
    }
    if (!g->running) {
        if (g->won) { g->wave=0; g->won=0; }
        game_start_wave(g);
    }
}
