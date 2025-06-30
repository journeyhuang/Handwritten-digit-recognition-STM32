# Embedded AI Handwriting Recognition System on STM32

![Project Demo](https://img.shields.io/badge/Platform-STM32F407-blue.svg)
![Framework](https://img.shields.io/badge/AI%20Framework-PyTorch%20%7C%20ONNX-orange.svg)
![Toolchain](https://img.shields.io/badge/Toolchain-STM32CubeAI-brightgreen.svg)
![Status](https://img.shields.io/badge/Status-Completed-success.svg)

A final project for the "Microcontrollers and Embedded Systems" course. This project implements a real-time handwriting recognition system on an STM32F407 microcontroller, featuring a touchscreen for input and a full user interface for interaction.

## 🌟 Project Highlights

[cite_start]This project was designed to meet several key objectives from the course requirements document[cite: 1], focusing on **Technical Depth**, **Code Readability**, and **Innovative Features**.

* [cite_start]**High-Performance AI Core:** Deploys a quantized INT8 neural network model, converted from PyTorch, ensuring high-speed inference and low memory footprint suitable for MCU constraints. [cite: 1]
* **Professional Architecture:** The C code is structured with a non-blocking, event-driven state machine, ensuring a responsive and fluid user interface without reliance on blocking `HAL_Delay()` calls.
* **Rich User Experience:** Features a polished graphical user interface (GUI) with:
    * [cite_start]A smooth, interpolated drawing canvas. [cite: 1]
    * A real-time input preview panel.
    * Clear, interactive buttons with visual feedback.
    * Detailed status updates and results, including prediction confidence.
* [cite_start]**Innovative Eraser Mode:** An extra feature allowing users to edit their input by erasing parts of the drawn digit, enhancing usability. [cite: 1]
* **Robustness Features:** Includes input validation to prevent prediction on empty canvases and an auto-clear timeout for the drawing area.

## 🛠️ Technology Stack

* **Hardware:**
    * **MCU:** STM32F407
    * **Development Board:** [Your Board Name, e.g., "Zhengdian Atom Explorer"]
    * **Display:** [Your LCD Model, e.g., "3.5-inch 320x480 Resistive Touch LCD"]
    * **Peripherals:** Buzzer, LEDs for status indication.

* **Software & AI Workflow:**
    * **AI Framework:** **PyTorch** for model training, exported to **ONNX**.
    * **Deployment Tool:** **STMicroelectronics X-CUBE-AI** for model conversion and optimization into C-code for the STM32.
    * **Embedded Development:** C language, using STM32 HAL libraries within the **Keil MDK** or **STM32CubeIDE** environment.

## 📋 Features

* **Handwriting Input:** Users can draw a single digit (0-9) in the designated input area on the touchscreen.
* **Smooth Drawing:** Implements Bresenham's line algorithm to interpolate between touch points, creating a continuous line even with fast movements.
* **Live Preview:** A smaller, 28x28 pixel preview window shows a down-scaled version of the input, visualizing exactly what the neural network will "see".
* **Real-time Recognition:** Pressing the "Predict" button triggers the onboard AI model to perform inference.
* **Detailed Results:** The system displays the top prediction with its confidence level (e.g., "Result: 7 (98.2%)").
* **Interactive Controls:**
    * **Clear:** Wipes the drawing canvas.
    * **Eraser:** Toggles a mode to erase parts of the drawing.
    * **Reset:** Resets the entire UI to its initial state.
* **User Feedback:** Provides instant auditory (buzzer) and visual (LEDs, on-screen status) feedback for all actions, such as writing, processing, success, or errors.

## 🚀 How It Works

1.  **Model Training (PC):** A Convolutional Neural Network (CNN) is built and trained in PyTorch on the MNIST dataset. The trained model is then exported to the standard ONNX format.
2.  **Model Conversion (STM32CubeAI):** The ONNX model is fed into ST's X-CUBE-AI tool. It analyzes the model, converts it to optimized C code, and quantizes the weights and activations from `float32` to `int8`. This step is crucial for achieving high performance on the MCU.
3.  **Data Acquisition (STM32):** The user's drawing on the resistive touchscreen is captured as a series of (x, y) coordinates.
4.  **Preprocessing (STM32):**
    * The coordinates are mapped from the 192x192 pixel input area to a 28x28 grid.
    * This 28x28 grid is populated and then **quantized**; the floating-point values (0.0 or 1.0) are converted into `int8` values using the scale and zero-point parameters from the AI model. This `int8_t` array is the input for the neural network.
5.  **Inference (STM32):** The `MX_X_CUBE_AI_Process()` function is called, which executes the neural network with the prepared `int8_t` input buffer. The output is also an `int8_t` array.
6.  **Post-processing (STM32):**
    * The `int8_t` output array is **de-quantized** back into `float` values, which represent the probabilities (confidences) for each digit (0-9).
    * The highest probability is identified to determine the final prediction.
7.  **Display:** The result is displayed on the LCD screen.

## ⚙️ How to Use

1.  **Hardware Setup:**
    * Connect the LCD and touch panel to the STM32F407 development board.
    * Power the board via USB.

2.  **Build and Flash:**
    * Clone this repository.
    * Open the project in your IDE (Keil MDK / STM32CubeIDE).
    * Re-generate the AI code with your own model using the provided `.ioc` file and STM32CubeMX if needed.
    * Build the project and flash the resulting binary to the STM32 board.

3.  **Operation:**
    * The system will boot up and display the main interface.
    * Draw a digit in the large "Input Area".
    * Press the "Predict" button to see the result.
    * Use "Clear" or "Eraser" to modify your input.
