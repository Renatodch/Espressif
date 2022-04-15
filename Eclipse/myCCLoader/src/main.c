/*
 * main.c
 *
 *  Created on: 2 set. 2020
 *      Author: Renato
 */

#include "main.h"
#include "periph.h"
#include "dev.h"

int main(void)
{
	periphInit();

	while(1)

		loop();
}
