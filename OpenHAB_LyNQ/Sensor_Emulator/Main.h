#define HOST "localhost"
#define	CLOSE_CONNECTION "Connection: close"

#define PORT 8080
#define MAX_WAIT 10
#define HUGE_SIZE 2048

#define GET_FORMAT "GET %s HTTP/1.1\r\nHost: %s:%d\r\n%s\r\n\r\n"
#define	SET_STATE_FORMAT "/CMD?%s=%d" 
#define SITE_MAP_FORMAT "%s item=%s"
#define ICON_TEMP "temperature"
#define ICON_HUM "humidity"
#define ITEM_FORMAT "%s %s \"%s [%s]\" <%s> (all) {mqtt=\">[%s:%s/out:state:*:default], <[%s:%s/in:state:default]\"}"

#define TEMP_SENSOR "TEMP_"
#define HUM_SENSOR "HUM_"

#define PREFIX_NAME "sensor_"
#define FILE_CONFIG "config.cfg"
#define FILE_ITEM "\\configurations\\items\\demo.items"
#define FILE_SITE_MAP "\\configurations\\sitemaps\\demo.sitemap"


typedef struct _ITEM
{
	int iType;
	int iTime;
	char strName[15];
	char strLabel[MAX_PATH];
} ITEM;


DWORD WINAPI threadStop(LPVOID lpParam);
DWORD WINAPI threadSendCommand(LPVOID lpParam);
BOOL readConfig();
void readLineInFile(FILE *f, char *str, int iSize);
BOOL isSpace(char c);
void standardizeLine(char *strLine, char *strOut);
void createFileSitemap();
void createFileItem();
int generateNumber(int iInitValue, int iType);