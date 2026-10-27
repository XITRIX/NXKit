#pragma once
// Scene data shared by the NXKit gallery, retained raster tests and Apple UIKit
// reference app. Keep geometry/material choices here, rather than in adapters.
enum GlassDemoID {
    GlassDemoClearFixed, GlassDemoRegularFixed, GlassDemoClearTinted, GlassDemoRegularTinted,
    GlassDemoClearMovable, GlassDemoRegularMovable, GlassDemoMorphing, GlassDemoTintedMorphing,
    GlassDemoResizing, GlassDemoInteractive, GlassDemoCorners, GlassDemoContrast26, GlassDemoTwoTints,
    GlassDemoClearButtons, GlassDemoTintedButtons, GlassDemoNavigationButtons, GlassDemoNavigationGrouped, GlassDemoCount
};
typedef struct {
    const char *title, *detail, *action;
    int clear, tinted, movable, grouped, interactive, legacy;
} GlassDemoDefinition;
static const GlassDemoDefinition GlassDemos[GlassDemoCount] = {
    {"Clear · fixed", "Clear capsules and panels over artwork. Their positions stay fixed.", "", 1,0,0,0,0,0},
    {"Regular · fixed", "Regular glass at three sizes. Compare body density, frost and edges.", "", 0,0,0,0,0,0},
    {"Clear · tinted", "Blue, coral and green clear glass. Compare color over different backdrops.", "", 1,1,0,0,0,0},
    {"Regular · tinted", "The same colors on regular glass, from a small capsule to a large panel.", "", 0,1,0,0,0,0},
    {"Clear · movable", "Drag any clear item across the backdrop. Items remain separate materials.", "Move", 1,0,1,0,1,0},
    {"Regular · movable", "Drag regular glass over light and dark content. Press feedback stays active.", "Move", 0,0,1,0,1,0},
    {"Morphing", "Drag the three items together or use Join / Separate. One container merges their surfaces.", "Join / Separate", 0,0,1,1,1,0},
    {"Tinted morphing", "Different tints share a merging surface. Move the items or replay the join.", "Join / Separate", 0,1,1,1,1,0},
    {"Resizing", "Grow and shrink the same retained glass panel. Inspect material changes during the animation.", "Grow / Shrink", 0,0,0,0,0,0},
    {"Press feedback", "Press and hold the fixed items to inspect inflation, stretch and the touch light.", "", 0,0,0,0,1,0},
    {"Corner curves", "Circular corners on the left, continuous corners on the right, at the same radius.", "", 0,0,0,0,0,0},
    {"iOS 26 contrast · opt-in", "Small regular glass opts into backdrop-driven appearance. Other demos keep this option off.", "", 0,0,0,0,0,1},
    {"Two tints · blue + coral", "Join two differently tinted items, then swap their order to inspect the color at the overlap.", "Join / Separate", 0,1,1,1,1,0},
    {"Interactive · clear buttons", "Fixed clear buttons in three shapes. Press, slide and release; tap to toggle the checkmark.", "", 1,0,0,0,1,0},
    {"Interactive · tinted buttons", "Fixed blue, coral and green buttons. Compare touch lighting and stretch without moving the controls.", "", 0,1,0,0,1,0},
    {"Navigation buttons · mock", "A button-placement example: Back on the leading edge, More and an emphasized + on the trailing edge. Tap to toggle a checkmark.", "", 0,0,0,0,1,0},
    {"Navigation buttons · container", "The same fixed buttons share one glass container. Press near the trailing pair to inspect their surfaces joining.", "", 0,0,0,1,1,0},
};
static inline int GlassDemoIsNavigationButtons(int demo) {
    return demo == GlassDemoNavigationButtons || demo == GlassDemoNavigationGrouped;
}
static inline int GlassDemoHasButtons(int demo) {
    return demo == GlassDemoClearButtons || demo == GlassDemoTintedButtons || GlassDemoIsNavigationButtons(demo);
}
static inline float GlassDemoContainerSpacing(int demo) { return demo == GlassDemoNavigationGrouped ? 8 : 24; }
typedef struct { float x,y,width,height,radius; int tint,circular; } GlassDemoElement;
static inline int GlassDemoElementCount(int demo) {
    return demo == GlassDemoResizing ? 1 : (demo == GlassDemoInteractive || demo == GlassDemoCorners || demo == GlassDemoTwoTints ? 2 : 3);
}
static inline GlassDemoElement GlassDemoElementAt(int demo, int index, float progress) {
    GlassDemoElement e[] = {{80,95,160,70,35,0,0},{330,100,260,170,52,0,0},{110,335,500,56,28,0,0}};
    GlassDemoElement item = e[index];
    if (GlassDemos[demo].movable && !GlassDemos[demo].grouped) {
        item.x += progress * (index == 1 ? -100 : 90);
        item.y += progress * (index == 1 ? 75 : 30);
    }
    if (GlassDemos[demo].grouped) {
        float x[] = {140,300,520}, w[] = {72,120,72}, target[] = {280,310,370};
        item = (GlassDemoElement){x[index]+(target[index]-x[index])*progress,184,w[index],72,36,0,0};
    }
    if (GlassDemos[demo].tinted) item.tint = index+1;
    if (demo == GlassDemoResizing) {
        item.width = 160+200*progress; item.height = 64+136*progress;
        item.x = 360-item.width*.5f; item.y = 220-item.height*.5f;
        item.radius = item.height*.42f < 52 ? item.height*.42f : 52;
    }
    if (demo == GlassDemoInteractive) item = (GlassDemoElement){160+index*250.0f,170,160,90,38,0,0};
    if (demo == GlassDemoCorners) item = (GlassDemoElement){100+index*300.0f,130,220,170,50,0,index==0};
    if (demo == GlassDemoContrast26) item = (GlassDemoElement){40+index*240.0f,180,160,64,32,0,0};
    if (demo == GlassDemoClearButtons || demo == GlassDemoTintedButtons) {
        float x[] = {234,302,426}, width[] = {52,112,60}, height[] = {52,52,72};
        item = (GlassDemoElement){x[index],184-(height[index]-52)*.5f,width[index],height[index],index==2 ? 22.0f : 26.0f,
                                 demo==GlassDemoTintedButtons ? index+1 : 0,0};
    }
    if (GlassDemoIsNavigationButtons(demo)) {
        float x[] = {20,584,648};
        item = (GlassDemoElement){x[index],24,52,52,26,index==2 ? 1 : 0,0};
    }
    return item;
}
// Fixed controls retain their point sizes on narrow previews instead of scaling
// their touch targets down with the artwork. Captures still use a 720x440 stage.
static inline GlassDemoElement GlassDemoElementAtSize(int demo, int index, float progress, float width, float height) {
    GlassDemoElement item=GlassDemoElementAt(demo,index,progress);
    if (GlassDemoIsNavigationButtons(demo)) {
        if (index>0) item.x+=width-720;
    } else if (GlassDemoHasButtons(demo)) {
        item.x+=(width-720)*.5f;
        item.y+=(height-440)*.5f;
    }
    return item;
}
enum GlassDemoSymbol { GlassDemoPlus, GlassDemoBack, GlassDemoMore, GlassDemoCheck };
static inline int GlassDemoSymbolForItem(int demo, int index) {
    if (GlassDemoIsNavigationButtons(demo) && index==0) return GlassDemoBack;
    return GlassDemoHasButtons(demo) && index==1 ? GlassDemoMore : GlassDemoPlus;
}
typedef struct { float x,y,width,height,angle,radius; } GlassDemoStroke;
static inline int GlassDemoStrokeCount(int symbol) { return symbol==GlassDemoMore ? 3 : 2; }
static inline GlassDemoStroke GlassDemoStrokeAt(int symbol, int index) {
    if (symbol==GlassDemoMore) return (GlassDemoStroke){1+index*7.5f,8.5f,3,3,0,1.5f};
    if (symbol==GlassDemoBack) return (GlassDemoStroke){3.35f,index ? 13.0f : 5.0f,11.3f,2,index ? 45.0f : -45.0f,1};
    if (symbol==GlassDemoCheck) return index ? (GlassDemoStroke){5.5f,9,15,2,-45,1} : (GlassDemoStroke){0,12,8,2,45,1};
    return index ? (GlassDemoStroke){9,0,2,20,0,1} : (GlassDemoStroke){0,9,20,2,0,1};
}
static inline void GlassDemoTint(int tint, unsigned char *r, unsigned char *g, unsigned char *b) {
    const unsigned char colors[][3] = {{255,255,255},{0,122,255},{255,59,48},{52,199,89}};
    *r=colors[tint][0]; *g=colors[tint][1]; *b=colors[tint][2];
}
