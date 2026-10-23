/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_LEDS 5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */

struct buttons{

	GPIO_TypeDef *port;
		uint16_t pin;

		uint32_t prevDebounceTime_ms;
		GPIO_PinState currentState;   // latest raw reading
		GPIO_PinState stableState;    // last debounced state
};

struct LED{
	GPIO_TypeDef *port[2];
		uint16_t pin[5];

		uint32_t prevTimeStamp_ms;
		uint32_t interval;
};

struct GAMELOGIC{
	uint32_t interval;
	uint32_t prevTimeStamp_ms;
	GPIO_PinState state;
	bool pressed;
	uint8_t win;
	uint8_t loss;
	uint8_t count;

	uint8_t Randomizer[3];
	bool lit[5];
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
void gameLogic(struct buttons *btn,struct GAMELOGIC *lgc,struct LED *led,uint32_t TimeStamp_ms);
GPIO_PinState debounce(struct buttons *btn);
bool checkButton(struct buttons *btn);
int startingFlicker(struct LED *led);
unsigned RNG(void);
void defineButton(struct buttons *button,GPIO_TypeDef *port,uint16_t pin);
uint16_t readPot(void);
uint32_t scaleTime(uint16_t pot, uint32_t easyMs, uint32_t hardMs);
void winAnim(struct LED *led);
void lossAnim(struct LED *led);
void defineLED(struct LED *led);
void defineLOGIC(struct GAMELOGIC *lgc);
bool resetGame(struct buttons *btn);
void RESET_L(struct GAMELOGIC *lgc,struct buttons *btn,struct LED *led);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  int gameState = 0;

  struct buttons Buttons[NUM_LEDS];
  struct LED led;
  struct GAMELOGIC lgc;

  defineLOGIC(&lgc);
  //defineButton(&Buttons);
  defineLED(&led);

  defineButton(&Buttons[0], GPIOA, GPIO_PIN_8);
  defineButton(&Buttons[1], GPIOB, GPIO_PIN_15);
  defineButton(&Buttons[2], GPIOB, GPIO_PIN_14);
  defineButton(&Buttons[3], GPIOA, GPIO_PIN_10);
  defineButton(&Buttons[4], GPIOA, GPIO_PIN_11);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if(gameState == 0){

		  startingFlicker(&led);

		  if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_9) == GPIO_PIN_SET){ // Check if user pressed start button
			  HAL_Delay(20);
			  	  gameState = 1;
	  	  }

	  }

	  if(gameState == 1){


		  uint32_t TimeStamp_ms = HAL_GetTick(); // GET CURRENT TIME IN MS (NON BLOCKING)

			  gameLogic(Buttons,&lgc,&led,TimeStamp_ms);


			if (lgc.loss > 2) {
				gameState = 2;
			}

			if (lgc.win > 30) {
				gameState = 3;
			}
	  }

	  if(gameState == 2){
		  lossAnim(&led);
		  if(resetGame(Buttons) == true){
			  RESET_L(&lgc,Buttons,&led);
			  gameState = 0;
		  }
	  }

	  if(gameState == 3){
		  winAnim(&led);
		  if(resetGame(Buttons) == true){
			  	RESET_L(&lgc,Buttons,&led);
		  		gameState = 0;
		  		  }
	  }



    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 PA10 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB5 PB6 PB7 PB8
                           PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void gameLogic(struct buttons *btn,struct GAMELOGIC *lgc,struct LED *led,uint32_t TimeStamp_ms){

	uint8_t size = sizeof(lgc->lit)/sizeof(lgc->lit[0]);

	for(uint8_t i = 0;i < size;i++){
	if(checkButton(&btn[i]) && lgc->lit[i] == true){
		HAL_GPIO_WritePin(GPIOB,led->pin[i], GPIO_PIN_RESET);
		lgc->lit[i] = false;
		lgc->count--;
		}
	}

	if(TimeStamp_ms - lgc->prevTimeStamp_ms >= lgc->interval){
		lgc->prevTimeStamp_ms = TimeStamp_ms;

		uint16_t pot = readPot();
		uint32_t onTime  = scaleTime(pot, 2000, 700);   // LEDs lit: 2 s (easy) to 0.5 s (hard)
		uint32_t offTime = scaleTime(pot, 1000, 350);

		if(lgc->state){

			if (lgc->count == 0) {
						lgc->win += 1;
					} else {
						lgc->loss += 1;
					}

			lgc->pressed = false;
			lgc->count = 0;

			for(uint8_t i =0;i < size;i++){
				lgc->lit[i] = false;
				HAL_GPIO_WritePin(GPIOB,led->pin[i],lgc->lit[i]);
					}
			lgc->state = 0;
			lgc->interval = offTime; //NOT DEFINED YET, WILL BE DEFINED BY DIFFICULTY RATING THING

		}else{

			for(uint8_t i = 0; i <size; i++){
				unsigned int flip = (RNG() % 2);

				if(flip == 0){
					lgc->lit[i] = false;
				}else{
					lgc->lit[i] = true;
					lgc->count++;
				}

				HAL_GPIO_WritePin(GPIOB,led->pin[i],lgc->lit[i]);
			}

			lgc->state = 1;
			lgc->interval = onTime; //SAME AS COMMENT ABOVE

		}
	}

}

GPIO_PinState debounce(struct buttons *btn){

	GPIO_PinState reading = HAL_GPIO_ReadPin(btn->port, btn->pin);

		if(reading != btn->currentState){
			btn->prevDebounceTime_ms = HAL_GetTick();
			btn->currentState = reading;
		}

		if(HAL_GetTick() - btn->prevDebounceTime_ms > 50){
			btn->stableState = btn->currentState;
		}

		return btn->stableState;
}

bool checkButton(struct buttons *btn){

	GPIO_PinState before = btn->stableState;
		GPIO_PinState after = debounce(btn);

		return (before == GPIO_PIN_RESET && after == GPIO_PIN_SET);
}

uint16_t readPot(void){
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 10);
	uint16_t value = HAL_ADC_GetValue(&hadc1);   // 0 to 4095
	HAL_ADC_Stop(&hadc1);
	return value;
}

uint32_t scaleTime(uint16_t pot, uint32_t easyMs, uint32_t hardMs){
	return easyMs - ((easyMs - hardMs) * pot) / 4095;
}

void winAnim(struct LED *led){

	// all off first
	for(uint8_t i = 0; i < NUM_LEDS; i++){
		HAL_GPIO_WritePin(GPIOB, led->pin[i], GPIO_PIN_RESET);
	}

	// one by one, earlier ones stay on
	for(uint8_t i = 0; i < NUM_LEDS; i++){
		HAL_GPIO_WritePin(GPIOB, led->pin[i], GPIO_PIN_SET);
		HAL_Delay(300);
	}

	// blink once: all off, then all on, then all off
	for(uint8_t i = 0; i < NUM_LEDS; i++){
		HAL_GPIO_WritePin(GPIOB, led->pin[i], GPIO_PIN_RESET);
	}
	HAL_Delay(200);

	for(uint8_t i = 0; i < NUM_LEDS; i++){
		HAL_GPIO_WritePin(GPIOB, led->pin[i], GPIO_PIN_SET);
	}
	HAL_Delay(200);

	for(uint8_t i = 0; i < NUM_LEDS; i++){
		HAL_GPIO_WritePin(GPIOB, led->pin[i], GPIO_PIN_RESET);
	}
	HAL_Delay(300);
}

void lossAnim(struct LED *led){

	for(uint8_t i = 0; i < NUM_LEDS; i++){
		HAL_GPIO_WritePin(GPIOB, led->pin[i], GPIO_PIN_SET);
	}
	HAL_Delay(400);

	for(uint8_t i = 0; i < NUM_LEDS; i++){
		HAL_GPIO_WritePin(GPIOB, led->pin[i], GPIO_PIN_RESET);
	}
	HAL_Delay(400);
}

int startingFlicker(struct LED *led){
	GPIO_PinState state = 0;
	uint32_t TimeStamp_ms = HAL_GetTick();



	if(TimeStamp_ms - led->prevTimeStamp_ms >= led->interval){

		led->prevTimeStamp_ms = TimeStamp_ms;

			unsigned int random = (RNG() % 5);
			uint16_t currentpin = led->pin[random];

		if(HAL_GPIO_ReadPin(GPIOB, currentpin) == GPIO_PIN_SET){
			state = 0;
			led->interval = 500;
		}else{
			state = 1;
			led->interval = 2000;
		}
		HAL_GPIO_WritePin(GPIOB,currentpin,state);
	}

	return 0;
}

bool resetGame(struct buttons *btn){

	GPIO_PinState a = debounce(&btn[0]);
	GPIO_PinState b = debounce(&btn[2]);

		return (a == GPIO_PIN_SET && b == GPIO_PIN_SET);
}

unsigned RNG(void){
	static unsigned short lfsr = 0xACE1u;
	unsigned bit;

	 bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
	 return lfsr =  (lfsr >> 1) | (bit << 15);

}

void RESET_L(struct GAMELOGIC *lgc,struct buttons *btn,struct LED *led){
	// RESET VALUES
		lgc->interval = 2000;
		lgc->prevTimeStamp_ms = 0;
		lgc->state = 0;
		lgc->pressed = false;
		lgc->win = 0;
		lgc->loss = 0;
		lgc->count = 0;

		for(uint8_t i = 0; i < NUM_LEDS; i++){
				lgc->lit[i] = false;
				HAL_GPIO_WritePin(GPIOB, led->pin[i], GPIO_PIN_RESET);
			}
}

void defineButton(struct buttons *button,GPIO_TypeDef *port,uint16_t pin){
		button->port = port;
		button->pin = pin;
		button->prevDebounceTime_ms = 0;
		button->currentState = GPIO_PIN_RESET;   // pull-down
		button->stableState = GPIO_PIN_RESET;

}

void defineLED(struct LED *led){
	led->port[0] = GPIOA;
	led->port[1] = GPIOB;

	led->pin[0] = GPIO_PIN_9; // FIRST BLUE LED
	led->pin[1] = GPIO_PIN_8; // SECOND WHITE LED
	led->pin[2] = GPIO_PIN_7; // THIRD RED LED
	led->pin[3] = GPIO_PIN_6; // FOURTH WHITE LED
	led->pin[4] = GPIO_PIN_5; // FIFTH BLUE LED

	led->prevTimeStamp_ms = 0;
	led->interval = 250;
}

void defineLOGIC(struct GAMELOGIC *lgc){
	lgc->interval = 2000;
	lgc->prevTimeStamp_ms = 0;
	lgc->state = 0;
	lgc->pressed = false;
	lgc->win = 0;
	lgc->loss = 0;
	lgc->count = 0;

	for(uint8_t i=0;i<5;i++){
		lgc->lit[i] = false;
	}

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
