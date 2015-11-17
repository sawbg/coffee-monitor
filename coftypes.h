#ifndef COFFEE_HEADER_H
#define COFFEE_HEADER_H

typedef enum {BREWING = 0, FRESH = 10, GOOD = 45, FAIR = 90, OLD = 120, STALE = 180} Status;
typedef struct {
  unsigned int atdVal;
  unsigned long time;
  float resistance;
  float temperature;
  float voltage;
  Status status;
} InfoBox;

#endif
