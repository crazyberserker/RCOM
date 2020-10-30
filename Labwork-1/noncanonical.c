/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


#define FLAG 0x7e
#define A_RCV_RSP 0x03
#define A_RCV_CMD 0x01
#define A_SND_CMD 0x03
#define A_SND_RSP 0x01

#define SET 0x03
#define UA 0x07


void send_cmd(int fd, char a, char c){
	unsigned char buf[5];
    buf[0]=FLAG;
    buf[1]=a;
    buf[2]=c;
    buf[3]=a ^ c;
    buf[4]=FLAG;
    write(fd,buf,5);
}

void send_ua(int fd)
{
  	unsigned char buf[5];
    buf[0]=FLAG;
    buf[1]=A_RCV_CMD;
    buf[2]=UA;
    buf[3]=A_RCV_CMD^ UA;
    buf[4]=FLAG;
    write(fd,buf,5);

    printf("wrote %x %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);

}

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[6];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) ) ) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 3;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    res=read(fd,buf,6);

    send_ua(fd);



    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}