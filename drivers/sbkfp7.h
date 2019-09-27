#ifndef SBKFP7_H
#define SBKFP7_H

typedef struct __attribute__((packed))
{
  unsigned char UpReadC; // default = 0; переменная отсчета цикла    
} sbk_inipar;

typedef struct __attribute__((packed))
{
  ssbool SbkSIGN[13]; // Результат расшифровки портов
  char emptyspace[64]; // Зарезервированная память
} sbk_data;

#define SBK 0xCC
#define SBK_SIZE sizeof(sbk_data)

void sbkfp7_ini(table_drv *drv);
void sbkfp7_dw(table_drv *drv);

#endif /* SBKFP7_H */
