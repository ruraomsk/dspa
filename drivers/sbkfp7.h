#ifndef SBKFP7_H
#define SBKFP7_H

typedef struct __attribute__((packed))
{
  unsigned char type;  // default = 0xC2;  
  unsigned int BoxLen; // default = 0xFF;  
  unsigned char vip;   // default = 0;     
  unsigned char NumCh; // default = 8;     
} sbk_inipar;

typedef struct __attribute__((packed))
{
  ssbool SbkSIGN[13];
} sbk_data;

#define SBK 0xCC
#define SBK_SIZE sizeof(sbk_data)

void sbkfp7_ini(table_drv *drv);
void sbkfp7_dw(table_drv *drv);

#endif /* SBKFP7_H */
