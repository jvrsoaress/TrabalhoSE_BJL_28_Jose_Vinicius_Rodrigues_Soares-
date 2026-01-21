<img width="100%" src="https://capsule-render.vercel.app/api?type=waving&color=02A6F4&height=120&section=header"/>

<h1 align="center">TinyML MNIST no Raspberry Pi Pico W</h1>

<p align="center">
  <strong>Inferência de Dígitos Manuscritos (MNIST) com Rede Neural Convolucional INT8 embarcada no RP2040</strong>
</p>

<p align="center">
  <a href="#-objetivos-do-projeto">Objetivos</a> •
  <a href="#-tecnologias-utilizadas">Tecnologias</a> •
  <a href="#-fluxo-de-trabalho-e-arquitetura">Arquitetura</a> •
  <a href="#-compilação-e-execução">Como Executar</a> •
  <a href="#-autoria">Autoria</a>
</p>

---

## 🎥 Demonstrações do Projeto

🔹 **Vídeo de Demonstração – Projeto em Funcionamento (Pico W + OLED + IA)** 👉 [Clique aqui para assistir](https://youtu.be/yJ6XPsIjuSo?si=08x0C2GUyspmLZWa)

🔹 **Notebook Google Colab – Treinamento, Avaliação e Conversão do Modelo** 👉 [Acessar Notebook](https://colab.research.google.com/drive/1mdFMdALq4t6T3cWZ-w8TESXvitqpyevu?usp=sharing)

---

## 🎯 Objetivos do Projeto

O objetivo central deste projeto é demonstrar a implementação completa de um pipeline de **TinyML**, partindo do treinamento de um modelo em ambiente de alto desempenho até sua execução embarcada em um microcontrolador com recursos limitados (Edge AI).

Os principais objetivos são:

1.  Treinar uma **Rede Neural Convolucional (CNN)** para classificação do dataset MNIST.
2.  Converter o modelo treinado para **TensorFlow Lite**.
3.  Aplicar **quantização INT8** (pós-treinamento) para otimização embarcada.
4.  Executar inferência local no **Raspberry Pi Pico W (RP2040)**.
5.  Implementar comunicação **PC ↔ Microcontrolador** via USB Serial.
6.  Exibir a predição em um **display OLED SSD1306**.

---

## 🧠 Dataset e Modelo

### O Dataset: MNIST
* **Conteúdo:** Imagens de dígitos manuscritos (0 a 9).
* **Formato:** Tons de cinza.
* **Resolução:** $28 \times 28$ pixels (Total: 784 pixels).
* **Entrada no Hardware:** Vetor *raw* (uint8), sem cabeçalhos ou compressão.

### O Modelo: CNN + Quantização
O treinamento foi realizado no **Google Colab** utilizando TensorFlow/Keras.
* **Etapas:** Carregamento $\to$ Normalização (0-1) $\to$ Treinamento CNN $\to$ Avaliação.
* **Conversão:** O modelo foi exportado para **TensorFlow Lite (.tflite)**.
* **Otimização:** Aplicada quantização **INT8** com *representative dataset* para garantir compatibilidade com o hardware.
* **Formato Final:** O arquivo `.tflite` foi convertido para um array hexadecimal C (`mnist_cnn_int8_model.h`) para inclusão direta no firmware.

---

## 🛠️ Tecnologias Utilizadas

### 🔹 Hardware
* **Microcontrolador:** Raspberry Pi Pico W (RP2040)
* **Display:** OLED SSD1306 (128×64)
* **Interface:** I²C (Display) e USB Serial (Comunicação com PC)

### 🔹 Software & Firmware
* **Linguagem:** C / C++ (Pico SDK)
* **Framework ML:** TensorFlow Lite for Microcontrollers (TFLM)
* **IDE/Compilador:** VS Code, CMake, GCC ARM

### 🔹 Interface PC (Host)
* **Linguagem:** Python 3
* **Bibliotecas:** PySerial, NumPy, TensorFlow/Keras

---

## 🔄 Fluxo de Trabalho e Arquitetura

O sistema opera através da interação entre um script Python no computador e o firmware no RP2040.

### 1. 🐍 Script Python (`enviar_imagem.py`)
Atua como a interface de entrada de dados.
1.  Carrega o dataset MNIST.
2.  Seleciona uma imagem aleatória.
3.  Converte a imagem para um vetor de **784 bytes**.
4.  Envia os dados (0-255) via **USB Serial**.
5.  Aguarda e exibe a resposta da inferência vinda do Pico W.

### 2. ⚙️ Firmware Embarcado (`cnn_mnist.c`)
Responsável pela inteligência local.
1.  **Inicialização:** Configura USB, I²C, Display OLED e o interpretador TFLM.
2.  **Recepção:** Recebe os 784 bytes da imagem.
3.  **Pré-processamento:**
    * Normalização: `pixel_norm = pixel / 255.0`
    * Quantização: Converte float para INT8 conforme os parâmetros do modelo.
4.  **Inferência:** Executa `tflm_invoke()` na CNN.
5.  **Pós-processamento:** Identifica a classe com maior probabilidade (*argmax*).
6.  **Saída:**
    * Envia `Predito: X` via Serial para o PC.
    * Desenha o dígito e o resultado no Display OLED.

---

## 🚀 Compilação e Execução

### Pré-requisitos
* Pico SDK instalado e configurado.
* Python 3 instalado no computador.

---

## 🚀 Execução do Projeto

### Pré-requisitos
* Firmware (`.uf2`) já carregado no Raspberry Pi Pico W.
* Python 3 instalado.

### Executar o Script Python
Identifique a porta serial e execute:


# Instalação das dependências
```bash pip install pyserial numpy tensorflow ```

# Execução da interface
```bash python enviar_imagem.py ```

🎯 Resultado Esperado

No Terminal: [Pico W Disse]: Predito: 7

No OLED: Mensagem "PREDICAO: 7" com interface gráfica.
