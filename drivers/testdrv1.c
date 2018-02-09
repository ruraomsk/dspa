#define SIZE_BUF1 17

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
void testdrv1_ini(table_drv* tdrv) {
    log_init(tdrv);
//    sprintf(str,"data: %d %d %d %d %d",tdrv->data[0],tdrv->data[1],tdrv->data[2],tdrv->data[3],tdrv->data[4]);
//    log(str);
//    tdrv->time=read_timer();
    tdrv->error = 0;
}

void testdrv1_step(table_drv* tdrv) {
//    char str[120];
//    sprintf(str,"data: %d %d %d %d %d",tdrv->data[0],tdrv->data[1],tdrv->data[2],tdrv->data[3],tdrv->data[4]);
//    log(str);
    int i;
    for (i = 0; i < SIZE_BUF1; i++) {
        tdrv->data[i] = tdrv->data[i] + 1;
    }
    i=1000;
    while (i--) {
        char in=ReadPort(100);
        if(ERR_PORT) break;
        WritePort(101,0);
        if(ERR_PORT) break;
    }

//    tdrv->time=read_timer();
    tdrv->error = 0;
    if(ERR_PORT){
        tdrv->error = ERR_PORT;
    }
    log_step(tdrv);
}