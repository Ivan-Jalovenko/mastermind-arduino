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

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lcd_wrapper.h"
#include "mastermind.h"


#define LEDS_NUMBER 4
#define BUTTONS_NUMBER 4
#define GAME_ELEMENTS 4
#define MAX_ATTEMPTS 10


bool number_is_present(int num, char *code) {
    if (!code) { return false; }

    for (int i = 0; code[i] != '\0'; ++i) {
        if (code[i] == num + '0') { return true; }
    }

    return false;
}

char* generate_code(bool repeat, int length) {
    // if lenght > 9 and repeat is false return NULL
    if (length < 1) { return NULL; }
    if (!repeat && length > 9) { return NULL; }

    char *code = malloc(sizeof(char) * (length + 1));
    if (!code) { return NULL; }
    else { code[length] = '\0'; }

    for (int i = 0; i < length; ++i) {
        int num;
        do {
            num = rand() % 10;
        } while(!repeat && number_is_present(num, code));
        code[i] = num + '0';
    }

    return code;
}

void get_score(const char *secret, const char *guess, int *peg_a, int *peg_b) {
    if (!secret || !guess || !peg_a || !peg_b) return;
    
    *peg_a = 0;
    *peg_b = 0;
    
    bool secret_counts[10] = {false};
    bool guess_counts[10] = {false};
    for (int i = 0; secret[i] != '\0' && guess[i] != '\0'; ++i) {
        if (guess[i] == secret[i]) { *peg_a += 1; }
        else {
            ++secret_counts[secret[i] - '0'];
            ++guess_counts[guess[i] - '0'];
        }
    }

    for (int i = 0; i < 10; ++i) {
        *peg_b += (secret_counts[i] < guess_counts[i]) ? secret_counts[i] : guess_counts[i];
    }
}

void render_leds(const int peg_a, const int peg_b) {
    turn_off_leds();
    
    const int red_leds[] = {LED_RED_1, LED_RED_2, LED_RED_3, LED_RED_4};
    const int blue_leds[] = {LED_BLUE_1, LED_BLUE_2, LED_BLUE_3, LED_BLUE_4};

    for(int i=0; i < LEDS_NUMBER; ++i) {
        if (i < peg_a) { digitalWrite(blue_leds[i], HIGH); }
        else if ((i - peg_a) < peg_b) { digitalWrite(red_leds[i], HIGH); }
    }
}

void render_history(char *secret, char **history, const int entry) {
    lcd_clear();
    
    lcd_set_cursor(0, 0);

    char str[3]; // 2 digits + 1 terminator
    sprintf(str, "%02d\0", entry + 1);
    
    /*===== Print previous entry =====*/
    lcd_print_at(0, 0, str);
    lcd_print_at(2, 0, ": ");
    lcd_print_at(4, 0, history[entry]);
    
    /*===== Print analysis of previous entry =====*/
    int peg_a = 0, peg_b = 0;
    get_score(secret, history[entry], &peg_a, &peg_b);
    render_leds(peg_a, peg_b);

    char peg_a_str[2] = {peg_a + '0', '\0'};
    lcd_print_at(12, 0, peg_a_str);
    delay(50);
    lcd_print_at(13, 0, "A");
    delay(50);
    
    char peg_b_str[2] = {peg_b + '0', '\0'};
    lcd_print_at(14, 0, peg_b_str);
    delay(50);
    lcd_print_at(15, 0, "B");
}

void turn_off_leds() {
    digitalWrite(LED_BLUE_1, LOW); // first LED
    digitalWrite(LED_RED_1, LOW);
    
    digitalWrite(LED_BLUE_2, LOW); // second LED
    digitalWrite(LED_RED_2, LOW);
    
    digitalWrite(LED_BLUE_3, LOW); // third LED
    digitalWrite(LED_RED_3, LOW);
    
    digitalWrite(LED_BLUE_4, LOW); // forth LED
    digitalWrite(LED_RED_4, LOW);
}

void play_game(char *code) {
    /*
     * - Greet user and wait till he press enter
     * - While attempts < 10 && all LEDs guessed
     * | -- = If attempt == 0 skip 
     * | -- = Else render_history of previous attempt 
     * | -- = While ENTER_BUTTON is not pressed
     * | -- = -- = If INCREMENT_BUTTON is pressed increment corresponding variable
     * | -- = -- = If update detected update value of variable on LCD
     * | -- = While ENTER_BUTTON is pressed -- = -- Do nothing
     * | -- = Get score and render LEDS
     * | -- = Update history
     * - If all LEDs guessed greet player
     * - Else mock player for being bad at the game
     */

    lcd_clear();
    turn_off_leds();

    /*===== User greetings =====*/
    lcd_print_at(0, 0, "Welcome user!");
    lcd_print_at(0, 1, code); // "Guess my comb:" 
    while(digitalRead(BTN_ENTER_PIN) == LOW) {} // wait for player to start the game
    while(digitalRead(BTN_ENTER_PIN) == HIGH) {} // prevent debounce & waste of attempts

    /*===== Game setup =====*/
    char** history = malloc(sizeof(char*) * 10); 
    int attempt = 0;
    int peg_a = 0, peg_b = 0;

    while((attempt < MAX_ATTEMPTS) && (peg_a != 4)) {
        /*===== Round setup =====*/
        lcd_clear();
        
        char *entry = malloc(sizeof(char) * (GAME_ELEMENTS + 1)); // buttons number + terminator
        strcpy(entry, "0000\0");

        const int buttons[] = {
            BTN_1_PIN,
            BTN_2_PIN, 
            BTN_3_PIN,
            BTN_4_PIN
        };

        if (!attempt) { lcd_print_at(0, 0, "Guess my code:"); }
        else { render_history(code, history, attempt - 1); }

        lcd_print_at(0, 1, "Your guess: ");
        delay(200);
        lcd_print_at(12, 1, entry);
        
        /*===== Input logic =====*/
        while (digitalRead(BTN_ENTER_PIN) == LOW) {
            bool need_update = false;
            for (int i=0; i < BUTTONS_NUMBER; ++i) {
                if (digitalRead(buttons[i]) == HIGH) {
                    if (entry[i] == '9') { entry[i] = '0'; }
                    else { ++entry[i]; }

                    need_update = true;
                    while(digitalRead(buttons[i]) == HIGH) { delay(100); } // prevent debounce & rapid increments
                    
                    break;
                }
            }
            delay(100);

            if (need_update) { lcd_print_at(12,1,entry); }
        }
        while (digitalRead(BTN_ENTER_PIN) == HIGH); // prevent debounce & rapid incements

        /*===== Game logic =====*/
        get_score(code, entry, &peg_a, &peg_b);
        render_leds(peg_a, peg_b);

        history[attempt] = entry;
        ++attempt;
    }
    
    /*===== Game result =====*/
    lcd_clear();

    if (peg_a != 4) { // print code that were guessed by player 
        lcd_print_at(0, 0, "You lost!");
        delay(100);
        lcd_print_at(0, 1, "My code: ");
        delay(100);
        lcd_print_at(12, 1, code); 
    } else { lcd_print_at(0, 1, "You won!"); } // greet player with win 

    /*===== Cleanup =====*/
    for(int i = 0; i < MAX_ATTEMPTS; ++i) { free(history[i]); }
    free(history);

    while (digitalRead(BTN_ENTER_PIN) == LOW) { delay(50); }
}

