#ifndef GENERAL_H
#define GENERAL_H

#ifdef _WIN32
#define clearscreen system("cls")
#else
#define clearscreen system("clear")
#endif

#endif // GENERAL_H