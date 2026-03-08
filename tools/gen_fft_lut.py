#!/usr/bin/env python3
import math

def generate_rfft_tables(fft_size):
    # For an N-point Real FFT, the tables are size N/2
    n = fft_size // 2
    
    pA = [0.0] * (2 * n)
    pB = [0.0] * (2 * n)
    
    # Calculate float values based on your formulas
    for i in range(n):
        # Table A
        pA[2 * i]     = 0.5 * (1.0 - math.sin(2 * math.pi / (2 * n) * i))
        pA[2 * i + 1] = 0.5 * (-1.0 * math.cos(2 * math.pi / (2 * n) * i))
        
        # Table B
        pB[2 * i]     = 0.5 * (1.0 + math.sin(2 * math.pi / (2 * n) * i))
        pB[2 * i + 1] = 0.5 * (1.0 * math.cos(2 * math.pi / (2 * n) * i))

    def to_q15(val):
        # Convert to Q15: round(val * 2^15)
        # Clamp to ensure it stays within int16 range [-32768, 32767]
        qval = int(round(val * 32768))
        return max(-32768, min(32767, qval))

    def format_array(name, data):
        s = f"const q15_t {name}[{len(data)}] = {{\n    "
        for i, val in enumerate(data):
            s += f"{to_q15(val):6d},"
            if (i + 1) % 8 == 0:
                s += "\n    "
        s = s.rstrip(", \n") + "\n};\n"
        return s

    print(f"/* Generated Tables for {fft_size} point Real FFT (n={n}) */\n")
    print("#include <zephyr/types.h>")
    print("typedef int16_t q15_t;\n")
    print(format_array("my_realCoefAQ15", pA))
    print(format_array("my_realCoefBQ15", pB))

# Change this to your desired Real FFT size (e.g., 256, 512, 1024)
generate_rfft_tables(256)
