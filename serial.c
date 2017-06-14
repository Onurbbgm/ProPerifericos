#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


 
#define IN  0
#define OUT 1
 
#define LOW  0
#define HIGH 1
 
static int
GPIOExport(int pin)
{
        #define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
 
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}
 
static int
GPIOUnexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
 
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}
 
static int
GPIODirection(int pin, int dir)
{
	static const char s_directions_str[]  = "in\0out";
 
        #define DIRECTION_MAX 35
	char path[DIRECTION_MAX];
	int fd;
 
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}
 
	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}
 
	close(fd);
	return(0);
}
 
static int
GPIORead(int pin)
{
        #define VALUE_MAX 30
	char path[VALUE_MAX];
	char value_str[3];
	int fd;
 
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}
 
	if (-1 == read(fd, value_str, 3)) {
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}
 
	close(fd);
 
	return(atoi(value_str));
}
 
static int
GPIOWrite(int pin, int value)
{
	static const char s_values_str[] = "01";
 
	char path[VALUE_MAX];
	int fd;
 
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}
 
	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}
 
	close(fd);
	return(0);
}

static int config_serial(char * device, unsigned int baudrate){
    struct termios options;
    int fd;
    
    fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY );
    if (fd < 0)
    {
        /*
        * Could not open the port.
        */
      
        perror("config_serial: Não pode abrir a serial - ");
        return -1;
    }

    fcntl(fd, F_SETFL, 0);

    /*
    * Get the current options for the port...
    */
    tcgetattr(fd, &options);
    
    /* sets the terminal to something like the "raw" mode */
    cfmakeraw(&options);
    
    /*
    * Set the baudrate...
    */
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);

    /*
    * Enable the receiver and set local mode...
    */
    options.c_cflag |= (CLOCAL | CREAD);
    
    /*
     * No parit, 1 stop bit, size 8
     */
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    
    /*
     * Clear old settings
     */
    options.c_cflag &= ~CRTSCTS;
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    /* non-caninical mode */
    options.c_lflag &= ~ICANON; 

    /*
    * Set the new options for the port...
    */
    tcsetattr(fd, TCSANOW, &options);
    
    /* configura a tty para escritas e leituras não bloqueantes */
    //fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    
    return fd;
}

 int readButton(int pushbutton){

    if (-1 == GPIODirection(pushbutton, OUT))
      return(2);

    /*
     * Write GPIO value
     */
    if (-1 == GPIOWrite(pushbutton, 1))
      return(3);
 
    /*
     * Set GPIO directions
     */
    if (-1 == GPIODirection(pushbutton, IN))
      return(2);

    /*
     * Read GPIO value
     */
   return  GPIORead(pushbutton);

}

int main(int argc, char** argv){
	int fd;
	char a;

	if(argc<2){
		printf("Usage: ./serial <char>");
		return 0;
	}
        fd = config_serial("/dev/ttyAMA0", B9600);	
	if(fd<0){
		return 0;
	}

//	a = argv[1][0];
//	printf("Teste %c\n", a);


 int repeat = 10000;
 int padrao1 = 0;
 int padrao2 = 0;
 int padrao3 = 0;

  /*
   * Enable GPIO pins
   */
    if (-1 == GPIOExport(22))
    	return(1); 
    if (-1 == GPIOExport(23))
     	return(1);
    if (-1 == GPIOExport(24))
     	return(1); 
  do {
    a = 0;
    if (readButton(22) == 0) { a = 1; }   // Botao 1
    if (readButton(23) == 0) { a = 2; }   // Botao 2
    if (readButton(24) == 0) { a = 3; }   // Botao 3
    if (a != 0) { // Se algum botao foi pressionado...
        printf("Botao %d pressionado.\n", a); 

    write(fd, &a, 1);   // Envia o botao pressionado
        if (a == 3) {       // Se o botao 3 foi pressionado, le o numero de vezes que cada padrao foi exibido
            read(fd, &padrao1, 1);
            read(fd, &padrao2, 1);
            read(fd, &padrao3, 1);
            printf("Padrao 1: %dx | Padrao 2: %dx | Padrao 3: %dx\n", padrao1, padrao2, padrao3);
        }
    }
    usleep(500 * 1000);
  }
  while (repeat--);

  close(fd);
 
  /*
   * Disable GPIO pins
   */
  if (-1 == GPIOUnexport(22))
    return(4);
  if (-1 == GPIOUnexport(23))
    return(4);
  if (-1 == GPIOUnexport(24))
    return(4);
 
 		
	return 0;


}
