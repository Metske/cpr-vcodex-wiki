#pragma once
#include <cstdio>
#define LOG_INF(tag, fmt, ...) printf("INFO: [%s] " fmt "\n", tag, ##__VA_ARGS__)
#define LOG_ERR(tag, fmt, ...) printf("ERR: [%s] " fmt "\n", tag, ##__VA_ARGS__)
