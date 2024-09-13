#include "util/input.h"

#include <stdio.h>

#ifdef _WIN32
#include <conio.h>

#define _us_getchar _getch

bool
us_chrdy(void)
{
    return _kbhit();
}
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define _us_getchar getchar

bool
us_chrdy(void)
{
    int bytesWaiting;
    return ioctl(0, FIONREAD, &bytesWaiting) == 0 && bytesWaiting;
}

extern struct termios newtw, newti;

static inline void
enableInputWait(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &newtw);
}

static inline void
disableInputWait(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &newti);
}

static inline void
clearInput(void)
{
    disableInputWait();
    while (getchar() > 0)
        ;
    enableInputWait();
}
#endif

USChar
us_getchar(void)
{
    int c = _us_getchar();
    switch (c)
    {
    case 3:
    case 4: return USCharExit;

#ifdef _WIN32
    case 0:
    case 224:
        char b = _us_getchar();
        switch (b)
        {
        case 72:   return USCharUp;
        case 80:   return USCharDown;
        case 77:   return USCharRight;
        case 75:   return USCharLeft;

        case 115:  return USCharLeftWord;
        case 116:  return USCharRightWord;
        case -115: return USCharUpWord;
        case -111: return USCharDownWord;

        case 71:   return USCharHome;
        case 79:   return USCharEnd;

        case 83:   return USCharDelete;
        case -109: return USCharDeleteWord;

        case 73:   return USCharPageUp;
        case 81:   return USCharPageDown;
        case -122: return USCharPageUpWord;
        case 118:  return USCharPageDownWord;
        case -108: return USCharDetails;

        default:   return USCharUnknown;
        }
        break;
#else
    case 27:
        disableInputWait();
        c = getchar();
        enableInputWait();
        if (c < 0) return 27;
        if (c == '[')
        {
            c = getchar();
            switch (c)
            {
            case 'A': return USCharUp;
            case 'B': return USCharDown;
            case 'C': return USCharRight;
            case 'D': return USCharLeft;
            case 'H': return USCharHome;
            case 'F': return USCharEnd;
            case 'Z': return USCharDetails;

            case '3':
                switch (c = getchar())
                {
                case '~': return USCharDelete;
                case ';':
                    switch (c = getchar())
                    {
                    case '5':
                        switch (c = getchar())
                        {
                        case '~': return USCharDeleteWord;
                        }
                    }
                }

            case '1':
                switch (c = getchar())
                {
                case ';':
                    switch (c = getchar())
                    {
                    case '5':
                        switch (c = getchar())
                        {
                        case 'A': return USCharUpWord;
                        case 'B': return USCharDownWord;
                        case 'C': return USCharRightWord;
                        case 'D': return USCharLeftWord;
                        }
                    }
                }

            case '5':
                switch (c = getchar())
                {
                case '~': return USCharPageUp;
                case ';':
                    switch (c = getchar())
                    {
                    case '5':
                        switch (c = getchar())
                        {
                        case '~': return USCharPageUpWord;
                        }
                    }
                }

            case '6':
                switch (c = getchar())
                {
                case '~': return USCharPageDown;
                case ';':
                    switch (c = getchar())
                    {
                    case '6':
                        switch (c = getchar())
                        {
                        case '~': return USCharPageDownWord;
                        }
                    }
                }
            }
            clearInput();
            return USCharUnknown;
        }
        else
        {
            ungetc(c, stdin);
        }
        break;
#endif

    default: return c;
    }
}