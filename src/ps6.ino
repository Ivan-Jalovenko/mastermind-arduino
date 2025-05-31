/* MIT License
 * 
 * Copyright (c) 2025 Ivan Yalovenko
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#include <Arduino.h>
#include <Wire.h>

#include <stdlib.h>

#include "mastermind.h"
#include "lcd_wrapper.h"


void setup() {
  // put your setup code here, to run once:
  lcd_init();

  /*===== Button setup ======*/
  pinMode(BTN_1_PIN, INPUT);
  pinMode(BTN_2_PIN, INPUT);
  pinMode(BTN_3_PIN, INPUT);
  pinMode(BTN_4_PIN, INPUT);
  
  pinMode(BTN_ENTER_PIN, INPUT);

  /*===== LED setup =====*/
  pinMode(LED_BLUE_1, OUTPUT); // LED 1
  pinMode(LED_RED_1, OUTPUT);

  pinMode(LED_BLUE_2, OUTPUT); // LED 2 
  pinMode(LED_RED_2, OUTPUT);
  
  pinMode(LED_BLUE_3, OUTPUT); // LED 3
  pinMode(LED_RED_3, OUTPUT);
  
  pinMode(LED_BLUE_4, OUTPUT); // LED 4
  pinMode(LED_RED_4, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  char* code = generate_code(false, 4);
  play_game(code);
  free(code);

  lcd_clear();
  delay(1000);
}
