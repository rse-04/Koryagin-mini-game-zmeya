#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    64
#define BRIGHTNESS  64
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// Кнопки
#define BTN_LEFT    2
#define BTN_DOWN    3
#define BTN_UP      4
#define BTN_RIGHT   7

#define WIDTH  8
#define HEIGHT 8

enum Direction { UP, DOWN, LEFT, RIGHT };
Direction dir = RIGHT;

struct Point {
  uint8_t x;
  uint8_t y;
};

#define MAX_SNAKE_LENGTH 64
Point snake[MAX_SNAKE_LENGTH];
int snakeLength = 3;

Point food;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 300;

int XYtoIndex(uint8_t x, uint8_t y) {
  // Защита от выхода за границы
  if (x >= WIDTH) x = WIDTH - 1;
  if (y >= HEIGHT) y = HEIGHT - 1;
  return y * WIDTH + x;  // Прямая нумерация
}

void placeFood() {
  bool valid = false;
  while (!valid) {
    food.x = random(0, WIDTH);
    food.y = random(0, HEIGHT);
    valid = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        valid = false;
        break;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  // Инициализация змейки в центре
  snake[0] = {4, 4};
  snake[1] = {3, 4};
  snake[2] = {2, 4};

  placeFood();
}

void loop() {
  // Обработка нажатий кнопок (исправленная логика)
  if (digitalRead(BTN_LEFT) == LOW && dir != RIGHT) {
    dir = RIGHT;
  }
  if (digitalRead(BTN_RIGHT) == LOW && dir != LEFT) {
    dir = LEFT;
  }
  if (digitalRead(BTN_UP) == LOW && dir != DOWN) {
    dir = UP;
  }
  if (digitalRead(BTN_DOWN) == LOW && dir != UP) {
    dir = DOWN;
  }

  Serial.print("L:"); Serial.print(digitalRead(BTN_LEFT));
  Serial.print(" R:"); Serial.print(digitalRead(BTN_RIGHT));
  Serial.print(" U:"); Serial.print(digitalRead(BTN_UP));
  Serial.print(" D:"); Serial.println(digitalRead(BTN_DOWN));

  unsigned long now = millis();
  if (now - lastUpdate > updateInterval) {
    lastUpdate = now;

    Point newHead = snake[0];
    switch (dir) {
      case UP:    newHead.y = (newHead.y + 1) % HEIGHT; break;
      case DOWN:  newHead.y = (newHead.y == 0) ? HEIGHT - 1 : newHead.y - 1; break;
      case LEFT:  newHead.x = (newHead.x == 0) ? WIDTH - 1 : newHead.x - 1; break;
      case RIGHT: newHead.x = (newHead.x + 1) % WIDTH; break;
    }

    // Проверка столкновения с телом змейки
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == newHead.x && snake[i].y == newHead.y) {
        // Сброс игры
        snakeLength = 3;
        snake[0] = {4, 4};
        snake[1] = {3, 4};
        snake[2] = {2, 4};
        dir = RIGHT;
        placeFood();
        return;
      }
    }

    // Сдвигаем змейку
    for (int i = snakeLength - 1; i > 0; i--) {
      snake[i] = snake[i - 1];
    }
    snake[0] = newHead;

    // Проверяем еду
    if (newHead.x == food.x && newHead.y == food.y) {
      if (snakeLength < MAX_SNAKE_LENGTH) {
        snakeLength++;
        snake[snakeLength - 1] = snake[snakeLength - 2];
      }
      placeFood();
    }

    // Отрисовка
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
    leds[XYtoIndex(food.x, food.y)] = CRGB::Red;
    for (int i = 0; i < snakeLength; i++) {
      leds[XYtoIndex(snake[i].x, snake[i].y)] = (i == 0) ? CRGB::Green : CRGB(0, 150, 0);
    }
    FastLED.show();

    // Отладка координат еды и головы змейки
    Serial.print("Food: (");
    Serial.print(food.x);
    Serial.print(",");
    Serial.print(food.y);
    Serial.print(") Head: (");
    Serial.print(snake[0].x);
    Serial.print(",");
    Serial.print(snake[0].y);
    Serial.println(")");

    // Отладка змейки
    Serial.print("Snake: ");
    for (int i = 0; i < snakeLength; i++) {
      Serial.print("(");
      Serial.print(snake[i].x);
      Serial.print(",");
      Serial.print(snake[i].y);
      Serial.print(") ");
    }
    Serial.println();
  }
}
