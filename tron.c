#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


#define UP          0
#define DOWN        1
#define LEFT        2
#define RIGHT       3

#define EXIT_KEY   'q'
#define UP_KEY     'w'
#define DOWN_KEY   's'
#define LEFT_KEY   'a'
#define RIGHT_KEY  'd'

#define BACK_COLOR  0x00000000
#define GREEN       0x0000FF00
#define BLUE        0x000000FF
#define RED         0x00FF0000

#define HEIGHT      5
#define LENGHT      8

#define SLEEP_TIME  62500
#define LOSE_CODE   1


int work_flag = 1;
int flag = 0;
int step = 0;

typedef struct Bike
{

    unsigned short coord[2];
    uint8_t direct;
    uint8_t st_direct;
    uint8_t old_direct;
    uint32_t ip;
    uint8_t ready_flag;
    uint8_t status;
    uint32_t color;

}Bike;


int strToInt(char* str_adress)
{
    unsigned int result = 0;
    int buffer = 0;
    int i = 0;
    while((str_adress[i] <= '9' && str_adress[i] >= '0'))
    {

        while(str_adress[i] != '.' && (str_adress[i] <= '9' && str_adress[i] >= '0'))
        {
                buffer *= 10;
                buffer += str_adress[i] - '0';
                i++;
        }
        result *= 256;

        result += buffer;

        buffer = 0;
        i++;
    }

    return result;
}

int check_ip(unsigned int first_ip,unsigned int second_ip, unsigned int netmask){

    if((first_ip&netmask)!=(second_ip&netmask)){
        return -1;
    }
    else if(first_ip > second_ip){
        return 1;
    }
return 0;
}

int network_check(char *enemyIP){
    int flag;
    char *nt_addr;
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }

        char addressBuffer[INET_ADDRSTRLEN];
        char maskBuffer[INET_ADDRSTRLEN];

        if (ifa->ifa_addr->sa_family == AF_INET) {

            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;

            tmpAddrPtr = &((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            tmpAddrPtr = &((struct sockaddr_in *)(ifa->ifa_netmask))->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, maskBuffer, INET_ADDRSTRLEN);

            if(-1==(flag = check_ip(strToInt(addressBuffer), strToInt(enemyIP), strToInt(maskBuffer))))
            {
                continue;
            }
            else{
                if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
                return flag;
            }
        }
    }
    return -1;
}
void* serv_func(void* args){

    Bike* bike = (Bike*)args;
    char *buf = (char*) malloc(1);
    int sockfd;
    int pack = 0;
    struct sockaddr_in servaddr, cliaddr;

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){

        perror("socket creation failed");
        exit(EXIT_FAILURE);

    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(12345);

    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){

        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    socklen_t addr_len = sizeof(servaddr);
    while(work_flag)
    {

        int n = recvfrom(sockfd, buf, 1, MSG_WAITALL, (struct sockaddr *)&cliaddr, &addr_len);

        if(cliaddr.sin_addr.s_addr == (bike -> ip))
        {

            if(!(bike->ready_flag)) bike->ready_flag = 1;

                    bike -> st_direct = (int)*buf;
                    flag = 1;

        }

    }
    close(sockfd);

}

void cli_func(char* ip, int port,  char *button){

    int sockfd;
    struct sockaddr_in servaddr;

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {

        perror("socket creation failed");
        exit(EXIT_FAILURE);

    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);

    sendto(sockfd, (const char*)button, 1, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    close(sockfd);

}


void* control(void* args)
{

    Bike* bike = (Bike*) args;


    while(work_flag)
    {
        int controlKey = getchar();
        switch(controlKey)
        {
            case UP_KEY:
                if(bike -> direct != DOWN) bike -> direct = UP;
                break;
            case DOWN_KEY:
                if(bike -> direct != UP) bike -> direct = DOWN;
                break;
            case RIGHT_KEY:
                if(bike -> direct != LEFT) bike -> direct = RIGHT;
                break;
            case LEFT_KEY:
                if(bike -> direct != RIGHT) bike -> direct = LEFT;
                break;
            case EXIT_KEY:
                bike->status = LOSE_CODE;
                break;
       }
    }
}

void displaying(Bike* bike, struct fb_var_screeninfo info, uint32_t *ptr)
{
    // while(work_flag)
    // {
        switch(bike -> old_direct)
        {

           case UP:
                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 5; j++)
	                {

	                    ptr[(bike-> coord[0] - i) * info.xres_virtual + bike-> coord[1] - 2 + j] = BACK_COLOR;

	                }
                }
                ptr[bike-> coord[0] * info.xres_virtual + bike-> coord[1]] = bike -> color;
                break;
            case DOWN:
                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 5; j++)
	                {

	                    ptr[(bike-> coord[0] + i) * info.xres_virtual + bike-> coord[1] - 2 + j] = BACK_COLOR;

	                }
                }
                ptr[bike-> coord[0] * info.xres_virtual + bike-> coord[1]] = bike -> color;
                break;
            case LEFT:
                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 5; j++)
	                {

	                    ptr[(bike-> coord[0] - 2 + j) * info.xres_virtual + bike-> coord[1] - i] = BACK_COLOR;

	                }
                }
                ptr[bike-> coord[0] * info.xres_virtual + bike-> coord[1]] = bike -> color;
                break;
            case RIGHT:
                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 5; j++)
	                {

	                    ptr[(bike-> coord[0] - 2 + j) * info.xres_virtual + bike-> coord[1] + i] = BACK_COLOR;

	                }
                }
                ptr[bike-> coord[0] * info.xres_virtual + bike-> coord[1]] = bike -> color;
                break;
        }

        bike -> old_direct = bike -> st_direct;

        switch(bike -> st_direct)
        {

            case UP:
		        //ptr[(bike -> coord[0] - 1) * info.xres_virtual + bike->coord[1]] = bike->color;
                bike -> coord[0] = bike -> coord[0] - 1;
                bike -> st_direct = bike -> direct;

                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 5; j++)
	                {
                        if(ptr[(bike-> coord[0] - i) * info.xres_virtual + bike-> coord[1] - 2 + j] == BACK_COLOR)
                        {
	                        ptr[(bike-> coord[0] - i) * info.xres_virtual + bike-> coord[1] - 2 + j] = bike -> color;
                        }
                        else
                        {

                            bike->status = LOSE_CODE;
                            return;

                        }
	                }
                }
        		break;
            case DOWN:
		        //ptr[(bike -> coord[0] + 1) * info.xres_virtual + bike->coord[1]] = bike->color;
                bike -> coord[0] = bike -> coord[0] + 1;

                bike -> st_direct = bike -> direct;

                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 5; j++)
	                {

                        if(ptr[(bike-> coord[0] + i) * info.xres_virtual + bike-> coord[1] - 2 + j] == BACK_COLOR)
                        {
	                        ptr[(bike-> coord[0] + i) * info.xres_virtual + bike-> coord[1] - 2 + j] = bike -> color;
                        }
                        else
                        {
                            bike->status = LOSE_CODE;
                            return;
                        }
	                }
                }
                break;
            case LEFT:
        		//ptr[(bike -> coord[0]) * info.xres_virtual + bike->coord[1] - 1] = bike->color;
                bike -> coord[1] = (bike -> coord[1] - 1);

                bike -> st_direct = bike -> direct;
                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 5; j++)
	                {

                        if(ptr[(bike-> coord[0] - 2 + j) * info.xres_virtual + bike-> coord[1] - i] == BACK_COLOR)
                        {
	                        ptr[(bike-> coord[0] - 2 + j) * info.xres_virtual + bike-> coord[1] - i] = bike->color;
                        }
                        else
                        {
                            bike->status = LOSE_CODE;
                            return;
                        }

	                }
                }
                break;
            case RIGHT:
		        //ptr[(bike -> coord[0]) * info.xres_virtual + bike->coord[1] + 1] = bike->color;
                bike -> coord[1] = (bike -> coord[1] + 1);
                bike -> st_direct = bike -> direct;
                for(int i = 0; i < 8; i++)
                {
                    for(int j = 0; j < 5; j++)
	                {

                        if(ptr[(bike-> coord[0] - 2 + j) * info.xres_virtual + bike-> coord[1] + i]  == BACK_COLOR)
                        {
	                        ptr[(bike-> coord[0] - 2 + j) * info.xres_virtual + bike-> coord[1] + i] = bike->color;
                        }
                        else
                        {
                            bike->status = LOSE_CODE;
                            return;
                        }

	                }
                }
                break;
        }

    //}

}

void argToInt(char* string, int result[])
{

    int i = 0;
    while(string[i] != 'x')
    {

        result[0] = result[0]*10+string[i]-'0';
        i++;

    }
    i++;
    while(string[i] != '\0')
    {

        result[1] = result[1]*10+string[i]-'0';
        i++;

    }

}

int main(int argc, char* argv[])
{


	int fb;
	struct fb_var_screeninfo info;
	size_t fb_size, map_size, page_size;
	uint32_t *ptr;
    Bike red, blue;

	page_size = sysconf(_SC_PAGESIZE);

    for(int i=0;i<1000;i++)
    {
        printf("\n");
    }


	if ( 0 > (fb = open("/dev/fb0", O_RDWR)))
	{

		perror("open");
		return __LINE__;

	}

	if ( (-1) == ioctl(fb, FBIOGET_VSCREENINFO, &info) )
	{

		perror("ioctl");
		close(fb);
		return __LINE__;

	}

	fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
	map_size = (fb_size + (page_size - 1 )) & (~(page_size-1));

	ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);

	if ( MAP_FAILED == ptr )
	{

		perror("mmap");
		close(fb);
		return __LINE__;

	}

  	initscr();
    noecho();
	//curs_set(0);
	keypad(stdscr, TRUE);

    int enxy[2] = {0, 0};

    argToInt(argv[1], enxy);
    if(enxy[0] < info.xres)info.xres = enxy[0];
    if(enxy[1] < info.yres)info.yres = enxy[1];

    red.color = RED;
    red.direct = red.st_direct = red.old_direct = RIGHT;
    red.coord[0] = 3;
    red.coord[1] = 1;
    red.status = 0;
    red.ready_flag = 0;
    red.ip = inet_addr(argv[2]);

    blue.color = BLUE;
    blue.direct = blue.st_direct = blue.old_direct = LEFT;
    blue.coord[0] = info.yres - 4;
    blue.coord[1] = info.xres - 2;
    blue.status = 0;
    blue.ready_flag = 0;
    blue.ip = inet_addr(argv[2]);

    for(int i=0; i < info.yres; i++)
    {

        for(int j=0; j < info.xres; j++)
        {

            ptr[i * info.xres_virtual + j] = BACK_COLOR;

        }

    }
    for(int i=0; i < info.xres; i++)
    {

        ptr[i] = GREEN;
        ptr[(info.yres - 1) * info.xres_virtual + i] = GREEN;

    }

    for(int i=0; i < info.yres; i++)
    {

        ptr[i * info.xres_virtual] = GREEN;
        ptr[i * info.xres_virtual + info.xres - 1] = GREEN;

    }

    for(int i = 0; i < LENGHT; i++)
    {
        for(int j = 0; j < HEIGHT; j++)
        {

            ptr[(blue.coord[0] - HEIGHT/2 + j) * info.xres_virtual + blue.coord[1] - i] = blue.color;
            ptr[(red.coord[0] - HEIGHT/2 + j) * info.xres_virtual + red.coord[1] + i] = red.color;

        }
    }

    struct timeb timebuffer;
    unsigned short mil;
    pthread_t thread;
    pthread_t server;

    int slave = network_check(argv[2]);
    //if(blue.ip == red.ip)return 0;
    if(slave < 0)work_flag = 0;

    if(slave)
    {
            pthread_create(&server, NULL, serv_func, (void*)&red);
            char ready = getchar();

            do
            {
                cli_func(argv[2], 12345, &ready);
            }while(!red.ready_flag);


            pthread_create(&thread, NULL, control, (void*)&blue);

            while(work_flag)
            {
                cli_func(argv[2], 12345, &(blue.st_direct));
                if(flag)
                {
                    ftime(&timebuffer);
                    mil = timebuffer.millitm;
                    if(!red.status)
                    displaying(&red, info, ptr);
                    if(!blue.status)
                    displaying(&blue, info, ptr);
                    if(red.status || blue.status)
                    {
                        work_flag = 0;
                    }
                    flag = 0;
                    ftime(&timebuffer);
                    mil = timebuffer.millitm - mil;
                    usleep(SLEEP_TIME - mil);

                }
            }
                cli_func(argv[2], 12345, &(blue.st_direct));
                if(flag)
                {
                    ftime(&timebuffer);
                    mil = timebuffer.millitm;
                    if(!red.status)
                    displaying(&red, info, ptr);
                    if(!blue.status)
                    displaying(&blue, info, ptr);
                    if(red.status || blue.status)
                    {
                        work_flag = 0;
                    }
                    flag = 0;
                    ftime(&timebuffer);
                    mil = timebuffer.millitm - mil;
                    usleep(SLEEP_TIME - mil);
                }

    }
    else
    {
            pthread_create(&server, NULL, serv_func, (void*)&blue);
            char ready = getchar();

            do
            {
                cli_func(argv[2], 12345, &ready);
            }while(!blue.ready_flag);


            pthread_create(&thread, NULL, control, (void*)&red);

            while(work_flag)
            {
                cli_func(argv[2], 12345, &(red.st_direct));
                if(flag)
                {
                    ftime(&timebuffer);
                    mil = timebuffer.millitm;
                    if(!red.status)
                    displaying(&red, info, ptr);
                    if(!blue.status)
                    displaying(&blue, info, ptr);
                    if(red.status || blue.status)
                    {
                        work_flag = 0;
                    }
                    flag = 0;
                    ftime(&timebuffer);
                    mil = timebuffer.millitm - mil;
                    usleep(SLEEP_TIME - mil);

                }
            }
                cli_func(argv[2], 12345, &(red.st_direct));
                if(flag)
                {
                    ftime(&timebuffer);
                    mil = timebuffer.millitm;
                    if(!red.status)
                    displaying(&red, info, ptr);
                    if(!blue.status)
                    displaying(&blue, info, ptr);
                    if(red.status || blue.status)
                    {
                        work_flag = 0;
                    }
                    flag = 0;
                    ftime(&timebuffer);
                    mil = timebuffer.millitm - mil;
                    usleep(SLEEP_TIME - mil);

                }
    }
    if(red.status == 1 && blue.status == 1 )
    {
        for(int i = 0; i < info.xres; i++)
        {
            for(int j = 0; j < info.yres; j++)
            {

                ptr[(j) * info.xres_virtual +  i] = GREEN;

            }
        }
    }
    else
    if(red.status == 1 )
    {
        for(int i = 0; i < info.xres; i++)
        {
            for(int j = 0; j < info.yres; j++)
            {

                ptr[(j) * info.xres_virtual +  i] = BLUE;

            }
        }
    }
    else
    if(blue.status == 1 )
    {
        for(int i = 0; i < info.xres; i++)
        {
            for(int j = 0; j < info.yres; j++)
            {

                ptr[(j) * info.xres_virtual +  i] = RED;

            }
        }
    }
    pthread_cancel(thread);
    pthread_cancel(server);
    endwin();
	munmap(ptr, map_size);
	close(fb);

    return 0;

}
