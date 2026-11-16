Weapon Fire Recoil — versi AML (.so), libGTASA arm64
Konsep & format .ini: cheeseburger (mod CLEO asli). Ini implementasi ulang, bukan port kode.

Build : taruh folder ini sejajar dengan AndroidModLoader-main, lalu jalankan ndk-build (ABI arm64-v8a).
Pasang: libWeaponFireRecoil.so -> folder mod AML; weapon_fire_recoil.ini -> folder config AML.
Offset (arm64): CCamera.ActiveCam=95, Cams=376, sizeof(CCam)=552, Alpha=132, Beta=148, m_vecMoveSpeed=104.
Pengaman: mod mati sendiri jika RVA CWeapon::Fire bukan 0x700C64 (build lain).
Status: belum dikompilasi/diuji di game. Kalau kick vertikal terbalik, set VerticalSign = -1.

GitHub Actions: upload isi folder ini ke repo (root repo = folder ini, termasuk .github/).
Workflow .github/workflows/build.yml otomatis clone AML & build; hasil ada di tab Actions > Artifacts.
