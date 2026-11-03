#include <ti/screen.h>
#include <ti/getcsc.h>
int main(void) {
    os_ClrHome();
    os_PutStrFull("LAUNCH CHECK OK");
    os_NewLine();
    os_PutStrFull("Press a key to exit");
    while (os_GetCSC()) {}
    while (!os_GetCSC()) {}
    return 0;
}
