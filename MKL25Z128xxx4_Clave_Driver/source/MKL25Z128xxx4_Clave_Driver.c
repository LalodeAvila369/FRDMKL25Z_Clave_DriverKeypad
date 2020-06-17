#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include <string.h>
#include "fsl_tpm.h"
#include "keypad.h"

void DelayTPM();

keypad k;
int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    //TPM Configuración
	tpm_config_t config;
	TPM_GetDefaultConfig(&config);
	config.prescale= kTPM_Prescale_Divide_128;
	TPM_Init(TPM0, &config);
	TPM_Init(TPM1, &config);

	//Keypad Configuración
	keypad k;
	keypad_config config1;
	get_default_keypad_config(&config1);
	set_keypad_config(&k, &config1);
	char key=0, anterior;



    uint8_t Print_enable=0;
    uint8_t estado =0;

	int intentos=3;
	int Counter= 0;



	char RPassword[12]={'A','C','A','B','A','D','A','1','3','7','9',0};
	char InPassword[12]={'0','0','0','0','0','0','0','0','0','0','0',0};
	char reset[12]={'0','0','0','0','0','0','0','0','0','0','0',0};

    while(1){
    	anterior=key;
		key=read_keypad(&k);
		switch (estado){

		case 0:

			if(Print_enable ==0){
				printf("\nPara introducir clave presionar tecla #");
				Print_enable=1;
			}
			if(key == 0 && anterior != 0){
				key=anterior;

				if(key == '#'){							//Cambio de Contraseña
					Print_enable=0;
					estado=1;
					DelayTPM();
				}
				key=0;
			}

		break;

		case 1:

			if(Print_enable==0){
				printf("\nClave: ");
				Print_enable=1;
			}

			if(key==0 && anterior!=0){
				key=anterior;
				if(key != '#'){
					InPassword[Counter]=key;
					printf("*");
					Counter++;
					DelayTPM();

				}
				else if(key=='#'){
					DelayTPM();
					if(Counter < 11){
						printf("\nFaltan Caracteres\n");
						Print_enable=0;
						strcpy(InPassword,reset);
						Counter=0;

					}else if (Counter == 11){
						if(strcmp(InPassword,RPassword)==0){
							strcpy(RPassword,InPassword);
							strcpy(InPassword,reset);
							printf("\nClave Correcta\n");
							estado=0;
							Print_enable=0;
							Counter=0;
							intentos=3;

						}else{
							intentos=intentos-1;
							printf("\nClave Incorrecta\n");
							if(intentos==0){
								printf("\nIntentos Agotados\n");
								Print_enable=0;
								strcpy(InPassword,reset);
								intentos=3;
								estado=0;
							}else{
								printf("\nIntentos restantes: %d \n",intentos);
								Print_enable=0;
								strcpy(InPassword,reset);
								Counter=0;
							}
						}

					}else if(Counter > 11){
						printf("\nSobran caracteres, clave de 11 digitos\n");
						Print_enable=0;
						strcpy(InPassword,reset);
						Counter=0;
					}

				}
				key=0;
			}
		break;
		}


	}

}
void DelayTPM(){
	uint32_t Mask= 1u<<8u;
	uint32_t Mask_Off = Mask;

	TPM_SetTimerPeriod(TPM1, 100u);
	TPM_StartTimer(TPM1, kTPM_SystemClock);
	while(!(TPM1->STATUS & Mask)){      //Wait
	}

	if(TPM1->STATUS & Mask){
		TPM1->STATUS &=Mask_Off;
		TPM_StopTimer(TPM1);
		TPM1->CNT=0;
	}
}

