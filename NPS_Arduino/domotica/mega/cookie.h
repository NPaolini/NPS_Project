//-------- cookie.h ----------------
#ifndef cookie_H_
#define cookie_H_
//----------------------------------
#define MAX_COOKIE    10
#define MAX_USER      10
//----------------------------------
#define DIM_COOKIE    8
#define DIM_USER      16
#define DIM_PASSWORD  16
//----------------------------------
struct cookie
{
  char c[DIM_COOKIE + 1];
  int8_t idUser;
  unsigned long startTime;
  cookie() : idUser(-1), startTime(0) 
  { 
    memset(c, 0, sizeof(c));
  }
};
//----------------------------------
class manCookie
{
  public:
    manCookie() {}
    ~manCookie() {}
    // se già esiste, riparte col time
    bool isValid(const char* c);
    // elimina i cookie scaduti
    void loop();
    // se non c'è posto, elimina quello col time più lungo, ne restituisce l'indice
    uint8_t add(const char* c, int8_t idUser);
    // lo crea e ne restituisce l'indice
    uint8_t make(int8_t idUser);
    void get(char* c, uint8_t pos);
    int8_t getUser(char* c);

    void rem(const char* c);
  private:
    // se lo manteniamo a lungo meglio metterli in infoSetup, altrimenti se si riavvia, si riparte
    cookie Cookie[MAX_COOKIE];
};
//----------------------------------
#endif
