

import core.simd;


alias i8  = byte;
alias i16 = short;
alias i32 = int;
alias i64 = long;

alias u8  = ubyte;
alias u16 = ushort;
alias u32 = uint;
alias u64 = ulong;

alias f32 = float;
alias f64 = double;

alias V2 = float[2];
alias V3 = float[3];
alias V4 = float[4];

alias M3 = float[9];
alias M4 = float[16];

alias vec4 = float4;
alias mat4 = float4[4];


int   mini(int   x, int   y) { return (x < y)? x: y; }
uint  minu(uint  x, uint  y) { return (x < y)? x: y; }
float minf(float x, float y) { return (x < y)? x: y; }

int   maxi(int   x, int   y) { return (x > y)? x: y; }
uint  maxu(uint  x, uint  y) { return (x > y)? x: y; }
float maxf(float x, float y) { return (x > y)? x: y; }

int   clampi(int   x, int   min, int   max) { return mini(maxi(x, min), max); }
uint  clampu(uint  x, uint  min, uint  max) { return minu(maxu(x, min), max); }
float clampf(float x, float min, float max) { return minf(maxf(x, min), max); }

