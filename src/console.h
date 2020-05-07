#ifndef GASMIC_CONSOLE_H
#define GASMIC_CONSOLE_H

#ifdef _WIN32
int console_is_init(void);
int init_console(void);
int restore_console(void);
#endif

#endif /* !defined(GASMIC_CONSOLE_H) */

