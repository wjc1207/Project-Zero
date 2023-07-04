#define BUFFER_SIZE 256  // I2S缓冲区大小
#define DMA_BUF_COUNT 6
#define DMA_BUF_LEN 512

//pin definition
#define BCK 26
#define WS 27
#define DATA_OUT 25
#define SW1 (33)
#define SW2 (16)
#define SW3 (32)
#define SHUTDOWN (23)

//audio volume
#define MINMUSICVOL (1)
#define MAXMUSICVOL (5)

//UI
#define MINUISTATE (0)
#define MAXUISTATE (5)
#define UISTATE_VOLUP (1)
#define UISTATE_VOLDOWN (5)
#define UISTATE_MUSICBACK (2)
#define UISTATE_MUSICNEXT (4)
#define UISTATE_PLAYPAUSE (3)
