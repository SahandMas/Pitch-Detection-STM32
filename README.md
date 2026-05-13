#  Real-Time Voice Pitch Detection on STM32F401RE

A real-time Digital Signal Processing (DSP) system for detecting the **fundamental frequency (pitch)** of the human voice using the **Autocorrelation Method** on the **STM32F401RE** microcontroller.

This project was developed for the **Microprocessors and Assembly Language Course**  
(Academic Year **2025–2026**).

---

# 📌 Project Overview

This system captures analog voice signals through the ADC peripheral of the STM32F401RE and processes them in real time to estimate the speaker’s pitch within the human vocal range (**85 Hz – 350 Hz**).

The detected frequency is mapped to the nearest musical note and displayed on a **16x2 Character LCD**.

The implementation focuses on:

- Real-time signal acquisition
- Time-domain DSP processing
- Low-level embedded programming
- Efficient memory management
- Register-level peripheral control without HAL abstraction

---

#  Features

## 🔹 Real-Time Pitch Detection
Detects the fundamental frequency of human speech using the **Autocorrelation Function (ACF)**.

## 🔹 Low-Level Peripheral Programming
LCD communication is implemented entirely using custom drivers in both:

- 4-bit mode
- 8-bit mode

No HAL LCD libraries or external drivers are used.

## 🔹 Circular Buffer (Ring Buffer)
Efficient FIFO-based sample storage for continuous ADC acquisition.

## 🔹 Signal Normalization
Input amplitudes are normalized before autocorrelation to improve resolution and stability.

## 🔹 Musical Note Mapping
Converts detected frequencies into corresponding musical notes such as:

- A4
- C3
- E2
- etc.

## 🔹 Proteus Simulation Support
Includes a complete Proteus project for simulation and testing.

---

##  Mathematical Background

The system uses the **Autocorrelation Function** to detect periodicity in quasi-periodic voice signals.

The autocorrelation of the discrete signal $x(n)$ is defined as:

```latex
R_x(l) = \sum_{n \in Z} x(n)x(n-l)
```

The pitch period is determined by finding the first significant non-zero peak in the autocorrelation output.

The pitch frequency is then calculated using:

```latex
f_{pitch} = \frac{f_s}{l_{peak}}
```

Where:

- $f_s$ → Sampling frequency  
- $l_{peak}$ → Lag corresponding to the first autocorrelation peak
---

#  Hardware Specifications

| Component | Description |
|---|---|
| Microcontroller | STM32F401RE (ARM Cortex-M4) |
| Display | 16x2 LCD (HD44780 Compatible) |
| Input Source | Microphone / Function Generator |
| Development Tools | STM32CubeIDE / Keil MDK |
| Simulation | Proteus Design Suite |

---

# ⚙️ Software Design

## 1️⃣ Sampling Stage
The ADC continuously samples the analog input signal and stores the data inside a circular FIFO buffer.

## 2️⃣ DSP Processing
The system computes the autocorrelation function over a fixed analysis window.

## 3️⃣ Peak Detection
Noise and insignificant peaks are ignored using threshold-based logic to identify the first valid autocorrelation peak.

## 4️⃣ Frequency Estimation
The lag value is converted into the estimated pitch frequency.

## 5️⃣ Musical Note Detection
The frequency is mapped to the nearest musical note.

## 6️⃣ LCD Display
Detected pitch and note information are displayed on the LCD in real time.

---

# 📂 Repository Structure

```text
├── main core/
├── 4-bit LCD driver
├── Core DSP algorithms
│   ├── System initialization
│   └── Main application logic
│
├── 8bit mode LCD manipulation/
│   ├── 8-bit LCD driver
│   └── Register-level LCD routines
│
├── proteus project/
│   ├── Proteus schematic
│   └── Simulation files (.pdsprj)
│
├── audio files/
│   ├── Sample WAV files
│   └── Test audio signals
│
├── Final technical report
    └── Design specifications
```

# Additional Documentation 
The complete project description, implementation details, mathematical analysis, hardware design, and final report are available in the PDF documents included in the repository.
