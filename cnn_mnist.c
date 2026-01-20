/**
 * @file cnn_mnist.c
 * @brief Inferência MNIST via USB Serial com Display OLED SSD1306 (Versão Limpa)
 */

#include <stdio.h>
#include <math.h>
#include <string.h> // Para snprintf
#include "pico/stdlib.h"

// Includes do TFLM
#include "tflm_wrapper.h"

// Includes do Display (da pasta lib)
#include "hardware/i2c.h"
#include "lib/ssd1306.h"

// ==========================================================
// CONFIGURAÇÕES DO DISPLAY
// ==========================================================
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

// Tamanho da imagem MNIST
#define IMAGE_SIZE 784 

// Variável global para o display
ssd1306_t ssd;

// Função auxiliar: retorna índice do maior valor
static int argmax_i8(const int8_t* v, int n) {
    int best = 0;
    int8_t bestv = v[0];
    for (int i = 1; i < n; i++) {
        if (v[i] > bestv) { bestv = v[i]; best = i; }
    }
    return best;
}

// Função auxiliar: quantização
static int8_t quantize_f32_to_i8(float x, float scale, int zp) {
    long q = lroundf(x / scale) + zp;
    if (q < -128) q = -128;
    if (q >  127) q = 127;
    return (int8_t)q;
}

int main() {
    stdio_init_all(); // Inicializa USB Serial

    // ==========================================================
    // INICIALIZAÇÃO DO I2C E DISPLAY
    // ==========================================================
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "INICIANDO...", 10, 20);
    ssd1306_send_data(&ssd);

    sleep_ms(2000); // Tempo para conectar serial

    // Inicializa TFLM
    if (tflm_init() != 0) {
        printf("Erro tflm_init\n");
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "ERRO TFLM", 10, 20);
        ssd1306_send_data(&ssd);
        while (true);
    }

    // Ponteiros do modelo
    int in_bytes = 0, out_bytes = 0;
    int8_t* in_tensor = tflm_input_ptr(&in_bytes);
    int8_t* out_tensor = tflm_output_ptr(&out_bytes);
    
    float in_scale = tflm_input_scale();
    int   in_zp    = tflm_input_zero_point();
    // float out_scale = tflm_output_scale(); // Removido pois não vamos usar confiança
    // int   out_zp    = tflm_output_zero_point(); // Removido pois não vamos usar confiança

    uint8_t raw_buffer[IMAGE_SIZE];
    int buffer_index = 0;

    // Tela inicial de espera
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "AGUARDANDO", 15, 10);
    ssd1306_draw_string(&ssd, "DADOS PC...", 15, 25);
    ssd1306_send_data(&ssd);
    printf("Aguardando dados...\n");

    while (true) {
        int c = getchar_timeout_us(100);

        if (c != PICO_ERROR_TIMEOUT) {
            raw_buffer[buffer_index++] = (uint8_t)c;

            // Barra de progresso visual simples no display enquanto carrega
            if (buffer_index % 100 == 0) {
                ssd1306_draw_char(&ssd, '.', (buffer_index/100)*8, 50);
                ssd1306_send_data(&ssd);
            }

            if (buffer_index >= IMAGE_SIZE) {
                printf("Recebido. Processando...\n");
                
                // Limpa display e avisa
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "PROCESSANDO", 10, 20);
                ssd1306_send_data(&ssd);

                // 1. Quantização
                for (int i = 0; i < IMAGE_SIZE; i++) {
                    float pixel_norm = (float)raw_buffer[i] / 255.0f;
                    in_tensor[i] = quantize_f32_to_i8(pixel_norm, in_scale, in_zp);
                }

                // 2. Inferência
                if (tflm_invoke() == 0) {
                    int pred = argmax_i8(out_tensor, 10);
                    
                    // Envia Serial (APENAS O NÚMERO PREDITO)
                    printf("Predito: %d\n", pred);

                    // ==========================================
                    // MOSTRA NO DISPLAY (APENAS O NÚMERO)
                    // ==========================================
                    ssd1306_fill(&ssd, false);
                    
                    // Texto do topo
                    ssd1306_draw_string(&ssd, "PREDICAO:", 25, 5);

                    // Desenha o número GRANDE no meio
                    char pred_str[2];
                    snprintf(pred_str, sizeof(pred_str), "%d", pred);
                    
                    // Desenha o número centralizado (aproximadamente)
                    // x=60, y=25
                    ssd1306_draw_char(&ssd, pred_str[0], 60, 25); 

                    // Desenha borda
                    ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);

                    ssd1306_send_data(&ssd);

                } else {
                    printf("Erro IA\n");
                    ssd1306_fill(&ssd, false);
                    ssd1306_draw_string(&ssd, "ERRO IA", 30, 30);
                    ssd1306_send_data(&ssd);
                }

                buffer_index = 0;
            }
        }
    }
}