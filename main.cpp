// Weapon Fire Recoil (versi .so / AML) — implementasi ulang berdasarkan
// perilaku & format .ini dari mod CLEO "Weapon Fire Recoil for Mobile" by cheeseburger.
// Tidak memakai kode dari script .csa aslinya.
//
// Offset di bawah diambil dari info debug (DWARF) + disassembly libGTASA.so arm64
// milik pengguna. Hanya berlaku untuk build yang sama (dicek lewat FIRE_SYM_RVA).
#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <stdint.h>

MYMODCFGNAME(weaponfirerecoil_aml, Weapon Fire Recoil, 1.0.1, rewrite, weapon_fire_recoil)
NEEDGAME(com.rockstargames.gtasa)

#ifdef AML64
  // CCamera
  #define OFF_CAMERA_ACTIVECAM   95     // uint8  CCamera::ActiveCam
  #define OFF_CAMERA_CAMS        376    // CCam   CCamera::Cams[3]
  #define SIZEOF_CCAM            552
  // CCam
  #define OFF_CAM_ALPHA          132    // float  (sudut vertikal)
  #define OFF_CAM_BETA           148    // float  (sudut horizontal)
  // CPhysical (dari CPhysical::ApplyMoveSpeed)
  #define OFF_PHYS_MOVESPEED     104    // CVector m_vecMoveSpeed
  // Penanda build: RVA CWeapon::Fire di build ini
  #define FIRE_SYM_RVA           0x700C64
#endif

#define SYM_CWEAPON_FIRE   "_ZN7CWeapon4FireEP7CEntityP7CVectorS3_S1_S3_S3_"
#define SYM_FINDPLAYERPED  "_Z13FindPlayerPedi"
#define SYM_THECAMERA      "TheCamera"

struct WeaponCfg { int id; const char* key; float v, h; };
static WeaponCfg g_W[] = {
    {22,"Colt45"},{23,"Silenced"},{24,"Deagle"},{25,"PumpSG"},{26,"Sawnoff"},
    {27,"CombatSG"},{28,"MicroSMG"},{29,"MP5"},{32,"Tec9"},{30,"AK47"},
    {31,"M4"},{33,"CountryRifle"},{34,"Sniper"},{38,"Minigun"},
};
static int   g_penaltyPercent = 100;
static float g_vertSign = 1.0f;     // ubah ke -1 di .ini bila arah kick vertikal terbalik
static float g_kickScale = 0.01f;   // radian per 1.0 nilai .ini

static void* (*FindPlayerPed)(int) = NULL;
static uint8_t* pTheCamera = NULL;
typedef bool (*FireFn)(void* self, void* owner, void* a, void* b, void* c, void* d, void* e);
static FireFn Fire_orig = NULL;

static bool IsMoving(void* ped)
{
    float* v = (float*)((uintptr_t)ped + OFF_PHYS_MOVESPEED);
    return (v[0]*v[0] + v[1]*v[1]) > 0.0004f;
}

static void ApplyCameraKick(float vert, float horiz)
{
    if(!pTheCamera) return;
    uint8_t idx = pTheCamera[OFF_CAMERA_ACTIVECAM];
    if(idx > 2) return;
    uint8_t* cam = pTheCamera + OFF_CAMERA_CAMS + idx * SIZEOF_CCAM;
    *(float*)(cam + OFF_CAM_ALPHA) += g_vertSign * vert * g_kickScale;
    *(float*)(cam + OFF_CAM_BETA)  += horiz * g_kickScale * (float)randsign();
}

static bool Fire_hook(void* self, void* owner, void* a, void* b, void* c, void* d, void* e)
{
    bool fired = Fire_orig(self, owner, a, b, c, d, e);
    if(!fired || !owner || owner != FindPlayerPed(-1)) return fired;

    int type = *(int*)self;                       // CWeapon::m_eWeaponType (offset 0)
    for(auto& w : g_W) if(w.id == type)
    {
        float v = w.v, h = w.h;
        if(IsMoving(owner)) v *= (float)g_penaltyPercent / 100.0f;
        if(v != 0.0f || h != 0.0f) ApplyCameraKick(v, h);
        break;
    }
    return fired;
}

ON_MOD_LOAD()
{
#ifndef AML64
    logger->Error("Mod ini baru mendukung libGTASA arm64");
    return;
#else
    g_penaltyPercent = clampint(0, 300, cfg->GetInt("PenaltyPercent", 100, "Main"));
    g_vertSign       = (cfg->GetInt("VerticalSign", 1, "Main") < 0) ? -1.0f : 1.0f;
    for(auto& w : g_W)
    {
        w.v = clampfloat(0.0f, 4.0f, cfg->GetFloat(w.key, 0.0f, "Vertical"));
        w.h = clampfloat(0.0f, 4.0f, cfg->GetFloat(w.key, 0.0f, "Horizontal"));
    }

    uintptr_t base = aml->GetLib("libGTASA.so");
    void* h = aml->GetLibHandle("libGTASA.so");
    if(!base || !h) { logger->Error("libGTASA.so tidak ditemukan"); return; }

    uintptr_t fire = aml->GetSym(h, SYM_CWEAPON_FIRE);
    if(!fire || (fire - base) != FIRE_SYM_RVA)
    {
        logger->Error("Build libGTASA tidak cocok, mod dinonaktifkan");
        return;
    }
    FindPlayerPed = (void*(*)(int))aml->GetSym(h, SYM_FINDPLAYERPED);
    pTheCamera    = (uint8_t*)aml->GetSym(h, SYM_THECAMERA);
    if(!FindPlayerPed || !pTheCamera) { logger->Error("Simbol tidak lengkap"); return; }

    aml->Hook((void*)fire, (void*)Fire_hook, (void**)&Fire_orig);
#endif
}
