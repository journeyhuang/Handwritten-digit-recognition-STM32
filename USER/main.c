#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "usmart.h"
#include "touch.h"
#include "app_x-cube-ai.h"
#include <stdio.h>

// ?????????
float in_buffer[784] = {0};
float out_buffer[10] = {0};

// ???????UI??(??320x480)
void Load_Drow_Dialog(void)
{
    LCD_Clear(WHITE); // ??
    POINT_COLOR = BLUE; // ??????
    BACK_COLOR = WHITE; // ????

    // ??????(192x192??,28x28???7?)
    LCD_DrawRectangle(20, 20, 212, 212);
    LCD_ShowString(20, 220, 200, 16, 16, "Input Area");

    // ??????(96x96??,28x28???3.43?)
    LCD_DrawRectangle(220, 20, 316, 116);
    LCD_ShowString(220, 124, 200, 16, 16, "Preview");

    // ??????(80x40??)
    LCD_Fill(20, 260, 100, 300, BLUE); // Clear??
    LCD_ShowString(30, 270, 80, 16, 16, "Clear");
    LCD_Fill(120, 260, 200, 300, BLUE); // Predict??
    LCD_ShowString(130, 270, 80, 16, 16, "Predict");
    LCD_Fill(220, 260, 300, 300, BLUE); // Reset??
    LCD_ShowString(230, 270, 80, 16, 16, "Reset");

    // ?????????
    LCD_ShowString(20, 320, 300, 16, 16, "Status: Ready");
    LCD_ShowString(20, 350, 300, 24, 24, "Result: -");
}

// ??????(28x28?????3.43?)
void Update_Preview(void)
{
    LCD_Fill(220, 20, 316, 116, WHITE); // ?????
    LCD_DrawRectangle(220, 20, 316, 116);
    for (int y = 0; y < 28; y++)
    {
        for (int x = 0; x < 28; x++)
        {
            if (in_buffer[y * 28 + x] > 0.5)
            {
                LCD_Fill(220 + x * 3, 20 + y * 3, 223 + x * 3, 23 + y * 3, BLACK);
            }
        }
    }
}

// ????,????????
void Interpolate_Points(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        if (x1 >= 20 && x1 < 212 && y1 >= 20 && y1 < 212)
        {
            LCD_Fast_DrawPoint(x1, y1, RED);
            int in_x = (x1 - 20) / 7; // 192/28 ˜ 7
            int in_y = (y1 - 20) / 7;
            if (in_x >= 0 && in_x < 28 && in_y >= 0 && in_y < 28)
            {
                in_buffer[in_y * 28 + in_x] = 1.0;
            }
        }
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
    Update_Preview();
}

// ??????????
void rtp_test(void)
{
    u8 key;
    u8 i = 0;
    float max_value = 0;
    uint32_t max_index = 0;
    static u8 state = 0; // 0: ????, 1: ???, 2: ????
    static int last_x = -1, last_y = -1; // ?????
    static uint32_t last_touch_time = 0; // ??????

    while (1)
    {
        key = KEY_Scan(0);
        tp_dev.scan(0);

        // ??????(10????????)
        if (state == 0 && HAL_GetTick() - last_touch_time > 10000)
        {
            LCD_Fill(20, 20, 212, 212, WHITE);
            LCD_DrawRectangle(20, 20, 212, 212);
            for (int i = 0; i < 784; i++) in_buffer[i] = 0.0;
            Update_Preview();
            LCD_ShowString(20, 320, 300, 16, 16, "Status: Timeout, Cleared");
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // ?????
            HAL_Delay(200);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
            LCD_ShowString(20, 320, 300, 16, 16, "Status: Ready");
            state = 0;
            last_x = -1;
            last_y = -1;
        }

        if (tp_dev.sta & TP_PRES_DOWN) // ??????
        {
            if (tp_dev.x[0] < lcddev.width && tp_dev.y[0] < lcddev.height)
            {
                last_touch_time = HAL_GetTick();

                // Clear?? (20,260,100,300)
                if (tp_dev.x[0] > 20 && tp_dev.x[0] < 100 && tp_dev.y[0] > 260 && tp_dev.y[0] < 300)
                {
                    LCD_Fill(20, 20, 212, 212, WHITE); // ??????
                    LCD_DrawRectangle(20, 20, 212, 212);
                    for (int i = 0; i < 784; i++) in_buffer[i] = 0.0;
                    Update_Preview();
                    LCD_ShowString(20, 320, 300, 16, 16, "Status: Cleared");
                    LCD_ShowString(20, 350, 300, 24, 24, "Result: -");
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // ?????
                    HAL_Delay(200);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                    state = 0;
                    last_x = -1;
                    last_y = -1;
                    LED1 = 1;
                }
                // Predict?? (120,260,200,300)
                else if (tp_dev.x[0] > 120 && tp_dev.x[0] < 200 && tp_dev.y[0] > 260 && tp_dev.y[0] < 300)
                {
                    // ???????
                    int pixel_count = 0;
                    for (int i = 0; i < 784; i++) if (in_buffer[i] > 0) pixel_count++;
                    if (pixel_count < 10)
                    {
                        LCD_ShowString(20, 320, 300, 16, 16, "Status: Insufficient Input");
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // ?????
                        HAL_Delay(100);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                        HAL_Delay(100);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
                        HAL_Delay(100);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                        state = 0; // ??????
                        continue;
                    }
                    LCD_ShowString(20, 320, 300, 16, 16, "Status: Recognizing...");
                    LCD_Fill(20, 230, 220, 240, GREEN); // ???????
                    LED1 = 0; // ??LED1
                    state = 1;

                    // AI ???????
                    uint32_t start_time = HAL_GetTick();
                    int res = MX_X_CUBE_AI_Process();
                    uint32_t ai_time = HAL_GetTick() - start_time;
                    printf("AI Process Time: %ld ms, Result: %d\n", ai_time, res); // ????

                    if (res == 0) // ????
                    {
                        max_value = 0;
                        max_index = 0;
                        for (int i = 0; i < 10; i++)
                        {
                            if (out_buffer[i] > max_value)
                            {
                                max_value = out_buffer[i];
                                max_index = i;
                            }
                            printf("out_buffer[%d]: %.4f\n", i, out_buffer[i]); // ????
                        }
                        char result[20];
                        sprintf(result, "Result: %d (%.2f%%)", max_index, max_value * 100);
                        POINT_COLOR = GREEN; // ?????
                        LCD_ShowString(20, 350, 300, 24, 24, result);
                        LCD_ShowString(20, 320, 300, 16, 16, "Status: Completed");
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // ????
                        HAL_Delay(100);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                        LED1 = 0;
                        HAL_Delay(1000); // LED1??1?
                        LED1 = 1;
                    }
                    else // ????
                    {
                        POINT_COLOR = RED; // ?????
                        LCD_ShowString(20, 350, 300, 24, 24, "Result: Error");
                        LCD_ShowString(20, 320, 300, 16, 16, "Status: AI Error");
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // ?????
                        HAL_Delay(100);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                        HAL_Delay(100);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
                        HAL_Delay(100);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                        LED1 = 1;
                    }
                    POINT_COLOR = BLUE; // ??????
                    state = 0; // ??????
                    // ?????????
                    tp_dev.sta = 0;
                    tp_dev.x[0] = 0;
                    tp_dev.y[0] = 0;
                }
                // Reset?? (220,260,300,300)
                else if (tp_dev.x[0] > 220 && tp_dev.x[0] < 300 && tp_dev.y[0] > 260 && tp_dev.y[0] < 300)
                {
                    Load_Drow_Dialog();
                    for (int i = 0; i < 784; i++) in_buffer[i] = 0.0;
                    for (int i = 0; i < 10; i++) out_buffer[i] = 0.0;
                    Update_Preview();
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // ?????
                    HAL_Delay(200);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                    state = 0;
                    last_x = -1;
                    last_y = -1;
                    LED1 = 1;
                }
                // ???? (20,20,212,212)
                else if (tp_dev.x[0] > 20 && tp_dev.x[0] < 212 && tp_dev.y[0] > 20 && tp_dev.y[0] < 212)
                {
                    int x = tp_dev.x[0], y = tp_dev.y[0];
                    if (last_x != -1 && last_y != -1)
                    {
                        Interpolate_Points(last_x, last_y, x, y); // ????
                    }
                    else
                    {
                        LCD_Fast_DrawPoint(x, y, RED);
                        int in_x = (x - 20) / 7;
                        int in_y = (y - 20) / 7;
                        if (in_x >= 0 && in_x < 28 && in_y >= 0 && in_y < 28)
                        {
                            in_buffer[in_y * 28 + in_x] = 1.0;
                        }
                        Update_Preview();
                    }
                    last_x = x;
                    last_y = y;
                    LCD_ShowString(20, 320, 300, 16, 16, "Status: Writing...");
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // ?????
                    HAL_Delay(50);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                    LED1 = !LED1; // LED1??
                    state = 0;
                }
            }
        }
        else
        {
            last_x = -1;
            last_y = -1; // ?????
            delay_ms(10);
        }

        if (key == KEY0_PRES) // KEY0?????
        {
            LCD_Clear(WHITE);
            TP_Adjust();
            TP_Save_Adjdata();
            Load_Drow_Dialog();
            for (int i = 0; i < 784; i++) in_buffer[i] = 0.0;
            Update_Preview();
            state = 0;
        }

        i++;
        if (i % 20 == 0) LED0 = !LED0; // LED0?200ms??
    }
}

int main(void)
{
    HAL_Init();
    Stm32_Clock_Init(336, 8, 2, 7); // ????168MHz
    delay_init(168);
    uart_init(115200);
    usmart_dev.init(84);
    LED_Init();
    KEY_Init();
    LCD_Init();
    tp_dev.init();
    POINT_COLOR = RED;
    // ??????(PB0)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // ????:???????
    printf("System Initialized\n");

    Load_Drow_Dialog();
    MX_X_CUBE_AI_Init();
    printf("AI Initialized\n");
    rtp_test();
}