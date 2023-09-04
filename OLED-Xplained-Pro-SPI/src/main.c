#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"
#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)


//Led
#define LED1_PIO	PIOA	//Periferico que controla o Led
#define LED1_PIO_ID    ID_PIOA   //ID do periferico PIOC(Controla o Led)
#define LED1_PIO_IDX       0 //ID do Led no PIO
#define LED1_IDX_MASK  (1<<LED1_PIO_IDX) //Mascara para controlar o led

#define LED2_PIO	PIOC
#define LED2_PIO_ID    ID_PIOC
#define LED2_PIO_IDX       30
#define LED2_IDX_MASK  (1<<LED2_PIO_IDX)

#define LED3_PIO	PIOB
#define LED3_PIO_ID    ID_PIOB
#define LED3_PIO_IDX       2
#define LED3_IDX_MASK  (1<<LED3_PIO_IDX)


// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)


//Botao 1
#define BUT1_PIO  PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_IDX_MASK (1u << BUT1_PIO_IDX)

//Botao 2
#define BUT2_PIO  PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_IDX_MASK (1u << BUT2_PIO_IDX)

//Botao 3
#define BUT3_PIO  PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_IDX_MASK (1u << BUT3_PIO_IDX)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
volatile char but_flag = 0;
volatile char but1_flag;

/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void pisca_led(int n, int t);
void pisca_led1(int n, int t);

void but_callback()
{
	but_flag = 1;
}

void but1_callback()
{
	but1_flag = 1;
}


void pisca_led(int n, int t){
	for (int i=0;i<n;i++){
		pio_clear(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
		pio_set(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
	}
}


void pisca_led1(int n, int t){
	for (int i=0;i<n;i++){
		pio_clear(LED1_PIO, LED1_IDX_MASK);
		delay_ms(t);
		pio_set(LED1_PIO, LED1_IDX_MASK);
		delay_ms(t);
	}
}





void io_init(void)

{

	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED1_PIO_ID); //Led 1

	
	
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_IDX_MASK, PIO_DEFAULT);
	

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_IDX_MASK, 60);
	
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_IDX_MASK, 60);
	

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_IDX_MASK, PIO_IT_EDGE, but_callback);
	
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_IDX_MASK, PIO_IT_FALL_EDGE,but1_callback);
	
	

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 5
	
}



int main (void)
{
	int delay  = 300;
	char freq[5];
	board_init();
	sysclk_init();
	delay_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;

	// configura botao com interrupcao
	io_init();

  // Init OLED
	gfx_mono_ssd1306_init();
  
	gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	sprintf(freq, "%d", delay);
	gfx_mono_draw_string(freq, 40,0, &sysfont);
  

  /* Insert application code here, after the board has been initialized. */
	while(1) {
		
			if(but1_flag){
				delay_ms(1000);
				if (pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK)){
					if (delay>100){
						delay -= 100;
						gfx_mono_draw_string("-", 70,0, &sysfont);
						delay_ms(500);
						gfx_mono_draw_string(" ", 70,0, &sysfont);
					}
					
				}
				else
				{
					delay += 100;
					gfx_mono_draw_string("+", 70,0, &sysfont);
					delay_ms(500);
					gfx_mono_draw_string(" ", 70,0, &sysfont);
					
				}
				sprintf(freq, "%d", delay);
				gfx_mono_draw_string(freq, 40,0, &sysfont);
				
				pisca_led1(10, delay);
				but1_flag = 0;
			}
			
			// Escreve na tela um circulo e um texto
			
			
		
			
			
	
	}
	
}
