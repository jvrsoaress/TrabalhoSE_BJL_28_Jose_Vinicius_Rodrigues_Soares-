import serial
import time
import numpy as np
import tensorflow as tf

# =============================================================================
# CONFIGURAÇÃO
# =============================================================================
SERIAL_PORT = 'COM4'  # <--- VERIFIQUE SE A PORTA ESTÁ CORRETA
BAUD_RATE = 115200

# =============================================================================
# 1. Carregar Dataset
# =============================================================================
print("Carregando dataset MNIST...")
(_, _), (x_test, y_test) = tf.keras.datasets.mnist.load_data()
print("Dataset carregado. Iniciando comunicação...")

try:
    # Abre a porta serial UMA VEZ
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    time.sleep(2) # Espera o Pico reiniciar a conexão USB
    print(f"Conectado na porta {SERIAL_PORT}")
    ser.reset_input_buffer()

    # =========================================================================
    # 2. Loop Principal (Envia imagens repetidamente)
    # =========================================================================
    while True:
        input("\nPressione ENTER para enviar uma nova imagem aleatória (ou Ctrl+C para sair)...")
        
        # Escolhe imagem aleatória
        idx = np.random.randint(0, len(x_test))
        img = x_test[idx]
        label_real = y_test[idx]
        
        # Achata para 784 bytes
        img_flat = img.flatten().astype(np.uint8)

        print(f"--> Enviando Imagem ID: {idx} | Numero Real: {label_real}")
        
        # Envia
        ser.write(img_flat.tobytes())

        # Aguarda e lê a resposta do Pico
        print("Aguardando resposta do Pico...")
        start_time = time.time()
        
        # Loop de leitura da resposta
        while (time.time() - start_time) < 3: # Timeout de 3 segundos
            if ser.in_waiting:
                try:
                    linha = ser.readline().decode('utf-8').strip()
                    if linha:
                        print(f"[Pico W Disse]: {linha}")
                        # Se o Pico confirmou a predição, podemos parar de ler por enquanto
                        if "Predito" in linha:
                            break
                except:
                    pass
            time.sleep(0.01)

except serial.SerialException:
    print(f"\nERRO CRÍTICO: Não foi possível abrir a porta {SERIAL_PORT}.")
    print("DICA: Feche o Monitor Serial do VS Code ou qualquer outro programa usando a porta!")
except KeyboardInterrupt:
    print("\nEncerrando programa...")
    if 'ser' in locals() and ser.is_open:
        ser.close()