#ifndef EDGEFLOW_LOGGER_H
#define EDGEFLOW_LOGGER_H

int logger_init(const char *dir);
void logger_shutdown(void);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif
