#pragma config FOSC = INTOSCIO // Oscillator Selection (Internal oscillator)
#pragma config WDTEN = OFF // Watchdog Timer Enable bits (WDT disabled in hardware (SWDTEN ignored))
#pragma config MCLRE = ON // Master Clear Reset Pin Enable (MCLR pin enabled; RE3 input disabled)
#pragma config LVP = OFF // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config ICPRT = OFF // Dedicated In-Circuit Debug/Programming Port Enable (ICPORT disabled)
#include <xc.h>
#include <stdio.h>
#define _XTAL_FREQ 1000000  
unsigned int temp =0x00;
int speed= 0x00;
void Configuracion(void)
{​​​​​​​
//TRISA=0xFF; //Entradas
TRISD=0;
TRISB=0xFF;
ANSELD=0;
//ANSELA=0;
ANSELB=0;
LATBbits.LB0 = 0;
//comunicacion bluetooth
    ANSELC=0;
    TRISC=0xC0;  //Salida digital (Ansel salida digital) 
 
    BAUDCON1 = 0x08; // Configuración del módulo 
    SPBRG1 = 25;     //
    TXSTA1 = 0x24;   //
    SPBRGH1 = 0x00;  // 9600 Baudios razon de boud rate 
    RCSTA1 = 0x90;   // Frecuencia = 1Mh 
//sensor de temperatura
    ADCON0 = 0x05;
    ADCON1 = 0x08;
    ADCON2 = 0xAC;
    VREFCON0=0x90;//Activa el voltaje de referencia 
    
//Motor
 T2CON=0x06; //Postscaler 1:1, prescaler 1:16 //Configuracion (lo único raro es la frecuencia v:) bit 1:0 -> prescaler 16 
 CCP1CON=0x0C; // Selecciona modo PWM 
 CCPR1L=0x0B; // Carga CCPR1L con un valor arbitrario 0000 1011
 
//Habilitación de Interrupcion
 INTCON=0xD0; // Habilita interrupcion externa RBO // 1101 0000
 INTCON3=0xC8; // Habilita interrupción externa RB1, Alta prioridad INT1-INT2 // 1100 0000
 RCONbits.IPEN =1; // Habilita niveles de interrupción   
 
}​​​​​​​
 


void putch(char data) {​​​​​​​
    char Activa;
    Activa = data & 0xF0;
    LATD = Activa | 0x05;
    __delay_us(10);
    LATD = Activa | 0x01;
    __delay_ms(1);
    Activa = data << 4;
    LATD = Activa | 0x05;
    __delay_us(10);
    LATD = Activa | 0x01;    
}​​​​​​​
 
void putcm(char data) {​​​​​​​
    char Activa;
    Activa = data & 0xF0;
    LATD = Activa | 0x04;
    __delay_us(10);
    LATD = Activa;
    __delay_ms(1);
    Activa = data << 4;
    LATD = Activa | 0x04;
    __delay_us(10);
    LATD = Activa;
}​​​​​​​
 
void InicializaLCD(void)
{​​​​​​​
__delay_ms(30);
putcm(0x02);    // Inicializa en modo 4 bits
__delay_ms(1);
//__delay_ms(1);
putcm(0x28);    // Inicializa en 2 líneas 5x7
__delay_ms(1);
//__delay_ms(30);
putcm(0x2C);
__delay_ms(1);
putcm(0x0C);
__delay_ms(1);
putcm(0x06);
__delay_ms(1);
putcm(0x80); //Posiciona el curson en 1,1
__delay_ms(1);
putcm(0x18);
__delay_ms(1);
}​​​​​​​
 
char lectura (void){​​​​​​​
    if(PIR1bits.RCIF == 1)  //
    return RCREG1;         //  Entra en 1 b valor que está entrando de la aplicación 
    else
    return 0;
}​​​​​​​
 
void __interrupt(high_priority) myHiIsr(void)
{​​​​​​​     
    int vel=0x00;
    char Datos=0;
   if((PORTB==0x01)||(temp>=50))
   {​​​​​​​
       char A=5;
       
       while(A>0)
       {​​​​​​​
       putcm(0x80);
       printf(" EMERGENCIA%s");  
       __delay_ms(500);
       A -= 1;
        CCPR1L= vel;
        Datos=vel<<4;
        Datos=Datos&0x30; //Enmascara todos los bits menos 5:4
        CCP1CON = CCP1CON | Datos;
       }​​​​​​​
   putcm(0x01);
   INTCONbits.INT0IF=0;   
   }​​​​​​​
}​​​​​​​
 
int Conversion(void) {​​​​​​​
    
ADCON0bits.GO = 1; 
while (ADCON0bits.GO); 
return  ADRESL + ADRESH*256; // Retorna los 10 bits como int justificado a la derecha }​​​​​​​
}​​​​​​​
 
void main(void)
{​​​​​​​
char Datos = 0;
char value;
int auxt;
 
Configuracion();
InicializaLCD();
__delay_ms(10);
 
while (1) 
{​​​​​​​
//putcm(0x01);
temp= (Conversion()/10); 
putcm(0x80);
printf("  Temper=  %d", temp);      
while(PIR1bits.RC1IF==0);                  // No entra valor si está ocupado
{​​​​​​​
    value=RCREG1;                          //Llega el valor y se lee, se pone en blanco otra vez, entonces siempre se ejecuta el ciclo.
    speed=(value*255)/100;                 // value va de 1 a 100, 11111111 = 255, es la relción para convertir el valor binario 
    
	                                       //CCP1CON hace la modulación por ancho de pulso y utiliza los bits 4 y 5 de CCPR1L 
	
    CCPR1L= speed;                         // solo ocupas bit 4 y 5 y se los mandas a ccp1con, ccpr1l solo se "inicializa" 
    Datos=speed<<4;                        // Corrimiento de 4 bits a la izquierda 11111111 -> 11110000
    Datos=Datos&0x30;                      //Enmascara todos los bits menos 5:4 00110000 & 11110000 (datos) = 00110000 
    CCP1CON = CCP1CON | Datos;             // 00001100 | 00110000 = 00111100
}​​​​​​​
    putcm(0xC2);
    printf(" Potencia= %d",value);
__delay_ms(1000);
putcm(0x01); //Limpia la pantalla cada segundo 
}​​​​​​​
return;
}​​​​​​​



