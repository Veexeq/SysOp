#pragma once

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void mode_setup(char);
void sig_default(int);
void sig_mask(int);
void sig_ignore(int);
void sig_handle(int);
void custom_handler(int);
