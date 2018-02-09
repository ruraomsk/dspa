/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#define SIZE_BUF3 5

void testdrv3_ini(table_drv* tdrv) {
    memcpy(tdrv->data, tdrv->inimod, SIZE_BUF3);
//    tdrv->time=read_timer();
    tdrv->error = 0;

    log_init(tdrv);

}

void testdrv3_step(table_drv* tdrv) {
    int i;
    
    for (i = 0; i < SIZE_BUF3; i++) {
        tdrv->data[i] = (tdrv->data[i]<<1)|1 ;
    }
//    tdrv->time=read_timer();
    tdrv->error = 0;
    log_step(tdrv);
}
