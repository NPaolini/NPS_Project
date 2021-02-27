//-------- mydebug.h ----------------
#ifndef mydebug_H_
#define mydebug_H_
//----------------------------------
#define DEBUG_1(a) if(useDebug & 1) { a; }
#define DEBUG_2(a) if(useDebug & 2) { a; }
#define DEBUG_4(a) if(useDebug & 4) { a; }
#define DEBUG_8(a) if(useDebug & 8) { a; }
#define DEBUG_16(a) if(useDebug & 16) { a; }
#define DEBUG_32(a) if(useDebug & 32) { a; }
#define DEBUG_64(a) if(useDebug & 64) { a; }
#define DEBUG_128(a) if(useDebug & 128) { a; }
#define DEBUG_256(a) if(useDebug & 256) { a; }
#define DEBUG_512(a) if(useDebug & 512) { a; }
#define DEBUG_1024(a) if(useDebug & 1024) { a; }
#define DEBUG_2048(a) if(useDebug & 2048) { a; }
#define DEBUG_4096(a) if(useDebug & 4096) { a; }
#define DEBUG_8192(a) if(useDebug & 8192) { a; }
//----------------------------------
#define DEBUG_SETUP DEBUG_1
#define D_BIT_SETUP (1 << 0)

#define DEBUG_EMAIL DEBUG_2
#define D_BIT_EMAIL (1 << 1)

#define DEBUG_ALARM DEBUG_4
#define D_BIT_ALARM (1 << 2)

#define DEBUG_DATE  DEBUG_8
#define D_BIT_DATE (1 << 3)

#define DEBUG_WEB   DEBUG_16
#define D_BIT_WEB (1 << 4)

#define DEBUG_CRONO DEBUG_32
#define D_BIT_CRONO (1 << 5)

#define DEBUG_RAIN  DEBUG_64
#define D_BIT_RAIN (1 << 6)

#define DEBUG_LIGHT DEBUG_128
#define D_BIT_LIGHT (1 << 7)

#define DEBUG_LOG DEBUG_256
#define D_BIT_LOG (1 << 8)

#define DEBUG_PID DEBUG_512
#define D_BIT_PID (1 << 9)

#define DEBUG_PING DEBUG_1024
#define D_BIT_PING (1 << 10)

#define DEBUG_RFID DEBUG_2048
#define D_BIT_RFID (1 << 11)

#define DEBUG_USER DEBUG_4096
#define D_BIT_USER (1 << 12)

#define DEBUG_IP DEBUG_8192
#define D_BIT_IP (1 << 13)
//----------------------------------
#endif
